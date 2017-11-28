/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003, 2007, 2009-2012, 2014-2017 Free Software
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

#include "pop3d.h"

/* STLS command -- TLS/SSL encryption */

int
pop3d_stls (char *arg, struct pop3d_session *session)
{
  if (session->tls_mode == tls_no)
    return ERR_BAD_CMD;
      
  if (strlen (arg) != 0)
    return ERR_BAD_ARGS;

  if (state != initial_state)
    return ERR_WRONG_STATE;

  switch (session->tls_mode)
    {
    case tls_ondemand:
    case tls_required:
      break;
    default:
      return ERR_WRONG_STATE;
    }
  
  pop3d_outf ("+OK Begin TLS negotiation\n");
  pop3d_flush_output ();

  if (pop3d_init_tls_server (session->tls_conf))
    {
      mu_diag_output (MU_DIAG_ERROR, _("Session terminated"));
      state = ABORT;
      return ERR_UNKNOWN;
    }

  session->tls_mode = tls_no;
  state = AUTHORIZATION;  /* Confirm we're in this state. Necessary for
			     "tls required" to work */

  return OK;
}

