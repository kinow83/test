#ifndef RADIUSD_H
#define RADIUSD_H
/*
 * radiusd.h	Structures, prototypes and global variables
 *		for the FreeRADIUS server.
 *
 * Version:	$Id: radiusd.h,v 1.1.1.1 2006/03/05 08:29:47 cvs Exp $
 *
 */
#include "libradius.h"
#include "radpaths.h"
#include "conf.h"
#include "conffile.h"

#include <stdarg.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PTHREAD_H
#include	<pthread.h>
typedef pthread_t child_pid_t;
#define child_kill pthread_kill
#else
typedef pid_t child_pid_t;
#define child_kill kill
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "missing.h"

#define NO_SUCH_CHILD_PID (child_pid_t) (0)

#ifndef NDEBUG
#define REQUEST_MAGIC (0xdeadbeef)
#endif

#ifndef CBYTE
#define CBYTE char
#endif


/*
 *	See util.c
 */
typedef struct request_data_t request_data_t;

#define REQUEST_DATA_REGEX (0xadbeef00)
#define REQUEST_MAX_REGEX (8)

typedef struct auth_req {
#ifndef NDEBUG
	uint32_t		magic; /* for debugging only */
#endif
	RADIUS_PACKET		*packet;
	RADIUS_PACKET		*proxy;
	RADIUS_PACKET		*reply;
	RADIUS_PACKET		*proxy_reply;
	VALUE_PAIR		*config_items;
	VALUE_PAIR		*username;
	VALUE_PAIR		*password;
// added by ryoo	2012.06.
	VALUE_PAIR		*calledid;
	VALUE_PAIR		*callingid;
//
	request_data_t		*data;
	char			secret[48];
	child_pid_t    		child_pid;
	time_t			timestamp;
	int			number; /* internal server number */

	/*
	 *	We could almost keep a const char here instead of a
	 *	_copy_ of the secret... but what if the RADCLIENT
	 *	structure is freed because it was taken out of the
	 *	config file and SIGHUPed?
	 */
	char			proxysecret[48];
	int			proxy_try_count;
	int			proxy_outstanding;
	time_t			proxy_next_try;

	int                     simul_max;
	int                     simul_count;
	int                     simul_mpp; /* WEIRD: 1 is false, 2 is true */

	int			finished;
	int			options; /* miscellanous options */
	void			*container;
} REQUEST;

#define RAD_REQUEST_OPTION_NONE            (0)
#define RAD_REQUEST_OPTION_LOGGED_CHILD    (1 << 0)
#define RAD_REQUEST_OPTION_DELAYED_REJECT  (1 << 1)
#define RAD_REQUEST_OPTION_DONT_CACHE      (1 << 2)
#define RAD_REQUEST_OPTION_FAKE_REQUEST    (1 << 3)
#define RAD_REQUEST_OPTION_REJECTED        (1 << 4)
#define RAD_REQUEST_OPTION_PROXIED         (1 << 5)

/*
 *  Function handler for requests.
 */
typedef		int (*RAD_REQUEST_FUNP)(REQUEST *);

typedef struct radclient_list RADCLIENT_LIST;

typedef struct radclient {
	fr_ipaddr_t		ipaddr;
	int				prefix;
	char			longname[128];
	u_char			secret[72];
	char			shortname[64];
	char			nastype[32];
	char			login[32];
	char			password[32];
	char            vlan_use; /* added by hazard */
#define  NAS_TYPE_AP			'A'
#define  NAS_TYPE_SWITCH		'S'
#define  NAS_TYPE_FIREWALL		'F'
#define  NAS_TYPE_ETC			'E'
	char			classification;
	int				number;	/* internal use only */

#define NAS_DHCP_NO             0
#define NAS_DHCP_YES            1
#define NAS_DHCP_GLOBAL         2

    int             dhcp_use;     // nas 별 dhcp 적용여부
    char			macaddr[18];	//NAC Called-station-id
    
} RADCLIENT;

/*
 *  Group reply attribute structure
 *  If extdb is used. if user belongs to group
 *  reply attribute is consulted.
 */
typedef struct gp_reply_attr {
	char    target_id[40];	// group member attribute
	char    attr_id[40];
	char    op[6];
	char    value[256];
	struct gp_reply_attr *next;
} GP_REPLY_ATTR;

typedef struct nas {
	fr_ipaddr_t		ipaddr;
	char			longname[256];
	char			shortname[32];
	char			nastype[32];
	struct nas		*next;
} NAS;

