%{
/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2002, 2004, 2007, 2009-2012, 2014-2017 Free
   Software Foundation, Inc.

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

#include <mh.h>
#include <mh_format.h>
#include <sys/stat.h>
  
int yyerror (const char *s);
int yylex (void);
 
static mu_opool_t tokpool;     /* Temporary token storage */


/* Lexical context */
enum context
  {
    ctx_init,   /* Normal text */
    ctx_if,     /* After %< or %? */
    ctx_expr,   /* Expression within cond */
    ctx_func,   /* after (func */
  };

static enum context *ctx_stack;
size_t ctx_tos;
size_t ctx_max;
 
static inline void
ctx_push (enum context ctx)
{
  if (ctx_tos == ctx_max)
    ctx_stack = mu_2nrealloc (ctx_stack, &ctx_max, sizeof (ctx_stack[0]));
  ctx_stack[ctx_tos++] = ctx;
}

static inline void
ctx_pop (void)
{
  if (ctx_tos == 0)
    {
      yyerror ("out of context");
      abort ();
    }
  ctx_tos--;
}

static inline enum context
ctx_get (void)
{
  return ctx_stack[ctx_tos-1];
}

enum node_type
{
  fmtnode_print,
  fmtnode_literal,
  fmtnode_number,
  fmtnode_body,
  fmtnode_comp,
  fmtnode_funcall,
  fmtnode_cntl,
  fmtnode_typecast,
};

struct node
{
  enum node_type nodetype;
  enum mh_type datatype;
  int printflag;
  struct node *prev, *next;
  union
  {
    char *str;
    long num;
    struct node *arg;
    struct
    {
      int fmtspec;
      struct node *arg;
    } prt;
    struct
    {
      mh_builtin_t *builtin;
      struct node *arg;
    } funcall;
    struct
    {
      struct node *cond;
      struct node *iftrue;
      struct node *iffalse;
    } cntl;
  } v;
};

static struct node *parse_tree;
static struct node *new_node (enum node_type nodetype, enum mh_type datatype);

static struct node *printelim (struct node *root);
static void codegen (mh_format_t *fmt, int tree);
static struct node *typecast (struct node *node, enum mh_type type);
 
%}

%union {
  char *str;
  char const *mesg;
  long num;
  struct {
    struct node *head, *tail;
  } nodelist;
  struct node *nodeptr;
  mh_builtin_t *builtin;
  int fmtspec;
  struct {
    enum mh_type type;
    union
    {
      char *str;
      long num;
    } v;
  } arg;
};

%token <num> NUMBER "number"
%token <str> STRING "string" COMPONENT "component"
%token <arg> ARGUMENT "argument"
%token <builtin> FUNCTION "function name"
%token IF "%<" ELIF "%?" ELSE "%|" FI "%>"
%token <fmtspec> FMTSPEC "format specifier"
%token BOGUS
%token EOFN ")"

%type <nodelist> list zlist elif_list
%type <nodeptr> item escape component funcall cntl argument
%type <nodeptr> cond cond_expr elif_part else_part printable
%type <builtin> function
%type <fmtspec> fmtspec

%error-verbose

%%

input     : list
            {
	      parse_tree = $1.head;
	    }
          ;

list      : item
            {
	      $$.head = $$.tail = $1;
	    }
          | list item
	    {
	      $2->prev = $1.tail;
	      $1.tail->next = $2;
	      $1.tail = $2;
	      $$ = $1;
	    }
          ;

item      : STRING
            {
	      struct node *n = new_node (fmtnode_literal, mhtype_str);
	      n->v.str = $1;
	      $$ = new_node (fmtnode_print, mhtype_str);
	      $$->v.prt.arg = n;
	    }
          | escape
          ;

escape    : cntl
          | fmtspec printable
            {
	      if ($2->printflag & MHA_NOPRINT)
		$$ = $2;
	      else
		{
		  $$ = new_node (fmtnode_print, $2->datatype);
		  $$->v.prt.fmtspec = ($2->printflag & MHA_IGNOREFMT) ? 0 : $1;
		  $$->v.prt.arg = $2;
		}
	    }
          ;

printable : component
          | funcall
          ;

component : COMPONENT
            {
	      if (mu_c_strcasecmp ($1, "body") == 0)
		$$ = new_node (fmtnode_body, mhtype_str);
	      else
		{
		  $$ = new_node (fmtnode_comp, mhtype_str);
		  $$->v.str = $1;
		}
	    }
          ;

