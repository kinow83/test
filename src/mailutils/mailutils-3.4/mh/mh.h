/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2002, 2004-2007, 2009-2012, 2014-2017 Free
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <mh_getopt.h>
#include <mailutils/io.h>

#include <string.h>

#include <mailutils/alloc.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/parse822.h>
#include <mailutils/mailbox.h>
#include <mailutils/message.h>
#include <mailutils/header.h>
#include <mailutils/body.h>
#include <mailutils/registrar.h>
#include <mailutils/list.h>
#include <mailutils/iterator.h>
#include <mailutils/address.h>
#include <mailutils/util.h>
#include <mailutils/stream.h>
#include <mailutils/filter.h>
#include <mailutils/url.h>
#include <mailutils/attribute.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/nls.h>
#include <mailutils/argcv.h>
#include <mailutils/wordsplit.h>
#include <mailutils/debug.h>
#include <mailutils/mailer.h>
#include <mailutils/envelope.h>
#include <mailutils/mime.h>
#include <mailutils/io.h>
#include <mailutils/property.h>
#include <mailutils/prog.h>
#include <mailutils/opool.h>
#include <mailutils/mh.h>
#include <mailutils/stdstream.h>
#include <mailutils/datetime.h>
#include <mailutils/msgset.h>

#include <mu_umaxtostr.h>

#define MH_SEQUENCES_FILE ".mh_sequences"
#define MH_USER_PROFILE ".mh_profile"
#define MH_GLOBAL_PROFILE "mh-profile"
#define MH_CONTEXT_FILE "context"
#define DEFAULT_ALIAS_FILE MHLIBDIR "/MailAliases"

#define is_true(arg) ((arg) == NULL||mu_true_answer_p (arg) == 1)

typedef struct
{
  const char *name;
  mu_header_t header;
} mh_context_t;

/* Recipient masks */
#define RCPT_NONE 0
#define RCPT_TO   0x0001
#define RCPT_CC   0x0002
#define RCPT_ME   0x0004
#define RCPT_ALL  (RCPT_TO|RCPT_CC|RCPT_ME)

#define RCPT_DEFAULT RCPT_NONE

struct mh_whatnow_env     /* whatnow shell environment */
{  
  char *file;             /* The file being processed */
  char *msg;              /* File name of the original message (if any) */
  char *draftfile;        /* File to preserve the draft into */
  const char *editor;     /* Default editor */
  char *prompt;
  char *anno_field;       /* Annotate field to be used */
  mu_list_t anno_list;    /* List of messages (mu_message_t) to annotate */
  mu_mailbox_t mbox;
  int nowhatnowproc;
  int reedit:1;           /* Set if the editor was already invoked */
  char *last_ed;          /* Last used editor */
};

#define DISP_QUIT 0
#define DISP_USE 1
#define DISP_REPLACE 2

#define SEQ_PRIVATE 1
#define SEQ_ZERO    2

extern int rcpt_mask;
extern int mh_mailbox_cur_default;

void mh_init (void);
void mh_init2 (void);
void mh_read_profile (void);
int mh_read_formfile (char const *name, char **pformat);
mu_message_t mh_file_to_message (const char *folder, const char *file_name);
mu_message_t mh_stream_to_message (mu_stream_t stream);
void mh_install (char *name, int automode);

mu_property_t mh_read_property_file (char *name, int ro);
void mh_property_merge (mu_property_t dst, mu_property_t src);

int mh_width (void);

#define mh_global_profile_get(name, defval) \
  mu_mhprop_get_value (mu_mh_profile, name, defval)
#define mh_global_profile_set(name, value) \
  mu_property_set_value (mu_mh_profile, name, value, 1))
#define mh_global_profile_iterate(fp, data) \
  mu_mhprop_iterate (mu_mh_profile, fp, data)

#define mh_global_context_get(name, defval) \
  mu_mhprop_get_value (mu_mh_context, name, defval)
#define mh_global_context_set(name, value) \
  mu_property_set_value (mu_mh_context, name, value, 1)
#define mh_global_context_iterate(fp, data) \
  mu_mhprop_iterate (mu_mh_context, fp, data)

const char *mh_set_current_folder (const char *val);
const char *mh_current_folder (void);

mu_property_t mh_mailbox_get_property (mu_mailbox_t mbox);
const char *mh_global_sequences_get (mu_mailbox_t mbox,
				     const char *name, const char *defval);
void mh_global_sequences_set (mu_mailbox_t mbox,
			      const char *name, const char *value);
void mh_global_sequences_iterate (mu_mailbox_t mbox,
				  mu_mhprop_iterator_t fp, void *data);
void mh_global_sequences_drop (mu_mailbox_t mbox);

void mh_global_save_state (void);

int mh_private_sequences_iterate (mu_mailbox_t mbox, mu_mhprop_iterator_t fp,
				  void *data);

int mh_interactive_mode_p (void);
int mh_getyn (const char *fmt, ...) MU_PRINTFLIKE(1,2);
int mh_getyn_interactive (const char *fmt, ...) MU_PRINTFLIKE(1,2);
int mu_vgetans (const char *variants, const char *fmt, va_list ap);
int mu_getans (const char *variants, const char *fmt, ...)
  MU_PRINTFLIKE(2,3);
int mh_check_folder (const char *pathname, int confirm);
int mh_makedir (const char *p);

typedef struct mh_format *mh_format_t;
typedef struct mh_fvm *mh_fvm_t;

#define MH_FMT_FORCENL 1
void mh_fvm_create (mh_fvm_t *fvm, int flags);
void mh_fvm_destroy (mh_fvm_t *fvm);

