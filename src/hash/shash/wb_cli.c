/*
 * wb_cli.c    Handles that are called from eap
 *
 * Copyright 2005  UNETSystem CORP.
 * written by Seung Yong Yang <joshua.yang@samsung.com>
 */

#define F_DUPFD 1
#define O_NONBLOCK 1
#define FD_CLOEXEC 1
#define HAVE_UNIXSOCKET 1
#include "autoconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/des.h>
#include <rad_assert.h>

//#include "eap_mschapv2.h"
#include "wb_cli.h"
#include "radius.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

static BOOL winbind_env_set( void );
BOOL winbind_off( void );
BOOL winbind_on( void );

/* for SUPPORT_CC */
extern const char * cc_int2char(int data);
extern void cc_func_log(const char *func_name, const char *params, const char *ret_val);

/* Global variables.  These are effectively the client state information */

#if 0 
/* by syyang
 * Since. radiusd is multi-threaded. global socket is not needed
 */
static int winbindd_fd = -1;           /* fd for winbindd socket */
static int is_privileged = 0;
#endif

/* Free a response structure */

static void free_response(struct winbindd_response *response)
{
	/* Free any allocated extra_data */

	if (response)
		SAFE_FREE(response->extra_data.data);
}

/* Initialise a request structure */

static void init_request(struct winbindd_request *request, int request_type)
{
	request->length = sizeof(struct winbindd_request);

	request->cmd = (enum winbindd_cmd)request_type;
	request->pid = getpid();

}

/* Initialise a response structure */

static void init_response(struct winbindd_response *response)
{
	/* Initialise return value */

	response->result = WINBINDD_ERROR;
}

/* Close established socket */

static void close_sock(int pipe_sock)
{
	if (pipe_sock != -1) {
		close(pipe_sock);
	}
}

#define CONNECT_TIMEOUT 30

/* Make sure socket handle isn't stdin, stdout or stderr */
#define RECURSION_LIMIT 3

static int make_nonstd_fd_internals(int fd, int limit /* Recursion limiter */) 
{
	int new_fd;
	if (fd >= 0 && fd <= 2) {
#ifdef F_DUPFD 
		if ((new_fd = fcntl(fd, F_DUPFD, 3)) == -1) {
			return -1;
		}
		/* Paranoia */
		if (new_fd < 3) {
			close(new_fd);
			return -1;
		}
		close(fd);
		return new_fd;
#else
		if (limit <= 0)
			return -1;
		
		new_fd = dup(fd);
		if (new_fd == -1) 
			return -1;

		/* use the program stack to hold our list of FDs to close */
		new_fd = make_nonstd_fd_internals(new_fd, limit - 1);
		close(fd);
		return new_fd;
#endif
	}
	return fd;
}

/****************************************************************************
 Set a fd into blocking/nonblocking mode. Uses POSIX O_NONBLOCK if available,
 else
 if SYSV use O_NDELAY
 if BSD use FNDELAY
 Set close on exec also.
****************************************************************************/

static int make_safe_fd(int fd) 
{
	int result, flags;
	int new_fd = make_nonstd_fd_internals(fd, RECURSION_LIMIT);
	if (new_fd == -1) {
		close(fd);
		return -1;
	}

	/* Socket should be nonblocking. */
#ifdef O_NONBLOCK
#define FLAG_TO_SET O_NONBLOCK
#else
#ifdef SYSV
#define FLAG_TO_SET O_NDELAY
#else /* BSD */
#define FLAG_TO_SET FNDELAY
#endif
#endif

	if ((flags = fcntl(new_fd, F_GETFL)) == -1) {
		close(new_fd);
		return -1;
	}

	flags |= FLAG_TO_SET;
	if (fcntl(new_fd, F_SETFL, flags) == -1) {
		close(new_fd);
		return -1;
	}

#undef FLAG_TO_SET

	/* Socket should be closed on exec() */
#ifdef FD_CLOEXEC
	result = flags = fcntl(new_fd, F_GETFD, 0);
	if (flags >= 0) {
		flags |= FD_CLOEXEC;
		result = fcntl( new_fd, F_SETFD, flags );
	}
	if (result < 0) {
		close(new_fd);
		return -1;
	}
#endif
	return new_fd;
}

/* Connect to winbindd socket */