funcall   : function argument EOFN
            {
	      struct node *arg;

	      ctx_pop ();

	      arg = $2;
	      if ($1->argtype == mhtype_none)
		{
		  if (arg)
		    {
		      yyerror ("function doesn't take arguments");
		      YYABORT;
		    }
		}
	      else if (arg == NULL)
		{
		  if ($1->flags & MHA_OPTARG_NIL)
		    {
		      switch ($1->argtype)
			{
			case mhtype_str:
			  arg = new_node (fmtnode_literal, mhtype_str);
			  arg->v.str = "";
			  break;
			  
			case mhtype_num:
			  arg = new_node (fmtnode_number, mhtype_num);
			  arg->v.num = 0;
			  break;
			  
			default:
			  abort ();
			}
		    }
		  else if ($1->flags & MHA_OPTARG)
		    {
		      /* ok - ignore */;
		    }
		  else
		    {
		      yyerror ("required argument missing");
		      YYABORT;
		    }
		}
	      else if ($1->flags & MHA_LITERAL)
		{
		  switch ($1->argtype)
		    {
		    case mhtype_num:
		      if (arg->nodetype == fmtnode_number)
			/* ok */;
		      else
			{
			  yyerror ("argument must be a number");
			  YYABORT;
			}
		      break;

		    case mhtype_str:
		      if (arg->nodetype == fmtnode_literal)
			/* ok */;
		      else if (arg->nodetype == fmtnode_number)
			{
			  char *s;
			  mu_asprintf (&s, "%ld", arg->v.num);
			  arg->nodetype = fmtnode_literal;
			  arg->datatype = mhtype_str;
			  arg->v.str = s;
			}
		      else
			{
			  yyerror ("argument must be literal");
			  YYABORT;
			}
		      break;

		    default:
		      break;
		    }
		}
	      
	      if ($1->flags & MHA_VOID)
		{
		  $2->printflag = MHA_NOPRINT;
		  $$ = $2;
		}
	      else
		{
		  $$ = new_node (fmtnode_funcall, $1->type);
		  $$->v.funcall.builtin = $1;
		  $$->v.funcall.arg = typecast (arg, $1->argtype);
		  $$->printflag = $1->flags & MHA_PRINT_MASK;
		  if ($1->type == mhtype_none)
		    $$->printflag = MHA_NOPRINT;
		}
	    }
          ;

fmtspec   : /* empty */
            {
	      $$ = 0;
	    }
          | FMTSPEC
          ;

function  : FUNCTION
            {
	      ctx_push (ctx_func);
	    }
          ;

argument  : /* empty */
            {
	      $$ = NULL;
	    }
          | ARGUMENT
            {
	      switch ($1.type)
		{
		case mhtype_none:
		  $$ = NULL;
		  break;
		  
		case mhtype_str:
		  $$ = new_node (fmtnode_literal, mhtype_str);
		  $$->v.str = $1.v.str;
		  break;

		case mhtype_num:
		  $$ = new_node (fmtnode_number, mhtype_num);
		  $$->v.num = $1.v.num;
		}
	    }
	  | escape
	    {
	      $$ = printelim ($1);
	    }
          ;

/*           1   2    3       4     5    */
cntl      : if cond zlist elif_part fi
            {
	      $$ = new_node(fmtnode_cntl, mhtype_num);
	      $$->v.cntl.cond = $2;
	      $$->v.cntl.iftrue = $3.head;
	      $$->v.cntl.iffalse = $4;
	    }
          ;

zlist     : /* empty */
            {
	      $$.head = $$.tail = NULL;
	    }
          | list
          ;

if        : IF
            {
	      ctx_push (ctx_if);
	    }
          ;

fi        : FI
            {
	      ctx_pop ();
	    }
          ;

elif      : ELIF
            {
	      ctx_pop ();
	      ctx_push (ctx_if);
	    }
          ;

cond      : cond_expr
            {
	      ctx_pop ();
	      ctx_push (ctx_expr);
	      $$ = printelim ($1);
	    }
          ;

cond_expr : component
          | funcall
          ;

elif_part : /* empty */
            {
	      $$ = NULL;
	    }
          | else_part
          | elif_list
	    {
	      $$ = $1.head;
	    }
          ;

elif_list : elif cond zlist
            {
	      struct node *np = new_node (fmtnode_cntl, mhtype_num);
	      np->v.cntl.cond = $2;
	      np->v.cntl.iftrue = $3.head;
	      np->v.cntl.iffalse = NULL;
	      $$.head = $$.tail = np;
	    }
          | elif_list elif cond zlist
	    {
	      struct node *np = new_node(fmtnode_cntl, mhtype_num);
	      np->v.cntl.cond = $3;
	      np->v.cntl.iftrue = $4.head;
	      np->v.cntl.iffalse = NULL;

	      $1.tail->v.cntl.iffalse = np;
	      $1.tail = np;

	      $$ = $1;
	    }
          | elif_list else_part
	    {
	      $1.tail->v.cntl.iffalse = $2;
	      $1.tail = $2;
	      $$ = $1;
	    }	      
          ;

else_part : ELSE zlist
	    {
	      $$ = $2.head;
	    }
	  ;

%%

static char *start;
static char *tok_start;
static char *curp;
static mu_linetrack_t trk;
static struct mu_locus_range yylloc;

static inline size_t
token_leng (void)
{
  return curp - tok_start;
}

