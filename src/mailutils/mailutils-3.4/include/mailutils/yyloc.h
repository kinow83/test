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

#ifndef _MAILUTILS_YYLOC_H
#define _MAILUTILS_YYLOC_H

void mu_file_print_locus_point (FILE *,
				struct mu_locus_point const *lpt);
void mu_file_print_locus_range (FILE *,
				struct mu_locus_range const *loc);

#define YYLTYPE struct mu_locus_range
#define YYLLOC_DEFAULT(Current, Rhs, N)				  \
  do								  \
    {								  \
      if (N)							  \
	{							  \
	  (Current).beg = YYRHSLOC(Rhs, 1).beg;			  \
	  (Current).end = YYRHSLOC(Rhs, N).end;			  \
	}							  \
      else							  \
	{							  \
	  (Current).beg = YYRHSLOC(Rhs, 0).end;			  \
	  (Current).end = (Current).beg;			  \
	}							  \
    } while (0)

#define YY_LOCATION_PRINT(File, Loc)     		        \
  mu_file_print_locus_range (File, &(Loc))

#endif    
  
