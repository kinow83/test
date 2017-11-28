/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2001, 2005, 2007, 2009-2012, 2014-2017 Free
   Software Foundation, Inc.

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
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <mailutils/diag.h>
#include <mailutils/nls.h>
#include <mailutils/errno.h>
#include <mailutils/stdstream.h>
#include <mailutils/stream.h>
#include <mailutils/locus.h>

void
mu_diag_init ()
{
  if (!mu_strerr)
    mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);
}

void
mu_diag_voutput (int level, const char *fmt, va_list ap)
{
  mu_diag_init ();  
  mu_stream_printf (mu_strerr, "\033s<%d>", level);
  mu_stream_vprintf (mu_strerr, fmt, ap);
  mu_stream_write (mu_strerr, "\n", 1, NULL);
}

void
mu_diag_output (int level, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  mu_diag_voutput (level, fmt, ap);
  va_end (ap);
}

void
mu_vdiag_at_locus_range (int level, struct mu_locus_range const *loc,
			 const char *fmt, va_list ap)
{
  struct mu_locus_range old_loc = MU_LOCUS_RANGE_INITIALIZER;
  int old_mode;
  int restore = 0;
  
  if (loc)
    {
      if (mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
			   MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE, &old_loc) == 0)
	{
	  if (mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
			       MU_IOCTL_LOGSTREAM_GET_MODE, &old_mode) == 0)
	    {
	      int mode = old_mode | MU_LOGMODE_LOCUS;
	      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
			       MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
	      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
			       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, (void*) loc);
	      restore = 1;
	    }
	}
    }

  mu_diag_voutput (level, fmt, ap);

  if (restore)
    {
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &old_loc);
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		       MU_IOCTL_LOGSTREAM_SET_MODE, &old_mode);
      mu_locus_range_deinit (&old_loc);
    }
}

void
mu_diag_at_locus_range (int level, struct mu_locus_range const *loc,
			const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  mu_vdiag_at_locus_range (level, loc, fmt, ap);
  va_end (ap);
}

void
mu_diag_at_locus_point (int level, struct mu_locus_point const *loc,
			const char *fmt, ...)
{
  va_list ap;
  struct mu_locus_range lr = MU_LOCUS_RANGE_INITIALIZER;
  lr.beg = *loc;
  va_start (ap, fmt);
  mu_vdiag_at_locus_range (level, &lr, fmt, ap);
  va_end (ap);
}

void
mu_diag_vprintf (int level, const char *fmt, va_list ap)
{
  mu_diag_init ();
  mu_stream_printf (mu_strerr, "\033<%d>", level);
  mu_stream_vprintf (mu_strerr, fmt, ap);
}

void
mu_diag_cont_vprintf (const char *fmt, va_list ap)
{
  mu_diag_init ();
  mu_stream_vprintf (mu_strerr, fmt, ap);
}

void
mu_diag_printf (int level, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  mu_diag_vprintf (level, fmt, ap);
  va_end (ap);
}

void
mu_diag_cont_printf (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  mu_diag_cont_vprintf (fmt, ap);
  va_end (ap);
}

const char *
mu_diag_level_to_string (int level)
{
  switch (level)
    {
    case MU_DIAG_EMERG:
      return _("emergency");
      
    case MU_DIAG_ALERT:
      return _("alert");
	
    case MU_DIAG_CRIT:
      return _("critical");
      
    case MU_DIAG_ERROR:
      return _("error");
      
    case MU_DIAG_WARNING:
      return _("warning");
      
    case MU_DIAG_NOTICE:
      return _("notice");
      
    case MU_DIAG_INFO:
      return _("info");
      
    case MU_DIAG_DEBUG:
      return _("debug");
    }
  return _("unknown");
}

void
mu_diag_funcall (mu_log_level_t level, const char *func,
		 const char *arg, int err)
{
  if (err)
    /* TRANSLATORS: First %s stands for function name, second for its
       arguments, third one for the actual error message. */
    mu_diag_output (level, _("%s(%s) failed: %s"), func, mu_prstr (arg),
		    mu_strerror (err));
  else
    /* TRANSLATORS: First %s stands for function name, second for its
       arguments. */
    mu_diag_output (level, _("%s(%s) failed"), func, mu_prstr (arg));
}
