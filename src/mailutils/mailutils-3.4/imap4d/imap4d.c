/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999, 2001-2012, 2014-2017 Free Software Foundation,
   Inc.

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

#include "imap4d.h"
#include <mailutils/gsasl.h>
#include <mailutils/cli.h>
#include <mailutils/kwd.h>
#include "tcpwrap.h"

mu_m_server_t server;
unsigned int idle_timeout = 1800;
int imap4d_transcript;

mu_mailbox_t mbox;              /* Current mailbox */
char *real_homedir;             /* Homedir as returned by user database */
int state = STATE_NONAUTH;      /* Current IMAP4 state */
struct mu_auth_data *auth_data; 

int login_disabled;             /* Disable LOGIN command */
int create_home_dir;            /* Create home directory if it does not
				   exist */
mu_list_t user_retain_groups;

int home_dir_mode = S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;

/* Saved command line. */
int imap4d_argc;                 
char **imap4d_argv;

enum imap4d_preauth preauth_mode;
char *preauth_program;
int preauth_only;
int ident_port;
char *ident_keyfile;
int ident_encrypt_only;

int test_mode;

const char *program_version = "imap4d (" PACKAGE_STRING ")";


static void
set_foreground (struct mu_parseopt *po, struct mu_option *opt,
		char const *arg)
{
  mu_m_server_set_foreground (server, 1);
}

static void
set_inetd_mode (struct mu_parseopt *po, struct mu_option *opt,
		char const *arg)
{
  mu_m_server_set_mode (server, MODE_INTERACTIVE);
}
  
static void
set_daemon_mode (struct mu_parseopt *po, struct mu_option *opt,
		 char const *arg)
{
  mu_m_server_set_mode (server, MODE_DAEMON);
  if (arg)
    {
      size_t max_children;
      char *errmsg;
      int rc = mu_str_to_c (arg, mu_c_size, &max_children, &errmsg);
      if (rc)
	{
	  mu_parseopt_error (po, _("%s: bad argument"), arg);
	  exit (po->po_exit_error);
	}
      mu_m_server_set_max_children (server, max_children);
    }
}

static void
set_preauth (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  preauth_mode = preauth_stdio;
}

static struct mu_option imap4d_options[] = {
  { "foreground",  0, NULL, MU_OPTION_DEFAULT,
    N_("remain in foreground"),
    mu_c_bool, NULL, set_foreground },
  { "inetd",  'i', NULL, MU_OPTION_DEFAULT,
    N_("run in inetd mode"),
    mu_c_bool, NULL, set_inetd_mode },
  { "daemon", 'd', N_("NUMBER"), MU_OPTION_ARG_OPTIONAL,
    N_("runs in daemon mode with a maximum of NUMBER children"),
    mu_c_string, NULL, set_daemon_mode },

  { "test", 0, NULL, MU_OPTION_DEFAULT,
    N_("run in test mode"),
    mu_c_bool, &test_mode },

  { "preauth", 0, NULL, MU_OPTION_DEFAULT,
    N_("start in preauth mode"),
    mu_c_string, NULL, set_preauth },

  MU_OPTION_END
}, *options[] = { imap4d_options, NULL };


static char *capa[] = {
  "auth",
  "debug",
  "mailbox",
  "locking",
  "logging",
  NULL
};

static int
cb_mode (void *data, mu_config_value_t *val)
{
  char *p;
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  home_dir_mode = strtoul (val->v.string, &p, 8);
  if (p[0] || (home_dir_mode & ~0777))
    mu_error (_("invalid mode specification: %s"), val->v.string);
  return 0;
}

int
parse_preauth_scheme (const char *scheme, mu_url_t url)
{
  int rc = 0;
  if (strcmp (scheme, "stdio") == 0)
    preauth_mode = preauth_stdio;
  else if (strcmp (scheme, "prog") == 0)
    {
      char *path;
      rc = mu_url_aget_path (url, &path);
      if (rc)
	{
	  mu_error (_("URL error: cannot get path: %s"), mu_strerror (rc));
	  return 1;
	}
      preauth_program = path;
      preauth_mode = preauth_prog;
    }
  else if (strcmp (scheme, "ident") == 0)
    {
      struct servent *sp;
      unsigned n;
      if (url && mu_url_get_port (url, &n) == 0)
	ident_port = (short) n;
      else if ((sp = getservbyname ("auth", "tcp")))
	ident_port = ntohs (sp->s_port);
      else
	ident_port = 113;
      preauth_mode = preauth_ident;
    }
  else
    {
      mu_error (_("unknown preauth scheme"));
      rc = 1;
    }

  return rc;
}
      
