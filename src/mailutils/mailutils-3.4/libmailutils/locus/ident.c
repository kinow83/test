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

#include <stdlib.h>
#include <mailutils/types.h>
#include <mailutils/assoc.h>
#include <mailutils/locus.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/diag.h>
#include <mailutils/list.h>
#include <mailutils/io.h>
#include <mailutils/stream.h>
#include <mailutils/iterator.h>

struct mu_ident_ref
{
  size_t count;
};

static mu_assoc_t nametab;

int
mu_ident_ref (char const *name, char const **refname)
{
  int rc;
  struct mu_ident_ref *ref, **refptr;
  
  if (!refname)
    return MU_ERR_OUT_PTR_NULL;
  if (!name)
    {
      *refname = NULL;
      return 0;
    }
  
  if (!nametab)
    {
      rc = mu_assoc_create (&nametab, 0);
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_assoc_create", NULL, rc);
	  return rc;
	}
      mu_assoc_set_destroy_item (nametab, mu_list_free_item);
    }
  rc = mu_assoc_install_ref2 (nametab, name, &refptr, refname);
  switch (rc)
    {
    case 0:
      ref = malloc (sizeof *ref);
      if (!ref)
	{
	  rc = errno;
	  mu_assoc_remove (nametab, name);
	  return rc;
	}
      *refptr = ref;
      ref->count = 0;
      break;
      
    case MU_ERR_EXISTS:
      ref = *refptr;
      break;
      
    default:
      mu_diag_funcall (MU_DIAG_ERROR, "mu_assoc_install_ref2", name, rc);
      return rc;
    }

  ref->count++;
  return 0;
}

int
mu_ident_deref (char const *name)
{
  struct mu_ident_ref *ref;
  int rc;

  if (!name || !nametab)
    return 0;
  
  rc = mu_assoc_lookup (nametab, name, &ref);
  switch (rc)
    {
    case 0:
      if (--ref->count == 0)
	mu_assoc_remove (nametab, name);
      break;

    case MU_ERR_NOENT:
      break;

    default:
      mu_diag_funcall (MU_DIAG_ERROR, "mu_assoc_lookup", name, rc);
      return rc;
    }

  return 0;
}
  
void
mu_ident_stat (mu_stream_t str)
{
  size_t count, i;
  mu_iterator_t itr;
  
  mu_stream_printf (str, "BEGIN IDENT STAT\n");

  mu_assoc_count (nametab, &count);
  mu_stream_printf (str, "N=%zu\n", count);

  if (count > 0)
    {
      int rc = mu_assoc_get_iterator (nametab, &itr);
      if (rc)
	mu_stream_printf (str, "mu_assoc_get_iterator: %s\n",
			  mu_strerror (rc));
      else
	{
	  i = 0;
	  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
	       mu_iterator_next (itr), i++)
	    {
	      const char *key;
	      struct mu_ident_ref *ref;
	      
	      mu_iterator_current_kv (itr,
				      (const void **)&key, (void **)&ref);  
	      mu_stream_printf (str, "%04zu: %s: %zu\n", i, key, ref->count);
	    }
	}
      mu_iterator_destroy (&itr);
    }
  mu_stream_printf (str, "END IDENT STAT\n");
}

