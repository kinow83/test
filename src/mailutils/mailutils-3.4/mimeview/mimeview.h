/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2005, 2007, 2010-2012, 2014-2017 Free Software
   Foundation, Inc.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#include <mailutils/mailutils.h>
#include <mailutils/locus.h>
#include <mailutils/yyloc.h>
#include <fnmatch.h>

struct mimetypes_string
{
  char *ptr;
  size_t len;
};

int mimetypes_yylex (void);
int mimetypes_yyerror (char *s);

int mimetypes_open (const char *name);
void mimetypes_close (void);
int mimetypes_parse (const char *name);
void mimetypes_lex_init (void);

void lex_next_rule (void);
void *mimetypes_malloc (size_t size);

struct mimetypes_string *mimetypes_string_dup (struct mimetypes_string *s);

const char *get_file_type (void);

extern char const *mimeview_file;
extern mu_stream_t mimeview_stream;    