/* preauth prog:///usr/sbin/progname
   preauth ident[://:port]
   preauth stdio
*/
static int
cb_preauth (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  if (strcmp (val->v.string, "stdio") == 0)
    preauth_mode = preauth_stdio;
  else if (strcmp (val->v.string, "ident") == 0)
    return parse_preauth_scheme (val->v.string, NULL);
  else if (val->v.string[0] == '/')
    {
      preauth_program = mu_strdup (val->v.string);
      preauth_mode = preauth_prog;
    }
  else
    {
      mu_url_t url;
      char *scheme;
      int rc = mu_url_create (&url, val->v.string);

      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_url_create", val->v.string, rc);
	  return 1;
	}

      rc = mu_url_aget_scheme (url, &scheme);
      if (rc)
	{
	  mu_url_destroy (&url);
	  mu_error (_("URL error: %s"), mu_strerror (rc));
	  return 1;
	}

      rc = parse_preauth_scheme (scheme, url);
      mu_url_destroy (&url);
      free (scheme);
      return rc;
    }
  return 0;
}

static int
cb_mailbox_mode (void *data, mu_config_value_t *val)
{
  const char *p;
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  if (mu_parse_stream_perm_string ((int *)data, val->v.string, &p))
    mu_error (_("invalid mode string near %s"), p);
  return 0;
}

static int
cb2_group (const char *gname, void *data)
{
  mu_list_t list = data;
  struct group *group;
  
  group = getgrnam (gname);
  if (!group)
    mu_error (_("unknown group: %s"), gname);
  else
    mu_list_append (list, (void*) (intptr_t) group->gr_gid);
  return 0;
}
  
static int
cb_group (void *data, mu_config_value_t *arg)
{
  mu_list_t *plist = data;

  if (!*plist)
    mu_list_create (plist);
  return mu_cfg_string_value_cb (arg, cb2_group, *plist);
}


static int
cb_tls (void *data, mu_config_value_t *val)
{
  int *res = data;
  static struct mu_kwd tls_kwd[] = {
    { "no", tls_no },
    { "false", tls_no },
    { "off", tls_no },
    { "0", tls_no },
    { "ondemand", tls_ondemand },
    { "stls", tls_ondemand },
    { "required", tls_required },
    { "connection", tls_connection },
    /* For compatibility with prior versions: */
    { "yes", tls_connection }, 
    { "true", tls_connection },
    { "on", tls_connection },
    { "1", tls_connection },
    { NULL }
  };
  
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;

  if (mu_kwd_xlat_name (tls_kwd, val->v.string, res))
    mu_error (_("not a valid tls keyword: %s"), val->v.string);
  return 0;
}

static mu_list_t auth_deny_user_list, auth_allow_user_list;
static mu_list_t auth_deny_group_list, auth_allow_group_list;

static int
check_user_groups (void *item, void *data)
{
  char *gname = item;
  struct group *gp;
  char **p;
  
  gp = getgrnam (gname);
  if (!gp)
    return 0;

  if (gp->gr_gid == auth_data->gid)
    return MU_ERR_USER0;

  for (p = gp->gr_mem; *p; p++)
    if (strcmp (*p, auth_data->name) == 0)
      return MU_ERR_USER0;

  return 0;
}

static int
imap_check_group_list (mu_list_t l)
{
  int rc = mu_list_foreach (l, check_user_groups, NULL);
  if (rc == MU_ERR_USER0) 
    return 0;
  else if (rc == 0)
    return MU_ERR_NOENT;
  return rc;
}

static int
cb_prefix_delim (void *data, mu_config_value_t *val)
{
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  if (val->v.string[0] == 0)
    mu_error (_("delimiter cannot be empty"));
  else if (val->v.string[1] != 0)
    mu_error (_("delimiter must be a single character"));
  else
    *(int*) data = val->v.string[0];
  return 0;
}

