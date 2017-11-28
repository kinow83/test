/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2016-2017 Free Software Foundation, Inc.

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
#include <limits.h>
#include <string.h>
#include <mailutils/util.h>

/* Count number of occurrences of each ASCII character from CHR in the
   UTF-8 string STR.  Unless CNT is NULL, fill it with the counts for
   each character (so that CNT[i] contains number of occurrences of
   CHR[i]).  Return total number of occurrences. */
size_t
mu_str_count (char const *str, char const *chr, size_t *cnt)
{
  unsigned char c;
  int consume = 0;
  size_t count = 0;

  if (!str || !chr)
    return 0;
  
  if (cnt)
    {
      int i;
      
      for (i = 0; chr[i]; i++)
	cnt[i] = 0;
    }
  
  while ((c = *str++) != 0)
    {
      if (consume)
	consume--;
      else if (c < 0xc0)
	{
	  char *p = strchr (chr, c);
	  if (p)
	    {
	      if (cnt)
		cnt[p - chr]++;
	      count++;
	    }
	}
      else if (c & 0xc0)
	consume = 1;
      else if (c & 0xe0)
	consume = 2;
      else if (c & 0xf0)
	consume = 3;      
    }
  return count;
}