static inline void
mark (void)
{
  if (curp > tok_start)
    mu_linetrack_advance (trk, &yylloc, tok_start, token_leng ());
  tok_start = curp;
}

static inline int
input (void)
{
  if (*curp == 0)
    return 0;
  return *curp++;
}

static inline void
eatinput (size_t n)
{
  mark ();
  while (n--)
    input ();
  mark ();
}

static inline int
peek (void)
{
  return *curp;
}

static inline int
unput (int c)
{
  if (curp == start)
    {
      mu_error (_("%s:%d: INTERNAL ERROR: out of unput space: please report"),
		__FILE__, __LINE__);
      abort ();
    }
  return *--curp = c;
}

static int
skip (int class)
{
  curp = mu_str_skip_class (curp, class);
  return *curp;
}

static int
skipeol (void)
{
  int c;

  do
    {
      c = input ();
      if (c == '\\' && (c = input ()) == '\n')
	c = input ();
    }
  while (c && c != '\n');
  return *curp;
}
	 

static inline int
bogus (const char *mesg)
{
  yylval.mesg = mesg;
  return BOGUS;
}

static char *
find_bol (unsigned line)
{
  char *p = start;

  while (--line)
    {
      while (*p != '\n')
	{
	  if (*p == 0)
	    return p;
	  p++;
	}
      p++;
    }
  return p;
}	      
  
int
yyerror (const char *s)
{
  if (yychar != BOGUS)
    {
      char *bol;
      size_t len;
      static char tab[] = "        ";
      size_t b = 0, e = 0;
      size_t i;
      
      bol = find_bol (yylloc.beg.mu_line);
      len = strcspn (bol, "\n");
      
      mu_diag_at_locus_range (MU_DIAG_ERROR, &yylloc, "%s", s);
      for (i = 0; i < len; i++)
	/* How ... tribal! */
	{
	  if (bol[i] == '\t')
	    {
	      mu_stream_write (mu_strerr, tab, strlen (tab), NULL);
	      if (yylloc.beg.mu_col > i)
		b += strlen (tab) - 1;
	      if (yylloc.end.mu_col > i)
		e += strlen (tab) - 1;
	    }
	  else
	    mu_stream_write (mu_strerr, bol + i, 1, NULL);
	}
      mu_stream_write (mu_strerr, "\n", 1, NULL);
      if (mu_locus_point_eq (&yylloc.beg, &yylloc.end))
	mu_error ("%*.*s^",
		  (int) (b + yylloc.beg.mu_col - 1),
		  (int) (b + yylloc.beg.mu_col - 1), "");
      else
	mu_error ("%*.*s^%*.*s^",
		  (int)(b + yylloc.beg.mu_col - 1),
		  (int)(b + yylloc.beg.mu_col - 1), "",
		  (int)(e + yylloc.end.mu_col - yylloc.beg.mu_col - b - 1),
		  (int)(e + yylloc.end.mu_col - yylloc.beg.mu_col - b - 1),
		  "");
    }
  return 0;
}

static int backslash(int c);

struct lexer_tab
{
  char *ctx_name;
  int (*lexer) (void);
};

static int yylex_initial (void);
static int yylex_cond (void);
static int yylex_expr (void);
static int yylex_func (void);

static struct lexer_tab lexer_tab[] = {
  [ctx_init] = { "initial",    yylex_initial },
  [ctx_if]   = { "condition",  yylex_cond },
  [ctx_expr] = { "expression", yylex_expr },
  [ctx_func] = { "function",   yylex_func }
};
  
int
yylex (void)
{
  int tok;

  do
    {
      mark ();
      if (yydebug)
	fprintf (stderr, "lex: [%s] at %-10.10s...]\n",
		 lexer_tab[ctx_get ()].ctx_name, curp);
      tok = lexer_tab[ctx_get ()].lexer ();
    }
  while (tok == STRING && yylval.str[0] == 0);
  
  mark ();
  if (tok == BOGUS)
    yyerror (yylval.mesg);
  return tok;
}

static int
token_fmtspec (int flags)
{
  int num = 0;
  
  if (peek () == '0')
    {
      flags |= MH_FMT_ZEROPAD;
      input ();
    }
  else if (!mu_isdigit (peek ()))
    {
      return bogus ("expected digit");
    }
  mark ();
  while (*curp && mu_isdigit (peek ()))
    num = num * 10 + input () - '0';
  yylval.fmtspec = flags | num;
  unput ('%');
  return FMTSPEC;
}

static int
token_function (void)
{
  eatinput (1);
  skip (MU_CTYPE_IDENT);
  if (token_leng () == 0 || !strchr (" \t(){%", peek ()))
    {
      return bogus ("expected function name");
    }

  yylval.builtin = mh_lookup_builtin (tok_start, token_leng ());

  if (!yylval.builtin)
    {
      return bogus ("unknown function");
    }
  if (!yylval.builtin->fun
      && !(yylval.builtin->flags & (MHA_SPECIAL|MHA_VOID)))
    {
      mu_error ("INTERNAL ERROR at %s:%d: \"%s\" has no associated function"
		" and is not marked as MHA_SPECIAL",
		__FILE__, __LINE__, yylval.builtin->name);
      abort ();
    }
  
  return FUNCTION;
}
  