static int
cb_prefix_scheme (void *data, mu_config_value_t *val)
{
  struct namespace_prefix *pfx = data;
  char *scheme;
  mu_record_t rec;
  int rc;
  int (*mbx) (mu_mailbox_t);
  
  if (mu_cfg_assert_value_type (val, MU_CFG_STRING))
    return 1;
  scheme = mu_strdup (val->v.string);
  rc = mu_registrar_lookup_scheme (scheme, &rec);
  if (rc == MU_ERR_NOENT)
    {
      mu_error (_("unknown mailbox type"));
      return 1;
    }
  else if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_registrar_lookup_scheme", scheme, rc);
      return 1;
    }

  rc = mu_record_get_mailbox (rec, &mbx);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_record_get_mailbox", scheme, rc);
      return 1;
    }

  if (!mbx || !mu_record_is_local (rec))
    {
      mu_error (_("not a local mailbox type"));
      return 1;
    }
  
  pfx->scheme = scheme;
  pfx->record = rec;

  return 0;
}

static struct mu_cfg_param prefix_param[] = {
  { "directory", mu_c_string,
    NULL, mu_offsetof (struct namespace_prefix, dir), NULL,
    N_("Directory in the file system") },
  { "delimiter", mu_cfg_callback,
    NULL, mu_offsetof (struct namespace_prefix, delim), cb_prefix_delim,
    N_("Hierarchy delimiter character"),
    N_("arg: character") },
  { "mailbox-type", mu_cfg_callback,
    NULL, 0, cb_prefix_scheme,
    N_("Type of mailboxes residing under this prefix"),
    N_("type: string") },
  { NULL }
};

static struct mu_cfg_param namespace_param[] = {
  { "mailbox-mode", mu_cfg_callback,
    NULL, mu_offsetof (struct namespace, mode), cb_mailbox_mode,
    N_("File mode for newly created mailboxes in this namespace"),
    N_("mode: g(+|=)[wr]+,o(+|=)[wr]+") },
  { "prefix", mu_cfg_section },
  { NULL }
};

static int
prefix_section_parser (enum mu_cfg_section_stage stage,
		       const mu_cfg_node_t *node,
		       const char *section_label, void **section_data,
		       void *call_data,
		       mu_cfg_tree_t *tree)
{
  struct namespace_prefix *pfx;

  switch (stage)
    {
    case mu_cfg_section_start:
      {
	struct namespace *nspace = *section_data;
      
	if (node->label == NULL || node->label->type != MU_CFG_STRING)
	  return 1;

	pfx = mu_zalloc (sizeof (*pfx));
	pfx->prefix = mu_strdup (node->label->v.string);
	mu_list_append (nspace->prefixes, pfx);
	*section_data = pfx;
      }
      break;

    case mu_cfg_section_end:
      pfx = *section_data;
      if (!pfx->delim)
	pfx->delim = '/';
      if (!pfx->dir)
	{
	  if (pfx->prefix)
	    pfx->dir = mu_strdup (pfx->prefix);
	  else
	    {
	      mu_error (_("bad prefix definition"));
	      return 1;
	    }
	}
      else if (!pfx->prefix)
	{
	  pfx->prefix = mu_alloc (strlen (pfx->dir) + 1);
	  translate_delim (pfx->prefix, pfx->dir, pfx->delim, '/');
	}
    }
  return 0;
}
      
static int
namespace_section_parser (enum mu_cfg_section_stage stage,
			  const mu_cfg_node_t *node,
			  const char *section_label, void **section_data,
			  void *call_data,
			  mu_cfg_tree_t *tree)
{
  if (stage == mu_cfg_section_start)
    {
      struct namespace *ns;
      
      if (node->label == NULL || node->label->type != MU_CFG_STRING)
	return 1;

      ns = namespace_lookup (node->label->v.string);
      if (!ns)
	{
	  mu_error (_("unknown namespace"));
	  return 0;
	}
	
      *section_data = ns;
    }
  return 0;
}

static void
namespace_cfg_init (void)
{
  struct mu_cfg_section *section;

  if (mu_create_canned_section ("prefix", &section))
    abort ();
  section->docstring = N_("Define a single prefix");
  section->label = N_("string");
  mu_cfg_section_add_params (section, prefix_param);
  section->parser = prefix_section_parser;
  
  if (mu_create_canned_section ("namespace", &section))
    abort ();
  section->docstring = N_("Define a namespace");
  section->label = "personal | other | shared";
  section->parser = namespace_section_parser;
  mu_cfg_section_add_params (section, namespace_param);
}