static int winbind_named_pipe_sock(const char *dir)
{
	struct sockaddr_un sunaddr;
	struct stat st;
	pstring path;
	int fd;
	int wait_time;
	int slept;
	
	/* Check permissions on unix socket directory */
	
	if (lstat(dir, &st) == -1) {
		return -1;
	}
	
	if (!S_ISDIR(st.st_mode) || 
	    (st.st_uid != 0 && st.st_uid != geteuid())) {
		return -1;
	}
	
	/* Connect to socket */
	snprintf(path, sizeof(path), "%s/%s", dir, WINBINDD_SOCKET_NAME);
	
	memset(&sunaddr, 0, sizeof(sunaddr));
	sunaddr.sun_family = AF_UNIX;
	strncpy(sunaddr.sun_path, path, sizeof(sunaddr.sun_path) - 1);
	
	/* If socket file doesn't exist, don't bother trying to connect
	   with retry.  This is an attempt to make the system usable when
	   the winbindd daemon is not running. */

	if (lstat(path, &st) == -1) {
		return -1;
	}
	
	/* Check permissions on unix socket file */
	
	if (!S_ISSOCK(st.st_mode) || 
	    (st.st_uid != 0 && st.st_uid != geteuid())) {
		return -1;
	}
	
	/* Connect to socket */
	
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		return -1;
	}

	/* Set socket non-blocking and close on exec. */

	if ((fd = make_safe_fd( fd)) == -1) {
		return fd;
	}

	for (wait_time = 0; connect(fd, (struct sockaddr *)&sunaddr, sizeof(sunaddr)) == -1;
			wait_time += slept) {
		struct timeval tv;
		fd_set w_fds;
		int ret;
		int connect_errno = 0;
		socklen_t errnosize;

		if (wait_time >= CONNECT_TIMEOUT)
			goto error_out;

		switch (errno) {
			case EINPROGRESS:
				FD_ZERO(&w_fds);
				FD_SET(fd, &w_fds);
				tv.tv_sec = CONNECT_TIMEOUT - wait_time;
				tv.tv_usec = 0;

				ret = select(fd + 1, NULL, &w_fds, NULL, &tv);

				if (ret > 0) {
					errnosize = sizeof(connect_errno);

					ret = getsockopt(fd, SOL_SOCKET,
							SO_ERROR, &connect_errno, &errnosize);

					if (ret >= 0 && connect_errno == 0) {
						/* Connect succeed */
						goto out;
					}
				}

				slept = CONNECT_TIMEOUT;
				break;
			case EAGAIN:
				slept = rand() % 3 + 1;
				sleep(slept);
				break;
			default:
				goto error_out;
		}

	}

  out:

	return fd;

  error_out:

	close(fd);
	return -1;
}

/* Connect to winbindd socket */

static int winbind_open_pipe_sock(int recursing, int need_priv)
{
#ifdef HAVE_UNIXSOCKET
#if 0 /* by syyang */
	static pid_t our_pid;
	struct winbindd_request request;
	struct winbindd_response response;
	memset(&request, 0, sizeof(request));
	memset(&response, 0, sizeof(response));

	if (our_pid != getpid()) {
		close_sock();
		our_pid = getpid();
	}

	if ((need_priv != 0) && (is_privileged == 0)) {
		close_sock();
	}
	
	if (winbindd_fd != -1) {
		return winbindd_fd;
	}

	if (recursing) {
		return -1;
	}

	if ((winbindd_fd = winbind_named_pipe_sock(WINBINDD_SOCKET_DIR)) == -1) {
		return -1;
	}

	is_privileged = 0;

	/* version-check the socket */

	request.flags = WBFLAG_RECURSE;
	if ((winbindd_request_response(WINBINDD_INTERFACE_VERSION, &request, &response) != NSS_STATUS_SUCCESS) || (response.data.interface_version != WINBIND_INTERFACE_VERSION)) {
		close_sock();
		return -1;
	}

	/* try and get priv pipe */

	request.flags = WBFLAG_RECURSE;
	if (winbindd_request_response(WINBINDD_PRIV_PIPE_DIR, &request, &response) == NSS_STATUS_SUCCESS) {
		int fd;
		if ((fd = winbind_named_pipe_sock((char *)response.extra_data.data)) != -1) {
			close(winbindd_fd);
			winbindd_fd = fd;
			is_privileged = 1;
		}
	}

	if ((need_priv != 0) && (is_privileged == 0)) {
		return -1;
	}

	SAFE_FREE(response.extra_data.data);

	return winbindd_fd;
#else
	return winbind_named_pipe_sock(WINBINDD_PRIV_SOCKET_DIR);
#endif
#else
	return -1;
#endif /* HAVE_UNIXSOCKET */
}

