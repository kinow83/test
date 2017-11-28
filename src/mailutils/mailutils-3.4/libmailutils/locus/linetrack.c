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
#include <errno.h>
#include <mailutils/types.h>
#include <mailutils/locus.h>
#include <mailutils/error.h>

/* The line-tracker structure keeps track of the last N lines read from one
   or more input files.  For each line read it keeps the number of characters
   in that line including the newline.  This information is stored in a
   cyclic stack of N elements (N >= 2).  Top of stack always represents the
   current line.  For the purpose of line tracker, current line is the line
   that is being visited, such that its final newline character has not yet
   been seen.  Once the newline is seen, the line is pushed on stack, and a
   new current line is assumed.

   Each input file is represented by a directory entry keeping its name,
   number of the first line that is stored in the tracker and the index of
   that line in the cols stack.  Entries form a doubly-linked list, with
   head pointing to the most recent (current) source.  When a new line is
   being added to the stack which is full, its eldest entry is discarded
   and is assigned to that line and the directory of the eldest source is
   updated accordingly.  If the entry represented the only line of the
   source, the source is discarded.   
*/

struct source
{
  char const *file_name; /* Name of the source file */
  size_t idx;            /* Index of the first element on stack */
  unsigned line;         /* Number of line corresponding to cols[idx] */
  struct source *next, *prev;
};

struct mu_linetrack
{
  struct source *s_head, *s_tail;
                         /* Directory of source files.  Most recent one is
			    s_head */

  size_t max_lines;      /* Max. number of lines history kept by tracker (N) */
  size_t head;           /* Index of the eldest element on stack */
  size_t tos;            /* Index of the most recent element on stack
			    (< max_lines) */
  unsigned *cols;        /* Cyclic stack or character counts.
			    Number of characters in line (line + n) is
			    cols[head + n] (0 <= n <= tos). */
};

static inline size_t
trk_incr (struct mu_linetrack *trk, size_t a)
{
  return (a + 1) % trk->max_lines;
}

static inline size_t
trk_decr (struct mu_linetrack *trk, size_t a)
{
  return (a + trk->max_lines - 1) % trk->max_lines;
}
	 
static inline unsigned
count_lines (mu_linetrack_t trk, size_t from)
{
  return (trk->tos + trk->max_lines - from) % trk->max_lines + 1;
}

#ifndef SIZE_MAX
# define SIZE_MAX (~((size_t)0))
#endif

static int
count_chars (struct mu_linetrack *trk, size_t i, size_t *ret)
{
  size_t nch = 0;

  while (1)
    {
      unsigned n = trk->cols[i];
      if (SIZE_MAX - nch < n)
	return ERANGE;
      nch += n;
      if (i == trk->tos)
	break;
      i = trk_incr (trk, i);
    }
  *ret = nch;
  return 0;
}

static size_t
count_files (struct mu_linetrack *trk)
{
  struct source *sp;
  size_t n = 0;
  for (sp = trk->s_head; sp; sp = sp->next)
    n++;
  return n;
}

static void
del_source (mu_linetrack_t trk, struct source *sp)
{
  if (sp->prev)
    sp->prev->next = sp->next;
  else
    trk->s_head = sp->next;
  if (sp->next)
    sp->next->prev = sp->prev;
  else
    trk->s_tail = sp->prev;
  mu_ident_deref (sp->file_name);
  free (sp);
}
  
static inline unsigned *
push (mu_linetrack_t trk)
{
  trk->tos = trk_incr (trk, trk->tos);
  if (trk->tos == trk->head)
    {
      trk->head = trk_incr (trk, trk->head);
      trk->s_tail->idx = trk->head;
      trk->s_tail->line++;
    }
  if (trk->s_tail->prev && trk->s_tail->idx == trk->s_tail->prev->idx)
    del_source (trk, trk->s_tail);
  trk->cols[trk->tos] = 0;
  return &trk->cols[trk->tos];
}

static inline unsigned *
pop (mu_linetrack_t trk)
{
  if (trk->tos == trk->head)
    return NULL;
  if (trk->tos == trk->s_head->idx)
    del_source (trk, trk->s_head);
  
  trk->tos = trk_decr (trk, trk->tos);
  
  return &trk->cols[trk->tos];
}

int
mu_linetrack_origin (mu_linetrack_t trk, struct mu_locus_point const *pt)
{
  int rc;
  struct source *sp;
  char const *file_name;
  
  if (!trk || !pt || pt->mu_line == 0)
    return EINVAL;
  if (pt->mu_file)
    file_name = pt->mu_file;
  else if (trk->s_head)
    file_name = trk->s_head->file_name;
  else
    return EINVAL;
  sp = malloc (sizeof *sp);
  if (!sp)
    return errno;
  rc = mu_ident_ref (file_name, &sp->file_name);
  if (rc)
    {
      free (sp);
      return rc;
    }

  if (trk->cols[trk->tos])
    push (trk);
  
  sp->idx = trk->tos;
  sp->line = pt->mu_line;
  trk->cols[sp->idx] = pt->mu_col;

  sp->prev = NULL;
  sp->next = trk->s_head;
  if (trk->s_head)
    trk->s_head->prev = sp;
  else
    trk->s_tail = sp;
  trk->s_head = sp;
  return 0;
}

