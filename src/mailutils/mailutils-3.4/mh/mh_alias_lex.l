%top {
/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003, 2005-2007, 2009-2012, 2014-2017 Free Software
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
#include <mh.h>
#include <mailutils/locus.h>
#include <mailutils/yyloc.h>
#include <mh_alias_gram.h>
#include <sys/stat.h>
#include <mailutils/cctype.h>
#include <mailutils/io.h>

static mu_linetrack_t trk;
static ino_t ali_source_inode;

int
yyerror (char *s)
{
  mu_error ("%s", s);
  return 0;
}
 
#define xinput() (yyin ? getc(yyin) : EOF)
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)  do { \
        int i;                                  \
        for (i = 0; i < max_size; i++) {        \
                int ch = xinput();              \
                if (ch == EOF)                  \
                        break;                  \
                buf[i] = ch;                    \
        }                                       \
        result = i;                             \
} while (0) 
#define LEX_BUFFER_STATE YY_BUFFER_STATE
#define SET_BUFFER_STATE(s) do { \
        (s) = YY_CURRENT_BUFFER; \
        yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE)); \
} while (0)
#define RESTORE_BUFFER_STATE(s) do { \
        yy_delete_buffer(YY_CURRENT_BUFFER); \
        yy_switch_to_buffer(s); \
} while (0)

#define YY_USER_ACTION							\
  do									\
    {									\
      mu_linetrack_advance (trk, &yylloc, yytext, yyleng);		\
      mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,			\
		       MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &yylloc);	\
    }									\
  while (0);
 
struct buffer_ctx {
  struct buffer_ctx *prev;
  mu_linetrack_t trk;
  ino_t i_node;
  struct mu_locus_range incl_range;
  FILE *yyin;
  int exec_p;
  LEX_BUFFER_STATE state;
};

static struct buffer_ctx *context_stack;
static int exec_p;

static struct buffer_ctx *
ctx_lookup (ino_t ino)
{
  struct buffer_ctx *ctx;

  for (ctx = context_stack; ctx; ctx = ctx->prev)
    if (ctx->i_node == ino)
      break;
  return ctx;
}
        
static int
push_source (const char *name, int fail)
{
  FILE *fp;
  struct buffer_ctx *ctx;
  struct stat st;
  char *filename;
  int ex = 0;
  
  filename = mh_expand_name (NULL, name, NAME_ANY);
  if (stat (filename, &st))
    {
      if (fail)
	mu_error (_("can't stat `%s': %s"), filename, strerror (errno));
      free (filename);
      return 1;
    }
    
  if (yylloc.beg.mu_file && st.st_ino == ali_source_inode)
    {
      mu_error (_("recursive inclusion"));
      free (filename);
      return 1;
    }
  if ((ctx = ctx_lookup (st.st_ino)))
    {
      mu_error (_("recursive inclusion"));
      if (ctx->prev)
	mu_diag_at_locus_range (MU_LOG_ERROR, &ctx->incl_range,
				_("`%s' already included here"),
				filename);
      else
	mu_error (_("`%s' already included at top level"), filename);
      free (filename);
      return 1;
    }
                
  fp = fopen (filename, "r");
  if (!fp)
    {
      mu_error (_("can't open `%s': %s"), filename, strerror (errno));
      free (filename);
      return 1;
    }
  if (access (filename, X_OK) == 0)
    {
      char sig[4];

      if (fread (sig, sizeof(sig), 1, fp) == 1 &&
	  (memcmp(sig, "#!/", 3) == 0 ||
	   memcmp(sig, "#! /", 4) == 0))
	{
	  ex = 1;
	  fclose (fp);
	  fp = popen (filename, "r");
	  if (!fp)
	    {
	      mu_error (_("can't execute `%s': %s"),
			filename, strerror (errno));
	      free (filename);
	      return 1;
	    }
	}
      else
	rewind (fp);
    }
  
  /* Push current context */
  if (yylloc.beg.mu_file)
    {
      ctx = mu_alloc (sizeof (*ctx));
      ctx->trk = trk;
      mu_locus_range_copy (&ctx->incl_range, &yylloc);
      ctx->exec_p = exec_p;
      ctx->i_node = ali_source_inode;
      ctx->yyin = yyin;
      ctx->prev = context_stack;
      context_stack = ctx;
      
      /* Switch to the new context */
      yyin = fp;
      SET_BUFFER_STATE (ctx->state);
    }
  else
    {
#ifdef FLEX_SCANNER
      yyrestart (fp);
#else           
      yyin = fp;
      lex_set_buffer (yyin);
#endif
    }
  MU_ASSERT (mu_linetrack_create (&trk, filename, 2));
  free (filename);
  ali_source_inode = st.st_ino;
  exec_p = ex;
  return 0;
}

static int
pop_source (void)
{
  struct buffer_ctx *ctx;

  if (yyin)
    (exec_p ? pclose : fclose) (yyin);
  mu_linetrack_destroy (&trk);
  if (!context_stack)
    {
      mu_locus_range_deinit (&yylloc);
      yyin = NULL;
      return 1;
    }
  mu_locus_range_deinit (&context_stack->incl_range);
  /* Restore previous context */
  trk = context_stack->trk;
  ali_source_inode = context_stack->i_node;
  exec_p = context_stack->exec_p;
  RESTORE_BUFFER_STATE (context_stack->state);
  ctx = context_stack->prev;
  free (context_stack);
  context_stack = ctx;
  return 0;
}

extern int yyparse (void);

%}
%option nounput
%option noinput

WS [ \t]+
WORD [^ \t\n,:;<+=\*]+
SPEC [,:;+=\*]
%s VERBATIM
%%
\\\n           ;
\n+            return EOL;
^[ \t]*\;.*\n  ; 
^[ \t]*{WORD}\*  { char *p;
                   for (p = yytext; p < yytext + yyleng; p++)
		     if (!mu_isspace (*p))
		       break;
		   yylval.string = mu_strdup (p);
		   return STRING; }
{WS}           ;
{WORD}         { yylval.string = mu_strdup (yytext); return STRING;}
^{WS}?"<"{WS}?{WORD} {
                 char *p;
                 for (p = yytext; p < yytext + yyleng && mu_isblank(*p); p++)
		   ;
                 for (p++; p < yytext + yyleng; p++)
		   if (!mu_isspace (*p))
		     break;
                 push_source (p, 1); }
{SPEC}         return yytext[0];
<VERBATIM>[^ \t\n,:;+=\*][^\n,]* {
                    yylval.string = mu_alloc (yyleng + 1);
 		    memcpy(yylval.string, yytext, yyleng);
		    yylval.string[yyleng] = 0;
		    return STRING;}
.              { mu_error (_("Stray character %03o in alias file"), 
			   yytext[0]); }
%%
		 
int
yywrap (void)
{
  return pop_source ();
}

/* Parses the named alias file */
int
mh_alias_read (char const *name, int fail)
{
  int rc;
  int old_mode, mode;
  extern int yydebug;
  char *p = getenv ("ALI_YYDEBUG");

  if (p && *p > '0' && *p < '9')
    yydebug = 1;
  
  if (push_source (name, fail))
    return 1;
  if (yydebug)
    fprintf (stderr, "Starting parse of %s\n", name);

  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_GET_MODE, &old_mode);
  mode = old_mode | MU_LOGMODE_LOCUS;
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  rc = yyparse ();
  mu_stream_ioctl (mu_strerr, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &old_mode);
  return rc;
}

void
ali_verbatim (int enable)
{
  if (enable)
    BEGIN(VERBATIM);
  else
    BEGIN(INITIAL);
}
  
