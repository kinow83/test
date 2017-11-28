/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2005, 2007-2012, 2014-2017 Free Software
   Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _POP3D_H
#define _POP3D_H	1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <mailutils/types.h>
#include <mailutils/stream.h>
#include <mailutils/io.h>
#include <mailutils/dbm.h>
#include <mu_umaxtostr.h>
#include <muaux.h>

/* The implementation */
#define	IMPL		"GNU POP3 Daemon"

/* APOP password file, without .db or .passwd, which are added based on file
   type automatically */
#define APOP_PASSFILE_NAME "apop"

#ifdef ENABLE_DBM
# define APOP_PASSFILE SYSCONFDIR "/" APOP_PASSFILE_NAME ".db"
# define ENABLE_LOGIN_DELAY
#else
# define APOP_PASSFILE SYSCONFDIR "/" APOP_PASSFILE_NAME ".passwd"
# undef ENABLE_LOGIN_DELAY
#endif

struct pop3d_session;

#ifdef ENABLE_LOGIN_DELAY
# define LOGIN_STAT_FILE "/var/run/pop3-login"
extern time_t login_delay;
extern char *login_stat_file;
extern int check_login_delay (char *username);
extern void update_login_delay (char *username);
extern void login_delay_capa (const char *, struct pop3d_session *);
#else
# define check_login_delay(u) 0
# define update_login_delay(u)
# define login_delay_capa NULL
#endif

/* Minimum advertise retention time for messages.  */
extern unsigned expire;
extern int expire_on_exit;

#define EXPIRE_NEVER ((unsigned)-1)

/* Size of the MD5 digest for APOP */
#define APOP_DIGEST	70

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#include <sysexits.h>

#include <mailutils/alloc.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/attribute.h>
#include <mailutils/body.h>
#include <mailutils/daemon.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/header.h>
#include <mailutils/list.h>
#include <mailutils/locker.h>
#include <mailutils/mailbox.h>
#include <mailutils/message.h>
#include <mailutils/util.h>
#include <mailutils/mu_auth.h>
#include <mailutils/nls.h>
#include <mailutils/registrar.h>
#include <mailutils/tls.h>
#include <mailutils/url.h>
#include <mailutils/md5.h>
#include <mailutils/acl.h>
#include <mailutils/server.h>
#include <mailutils/filter.h>
#include <mailutils/stdstream.h>
#include <mailutils/stream.h>
#include "mailutils/syslog.h"

/* For Berkley DB2 APOP password file */
#ifdef HAVE_DB_H
#include <db.h>
#endif

#ifdef HAVE_SECURITY_PAM_APPL_H
#include <security/pam_appl.h>
#endif

#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#define POP3_ATTRIBUTE_DELE 0x0001
#define POP3_ATTRIBUTE_RETR 0x0010

#define INITIAL         -1
#define AUTHORIZATION	0
#define TRANSACTION	1
#define UPDATE		2
#define ABORT           3

enum pop3d_error {
  OK = 0,
  ERR_WRONG_STATE,	
  ERR_BAD_ARGS,	
  ERR_BAD_LOGIN,	
  ERR_NO_MESG,	
  ERR_MESG_DELE,   
  ERR_NOT_IMPL,	
  ERR_BAD_CMD,	
  ERR_MBOX_LOCK,
  ERR_TOO_LONG,	
  ERR_NO_MEM,
  ERR_SIGNAL,
  ERR_FILE,
  ERR_NO_IFILE,
  ERR_NO_OFILE,
  ERR_IO,
  ERR_PROTO,
  ERR_TIMEOUT,
  ERR_UNKNOWN,
  ERR_MBOX_SYNC,
  ERR_TLS_IO,
  ERR_LOGIN_DELAY,
  ERR_TERMINATE,
  ERR_SYS_LOGIN
};

enum tls_mode
  {
    tls_unspecified,
    tls_no,
    tls_ondemand,
    tls_required,
    tls_connection
  };

enum pop3d_capa_type
  {
    capa_string,
    capa_func
  };

struct pop3d_capa
{
  enum pop3d_capa_type type;
  const char *name;
  union
  {
    char *string;
    void (*func) (const char *, struct pop3d_session *);
  } value;
};

struct pop3d_session
{
  mu_list_t capa;
  enum tls_mode tls_mode;
  struct mu_tls_config *tls_conf;
};