static int
token_component (void)
{
  eatinput (1);
  if (!mu_isalpha (peek ()))
    {
      return bogus ("component name expected");
    }
  mark ();
  if (skip (MU_CTYPE_HEADR) != '}')
    {
      return bogus ("component name expected");
    }
  mu_opool_append (tokpool, tok_start, token_leng ());
  mu_opool_append_char (tokpool, 0);
  yylval.str = mu_opool_finish (tokpool, NULL);
  eatinput (1);
  return COMPONENT;
}

int
yylex_initial (void)
{
  int c;

 again:
  mark ();
  if (peek () == '%')
    {
      input ();

      switch (c = input ())
	{
	case ';':
	  skipeol ();
	  goto again;
	case '<':
	  return IF;
	case '%':
	  unput (c);
	  unput (c);
	  break;
	case '(':
	  unput (c);
	  return token_function ();
	case '{':
	  unput (c);
	  return token_component ();
	case '-':
	  return token_fmtspec (MH_FMT_RALIGN);
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	  unput (c);
	  return token_fmtspec (MH_FMT_DEFAULT);
	default:
	  return bogus ("component or function name expected");
      }
    }
  
  c = peek ();
  
  if (c == 0)
    return 0;

  while ((c = input ()) != 0)
    {
      if (c == '%')
	{
	  if (peek () == '%')
	    mu_opool_append_char (tokpool, input ());
	  else
	    {
	      unput (c);
	      break;
	    }
	}
      else if (c == '\\')
	{
	  if ((c = input ()) == 0)
	    {
	      return bogus ("unexpected end of file");
	    }
	  if (c != '\n')
	    mu_opool_append_char (tokpool, backslash (c));
	}
      else
	mu_opool_append_char (tokpool, c);
    }

  mu_opool_append_char (tokpool, 0);
  yylval.str = mu_opool_finish (tokpool, NULL);
  return STRING;
}  

int
yylex_cond (void)
{
  while (1)
    {
      switch (peek ())
	{
	case '(':
	  return token_function ();
	case '{':
	  return token_component ();
	case '\\':
	  input ();
	  if (input () == '\n')
	    continue;
	default:
	  return bogus ("'(' or '{' expected");
	}
    }
}

int
yylex_expr (void)
{
  int c;
  
  if ((c = input ()) == '%')
    {
      switch (c = input ())
	{
	case '?':
	  return ELIF;
	case '|':
	  return ELSE;
	case '>':
	  return FI;
	}
      unput (c);
      unput ('%');
    }
  else
    unput (c);
  return yylex_initial ();
}

int
yylex_func (void)
{
  int c;

  /* Expected argument or closing parenthesis */
 again:
  mark ();
  switch (peek ())
    {
    case '(':
      return token_function ();
      
    case ')':
      eatinput (1);
      return EOFN;
      
    case '{':
      return token_component ();

    case '%':
      input ();
      switch (peek ())
	{
	case '<':
	  input ();
	  return IF;

	case '%':
	  break;

	default:
	  return bogus ("expected '%' or '<'");
	}
      break;

    case ' ':
    case '\t':
      skip (MU_CTYPE_SPACE);
      if (peek () == '%')
	goto again;
      break;

    default:
      return input ();
    }

  mark ();

  while ((c = input ()) != ')')
    {
      if (c == 0)
	{
	  return bogus ("expected ')'");
	}
      
      if (c == '\\')
	{
	  if ((c = input ()) == 0)
	    {
	      return bogus ("unexpected end of file");
	    }
	  mu_opool_append_char (tokpool, backslash (c));
	}
      else
	mu_opool_append_char (tokpool, c);
    }
  mu_opool_append_char (tokpool, 0);

  yylval.arg.v.str = mu_opool_finish (tokpool, NULL);
  yylval.arg.type = mhtype_str;
  unput (c);
  
  if (mu_isdigit (yylval.arg.v.str[0])
      || (yylval.arg.v.str[0] == '-' && mu_isdigit (yylval.arg.v.str[1])))
    {
      long n;
      char *p;
      errno = 0;
      n = strtol (yylval.arg.v.str, &p, 0);
      if (errno == 0 && *p == 0)
	{
	  yylval.arg.type = mhtype_num;
	  yylval.arg.v.num = n;
	}
    }

  if (peek () != ')')
    {
      return bogus ("expected ')'");
    }
  
  return ARGUMENT;
}