typedef struct _realm {
	char			realm[64];
	char			server[64];
	char			acct_server[64];
	fr_ipaddr_t		ipaddr;	/* authentication */
	fr_ipaddr_t		acct_ipaddr;
	u_char			secret[48];
	time_t			last_reply; /* last time we saw a packet */
	int			auth_port;
	int			acct_port;
	int			striprealm;
	int			trusted; /* old */
	int			notrealm;
	int			active;	/* is it dead? */
	time_t			wakeup;	/* when we should try it again */
	int			acct_active;
	time_t			acct_wakeup;
	int			ldflag;
	struct _realm		*next;
} REALM;

typedef struct pair_list {
	char			*name;
	VALUE_PAIR		*check;
	VALUE_PAIR		*reply;
	int			lineno;
	struct pair_list	*next;
	struct pair_list	*lastdefault;
} PAIR_LIST;


/*
 *	Types of listeners.
 */
typedef enum RAD_LISTEN_TYPE {
	RAD_LISTEN_NONE = 0,
	RAD_LISTEN_AUTH,
	RAD_LISTEN_ACCT,
	RAD_LISTEN_PROXY
} RAD_LISTEN_TYPE;


/*
 *	For listening on multiple IP's and ports.
 */
typedef struct rad_listen_t {
	struct rad_listen_t *next; /* could be rbtree stuff */
	fr_ipaddr_t		ipaddr;
	RAD_LISTEN_TYPE	type;
	int		port;
	int		fd;
	char	interface[128];
} rad_listen_t;


