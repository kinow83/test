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
#include <mailutils/types.h>
#include <mailutils/errno.h>
#include <mailutils/list.h>
#include <mailutils/msgset.h>
#include <mailutils/sys/msgset.h>

int
mu_msgset_last (mu_msgset_t msgset, size_t *ret)
{
  int rc;
  struct mu_msgrange const *range;

  if (mu_msgset_is_empty (msgset))
    return EINVAL;
  if (!ret)
    return MU_ERR_OUT_PTR_NULL;
  rc = mu_msgset_aggregate (msgset);
  if (rc)
    return rc;
  rc = mu_list_tail (msgset->list, (void **) &range);
  if (rc == 0)
    *ret = range->msg_end;
  return rc;
}