static int
format_parse (mh_format_t *fmtptr, char *format_str,
	      struct mu_locus_point const *locus,
	      int flags)
{
  int rc;
  char *p = getenv ("MHFORMAT_DEBUG");
  
  if (p || (flags & MH_FMT_PARSE_DEBUG))
    yydebug = 1;
  start = tok_start = curp = format_str;
  mu_opool_create (&tokpool, MU_OPOOL_ENOMEMABRT);

  ctx_tos = ctx_max = 0;
  ctx_stack = NULL;
  ctx_push (ctx_init);
  mu_linetrack_create (&trk, "input", 2);
  if (locus && locus->mu_file)
    mu_linetrack_rebase (trk, locus);
  mu_locus_range_init (&yylloc);
  
  rc = yyparse ();
  if (rc == 0)
    codegen (fmtptr, flags & MH_FMT_PARSE_TREE);
  else
    mu_opool_destroy (&tokpool);

  mu_locus_range_deinit (&yylloc);
  mu_linetrack_destroy (&trk);
  free (ctx_stack);
  
  parse_tree = NULL;
  tokpool = NULL;
  return rc;
}

int
mh_format_string_parse (mh_format_t *retfmt, char const *format_str,
			struct mu_locus_point const *locus,
			int flags)
{
  char *fmts = mu_strdup (format_str);
  int rc = format_parse (retfmt, fmts, locus, flags);
  free (fmts);
  return rc;
}

int
mh_read_formfile (char const *name, char **pformat)
{
  FILE *fp;
  struct stat st;
  char *format_str;
  char *file_name;
  int rc;
  
  rc = mh_find_file (name, &file_name);
  if (rc)
    {
      mu_error (_("cannot access format file %s: %s"), name, strerror (rc));
      return -1;
    }
  
  if (stat (file_name, &st))
    {
      mu_error (_("cannot stat format file %s: %s"), file_name,
		strerror (errno));
      free (file_name);
      return -1;
    }
  
  fp = fopen (file_name, "r");
  if (!fp)
    {
      mu_error (_("cannot open format file %s: %s"), file_name,
		strerror (errno));
      free (file_name);
      return -1;
    }
  
  format_str = mu_alloc (st.st_size + 1);
  if (fread (format_str, st.st_size, 1, fp) != 1)
    {
      mu_error (_("error reading format file %s: %s"), file_name,
		strerror (errno));
      free (file_name);
      return -1;
    }
  free (file_name);
  
  format_str[st.st_size] = 0;
  if (format_str[st.st_size-1] == '\n')
    format_str[st.st_size-1] = 0;
  fclose (fp);
  *pformat = format_str;
  return 0;
}

int
mh_format_file_parse (mh_format_t *retfmt, char const *formfile, int flags)
{
  char *fmts;
  int rc;
  
  rc = mh_read_formfile (formfile, &fmts);
  if (rc == 0)
    {
      struct mu_locus_point loc;
      loc.mu_file = formfile;
      loc.mu_line = 1;
      loc.mu_col = 0;   
      rc = format_parse (retfmt, fmts, &loc, flags);
      free (fmts);
    }
  return rc;
}

int
backslash (int c)
{
  static char transtab[] = "b\bf\fn\nr\rt\t";
  char *p;
  
  for (p = transtab; *p; p += 2)
    {
      if (*p == c)
	return p[1];
    }
  return c;
}

static struct node *
new_node (enum node_type nodetype, enum mh_type datatype)
{
  struct node *np = mu_zalloc (sizeof *np);
  np->nodetype = nodetype;
  np->datatype = datatype;
  return np;
}

static void node_list_free (struct node *node);

static void
node_free (struct node *node)
{
  if (!node)
    return;
  switch (node->nodetype)
    {
    case fmtnode_print:
      node_free (node->v.prt.arg);
      break;

    case fmtnode_literal:
      break;

    case fmtnode_number:
      break;

    case fmtnode_body:
      break;

    case fmtnode_comp:
      break;

    case fmtnode_funcall:
      node_free (node->v.funcall.arg);
      break;

    case fmtnode_cntl:
      node_list_free (node->v.cntl.cond);
      node_list_free (node->v.cntl.iftrue);
      node_list_free (node->v.cntl.iffalse);
      break;

    default:
      abort ();
    }
  free (node);
}

static void
node_list_free (struct node *node)
{
  while (node)
    {
      struct node *next = node->next;
      node_free (node);
      node = next;
    }
}

static struct node *
typecast (struct node *node, enum mh_type type)
{
  if (!node)
    /* FIXME: when passing optional argument, the caller must know the
       type of value returned by the previous expression */
    return node;
  
  if (node->datatype == type)
    return node;
  switch (node->nodetype)
    {
    case fmtnode_cntl:
      node->v.cntl.iftrue = typecast (node->v.cntl.iftrue, type);
      node->v.cntl.iffalse = typecast (node->v.cntl.iffalse, type);
      node->datatype = type;
      break;

    default:
      {
	struct node *arg = new_node (fmtnode_typecast, type);
	arg->v.arg = node;
	node = arg;
      }
    }
  return node;
}

#define INLINE -1

static inline void
indent (int level)
{
  printf ("%*.*s", 2*level, 2*level, "");
}

