/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2011-2012, 2014-2017 Free Software Foundation, Inc.

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
#include <stdlib.h>
#include <mailutils/types.h>
#include <mailutils/errno.h>
#include <mailutils/list.h>
#include <mailutils/stream.h>
#include <mailutils/io.h>
#include <mailutils/msgset.h>
#include <mailutils/sys/msgset.h>

struct mu_msgset_format const mu_msgset_formats[] = {
  [MU_MSGSET_FMT_IMAP] = {
    .delim = ",",
    .range = ":",
    .last  = "*",
    .empty = "NIL"
  },
  [MU_MSGSET_FMT_MH] = {
    .delim = " ",
    .range = "-",
    .last  = "last"
  }
};


struct print_env
{
  mu_stream_t stream;
  int cont;
  struct mu_msgset_format const *fmt;
};

static int
_msgrange_printer (void *item, void *data)
{
  int rc;
  struct mu_msgrange *range = item;
  struct print_env *env = data;
  
  if (env->cont)
    {
      rc = mu_stream_printf (env->stream, "%s", env->fmt->delim);
      if (rc)
	return rc;
    }
  else
    env->cont = 1;

  if (range->msg_beg == range->msg_end)
    rc = mu_stream_printf (env->stream, "%lu", (unsigned long) range->msg_beg);
  else if (range->msg_end == 0)
    rc = mu_stream_printf (env->stream, "%lu%s%s",
			   (unsigned long) range->msg_beg,
			   env->fmt->range,
			   env->fmt->last);
  else if (range->msg_end == range->msg_beg + 1)
    rc = mu_stream_printf (env->stream, "%lu%s%lu",
			   (unsigned long) range->msg_beg,
			   env->fmt->delim,
			   (unsigned long) range->msg_end);
  else
    rc = mu_stream_printf (env->stream, "%lu%s%lu",
			   (unsigned long) range->msg_beg,
			   env->fmt->range,
			   (unsigned long) range->msg_end);
  return rc;
}

int
mu_stream_msgset_format (mu_stream_t str, struct mu_msgset_format const *fmt,
			 mu_msgset_t mset)
{
  struct print_env env;
  int rc;

  env.stream = str;
  env.cont = 0;
  env.fmt = fmt;
  
  if (mu_list_is_empty (mset->list))
    {
      if (env.fmt->empty)
	return mu_stream_printf (str, "%s", env.fmt->empty);
      return 0;
    }
  rc = mu_msgset_aggregate (mset);
  if (rc)
    return rc;
  return mu_list_foreach (mset->list, _msgrange_printer, &env);
}

int
mu_msgset_print (mu_stream_t str, mu_msgset_t mset)
{
  return mu_stream_msgset_format (str, mu_msgset_fmt_imap, mset);
}
    
