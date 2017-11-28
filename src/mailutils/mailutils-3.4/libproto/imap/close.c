/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2012, 2014-2017 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see
   <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <mailutils/imap.h>
#include <mailutils/sys/imap.h>

void
_mu_close_handler (mu_imap_t imap)
{
  if (imap->response == MU_IMAP_OK)
    imap->session_state = MU_IMAP_SESSION_AUTH;
}

int
mu_imap_close (mu_imap_t imap)
{
  static char const *command = "CLOSE";
  static struct imap_command com = {
    MU_IMAP_SESSION_SELECTED,
    NULL,
    MU_IMAP_CLIENT_CLOSE_RX,
    1,
    &command,
    NULL,
    NULL,
    _mu_close_handler
  };
  return mu_imap_gencom (imap, &com);
}
      
