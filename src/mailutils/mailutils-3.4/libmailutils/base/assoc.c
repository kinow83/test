/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2007, 2009-2012, 2014-2017 Free Software Foundation,
   Inc.

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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <mailutils/types.h>
#include <mailutils/assoc.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/iterator.h>
#include <mailutils/util.h>
#include <mailutils/cstr.h>
#include <mailutils/sys/iterator.h>

/* |hash_size| defines a sequence of symbol table sizes. These are prime
   numbers, the distance between each pair of them grows exponentially,
   starting from 64. Hardly someone will need more than 16411 symbols, and
   even if someone will, it is easy enough to add more numbers to the
   sequence. */

static unsigned int hash_size[] = {
    37,   101,  229,  487, 1009, 2039, 4091, 8191, 16411
};

/* |max_rehash| keeps the number of entries in |hash_size| table. */
static unsigned int max_rehash = sizeof (hash_size) / sizeof (hash_size[0]);

struct _mu_assoc_elem
{
  char *name;
  struct _mu_assoc_elem *next, *prev;
  int mark:1;
  char *data;
};

struct _mu_assoc
{
  int flags;
  unsigned int hash_num;  /* Index to hash_size table */
  struct _mu_assoc_elem **tab;
  struct _mu_assoc_elem *head, *tail;
  mu_deallocator_t free;
  mu_iterator_t itr;
};

static void
assoc_elem_unlink (mu_assoc_t assoc, int idx)
{
  struct _mu_assoc_elem *p;

  p = assoc->tab[idx]->prev;
  if (p)
    p->next = assoc->tab[idx]->next;
  else
    assoc->head = assoc->tab[idx]->next;

  p = assoc->tab[idx]->next;
  if (p)
    p->prev = assoc->tab[idx]->prev;
  else
    assoc->tail = assoc->tab[idx]->prev;

  assoc->tab[idx]->prev = assoc->tab[idx]->next = NULL;
}

static void
assoc_elem_link (mu_assoc_t assoc, int idx)
{
  assoc->tab[idx]->next = NULL;
  assoc->tab[idx]->prev = assoc->tail;

  if (assoc->tail)
    assoc->tail->next = assoc->tab[idx];
  else
    assoc->head = assoc->tab[idx];
  assoc->tail = assoc->tab[idx];
}

static unsigned
hash (const char *name, unsigned long hash_num)
{
  unsigned i;
	
  for (i = 0; *name; name++)
    {
      i <<= 1;
      i ^= *(unsigned char*) name;
    }
  return i % hash_size[hash_num];
};

static int assoc_find_slot (mu_assoc_t assoc, const char *name,
			    int *install, unsigned *slot);

static int
assoc_rehash (mu_assoc_t assoc)
{
  struct _mu_assoc_elem **old_tab = assoc->tab;
  struct _mu_assoc_elem **new_tab;
  unsigned int i;
  unsigned int hash_num = assoc->hash_num + 1;
  
  if (hash_num >= max_rehash)
    return MU_ERR_BUFSPACE;

  new_tab = calloc (hash_size[hash_num], sizeof (new_tab[0]));
  if (!new_tab)
    return errno;
  assoc->tab = new_tab;
  if (old_tab)
    {
      assoc->hash_num = hash_num;
      for (i = 0; i < hash_size[hash_num-1]; i++)
	{
	  if (old_tab[i])
	    {
	      int tmp;
	      unsigned slot;
	      int rc = assoc_find_slot (assoc, old_tab[i]->name,
					&tmp, &slot);
	      if (rc)
		return rc;
	      assoc->tab[slot] = old_tab[i];
	    }
	}
      free (old_tab);
    }
  return 0;
}

static void
assoc_free_elem (mu_assoc_t assoc, unsigned idx)
{
  if (assoc->tab[idx])
    {
      assoc_elem_unlink (assoc, idx);
      if (assoc->free)
	assoc->free (assoc->tab[idx]->data);
      if (!(assoc->flags & MU_ASSOC_COPY_KEY))
	free (assoc->tab[idx]->name);
      free (assoc->tab[idx]);
      assoc->tab[idx] = NULL;
    }
}