/* Write data to winbindd socket */

static int write_sock(int pipe_sock, void *buffer, int count, 
					int recursing, int need_priv)
{
	int result, nwritten;
	
	/* Open connection to winbind daemon */
	
#if 0
 restart:
	
	if (winbind_open_pipe_sock(recursing, need_priv) == -1) {
		return -1;
	}
#endif
	
	/* Write data to socket */
	
	nwritten = 0;
	
	while(nwritten < count) {
		struct timeval tv;
		fd_set r_fds;
		
		/* Catch pipe close on other end by checking if a read()
		   call would not block by calling select(). */

		FD_ZERO(&r_fds);
		FD_SET(pipe_sock, &r_fds);
		memset(&tv, 0, sizeof(tv));
		
		if (select(pipe_sock + 1, &r_fds, NULL, NULL, &tv) == -1) {
			return -1;                   /* Select error */
		}
		
		/* Write should be OK if fd not available for reading */
		
		if (!FD_ISSET(pipe_sock, &r_fds)) {
			
			/* Do the write */
			
			result = write(pipe_sock,
				       (char *)buffer + nwritten, 
				       count - nwritten);
			
			if ((result == -1) || (result == 0)) {
				
				/* Write failed */
				
				return -1;
			}
			
			nwritten += result;
			
		} else {
			
			/* Pipe has closed on remote end */
			
			return -1;
		}
	}
	
	return nwritten;
}

/* Read data from winbindd socket */

static int read_sock(int pipe_sock, void *buffer, int count)
{
	int nread = 0;
	int selret;

	if (pipe_sock == -1) {
		return -1;
	}

	/* Read data from socket */
	while(nread < count) {
		struct timeval tv;
		fd_set r_fds;
		
		/* Catch pipe close on other end by checking if a read()
		   call would not block by calling select(). */

		FD_ZERO(&r_fds);
		FD_SET(pipe_sock, &r_fds);
		memset(&tv, 0, sizeof(tv));
		/* Wait for 2 seconds for a reply. May need to parameterise this... */
		tv.tv_sec = 2;

		if ((selret = select(pipe_sock + 1, &r_fds, NULL, NULL, &tv)) == -1) {
			return -1;                   /* Select error */
		}
		
		if (selret == 0) {
			/* Select timeout */
			return -1;
		}

		if (FD_ISSET(pipe_sock, &r_fds)) {
			
			/* Do the Read */
			
			int result = read(pipe_sock, (char *)buffer + nread, 
			      count - nread);
			
			if ((result == -1) || (result == 0)) {
				
				/* Read failed.  I think the only useful thing we
				   can do here is just return -1 and fail since the
				   transaction has failed half way through. */
			
				return -1;
			}
			
			nread += result;
			
		}
	}
	
	return nread;
}

/* Read reply */

static int read_reply(int pipe_sock, struct winbindd_response *response)
{
	int result1, result2 = 0;

	if (!response) {
		return -1;
	}
	
	/* Read fixed length response */
	
	if ((result1 = read_sock(pipe_sock, response, sizeof(struct winbindd_response)))
	    == -1) {
		
		return -1;
	}
	
	/* We actually send the pointer value of the extra_data field from
	   the server.  This has no meaning in the client's address space
	   so we clear it out. */

	response->extra_data.data = NULL;

	/* Read variable length response */
	
	if (response->length > sizeof(struct winbindd_response)) {
		int extra_data_len = response->length - 
			sizeof(struct winbindd_response);
		
		/* Mallocate memory for extra data */
		
		if (!(response->extra_data.data = malloc(extra_data_len))) {
			return -1;
		}
		
		if ((result2 = read_sock(pipe_sock, response->extra_data.data, extra_data_len))
		    == -1) {
			free_response(response);
			return -1;
		}
	}
	
	/* Return total amount of data read */
	
	return result1 + result2;
}

static BOOL winbind_env_set( void )
{
	char *env;
	
	if ((env=getenv(WINBINDD_DONT_ENV)) != NULL) {
		if(strcmp(env, "1") == 0) {
			return True;
		}
	}
	return False;
}

