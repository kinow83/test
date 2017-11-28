/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2002, 2005-2007, 2009-2012, 2014-2017 Free
   Software Foundation, Inc.

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

#ifndef _MAIL_H
#define _MAIL_H 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DMALLOC
# include <dmalloc.h>
#endif

#include <errno.h>
#include <limits.h>
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#include <sys/wait.h>
#include <sys/types.h>
#include <stdarg.h>
#include <signal.h>

#include <confpaths.h>

#ifdef HAVE_READLINE_READLINE_H
# include <readline/readline.h>
# include <readline/history.h>
#endif

#include <mailutils/address.h>
#include <mailutils/alloc.h>
#include <mailutils/assoc.h>
#include <mailutils/attribute.h>
#include <mailutils/body.h>
#include <mailutils/debug.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/envelope.h>
#include <mailutils/filter.h>
#include <mailutils/header.h>
#include <mailutils/iterator.h>
#include <mailutils/list.h>
#include <mailutils/mailbox.h>
#include <mailutils/mailer.h>
#include <mailutils/message.h>
#include <mailutils/util.h>
#include <mailutils/datetime.h>
#include <mailutils/registrar.h>
#include <mailutils/stream.h>
#include <mailutils/url.h>
#include <mailutils/nls.h>
#include <mailutils/tls.h>
#include <mailutils/argcv.h>
#include <mailutils/wordsplit.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/io.h>
#include <mailutils/stdstream.h>
#include <mailutils/prog.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions */
#ifndef function_t
typedef int function_t (int, char **);
#endif

/* Values for mail_command_entry.flags */
#define EF_REG    0x00    /* Regular command */
#define EF_FLOW   0x01    /* Flow control command */
#define EF_SEND   0x02    /* Send command */
#define EF_HIDDEN 0x04    /* Hiddent command */
  
typedef struct compose_env
{
  mu_header_t header;   /* The message headers */
  mu_stream_t compstr;  /* Temporary compose stream */
  char **outfiles;      /* Names of the output files. The message is to be
		           saved in each of these. */
  int nfiles;           /* Number of output files */
  int alt;              /* Use multipart/alternative type */
  mu_list_t attlist;    /* Attachments */
  mu_mime_t mime;       /* Associated MIME object */
} compose_env_t;

#define MAIL_COMMAND_COMMON_MEMBERS \
  const char *shortname;\
  const char *longname;\
  const char *synopsis
  
struct mail_command
{
  MAIL_COMMAND_COMMON_MEMBERS;
};

struct mail_command_entry
{
  MAIL_COMMAND_COMMON_MEMBERS;
  int flags;
  int (*func) (int, char **);
  char **(*command_completion) (int argc, char **argv, int ws);
};

struct mail_escape_entry
{
  const char *shortname;
  const char *longname;
  const char *synopsis;
  int (*escfunc) (int, char **, compose_env_t *);
};

enum mailvar_type
  {
    mailvar_type_whatever,
    mailvar_type_number,
    mailvar_type_string,
    mailvar_type_boolean
  };

union mailvar_value
{
  char *string;
  int number;
  int bool;
};

struct mailvar_variable
{
  char *name;
  enum mailvar_type type;
  int set;
  union mailvar_value value;
};

typedef struct message_set msgset_t;

struct message_set
{
  msgset_t *next;       /* Link to the next message set */
  unsigned int npart;   /* Number of parts in this set */
  size_t *msg_part;     /* Array of part numbers: msg_part[0] is the 
                           message number */
};

typedef int (*msg_handler_t) (msgset_t *mp, mu_message_t mesg, void *data);

/* Global variables and constants*/
extern mu_mailbox_t mbox;
extern size_t total;
extern int interactive;
extern const char *program_version;
extern char *default_encoding;
extern char *default_content_type;
extern int skip_empty_attachments;
extern int multipart_alternative;