typedef struct main_config_t {
	struct main_config *next;
	time_t		config_dead_time;
	fr_ipaddr_t	myip;
	int		log_auth;
	int		log_auth_badpass;
	int		log_auth_goodpass;
	int		do_usercollide;
#ifdef WITH_SNMP
	int		do_snmp;
#endif
	int		allow_core_dumps;
	int		debug_level;
	int		proxy_requests;
	int		post_proxy_authorize;
	int		wake_all_if_all_dead;
	int		proxy_synchronous;
	int		proxy_dead_time;
	int		proxy_retry_count;
	int		proxy_retry_delay;
	int		proxy_fallback;
	int		reject_delay;
	int		status_server;
	int		max_request_time;
	int		cleanup_delay;
	int		max_requests;
	int		kill_unresponsive_children;
	char 		*do_lower_user;
	char		*do_lower_pass;
	char		*do_nospace_user;
	char		*do_nospace_pass;
	char		*nospace_time;
	char		*log_file;
	char		*checkrad;
	const char      *pid_file;
	const char	*uid_name;
	const char	*gid_name;
	rad_listen_t	*listen;
	CONF_SECTION	*config;
	REALM		*realms;

    /* added by hazard  2004.12 */
    const char  *start_file;  /* startup time */
    const char  *log_use ;   /* file log */
    const char  *log_type;
    const char  *auth_log_use;
    const char  *auth_log_type;
    const char  *accounting_use;
    const char  *mac_check; 
	int   		init_mac_on_auth;
    int  system_account_auth;
    /* added by hazard for checking license 2005.01.19 */

    const char  *lic_serial_number;
    const char  *lic_number_of_users;
    const char  *lic_license_type;
    const char  *lic_expiration_date;
    const char  *lic_feature;
    const char  *lic_signature;
    const char  *anonymous_id ;
    int         default_vlan_use ;
    const char  *default_vlan_id;

	// added by hazard 2005.09.16 syyang
	int		block_dup_login;
	const char *extDbType;
    int   session_timeout;
        /* added by hazard   */
	/* added by syyang : 2005.09.25 */
    int   log_use_flag ;

#define LOG_TYPE_FLAG_INFO	0
#define LOG_TYPE_FLAG_AUTH	1
#define LOG_TYPE_FLAG_ERROR	2
    int   log_type_flag;
    int   auth_log_use_flag;
#define AUTH_LOG_TYPE_FLAG_BOTH 0x03
#define AUTH_LOG_TYPE_FLAG_SUCCESS 0x02
#define AUTH_LOG_TYPE_FLAG_FAILURE 0x01
    int   auth_log_type_flag;
	/* end added by syyang : 2005.09.25 */

	/* added by hazard 2005.11. */
    int   accounting_use_flag;
    int   mac_check_flag; 
	int   syslog_use;
	const char *server_ip_addr;	/* included in /opt/anyclick/.env VIPADDRESS */
	const char *ipaddress;		/* included in /opt/anyclick/.env IPADDRESS */
	const char *ha_enable;
	int   was_port; 

	/*
	 * Added for peapv1 peapv2 and msg sent to client
	 */
	int   peap_version; 
	char *cli_update_url;	// 업데이트 서버 주소 env_tbl 에서 읽는다.
	char *pms_server_ip; 	// PMS 서버 주소 env_tbl 에서 읽는다.
	int	  force_dhcp;			// 강제 DHCP 를 사용할 것인가 env_tbl

	char *cli_msg;			// message to be sent to client when success

    /*
     * Added by kenneth 2008.2.20
     */  
    int  use_peap_default_acl;  // PEAP 사용시 default ACL을 사용할지 여부.
    char *default_acl_file;     // default ACL 파일.

    char *no_logging_user;      // radauthlog 에 로그기록하지 않을 사용자.
    char *no_password_check_user;      // 인증시 패스워드 체크하지 않는 사용자 (무조건 인증됨)

	int  portman_port;
	int   portman_sock;
	struct sockaddr_in portman_dest;

	int initech_ldap_use ;    // added by hazard for KTN

	/*
	 * tunnel external authentication configuration variables
	 * added by gyeongsu.kim
	 */
	int use_tunnel_extauth;
	int tunnel_extauth_port;
	//uint32_t tunnel_extauth_ip;
	const char *tunnel_extauth_ip;
	const char *tunnel_extauth_secret;

	/*
	 * hardware id check configuration variables
	 * added by gyeongsu.kim
	 */
	int check_hw_id;
	int hw_id_mapping;

	/*
	 * force reject when sql not found occurs
	 */
	int reject_on_sqlnotfound;

	/*
	 * force assign client(agent)'s IP address from server
	 */
	int force_static_ip;

	/*
	 * force authorize for hyundai auto
	 */
	int use_hd_gw_auth;

	/*
	 * turn to Authentication by pass when server cannot perform auth
	 * i.e. LDAP server down ...
	 */
	int use_auth_bypass;

	/*
	 * whether auth byass in turned on
	 * if you want to turn off bypass. signal HUP to radiusd
	 */
	int auth_bypass_running;
	int reject_on_mac_unavailable;

	/* Kookmin bank certification authentication */
	int use_kbcert;
	const char *extlib_dir;
	const char *kbcert_mod;
	const char *kbcert_conffile;
	const char *kbcert_cache_dir;

	/*  for group reply attribute processing in extdb */
	GP_REPLY_ATTR  *gp_reply_attrs;

	int extdb_reject_multiple_login;

	/* Password column is hexa encoded NtPasswordHashHash */
	int extdb_get_nt_password;

	/* Password column is sha1-hashed hexa string */
	int sha1_hashed_hexa_password;
	int sha256_hashed_hexa_password;
	int sha512_hashed_hexa_password;

	/*
	 * Juniper switches never send CALLING-STATION-ID on authentication,
	 * to that we have to update radonline table on accounting message
	 */
	int update_radonline_on_accounting;
	int determine_nas_by_mac;	// NAS의 MAC주소 등록 허용여부

	/*
	 * peap 인증시 사용자 인증 성공 전송후 client 응답의 tunneled packet에서
	 * TLV 패킷을 찾을 수 없을때 인증 과정을 계속하도록 하는 옵션.
	 */
	int ignore_non_tlv_packet_on_peap;

	/*
	 * by sjkang. FAST protocol version
	 */
	int fast_version;
	int ignore_non_tlv_packet_on_fast;
	int use_fast_default_acl;  // FAST 사용시 default ACL을 사용할지 여부.
	int unique_ipmac;	// 사용자 MAC이 unique 한 경우만 인증성공 added by sjkang.

	const char *server_ip_addr_v6; /* included in /opt/anyclick/.env VIPADDRESS_V6 */
    const char *ipaddress_v6;      /* included in /opt/anyclick/.env IPADDRESS_V6 */
	fr_ipaddr_t	myip_v6;
	const char *interface;		/* for binding IPv6 by bind_address_v6 */
	
    /* Force set src-ip-address to NAS-IP-Address. */
    int force_src_to_nasaddr;

	/* Password column is md5 hexa password */
	int extdb_get_md5_hexa_password;
	int use_pap_external_jsp;
	char	*ldap_auth_type;
	int	max_init_mac_cnt;
	
	/* added by ryoo : 2012. 06 */
	int radauth_detail_log;
	int enable_send_log;

	/* global env: SQL login/password */
	char *sql_login;
	char *sql_password;

	int cisco_accounting_username_bug;
	int pap_external_jsp_force;

	/* WAUS support */
	int waus_system_support;

	int	use_virtual_user;
} MAIN_CONFIG_T;