int
mu_linetrack_create (mu_linetrack_t *ret,
		     char const *file_name, size_t max_lines)
{
  int rc;
  struct mu_linetrack *trk;
  struct mu_locus_point pt;
  
  trk = malloc (sizeof *trk);
  if (!trk)
    return errno;

  trk->cols = calloc (max_lines, sizeof (trk->cols[0]));
  if (!trk->cols)
    {
      rc = errno;
      free (trk);
      return rc;
    }
  trk->s_head = trk->s_tail = NULL;
  
  if (max_lines < 2)
    max_lines = 2;
  trk->max_lines = max_lines;
  trk->head = 0;
  trk->tos = 0;
  trk->cols[0] = 0;

  pt.mu_file = file_name;
  pt.mu_line = 1;
  pt.mu_col = 0;
  rc = mu_linetrack_origin (trk, &pt);
  if (rc)
    {
      free (trk->cols);
      free (trk);
      return rc;
    }
  
  *ret = trk;
  return 0;
}

int
mu_linetrack_rebase (mu_linetrack_t trk, struct mu_locus_point const *pt)
{
  char const *file_name;
  int rc = mu_ident_ref (pt->mu_file, &file_name);
  if (rc)
    return rc;
  mu_ident_deref (trk->s_head->file_name);
  trk->s_head->file_name = file_name;
  trk->s_head->line = pt->mu_line;
  trk->cols[trk->s_head->idx] = pt->mu_col;
  return 0;
}
  
void
mu_linetrack_free (mu_linetrack_t trk)
{
  if (trk)
    {
      while (trk->s_head)
	del_source (trk, trk->s_head);
      free (trk->cols);
      free (trk);
    }
}

void
mu_linetrack_destroy (mu_linetrack_t *trk)
{
  if (trk)
    {
      mu_linetrack_free (*trk);
      *trk = NULL;
    }
}   

int
mu_linetrack_stat (struct mu_linetrack *trk, struct mu_linetrack_stat *st)
{
  if (count_chars (trk, trk->head, &st->n_chars))
    return ERANGE;
  st->n_files = count_files (trk);
  st->n_lines = count_lines (trk, trk->head);
  return 0;
}

int
mu_linetrack_at_bol (struct mu_linetrack *trk)
{
  return trk->cols[trk->tos] == 0;
}

void
mu_linetrack_advance (struct mu_linetrack *trk,
		      struct mu_locus_range *loc,
		      char const *text, size_t leng)
{
  unsigned *ptr;

  if (text == NULL || leng == 0)
    return;

  mu_locus_point_set_file (&loc->beg, trk->s_head->file_name);
  mu_locus_point_set_file (&loc->end, trk->s_head->file_name);
  loc->beg.mu_line =
    trk->s_head->line + count_lines (trk, trk->s_head->idx) - 1;
  ptr = &trk->cols[trk->tos];
  loc->beg.mu_col = *ptr + 1;
  while (leng--)
    {
      (*ptr)++;
      if (*text == '\n')
	ptr = push (trk);
      text++;
    }

  loc->end.mu_line =
    trk->s_head->line + count_lines (trk, trk->s_head->idx) - 1;
  if (*ptr)
    {
      loc->end.mu_col = *ptr;
    }
  else
    {
      /* Text ends with a newline.  Keep the previous line number. */
      loc->end.mu_line--;
      loc->end.mu_col = trk->cols[trk_decr (trk, trk->tos)] - 1;
      if (loc->end.mu_col + 1 == loc->beg.mu_col)
	{
	  /* This happens if the previous line contained only newline. */
	  loc->beg.mu_col = loc->end.mu_col;
	}	  
   }
}

int
mu_linetrack_locus (struct mu_linetrack *trk, struct mu_locus_point *lp)
{
  int rc = mu_locus_point_set_file (lp, trk->s_head->file_name);
  if (rc == 0)
    {
      lp->mu_line =
	trk->s_head->line + count_lines (trk, trk->s_head->idx) - 1;
      lp->mu_col = trk->cols[trk->tos];
    }
  return rc;
}

int
mu_linetrack_retreat (struct mu_linetrack *trk, size_t n)
{
  size_t nch;

  if (count_chars (trk, trk->head, &nch))
    return ERANGE;
  if (n > nch)
    return ERANGE;
  else
    {
      unsigned *ptr = &trk->cols[trk->tos];
      while (n--)
	{
	  if (*ptr == 0)
	    {
	      ptr = pop (trk);
	      if (!ptr || *ptr == 0)
		{
		  mu_error ("%s:%d: INTERNAL ERROR: out of pop back\n",
			    __FILE__, __LINE__);
		  return ERANGE;
		}
	    }
	  --*ptr;
	}
    }
  return 0;
}
    
  