static struct mu_cfg_param imap4d_srv_param[] = {
  { "tls-mode", mu_cfg_callback,
    NULL, mu_offsetof (struct imap4d_srv_config, tls_mode), cb_tls,
    N_("Kind of TLS encryption to use for this server"),
    /* TRANSLATORS: translate only arg:, the rest are keywords */
    N_("arg: false|true|ondemand|stls|requred|connection") },
  { "tls", mu_cfg_section,
    NULL, mu_offsetof (struct imap4d_srv_config, tls_conf) },
  { NULL }
};

static struct mu_cfg_param imap4d_cfg_param[] = {
  { "allow-users", MU_CFG_LIST_OF(mu_c_string), &auth_allow_user_list,
    0, NULL, 
    N_("Allow access to users from this list.") },
  { "deny-users", MU_CFG_LIST_OF(mu_c_string), &auth_deny_user_list,
    0, NULL, 
    N_("Deny access to users from this list.") },
  { "allow-groups", MU_CFG_LIST_OF(mu_c_string), &auth_allow_group_list,
    0, NULL, 
    N_("Allow access if the user group is in this list.") },
  { "deny-groups", MU_CFG_LIST_OF(mu_c_string), &auth_deny_group_list,
    0, NULL, 
    N_("Deny access if the user group is in this list.") },
  
  { "namespace", mu_cfg_section },
  
  { "login-disabled", mu_c_bool, &login_disabled, 0, NULL,
    N_("Disable LOGIN command.") },
  { "create-home-dir", mu_c_bool, &create_home_dir, 0, NULL,
    N_("If true, create non-existing user home directories.") },
  { "home-dir-mode", mu_cfg_callback, NULL, 0, cb_mode,
    N_("File mode for creating user home directories."),
    N_("mode: octal") },
  { "retain-groups", mu_cfg_callback, &user_retain_groups, 0, cb_group,
    N_("Retain these supplementary groups when switching to user privileges"),
    N_("groups: list of string") },
  { "tls", mu_cfg_section, &global_tls_conf },
  { "preauth", mu_cfg_callback, NULL, 0, cb_preauth,
    N_("Configure PREAUTH mode.  <value> is one of:\n"
       "  prog:///<full-program-name: string>\n"
       "  ident[://:<port: string-or-number>]\n"
       "  stdio"),
    N_("mode: value") },
  { "preauth-only", mu_c_bool, &preauth_only, 0, NULL,
    N_("Use only preauth mode.  If unable to setup it, disconnect "
       "immediately.") },
  { "ident-keyfile", mu_c_string, &ident_keyfile, 0, NULL,
    N_("Name of DES keyfile for decoding encrypted ident responses.") },
  { "ident-encrypt-only", mu_c_bool, &ident_encrypt_only, 0, NULL,
    N_("Use only encrypted ident responses.") },
  { "id-fields", MU_CFG_LIST_OF(mu_c_string), &imap4d_id_list, 0, NULL,
    N_("List of fields to return in response to ID command.") },
  { "mandatory-locking", mu_cfg_section },
  { ".server", mu_cfg_section, NULL, 0, NULL,
    N_("Server configuration.") },
  { "transcript", mu_c_bool, &imap4d_transcript, 0, NULL,
    N_("Set global transcript mode.") },
  TCP_WRAPPERS_CONFIG
  { NULL }
};

struct mu_cli_setup cli = {
  .optv = options,
  .cfg = imap4d_cfg_param,
  .prog_doc = N_("GNU imap4d -- the IMAP4D daemon."),
  .server = 1
};

int
mu_get_user_groups (const char *user, mu_list_t retain, mu_list_t *pgrouplist)
{
  int rc;
  struct group *gr;
  mu_list_t list;
	
  if (!*pgrouplist)
    {
      rc = mu_list_create (pgrouplist);
      if (rc)
	{
	  mu_error(_("%s: cannot create list: %s"),
		   "mu_get_user_groups", mu_strerror (rc));
	  return rc;
	}
    }

  list = *pgrouplist;
  setgrent ();
  for (rc = 0; rc == 0 && (gr = getgrent ()); )
    {
      char **p;
      for (p = gr->gr_mem; *p; p++)
	if (strcmp (*p, user) == 0)
	  {
	    if (retain
		&& mu_list_locate (retain, (void*) (intptr_t) gr->gr_gid,
				   NULL))
	      continue;
	      
	    /* FIXME: Avoid duplicating gids */
	    rc = mu_list_append (list, (void*) (intptr_t) gr->gr_gid);
	    if (rc) 
	      mu_error(_("%s: cannot append to list: %s"),
		       "mu_get_user_groups",
		       mu_strerror (rc));
	    break;
	  }
    }
  endgrent ();
  return rc;
}