/* 
 * send simple types of requests 
 */

NSS_STATUS winbindd_send_request(int pipe_sock, int req_type, int need_priv,
				 struct winbindd_request *request)
{
	struct winbindd_request lrequest;

	/* Check for our tricky environment variable */

	if (winbind_env_set()) {
		return NSS_STATUS_NOTFOUND;
	}

	if (!request) {
		memset(&lrequest, 0, sizeof(lrequest));
		request = &lrequest;
	}
	
	/* Fill in request and send down pipe */

	init_request(request, req_type);
	
	if (write_sock(pipe_sock, request, sizeof(*request),
		       request->flags & WBFLAG_RECURSE, need_priv) == -1) {
		return NSS_STATUS_UNAVAIL;
	}

	if ((request->extra_len != 0) &&
	    (write_sock(pipe_sock, request->extra_data.data, request->extra_len,
			request->flags & WBFLAG_RECURSE, need_priv) == -1)) {
		return NSS_STATUS_UNAVAIL;
	}
	
	return NSS_STATUS_SUCCESS;
}

/*
 * Get results from winbindd request
 */

NSS_STATUS winbindd_get_response(int pipe_sock, struct winbindd_response *response)
{
	struct winbindd_response lresponse;

	if (!response) {
		memset(&lresponse, 0, sizeof(lresponse));
		response = &lresponse;
	}

	init_response(response);

	/* Wait for reply */
	if (read_reply(pipe_sock, response) == -1) {
		return NSS_STATUS_UNAVAIL;
	}

	/* Throw away extra data if client didn't request it */
	if (response == &lresponse) {
		free_response(response);
	}

	/* Copy reply data from socket */
	if (response->result != WINBINDD_OK) {
		return NSS_STATUS_NOTFOUND;
	}
	
	return NSS_STATUS_SUCCESS;
}

/* Handle simple types of requests */

NSS_STATUS winbindd_request_response(int pipe_sock, int req_type, 
			    struct winbindd_request *request,
			    struct winbindd_response *response)
{
	NSS_STATUS status = NSS_STATUS_UNAVAIL;
	int count = 0;

	while ((status == NSS_STATUS_UNAVAIL) && (count < 10)) {
		status = winbindd_send_request(pipe_sock, req_type, 0, request);
		if (status != NSS_STATUS_SUCCESS) 
			return(status);
		status = winbindd_get_response(pipe_sock, response);
		count += 1;
	}

	return status;
}

/*************************************************************************
 A couple of simple functions to disable winbindd lookups and re-
 enable them
 ************************************************************************/
 
/* Use putenv() instead of setenv() in these functions as not all
   environments have the latter. */

BOOL winbind_off( void )
{
	static char *s = CONST_DISCARD(char *, WINBINDD_DONT_ENV "=1");

	return putenv(s) != -1;
}

BOOL winbind_on( void )
{
	static char *s = CONST_DISCARD(char *, WINBINDD_DONT_ENV "=0");

	return putenv(s) != -1;
}

static int get_winbind_domain(int pipe_sock, char *buf, size_t buf_len)
{
	struct winbindd_response response;

	memset(&response, 0, sizeof(response));

	/* Send off request */
	if (winbindd_request_response(pipe_sock, WINBINDD_DOMAIN_NAME, NULL, &response) !=
		NSS_STATUS_SUCCESS) {
//DEBUG("could not obtain winbind domain name!\n");
		return -1;
    }

	strncpy(buf, response.data.domain_name, buf_len);

	return 0;
}

/** 
 * ------------------------------------------------
 *      
 * @brief 	challenge 와 response 에 대한 검증을 ActiveDirectory 에 의뢰 한다.
 *      
 * @note    1.winbind 접속 설정값 및 옵션을 초기화한다.
 * @note    2.winbind 데몬과 pipe통신을 위한 소켓을 개설한다.
 * @note    3.winbind pipe 소켓으로 Domain 이행을 실시한다.
 * @note    4.challenge에 대한 response를 정상적으로 받으면 user_session_key(nt_hash 키)를 저장한다.
 *      
 * @param   username: 사용자 ID
 *			domain: AD 도메인
 *			workstation: workstation
 *			challenge: peer challenge
 *			nt_response: peer response
 *			user_session_key: nt_hash_hash (해쉬 값)
 *          err_string: 에러 메시지
 *  
 * @return  NTSTATUS: 수행 결과 상태 값
 *
 * @date    2011/02/21
 * @author  강석주(kinow@unet.kr)
 *         
 * ------------------------------------------------
 */