#define DEBUG	if (debug_flag)log_debug
#define DEBUG2  if (debug_flag > 1)log_debug
#define DEBUG3  if (debug_flag > 2)log_debug

#define SECONDS_PER_DAY		86400
#define MAX_REQUEST_TIME	30
#define CLEANUP_DELAY		5
#define MAX_REQUESTS		256
#define RETRY_DELAY             5
#define RETRY_COUNT             3
#define DEAD_TIME               120

#define L_DBG			1
#define L_AUTH			2
#define L_INFO			3
#define L_ERR			4
#define L_PROXY			5
#define L_CONS			128

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
/*
 *	This definition of true as NOT false is definitive. :) Making
 *	it '1' can cause problems on stupid platforms.  See articles
 *	on C portability for more information.
 */
#define TRUE (!FALSE)
#endif

/* for paircompare_register */
typedef int (*RAD_COMPARE_FUNC)(void *instance, REQUEST *,VALUE_PAIR *, VALUE_PAIR *, VALUE_PAIR *, VALUE_PAIR **);

typedef enum radlog_dest_t {
  RADLOG_FILES = 0,
  RADLOG_SYSLOG,
  RADLOG_STDOUT,
  RADLOG_STDERR,
  RADLOG_NULL
} radlog_dest_t;

/*
 *	Global variables.
 *
 *	We really shouldn't have this many.
 */
extern const char	*progname;
extern int		debug_flag;
extern int		syslog_facility;
extern const char	*radacct_dir;
extern const char	*radlog_dir;
extern const char	*radlib_dir;
extern const char	*radius_dir;
extern const char	*radius_libdir;
extern radlog_dest_t	radlog_dest;
extern uint32_t		expiration_seconds;
extern int		log_stripped_names;
extern int		log_auth_detail;
extern int		acct_port;
extern const char      *radiusd_version;

/*
 *  The following 3 variables are introduced due to TLS session resumption
 */
extern int		eaptls_session_idx;
extern void		*rad_ssl_ctx;
extern time_t	ssl_sessions_last_flushed;

/*
 *	Function prototypes.
 */

/* acct.c */
int		rad_accounting(REQUEST *);

/* session.c */
int		rad_check_ts(uint32_t nasaddr, unsigned int port, const char *user,
			     const char *sessionid);
int		session_zap(REQUEST *request, uint32_t nasaddr,
			    unsigned int port, const char *user,
			    const char *sessionid, uint32_t cliaddr,
			    char proto);

/* radiusd.c */
#ifndef _LIBRADIUS
void		debug_pair(FILE *, VALUE_PAIR *);
#endif
int		log_err (char *);
int		rad_respond(REQUEST *, RAD_REQUEST_FUNP fun);

/* util.c */
void (*reset_signal(int signo, void (*func)(int)))(int);
void		request_free(REQUEST **request);
int		rad_mkdir(char *directory, int mode);
int		rad_checkfilename(const char *filename);
void		*rad_malloc(size_t size); /* calls exit(1) on error! */
void		xfree(const char *ptr);
REQUEST		*request_alloc(void);
REQUEST		*request_alloc_fake(REQUEST *oldreq);
int		request_data_add(REQUEST *request,
				 void *unique_ptr, int unique_int,
				 void *opaque, void (*free_opaque)(void *));
void		*request_data_get(REQUEST *request,
				  void *unique_ptr, int unique_int);
void		request_reject(REQUEST *request);
void		rfc_clean(RADIUS_PACKET *packet);

/* client.c */
RADCLIENT_LIST *clients_init(void);
RADCLIENT	*client_find(const fr_ipaddr_t *ipaddr);
RADCLIENT	*client_find_by_mac(char *macaddr);
const char	*client_name(const fr_ipaddr_t *ipaddr);
char		*client_name_by_mac(char *macaddr);
void		clients_free(RADCLIENT_LIST *clients);
void		clients_free_global(void);
void        client_free(RADCLIENT *client);
int     client_add(RADCLIENT_LIST *clients, RADCLIENT *client);
int		client_add_by_mac(RADCLIENT_LIST *clients, RADCLIENT *client);
RADCLIENT   *client_findbynumber(const RADCLIENT_LIST *clients,
                     int number);
