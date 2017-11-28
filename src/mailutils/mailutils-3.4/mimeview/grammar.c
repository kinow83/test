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
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "grammar.y"

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
  
#include <mailutils/cctype.h>
#include <mimeview.h>
#include <grammar.h>
#include <regex.h>
  
static void
yyprint (FILE *output, unsigned short toknum, YYSTYPE val)
{
  switch (toknum)
    {
    case TYPE:
    case IDENT:
    case STRING:
      fprintf (output, "[%lu] %s", (unsigned long) val.string.len,
	       val.string.ptr);
      break;

    case EOL:
      fprintf (output, "\\n");
      break;
      
    default:
      if (mu_isprint (toknum))
	fprintf (output, "'%c'", toknum);
      else
	fprintf (output, "tok(%d)", toknum);
      break;
    }
}

#define YYPRINT yyprint

static mu_list_t arg_list; /* For error recovery */

#define L_OR  0
#define L_AND 1

enum node_type
  {
    true_node,
    functional_node,
    binary_node,
    negation_node,
    suffix_node
  };

union argument
{
  struct mimetypes_string *string;
  unsigned number;
  int c;
  regex_t rx;
};

typedef int (*builtin_t) (union argument *args);

struct node
{
  enum node_type type;
  struct mu_locus_range loc;
  union
  {
    struct
    {
      builtin_t fun;
      union argument *args;
    } function;
    struct node *arg;
    struct
    {
      int op;
      struct node *arg1;
      struct node *arg2;
    } bin; 
    struct mimetypes_string suffix;
  } v;
};

static struct node *make_node (enum node_type type,
			       struct mu_locus_range const *loc); 
static struct node *make_binary_node (int op,
				      struct node *left, struct node *rigth,
				      struct mu_locus_range const *loc);
static struct node *make_negation_node (struct node *p,
					struct mu_locus_range const *loc);

static struct node *make_suffix_node (struct mimetypes_string *suffix,
				      struct mu_locus_range const *loc);
static struct node *make_functional_node (char *ident, mu_list_t list,
					  struct mu_locus_range const *loc);

static int eval_rule (struct node *root);

struct rule_tab
{
  char *type;
  int priority;
  struct mu_locus_range loc;
  struct node *node;
};

static mu_list_t rule_list;
static size_t errors;


/* Line 268 of yacc.c  */
#line 199 "grammar.c"

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
# define YYERROR_VERBOSE 0
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
     TYPE = 258,
     IDENT = 259,
     STRING = 260,
     EOL = 261,
     BOGUS = 262,
     PRIORITY = 263
   };
#endif
/* Tokens.  */
#define TYPE 258
#define IDENT 259
#define STRING 260
#define EOL 261
#define BOGUS 262
#define PRIORITY 263




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 295 of yacc.c  */
#line 143 "grammar.y"

  struct mimetypes_string string;
  char *s;
  mu_list_t list;
  int result;
  struct node *node;



