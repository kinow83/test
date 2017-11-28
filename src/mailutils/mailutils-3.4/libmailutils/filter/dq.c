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

/* Doble-quote escaping filter.
   In encode mode, escapes each occurrence of double-quote and backslash
   by prefixing it with backslash.
   In decode mode, removes backslash prefixes.
*/
   
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <mailutils/errno.h>
#include <mailutils/filter.h>
#include <mailutils/wordsplit.h>
#include <mailutils/cctype.h>

/* Move min(isize,osize) bytes from iptr to optr, replacing each
   escapable control character with its escape sequence. */
static enum mu_filter_result
_dq_encoder (void *xd MU_ARG_UNUSED,
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

  for (i = j = 0; i < isize && j < osize; i++)
    {
      unsigned char c = *iptr++;

      if (strchr ("\\\"", c))
	{
	  if (j + 1 == osize)
	    {
	      if (i == 0)
		{
		  iobuf->osize = 2;
		  return mu_filter_moreoutput;
		}
	      break;
	    }
	  else
	    {
	      optr[j++] = '\\';
	      optr[j++] = c;
	    }
	}
      else
	optr[j++] = c;
    }
  iobuf->isize = i;
  iobuf->osize = j;
  return mu_filter_ok;
}

/* Move min(isize,osize) bytes from iptr to optr, replacing each escape
   sequence with its ASCII code. */
static enum mu_filter_result
_dq_decoder (void *xd MU_ARG_UNUSED,
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

  for (i = j = 0; i < isize && j < osize; i++)
    {
      unsigned char c = *iptr++;
      if (c == '\\')
	{
	  if (i + 1 == isize)
	    break;
	  optr[j++] = *iptr++;
	}
      else
	optr[j++] = c;
    }

  iobuf->isize = i;
  iobuf->osize = j;
  return mu_filter_ok;
}

static struct _mu_filter_record _dq_filter = {
  "dq",
  NULL,
  _dq_encoder,
  _dq_decoder,
};

mu_filter_record_t mu_dq_filter = &_dq_filter;