/* Functions */
int mail_alias (int argc, char **argv);
int mail_alt (int argc, char **argv);	/* command alternates */
int mail_cd (int argc, char **argv);
int mail_copy (int argc, char **argv);
int mail_decode (int argc, char **argv);
int mail_delete (int argc, char **argv);
int mail_discard (int argc, char **argv);
int mail_dp (int argc, char **argv);
int mail_echo (int argc, char **argv);
int mail_edit (int argc, char **argv);
int mail_else (int argc, char **argv);
int mail_endif (int argc, char **argv);
int mail_exit (int argc, char **argv);
int mail_file (int argc, char **argv);
int mail_folders (int argc, char **argv);
int mail_followup (int argc, char **argv);
int mail_from (int argc, char **argv);
int mail_from0 (msgset_t *mspec, mu_message_t msg, void *data);
void mail_compile_headline (char const *str);

int mail_headers (int argc, char **argv);
int mail_hold (int argc, char **argv);
int mail_help (int argc, char **argv);
int mail_if (int argc, char **argv);
int mail_inc (int argc, char **argv);
int mail_list (int argc, char **argv);
int mail_send (int argc, char **argv);	/* command mail */
int mail_mbox (int argc, char **argv);
int mail_next (int argc, char **argv);
int mail_nounfold (int argc, char **argv);
int mail_variable (int argc, char **argv);
int mail_pipe (int argc, char **argv);
int mail_previous (int argc, char **argv);
int mail_print (int argc, char **argv);
int mail_quit (int argc, char **argv);
int mail_reply (int argc, char **argv);
int mail_retain (int argc, char **argv);
int mail_save (int argc, char **argv);
int mail_sendheader (int argc, char **argv);
int mail_set (int argc, char **argv);
int mail_shell (int argc, char **argv);
int mail_execute (int shell, char *progname, int argc, char **argv);
int mail_size (int argc, char **argv);
int mail_source (int argc, char **argv);
int mail_summary (int argc, char **argv);
int mail_tag (int argc, char **argv);
int mail_top (int argc, char **argv);
int mail_touch (int argc, char **argv);
int mail_unalias (int argc, char **argv);
int mail_undelete (int argc, char **argv);
int mail_unfold (int argc, char **argv);
int mail_unset (int argc, char **argv);
int mail_version (int argc, char **argv);
int mail_visual (int argc, char **argv);
int mail_warranty (int argc, char **argv);
int mail_write (int argc, char **argv);
int mail_z (int argc, char **argv);
int mail_eq (int argc, char **argv);	/* command = */
int mail_setenv (int argc, char **argv);
int mail_envelope (int argc, char **argv);
int print_envelope (msgset_t *mspec, mu_message_t msg, void *data);
int mail_struct (int argc, char **argv);

int if_cond (void);

void mail_mainloop (char *(*input) (void *, int), void *closure, int do_history);
int mail_copy0 (int argc, char **argv, int mark);
int mail_send0 (compose_env_t *env, int save_to);
void free_env_headers (compose_env_t *env);

/*extern void print_message (mu_message_t mesg, char *prefix, int all_headers, FILE *file);*/

int mail_mbox_commit (void);
int mail_is_my_name (const char *name);
void mail_set_my_name (char *name);
char *mail_whoami (void);
int mail_header_is_visible (const char *str);
int mail_header_is_unfoldable (const char *str);
int mail_mbox_close (void);
char *mail_expand_name (const char *name);

void send_append_header (char const *text);
void send_append_header2 (char const *name, char const *value, int mode);
int send_attach_file (int fd,
		      const char *filename,
		      const char *content_filename,
		      const char *content_name,
		      const char *content_type, const char *encoding);

int escape_check_args (int argc, char **argv, int minargs, int maxargs);

int escape_shell (int argc, char **argv, compose_env_t *env);
int escape_command (int argc, char **argv, compose_env_t *env);
int escape_help (int argc, char **argv, compose_env_t *env);
int escape_sign (int argc, char **argv, compose_env_t *env);
int escape_bcc (int argc, char **argv, compose_env_t *env);
int escape_cc (int argc, char **argv, compose_env_t *env);
int escape_deadletter (int argc, char **argv, compose_env_t *env);
int escape_editor (int argc, char **argv, compose_env_t *env);
int escape_print (int argc, char **argv, compose_env_t *env);
int escape_headers (int argc, char **argv, compose_env_t *env);
int escape_insert (int argc, char **argv, compose_env_t *env);
int escape_quote (int argc, char **argv, compose_env_t *env);
int escape_type_input (int argc, char **argv, compose_env_t *env);
int escape_read (int argc, char **argv, compose_env_t *env);
int escape_subj (int argc, char **argv, compose_env_t *env);
int escape_to (int argc, char **argv, compose_env_t *env);
int escape_visual (int argc, char **argv, compose_env_t *env);
int escape_write (int argc, char **argv, compose_env_t *env);
int escape_exit (int argc, char **argv, compose_env_t *env);
int escape_pipe (int argc, char **argv, compose_env_t *env);
int escape_list_attachments  (int argc, char **argv, compose_env_t *env);
int escape_attach (int argc, char **argv, compose_env_t *env);
int escape_remove_attachment (int argc, char **argv, compose_env_t *env);
int escape_toggle_multipart_type (int argc, char **argv, compose_env_t *env);