static inline void
delim (int level, char const *dstr)
{
  if (level == INLINE)
    printf ("%s", dstr);
  else
    {
      printf ("\n");
      indent (level);
    }
}

static void dump_statement (struct node *node, int level);

void
mh_print_fmtspec (int fmtspec)
{
  if (!(fmtspec & (MH_FMT_RALIGN|MH_FMT_ZEROPAD|MH_FMT_COMPWS)))
    printf ("NONE");
  else
    {
      if (!(fmtspec & MH_FMT_RALIGN))
	printf ("NO");
      printf ("RALIGN|");
      if (!(fmtspec & MH_FMT_ZEROPAD))
	printf ("NO");
      printf ("ZEROPAD|");
      if (!(fmtspec & MH_FMT_COMPWS))
	printf ("NO");
      printf ("COMPWS");
    }
}

static char *typename[] = { "NONE", "NUM", "STR" };

static void
dump_node_pretty (struct node *node, int level)
{
  if (!node)
    return;  
  switch (node->nodetype)
    {
    case fmtnode_print:
      if (node->v.prt.fmtspec)
	{
	  printf ("FORMAT(");
	  mh_print_fmtspec (node->v.prt.fmtspec);
	  printf(", %d, ", node->v.prt.fmtspec & MH_WIDTH_MASK);
	}
      else
	printf ("PRINT(");
      dump_statement (node->v.prt.arg, INLINE);
      printf (")");
      break;
      
    case fmtnode_literal:
      {
	char const *p = node->v.str;
	putchar ('"');
	while (*p)
	  {
	    if (*p == '\\' || *p == '"')
	      {
		putchar ('\\');
		putchar (*p);
	      }
	    else if (*p == '\n')
	      {
		putchar ('\\');
		putchar ('n');
	      }
	    else
	      putchar (*p);
	    p++;
	  }
	putchar ('"');
      }	
      break;
      
    case fmtnode_number:
      printf ("%ld", node->v.num);
      break;
      
    case fmtnode_body:
      printf ("BODY");
      break;
      
    case fmtnode_comp:
      printf ("COMPONENT.%s", node->v.str);
      break;
      
    case fmtnode_funcall:
      printf ("%s(", node->v.funcall.builtin->name);
      dump_statement (node->v.funcall.arg, INLINE);
      printf (")");
      break;
      
    case fmtnode_cntl:
      printf ("IF (");
      dump_node_pretty (node->v.cntl.cond, INLINE);
      printf (") THEN");

      if (level != INLINE)
	level++;
      
      delim (level, "; ");

      dump_statement (node->v.cntl.iftrue, level);

      if (node->v.cntl.iffalse)
	{
	  delim (level == INLINE ? level : level - 1, "; ");
	  printf ("ELSE");
	  delim (level, " ");
	  dump_statement (node->v.cntl.iffalse, level);
	}

      if (level != INLINE)
	level--;
      delim (level, "; ");
      printf ("FI");
      break;

    case fmtnode_typecast:
      printf ("%s(", typename[node->datatype]);
      dump_node_pretty (node->v.arg, INLINE);
      printf (")");
      break;

    default:
      abort ();
    }
}

static void
dump_statement (struct node *node, int level)
{
  while (node)
    {
      dump_node_pretty (node, level);
      node = node->next;
      if (node)
	delim (level, "; ");
    }
}

void
mh_format_dump_code (mh_format_t fmt)
{
  dump_statement (fmt->tree, 0);
  printf ("\n");
}

void
mh_format_free_tree (mh_format_t fmt)
{
  if (fmt)
    {
      node_list_free (fmt->tree);
      fmt->tree = NULL;
      mu_opool_destroy (&fmt->pool);
    }
}

void
mh_format_free (mh_format_t fmt)
{
  if (!fmt)
    return;
  
  mh_format_free_tree (fmt);
    
  if (fmt->prog)
    free (fmt->prog);
  fmt->progmax = fmt->progcnt = 0;
  fmt->prog = NULL;
}

void
mh_format_destroy (mh_format_t *fmt)
{
  if (fmt)
    {
      mh_format_free (*fmt);
      *fmt = NULL;
    }
}

static struct node *
printelim (struct node *node)
{
  if (node->nodetype == fmtnode_print)
    {
      struct node *arg = node->v.prt.arg;
      arg->next = node->next;
      free (node);
      node = arg;
    }
  return node;
}

#define PROG_MIN_ALLOC 8

static inline void
ensure_space (struct mh_format *fmt, size_t n)
{
  while (fmt->progcnt + n >= fmt->progmax)
    {
      if (fmt->progmax == 0)
	fmt->progmax = n < PROG_MIN_ALLOC ? PROG_MIN_ALLOC : n;
      fmt->prog = mu_2nrealloc (fmt->prog, &fmt->progmax, sizeof fmt->prog[0]);
    }
}
  
static void
emit_instr (struct mh_format *fmt, mh_instr_t instr)
{
  ensure_space (fmt, 1);
  fmt->prog[fmt->progcnt++] = instr;
}