char client_vlanuse(const fr_ipaddr_t *ipaddr);

/* files.c */
REALM		*realm_find(const char *, int);
REALM		*realm_findbyaddr(fr_ipaddr_t *ipaddr, int port);
void		realm_free(REALM *cl);
void		realm_disable(fr_ipaddr_t *ipaddr, int port);
int		pairlist_read(const char *file, PAIR_LIST **list, int complain);
void		pairlist_free(PAIR_LIST **);
int		read_config_files(void);
int		read_realms_file(const char *file);

/* version.c */
void		version(void);

/* log.c */ //modified by ryoo	2012.06.
int		vradlog(REQUEST *, int, int, const char *, va_list ap);
int		radlog(REQUEST *, int, const char *, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 3, 4)))
#endif
;
int		radsendlog(REQUEST *, int, const char *, ...)
// added by ryoo 2012.06.
#ifdef	__GNUC__
		__attribute__ ((format (printf, 3, 4)))
#endif
;
//
int		log_debug(const char *, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 1, 2)))
#endif
;
void 		vp_listdebug(VALUE_PAIR *vp);

/* proxy.c */
int proxy_receive(REQUEST *request);
int proxy_send(REQUEST *request);

/* auth.c */
char	*auth_name(char *buf, size_t buflen, REQUEST *request, int do_cli);
int		rad_authenticate (REQUEST *);
int		rad_check_password(REQUEST *request);
int		rad_postauth(REQUEST *);
int     module_insert_authlog(  REQUEST *request ); /* syyang */
int     module_online_user(  REQUEST *request ); /* syyang */

/* exec.c */
int		radius_exec_program(const char *,  REQUEST *, int,
				    char *user_msg, int msg_len,
				    VALUE_PAIR *input_pairs,
				    VALUE_PAIR **output_pairs);

/* timestr.c */
int		timestr_match(char *, time_t);

/* valuepair.c */
int		paircompare_register(int attr, int otherattr,
				     RAD_COMPARE_FUNC func,
				     void *instance);
void		paircompare_unregister(int attr, RAD_COMPARE_FUNC func);
int		paircmp(REQUEST *req, VALUE_PAIR *request, VALUE_PAIR *check,
			VALUE_PAIR **reply);
int		simplepaircmp(REQUEST *, VALUE_PAIR *, VALUE_PAIR *);
void		pair_builtincompare_init(void);
void		pairxlatmove(REQUEST *, VALUE_PAIR **to, VALUE_PAIR **from);

/* xlat.c */
typedef int (*RADIUS_ESCAPE_STRING)(char *out, int outlen, const char *in);

int            radius_xlat(char * out, int outlen, const char *fmt,
			   REQUEST * request, RADIUS_ESCAPE_STRING func);
typedef int (*RAD_XLAT_FUNC)(void *instance, REQUEST *, char *, char *, size_t, RADIUS_ESCAPE_STRING func);
int		xlat_register(const char *module, RAD_XLAT_FUNC func, void *instance);
void		xlat_unregister(const char *module, RAD_XLAT_FUNC func);


/* threads.c */
extern		int thread_pool_init(void);
extern		int thread_pool_clean(time_t now);
extern		int thread_pool_addrequest(REQUEST *, RAD_REQUEST_FUNP);
extern		pid_t rad_fork(void);
extern		pid_t rad_waitpid(pid_t pid, int *status);
extern          int total_active_threads(void);

/*added by hazard....*/
extern		int check_license(void);
/* end added by hazard */

#ifndef HAVE_PTHREAD_H
#define rad_fork(n) fork()
#define rad_waitpid waitpid
#endif

/* mainconfig.h */
/* Define a global config structure */
extern struct main_config_t mainconfig;

int proxy_new_listener(fr_ipaddr_t *ipaddr);
int read_mainconfig(int reload);
int free_mainconfig(void);
CONF_SECTION *read_radius_conf_file(void); /* for radwho and friends. */

void lrad_bin2hex(const uint8_t *bin, char *hex, size_t len);

/* auth_external_jsp.c  */
int authentication_with_external_jsp(REQUEST *request);
void external_jsp_init();
void external_jsp_deinit();

#endif /*RADIUSD_H*/
