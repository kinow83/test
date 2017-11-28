/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003, 2007-2012, 2014-2017 Free Software Foundation,
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

struct mu_tls_config global_tls_conf;

/*
6.2.1.  STARTTLS Command

   Arguments:  none

   Responses:  no specific response for this command

   Result:     OK - starttls completed, begin TLS negotiation
               BAD - command unknown or arguments invalid
*/
int
imap4d_starttls (struct imap4d_session *session,
                 struct imap4d_command *command, imap4d_tokbuf_t tok)
{
  int status;

  if (session->tls_mode == tls_no)
    return io_completion_response (command, RESP_BAD, "Invalid command");

  if (imap4d_tokbuf_argc (tok) != 2)
    return io_completion_response (command, RESP_BAD, "Invalid arguments");

  status = io_completion_response (command, RESP_OK, "Begin TLS negotiation");
  io_flush ();

  if (imap4d_init_tls_server (session->tls_conf) == 0)
    tls_encryption_on (session);
  else
    {
      mu_diag_output (MU_DIAG_ERROR, _("session terminated"));
      util_bye ();
      exit (EX_OK);
    }

  return status;
}

void
tls_encryption_on (struct imap4d_session *session)
{
  session->tls_mode = tls_no;
  imap4d_capability_remove (IMAP_CAPA_STARTTLS);
      
  login_disabled = 0;
  imap4d_capability_remove (IMAP_CAPA_LOGINDISABLED);

  imap4d_capability_remove (IMAP_CAPA_XTLSREQUIRED);
}

int
starttls_init (mu_m_server_t msrv)
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
      struct imap4d_srv_config *cfg;
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

/* EOF */
