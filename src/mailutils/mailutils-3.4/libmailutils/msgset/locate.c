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
#include <mailutils/msgset.h>
#include <mailutils/sys/msgset.h>

int
mu_msgset_locate (mu_msgset_t msgset, size_t n,
		  struct mu_msgrange const **prange)
{
  int rc;
  if (!msgset || n == 0)
    return EINVAL;
  rc = mu_msgset_aggregate (msgset);
  if (rc)
    return rc;
  return mu_list_locate (msgset->list, &n, (void**)prange);
}
