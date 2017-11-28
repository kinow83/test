%top {
/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2005, 2007, 2009-2012, 2014-2017 Free Software
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
}

%{
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <mimeview.h>
#include <grammar.h>  
#include <mailutils/io.h>
 
static mu_linetrack_t trk;
struct mu_locus_point string_beg;
 
static mu_opool_t pool;
 
static unsigned 
digit_to_number (char c)
{
  return (unsigned) (c >= '0' && c <= '9' ? c-'0' :
                     c >= 'A' && c <= 'Z' ? c-'A'+10 :
                     c-'a'+10);
}

static void
drop_string (void)
{
  mu_opool_clear (pool);
}
 
static void
finish_string (void)
{
  mu_opool_append_char (pool, 0);
  yylval.string.ptr = mu_opool_finish (pool, &yylval.string.len);
  yylval.string.len--;
  
  mu_locus_point_copy (&yylloc.end, &yylloc.beg);
  yylloc.end.mu_col--;
  mu_locus_point_copy (&yylloc.beg, &string_beg);
  mu_locus_point_deinit (&string_beg);
  
  if (mu_debug_level_p (MU_DEBCAT_APP, MU_DEBUG_TRACE5))
    {
      size_t i;
      mu_debug_log_begin ("string %zu: ", yylval.string.len);
      for (i = 0; i < yylval.string.len; i++)
	if (mu_isprint (yylval.string.ptr[i]))
	  mu_debug_log_cont ("%c", yylval.string.ptr[i]);
        else
	  mu_debug_log_cont ("\\%03o", yylval.string.ptr[i]);
      mu_debug_log_nl ();
    }
#if 0
  YY_LOCATION_PRINT (stderr, yylloc);
  fprintf (stderr, ": %s\n", yylval.string.ptr);
#endif
}  

#define YY_USER_ACTION							\
  do									\
    {									\
      mu_linetrack_advance (trk, &yylloc, yytext, yyleng);		\
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,			\
		       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &yylloc);	\
    }									\
  while (0);

%}

%x RULE ARGS ASTRING
X [0-9a-fA-F]
IDENT [a-zA-Z_\.][a-zA-Z0-9_\.-]*
WS [ \t][ \t]*
%%

<INITIAL>{
     /* Comments */
^#.*\n               ;
\n                   ;
^[^ \t\n/]+"/"[^ \t\n]+ {
  mu_opool_append (pool, yytext, yyleng);
  mu_opool_append_char (pool, 0);
  yylval.string.ptr = mu_opool_finish (pool, &yylval.string.len);
  yylval.string.len--;
  BEGIN (RULE);
  return TYPE;
}

. {
  mu_error (_("type/subtype is missing")); 
  return BOGUS;
}
}

<RULE>{
\\\n                 ;
\n                   {
  BEGIN (INITIAL);
  return EOL;
}
{WS}                 ;

   /* Operators */
"!"|"+"|","|"("|")"|"/"  return yytext[0];
  /* Special cases: && and ||. Docs don't say anything about them, but
     I've found them in my mime.types file...         --Sergey */
"&&"  return '+';
"||"  return ',';

"priority"/"(" {
  return PRIORITY;
}

{IDENT}/"(" {
  mu_opool_append (pool, yytext, yyleng);
  mu_opool_append_char (pool, 0);
  yylval.string.ptr = mu_opool_finish (pool, &yylval.string.len);
  BEGIN (ARGS);
  return IDENT;
} 

[a-zA-Z0-9_.-]+/[^(] {
  mu_opool_append (pool, yytext, yyleng);
  yylval.string.ptr = mu_opool_finish (pool, &yylval.string.len);
  return STRING;
}

. mu_error("unexpected character '%c'", yytext[0]);
}

<ARGS>{
"("|","    return yytext[0];
")"  {
  BEGIN (RULE);
  return yytext[0];
}
{WS} mu_error ("unexpected whitespace in argument list");
\n   {
  mu_error ("unexpected newline in argument list");
  return BOGUS;
}
. {
  mu_locus_point_copy (&string_beg, &yylloc.beg);
  mu_linetrack_retreat (trk, 1); 
  yyless (0);
  BEGIN (ASTRING);
}
}

<ASTRING>{
  /* Quoted string */
\"[^"\n]*\"        {
  mu_opool_append (pool, yytext+1, yyleng-2);
}
"'"[^'\n]*"'" {
  mu_opool_append (pool, yytext+1, yyleng-2);
}
  
  /* Hex string */