enum
  {
    parse_headers_ok,
    parse_headers_error,
    parse_headers_fatal
  };

int parse_headers (mu_stream_t input, compose_env_t *env);

/* Cursor */
void set_cursor (unsigned value);
size_t get_cursor (void);
void page_invalidate (int hard);
void cond_page_invalidate (size_t value);
void page_do (msg_handler_t func, void *data);
size_t page_move (off_t offset);
int is_current_message (size_t n);

/* msgsets */
void msgset_free (msgset_t *msg_set);
msgset_t *msgset_make_1 (size_t number);
msgset_t *msgset_append (msgset_t *one, msgset_t *two);
msgset_t *msgset_range (int low, int high);
msgset_t *msgset_expand (msgset_t *set, msgset_t *expand_by);
msgset_t *msgset_dup (const msgset_t *set);
int msgset_parse (const int argc, char **argv, int flags, msgset_t **mset);
int msgset_member (msgset_t *set, size_t n);
msgset_t *msgset_negate (msgset_t *set);
size_t msgset_count (msgset_t *set);


#define MDHINT_SELECTED_HEADERS 0x1

struct mime_descend_closure
{
  int hints;
  const msgset_t *msgset;
  mu_message_t message;
  const char *type;
  const char *encoding;
  const struct mime_descend_closure *parent;
};

typedef int (*mime_descend_fn) (struct mime_descend_closure *closure,
				void *data);

int mime_descend (struct mime_descend_closure *closure,
		  mime_descend_fn fun, void *data);



int util_do_command (const char *cmd, ...) MU_PRINTFLIKE(1,2);

int util_foreach_msg (int argc, char **argv, int flags,
		      msg_handler_t func, void *data);
size_t util_range_msg (size_t low, size_t high, int flags, 
		       msg_handler_t func, void *data);

function_t *util_command_get (const char *cmd);

void *util_find_entry (void *table, size_t nmemb, size_t size,
		       const char *cmd);
int util_help (void *table, size_t nmemb, size_t size, const char *word);
int util_command_list (void *table, size_t nmemb, size_t size);

const struct mail_command_entry *mail_find_command (const char *cmd);
const struct mail_escape_entry *mail_find_escape (const char *cmd);
int mail_command_help (const char *command);
int mail_escape_help (const char *command);
void mail_command_list (void);
const struct mail_command *mail_command_name (int i);

int util_getcols (void);
int util_getlines (void);
int util_screen_lines (void);
int util_screen_columns (void);
int util_get_crt (void);
struct mailvar_variable *mailvar_find_variable (const char *var, int create);
int mailvar_get (void *ptr, const char *variable,
		 enum mailvar_type type, int warn);
int mailvar_is_true (char const *name);

void mailvar_print (int set);
void mailvar_variable_format (mu_stream_t,
			      const struct mailvar_variable *,
			      const char *defval);

#define MOPTF_OVERWRITE 0x001
#define MOPTF_QUIET     0x002
#define MOPTF_UNSET     0x004
int mailvar_set (const char *name, void *value,
		 enum mailvar_type type, int flags);
int util_isdeleted (size_t msgno);
char *util_get_homedir (void);
char *util_fullpath (const char *inpath);
char *util_folder_path (const char *name);
char *util_get_sender (int msgno, int strip);

