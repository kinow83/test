/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2012, 2014-2017 Free Software Foundation, Inc.

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
#include <string.h>
#include <limits.h>
#include <mailutils/errno.h>
#include <mailutils/filter.h>
#include <mailutils/cctype.h>

static char xchar[] = "0123456789ABCDEF";

static enum mu_filter_result
percent_encoder (void *xd,
		 enum mu_filter_command cmd,
		 struct mu_filter_io *iobuf)
{
  size_t i, j;
  const unsigned char *iptr;
  size_t isize;
  char *optr;
  size_t osize;
  char *escape_chars = xd;
  
  switch (cmd)
    {
    case mu_filter_init:
    case mu_filter_done:
      return mu_filter_ok;
    default:
      break;
    }

  iptr = (const unsigned char *) iobuf->input;
  isize = iobuf->isize;
  optr = iobuf->output;
  osize = iobuf->osize;

  for (i = j = 0; i < isize && j < osize; i++)
    {
      unsigned char c = iptr[i];

      if (c == 0 || strchr (escape_chars, c))
	{
	  if (j + 3 >= osize)
	    {
	      if (i == 0)
		{
		  iobuf->osize = 3;
		  return mu_filter_moreoutput;
		}
	      break;
	    }
	  optr[j++] = '%';
	  optr[j++] = xchar[((c >> 4) & 0xf)];
	  optr[j++] = xchar[c & 0xf];
	}
      else
	optr[j++] = c;
    }
  iobuf->isize = i;
  iobuf->osize = j;
  return mu_filter_ok;
}

static enum mu_filter_result
percent_decoder (void *xd MU_ARG_UNUSED,
		 enum mu_filter_command cmd,
		 struct mu_filter_io *iobuf)
{
  size_t i, j;
  const unsigned char *iptr;
  size_t isize;
  char *optr;
  size_t osize;
  
  switch (cmd)
    {
    case mu_filter_init:
    case mu_filter_done:
      return mu_filter_ok;
    default:
      break;
    }
  
  iptr = (const unsigned char *) iobuf->input;
  isize = iobuf->isize;
  optr = iobuf->output;
  osize = iobuf->osize;

  for (i = j = 0; i < isize && j < osize; j++)
    {
      unsigned char c = iptr[i++];
      if (c == '%')
	{
	  char *phi, *plo;
	  
	  if (i + 2 >= isize)
	    break;
	  phi = strchr (xchar, mu_toupper (iptr[i]));
	  plo = strchr (xchar, mu_toupper (iptr[i+1]));
	  if (phi && plo)
	    {
	      optr[j] = ((phi - xchar) << 4) + (plo - xchar);
	      i += 2;
	    }
	  else
	    optr[j] = c;
	}
      else
	optr[j] = c;
    }
  iobuf->isize = i;
  iobuf->osize = j;
  return mu_filter_ok;
}

static int
percent_alloc (void **pret, int mode, int argc, const char **argv)
{
  if (mode == MU_FILTER_ENCODE)
    {
      char *s;
      if (argc > 1)
	{
	  int i;
	  size_t len = 0;

	  for (i = 1; i < argc; i++)
	    len += strlen (argv[i]);
	  s = malloc (len + 1);
	  if (!s)
	    return ENOMEM;
	  *pret = s;
	  *s = 0;
	  for (i = 1; i < argc; i++)
	    strcat (s, argv[i]);
	}
      else
	{
	  int i;
	  
	  s = malloc (UCHAR_MAX);
	  if (!s)
	    return ENOMEM;
	  *pret = s;
	  for (i = 1; i <= UCHAR_MAX; i++)
	    {
	      if (i == '%' || i == '"' || !mu_isgraph (i))
		*s++ = i;
	    }
	  *s = 0;
	}
    }
  else
    *pret = NULL;
  return 0;
}

static struct _mu_filter_record _percent_filter = {
  "percent",
  percent_alloc,
  percent_encoder,
  percent_decoder
};

mu_filter_record_t mu_percent_filter = &_percent_filter;

  