"<"({X}{X})+">" {
  int i;
  for (i = 1; i < yyleng - 2; i += 2)
    {
      mu_opool_append_char (pool, digit_to_number (yytext[i])*16
                                  + digit_to_number (yytext[i+1]));
    }  
}

  /* Unquoted character sequence */
[^ \t\n,)<"']+/[^"'<] {
  mu_opool_append (pool, yytext, yyleng);
}

[^ \t\n,)<"]+/< {
  mu_opool_append (pool, yytext, yyleng);
}

[^ \t\n,)<"]+/["'] {
  mu_opool_append (pool, yytext, yyleng);
}

\n   {
  mu_error ("unexpected newline in argument");
  drop_string ();
  return BOGUS;
}

. {
  mu_linetrack_retreat (trk, 1);
  yyless (0);
  BEGIN (ARGS);
  finish_string ();  
  return STRING;
}
}

%%
int
mimetypes_open (const char *name)
{
  struct stat st;
  int mode;
  char *filename;
  
  yy_flex_debug = mu_debug_level_p (MU_DEBCAT_APP, MU_DEBUG_TRACE4);

  if (stat (name, &st))
    {
      mu_error (_("cannot stat `%s': %s"), name, mu_strerror (errno));
      return -1;
    }
  
  if (S_ISDIR (st.st_mode))
    filename = mu_make_file_name (name, "mime.types");
  else
    filename = mu_strdup (name);
  
  yyin = fopen (filename, "r");
  if (!yyin)
    {
      mu_error (_("cannot open `%s': %s"), filename, mu_strerror (errno));
      free (filename);
      return -1;
    }

  MU_ASSERT (mu_linetrack_create (&trk, filename, 3));
  free (filename);
  
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_GET_MODE, &mode);
  mode |= MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);

  mu_opool_create (&pool, MU_OPOOL_ENOMEMABRT);
  return 0;
}

void
mimetypes_close ()
{
  int mode;

  fclose (yyin);
  mu_locus_range_deinit (&yylloc);
  mu_linetrack_destroy (&trk);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_GET_MODE, &mode);
  mode &= ~MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
                   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, NULL);
}

int
yyerror (char *s)
{
  mu_error (s);
  return 0;
}

int
yywrap ()
{
  return 1;
}

struct mimetypes_string *     
mimetypes_string_dup (struct mimetypes_string *s)
{
  mu_opool_append (pool, s, sizeof *s);
  return mu_opool_finish (pool, NULL);
}

void *
mimetypes_malloc (size_t size)
{
  mu_opool_alloc (pool, size);
  return mu_opool_finish (pool, NULL);
}

/* Position input at the beginning of the next rule as a final part of error
   recovery */
void
lex_next_rule (void)
{
  int c;
  int dbg = yy_flex_debug
               || mu_debug_level_p (MU_DEBCAT_APP, MU_DEBUG_TRACE6);
  
  if (dbg)
    {
      YY_LOCATION_PRINT (stderr, yylloc);
      fprintf (stderr, ": started error recovery\n");
    }
  while ((c = input ()) != EOF)
    {
      char ch = c;
      if (!mu_isspace (c) && mu_linetrack_at_bol (trk))
	{
	  unput (c);
	  break;
 	}
      mu_linetrack_advance (trk, &yylloc, &ch, 1);
    }
  if (dbg)
    {
      struct mu_locus_range lr = MU_LOCUS_RANGE_INITIALIZER;
      mu_linetrack_locus (trk, &lr.beg);
      YY_LOCATION_PRINT (stderr, lr);
      fprintf (stderr, ": finished error recovery\n");
      mu_locus_point_deinit (&lr.beg);
    }
  BEGIN (RULE);
  unput ('\n');
  mu_linetrack_retreat (trk, 1);
 }