NTSTATUS contact_winbind_auth_crap(const char *username,
                   const char *domain,
                   const char *workstation,
                   const unsigned char *challenge,
                   const unsigned char *nt_response,
                   unsigned char user_session_key[16],
                   char **error_string)
{
	NTSTATUS nt_status;
	NSS_STATUS result;
	struct winbindd_request *request = NULL;
	struct winbindd_response *response = NULL;
	int pipe_sock = -1;

	///1.winbind 접속 설정값 및 옵션을 초기화한다.
	request = (struct winbindd_request *) malloc(sizeof(struct winbindd_request));
	response = (struct winbindd_response *) malloc(sizeof(struct winbindd_response));

	if (request == NULL || response == NULL) {
		free(request);
		free(response);
		cc_func_log("contact_winbind_auth_crap", 
					"username,domain,workstation,challenge,nt_response,user_session_key,error_string", 
					"NT_STATUS_UNSUCCESSFUL");
		return NT_STATUS_UNSUCCESSFUL;
	}
	memset(request, 0, sizeof(*request));
	memset(response, 0, sizeof(*response));

	request->flags = WBFLAG_PAM_USER_SESSION_KEY;

	request->data.auth_crap.logon_parameters = 
	MSV1_0_ALLOW_WORKSTATION_TRUST_ACCOUNT | MSV1_0_ALLOW_SERVER_TRUST_ACCOUNT;

	///2.winbind 데몬과 pipe통신을 위한 소켓을 개설한다.
	pipe_sock = winbind_open_pipe_sock(0, 1);
	if (pipe_sock < 0) {
		free(request);
		free(response);
		cc_func_log("contact_winbind_auth_crap", 
					"username,domain,workstation,challenge,nt_response,user_session_key,error_string", 
					"NT_STATUS_UNSUCCESSFUL");
		return NT_STATUS_UNSUCCESSFUL;
	}

	///3.winbind pipe 소켓으로 Domain 이행을 실시한다.
	strncpy(request->data.auth_crap.user, username, sizeof(request->data.auth_crap.user));
	if (domain != NULL)
		strncpy(request->data.auth_crap.domain, domain, sizeof(request->data.auth_crap.domain));
	else
		get_winbind_domain(pipe_sock, request->data.auth_crap.domain, sizeof(fstring));

	if (workstation) 
		strncpy(request->data.auth_crap.workstation, workstation, sizeof(request->data.auth_crap.workstation));
	else
		strncpy(request->data.auth_crap.workstation, "", sizeof(request->data.auth_crap.workstation));

	memcpy(request->data.auth_crap.chal, challenge, 8);

	if (nt_response) {
		memcpy(request->data.auth_crap.nt_resp, nt_response, 24);
		request->data.auth_crap.nt_resp_len = 24;
	}

	///4.challenge에 대한 response를 정상적으로 받으면 user_session_key(nt_hash 키)를 저장한다.
	result = winbindd_request_response(pipe_sock, WINBINDD_PAM_AUTH_CRAP, 
			request, response);

    /* Display response */
    if ((result != NSS_STATUS_SUCCESS) && (response->data.auth.nt_status == 0)) {
		nt_status = NT_STATUS_UNSUCCESSFUL;
		if (error_string)
			*error_string = strdup("Reading winbind reply failed!");
		free_response(response);
		close_sock(pipe_sock);
		free(request);
		free(response);
		cc_func_log("contact_winbind_auth_crap", 
					"username,domain,workstation,challenge,nt_response,user_session_key,error_string", 
					cc_int2char((int)nt_status));
		return nt_status;
	}

	nt_status = (NT_STATUS(response->data.auth.nt_status));
    if (!NT_STATUS_IS_OK(nt_status)) {
		if (error_string)
			*error_string = strdup(response->data.auth.error_string);
		free_response(response);
		close_sock(pipe_sock);
		free(request);
		free(response);
		cc_func_log("contact_winbind_auth_crap", 
					"username,domain,workstation,challenge,nt_response,user_session_key,error_string", 
					cc_int2char((int)nt_status));
		return nt_status;
	}

	memcpy(user_session_key, response->data.auth.user_session_key,
			sizeof(response->data.auth.user_session_key));

	free_response(response);
	close_sock(pipe_sock);
	free(request);
	free(response);
	cc_func_log("contact_winbind_auth_crap", 
				"username,domain,workstation,challenge,nt_response,user_session_key,error_string", 
				cc_int2char((int)nt_status));
	return nt_status;
}