static int
assoc_remove (mu_assoc_t assoc, unsigned idx)
{
  unsigned int i, j, r;

  if (!(idx < hash_size[assoc->hash_num]))
    return EINVAL;

  mu_iterator_delitem (assoc->itr, assoc->tab[idx]);
  assoc_free_elem (assoc, idx);
  
  for (i = idx;;)
    {
      assoc->tab[i] = NULL;
      j = i;

      do
	{
	  if (++i >= hash_size[assoc->hash_num])
	    i = 0;
	  if (!assoc->tab[i])
	    return 0;
	  r = hash (assoc->tab[i]->name, assoc->hash_num);
	}
      while ((j < r && r <= i) || (i < j && j < r) || (r <= i && i < j));

      if (j != i)
	assoc->tab[j] = assoc->tab[i];
    }
  return 0;
}

#define name_cmp(assoc,a,b) (((assoc)->flags & MU_ASSOC_ICASE) ? \
                             mu_c_strcasecmp(a,b) : strcmp(a,b))

static int
assoc_find_slot (mu_assoc_t assoc, const char *name,
		 int *install, unsigned *slot)
{
  int rc;
  unsigned i, pos;
  struct _mu_assoc_elem *elem;
  
  if (!assoc->tab)
    {
      if (install)
	{
	  rc = assoc_rehash (assoc);
	  if (rc)
	    return rc;
	}
      else
	return MU_ERR_NOENT;
    }

  pos = hash (name, assoc->hash_num);

  for (i = pos; (elem = assoc->tab[i]);)
    {
      if (name_cmp (assoc, elem->name, name) == 0)
	{
	  if (install)
	    *install = 0;
	  *slot = i;
	  return 0;
	}
      
      if (++i >= hash_size[assoc->hash_num])
	i = 0;
      if (i == pos)
	break;
    }

  if (!install)
    return MU_ERR_NOENT;
  
  if (!elem)
    {
      *slot = i;
      *install = 1;
      return 0; 
    }

  if ((rc = assoc_rehash (assoc)) != 0)
    return rc;

  return assoc_find_slot (assoc, name, install, slot);
}

int
mu_assoc_create (mu_assoc_t *passoc, int flags)
{
  mu_assoc_t assoc = calloc (1, sizeof (*assoc));
  if (!assoc)
    return ENOMEM;
  assoc->flags = flags;
  *passoc = assoc;
  return 0;
}

void
mu_assoc_clear (mu_assoc_t assoc)
{
  unsigned i, hs;
  
  if (!assoc || !assoc->tab)
    return;

  hs = hash_size[assoc->hash_num];
  for (i = 0; i < hs; i++)
    assoc_free_elem (assoc, i);
}

void
mu_assoc_destroy (mu_assoc_t *passoc)
{
  mu_assoc_t assoc;
  if (passoc && (assoc = *passoc) != NULL)
    {
      mu_assoc_clear (assoc);
      free (assoc->tab);
      free (assoc);
      *passoc = NULL;
    }
}

int
mu_assoc_set_destroy_item (mu_assoc_t assoc, mu_deallocator_t fn)
{
  if (!assoc)
    return EINVAL;
  assoc->free = fn;
  return 0;
}

int
mu_assoc_lookup (mu_assoc_t assoc, const char *name, void *dataptr)
{
  int rc;
  unsigned idx;
  
  if (!assoc || !name)
    return EINVAL;

  rc = assoc_find_slot (assoc, name, NULL, &idx);
  if (rc == 0)
    {
      if (dataptr)
	*(void**)dataptr = assoc->tab[idx]->data;
    }
  return rc;
}


void *
mu_assoc_get (mu_assoc_t assoc, const char *name)
{
  int rc;
  unsigned idx;
  
  if (!assoc || !name)
    return NULL;

  rc = assoc_find_slot (assoc, name, NULL, &idx);
  if (rc == 0)
    return assoc->tab[idx]->data;
  return NULL;
}

