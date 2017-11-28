/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2017 Free Software Foundation, Inc.

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

#include <stdlib.h>
#include <stdarg.h>
#include <mailutils/types.h>
#include <mailutils/locus.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/diag.h>
#include <mailutils/stream.h>
#include <mailutils/stdstream.h>

void
mu_stream_vlprintf (mu_stream_t stream,
		    struct mu_locus_range const *loc,
		    char const *fmt, va_list ap)
{
  mu_stream_print_locus_range (stream, loc);
  mu_stream_write (stream, ": ", 2, NULL);
  mu_stream_vprintf (stream, fmt, ap);
}

void
mu_stream_lprintf (mu_stream_t stream,
		   struct mu_locus_range const *loc,
		   char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  mu_stream_vlprintf (stream, loc, fmt, ap);
  va_end (ap);
}

void
mu_lrange_debug (struct mu_locus_range const *loc,
		 char const *fmt, ...)
{
  va_list ap;
  int rc, mode;

  rc = mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
			MU_IOCTL_LOGSTREAM_GET_MODE, &mode);
  if (rc == 0)
    {
      int new_mode = mode & ~MU_LOGMODE_LOCUS;
      rc = mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
			    MU_IOCTL_LOGSTREAM_SET_MODE, &new_mode);
    }

  va_start (ap, fmt);
  mu_stream_vlprintf (mu_strerr, loc, fmt, ap);
  va_end (ap);
  mu_stream_write (mu_strerr, "\n", 1, NULL);

  if (rc == 0)
    mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		     MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
}