/* contact server to change user password using auth crap */
NTSTATUS contact_winbind_change_pswd(const char *username,
                              const char *domain,
                              const unsigned char *new_nt_pswd,
                              const unsigned char *old_nt_hash_enc,
                              char  **error_string)
{
    NTSTATUS nt_status;
    NSS_STATUS result;
    struct winbindd_request *request = NULL;
    struct winbindd_response *response = NULL;
	int pipe_sock;

	request = (struct winbindd_request *) malloc(sizeof(struct winbindd_request));
	response = (struct winbindd_response *) malloc(sizeof(struct winbindd_response));

	if (request == NULL || response == NULL) {
		free(request);
		free(response);
		return NT_STATUS_UNSUCCESSFUL;
	}
	memset(request, 0, sizeof(*request));
	memset(response, 0, sizeof(*response));

	pipe_sock = winbind_open_pipe_sock(0, 1);
	if (pipe_sock < 0) {
		free(request);
		free(response);
		return NT_STATUS_UNSUCCESSFUL;
	}

    if (username != NULL)
        strncpy(request->data.chng_pswd_auth_crap.user, username, sizeof(fstring));
    if (domain != NULL) {
        strncpy(request->data.chng_pswd_auth_crap.domain,domain,sizeof(fstring));
	}
	else {
		/* get domain information */
		get_winbind_domain(pipe_sock, request->data.chng_pswd_auth_crap.domain, sizeof(fstring));
	}

    memcpy(request->data.chng_pswd_auth_crap.new_nt_pswd, new_nt_pswd, 
			sizeof(request->data.chng_pswd_auth_crap.new_nt_pswd));
	request->data.chng_pswd_auth_crap.new_nt_pswd_len = 516;

    memcpy(request->data.chng_pswd_auth_crap.old_nt_hash_enc, old_nt_hash_enc, 
			sizeof(request->data.chng_pswd_auth_crap.old_nt_hash_enc));
    request->data.chng_pswd_auth_crap.old_nt_hash_enc_len = 16;

    result = winbindd_request_response(pipe_sock,
				WINBINDD_PAM_CHNG_PSWD_AUTH_CRAP, request, response);
                                                                                
    /* Display response */
                                                                                
    if ((result != NSS_STATUS_SUCCESS) && (response->data.auth.nt_status == 0)) {
		nt_status = NT_STATUS_UNSUCCESSFUL;
		free_response(response);
		close_sock(pipe_sock);
		free(request);
		free(response);
		return nt_status;
    }

    nt_status = (NT_STATUS(response->data.auth.nt_status));
    if (!NT_STATUS_IS_OK(nt_status)) {
        if (error_string)
            *error_string = strdup(response->data.auth.error_string);
        free_response(response);
		close_sock(pipe_sock);
		free(request);
		free(response);
        return nt_status;
    }

    free_response(response);                                                   
	close_sock(pipe_sock);
	free(request);
	free(response);

    return nt_status;
}

/**     
 * ------------------------------------------------
 *      
 * @brief   Do plain test password authentication (AD environment)
 *
 * @param   user: username (not including domain component)
 *          domain: domain (if NULL, default domain is used
 *          passw: plaintext password
 *          error_string: error string returned if any
 *  
 * @return  NTSTATUS (0, if successful, NT status code if fail)
 *  
 * @date    2011/07/18
 * @author  Seung Yong Yang(syyang@unet.kr)
 *   
 * ------------------------------------------------
 */