int
mu_assoc_install (mu_assoc_t assoc, const char *name, void *value)
{
  int rc;
  int inst;
  unsigned idx;
  struct _mu_assoc_elem *elem;
  
  if (!assoc || !name)
    return EINVAL;

  rc = assoc_find_slot (assoc, name, &inst, &idx);
  if (rc)
    return rc;
  if (!inst)
    return MU_ERR_EXISTS;

  elem = malloc (sizeof *elem);
  if (!elem)
    return errno;
      
  if (assoc->flags & MU_ASSOC_COPY_KEY)
    elem->name = (char *) name;
  else
    {
      elem->name = strdup (name);
      if (!elem->name)
	{
	  int rc = errno;
	  free (elem);
	  return rc;
	}
    }
  elem->data = value;
  assoc->tab[idx] = elem;
  assoc_elem_link (assoc, idx);
  return 0;
}

int
mu_assoc_lookup_ref (mu_assoc_t assoc, const char *name, void *dataptr)
{
  int rc;
  unsigned idx;
  
  if (!assoc || !name)
    return EINVAL;

  rc = assoc_find_slot (assoc, name, NULL, &idx);
  if (rc == 0)
    {
      if (dataptr)
	*(void**)dataptr = &assoc->tab[idx]->data;
    }
  return rc;
}

int
mu_assoc_install_ref2 (mu_assoc_t assoc, const char *name,
		       void *ret_val,
		       const char **ret_name)
{
  int rc;
  int inst;
  unsigned idx;

  if (!assoc || !name)
    return EINVAL;
  
  rc = assoc_find_slot (assoc, name, &inst, &idx);
  if (rc)
    return rc;

  if (inst)
    {
      struct _mu_assoc_elem *elem;
      
      elem = malloc (sizeof *elem);
      if (!elem)
	return errno;
      
      if (assoc->flags & MU_ASSOC_COPY_KEY)
	elem->name = (char *) name;
      else
	{
	  elem->name = strdup (name);
	  if (!elem->name)
	    {
	      int rc = errno;
	      free (elem);
	      return rc;
	    }
	}
      elem->data = NULL;
      assoc->tab[idx] = elem;
      assoc_elem_link (assoc, idx);
    }
  
  *(void**)ret_val = &assoc->tab[idx]->data;
  if (ret_name)
    *ret_name = assoc->tab[idx]->name;
  
  return inst ? 0 : MU_ERR_EXISTS;
}  

int
mu_assoc_install_ref (mu_assoc_t assoc, const char *name, void *pval)
{
  return mu_assoc_install_ref2 (assoc, name, pval, NULL);
}

int
mu_assoc_remove (mu_assoc_t assoc, const char *name)
{
  int rc;
  unsigned idx;

  if (!assoc || !name)
    return EINVAL;
  rc = assoc_find_slot (assoc, name, NULL, &idx);
  if (rc)
    return rc;
  return assoc_remove (assoc, idx);
}

/* Iterator interface */

struct assoc_iterator
{
  mu_assoc_t assoc;
  struct _mu_assoc_elem *elem;
  int backwards; /* true if iterating backwards */
};

static int
itrctl (void *owner, enum mu_itrctl_req req, void *arg)
{
  struct assoc_iterator *itr = owner;
  mu_assoc_t assoc = itr->assoc;

  switch (req)
    {
    case mu_itrctl_tell:
      /* Return current position in the object */
      {
	size_t n = 0;
	struct _mu_assoc_elem *elem;

	for (elem = itr->elem; elem; elem = elem->prev)
	  n++;
	*(size_t*)arg = n;
      }
      break;

    case mu_itrctl_delete:
    case mu_itrctl_delete_nd:
      /* Delete current element */
      if (itr->elem)
	{
	  unsigned i;

	  for (i = 0; i < hash_size[assoc->hash_num]; i++)
	    {
	      if (assoc->tab[i] == itr->elem)
		{
		  if (req == mu_itrctl_delete_nd)
		    assoc->tab[i]->data = NULL;
		  assoc_remove (assoc, i);
		  return 0;
		}
	    }
	}
      return MU_ERR_NOENT;

    case mu_itrctl_replace:
    case mu_itrctl_replace_nd:
      /* Replace current element */
      if (itr->elem == NULL)
	return MU_ERR_NOENT;
      if (req == mu_itrctl_replace && assoc->free)
	assoc->free (itr->elem->data);
      itr->elem->data = arg;
      break;
      
    case mu_itrctl_qry_direction:
      if (!arg)
	return EINVAL;
      else
	*(int*)arg = itr->backwards;
      break;

    case mu_itrctl_set_direction:
      if (!arg)
	return EINVAL;
      else
	itr->backwards = !!*(int*)arg;
      break;

    case mu_itrctl_count:
      if (!arg)
	return EINVAL;
      return mu_assoc_count (assoc, arg);
      
    default:
      return ENOSYS;
    }
  return 0;
}

