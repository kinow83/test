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

#ifndef STREAM_TYPE
# error "STREAM_TYPE not defined"
#endif
#ifndef PRINT_LOCUS_POINT
# error "PRINT_LOCUS_POINT not defined"
#endif
#ifndef PRINT_LOCUS_RANGE
# error "PRINT_LOCUS_RANGE not defined"
#endif
#ifndef STREAM_PRINTF
# error "STREAM_PRINTF not defined"
#endif

#include <mailutils/types.h>
#include <mailutils/locus.h>

void
PRINT_LOCUS_POINT (STREAM_TYPE stream, struct mu_locus_point const *lp)
{
  if (lp->mu_file)
    {
      STREAM_PRINTF (stream, "%s:%u",
			lp->mu_file, lp->mu_line);
      if (lp->mu_col)
	STREAM_PRINTF (stream, ".%u", lp->mu_col);
    }
}

void
PRINT_LOCUS_RANGE (STREAM_TYPE stream, struct mu_locus_range const *loc)
{
  PRINT_LOCUS_POINT (stream, &loc->beg);
  if (loc->end.mu_file)
    {
      if (!mu_locus_point_same_file (&loc->beg, &loc->end))
	{
	  STREAM_PRINTF (stream, "-");
	  PRINT_LOCUS_POINT (stream, &loc->end);
	}
      else if (loc->beg.mu_line != loc->end.mu_line)
	{
	  STREAM_PRINTF (stream, "-");
	  STREAM_PRINTF (stream, "%u", loc->end.mu_line);
	  if (loc->end.mu_col)
	    STREAM_PRINTF (stream, ".%u", loc->end.mu_col);
	}
      else if (loc->beg.mu_col
	       && loc->beg.mu_col != loc->end.mu_col)
	{
	  STREAM_PRINTF (stream, "-");
	  STREAM_PRINTF (stream, "%u", loc->end.mu_col);
	}
    }
}
