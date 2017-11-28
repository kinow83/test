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

#ifndef _MAILUTILS_LOCUS_H
#define _MAILUTILS_LOCUS_H

#include <string.h>
#include <stdarg.h>

struct mu_locus_point
{
  char const *mu_file;
  unsigned mu_line;
  unsigned mu_col;
};

#define MU_LOCUS_POINT_INITIALIZER { NULL, 0, 0 }

struct mu_locus_range
{
  struct mu_locus_point beg;
  struct mu_locus_point end;
};

#define MU_LOCUS_RANGE_INITIALIZER \
  { MU_LOCUS_POINT_INITIALIZER, MU_LOCUS_POINT_INITIALIZER }

typedef struct mu_linetrack *mu_linetrack_t;

struct mu_linetrack_stat
{
  size_t n_files; /* Number of source files */
  size_t n_lines; /* Number of lines, including the recent (incomplete) one */
  size_t n_chars; /* Total number of characters */
};
  
int mu_ident_ref (char const *name, char const **refname);
int mu_ident_deref (char const *);
void mu_ident_stat (mu_stream_t str);

int mu_locus_point_set_file (struct mu_locus_point *pt, const char *filename);
void mu_locus_point_init (struct mu_locus_point *pt);
void mu_locus_point_deinit (struct mu_locus_point *pt);
int mu_locus_point_copy (struct mu_locus_point *dest,
			 struct mu_locus_point const *src);

void mu_locus_range_init (struct mu_locus_range *dest);
int mu_locus_range_copy (struct mu_locus_range *dest,
			 struct mu_locus_range const *src);
void mu_locus_range_deinit (struct mu_locus_range *lr);

static inline int
mu_locus_point_same_file (struct mu_locus_point const *a,
			  struct mu_locus_point const *b)
{
  return a->mu_file == b->mu_file
         || (a->mu_file && b->mu_file && strcmp(a->mu_file, b->mu_file) == 0);
}

static inline int
mu_locus_point_same_line (struct mu_locus_point const *a,
			  struct mu_locus_point const *b)
{
  return mu_locus_point_same_file (a, b) && a->mu_line == b->mu_line;
}

static inline int
mu_locus_point_eq (struct mu_locus_point const *a,
		   struct mu_locus_point const *b)
{
  return mu_locus_point_same_line (a, b) && a->mu_col == b->mu_col;
}

int mu_linetrack_create (mu_linetrack_t *ret,
			   char const *file_name, size_t max_lines);
int mu_linetrack_origin (mu_linetrack_t trk, struct mu_locus_point const *pt);
int mu_linetrack_rebase (mu_linetrack_t trk, struct mu_locus_point const *pt);
void mu_linetrack_free (mu_linetrack_t trk);
void mu_linetrack_destroy (mu_linetrack_t *trk);
void mu_linetrack_advance (mu_linetrack_t trk,
			   struct mu_locus_range *loc,
			   char const *text, size_t leng);
int mu_linetrack_retreat (mu_linetrack_t trk, size_t n);

int mu_linetrack_locus (struct mu_linetrack *trk, struct mu_locus_point *lp);
int mu_linetrack_stat (mu_linetrack_t trk, struct mu_linetrack_stat *st);
int mu_linetrack_at_bol (struct mu_linetrack *trk);

void mu_stream_print_locus_point (mu_stream_t stream,
				  struct mu_locus_point const *lpt);
void mu_stream_print_locus_range (mu_stream_t stream,
				  struct mu_locus_range const *loc);

void mu_stream_vlprintf (mu_stream_t stream,
			 struct mu_locus_range const *loc,
			 char const *fmt, va_list ap);
void mu_stream_lprintf (mu_stream_t stream,
			struct mu_locus_range const *loc,
			char const *fmt, ...);
void mu_lrange_debug (struct mu_locus_range const *loc,
		      char const *fmt, ...);


#endif