int
imap4d_session_setup0 ()
{
  if (auth_deny_user_list &&
      mu_list_locate (auth_deny_user_list, auth_data->name, NULL) == 0)
    {
      mu_error (_("%s is in deny-users, rejecting"), auth_data->name);
      return 1;
    }
  if (auth_allow_user_list &&
      mu_list_locate (auth_allow_user_list, auth_data->name, NULL))
    {
      mu_error (_("%s is not in allow-users, rejecting"), auth_data->name);
      return 1;
    }
  if (auth_deny_group_list &&
      imap_check_group_list (auth_deny_group_list) == 0)
    {
      mu_error (_("%s is in deny-groups, rejecting"), auth_data->name);
      return 1;
    }
  if (auth_allow_group_list &&
      imap_check_group_list (auth_allow_group_list))
    {
      mu_error (_("%s is not in allow-groups, rejecting"), auth_data->name);
      return 1;
    }

  real_homedir = mu_normalize_path (mu_strdup (auth_data->dir));
  if (imap4d_check_home_dir (real_homedir, auth_data->uid, auth_data->gid))
    return 1;
  
  if (auth_data->change_uid)
    {
      struct group *gr;
      mu_list_t groups = NULL;
      int rc;
      uid_t uid;

      uid = getuid ();
      if (uid != auth_data->uid)
	{
	  rc = mu_list_create (&groups);
	  if (rc)
	    {
	      mu_error(_("cannot create list: %s"), mu_strerror (rc));
	      free (real_homedir);
	      return 1;
	    }
	  mu_list_append (groups, (void*) (intptr_t) auth_data->gid);

	  rc = mu_get_user_groups (auth_data->name, user_retain_groups,
				   &groups);
	  if (rc)
	    {
	      /* FIXME: When mu_get_user_groups goes to the library, add a
		 diag message here */
	      free (real_homedir);
	      return 1;
	    }
	  gr = getgrnam ("mail");
	  rc = mu_switch_to_privs (auth_data->uid, gr->gr_gid, groups);
	  mu_list_destroy (&groups);
	  if (rc)
	    {
	      mu_error (_("can't switch to user %s privileges: %s"),
			auth_data->name, mu_strerror (rc));
	      free (real_homedir);
	      return 1;
	    }
	}
    }

  util_chdir (real_homedir);
  
  mu_diag_output (MU_DIAG_INFO,
		  _("user `%s' logged in (source: %s)"), auth_data->name,
		  auth_data->source);

  if (auth_data->quota)
    quota_setup ();
  
  return 0;
}

int
imap4d_session_setup (char *username)
{
  auth_data = mu_get_auth_by_name (username);
  if (auth_data == NULL)
    {
      mu_diag_output (MU_DIAG_INFO, _("user `%s' nonexistent"), username);
      return 1;
    }
  return imap4d_session_setup0 ();
}

void
imap4d_child_signal_setup (RETSIGTYPE (*handler) (int signo))
{
  static int sigtab[] = { SIGPIPE, SIGABRT, SIGINT, SIGQUIT,
			  SIGTERM, SIGHUP, SIGALRM };
  mu_set_signals (handler, sigtab, MU_ARRAY_SIZE (sigtab));
}

