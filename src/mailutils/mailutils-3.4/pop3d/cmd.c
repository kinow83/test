/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2009-2012, 2014-2017 Free Software Foundation, Inc.

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

#include "pop3d.h"

struct mu_tls_config global_tls_conf;

static struct pop3d_command command_table[] = {
  { "STLS", pop3d_stls },
  { "RETR", pop3d_retr },
  { "DELE", pop3d_dele },
  { "USER", pop3d_user },
  { "QUIT", pop3d_quit },
  { "APOP", pop3d_apop },
  { "AUTH", pop3d_auth },
  { "STAT", pop3d_stat },
  { "LIST", pop3d_list },
  { "NOOP", pop3d_noop },
  { "RSET", pop3d_rset },
  { "TOP",  pop3d_top },
  { "UIDL", pop3d_uidl },
  { "CAPA", pop3d_capa },
  { NULL }
};

pop3d_command_handler_t
pop3d_find_command (const char *name)
{
  struct pop3d_command *p;
  for (p = command_table; p->name; p++)
    {
      if (mu_c_strcasecmp (name, p->name) == 0)
	return p->handler;
    }
  return p->handler;
}

int
stls_preflight (mu_m_server_t msrv)
{
  mu_list_t srvlist;
  mu_iterator_t itr;
  int errors = 0;
  int tls_ok = mu_init_tls_libs ();
  int tls_requested = 0;
  int global_conf_status = 0;

  if (global_tls_conf.cert_file)
    global_conf_status = mu_tls_config_check (&global_tls_conf, 1);
  else
    global_conf_status = MU_TLS_CONFIG_NULL;
  
  mu_m_server_get_srvlist (msrv, &srvlist);
  mu_list_get_iterator (srvlist, &itr);
  for (mu_iterator_first (itr); !mu_iterator_is_done (itr); mu_iterator_next (itr))
    {
      mu_ip_server_t ipsrv;
      struct pop3d_srv_config *cfg;
      mu_iterator_current (itr, (void**) &ipsrv);
      cfg = mu_ip_server_get_data (ipsrv);
      switch (cfg->tls_mode)
	{
	case tls_unspecified:
	  if (cfg->tls_conf.cert_file)
	    {
	      cfg->tls_mode = tls_ondemand;
	      break;
	    }
	  else
	    cfg->tls_mode = tls_no;
	  /* fall through */
	case tls_no:
	  continue;
	  
	default:
	  break;
	}

      switch (mu_tls_config_check (&cfg->tls_conf, 1))
	{
	case MU_TLS_CONFIG_OK:
	  if (!cfg->tls_conf.cert_file)
	    {
	      mu_error (_("server %s: no certificate set"),
			mu_ip_server_addrstr (ipsrv));
	      errors = 1;
	    }
	  break;
	  
	case MU_TLS_CONFIG_NULL:
	  if (global_conf_status != MU_TLS_CONFIG_NULL)
	    {
	      cfg->tls_conf = global_tls_conf;
	    }
	  else
	    {
	      mu_error (_("server %s: no certificate set"),
			mu_ip_server_addrstr (ipsrv));
	      errors = 1;
	    }
	  break;

	default:
	  mu_error (_("server %s: TLS configuration failed"),
		    mu_ip_server_addrstr (ipsrv));
	  errors = 1;
	}
      
      tls_requested = 1;
    }
  mu_iterator_destroy (&itr);

  if (tls_requested && !tls_ok)
    {
      mu_error (_("TLS is not configured, but requested in the "
		  "configuration"));
      errors = 1;
    }
  
  if (errors)
    return 1;

  return 0;
}

static char *error_table[] = {
  [ERR_WRONG_STATE] =   "Incorrect state",
  [ERR_BAD_ARGS]    =   "Invalid arguments",
  [ERR_BAD_LOGIN]   =   "[AUTH] Bad login",
  [ERR_NO_MESG]     =   "No such message",
  [ERR_MESG_DELE]   =   "Message has been deleted",
  [ERR_NOT_IMPL]    =   "Not implemented",
  [ERR_BAD_CMD]     =   "Invalid command",
  [ERR_MBOX_LOCK]   =   "[IN-USE] Mailbox in use",
  [ERR_TOO_LONG]    =   "Argument too long",
  [ERR_NO_MEM]      =   "Out of memory] =quitting",
  [ERR_SIGNAL]      =   "Quitting on signal",
  [ERR_FILE]        =   "Some deleted messages not removed",
  [ERR_NO_IFILE]    =   "No input stream",
  [ERR_NO_OFILE]    =   "No output stream",
  [ERR_IO]          =   "I/O error",
  [ERR_PROTO]       =   "Remote protocol error",
  [ERR_TIMEOUT]     =   "Session timed out",
  [ERR_UNKNOWN]     =   "Unknown error",
  [ERR_MBOX_SYNC]   =   "Mailbox was updated by other process",
  [ERR_TLS_IO]      =   "TLS I/O error",
  [ERR_LOGIN_DELAY] =
    "[LOGIN-DELAY] Attempt to log in within the minimum login delay interval",
  [ERR_TERMINATE]   =   "Terminating on request",
  [ERR_SYS_LOGIN]   =   "[SYS/PERM] Cannot authenticate",
};

const char *
pop3d_error_string (int code)
{
  if (code >= 0 && code < MU_ARRAY_SIZE (error_table) && error_table[code])
    return error_table[code];
  return "unknown error";
}
