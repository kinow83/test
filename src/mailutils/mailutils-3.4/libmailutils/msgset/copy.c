/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2017 Free Software Foundation, Inc.

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
#include <mailutils/msgset.h>
#include <mailutils/sys/msgset.h>

static int
copy_range (void *item, void *data)
{
  struct mu_msgrange const *range = item;
  mu_list_t list = data;
  struct mu_msgrange *copy;

  copy = malloc (sizeof (*copy));
  if (!copy)
    return ENOMEM;
  *copy = *range;
  return mu_list_append (list, copy);
}
	    
int
mu_msgset_copy (mu_msgset_t src, mu_msgset_t dst)
{
  mu_list_t list;
  int rc;
  
  if (!src || !dst)
    return EINVAL;

  rc = mu_list_create (&list);
  if (rc)
    return rc;

  rc = mu_list_foreach (src->list, copy_range, list);
  if (rc == 0)
    mu_list_append_list (dst->list, list);
  mu_list_destroy (&list);

  return rc;
}