static int
imap4d_mainloop (int ifd, int ofd, struct imap4d_srv_config *cfg)
{
  imap4d_tokbuf_t tokp;
  char *text;
  int signo;
  struct imap4d_session session;

  if (!test_mode)
    test_mode = isatty (ifd);

  if ((signo = setjmp (child_jmp)))
    {
      mu_diag_output (MU_DIAG_CRIT, _("got signal `%s'"), strsignal (signo));
      switch (signo)
	{
	case SIGTERM:
	case SIGHUP:
	  signo = ERR_TERMINATE;
	  break;
      
	case SIGALRM:
	  signo = ERR_TIMEOUT;
	  break;
      
	case SIGPIPE:
	  signo = ERR_NO_OFILE;
	  break;
      
	default:
	  signo = ERR_SIGNAL;
	}
      imap4d_bye (signo);
    }
  else
    {
      /* Restore default handling for these signals: */
      static int defsigtab[] = { SIGILL, SIGBUS, SIGFPE, SIGSEGV, SIGSTOP };
      mu_set_signals (SIG_DFL, defsigtab, MU_ARRAY_SIZE (defsigtab));
      /* Set child-specific signal handlers */
      imap4d_child_signal_setup (imap4d_child_signal);
    }

  switch (cfg->tls_mode)
    {
    case tls_required:
      imap4d_capability_add (IMAP_CAPA_XTLSREQUIRED);
      imap4d_capability_add (IMAP_CAPA_STARTTLS);
      break;

    case tls_ondemand:
      imap4d_capability_add (IMAP_CAPA_STARTTLS);
      break;
      
    default:
      break;
    }
  
  session.tls_mode = cfg->tls_mode;
  session.tls_conf = &cfg->tls_conf;
  
  io_setio (ifd, ofd, cfg->tls_mode == tls_connection ? &cfg->tls_conf : NULL);
  if (cfg->tls_mode == tls_connection)
    tls_encryption_on (&session);

  if (imap4d_preauth_setup (ifd) == 0)
    {
      if (test_mode)
	{
	  mu_diag_output (MU_DIAG_INFO, _("started in test mode"));
	  text = "IMAP4rev1 Test mode";
	}
      else
	text = "IMAP4rev1";
    }
  else
    {
      io_flush ();
      return 0;
    }

  /* Greetings. */
  io_untagged_response ((state == STATE_AUTH) ? 
                        RESP_PREAUTH : RESP_OK, "%s", text);
  io_flush ();

  set_xscript_level ((state == STATE_AUTH) ?
                      MU_XSCRIPT_NORMAL : MU_XSCRIPT_SECURE);
  
  tokp = imap4d_tokbuf_init ();
  while (1)
    {
      if (idle_timeout && io_wait_input (idle_timeout) != 1)
	imap4d_bye (ERR_TIMEOUT);
      imap4d_readline (tokp);
      /* check for updates */
      imap4d_sync ();
      util_do_command (&session, tokp);
      imap4d_sync ();
      io_flush ();
    }

  return 0;
}

int
imap4d_connection (int fd, struct sockaddr *sa, int salen,
		   struct mu_srv_config *pconf, void *data)
{
  struct imap4d_srv_config *cfg = (struct imap4d_srv_config *) pconf;
  
  idle_timeout = cfg->m_cfg.timeout;
  imap4d_transcript = cfg->m_cfg.transcript;

  imap4d_mainloop (fd, fd, cfg);

  return 0;
}

int
imap4d_check_home_dir (const char *dir, uid_t uid, gid_t gid)
{
  struct stat st;

  if (stat (dir, &st))
    {
      if (errno == ENOENT && create_home_dir)
	{
	  mode_t mode = umask (0);
	  int rc = mkdir (dir, home_dir_mode);
	  umask (mode);
	  if (rc)
	    {
	      mu_error ("Cannot create home directory `%s': %s",
			dir, mu_strerror (errno));
	      return 1;
	    }
	  if (chown (dir, uid, gid))
	    {
	      mu_error ("Cannot set owner for home directory `%s': %s",
			dir, mu_strerror (errno));
	      return 1;
	    }
	}
    }
  
  return 0;
}


jmp_buf master_jmp;

RETSIGTYPE
imap4d_master_signal (int signo)
{
  longjmp (master_jmp, signo);
}

static void
imap4d_alloc_die ()
{
  imap4d_bye (ERR_NO_MEM);
}


