/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2007, 2010-2012, 2014-2017 Free Software Foundation,
   Inc.

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

#ifndef _MAILUTILS_ASSOC_H
#define _MAILUTILS_ASSOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mailutils/types.h>

#define MU_ASSOC_COPY_KEY 0x01
#define MU_ASSOC_ICASE    0x02
  
int mu_assoc_create (mu_assoc_t *passoc, int flags);
void mu_assoc_destroy (mu_assoc_t *passoc);
void mu_assoc_clear (mu_assoc_t assoc);
int mu_assoc_lookup (mu_assoc_t assoc, const char *name, void *dataptr);
void *mu_assoc_get (mu_assoc_t assoc, const char *name);
int mu_assoc_install (mu_assoc_t assoc, const char *name, void *value);

int mu_assoc_lookup_ref (mu_assoc_t assoc, const char *name, void *dataptr);
int mu_assoc_install_ref (mu_assoc_t assoc, const char *name, void *pval);
int mu_assoc_install_ref2 (mu_assoc_t assoc, const char *name,
			   void *ret_val, const char **ret_name);
  
int mu_assoc_get_iterator (mu_assoc_t assoc, mu_iterator_t *piterator);
int mu_assoc_remove (mu_assoc_t assoc, const char *name);  
int mu_assoc_set_destroy_item (mu_assoc_t assoc, mu_deallocator_t fn);
int mu_assoc_count (mu_assoc_t assoc, size_t *pcount);
int mu_assoc_is_empty (mu_assoc_t assoc);

typedef int (*mu_assoc_action_t) (char const *, void *, void *);
int mu_assoc_foreach (mu_assoc_t assoc, mu_assoc_action_t action, void *data);

typedef int (*mu_assoc_comparator_t) (const char *, const void *,
				      const char *, const void *,
				      void *);

int mu_assoc_sort_r (mu_assoc_t assoc, mu_assoc_comparator_t cmp, void *data);
  
int mu_assoc_mark (mu_assoc_t asc, int (*cond) (char const *, void *, void *),
		   void *data);
int mu_assoc_sweep (mu_assoc_t asc);

  
#ifdef __cplusplus
}
#endif

#endif /* _MAILUTILS_ASSOC_H */

     