static inline void
emit_opcode (struct mh_format *fmt, mh_opcode_t op)
{
  emit_instr (fmt, (mh_instr_t) op);
}

static void
emit_string (struct mh_format *fmt, char const *str)
{
  size_t length = strlen (str) + 1;
  size_t count = (length + sizeof (mh_instr_t)) / sizeof (mh_instr_t) + 1;
  
  ensure_space (fmt, count);
  emit_instr (fmt, (mh_instr_t) count);
  memcpy (MHI_STR (fmt->prog[fmt->progcnt]), str, length);
  fmt->progcnt += count;
}

static void codegen_node (struct mh_format *fmt, struct node *node);
static void codegen_nodelist (struct mh_format *fmt, struct node *node);

static void
emit_opcode_typed (struct mh_format *fmt, enum mh_type type,
		   enum mh_opcode opnum, enum mh_opcode opstr)
{
  switch (type)
    {
    case mhtype_num:
      emit_opcode (fmt, opnum);
      break;

    case mhtype_str:
      emit_opcode (fmt, opstr);
      break;

    default:
      abort ();
    }
}

static void
emit_special (struct mh_format *fmt, mh_builtin_t *builtin, struct node *arg)
{
  if (arg)
    {
      if (builtin->flags & MHA_LITERAL)
	{
	  switch (arg->nodetype)
	    {
	    case fmtnode_literal:
	      emit_opcode (fmt, mhop_sets);
	      emit_instr (fmt, (mh_instr_t) (long) R_REG);
	      emit_string (fmt, arg->v.str);
	      break;

	    case fmtnode_number:
	      emit_opcode (fmt, mhop_setn);
	      emit_instr (fmt, (mh_instr_t) (long) R_REG);
	      emit_instr (fmt, (mh_instr_t) (long) arg->v.num);
	      break;

	    default:
	      abort ();
	    }
	}
      else
	codegen_node (fmt, arg);
    }
}

static void
emit_funcall (struct mh_format *fmt, mh_builtin_t *builtin, struct node *arg)
{
  if (builtin->flags & MHA_ACC)
    {
      emit_opcode (fmt, mhop_movs);
      emit_instr (fmt, (mh_instr_t) (long) R_ACC);
      emit_instr (fmt, (mh_instr_t) (long) R_REG);
    }
  
  if (builtin->flags & MHA_SPECIAL)
    {
      emit_special (fmt, builtin, arg);
      return;
    }

  if (arg)
    {
      if (builtin->flags & MHA_LITERAL)
	{
	  switch (arg->nodetype)
	    {
	    case fmtnode_literal:
	      emit_opcode (fmt, mhop_sets);
	      emit_instr (fmt, (mh_instr_t) (long) R_ARG);
	      emit_string (fmt, arg->v.str);
	      break;

	    case fmtnode_number:
	      emit_opcode (fmt, mhop_setn);
	      emit_instr (fmt, (mh_instr_t) (long) R_ARG);
	      emit_instr (fmt, (mh_instr_t) (long) arg->v.num);
	      break;

	    default:
	      abort ();
	    }
	}
      else
	{
	  codegen_node (fmt, arg);
	  emit_opcode_typed (fmt, arg->datatype, mhop_movn, mhop_movs);
	  emit_instr (fmt, (mh_instr_t) (long) R_ARG);
	  emit_instr (fmt, (mh_instr_t) (long) R_REG);
	}
    }
  else if (builtin->argtype != mhtype_none)
    {
      emit_opcode_typed (fmt, builtin->argtype, mhop_movn, mhop_movs);
      emit_instr (fmt, (mh_instr_t) (long) R_ARG);
      emit_instr (fmt, (mh_instr_t) (long) R_REG);
    }

  emit_opcode (fmt, mhop_call);
  emit_instr (fmt, (mh_instr_t) builtin->fun);
}