struct pop3d_srv_config
{
  struct mu_srv_config m_cfg;
  enum tls_mode tls_mode;
  struct mu_tls_config tls_conf;
};

void pop3d_session_init (struct pop3d_session *session);
void pop3d_session_free (struct pop3d_session *session);


typedef struct mu_pop_server *mu_pop_server_t;
typedef int (*pop3d_command_handler_t) (char *, struct pop3d_session *sess);

struct pop3d_command
{
  const char *name;
  pop3d_command_handler_t handler;
};

extern mu_stream_t iostream;
extern mu_pop_server_t pop3srv;
extern mu_mailbox_t mbox;
extern int state;
extern int initial_state;
extern char *username;
extern char *maildir;
extern char *md5shared;
extern size_t children;
extern struct daemon_param daemon_param;
extern int debug_mode;

extern int undelete_on_startup;
extern struct mu_auth_data *auth_data;
extern unsigned int idle_timeout;
extern int pop3d_transcript;
extern size_t pop3d_output_bufsize;
extern int pop3d_xlines;
extern char *apop_database_name;
extern int apop_database_safety;
extern uid_t apop_database_owner;
extern int apop_database_owner_set;

extern struct mu_tls_config global_tls_conf;

/* Safety checks for group-rw database files, such as stat and bulletin
   databases */
   
#define DEFAULT_GROUP_DB_SAFETY					\
  (MU_FILE_SAFETY_WORLD_WRITABLE|				\
   MU_FILE_SAFETY_WORLD_READABLE|				\
   MU_FILE_SAFETY_LINKED_WRDIR|					\
   MU_FILE_SAFETY_DIR_IWGRP|					\
   MU_FILE_SAFETY_DIR_IWOTH)


extern pop3d_command_handler_t pop3d_find_command (const char *name);

extern int pop3d_stat           (char *, struct pop3d_session *);
extern int pop3d_top            (char *, struct pop3d_session *);
extern int pop3d_uidl           (char *, struct pop3d_session *);
extern int pop3d_user           (char *, struct pop3d_session *);
extern int pop3d_apop           (char *, struct pop3d_session *);
extern int pop3d_auth           (char *, struct pop3d_session *);
extern int pop3d_capa           (char *, struct pop3d_session *);
extern int pop3d_dele           (char *, struct pop3d_session *);
extern int pop3d_list           (char *, struct pop3d_session *);
extern int pop3d_noop           (char *, struct pop3d_session *);
extern int pop3d_quit           (char *, struct pop3d_session *);
extern int pop3d_retr           (char *, struct pop3d_session *);
extern int pop3d_rset           (char *, struct pop3d_session *);

void pop3d_send_payload (mu_stream_t stream, mu_stream_t linestr,
			 size_t maxlines);

extern void pop3d_bye           (void);
extern int pop3d_abquit         (int);
extern char *pop3d_apopuser     (const char *);
extern void process_cleanup     (void);

extern void pop3d_parse_command (char *cmd, char **pcmd, char **parg);

extern RETSIGTYPE pop3d_master_signal  (int);
extern RETSIGTYPE pop3d_child_signal  (int);

extern int pop3d_stls           (char *, struct pop3d_session *);
int stls_preflight (mu_m_server_t msrv);

extern void pop3d_outf          (const char *fmt, ...) MU_PRINTFLIKE(1,2);

extern void pop3d_setio         (int, int, struct mu_tls_config *);
extern char *pop3d_readline     (char *, size_t);
extern void pop3d_flush_output  (void);

extern int pop3d_is_master      (void);

extern void pop3d_mark_deleted (mu_attribute_t attr);
extern int pop3d_is_deleted (mu_attribute_t attr);
extern void pop3d_unset_deleted (mu_attribute_t attr);
void pop3d_undelete_all (void);

extern int pop3d_init_tls_server    (struct mu_tls_config *tls_conf);

extern void pop3d_mark_retr (mu_attribute_t attr);
extern int pop3d_is_retr (mu_attribute_t attr);
extern void pop3d_unmark_retr (mu_attribute_t attr);

extern void expire_mark_message (mu_message_t msg, char **value);

extern void deliver_pending_bulletins (void);
extern void set_bulletin_db (const char *file);
extern int set_bulletin_source (const char *source);
extern int pop3d_begin_session (void);
extern const char *pop3d_error_string (int code);

extern int set_xscript_level (int xlev);

#endif /* _POP3D_H */