void mh_fvm_set_output (mh_fvm_t fvm, mu_stream_t str);
void mh_fvm_set_width (mh_fvm_t fvm, size_t width);
void mh_fvm_set_format (mh_fvm_t fvm, mh_format_t fmt);

void mh_fvm_run (mh_fvm_t fvm, mu_message_t msg);

int mh_format_str (mh_format_t fmt, char *str, size_t width, char **pret);

void mh_format_dump_code (mh_format_t fmt);
void mh_format_dump_disass (mh_format_t fmt, int addr);

#define MH_FMT_PARSE_DEFAULT 0
#define MH_FMT_PARSE_TREE 0x01
#define MH_FMT_PARSE_DEBUG 0x02
int mh_format_string_parse (mh_format_t *retfmt, char const *format_str,
			    struct mu_locus_point const *locus,
			    int flags);
int mh_format_file_parse (mh_format_t *retfmt, char const *formfile,
			  int flags);
mh_format_t mh_scan_format (void);


void mh_format_free (mh_format_t fmt);
void mh_format_destroy (mh_format_t *fmt);

void mh_error (const char *fmt, ...) MU_PRINTFLIKE(1,2);
void mh_err_memory (int fatal);

mu_stream_t mh_audit_open (char *name, mu_mailbox_t mbox);
void mh_audit_close (mu_stream_t);

int mh_message_number (mu_message_t msg, size_t *pnum);

mu_mailbox_t mh_open_folder (const char *folder, int flags);

void mh_msgset_parse (mu_msgset_t *msgset, mu_mailbox_t mbox, 
		      int argc, char **argv, const char *def);
void mh_msgset_parse_string (mu_msgset_t *msgset, mu_mailbox_t mbox, 
			     const char *string, const char *def);
#define RET_MSGNO 0
#define RET_UID   1
size_t mh_msgset_first (mu_msgset_t msgset, int uid);
size_t mh_msgset_last  (mu_msgset_t msgset, int uid);
int mh_msgset_single_message (mu_msgset_t msgset);

#define NAME_ANY    0
#define NAME_FOLDER 1
#define NAME_FILE   2
char *mh_expand_name (const char *base, const char *name, int what);

char *mh_get_dir (void);
int mh_find_file (const char *name, char **resolved_name);
void mh_quote (const char *in, char **out);
void mh_expand_aliases (mu_message_t msg, mu_address_t *addr_to,
			mu_address_t *addr_cc,
			mu_address_t *addr_bcc);

int mh_is_my_name (const char *name);
char const *mh_get_my_real_name (void);
char const *mh_get_my_user_name (void);
char const *mh_my_email (void);
char const *mh_my_host (void);


size_t mh_get_message (mu_mailbox_t mbox, size_t seqno, mu_message_t *mesg);

int mh_decode_rcpt_flag (const char *arg);

int mh_draft_message (const char *name, const char *msgspec, char **pname);
     
int mh_spawnp (const char *prog, const char *file);
int mh_whatnow (struct mh_whatnow_env *wh, int initial_edit);
int mh_whatnowproc (struct mh_whatnow_env *wh, int initial_edit,
		    const char *prog);

int mh_disposition (const char *filename);
int mh_usedraft (const char *filename);
int mh_file_copy (const char *from, const char *to);
char *mh_draft_name (void);
char *mh_create_message_id (int);

int mh_whom_header (mu_header_t hdr);
int mh_whom_file (const char *filename, int check);
int mh_whom_message (mu_message_t msg, int check);

void mh_set_reply_regex (const char *str);
int mh_decode_2047 (char const *text, char **decoded_text);
const char *mh_charset (const char *);

int mh_alias_read (char const *name, int fail);
int mh_alias_get (const char *name, mu_list_t *return_list);
int mh_alias_get_address (const char *name, mu_address_t *addr, int *incl);
int mh_alias_get_alias (const char *uname, mu_list_t *return_list);
int mh_read_aliases (void);
int mh_alias_expand (const char *str, mu_address_t *paddr, int *incl);

typedef int (*mh_alias_enumerator_t) (char *alias, mu_list_t names, void *data);
void mh_alias_enumerate (mh_alias_enumerator_t fun, void *data);


void mh_annotate (mu_message_t msg, const char *field, const char *text,
		  int date);

#define MHL_DECODE       1
#define MHL_CLEARSCREEN  2
#define MHL_BELL         4
#define MHL_DISABLE_BODY 8

mu_list_t mhl_format_compile (char *name);

int mhl_format_run (mu_list_t fmt, int width, int length, int flags,
		    mu_message_t msg, mu_stream_t output);
void mhl_format_destroy (mu_list_t *fmt);

void mh_seq_add (mu_mailbox_t mbox, const char *name, mu_msgset_t mset,
		 int flags);
int mh_seq_delete (mu_mailbox_t mbox, const char *name, mu_msgset_t mset,
		   int flags);
const char *mh_seq_read (mu_mailbox_t mbox, const char *name, int flags);

void mh_sequences_elim (mu_msgset_t msgset);

void mh_comp_draft (const char *formfile, const char *draftfile);
int check_draft_disposition (struct mh_whatnow_env *wh, int use_draft);

void ali_verbatim (int enable);

char *mh_safe_make_file_name (const char *dir, const char *file);
  
void mh_mailbox_get_cur (mu_mailbox_t mbox, size_t *pcur);
void mh_mailbox_set_cur (mu_mailbox_t mbox, size_t cur);

void mh_whatnow_env_from_environ_early (struct mh_whatnow_env *wh);
void mh_whatnow_env_from_environ_late (struct mh_whatnow_env *wh);
void mh_whatnow_env_to_environ (struct mh_whatnow_env *wh);