NTSTATUS check_plaintext_auth(const char *user,
							const char *domain,
							const char *pass,
							char **error_string)
{
	NTSTATUS nt_status;
	NSS_STATUS result;
	struct winbindd_request *request = NULL;
	struct winbindd_response *response = NULL;
	int pipe_sock = -1;
	fstring username;
	fstring domain_part;

	///1.winbind 접속 설정값 및 옵션을 초기화한다.
	request = (struct winbindd_request *) malloc(sizeof(struct winbindd_request));
	response = (struct winbindd_response *) malloc(sizeof(struct winbindd_response));

	if (request == NULL || response == NULL) {
		free(request);
		free(response);
		cc_func_log("check_plaintext_auth", 
					"user,domain,pass,error_string", 
					"NT_STATUS_UNSUCCESSFUL");
		return NT_STATUS_UNSUCCESSFUL;
	}
	memset(request, 0, sizeof(*request));
	memset(response, 0, sizeof(*response));

	///2.winbind 데몬과 pipe통신을 위한 소켓을 개설한다.
	pipe_sock = winbind_open_pipe_sock(0, 1);
	if (pipe_sock < 0) {
		free(request);
		free(response);
		cc_func_log("check_plaintext_auth", 
					"user,domain,pass,error_string", 
					"NT_STATUS_UNSUCCESSFUL");
		return NT_STATUS_UNSUCCESSFUL;
	}

	memset(username, 0, sizeof(username));
	memset(domain_part, 0, sizeof(domain_part));

	if (domain != NULL) {
		snprintf(username, sizeof(username), "%s\\%s", domain, user);
	}
	else {
		get_winbind_domain(pipe_sock, domain_part, sizeof(fstring));
		snprintf(username, sizeof(username), "%s\\%s", domain_part, user);
	}

	///3.winbind pipe 소켓으로 Domain 이행을 실시한다.
	strncpy(request->data.auth.user, username, sizeof(fstring));
	strncpy(request->data.auth.pass, pass, sizeof(fstring));

	result = winbindd_request_response(pipe_sock, WINBINDD_PAM_AUTH, 
			request, response);

    /* Display response */
    if ((result != NSS_STATUS_SUCCESS) && (response->data.auth.nt_status == 0)) {
		nt_status = NT_STATUS_UNSUCCESSFUL;
		if (error_string)
			*error_string = strdup("Reading winbind reply failed!");
		free_response(response);
		close_sock(pipe_sock);
		free(request);
		free(response);
		cc_func_log("check_plaintext_auth", 
					"user,domain,pass,error_string", 
					cc_int2char((int)nt_status));
		return nt_status;
	}

	nt_status = (NT_STATUS(response->data.auth.nt_status));
    if (!NT_STATUS_IS_OK(nt_status)) {
		if (error_string)
			*error_string = strdup(response->data.auth.error_string);
		free_response(response);
		close_sock(pipe_sock);
		free(request);
		free(response);
		cc_func_log("check_plaintext_auth", 
					"user,domain,pass,error_string", 
					cc_int2char((int)nt_status));
		return nt_status;
	}

	free_response(response);
	close_sock(pipe_sock);
	free(request);
	free(response);
	cc_func_log("check_plaintext_auth", 
				"user,domain,pass,error_string", 
				cc_int2char((int)nt_status));
	return nt_status;
}

int nt_status_to_error_code(NTSTATUS nt_status)
{
	int reason_code = 0;
	switch (nt_status)
	{
	case NT_STATUS_ACCOUNT_DISABLED:
		reason_code = PW_ERR_INVALID_STATUS_D;
		break;
	case NT_STATUS_ACCOUNT_EXPIRED:
		reason_code = PW_ERR_INVALID_STATUS_I;
		break;
	case NT_STATUS_INVALID_LOGON_HOURS:
		reason_code = PW_ERR_INVALID_USER_TIME;
		break;
	case NT_STATUS_WRONG_PASSWORD:
		reason_code = PW_ERR_WRONG_PASSWD;
		break;
	case NT_STATUS_NO_SUCH_USER:
		reason_code = PW_ERR_USER_NOT_FOUND;
		break;
	case 0x1c010014:
		reason_code = PW_ERR_SERVER_BUSY;
		break;
	case NT_STATUS_PASSWORD_MUST_CHANGE:
	case NT_STATUS_PASSWORD_EXPIRED:
		reason_code = PW_ERR_PASSWORD_CHANGE;
		break;
	default:
		reason_code = PW_ERR_NT_DOMAIN_LOGON_FAIL;
		break;
	}
	return reason_code;
}

#ifdef TEST
int main(void)
{
	char *error_string = NULL;
	const char *user = "dnyang";
	const char *pass = "a123456A";
	NTSTATUS nt_status;
	nt_status = check_plaintext_auth(user, NULL, pass, &error_string);

	printf("authentication result is %d %s\n", nt_status, error_string);

	return 0;
}
#endif
