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
#include <mailutils/util.h>

/* Return 1 if file name STR is safe, or 0 otherwise.
   The name is not safe if it begins with .. or contains /../
*/
int
mu_file_name_is_safe (char const *str)
{
  enum { st_init, st_slash, st_dot, st_dotdot } state;
  unsigned char c;
  int consume = 0;

  if (!str)
    return 0;
  
  state = (*str == '.') ? st_dot : st_init;

  while ((c = *str++) != 0)
    {
      if (consume)
	consume--;
      else if (c < 0xc0)
	{
	  switch (state)
	    {
	    case st_init:
	      if (c == '/')
		state = st_slash;
	      break;
	      
	    case st_slash:
	      if (c == '.')
		state = st_dot;
	      else if (c != '/')
		state = st_init;
	      break;
	      
	    case st_dot:
	      if (c == '.')
		state = st_dotdot;
	      else if (c == '/')
		state = st_slash;
	      else
		state = st_init;
	      break;
	      
	    case st_dotdot:
	      if (c == '/')
		return 0;
	      else
		state = st_init;
	      break;
	    }
	}
      else if (c & 0xc0)
	consume = 1;
      else if (c & 0xe0)
	consume = 2;
      else if (c & 0xf0)
	consume = 3;
    }

  if (state == st_dotdot)
    return 0;

  return 1;
}