static int
first (void *owner)
{
  struct assoc_iterator *itr = owner;
  mu_assoc_t assoc = itr->assoc;
  itr->elem = itr->backwards ? assoc->tail : assoc->head;
  return 0;
}

static int
next (void *owner)
{
  struct assoc_iterator *itr = owner;
  itr->elem = itr->backwards ? itr->elem->prev : itr->elem->next;
  return 0;
}

static int
getitem (void *owner, void **pret, const void **pkey)
{
  struct assoc_iterator *itr = owner;

  if (!itr->elem)
    return EINVAL;
  *pret = itr->elem->data;
  if (pkey)
    *pkey = itr->elem->name;
  return 0;
}

static int
finished_p (void *owner)
{
  struct assoc_iterator *itr = owner;
  return itr->elem == NULL;
}

static int
destroy (mu_iterator_t iterator, void *data)
{
  struct assoc_iterator *itr = data;
  mu_iterator_detach (&itr->assoc->itr, iterator);
  free (data);
  return 0;
}

static int
delitem (void *owner, void *item)
{
  struct assoc_iterator *itr = owner;
  return itr->elem == item ? MU_ITR_DELITEM_NEXT : MU_ITR_DELITEM_NOTHING;
}

static int
assoc_data_dup (void **ptr, void *owner)
{
  *ptr = malloc (sizeof (struct assoc_iterator));
  if (*ptr == NULL)
    return ENOMEM;
  memcpy (*ptr, owner, sizeof (struct assoc_iterator));
  return 0;
}

int
mu_assoc_get_iterator (mu_assoc_t assoc, mu_iterator_t *piterator)
{
  mu_iterator_t iterator;
  int status;
  struct assoc_iterator *itr;

  if (!assoc)
    return EINVAL;

  itr = calloc (1, sizeof *itr);
  if (!itr)
    return ENOMEM;
  itr->assoc = assoc;
  itr->elem = NULL;

  status = mu_iterator_create (&iterator, itr);
  if (status)
    {
      free (itr);
      return status;
    }

  mu_iterator_set_first (iterator, first);
  mu_iterator_set_next (iterator, next);
  mu_iterator_set_getitem (iterator, getitem);
  mu_iterator_set_finished_p (iterator, finished_p);
  mu_iterator_set_delitem (iterator, delitem);
  mu_iterator_set_destroy (iterator, destroy);
  mu_iterator_set_dup (iterator, assoc_data_dup);
  mu_iterator_set_itrctl (iterator, itrctl);
  
  mu_iterator_attach (&assoc->itr, iterator);

  *piterator = iterator;
  return 0;
}  

int
mu_assoc_count (mu_assoc_t assoc, size_t *pcount)
{
  size_t length = 0;

  if (!pcount)
    return MU_ERR_OUT_PTR_NULL;
  if (assoc)
    {
      struct _mu_assoc_elem *elt;
      for (elt = assoc->head; elt; elt = elt->next)
	length++;
    }
  *pcount = length;
  return 0;
}

int
mu_assoc_is_empty (mu_assoc_t assoc)
{
  return assoc == NULL || assoc->head == NULL;
}

int
mu_assoc_foreach (mu_assoc_t assoc, mu_assoc_action_t action, void *data)
{
  mu_iterator_t itr;
  int rc;
  
  if (!assoc || !action)
    return EINVAL;
  rc = mu_assoc_get_iterator (assoc, &itr);
  if (rc)
    return rc;
  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      char *name;
      void *value;

      rc = mu_iterator_current_kv (itr, (const void **)&name, (void**)&value);
      if (rc)
	break;
      
      rc = action (name, value, data);
      if (rc)
	break;
    }
  mu_iterator_destroy (&itr);
  return rc;
}