/* Line 295 of yacc.c  */
#line 269 "grammar.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 345 of yacc.c  */
#line 294 "grammar.c"

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
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union mimetypes_yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union mimetypes_yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  16
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   58

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  14
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  27
/* YYNRULES -- Number of states.  */
#define YYNSTATES  41

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   263

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    11,     2,     2,     2,     2,     2,     2,
      12,    13,     2,    10,     9,     2,     2,     2,     2,     2,
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
       5,     6,     7,     8
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     7,    11,    12,    16,    18,    20,
      21,    23,    25,    28,    32,    36,    39,    43,    45,    47,
      49,    54,    55,    57,    62,    64,    68,    70
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      15,     0,    -1,    16,    -1,    17,    -1,    16,     6,    17,
      -1,    -1,     3,    18,    22,    -1,     7,    -1,     1,    -1,
      -1,    19,    -1,    20,    -1,    19,    19,    -1,    19,     9,
      19,    -1,    19,    10,    19,    -1,    11,    20,    -1,    12,
      19,    13,    -1,     5,    -1,    23,    -1,     7,    -1,     8,
      12,    24,    13,    -1,    -1,    21,    -1,     4,    12,    24,
      13,    -1,    25,    -1,    24,     9,    25,    -1,     5,    -1,
       7,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   153,   153,   156,   157,   160,   161,   177,   181,   194,
     197,   200,   201,   208,   215,   224,   228,   232,   236,   237,
     243,   261,   264,   267,   279,   285,   292,   293
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TYPE", "IDENT", "STRING", "EOL",
  "BOGUS", "PRIORITY", "','", "'+'", "'!'", "'('", "')'", "$accept",
  "input", "list", "rule_line", "maybe_rule", "rule", "stmt", "priority",
  "maybe_priority", "function", "arglist", "arg", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,    44,
      43,    33,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 mimetypes_yyr1[] =
{
       0,    14,    15,    16,    16,    17,    17,    17,    17,    18,
      18,    19,    19,    19,    19,    20,    20,    20,    20,    20,
      21,    22,    22,    23,    24,    24,    25,    25
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 mimetypes_yyr2[] =
{
       0,     2,     1,     1,     3,     0,     3,     1,     1,     0,
       1,     1,     2,     3,     3,     2,     3,     1,     1,     1,
       4,     0,     1,     4,     1,     3,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     8,     9,     7,     0,     2,     3,     0,    17,    19,
       0,     0,    21,    10,    11,    18,     1,     0,     0,    15,
       0,     0,    22,     6,     0,     0,    12,     4,    26,    27,
       0,    24,    16,     0,    13,    14,     0,    23,     0,    25,
      20
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,     5,     6,    12,    26,    14,    22,    23,    15,
      30,    31
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -8
static const yytype_int8 mimetypes_yypact[] =
{
      51,    -8,    38,    -8,    16,    -5,    -8,     5,    -8,    -8,
      38,    38,    10,    20,    -8,    -8,    -8,    51,    -3,    -8,
       1,     8,    -8,    -8,    38,    38,    29,    -8,    -8,    -8,
      -6,    -8,    -8,    -3,    29,    38,    -3,    -8,     6,    -8,
      -8
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -8,    -8,    -8,     4,    -8,    -2,    18,    -8,    -8,    -8,
      -7,    -1
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -6
static const yytype_int8 mimetypes_yytable[] =
{
      13,    17,    28,    36,    29,     7,     8,    37,     9,    20,
      24,    25,    10,    11,    32,    36,    16,    18,    21,    40,
      33,    27,    34,    35,     7,     8,    38,     9,    19,    24,
      25,    10,    11,     7,     8,    39,     9,     0,     0,    25,
      10,    11,     7,     8,     0,     9,     0,     0,     0,    10,
      11,    -5,     1,     0,     2,     0,     0,    -5,     3
};

#define yypact_value_is_default(mimetypes_yystate) \
  ((mimetypes_yystate) == (-8))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int8 mimetypes_yycheck[] =
{
       2,     6,     5,     9,     7,     4,     5,    13,     7,    11,
       9,    10,    11,    12,    13,     9,     0,    12,     8,    13,
      12,    17,    24,    25,     4,     5,    33,     7,    10,     9,
      10,    11,    12,     4,     5,    36,     7,    -1,    -1,    10,
      11,    12,     4,     5,    -1,     7,    -1,    -1,    -1,    11,
      12,     0,     1,    -1,     3,    -1,    -1,     6,     7
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     7,    15,    16,    17,     4,     5,     7,
      11,    12,    18,    19,    20,    23,     0,     6,    12,    20,
      19,     8,    21,    22,     9,    10,    19,    17,     5,     7,
      24,    25,    13,    12,    19,    19,     9,    13,    24,    25,
      13
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(mimetypes_yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call mimetypes_yyerror.  This remains here temporarily
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
  if (mimetypes_yychar == YYEMPTY)                                        \
    {                                                           \
      mimetypes_yychar = (Token);                                         \
      mimetypes_yylval = (Value);                                         \
      YYPOPSTACK (mimetypes_yylen);                                       \
      mimetypes_yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      mimetypes_yyerror (YY_("syntax error: cannot back up")); \
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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `mimetypes_yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX mimetypes_yylex (YYLEX_PARAM)
#else
# define YYLEX mimetypes_yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (mimetypes_yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (mimetypes_yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
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
  if (mimetypes_yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int mimetypes_yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, mimetypes_yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int mimetypes_yyrule;
#endif
{
  int yynrhs = mimetypes_yyr2[mimetypes_yyrule];
  int yyi;
  unsigned long int yylno = yyrline[mimetypes_yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     mimetypes_yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[mimetypes_yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (mimetypes_yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int mimetypes_yydebug;
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
  YYSIZE_T mimetypes_yylen;
  for (mimetypes_yylen = 0; yystr[mimetypes_yylen]; mimetypes_yylen++)
    continue;
  return mimetypes_yylen;
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
  const char *mimetypes_yys = yysrc;

  while ((*yyd++ = *mimetypes_yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for mimetypes_yyerror.  The
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
     - The only way there can be no lookahead present (in mimetypes_yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated mimetypes_yychar.
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
      int yyn = mimetypes_yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both mimetypes_yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (mimetypes_yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (mimetypes_yytable[yyx + yyn]))
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

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
int mimetypes_yyparse (void *YYPARSE_PARAM);
#else
int mimetypes_yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int mimetypes_yyparse (void);
#else
int mimetypes_yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int mimetypes_yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE mimetypes_yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE mimetypes_yylloc;

/* Number of syntax errors so far.  */
int mimetypes_yynerrs;


/*----------.
| mimetypes_yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
mimetypes_yyparse (void *YYPARSE_PARAM)
#else
int
mimetypes_yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
mimetypes_yyparse (void)
#else
int
mimetypes_yyparse ()

#endif
#endif
{
    int mimetypes_yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE mimetypes_yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int mimetypes_yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  mimetypes_yystate = 0;
  yyerrstatus = 0;
  mimetypes_yynerrs = 0;
  mimetypes_yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  mimetypes_yylloc.first_line   = mimetypes_yylloc.last_line   = 1;
  mimetypes_yylloc.first_column = mimetypes_yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in mimetypes_yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = mimetypes_yystate;

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
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
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
	union mimetypes_yyalloc *yyptr =
	  (union mimetypes_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", mimetypes_yystate));

  if (mimetypes_yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = mimetypes_yypact[mimetypes_yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (mimetypes_yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      mimetypes_yychar = YYLEX;
    }

  if (mimetypes_yychar <= YYEOF)
    {
      mimetypes_yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (mimetypes_yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &mimetypes_yylval, &mimetypes_yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || mimetypes_yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = mimetypes_yytable[yyn];
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
  YY_SYMBOL_PRINT ("Shifting", yytoken, &mimetypes_yylval, &mimetypes_yylloc);

  /* Discard the shifted token.  */
  mimetypes_yychar = YYEMPTY;

  mimetypes_yystate = yyn;
  *++yyvsp = mimetypes_yylval;
  *++yylsp = mimetypes_yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[mimetypes_yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  mimetypes_yylen = mimetypes_yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  mimetypes_yyval = yyvsp[1-mimetypes_yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - mimetypes_yylen), mimetypes_yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:

/* Line 1810 of yacc.c  */
#line 162 "grammar.y"
    {
	     struct rule_tab *p = mimetypes_malloc (sizeof (*p));
	     if (!rule_list)
	       mu_list_create (&rule_list);
	     p->type = (yyvsp[(1) - (3)].string).ptr;
	     p->node = (yyvsp[(2) - (3)].node);
	     p->priority = (yyvsp[(3) - (3)].result);
	     mu_locus_point_copy (&p->loc.beg, &(yylsp[(1) - (3)]).beg);
	     mu_locus_point_copy (&p->loc.end, &(yylsp[(3) - (3)]).end);
#if 0
	     YY_LOCATION_PRINT (stderr, p->loc);
	     fprintf (stderr, ": rule %s\n", p->type);
#endif
	     mu_list_append (rule_list, p);
	   }
    break;

  case 7:

/* Line 1810 of yacc.c  */
#line 178 "grammar.y"
    {
	     YYERROR;
	   }
    break;

  case 8:

/* Line 1810 of yacc.c  */
#line 182 "grammar.y"
    {
	     errors++;
	     if (arg_list)
	       mu_list_destroy (&arg_list);
	     arg_list = NULL;
	     lex_next_rule ();
	     yyerrok;
	     yyclearin;
	   }
    break;

  case 9:

/* Line 1810 of yacc.c  */
#line 194 "grammar.y"
    {
	     (mimetypes_yyval.node) = make_node (true_node, &mimetypes_yylloc);
	   }
    break;

  case 12:

/* Line 1810 of yacc.c  */
#line 202 "grammar.y"
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[(1) - (2)]).beg;
	     lr.end = (yylsp[(2) - (2)]).end;
	     (mimetypes_yyval.node) = make_binary_node (L_OR, (yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node), &lr);
	   }
    break;

  case 13:

/* Line 1810 of yacc.c  */
#line 209 "grammar.y"
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[(1) - (3)]).beg;
	     lr.end = (yylsp[(3) - (3)]).end;
	     (mimetypes_yyval.node) = make_binary_node (L_OR, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), &lr);
	   }
    break;

  case 14:

/* Line 1810 of yacc.c  */
#line 216 "grammar.y"
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[(1) - (3)]).beg;
	     lr.end = (yylsp[(3) - (3)]).end;
	     (mimetypes_yyval.node) = make_binary_node (L_AND, (yyvsp[(1) - (3)].node), (yyvsp[(3) - (3)].node), &lr);
	   }
    break;

  case 15:

/* Line 1810 of yacc.c  */
#line 225 "grammar.y"
    {
	     (mimetypes_yyval.node) = make_negation_node ((yyvsp[(2) - (2)].node), &(yylsp[(2) - (2)]));
	   }
    break;

  case 16:

/* Line 1810 of yacc.c  */
#line 229 "grammar.y"
    {
	     (mimetypes_yyval.node) = (yyvsp[(2) - (3)].node);
	   }
    break;

  case 17:

/* Line 1810 of yacc.c  */
#line 233 "grammar.y"
    {
	     (mimetypes_yyval.node) = make_suffix_node (&(yyvsp[(1) - (1)].string), &(yylsp[(1) - (1)]));
	   }
    break;

  case 19:

/* Line 1810 of yacc.c  */
#line 238 "grammar.y"
    {
	     YYERROR;
	   }
    break;

  case 20:

/* Line 1810 of yacc.c  */
#line 244 "grammar.y"
    {
	     size_t count = 0;
	     struct mimetypes_string *arg;
	     
	     mu_list_count ((yyvsp[(3) - (4)].list), &count);
	     if (count != 1)
	       {
		 mimetypes_yyerror (_("priority takes single numberic argument"));
		 YYERROR;
	       }
	     mu_list_head ((yyvsp[(3) - (4)].list), (void**) &arg);
	     (mimetypes_yyval.result) = atoi (arg->ptr);
	     mu_list_destroy (&(yyvsp[(3) - (4)].list));
	   }
    break;

  case 21:

/* Line 1810 of yacc.c  */
#line 261 "grammar.y"
    {
	     (mimetypes_yyval.result) = 100;
	   }
    break;

  case 23:

/* Line 1810 of yacc.c  */
#line 268 "grammar.y"
    {
	     struct mu_locus_range lr;
	     lr.beg = (yylsp[(1) - (4)]).beg;
	     lr.end = (yylsp[(4) - (4)]).end;
	     
	     (mimetypes_yyval.node) = make_functional_node ((yyvsp[(1) - (4)].string).ptr, (yyvsp[(3) - (4)].list), &lr);
	     if (!(mimetypes_yyval.node))
	       YYERROR;
	   }
    break;

  case 24:

/* Line 1810 of yacc.c  */
#line 280 "grammar.y"
    {
	     mu_list_create (&arg_list);
	     (mimetypes_yyval.list) = arg_list;
	     mu_list_append ((mimetypes_yyval.list), mimetypes_string_dup (&(yyvsp[(1) - (1)].string)));
	   }
    break;

  case 25:

/* Line 1810 of yacc.c  */
#line 286 "grammar.y"
    {
	     mu_list_append ((yyvsp[(1) - (3)].list), mimetypes_string_dup (&(yyvsp[(3) - (3)].string)));
	     (mimetypes_yyval.list) = (yyvsp[(1) - (3)].list);
	   }
    break;

  case 27:

/* Line 1810 of yacc.c  */
#line 294 "grammar.y"
    {
	     YYERROR;
	   }
    break;



/* Line 1810 of yacc.c  */
#line 1796 "grammar.c"
      default: break;
    }
  /* User semantic actions sometimes alter mimetypes_yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering mimetypes_yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", mimetypes_yyr1[yyn], &mimetypes_yyval, &yyloc);

  YYPOPSTACK (mimetypes_yylen);
  mimetypes_yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = mimetypes_yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = mimetypes_yyr1[yyn];

  mimetypes_yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= mimetypes_yystate && mimetypes_yystate <= YYLAST && mimetypes_yycheck[mimetypes_yystate] == *yyssp)
    mimetypes_yystate = mimetypes_yytable[mimetypes_yystate];
  else
    mimetypes_yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = mimetypes_yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (mimetypes_yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++mimetypes_yynerrs;
#if ! YYERROR_VERBOSE
      mimetypes_yyerror (YY_("syntax error"));
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
        mimetypes_yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = mimetypes_yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (mimetypes_yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (mimetypes_yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &mimetypes_yylval, &mimetypes_yylloc);
	  mimetypes_yychar = YYEMPTY;
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

  yyerror_range[1] = yylsp[1-mimetypes_yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (mimetypes_yylen);
  mimetypes_yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  mimetypes_yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = mimetypes_yypact[mimetypes_yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && mimetypes_yycheck[yyn] == YYTERROR)
	    {
	      yyn = mimetypes_yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[mimetypes_yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      mimetypes_yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = mimetypes_yylval;

  yyerror_range[2] = mimetypes_yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  mimetypes_yystate = yyn;
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
  mimetypes_yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (mimetypes_yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (mimetypes_yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &mimetypes_yylval, &mimetypes_yylloc);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (mimetypes_yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
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
#line 299 "grammar.y"


int
mimetypes_parse (const char *name)
{
  int rc;
  if (mimetypes_open (name))
    return 1;
  mimetypes_yydebug = mu_debug_level_p (MU_DEBCAT_APP, MU_DEBUG_TRACE3);  
  rc = mimetypes_yyparse ();
  mimetypes_close ();
  return rc || errors;
}

static struct node *
make_node (enum node_type type, struct mu_locus_range const *loc)
{
  struct node *p = mimetypes_malloc (sizeof *p);
  p->type = type;
  mu_locus_range_init (&p->loc);
  mu_locus_range_copy (&p->loc, loc);
  return p;
}

static struct node *
make_binary_node (int op, struct node *left, struct node *right,
		  struct mu_locus_range const *loc)
{
  struct node *node = make_node (binary_node, loc);

  node->v.bin.op = op;
  node->v.bin.arg1 = left;
  node->v.bin.arg2 = right;
  return node;
}

struct node *
make_negation_node (struct node *p, struct mu_locus_range const *loc)
{
  struct node *node = make_node (negation_node, loc);
  node->v.arg = p;
  return node;
}

struct node *
make_suffix_node (struct mimetypes_string *suffix,
		  struct mu_locus_range const *loc)
{
  struct node *node = make_node (suffix_node, loc);
  node->v.suffix = *suffix;
  return node;
}

struct builtin_tab
{
  char *name;
  char *args;
  builtin_t handler;
};

/*        match("pattern")
            Pattern match on filename
*/
static int
b_match (union argument *args)
{
  return fnmatch (args[0].string->ptr, mimeview_file, 0) == 0;
}

/*       ascii(offset,length)
            True if bytes are valid printable ASCII (CR, NL, TAB,
            BS, 32-126)
*/
#define ISASCII(c) ((c) &&\
                    (strchr ("\n\r\t\b",c) \
                     || (32<=((unsigned) c) && ((unsigned) c)<=126)))
static int
b_ascii (union argument *args)
{
  int i;
  int rc;

  rc = mu_stream_seek (mimeview_stream, args[0].number, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_seek", NULL, rc);
      return 0;
    }

  for (i = 0; i < args[1].number; i++)
    {
      unsigned char c;
      size_t n;

      rc = mu_stream_read (mimeview_stream, &c, 1, &n);
      if (rc || n == 0)
	break;
      if (!ISASCII (c))
	return 0;
    }
      
  return 1;
}

/*       printable(offset,length)
            True if bytes are printable 8-bit chars (CR, NL, TAB,
            BS, 32-126, 128-254)
*/
#define ISPRINT(c) (ISASCII (c) \
		    || (128<=((unsigned) c) && ((unsigned) c)<=254))
static int
b_printable (union argument *args)
{
  int i;
  int rc;

  rc = mu_stream_seek (mimeview_stream, args[0].number, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_seek", NULL, rc);
      return 0;
    }

  for (i = 0; i < args[1].number; i++)
    {
      unsigned char c;
      size_t n;

      rc = mu_stream_read (mimeview_stream, &c, 1, &n);
      if (rc || n == 0)
	break;
      if (!ISPRINT (c))
	return 0;
    }
  return 1;
}

/*        string(offset,"string")
            True if bytes are identical to string
*/
static int
b_string (union argument *args)
{
  struct mimetypes_string *str = args[1].string;
  int i;
  int rc;
  
  rc = mu_stream_seek (mimeview_stream, args[0].number, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_seek", NULL, rc);
      return 0;
    }

  for (i = 0; i < str->len; i++)
    {
      char c;
      size_t n;

      rc = mu_stream_read (mimeview_stream, &c, 1, &n);
      if (rc || n == 0 || c != str->ptr[i])
	return 0;
    }
  return 1;
}

/*        istring(offset,"string")
            True if a case-insensitive comparison of the bytes is
            identical
*/
static int
b_istring (union argument *args)
{
  int i;
  struct mimetypes_string *str = args[1].string;
  
  int rc;

  rc = mu_stream_seek (mimeview_stream, args[0].number, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_seek", NULL, rc);
      return 0;
    }

  for (i = 0; i < str->len; i++)
    {
      char c;
      size_t n;

      rc = mu_stream_read (mimeview_stream, &c, 1, &n);
      if (rc || n == 0 || mu_tolower (c) != mu_tolower (str->ptr[i]))
	return 0;
    }
  return 1;
}

int
compare_bytes (union argument *args, void *sample, void *buf, size_t size)
{
  int rc;
  size_t n;
  
  rc = mu_stream_seek (mimeview_stream, args[0].number, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_seek", NULL, rc);
      return 0;
    }
  
  rc = mu_stream_read (mimeview_stream, buf, size, &n);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_read", NULL, rc);
      return 0;
    }
  else if (n != size)
    return 0;
  return memcmp (sample, buf, size) == 0;
}

/*       char(offset,value)
            True if byte is identical
*/
static int
b_char (union argument *args)
{
  char val = args[1].number;
  char buf;
  return compare_bytes (args, &val, &buf, sizeof (buf));
}

/*        short(offset,value)
            True if 16-bit integer is identical
	  FIXME: Byte order  
*/
static int
b_short (union argument *args)
{
  uint16_t val = args[1].number;
  uint16_t buf;
  return compare_bytes (args, &val, &buf, sizeof (buf));
}

/*        int(offset,value)
            True if 32-bit integer is identical
          FIXME: Byte order
*/
static int
b_int (union argument *args)
{
  uint32_t val = args[1].number;
  uint32_t buf;
  return compare_bytes (args, &val, &buf, sizeof (buf));
}

/*        locale("string")
            True if current locale matches string
*/
static int
b_locale (union argument *args)
{
  abort (); /* FIXME */
  return 0;
}

/*        contains(offset,range,"string")
            True if the range contains the string
*/
static int
b_contains (union argument *args)
{
  size_t i, count;
  char *buf;
  struct mimetypes_string *str = args[2].string;
  int rc;

  rc = mu_stream_seek (mimeview_stream, args[0].number, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_seek", NULL, rc);
      return 0;
    }

  buf = mu_alloc (args[1].number);
  rc = mu_stream_read (mimeview_stream, buf, args[1].number, &count);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_read", NULL, rc);
    }
  else if (count > str->len)
    for (i = 0; i <= count - str->len; i++)
      if (buf[i] == str->ptr[0] && memcmp (buf + i, str->ptr, str->len) == 0)
	{
	  free (buf);
	  return 1;
	}
  free (buf);
  return 0;
}

#define MIME_MAX_BUFFER 4096

/*   regex(offset,"regex")		True if bytes match regular expression
 */
static int
b_regex (union argument *args)
{
  size_t count;
  int rc;
  char buf[MIME_MAX_BUFFER];
  
  rc = mu_stream_seek (mimeview_stream, args[0].number, MU_SEEK_SET, NULL);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_seek", NULL, rc);
      return 0;
    }

  rc = mu_stream_read (mimeview_stream, buf, sizeof buf - 1, &count);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_read", NULL, rc);
      return 0;
    }
  buf[count] = 0;

  return regexec (&args[1].rx, buf, 0, NULL, 0) == 0;
} 
  

static struct builtin_tab builtin_tab[] = {
  { "match", "s", b_match },
  { "ascii", "dd", b_ascii },
  { "printable", "dd", b_printable },
  { "regex", "dx", b_regex },
  { "string", "ds", b_string },
  { "istring", "ds", b_istring },
  { "char", "dc", b_char },
  { "short", "dd", b_short },
  { "int", "dd", b_int },
  { "locale", "s", b_locale },
  { "contains", "dds", b_contains },
  { NULL }
};
  
struct node *
make_functional_node (char *ident, mu_list_t list,
		      struct mu_locus_range const *loc)
{
  size_t count, i;
  struct builtin_tab *p;
  struct node *node;
  union argument *args;
  mu_iterator_t itr;
  int rc;
  
  for (p = builtin_tab; ; p++)
    {
      if (!p->name)
	{
	  char *s;
	  mu_asprintf (&s, _("%s: unknown function"), ident);
	  mimetypes_yyerror (s);
	  free (s);
	  return NULL;
	}

      if (strcmp (ident, p->name) == 0)
	break;
    }

  mu_list_count (list, &count);
  i = strlen (p->args);

  if (count < i)
    {
      char *s;
      mu_asprintf (&s, _("too few arguments in call to `%s'"), ident);
      mimetypes_yyerror (s);
      free (s);
      return NULL;
    }
  else if (count > i)
    {
      char *s;
      mu_asprintf (&s, _("too many arguments in call to `%s'"), ident);
      mimetypes_yyerror (s);
      free (s);
      return NULL;
    }

  args = mimetypes_malloc (count * sizeof *args);
  
  mu_list_get_iterator (list, &itr);
  for (i = 0, mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr), i++)
    {
      struct mimetypes_string *data;
      char *tmp;
      
      mu_iterator_current (itr, (void **)&data);
      switch (p->args[i])
	{
	case 'd':
	  args[i].number = strtoul (data->ptr, &tmp, 0);
	  if (*tmp)
	    goto err;
	  break;
	  
	case 's':
	  args[i].string = data;
	  break;

	case 'x':
	  {
	    char *s;
	    
	    rc = mu_c_str_unescape_trans (data->ptr,
					  "\\\\\"\"a\ab\bf\fn\nr\rt\tv\v", &s);
	    if (rc)
	      {
		mu_diag_funcall (MU_DIAG_ERROR, "mu_c_str_unescape_trans",
				 data->ptr, rc);
		return NULL;
	      }
	    rc = regcomp (&args[i].rx, s, REG_EXTENDED|REG_NOSUB);
	    free (s);
	    if (rc)
	      {
		char errbuf[512];
		regerror (rc, &args[i].rx, errbuf, sizeof errbuf);
		mimetypes_yyerror (errbuf);
		return NULL;
	      }
	  }
	  break;
	  
	case 'c':
	  args[i].c = strtoul (data->ptr, &tmp, 0);
	  if (*tmp)
	    goto err;
	  break;
	  
	default:
	  abort ();
	}
    }

  node = make_node (functional_node, loc);
  node->v.function.fun = p->handler;
  node->v.function.args = args;
  return node;
  
 err:
  {
    char *s;
    mu_asprintf (&s,
	         _("argument %lu has wrong type in call to `%s'"),
	         (unsigned long) i, ident);
    mimetypes_yyerror (s);
    free (s);
    return NULL;
  }
}

static int
check_suffix (char *suf)
{
  char *p = strrchr (mimeview_file, '.');
  if (!p)
    return 0;
  return strcmp (p+1, suf) == 0;
}

void
mime_debug (int lev, struct mu_locus_range const *loc, char const *fmt, ...)
{
  if (mu_debug_level_p (MU_DEBCAT_APP, lev))
    {
      va_list ap;

      if (loc->beg.mu_col == 0)					       
	mu_debug_log_begin ("%s:%u", loc->beg.mu_file, loc->beg.mu_line);
      else if (strcmp(loc->beg.mu_file, loc->end.mu_file))
	mu_debug_log_begin ("%s:%u.%u-%s:%u.%u",
			    loc->beg.mu_file,
			    loc->beg.mu_line, loc->beg.mu_col,
			    loc->end.mu_file,
			    loc->end.mu_line, loc->end.mu_col);
      else if (loc->beg.mu_line != loc->end.mu_line)
	mu_debug_log_begin ("%s:%u.%u-%u.%u",
			    loc->beg.mu_file,
			    loc->beg.mu_line, loc->beg.mu_col,
			    loc->end.mu_line, loc->end.mu_col);
      else if (loc->beg.mu_col != loc->end.mu_col)
	mu_debug_log_begin ("%s:%u.%u-%u",
			    loc->beg.mu_file,
			    loc->beg.mu_line, loc->beg.mu_col,
			    loc->end.mu_col);
      else
	mu_debug_log_begin ("%s:%u.%u",
			    loc->beg.mu_file,
			    loc->beg.mu_line, loc->beg.mu_col);

      mu_stream_write (mu_strerr, ": ", 2, NULL);

      va_start (ap, fmt);
      mu_stream_vprintf (mu_strerr, fmt, ap);
      va_end (ap);
      mu_debug_log_nl ();
    }
}

static int
eval_rule (struct node *root)
{
  int result;
  
  switch (root->type)
    {
    case true_node:
      result = 1;
      break;
      
    case functional_node:
      result = root->v.function.fun (root->v.function.args);
      break;
      
    case binary_node:
      result = eval_rule (root->v.bin.arg1);
      switch (root->v.bin.op)
	{
	case L_OR:
	  if (!result)
	    result |= eval_rule (root->v.bin.arg2);
	  break;
	  
	case L_AND:
	  if (result)
	    result &= eval_rule (root->v.bin.arg2);
	  break;
	  
	default:
	  abort ();
	}
      break;
      
    case negation_node:
      result = !eval_rule (root->v.arg);
      break;
      
    case suffix_node:
      result = check_suffix (root->v.suffix.ptr);
      break;

    default:
      abort ();
    }
  mime_debug (MU_DEBUG_TRACE2, &root->loc, "result %s", result ? "true" : "false");
  return result;
}

static int
evaluate (void **itmv, size_t itmc, void *call_data)
{
  struct rule_tab *p = itmv[0];
  if (eval_rule (p->node))
    {
      itmv[0] = p;
      mime_debug (MU_DEBUG_TRACE1, &p->loc, "rule %s matches", p->type);
      return MU_LIST_MAP_OK;
    }
  return MU_LIST_MAP_SKIP;
}

static int
rule_cmp (const void *a, const void *b)
{
  struct rule_tab const *arule = a;
  struct rule_tab const *brule = b;

  if (arule->priority == brule->priority)
    {
      if (arule->node->type == true_node
	  && brule->node->type != true_node)
	return 1;
      else if (brule->node->type == true_node
	       && arule->node->type != true_node)
	return -1;
      else
	return mu_c_strcasecmp (arule->type, brule->type);
    }
  return arule->priority - brule->priority;
}

const char *
get_file_type ()
{
  mu_list_t res = NULL;
  const char *type = NULL;
  
  mu_list_map (rule_list, evaluate, NULL, 1, &res);
  if (!mu_list_is_empty (res))
    {
      struct rule_tab *rule;
      mu_list_sort (res, rule_cmp);
      mu_list_head (res, (void**) &rule);
      mime_debug (MU_DEBUG_TRACE0, &rule->loc, "selected rule %s", rule->type);
      type = rule->type;
    }
  mu_list_destroy (&res);
  return type;
}
    

