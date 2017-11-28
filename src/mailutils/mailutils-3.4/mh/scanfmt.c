/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2003, 2005-2012, 2014-2017 Free Software
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

#include <mh.h>

static char mh_scan_format_str[] = 
  "%4(msg)"
  "%<(cur)+%| %>"
  "%<{replied}-%?{encrypted}E%| %>"
  "%02(mon{date})/%02(mday{date})"
  "%<{date} %|*%>"
  "%<(mymbox{from})%<{to}To:%14(decode(friendly{to}))%>%>"
  "%<(zero)%17(decode(friendly{from}))%>"
  "  %(decode{subject})%<{body}<<%{body}>>%>";

mh_format_t
mh_scan_format (void)
{
  mh_format_t fmt;
  
  if (mh_format_string_parse (&fmt, mh_scan_format_str,
			      NULL, MH_FMT_PARSE_DEFAULT))
    {
      mu_error (_("INTERNAL ERROR: bad built-in format; please report"));
      exit (1);
    }
  return fmt;
}