int
main (int argc, char **argv)
{
  struct group *gr;
  int status = 0;
  static int sigtab[] = { SIGILL, SIGBUS, SIGFPE, SIGSEGV, SIGSTOP, SIGPIPE,
			  SIGABRT };

  imap4d_argc = argc;
  imap4d_argv = argv;
  
  /* Native Language Support */
  MU_APP_INIT_NLS ();

  state = STATE_NONAUTH;	/* Starting state in non-auth.  */

  MU_AUTH_REGISTER_ALL_MODULES ();
  /* Register the desired formats. */
  mu_register_local_mbox_formats ();
  
  imap4d_capability_init ();
  mu_tcpwrapper_cfg_init ();
  manlock_cfg_init ();
  mu_acl_cfg_init ();
  mu_tls_cfg_init ();
  namespace_cfg_init ();

  mu_m_server_create (&server, program_version);
  mu_m_server_set_config_size (server, sizeof (struct imap4d_srv_config));
  mu_m_server_set_conn (server, imap4d_connection);
  mu_m_server_set_prefork (server, mu_tcp_wrapper_prefork);
  mu_m_server_set_mode (server, MODE_INTERACTIVE);
  mu_m_server_set_max_children (server, 20);
  /* FIXME mu_m_server_set_pidfile (); */
  mu_m_server_set_default_port (server, 143);
  mu_m_server_set_timeout (server, 1800);  /* RFC2060: 30 minutes. */
  mu_m_server_set_strexit (server, mu_strexit);
  mu_m_server_cfg_init (server, imap4d_srv_param);
  
  mu_alloc_die_hook = imap4d_alloc_die;

  mu_log_syslog = 1;

  mu_cli (argc, argv, &cli, capa, server, &argc, &argv);
  if (argc)
    {
      mu_error (_("too many arguments"));
      exit (EX_USAGE);
    }

  namespace_init ();
  
  if (test_mode)
    mu_m_server_set_mode (server, MODE_INTERACTIVE);
  
  if (login_disabled)
    imap4d_capability_add (IMAP_CAPA_LOGINDISABLED);

  if (mu_gsasl_enabled ())
    {
      auth_gssapi_init ();
      auth_gsasl_init ();
    }

#ifdef USE_LIBPAM
  if (!mu_pam_service)
    mu_pam_service = "gnu-imap4d";
#endif

  if (mu_m_server_mode (server) == MODE_DAEMON)
    {
      /* Normal operation: */
      /* First we want our group to be mail so we can access the spool.  */
      errno = 0;
      gr = getgrnam ("mail");
      if (gr == NULL)
	{
	  if (errno == 0 || errno == ENOENT)
            {
               mu_error (_("%s: no such group"), "mail");
               exit (EX_CONFIG);
            }
          else
            {
	      mu_diag_funcall (MU_DIAG_ERROR, "getgrnam", "mail", errno);
	      exit (EX_OSERR);
            }
	}

      if (setgid (gr->gr_gid) == -1)
	{
	  mu_error (_("error setting mail group: %s"), mu_strerror (errno));
	  //exit (EX_OSERR);
	}
    }

  /* Set the signal handlers.  */
  if ((status = setjmp (master_jmp)))
    {
      int code;
      mu_diag_output (MU_DIAG_CRIT, _("MASTER: exiting on signal (%s)"),
		      strsignal (status));
      switch (status)
	{
	case SIGTERM:
	case SIGHUP:
	case SIGQUIT:
	case SIGINT:
	  code = EX_OK;
	  break;
	  
	default:
	  code = EX_SOFTWARE;
	  break;
	}
      
      exit (code); 
    }
  mu_set_signals (imap4d_master_signal, sigtab, MU_ARRAY_SIZE (sigtab));

  mu_stdstream_strerr_setup (mu_log_syslog ?
			     MU_STRERR_SYSLOG : MU_STRERR_STDERR);

  umask (S_IROTH | S_IWOTH | S_IXOTH);	/* 007 */

  /* Check TLS environment, i.e. cert and key files */
  mu_m_server_set_preflight (server, starttls_init);

  /* Actually run the daemon.  */
  if (mu_m_server_mode (server) == MODE_DAEMON)
    {
      mu_m_server_begin (server);
      status = mu_m_server_run (server);
      mu_m_server_end (server);
      mu_m_server_destroy (&server);
    }
  else
    {
      struct imap4d_srv_config cfg;
      memset (&cfg, 0, sizeof cfg);
      cfg.tls_mode = tls_no;
      /* Make sure we are in the root directory.  */
      chdir ("/");
      status = imap4d_mainloop (MU_STDIN_FD, MU_STDOUT_FD, &cfg);
    }

  if (status)
    mu_error (_("main loop status: %s"), mu_strerror (status));	  
  /* Close the syslog connection and exit.  */
  closelog ();

  return status ? EX_SOFTWARE : EX_OK;
}

