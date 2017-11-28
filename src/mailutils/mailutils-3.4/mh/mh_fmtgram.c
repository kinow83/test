/* A Bison parser, made by GNU Bison 2.5.1.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "mh_fmtgram.y"

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
  
int fmt_yyerror (const char *s);
int fmt_yylex (void);
 
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
      fmt_yyerror ("out of context");
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
 


/* Line 268 of yacc.c  */
#line 189 "mh_fmtgram.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     STRING = 259,
     COMPONENT = 260,
     ARGUMENT = 261,
     FUNCTION = 262,
     IF = 263,
     ELIF = 264,
     ELSE = 265,
     FI = 266,
     FMTSPEC = 267,
     BOGUS = 268,
     EOFN = 269
   };
#endif
/* Tokens.  */
#define NUMBER 258
#define STRING 259
#define COMPONENT 260
#define ARGUMENT 261
#define FUNCTION 262
#define IF 263
#define ELIF 264
#define ELSE 265
#define FI 266
#define FMTSPEC 267
#define BOGUS 268
#define EOFN 269




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 295 of yacc.c  */
#line 118 "mh_fmtgram.y"

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



/* Line 295 of yacc.c  */
#line 283 "mh_fmtgram.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 345 of yacc.c  */
#line 295 "mh_fmtgram.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union fmt_yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union fmt_yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   36

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  15
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  21
/* YYNRULES -- Number of rules.  */
#define YYNRULES  34
/* YYNRULES -- Number of states.  */
#define YYNSTATES  44

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   269

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    19,
      21,    23,    25,    29,    30,    32,    34,    35,    37,    39,
      45,    46,    48,    50,    52,    54,    56,    58,    60,    61,
      63,    65,    69,    74,    77
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      16,     0,    -1,    17,    -1,    18,    -1,    17,    18,    -1,
       4,    -1,    19,    -1,    26,    -1,    23,    20,    -1,    21,
      -1,    22,    -1,     5,    -1,    24,    25,    14,    -1,    -1,
      12,    -1,     7,    -1,    -1,     6,    -1,    19,    -1,    28,
      31,    27,    33,    29,    -1,    -1,    17,    -1,     8,    -1,
      11,    -1,     9,    -1,    32,    -1,    21,    -1,    22,    -1,
      -1,    35,    -1,    34,    -1,    30,    31,    27,    -1,    34,
      30,    31,    27,    -1,    34,    35,    -1,    10,    27,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   157,   157,   163,   167,   176,   183,   186,   187,   200,
     201,   204,   216,   316,   319,   322,   329,   332,   350,   357,
     367,   370,   373,   379,   385,   392,   400,   401,   405,   408,
     409,   415,   423,   435,   443
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "\"number\"", "\"string\"",
  "\"component\"", "\"argument\"", "\"function name\"", "\"%<\"", "\"%?\"",
  "\"%|\"", "\"%>\"", "\"format specifier\"", "BOGUS", "\")\"", "$accept",
  "input", "list", "item", "escape", "printable", "component", "funcall",
  "fmtspec", "function", "argument", "cntl", "zlist", "if", "fi", "elif",
  "cond", "cond_expr", "elif_part", "elif_list", "else_part", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 fmt_yyr1[] =
{
       0,    15,    16,    17,    17,    18,    18,    19,    19,    20,
      20,    21,    22,    23,    23,    24,    25,    25,    25,    26,
      27,    27,    28,    29,    30,    31,    32,    32,    33,    33,
      33,    34,    34,    34,    35
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 fmt_yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     2,     1,
       1,     1,     3,     0,     1,     1,     0,     1,     1,     5,
       0,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     3,     4,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      13,     5,    22,    14,     0,    13,     3,     6,     0,     7,
       0,     1,     4,    11,    15,     8,     9,    10,    13,    26,
      27,    20,    25,    17,    18,     0,    21,    28,    12,    24,
      20,     0,     0,    30,    29,    34,    20,    23,    19,     0,
      33,    31,    20,    32
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,    26,     6,     7,    15,    19,    20,     8,    18,
      25,     9,    27,    10,    38,    31,    21,    22,    32,    33,
      34
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -29
static const yytype_int8 fmt_yypact[] =
{
      11,   -29,   -29,   -29,    24,     1,   -29,   -29,    15,   -29,
      15,   -29,   -29,   -29,   -29,   -29,   -29,   -29,     4,   -29,
     -29,    -1,   -29,   -29,   -29,    14,    -1,    17,   -29,   -29,
      -1,    15,    18,    17,   -29,   -29,    -1,   -29,   -29,    15,
     -29,   -29,    -1,   -29
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -29,   -29,    30,    -5,    13,   -29,    25,    26,   -29,   -29,
     -29,   -29,   -28,   -29,   -29,     2,   -14,   -29,   -29,   -29,
       3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -17
static const yytype_int8 fmt_yytable[] =
{
      12,    -2,    35,     1,   -13,     1,   -13,     2,    41,     2,
      23,     3,     2,     3,    43,     1,     3,    36,   -16,     2,
      13,    12,    14,     3,    11,    42,    29,    30,    28,    37,
       5,    24,     0,    16,    17,    39,    40
};

#define yypact_value_is_default(fmt_yystate) \
  ((fmt_yystate) == (-29))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int8 fmt_yycheck[] =
{
       5,     0,    30,     4,     5,     4,     7,     8,    36,     8,
       6,    12,     8,    12,    42,     4,    12,    31,    14,     8,
       5,    26,     7,    12,     0,    39,     9,    10,    14,    11,
       0,    18,    -1,     8,     8,    33,    33
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,     8,    12,    16,    17,    18,    19,    23,    26,
      28,     0,    18,     5,     7,    20,    21,    22,    24,    21,
      22,    31,    32,     6,    19,    25,    17,    27,    14,     9,
      10,    30,    33,    34,    35,    27,    31,    11,    29,    30,
      35,    27,    31,    27
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(fmt_yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call fmt_yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (fmt_yychar == YYEMPTY)                                        \
    {                                                           \
      fmt_yychar = (Token);                                         \
      fmt_yylval = (Value);                                         \
      YYPOPSTACK (fmt_yylen);                                       \
      fmt_yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      fmt_yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `fmt_yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX fmt_yylex (YYLEX_PARAM)
#else
# define YYLEX fmt_yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (fmt_yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (fmt_yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (fmt_yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int fmt_yyrule)
#else
static void
yy_reduce_print (yyvsp, fmt_yyrule)
    YYSTYPE *yyvsp;
    int fmt_yyrule;
#endif
{
  int yynrhs = fmt_yyr2[fmt_yyrule];
  int yyi;
  unsigned long int yylno = yyrline[fmt_yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     fmt_yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[fmt_yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (fmt_yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int fmt_yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T fmt_yylen;
  for (fmt_yylen = 0; yystr[fmt_yylen]; fmt_yylen++)
    continue;
  return fmt_yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *fmt_yys = yysrc;

  while ((*yyd++ = *fmt_yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for fmt_yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in fmt_yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated fmt_yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = fmt_yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both fmt_yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (fmt_yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (fmt_yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int fmt_yyparse (void *YYPARSE_PARAM);
#else
int fmt_yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int fmt_yyparse (void);
#else
int fmt_yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int fmt_yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE fmt_yylval;

/* Number of syntax errors so far.  */
int fmt_yynerrs;


/*----------.
| fmt_yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
fmt_yyparse (void *YYPARSE_PARAM)
#else
int
fmt_yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
fmt_yyparse (void)
#else
int
fmt_yyparse ()

#endif
#endif
{
    int fmt_yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE fmt_yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int fmt_yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  fmt_yystate = 0;
  yyerrstatus = 0;
  fmt_yynerrs = 0;
  fmt_yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in fmt_yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = fmt_yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union fmt_yyalloc *yyptr =
	  (union fmt_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", fmt_yystate));

  if (fmt_yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = fmt_yypact[fmt_yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (fmt_yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      fmt_yychar = YYLEX;
    }

  if (fmt_yychar <= YYEOF)
    {
      fmt_yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (fmt_yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &fmt_yylval, &fmt_yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || fmt_yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = fmt_yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &fmt_yylval, &fmt_yylloc);

  /* Discard the shifted token.  */
  fmt_yychar = YYEMPTY;

  fmt_yystate = yyn;
  *++yyvsp = fmt_yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[fmt_yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  fmt_yylen = fmt_yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  fmt_yyval = yyvsp[1-fmt_yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1810 of yacc.c  */
#line 158 "mh_fmtgram.y"
    {
	      parse_tree = (yyvsp[(1) - (1)].nodelist).head;
	    }
    break;

  case 3:

/* Line 1810 of yacc.c  */
#line 164 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodelist).head = (fmt_yyval.nodelist).tail = (yyvsp[(1) - (1)].nodeptr);
	    }
    break;

  case 4:

/* Line 1810 of yacc.c  */
#line 168 "mh_fmtgram.y"
    {
	      (yyvsp[(2) - (2)].nodeptr)->prev = (yyvsp[(1) - (2)].nodelist).tail;
	      (yyvsp[(1) - (2)].nodelist).tail->next = (yyvsp[(2) - (2)].nodeptr);
	      (yyvsp[(1) - (2)].nodelist).tail = (yyvsp[(2) - (2)].nodeptr);
	      (fmt_yyval.nodelist) = (yyvsp[(1) - (2)].nodelist);
	    }
    break;

  case 5:

/* Line 1810 of yacc.c  */
#line 177 "mh_fmtgram.y"
    {
	      struct node *n = new_node (fmtnode_literal, mhtype_str);
	      n->v.str = (yyvsp[(1) - (1)].str);
	      (fmt_yyval.nodeptr) = new_node (fmtnode_print, mhtype_str);
	      (fmt_yyval.nodeptr)->v.prt.arg = n;
	    }
    break;

  case 8:

/* Line 1810 of yacc.c  */
#line 188 "mh_fmtgram.y"
    {
	      if ((yyvsp[(2) - (2)].nodeptr)->printflag & MHA_NOPRINT)
		(fmt_yyval.nodeptr) = (yyvsp[(2) - (2)].nodeptr);
	      else
		{
		  (fmt_yyval.nodeptr) = new_node (fmtnode_print, (yyvsp[(2) - (2)].nodeptr)->datatype);
		  (fmt_yyval.nodeptr)->v.prt.fmtspec = ((yyvsp[(2) - (2)].nodeptr)->printflag & MHA_IGNOREFMT) ? 0 : (yyvsp[(1) - (2)].fmtspec);
		  (fmt_yyval.nodeptr)->v.prt.arg = (yyvsp[(2) - (2)].nodeptr);
		}
	    }
    break;

  case 11:

/* Line 1810 of yacc.c  */
#line 205 "mh_fmtgram.y"
    {
	      if (mu_c_strcasecmp ((yyvsp[(1) - (1)].str), "body") == 0)
		(fmt_yyval.nodeptr) = new_node (fmtnode_body, mhtype_str);
	      else
		{
		  (fmt_yyval.nodeptr) = new_node (fmtnode_comp, mhtype_str);
		  (fmt_yyval.nodeptr)->v.str = (yyvsp[(1) - (1)].str);
		}
	    }
    break;

  case 12:

/* Line 1810 of yacc.c  */
#line 217 "mh_fmtgram.y"
    {
	      struct node *arg;

	      ctx_pop ();

	      arg = (yyvsp[(2) - (3)].nodeptr);
	      if ((yyvsp[(1) - (3)].builtin)->argtype == mhtype_none)
		{
		  if (arg)
		    {
		      fmt_yyerror ("function doesn't take arguments");
		      YYABORT;
		    }
		}
	      else if (arg == NULL)
		{
		  if ((yyvsp[(1) - (3)].builtin)->flags & MHA_OPTARG_NIL)
		    {
		      switch ((yyvsp[(1) - (3)].builtin)->argtype)
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
		  else if ((yyvsp[(1) - (3)].builtin)->flags & MHA_OPTARG)
		    {
		      /* ok - ignore */;
		    }
		  else
		    {
		      fmt_yyerror ("required argument missing");
		      YYABORT;
		    }
		}
	      else if ((yyvsp[(1) - (3)].builtin)->flags & MHA_LITERAL)
		{
		  switch ((yyvsp[(1) - (3)].builtin)->argtype)
		    {
		    case mhtype_num:
		      if (arg->nodetype == fmtnode_number)
			/* ok */;
		      else
			{
			  fmt_yyerror ("argument must be a number");
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
			  fmt_yyerror ("argument must be literal");
			  YYABORT;
			}
		      break;

		    default:
		      break;
		    }
		}
	      
	      if ((yyvsp[(1) - (3)].builtin)->flags & MHA_VOID)
		{
		  (yyvsp[(2) - (3)].nodeptr)->printflag = MHA_NOPRINT;
		  (fmt_yyval.nodeptr) = (yyvsp[(2) - (3)].nodeptr);
		}
	      else
		{
		  (fmt_yyval.nodeptr) = new_node (fmtnode_funcall, (yyvsp[(1) - (3)].builtin)->type);
		  (fmt_yyval.nodeptr)->v.funcall.builtin = (yyvsp[(1) - (3)].builtin);
		  (fmt_yyval.nodeptr)->v.funcall.arg = typecast (arg, (yyvsp[(1) - (3)].builtin)->argtype);
		  (fmt_yyval.nodeptr)->printflag = (yyvsp[(1) - (3)].builtin)->flags & MHA_PRINT_MASK;
		  if ((yyvsp[(1) - (3)].builtin)->type == mhtype_none)
		    (fmt_yyval.nodeptr)->printflag = MHA_NOPRINT;
		}
	    }
    break;

  case 13:

/* Line 1810 of yacc.c  */
#line 316 "mh_fmtgram.y"
    {
	      (fmt_yyval.fmtspec) = 0;
	    }
    break;

  case 15:

/* Line 1810 of yacc.c  */
#line 323 "mh_fmtgram.y"
    {
	      ctx_push (ctx_func);
	    }
    break;

  case 16:

/* Line 1810 of yacc.c  */
#line 329 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodeptr) = NULL;
	    }
    break;

  case 17:

/* Line 1810 of yacc.c  */
#line 333 "mh_fmtgram.y"
    {
	      switch ((yyvsp[(1) - (1)].arg).type)
		{
		case mhtype_none:
		  (fmt_yyval.nodeptr) = NULL;
		  break;
		  
		case mhtype_str:
		  (fmt_yyval.nodeptr) = new_node (fmtnode_literal, mhtype_str);
		  (fmt_yyval.nodeptr)->v.str = (yyvsp[(1) - (1)].arg).v.str;
		  break;

		case mhtype_num:
		  (fmt_yyval.nodeptr) = new_node (fmtnode_number, mhtype_num);
		  (fmt_yyval.nodeptr)->v.num = (yyvsp[(1) - (1)].arg).v.num;
		}
	    }
    break;

  case 18:

/* Line 1810 of yacc.c  */
#line 351 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodeptr) = printelim ((yyvsp[(1) - (1)].nodeptr));
	    }
    break;

  case 19:

/* Line 1810 of yacc.c  */
#line 358 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodeptr) = new_node(fmtnode_cntl, mhtype_num);
	      (fmt_yyval.nodeptr)->v.cntl.cond = (yyvsp[(2) - (5)].nodeptr);
	      (fmt_yyval.nodeptr)->v.cntl.iftrue = (yyvsp[(3) - (5)].nodelist).head;
	      (fmt_yyval.nodeptr)->v.cntl.iffalse = (yyvsp[(4) - (5)].nodeptr);
	    }
    break;

  case 20:

/* Line 1810 of yacc.c  */
#line 367 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodelist).head = (fmt_yyval.nodelist).tail = NULL;
	    }
    break;

  case 22:

/* Line 1810 of yacc.c  */
#line 374 "mh_fmtgram.y"
    {
	      ctx_push (ctx_if);
	    }
    break;

  case 23:

/* Line 1810 of yacc.c  */
#line 380 "mh_fmtgram.y"
    {
	      ctx_pop ();
	    }
    break;

  case 24:

/* Line 1810 of yacc.c  */
#line 386 "mh_fmtgram.y"
    {
	      ctx_pop ();
	      ctx_push (ctx_if);
	    }
    break;

  case 25:

/* Line 1810 of yacc.c  */
#line 393 "mh_fmtgram.y"
    {
	      ctx_pop ();
	      ctx_push (ctx_expr);
	      (fmt_yyval.nodeptr) = printelim ((yyvsp[(1) - (1)].nodeptr));
	    }
    break;

  case 28:

/* Line 1810 of yacc.c  */
#line 405 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodeptr) = NULL;
	    }
    break;

  case 30:

/* Line 1810 of yacc.c  */
#line 410 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodeptr) = (yyvsp[(1) - (1)].nodelist).head;
	    }
    break;

  case 31:

/* Line 1810 of yacc.c  */
#line 416 "mh_fmtgram.y"
    {
	      struct node *np = new_node (fmtnode_cntl, mhtype_num);
	      np->v.cntl.cond = (yyvsp[(2) - (3)].nodeptr);
	      np->v.cntl.iftrue = (yyvsp[(3) - (3)].nodelist).head;
	      np->v.cntl.iffalse = NULL;
	      (fmt_yyval.nodelist).head = (fmt_yyval.nodelist).tail = np;
	    }
    break;

  case 32:

/* Line 1810 of yacc.c  */
#line 424 "mh_fmtgram.y"
    {
	      struct node *np = new_node(fmtnode_cntl, mhtype_num);
	      np->v.cntl.cond = (yyvsp[(3) - (4)].nodeptr);
	      np->v.cntl.iftrue = (yyvsp[(4) - (4)].nodelist).head;
	      np->v.cntl.iffalse = NULL;

	      (yyvsp[(1) - (4)].nodelist).tail->v.cntl.iffalse = np;
	      (yyvsp[(1) - (4)].nodelist).tail = np;

	      (fmt_yyval.nodelist) = (yyvsp[(1) - (4)].nodelist);
	    }
    break;

  case 33:

/* Line 1810 of yacc.c  */
#line 436 "mh_fmtgram.y"
    {
	      (yyvsp[(1) - (2)].nodelist).tail->v.cntl.iffalse = (yyvsp[(2) - (2)].nodeptr);
	      (yyvsp[(1) - (2)].nodelist).tail = (yyvsp[(2) - (2)].nodeptr);
	      (fmt_yyval.nodelist) = (yyvsp[(1) - (2)].nodelist);
	    }
    break;

  case 34:

/* Line 1810 of yacc.c  */
#line 444 "mh_fmtgram.y"
    {
	      (fmt_yyval.nodeptr) = (yyvsp[(2) - (2)].nodelist).head;
	    }
    break;



/* Line 1810 of yacc.c  */
#line 1917 "mh_fmtgram.c"
      default: break;
    }
  /* User semantic actions sometimes alter fmt_yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering fmt_yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", fmt_yyr1[yyn], &fmt_yyval, &yyloc);

  YYPOPSTACK (fmt_yylen);
  fmt_yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = fmt_yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = fmt_yyr1[yyn];

  fmt_yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= fmt_yystate && fmt_yystate <= YYLAST && fmt_yycheck[fmt_yystate] == *yyssp)
    fmt_yystate = fmt_yytable[fmt_yystate];
  else
    fmt_yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = fmt_yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (fmt_yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++fmt_yynerrs;
#if ! YYERROR_VERBOSE
      fmt_yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        fmt_yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (fmt_yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (fmt_yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &fmt_yylval);
	  fmt_yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (fmt_yylen);
  fmt_yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  fmt_yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = fmt_yypact[fmt_yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && fmt_yycheck[yyn] == YYTERROR)
	    {
	      yyn = fmt_yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[fmt_yystate], yyvsp);
      YYPOPSTACK (1);
      fmt_yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = fmt_yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  fmt_yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  fmt_yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (fmt_yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (fmt_yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &fmt_yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (fmt_yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2071 of yacc.c  */
#line 449 "mh_fmtgram.y"


static char *start;
static char *tok_start;
static char *curp;
static mu_linetrack_t trk;
static struct mu_locus_range fmt_yylloc;

static inline size_t
token_leng (void)
{
  return curp - tok_start;
}

static inline void
mark (void)
{
  if (curp > tok_start)
    mu_linetrack_advance (trk, &fmt_yylloc, tok_start, token_leng ());
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
  fmt_yylval.mesg = mesg;
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
fmt_yyerror (const char *s)
{
  if (fmt_yychar != BOGUS)
    {
      char *bol;
      size_t len;
      static char tab[] = "        ";
      size_t b = 0, e = 0;
      size_t i;
      
      bol = find_bol (fmt_yylloc.beg.mu_line);
      len = strcspn (bol, "\n");
      
      mu_diag_at_locus_range (MU_DIAG_ERROR, &fmt_yylloc, "%s", s);
      for (i = 0; i < len; i++)
	/* How ... tribal! */
	{
	  if (bol[i] == '\t')
	    {
	      mu_stream_write (mu_strerr, tab, strlen (tab), NULL);
	      if (fmt_yylloc.beg.mu_col > i)
		b += strlen (tab) - 1;
	      if (fmt_yylloc.end.mu_col > i)
		e += strlen (tab) - 1;
	    }
	  else
	    mu_stream_write (mu_strerr, bol + i, 1, NULL);
	}
      mu_stream_write (mu_strerr, "\n", 1, NULL);
      if (mu_locus_point_eq (&fmt_yylloc.beg, &fmt_yylloc.end))
	mu_error ("%*.*s^",
		  (int) (b + fmt_yylloc.beg.mu_col - 1),
		  (int) (b + fmt_yylloc.beg.mu_col - 1), "");
      else
	mu_error ("%*.*s^%*.*s^",
		  (int)(b + fmt_yylloc.beg.mu_col - 1),
		  (int)(b + fmt_yylloc.beg.mu_col - 1), "",
		  (int)(e + fmt_yylloc.end.mu_col - fmt_yylloc.beg.mu_col - b - 1),
		  (int)(e + fmt_yylloc.end.mu_col - fmt_yylloc.beg.mu_col - b - 1),
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
fmt_yylex (void)
{
  int tok;

  do
    {
      mark ();
      if (fmt_yydebug)
	fprintf (stderr, "lex: [%s] at %-10.10s...]\n",
		 lexer_tab[ctx_get ()].ctx_name, curp);
      tok = lexer_tab[ctx_get ()].lexer ();
    }
  while (tok == STRING && fmt_yylval.str[0] == 0);
  
  mark ();
  if (tok == BOGUS)
    fmt_yyerror (fmt_yylval.mesg);
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
  fmt_yylval.fmtspec = flags | num;
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

  fmt_yylval.builtin = mh_lookup_builtin (tok_start, token_leng ());

  if (!fmt_yylval.builtin)
    {
      return bogus ("unknown function");
    }
  if (!fmt_yylval.builtin->fun
      && !(fmt_yylval.builtin->flags & (MHA_SPECIAL|MHA_VOID)))
    {
      mu_error ("INTERNAL ERROR at %s:%d: \"%s\" has no associated function"
		" and is not marked as MHA_SPECIAL",
		__FILE__, __LINE__, fmt_yylval.builtin->name);
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
  fmt_yylval.str = mu_opool_finish (tokpool, NULL);
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
  fmt_yylval.str = mu_opool_finish (tokpool, NULL);
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

  fmt_yylval.arg.v.str = mu_opool_finish (tokpool, NULL);
  fmt_yylval.arg.type = mhtype_str;
  unput (c);
  
  if (mu_isdigit (fmt_yylval.arg.v.str[0])
      || (fmt_yylval.arg.v.str[0] == '-' && mu_isdigit (fmt_yylval.arg.v.str[1])))
    {
      long n;
      char *p;
      errno = 0;
      n = strtol (fmt_yylval.arg.v.str, &p, 0);
      if (errno == 0 && *p == 0)
	{
	  fmt_yylval.arg.type = mhtype_num;
	  fmt_yylval.arg.v.num = n;
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
    fmt_yydebug = 1;
  start = tok_start = curp = format_str;
  mu_opool_create (&tokpool, MU_OPOOL_ENOMEMABRT);

  ctx_tos = ctx_max = 0;
  ctx_stack = NULL;
  ctx_push (ctx_init);
  mu_linetrack_create (&trk, "input", 2);
  if (locus && locus->mu_file)
    mu_linetrack_rebase (trk, locus);
  mu_locus_range_init (&fmt_yylloc);
  
  rc = fmt_yyparse ();
  if (rc == 0)
    codegen (fmtptr, flags & MH_FMT_PARSE_TREE);
  else
    mu_opool_destroy (&tokpool);

  mu_locus_range_deinit (&fmt_yylloc);
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



