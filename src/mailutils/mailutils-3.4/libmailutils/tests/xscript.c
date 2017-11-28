/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2005, 2007, 2009-2012, 2014-2017 Free Software
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

/* A simple echo server for testing transcript streams.
   Each input line is prefixed with the input line number and echoed on
   stdout.
   
   Input starting with '\\' is a command request.  It must contain a string
   consisting of one of the following letters:
     n       toggle normal transcript mode;
     s       toggle secure transcript mode (passwords are not displayed);
     p       toggle payload transcript mode.
     
   Several letters can be passed in a single request, although that doesn't
   have much sense.
   
   The server responds to the command request with a "// " followed by
   the letters describing previous transcript mode.
   
   Transcript of the session goes to stderr.
*/

#include <mailutils/mailutils.h>

mu_stream_t
create_transcript (void)
{
  mu_stream_t iostr;
  mu_stream_t xstr;
  mu_stream_t dstr;
  
  mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);
  MU_ASSERT (mu_iostream_create (&iostr, mu_strin, mu_strout));
  mu_stream_set_buffer (iostr, mu_buffer_line, 0);

  MU_ASSERT (mu_dbgstream_create (&dstr, MU_DIAG_DEBUG));
  
  MU_ASSERT (mu_xscript_stream_create (&xstr, iostr, dstr, NULL));
  mu_stream_unref (dstr);
  mu_stream_unref (iostr);
  return xstr;
}

static int
xstream_setopt (mu_stream_t str, char const *buf)
{
  int flag = MU_XSCRIPT_NORMAL;
  for (; *buf; buf++)
    {
      switch (*buf)
	{
	case 'n':
	case 'N':
	  flag |= MU_XSCRIPT_NORMAL;
	  break;
	case 's':
	case 'S':
	  flag |= MU_XSCRIPT_SECURE;
	  break;
	case 'p':
	case 'P':
	  flag |= MU_XSCRIPT_PAYLOAD;
	  break;
	case '\n':
	  break;
	default:
	  mu_error ("unrecognized flag: %c", *buf);
	  return -1;
	}
    }      
  MU_ASSERT (mu_stream_ioctl (str, MU_IOCTL_XSCRIPTSTREAM,
			      MU_IOCTL_XSCRIPTSTREAM_LEVEL, &flag));
  mu_stream_printf (str, "// ");
  if (flag == MU_XSCRIPT_NORMAL)
    mu_stream_printf (str, "n");
  else
    {
      if (flag & MU_XSCRIPT_SECURE)
	mu_stream_printf (str, "s");
      if (flag & MU_XSCRIPT_PAYLOAD)
	mu_stream_printf (str, "p");
    }
  mu_stream_printf (str, "\n");
  return 0;
}

int
main (int argc, char **argv)
{
  int rc;
  char *buf = NULL;
  size_t size = 0, n;
  unsigned line = 0;
  mu_stream_t str = create_transcript ();

  while ((rc = mu_stream_getline (str, &buf, &size, &n)) == 0 && n > 0)
    {
      line++;
      if (buf[0] == '\\')
	{
	  xstream_setopt (str, buf + 1);
	  continue;
	}
      mu_stream_printf (str, "%04u: %s", line, buf);
    }
  mu_stream_destroy (&str);
  exit (0);
}

  
