/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2012, 2014-2017 Free Software Foundation, Inc.

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

#include <config.h>
#include <mailutils/types.h>
#include <mailutils/imapio.h>
#include <mailutils/stream.h>
#include <mailutils/imaputil.h>
#include <mailutils/sys/imapio.h>

int
mu_imapio_send_flags (struct _mu_imapio *io, int flags)
{
  int rc;

  rc = mu_stream_write (io->_imap_stream, "(", 1, NULL);
  if (rc)
    return rc;
  rc = mu_imap_format_flags (io->_imap_stream, flags, io->_imap_server);
  if (rc == 0)
    rc = mu_stream_write (io->_imap_stream, ")", 1, NULL);
  return rc;
}