/* Merges the two NULL-terminated lists, LEFT and RIGHT, using CMP for
   element comparison.  DATA supplies call-specific data for CMP.

   Both LEFT and RIGHT are treated as singly-linked lists, with NEXT pointing
   to the successive element.  PREV pointers are ignored.
   
   Returns the resulting list.
 */
static struct _mu_assoc_elem *
merge (struct _mu_assoc_elem *left, struct _mu_assoc_elem *right,
       mu_assoc_comparator_t cmp, void *data)
{
  struct _mu_assoc_elem *head = NULL, **tailptr = &head, *tmp;

  while (left && right)
    {
      if (cmp (left->name, left->data, right->name, right->data, data) <= 0)
	{
	  tmp = left->next;
	  *tailptr = left;
	  tailptr = &left->next;
	  left = tmp;
	}
      else
	{
	  tmp = right->next;
	  *tailptr = right;
	  tailptr = &right->next;
	  right = tmp;
	}
    }

  *tailptr = left ? left : right;

  return head;
}

/* Sort the singly-linked LIST of LENGTH elements using merge sort algorithm.
   Elements are compared using the CMP function with DATA pointing to
   call-specific data.
   The elements of LIST are linked by the NEXT pointer.  The NEXT pointer of
   the last element (LIST[LENGTH], 1-based) must be NULL.

   Returns the resulting list.

   Side-effects: PREV pointers in the returned list are messed up.
*/
static struct _mu_assoc_elem *
merge_sort (struct _mu_assoc_elem *list, size_t length,
	    mu_assoc_comparator_t cmp, void *data)
{
  struct _mu_assoc_elem *left, *right;
  size_t left_len, right_len, i;
  struct _mu_assoc_elem *elt;
  
  if (length == 1)
    return list;

  if (length == 2)
    {
      elt = list->next;
      if (cmp (list->name, list->data, elt->name, elt->data, data) > 0)
	{
	  elt->next = list;
	  list->next = NULL;
	  return elt;
	}
      return list;
    }

  left = list;
  left_len = (length + 1) / 2;

  right_len = length / 2;
  for (elt = list, i = left_len - 1; i; i--)
    elt = elt->next;

  right = elt->next;
  elt->next = NULL;

  left = merge_sort (left, left_len, cmp, data);
  right = merge_sort (right, right_len, cmp, data);

  return merge (left, right, cmp, data);
}

/* Sort the linked list of elements in ASSOC using merge sort.  CMP points
   to the function to use for comparing two elements.  DATA supplies call-
   specific data for CMP.
*/
int
mu_assoc_sort_r (mu_assoc_t assoc, mu_assoc_comparator_t cmp, void *data)
{
  struct _mu_assoc_elem *head, *prev, *p;
  size_t length;
  
  if (!assoc)
    return EINVAL;
  if (!cmp)
    return 0;

  mu_assoc_count (assoc, &length);
  head = merge_sort (assoc->head, length, cmp, data);
  /* The above call leaves PREV pointers in inconsistent state.  Fix them
     up: */
  for (prev = NULL, p = head; p; prev = p, p = p->next)
    p->prev = prev;

  /* Update list head and tail */
  assoc->head = head;
  assoc->tail = prev;

  return 0;
}

int
mu_assoc_mark (mu_assoc_t asc, int (*cond) (char const *, void *, void *),
	       void *data)
{
  struct _mu_assoc_elem *elt;

  if (!asc)
    return EINVAL;
  for (elt = asc->head; elt; elt = elt->next)
    elt->mark = !!cond (elt->name, elt->data, data);
  return 0;
}

int
mu_assoc_sweep (mu_assoc_t asc)
{
  unsigned i;
  
  if (!asc)
    return EINVAL;

  for (i = hash_size[asc->hash_num]; i > 0; i--)
    {
      if (asc->tab[i-1] && asc->tab[i-1]->mark)
	assoc_remove (asc, i-1);
    }
  
  return 0;
}