void util_slist_print (mu_list_t list, int nl);
int util_slist_lookup (mu_list_t list, const char *str);
void util_slist_add (mu_list_t *list, char *value);
void util_slist_remove (mu_list_t *list, char *value);
void util_slist_destroy (mu_list_t *list);
char *util_slist_to_string (mu_list_t list, const char *delim);
void util_strcat (char **dest, const char *str);
char *util_outfolder_name (char *str);
void util_save_outgoing (mu_message_t msg, char *savefile);
int util_error_range (size_t msgno);
void util_noapp (void);
int util_tempfile (char **namep);
void util_msgset_iterate (msgset_t *msgset, 
			  int (*fun) (mu_message_t, msgset_t *, void *), 
			  void *closure);
int util_get_content_type (mu_header_t hdr, char **value, char **args);
int util_get_hdr_value (mu_header_t hdr, const char *name, char **value);
int util_merge_addresses (char **addr_str, const char *value);
int util_header_expand (mu_header_t *hdr);
int util_get_message (mu_mailbox_t mbox, size_t msgno, mu_message_t *msg);
void util_cache_command (mu_list_t *list, const char *fmt, ...)
  MU_PRINTFLIKE(2,3);
void util_run_cached_commands (mu_list_t *list);
const char *util_reply_prefix (void);
void util_rfc2047_decode (char **value);
char *util_get_charset (void);

void util_mark_read (mu_message_t msg);

const char *util_url_to_string (mu_url_t url);

mu_stream_t open_pager (size_t lines);

void format_msgset (mu_stream_t str, const msgset_t *msgset, size_t *count);

int is_address_field (const char *name);

int ml_got_interrupt (void);
void ml_clear_interrupt (void);
void ml_readline_init (void);
int ml_reread (const char *prompt, char **text);
char *ml_readline (const char *prompt);
char *ml_readline_with_intr (const char *prompt);

char *alias_expand (const char *name);
void alias_destroy (const char *name);

typedef struct alias_iterator *alias_iterator_t;
char *alias_find_first (const char *prefix, alias_iterator_t *itr);
const char *alias_iterate_next (alias_iterator_t itr);
const char *alias_iterate_first (const char *p, alias_iterator_t *itr);
void alias_iterate_end (alias_iterator_t *itr);

int mail_sender    (int argc, char **argv);
int mail_nosender  (int argc, char **argv);
mu_address_t get_sender_address (mu_message_t msg);

#define COMPOSE_APPEND      0
#define COMPOSE_REPLACE     1
#define COMPOSE_SINGLE_LINE 2

void compose_init (compose_env_t *env);
int compose_header_set (compose_env_t *env, const char *name,
		        const char *value, int replace);
char *compose_header_get (compose_env_t *env, char *name, char *defval);
void compose_destroy (compose_env_t *env);

#ifndef HAVE_READLINE_READLINE_H
char *readline (char *prompt);
#endif

/* Flags for util_get_message */
#define MSG_ALL       0
#define MSG_NODELETED 0x0001
#define MSG_SILENT    0x0002
#define MSG_COUNT     0x0004

/* Message attributes */
#define MAIL_ATTRIBUTE_MBOXED   0x0001
#define MAIL_ATTRIBUTE_PRESERVED 0x0002
#define MAIL_ATTRIBUTE_SAVED    0x0004
#define MAIL_ATTRIBUTE_TAGGED   0x0008
#define MAIL_ATTRIBUTE_SHOWN    0x0010
#define MAIL_ATTRIBUTE_TOUCHED  0x0020

void ml_attempted_completion_over (void);

#ifdef WITH_READLINE
char **file_compl (int argc, char **argv, int ws);
char **no_compl (int argc, char **argv, int ws);
char **msglist_compl (int argc, char **argv, int ws);
char **msglist_file_compl (int argc, char **argv, int ws);
char **dir_compl (int argc, char **argv, int ws);
char **command_compl (int argc, char **argv, int ws);
char **alias_compl (int argc, char **argv, int ws);
char **mailvar_set_compl (int argc, char **argv, int ws);
char **exec_compl (int argc, char **argv, int ws);
#else
# define file_compl NULL
# define no_compl NULL
# define msglist_compl NULL
# define msglist_file_compl NULL
# define dir_compl NULL
# define command_compl NULL
# define alias_compl NULL
# define var_compl NULL
# define exec_compl NULL     
# define mailvar_set_compl NULL
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MAIL_H */
