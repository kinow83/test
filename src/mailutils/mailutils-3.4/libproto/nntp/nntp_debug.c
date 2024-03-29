/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2004, 2007, 2010-2012, 2014-2017 Free Software
   Foundation, Inc.

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

#include <stdlib.h>
#include <errno.h>
#include <mailutils/sys/nntp.h>

int
mu_nntp_debug_cmd (mu_nntp_t nntp)
{
  mu_debug (MU_DEBCAT_FOLDER, MU_DEBUG_PROT, ("%s", nntp->io.buf));
  return 0;
}

int
mu_nntp_debug_ack (mu_nntp_t nntp)
{
  mu_debug (MU_DEBCAT_FOLDER, MU_DEBUG_PROT, "%s", nntp->ack.buf);
  return 0;
}