static void
codegen_node (struct mh_format *fmt, struct node *node)
{
  if (!node)
    return;
  switch (node->nodetype)
    {
    case fmtnode_print:
      if (node->v.prt.arg->nodetype == fmtnode_literal)
	{
	  emit_opcode (fmt, mhop_printlit);
	  emit_string (fmt, node->v.prt.arg->v.str);
	}
      else if (node->v.prt.arg->nodetype == fmtnode_number)
	{
	  char *s;
	  emit_opcode (fmt, mhop_printlit);
	  mu_asprintf (&s, "%ld", node->v.prt.arg->v.num);
	  emit_string (fmt, s);
	  free (s);
	}
      else
	{
	  codegen_node (fmt, node->v.prt.arg);
	  if (node->v.prt.fmtspec)
	    {
	      emit_opcode (fmt, mhop_fmtspec);
	      emit_instr (fmt, (mh_instr_t) (long) node->v.prt.fmtspec);
	    }
	  
	  if (node->v.prt.arg->datatype != mhtype_none)
	    emit_opcode_typed (fmt, node->v.prt.arg->datatype,
			       mhop_printn, mhop_prints);
	}
      break;

    case fmtnode_literal:
      emit_opcode (fmt, mhop_sets);
      emit_instr (fmt, (mh_instr_t) (long) R_REG);
      emit_string (fmt, node->v.str);
      break;

    case fmtnode_number:
      emit_opcode (fmt, mhop_setn);
      emit_instr (fmt, (mh_instr_t) (long) R_REG);
      emit_instr (fmt, (mh_instr_t) (long) node->v.num);
      break;

    case fmtnode_body:
      emit_opcode (fmt, mhop_ldbody);
      emit_instr (fmt, (mh_instr_t) (long) R_REG);
      break;

    case fmtnode_comp:
      emit_opcode (fmt, mhop_ldcomp);
      emit_instr (fmt, (mh_instr_t) (long) R_REG);
      emit_string (fmt, node->v.str);
      break;

    case fmtnode_funcall:
      emit_funcall (fmt, node->v.funcall.builtin, node->v.funcall.arg);
      break;

    case fmtnode_cntl:
      {
	long pc[2];
	
	/* Implementation of control escapes is a bit tricky. According to
	   the spec:
	   
	     "[f]unction escapes write their return value in 'num' for
	      functions returning integer or boolean values"

	   That means that after "%<(gt 1024)" the value of 'num' would be
	   1 or 0, depending on its value prior to entering the conditional.
	   However this would defeat the purpose of the conditional itself,
	   because then the following construct would be meaningless:

	       %<(gt 1024)...%?(gt 512)...%|...%>

	   Indeed, in MH implementation the value of 'num' propagates into
	   the conditional expression, because any function escape serving
	   as condition is evaluated in a separate context.

	   To ensure this behavior, the virtual machine of GNU MH holds the
	   value of the 'num' register on stack while evaluating the condition
	   and restores it afterward.

	   On the other hand, the spec says that:

 	     "[c]ontrol escapes return a boolean value, setting num to 1
	     if the last explicit condition evaluated by a `%<'  or `%?'
	     control succeeded, and 0 otherwise."

	   To ensure this, the value on top of stack is exchanged with the
	   value of the 'num' register upon entering the 'if' branch, and
	   the tos value is popped into the 'num' upon leaving it. Any
	   'else if' branches are handled the same way.

	   Before leaving the 'else' branch, the 'num' is set to 0 explicitly.
	*/
	emit_opcode (fmt, mhop_pushn);
	codegen_node (fmt, node->v.cntl.cond);
	emit_opcode_typed (fmt, node->v.cntl.cond->datatype,
			   mhop_brzn, mhop_brzs);
	pc[0] = fmt->progcnt;
	emit_instr (fmt, (mh_instr_t) NULL);
	if (node->v.cntl.iftrue)
	  {
	    emit_opcode (fmt, mhop_xchgn);
	    codegen_nodelist (fmt, node->v.cntl.iftrue);
	  }
	emit_opcode (fmt, mhop_popn);

	if (node->v.cntl.iffalse)
	  {
	    emit_opcode (fmt, mhop_branch);
	    pc[1] = fmt->progcnt;
	    emit_instr (fmt, (mh_instr_t) NULL);
	
	    fmt->prog[pc[0]].num = fmt->progcnt - pc[0];
	    emit_opcode (fmt, mhop_popn);
	    codegen_nodelist (fmt, node->v.cntl.iffalse);
	    if (node->v.cntl.iffalse->nodetype != fmtnode_cntl)
	      {
		emit_opcode (fmt, mhop_setn);
		emit_instr (fmt, (mh_instr_t) (long) R_REG);
		emit_instr (fmt, (mh_instr_t) (long) 0);
	      }
	    fmt->prog[pc[1]].num = fmt->progcnt - pc[1];
	  }
	else
	  fmt->prog[pc[0]].num = fmt->progcnt - pc[0];
      }
      break;

    case fmtnode_typecast:
      codegen_node (fmt, node->v.arg);
      switch (node->datatype)
	{
	case mhtype_num:
	  emit_opcode (fmt, mhop_atoi);
	  break;

	case mhtype_str:
	  emit_opcode (fmt, mhop_itoa);
	  break;

	default:
	  abort ();
	}
      break;

    default:
      abort ();
    }
}

static void
codegen_nodelist (struct mh_format *fmt, struct node *node)
{
  while (node)
    {
      codegen_node (fmt, node);
      node = node->next;
    }
}
	
static void
codegen (mh_format_t *fmtptr, int tree)
{
  struct mh_format *fmt;

  fmt = mu_zalloc (sizeof *fmt);
  
  *fmtptr = fmt;
  emit_opcode (fmt, mhop_stop);
  codegen_nodelist (fmt, parse_tree);
  emit_opcode (fmt, mhop_stop);
  
  if (tree)
    {
      fmt->tree = parse_tree;
      fmt->pool = tokpool;
    }
  else
    {
      node_list_free (parse_tree);
      mu_opool_destroy (&tokpool);
    }
}


