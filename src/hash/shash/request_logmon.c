#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h> 

#include "request_logmon.h"

typedef struct _request_head {
    uint32_t    len;
    uint16_t    type;
    uint8_t     subtype;
    uint8_t     code;
} _REQUEST_HEAD;

typedef struct _request {
    _REQUEST_HEAD       head;
    uint32_t    data_len;
    char        *data;
} _REQUEST; 
            
#define _REQUEST_TYPE   14
    
#define RETURN_RESULT       1
#define RETURN_IMMEDIATELY  2
    
#define _REQ_EXECUTE    1
#define _ACK_SUCCESS    2
#define _ACK_FAILURE    4

#define _ERROR      -1
#define _PROCESSED  0


// TODO:
#define UNET_BYTE_ORDER

#ifdef  UNET_BYTE_ORDER
#define UNET_NTOHL(i)   (i)
#define UNET_NTOHS(i)   (i)
#define UNET_HTONL(i)   (i)
#define UNET_HTONS(i)   (i)
#else       
#define UNET_NTOHL(i)   ntohl(i)
#define UNET_NTOHS(i)   ntohs(i)
#define UNET_HTONL(i)   htonl(i)
#define UNET_HTONS(i)   htons(i)
#endif


/*
 * secs: timeout (seconds) of connection, send, recv
 */
int make_command_socket(int *rscok, uint16_t port, int secs)
{       
    struct sockaddr_in server_addr;
    int sock;
    int socket_opt;
	int flag;
	int error;
	fd_set  rset, wset; 
	struct timeval tval; 

	tval.tv_sec = secs;     
	tval.tv_usec = 0; 

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);// only for localhost
    server_addr.sin_port = htons(port);
    
    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {   
        return -1;
    }

    socket_opt = 1;
	// socket reuse
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &socket_opt, sizeof(int));
	// recv timeout
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tval, sizeof(struct timeval));
	// send timeout
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tval, sizeof(struct timeval));

	if ((flag = fcntl(sock, F_GETFL, 0)) < 0) {
		return -1;
	}
	// non blocking
	if (fcntl(sock, F_SETFL, flag | O_NONBLOCK) < 0) {
		return -1;
	}

	// non blocking connect
	if ((error = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
		if (errno != EINPROGRESS) {
			return -1;
		}
	}

	// success connection
	if (error == 0) {
		fcntl(sock, F_SETFL, flag);
    	*rscok = sock;
		return 0;
	}

	FD_ZERO(&rset);
	FD_SET(sock, &rset); 
	wset = rset; 
    
	if (select(sock + 1, &rset, &wset, NULL, &tval) == 0) {
		errno = ETIMEDOUT;
		return -1;
	}

	error = 0;
	if (FD_ISSET(sock, &rset) || FD_ISSET(sock, &wset)) {
		socklen_t err_len = sizeof(error);
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &err_len) < 0) {
			return -1;
		}
	}
	else {
		return -1;
	}


	fcntl(sock, F_SETFL, flag);
	if (error) {
#ifdef TEST
		if (error == ECONNREFUSED) {
			printf("connection reject\n");
		}
		else if (error == ETIMEDOUT) {
			printf("connection timeout\n");
		}
#endif
		errno = error;
		return -1;
	}

    *rscok = sock;
    return 0;
}

int send_command_request(int sock, const char *command, int require_return, char **return_val)
{
	_REQUEST request;
	_REQUEST_HEAD *head;
	char *msg;
	int bytes;
	int command_type;

	if (command == NULL || command[0] == 0) {
		return -1;
	}
	command_type = (require_return) ? RETURN_RESULT : RETURN_IMMEDIATELY;

	memset(&request, 0, sizeof(_REQUEST));

	request.data_len = strlen(command);

	head = &request.head;
	head->len = UNET_HTONL(sizeof(_REQUEST_HEAD) + request.data_len);
	head->type = UNET_HTONS(_REQUEST_TYPE);
	head->subtype = (uint8_t)command_type;
	head->code = (uint8_t)_REQ_EXECUTE;

	msg = (char *)malloc(UNET_NTOHL(head->len));
	memset(msg, 0, UNET_NTOHL(head->len));
	memcpy(msg, (void *)head, sizeof(_REQUEST_HEAD));
	memcpy(msg + sizeof(_REQUEST_HEAD), command, request.data_len);

	bytes = send(sock, msg, UNET_NTOHL(head->len), 0);
	if (bytes != head->len) {
		free(msg);
		return -1;
	}

	free(msg);

	memset(&request, 0, sizeof(_REQUEST));
	head = &request.head;
	bytes = recv(sock, (void*)head, sizeof(_REQUEST_HEAD), 0);
	if (bytes != sizeof(_REQUEST_HEAD)) {
		return -1;
	}
	if(UNET_NTOHS(head->type) != _REQUEST_TYPE ||
	   head->subtype != command_type ||
		(head->code != _ACK_SUCCESS && 
		 head->code != _ACK_FAILURE)) {
		return -1;
	}

#ifdef TEST
	printf("***Received***\nlen = %d\ntype = %d\nsubtype = %d\nreturn code = %s\n",
                UNET_NTOHL(request.head.len), UNET_NTOHS(request.head.type),
                request.head.subtype,
                request.head.code==2?"SUCCESS":"FAILURE");
#endif


	if (return_val != NULL && require_return)
	{
		request.data_len = UNET_NTOHL(head->len) - sizeof(_REQUEST_HEAD);
		if(request.data_len == 0) {
			return -1;
		}

		request.data = (char*)malloc(request.data_len + 1);
		memset(request.data, 0, request.data_len + 1);

		bytes = recv(sock, (void*)request.data, request.data_len, 0);
		if (bytes != request.data_len) {
			free(request.data);
			return -1;
		}

		*return_val = request.data;
	}

	return 0;
}

#ifdef TEST
main(int a, char **b)
{
	int sock;
	char *result;
	char *command = b[1];

	if (make_command_socket(&sock, _COMMAND_PORT, 1)) {
		printf("socket error (%d)\n", errno);
		return -1;
	}

	if (send_command_request(sock, command, 1, &result)) {
		printf("recv error\n");
		return -1;
	}
	printf("result: %s\n", result);
	free(result);

}
#endif
