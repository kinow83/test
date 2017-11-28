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
#line 1 "parsedate.y"

/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003, 2007-2008, 2010-2012, 2014-2017 Free Software
   Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
   
   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */
  
/* A heavily modified version of the well-known public domain getdate.y.
   It was originally written by Steven M. Bellovin <smb@research.att.com>
   while at the University of North Carolina at Chapel Hill.  Later tweaked
   by a couple of people on Usenet.  Completely overhauled by Rich $alz
   <rsalz@bbn.com> and Jim Berets <jberets@bbn.com> in August, 1990.
   Rewritten using a proper union by Sergey Poznyakoff <gray@gnu.org> */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/datetime.h>

#define ISSPACE(c) mu_isspace (c)
#define ISALPHA(c) mu_isalpha (c)
#define ISUPPER(c) mu_isupper (c)

static inline int ISDIGIT (unsigned c)
{
  return mu_isdigit (c);
}

static int pd_yyparse (void); 
static int pd_yylex (void);
static int pd_yyerror (char *s);

#define EPOCH		1970
#define HOUR(x)		((x) * 60)

#define MAX_BUFF_LEN    128   /* size of buffer to read the date into */

/*
**  An entry in the lexical lookup table.
*/
typedef struct _lex_tab {
  const char	*name;
  int		type;
  int		value;
} SYMBOL;


/*
**  Meridian:  am, pm, or 24-hour style.
*/
typedef enum meridian {
  MERam,
  MERpm,
  MER24
} MERIDIAN;
 
#define MASK_IS_SET(f,m) (((f)&(m))==(m))
#define MASK_TEST(f,m)   ((f)&(m)) 
struct pd_date
{
  int mask;
  int day;
  int hour;
  int minute;
  int month;
  int second;
  int year;
  int tz;
  char const *tzname;
  enum meridian meridian;
  int number;
  int ordinal;
};

#define DATE_INIT(date) memset(&(date), 0, sizeof(date))
#define DATE_SET(date, memb, m, val, lim, onerror)                        \
 do                                                                       \
   {                                                                      \
     int __x = val;                                                       \
     if (((m) != MU_PD_MASK_TZ && __x < 0) || (lim && __x > lim)) onerror;\
     date . memb = __x; date.mask |= m;                                   \
   }                                                                      \
 while (0)
   
#define __SET_SECOND(d,v,a)   DATE_SET(d,second,MU_PD_MASK_SECOND,v,59,a)
#define __SET_MINUTE(d,v,a)   DATE_SET(d,minute,MU_PD_MASK_MINUTE,v,59,a)  
#define __SET_HOUR(d,v,a)     DATE_SET(d,hour,MU_PD_MASK_HOUR,v,23,a)
#define __SET_DAY(d,v,a)      DATE_SET(d,day,MU_PD_MASK_DAY,v,31,a)   
#define __SET_MONTH(d,v,a)    DATE_SET(d,month,MU_PD_MASK_MONTH,v,12,a)
#define __SET_YEAR(d,v,a)     DATE_SET(d,year,MU_PD_MASK_YEAR,v,0,a)  
#define __SET_TZ(d,v,a)       DATE_SET(d,tz,MU_PD_MASK_TZ,v,0,a)
#define __SET_MERIDIAN(d,v,a) DATE_SET(d,meridian,MU_PD_MASK_MERIDIAN,v,MER24,a)
#define __SET_ORDINAL(d,v,a)  DATE_SET(d,ordinal,MU_PD_MASK_ORDINAL,v,0,a)
#define __SET_NUMBER(d,v,a)   DATE_SET(d,number,MU_PD_MASK_NUMBER,v,0,a) 
 
#define SET_SECOND(d,v)   __SET_SECOND(d,v,YYERROR)   
#define SET_MINUTE(d,v)   __SET_MINUTE(d,v,YYERROR)   
#define SET_HOUR(d,v)     __SET_HOUR(d,v,YYERROR)     
#define SET_DAY(d,v)      __SET_DAY(d,v,YYERROR)      
#define SET_MONTH(d,v)    __SET_MONTH(d,v,YYERROR)    
#define SET_YEAR(d,v)     __SET_YEAR(d,v,YYERROR)     
#define SET_TZ(d,v)       __SET_TZ(d,v,YYERROR)
/* Set timezone from a packed representation (HHMM)

   The proper way of doing so would be:
   
#define SET_TZ_PACK(d,v)				  \
  SET_TZ (d, ((v) < 0 ? -(-(v) % 100 + (-(v) / 100) * 60) \
	              : ((v) % 100 + ((v) / 100) * 60)))

   However, we need to invert the sign in order for mktime
   to work properly (see mu_parse_date_dtl below).  The proper
   sign is then restored upon return from the function.
   
   Once mu_mktime is in place, this can be changed.
*/
#define SET_TZ_PACK(d,v)				  \
  SET_TZ (d, ((v) < 0 ? (-(v) % 100 + (-(v) / 100) * 60)  \
	              : -((v) % 100 + ((v) / 100) * 60)))

#define SET_MERIDIAN(d,v) __SET_MERIDIAN(d,v,YYERROR) 
#define SET_ORDINAL(d,v)  __SET_ORDINAL(d,v,YYERROR)  
#define SET_NUMBER(d,v)   __SET_NUMBER(d,v,YYERROR)   

int
pd_date_union (struct pd_date *a, struct pd_date *b)
{
  int diff = (~a->mask) & b->mask;
  if (!diff)
    return 1;

  a->mask |= diff;
  
  if (diff & MU_PD_MASK_SECOND)
    a->second = b->second;
  
  if (diff & MU_PD_MASK_MINUTE)
    a->minute = b->minute;

  if (diff & MU_PD_MASK_HOUR)
    a->hour = b->hour;

  if (diff & MU_PD_MASK_DAY)
    a->day = b->day;

  if (diff & MU_PD_MASK_MONTH)
    a->month = b->month;

  if (diff & MU_PD_MASK_YEAR)
    a->year = b->year;

  if (diff & MU_PD_MASK_TZ)
    a->tz = b->tz;

  if (diff & MU_PD_MASK_MERIDIAN)
    a->meridian = b->meridian;

  if (diff & MU_PD_MASK_ORDINAL)
    a->ordinal = b->ordinal;

  if (diff & MU_PD_MASK_NUMBER)
    a->number = b->number;

  return 0;
}

struct pd_datespec
{
  struct pd_date date;
  struct pd_date rel;
};

static struct pd_datespec pd;
 
static const char	*yyinput;



/* Line 268 of yacc.c  */
#line 267 "parsedate.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
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
     T_AGO = 258,
     T_DST = 259,
     T_ID = 260,
     T_DAY = 261,
     T_DAY_UNIT = 262,
     T_HOUR_UNIT = 263,
     T_MINUTE_UNIT = 264,
     T_MONTH = 265,
     T_MONTH_UNIT = 266,
     T_SEC_UNIT = 267,
     T_SNUMBER = 268,
     T_UNUMBER = 269,
     T_YEAR_UNIT = 270,
     T_ZONE = 271,
     T_DAYZONE = 272,
     T_MERIDIAN = 273
   };
#endif
/* Tokens.  */
#define T_AGO 258
#define T_DST 259
#define T_ID 260
#define T_DAY 261
#define T_DAY_UNIT 262
#define T_HOUR_UNIT 263
#define T_MINUTE_UNIT 264
#define T_MONTH 265
#define T_MONTH_UNIT 266
#define T_SEC_UNIT 267
#define T_SNUMBER 268
#define T_UNUMBER 269
#define T_YEAR_UNIT 270
#define T_ZONE 271
#define T_DAYZONE 272
#define T_MERIDIAN 273




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 295 of yacc.c  */
#line 196 "parsedate.y"

  int number;
  enum meridian meridian;
  struct pd_date date;
  struct pd_datespec datespec;
  struct { char const *name; int delta; } tz;



/* Line 295 of yacc.c  */
#line 357 "parsedate.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 345 of yacc.c  */
#line 369 "parsedate.c"

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
union pd_yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union pd_yyalloc) - 1)

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   74

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  22
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  54
/* YYNRULES -- Number of states.  */
#define YYNSTATES  69

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    20,     2,     2,    21,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    19,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    12,    15,    17,    19,
      21,    23,    26,    31,    36,    43,    50,    52,    54,    57,
      59,    62,    65,    69,    75,    79,    83,    86,    91,    94,
      98,   104,   107,   109,   111,   114,   117,   120,   122,   125,
     128,   130,   133,   136,   138,   141,   144,   146,   149,   152,
     154,   157,   160,   162,   163
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      23,     0,    -1,    24,    -1,    -1,    24,    25,    -1,    24,
      30,    -1,    24,    14,    -1,    26,    -1,    27,    -1,    29,
      -1,    28,    -1,    14,    18,    -1,    14,    19,    14,    33,
      -1,    14,    19,    14,    13,    -1,    14,    19,    14,    19,
      14,    33,    -1,    14,    19,    14,    19,    14,    13,    -1,
      16,    -1,    17,    -1,    16,     4,    -1,     6,    -1,     6,
      20,    -1,    14,     6,    -1,    14,    21,    14,    -1,    14,
      21,    14,    21,    14,    -1,    14,    13,    13,    -1,    14,
      10,    13,    -1,    10,    14,    -1,    10,    14,    20,    14,
      -1,    14,    10,    -1,    14,    10,    14,    -1,     6,    10,
      14,    26,    14,    -1,    31,     3,    -1,    31,    -1,    32,
      -1,    31,    32,    -1,    14,    15,    -1,    13,    15,    -1,
      15,    -1,    14,    11,    -1,    13,    11,    -1,    11,    -1,
      14,     7,    -1,    13,     7,    -1,     7,    -1,    14,     8,
      -1,    13,     8,    -1,     8,    -1,    14,     9,    -1,    13,
       9,    -1,     9,    -1,    14,    12,    -1,    13,    12,    -1,
      12,    -1,    -1,    18,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   219,   219,   226,   230,   236,   242,   285,   286,   287,
     288,   291,   297,   304,   312,   320,   331,   337,   343,   351,
     357,   363,   371,   377,   397,   405,   422,   428,   435,   441,
     448,   463,   473,   477,   483,   491,   496,   501,   506,   511,
     516,   521,   526,   531,   536,   541,   546,   551,   556,   561,
     566,   571,   576,   584,   587
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_AGO", "T_DST", "T_ID", "T_DAY",
  "T_DAY_UNIT", "T_HOUR_UNIT", "T_MINUTE_UNIT", "T_MONTH", "T_MONTH_UNIT",
  "T_SEC_UNIT", "T_SNUMBER", "T_UNUMBER", "T_YEAR_UNIT", "T_ZONE",
  "T_DAYZONE", "T_MERIDIAN", "':'", "','", "'/'", "$accept", "input",
  "spec", "item", "time", "zone", "day", "date", "rel", "relspec",
  "relunit", "o_merid", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,    58,
      44,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 pd_yyr1[] =
{
       0,    22,    23,    24,    24,    24,    24,    25,    25,    25,
      25,    26,    26,    26,    26,    26,    27,    27,    27,    28,
      28,    28,    29,    29,    29,    29,    29,    29,    29,    29,
      29,    30,    30,    31,    31,    32,    32,    32,    32,    32,
      32,    32,    32,    32,    32,    32,    32,    32,    32,    32,
      32,    32,    32,    33,    33
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 pd_yyr2[] =
{
       0,     2,     1,     0,     2,     2,     2,     1,     1,     1,
       1,     2,     4,     4,     6,     6,     1,     1,     2,     1,
       2,     2,     3,     5,     3,     3,     2,     4,     2,     3,
       5,     2,     1,     1,     2,     2,     2,     1,     2,     2,
       1,     2,     2,     1,     2,     2,     1,     2,     2,     1,
       2,     2,     1,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,    19,    43,    46,    49,     0,    40,
      52,     0,     6,    37,    16,    17,     4,     7,     8,    10,
       9,     5,    32,    33,     0,    20,    26,    42,    45,    48,
      39,    51,    36,    21,    41,    44,    47,    28,    38,    50,
       0,    35,    11,     0,     0,    18,    31,     0,    34,     0,
       0,    25,    29,    24,    53,    22,     0,     0,    27,    13,
      54,     0,    12,     0,    30,    53,    23,    15,    14
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,    16,    17,    18,    19,    20,    21,    22,
      23,    62
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -7
static const yytype_int8 pd_yypact[] =
{
      -7,     8,    21,    -7,     1,    -7,    -7,    -7,    -4,    -7,
      -7,    32,    -6,    -7,    38,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    11,    -7,    31,    -7,    34,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,     3,    -7,    -7,
      46,    -7,    -7,    48,    49,    -7,    -7,    41,    -7,    50,
      51,    -7,    -7,    -7,    42,    45,    39,    53,    -7,    -7,
      -7,    54,    -7,    55,    -7,    33,    -7,    -7,    -7
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -7,    -7,    -7,    -7,    22,    -7,    -7,    -7,    -7,    -7,
      52,     5
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 pd_yytable[] =
{
      33,    34,    35,    36,    37,    38,    39,    40,     3,    41,
      26,    24,    42,    43,    46,    44,    51,    52,     5,     6,
       7,    25,     9,    10,    11,    47,    13,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    27,
      28,    29,    45,    30,    31,    49,    67,    32,    34,    35,
      36,    60,    38,    39,    50,    59,    41,    42,    43,    53,
      60,    61,    54,    55,    56,    58,    63,    64,    65,    66,
      68,    57,     0,     0,    48
};

#define yypact_value_is_default(pd_yystate) \
  ((pd_yystate) == (-7))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int8 pd_yycheck[] =
{
       6,     7,     8,     9,    10,    11,    12,    13,     0,    15,
      14,    10,    18,    19,     3,    21,    13,    14,     7,     8,
       9,    20,    11,    12,    13,    14,    15,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     7,
       8,     9,     4,    11,    12,    14,    13,    15,     7,     8,
       9,    18,    11,    12,    20,    13,    15,    18,    19,    13,
      18,    19,    14,    14,    14,    14,    21,    14,    14,    14,
      65,    49,    -1,    -1,    22
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    23,    24,     0,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    25,    26,    27,    28,
      29,    30,    31,    32,    10,    20,    14,     7,     8,     9,
      11,    12,    15,     6,     7,     8,     9,    10,    11,    12,
      13,    15,    18,    19,    21,     4,     3,    14,    32,    14,
      20,    13,    14,    13,    14,    14,    14,    26,    14,    13,
      18,    19,    33,    21,    14,    14,    14,    13,    33
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(pd_yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call pd_yyerror.  This remains here temporarily
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
  if (pd_yychar == YYEMPTY)                                        \
    {                                                           \
      pd_yychar = (Token);                                         \
      pd_yylval = (Value);                                         \
      YYPOPSTACK (pd_yylen);                                       \
      pd_yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      pd_yyerror (YY_("syntax error: cannot back up")); \
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


/* YYLEX -- calling `pd_yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX pd_yylex (YYLEX_PARAM)
#else
# define YYLEX pd_yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (pd_yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (pd_yydebug)								  \
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
  if (pd_yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int pd_yyrule)
#else
static void
yy_reduce_print (yyvsp, pd_yyrule)
    YYSTYPE *yyvsp;
    int pd_yyrule;
#endif
{
  int yynrhs = pd_yyr2[pd_yyrule];
  int yyi;
  unsigned long int yylno = yyrline[pd_yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     pd_yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[pd_yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (pd_yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int pd_yydebug;
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
  YYSIZE_T pd_yylen;
  for (pd_yylen = 0; yystr[pd_yylen]; pd_yylen++)
    continue;
  return pd_yylen;
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
  const char *pd_yys = yysrc;

  while ((*yyd++ = *pd_yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for pd_yyerror.  The
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
     - The only way there can be no lookahead present (in pd_yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated pd_yychar.
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
      int yyn = pd_yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both pd_yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (pd_yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (pd_yytable[yyx + yyn]))
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
int pd_yyparse (void *YYPARSE_PARAM);
#else
int pd_yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int pd_yyparse (void);
#else
int pd_yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int pd_yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE pd_yylval;

/* Number of syntax errors so far.  */
int pd_yynerrs;


/*----------.
| pd_yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
pd_yyparse (void *YYPARSE_PARAM)
#else
int
pd_yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
pd_yyparse (void)
#else
int
pd_yyparse ()

#endif
#endif
{
    int pd_yystate;
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
  YYSTYPE pd_yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int pd_yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  pd_yystate = 0;
  yyerrstatus = 0;
  pd_yynerrs = 0;
  pd_yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in pd_yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = pd_yystate;

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
	union pd_yyalloc *yyptr =
	  (union pd_yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
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

  YYDPRINTF ((stderr, "Entering state %d\n", pd_yystate));

  if (pd_yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = pd_yypact[pd_yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (pd_yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      pd_yychar = YYLEX;
    }

  if (pd_yychar <= YYEOF)
    {
      pd_yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (pd_yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &pd_yylval, &pd_yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || pd_yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = pd_yytable[yyn];
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
  YY_SYMBOL_PRINT ("Shifting", yytoken, &pd_yylval, &pd_yylloc);

  /* Discard the shifted token.  */
  pd_yychar = YYEMPTY;

  pd_yystate = yyn;
  *++yyvsp = pd_yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[pd_yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  pd_yylen = pd_yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  pd_yyval = yyvsp[1-pd_yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1810 of yacc.c  */
#line 220 "parsedate.y"
    {
	    pd = (yyvsp[(1) - (1)].datespec);
	  }
    break;

  case 3:

/* Line 1810 of yacc.c  */
#line 226 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.datespec).date);
	    DATE_INIT ((pd_yyval.datespec).rel);
	  }
    break;

  case 4:

/* Line 1810 of yacc.c  */
#line 231 "parsedate.y"
    {
	    if (pd_date_union (&(yyvsp[(1) - (2)].datespec).date, &(yyvsp[(2) - (2)].date)))
	      YYERROR;
	    (pd_yyval.datespec) = (yyvsp[(1) - (2)].datespec);
	  }
    break;

  case 5:

/* Line 1810 of yacc.c  */
#line 237 "parsedate.y"
    {
	    if (pd_date_union (&(yyvsp[(1) - (2)].datespec).rel, &(yyvsp[(2) - (2)].date)))
	      YYERROR;
	    (pd_yyval.datespec) = (yyvsp[(1) - (2)].datespec);
	  }
    break;

  case 6:

/* Line 1810 of yacc.c  */
#line 243 "parsedate.y"
    {
	    if (MASK_IS_SET ((yyvsp[(1) - (2)].datespec).date.mask, (MU_PD_MASK_TIME|MU_PD_MASK_DATE))
		&& !(yyvsp[(1) - (2)].datespec).rel.mask)
	      {
		if (MASK_IS_SET ((yyvsp[(1) - (2)].datespec).date.mask, MU_PD_MASK_YEAR))
		  {
		    if (!MASK_IS_SET ((yyvsp[(1) - (2)].datespec).date.mask, MU_PD_MASK_TZ))
		      SET_TZ_PACK ((yyvsp[(1) - (2)].datespec).date, (yyvsp[(2) - (2)].number));
		    else
		      YYERROR;
		  }
		else
		  {
		    SET_YEAR ((yyvsp[(1) - (2)].datespec).date, (yyvsp[(2) - (2)].number));
		  }
	      }
	    else
	      {
		if ((yyvsp[(2) - (2)].number) > 10000)
		  {
		    SET_DAY ((yyvsp[(1) - (2)].datespec).date, (yyvsp[(2) - (2)].number) % 100);
		    SET_MONTH ((yyvsp[(1) - (2)].datespec).date, ((yyvsp[(2) - (2)].number) / 100) %100);
		    SET_YEAR ((yyvsp[(1) - (2)].datespec).date, (yyvsp[(2) - (2)].number) / 10000);
		  }
		else
		  {
		    if ((yyvsp[(2) - (2)].number) < 100)
		      {
			SET_YEAR ((yyvsp[(1) - (2)].datespec).date, (yyvsp[(2) - (2)].number));
		      }
		    else
		      {
		    	SET_HOUR ((yyvsp[(1) - (2)].datespec).date, (yyvsp[(2) - (2)].number) / 100);
		    	SET_MINUTE ((yyvsp[(1) - (2)].datespec).date, (yyvsp[(2) - (2)].number) % 100);
		      }
		    SET_MERIDIAN ((yyvsp[(1) - (2)].datespec).date, MER24);
		  }
	      }
	    (pd_yyval.datespec) = (yyvsp[(1) - (2)].datespec);
	  }
    break;

  case 11:

/* Line 1810 of yacc.c  */
#line 292 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (2)].number));
	    SET_MERIDIAN ((pd_yyval.date), (yyvsp[(2) - (2)].meridian));
	  }
    break;

  case 12:

/* Line 1810 of yacc.c  */
#line 298 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (4)].number));
	    SET_MINUTE ((pd_yyval.date), (yyvsp[(3) - (4)].number));
	    SET_MERIDIAN ((pd_yyval.date), (yyvsp[(4) - (4)].meridian));
	  }
    break;

  case 13:

/* Line 1810 of yacc.c  */
#line 305 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (4)].number));
	    SET_MINUTE ((pd_yyval.date), (yyvsp[(3) - (4)].number));
	    SET_MERIDIAN ((pd_yyval.date), MER24);
	    SET_TZ_PACK ((pd_yyval.date), (yyvsp[(4) - (4)].number));
	  }
    break;

  case 14:

/* Line 1810 of yacc.c  */
#line 313 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (6)].number));
	    SET_MINUTE ((pd_yyval.date), (yyvsp[(3) - (6)].number));
	    SET_SECOND ((pd_yyval.date), (yyvsp[(5) - (6)].number));
	    SET_MERIDIAN ((pd_yyval.date), (yyvsp[(6) - (6)].meridian));
	  }
    break;

  case 15:

/* Line 1810 of yacc.c  */
#line 321 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (6)].number));
	    SET_MINUTE ((pd_yyval.date), (yyvsp[(3) - (6)].number));
	    SET_SECOND ((pd_yyval.date), (yyvsp[(5) - (6)].number));
	    SET_MERIDIAN ((pd_yyval.date), MER24);
	    SET_TZ_PACK ((pd_yyval.date), (yyvsp[(6) - (6)].number));
	  }
    break;

  case 16:

/* Line 1810 of yacc.c  */
#line 332 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_TZ ((pd_yyval.date), (yyvsp[(1) - (1)].tz).delta);
	    (pd_yyval.date).tzname = (yyvsp[(1) - (1)].tz).name;
	  }
    break;

  case 17:

/* Line 1810 of yacc.c  */
#line 338 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_TZ ((pd_yyval.date), (yyvsp[(1) - (1)].tz).delta - 60);
	    (pd_yyval.date).tzname = (yyvsp[(1) - (1)].tz).name;
	  }
    break;

  case 18:

/* Line 1810 of yacc.c  */
#line 344 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_TZ ((pd_yyval.date), (yyvsp[(1) - (2)].tz).delta - 60);
	    (pd_yyval.date).tzname = (yyvsp[(1) - (2)].tz).name;
	  }
    break;

  case 19:

/* Line 1810 of yacc.c  */
#line 352 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_ORDINAL ((pd_yyval.date), 1);
	    SET_NUMBER ((pd_yyval.date), (yyvsp[(1) - (1)].number));
	  }
    break;

  case 20:

/* Line 1810 of yacc.c  */
#line 358 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_ORDINAL ((pd_yyval.date), 1);
	    SET_NUMBER ((pd_yyval.date), (yyvsp[(1) - (2)].number));
	  }
    break;

  case 21:

/* Line 1810 of yacc.c  */
#line 364 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_ORDINAL ((pd_yyval.date), (yyvsp[(1) - (2)].number));
	    SET_NUMBER ((pd_yyval.date), (yyvsp[(2) - (2)].number));
	  }
    break;

  case 22:

/* Line 1810 of yacc.c  */
#line 372 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(1) - (3)].number));
	    SET_DAY ((pd_yyval.date), (yyvsp[(3) - (3)].number));
	  }
    break;

  case 23:

/* Line 1810 of yacc.c  */
#line 378 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    /* Interpret as YYYY/MM/DD if $1 >= 1000, otherwise as MM/DD/YY.
	       The goal in recognizing YYYY/MM/DD is solely to support legacy
	       machine-generated dates like those in an RCS log listing.  If
	       you want portability, use the ISO 8601 format.  */
	    if ((yyvsp[(1) - (5)].number) >= 1000)
	      {
		SET_YEAR ((pd_yyval.date), (yyvsp[(1) - (5)].number));
		SET_MONTH ((pd_yyval.date), (yyvsp[(3) - (5)].number));
		SET_DAY ((pd_yyval.date), (yyvsp[(5) - (5)].number));
	      }
	    else
	      {
		SET_MONTH ((pd_yyval.date), (yyvsp[(1) - (5)].number));
		SET_DAY ((pd_yyval.date), (yyvsp[(3) - (5)].number));
		SET_YEAR ((pd_yyval.date), (yyvsp[(5) - (5)].number));
	      }
	  }
    break;

  case 24:

/* Line 1810 of yacc.c  */
#line 398 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    /* ISO 8601 format.  yyyy-mm-dd.  */
	    SET_YEAR ((pd_yyval.date), (yyvsp[(1) - (3)].number));
	    SET_MONTH ((pd_yyval.date), -(yyvsp[(2) - (3)].number));
	    SET_DAY ((pd_yyval.date), -(yyvsp[(3) - (3)].number));
	  }
    break;

  case 25:

/* Line 1810 of yacc.c  */
#line 406 "parsedate.y"
    {
	    /* either 17-JUN-1992 or 1992-JUN-17 */
	    DATE_INIT ((pd_yyval.date));
	    if ((yyvsp[(1) - (3)].number) < 32)
	      {
		SET_DAY ((pd_yyval.date), (yyvsp[(1) - (3)].number));
		SET_MONTH ((pd_yyval.date), (yyvsp[(2) - (3)].number));
		SET_YEAR ((pd_yyval.date), -(yyvsp[(3) - (3)].number));
	      }
	    else
	      {
		SET_DAY ((pd_yyval.date), -(yyvsp[(3) - (3)].number));
		SET_MONTH ((pd_yyval.date), (yyvsp[(2) - (3)].number));
		SET_YEAR ((pd_yyval.date), (yyvsp[(1) - (3)].number));
	      }
	  }
    break;

  case 26:

/* Line 1810 of yacc.c  */
#line 423 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(1) - (2)].number));
	    SET_DAY ((pd_yyval.date), (yyvsp[(2) - (2)].number));
	  }
    break;

  case 27:

/* Line 1810 of yacc.c  */
#line 429 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(1) - (4)].number));
	    SET_DAY ((pd_yyval.date), (yyvsp[(2) - (4)].number));
	    SET_YEAR ((pd_yyval.date), (yyvsp[(4) - (4)].number));
	  }
    break;

  case 28:

/* Line 1810 of yacc.c  */
#line 436 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(2) - (2)].number));
	    SET_DAY ((pd_yyval.date), (yyvsp[(1) - (2)].number));
	  }
    break;

  case 29:

/* Line 1810 of yacc.c  */
#line 442 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(2) - (3)].number));
	    SET_DAY ((pd_yyval.date), (yyvsp[(1) - (3)].number));
	    SET_YEAR ((pd_yyval.date), (yyvsp[(3) - (3)].number));
	  }
    break;

  case 30:

/* Line 1810 of yacc.c  */
#line 449 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));

	    SET_ORDINAL ((pd_yyval.date), 1);
	    SET_NUMBER ((pd_yyval.date), (yyvsp[(1) - (5)].number));

	    SET_MONTH ((pd_yyval.date), (yyvsp[(2) - (5)].number));
	    SET_DAY ((pd_yyval.date), (yyvsp[(3) - (5)].number));
	    SET_YEAR ((pd_yyval.date), (yyvsp[(5) - (5)].number));
	    if (pd_date_union (&(pd_yyval.date), &(yyvsp[(4) - (5)].date)))
	      YYERROR;
	  }
    break;

  case 31:

/* Line 1810 of yacc.c  */
#line 464 "parsedate.y"
    {
	    (yyvsp[(1) - (2)].date).second = - (yyvsp[(1) - (2)].date).second;
	    (yyvsp[(1) - (2)].date).minute = - (yyvsp[(1) - (2)].date).minute;
	    (yyvsp[(1) - (2)].date).hour = - (yyvsp[(1) - (2)].date).hour;
	    (yyvsp[(1) - (2)].date).day = - (yyvsp[(1) - (2)].date).day;
	    (yyvsp[(1) - (2)].date).month = - (yyvsp[(1) - (2)].date).month;
	    (yyvsp[(1) - (2)].date).year = - (yyvsp[(1) - (2)].date).year;
	    (pd_yyval.date) = (yyvsp[(1) - (2)].date);
	  }
    break;

  case 33:

/* Line 1810 of yacc.c  */
#line 478 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    if (pd_date_union (&(pd_yyval.date), &(yyvsp[(1) - (1)].date)))
	      YYERROR;
	  }
    break;

  case 34:

/* Line 1810 of yacc.c  */
#line 484 "parsedate.y"
    {
	    if (pd_date_union (&(yyvsp[(1) - (2)].date), &(yyvsp[(2) - (2)].date)))
	      YYERROR;
	    (pd_yyval.date) = (yyvsp[(1) - (2)].date);
	  }
    break;

  case 35:

/* Line 1810 of yacc.c  */
#line 492 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_YEAR ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 36:

/* Line 1810 of yacc.c  */
#line 497 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_YEAR ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 37:

/* Line 1810 of yacc.c  */
#line 502 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_YEAR ((pd_yyval.date), (yyvsp[(1) - (1)].number));
	  }
    break;

  case 38:

/* Line 1810 of yacc.c  */
#line 507 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 39:

/* Line 1810 of yacc.c  */
#line 512 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 40:

/* Line 1810 of yacc.c  */
#line 517 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MONTH ((pd_yyval.date), (yyvsp[(1) - (1)].number));
	  }
    break;

  case 41:

/* Line 1810 of yacc.c  */
#line 522 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_DAY ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 42:

/* Line 1810 of yacc.c  */
#line 527 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_DAY ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 43:

/* Line 1810 of yacc.c  */
#line 532 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_DAY ((pd_yyval.date), (yyvsp[(1) - (1)].number));
	  }
    break;

  case 44:

/* Line 1810 of yacc.c  */
#line 537 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 45:

/* Line 1810 of yacc.c  */
#line 542 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 46:

/* Line 1810 of yacc.c  */
#line 547 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_HOUR ((pd_yyval.date), (yyvsp[(1) - (1)].number));
	  }
    break;

  case 47:

/* Line 1810 of yacc.c  */
#line 552 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MINUTE ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 48:

/* Line 1810 of yacc.c  */
#line 557 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MINUTE ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 49:

/* Line 1810 of yacc.c  */
#line 562 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_MINUTE ((pd_yyval.date), (yyvsp[(1) - (1)].number));
	  }
    break;

  case 50:

/* Line 1810 of yacc.c  */
#line 567 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_SECOND ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 51:

/* Line 1810 of yacc.c  */
#line 572 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_SECOND ((pd_yyval.date), (yyvsp[(1) - (2)].number) * (yyvsp[(2) - (2)].number));
	  }
    break;

  case 52:

/* Line 1810 of yacc.c  */
#line 577 "parsedate.y"
    {
	    DATE_INIT ((pd_yyval.date));
	    SET_SECOND ((pd_yyval.date), (yyvsp[(1) - (1)].number));
	  }
    break;

  case 53:

/* Line 1810 of yacc.c  */
#line 584 "parsedate.y"
    {
	    (pd_yyval.meridian) = MER24;
	  }
    break;

  case 54:

/* Line 1810 of yacc.c  */
#line 588 "parsedate.y"
    {
	    (pd_yyval.meridian) = (yyvsp[(1) - (1)].meridian);
	  }
    break;



/* Line 1810 of yacc.c  */
#line 2246 "parsedate.c"
      default: break;
    }
  /* User semantic actions sometimes alter pd_yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering pd_yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", pd_yyr1[yyn], &pd_yyval, &yyloc);

  YYPOPSTACK (pd_yylen);
  pd_yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = pd_yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = pd_yyr1[yyn];

  pd_yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= pd_yystate && pd_yystate <= YYLAST && pd_yycheck[pd_yystate] == *yyssp)
    pd_yystate = pd_yytable[pd_yystate];
  else
    pd_yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = pd_yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (pd_yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++pd_yynerrs;
#if ! YYERROR_VERBOSE
      pd_yyerror (YY_("syntax error"));
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
        pd_yyerror (yymsgp);
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

      if (pd_yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (pd_yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &pd_yylval);
	  pd_yychar = YYEMPTY;
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
  YYPOPSTACK (pd_yylen);
  pd_yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  pd_yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = pd_yypact[pd_yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && pd_yycheck[yyn] == YYTERROR)
	    {
	      yyn = pd_yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[pd_yystate], yyvsp);
      YYPOPSTACK (1);
      pd_yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = pd_yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  pd_yystate = yyn;
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
  pd_yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (pd_yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (pd_yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &pd_yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (pd_yylen);
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
#line 593 "parsedate.y"


#include <mailutils/types.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <mailutils/util.h>

/* Month and day table. */
static SYMBOL const month_day_tab[] = {
  { "january",	T_MONTH,  1 },
  { "february",	T_MONTH,  2 },
  { "march",	T_MONTH,  3 },
  { "april",	T_MONTH,  4 },
  { "may",	T_MONTH,  5 },
  { "june",	T_MONTH,  6 },
  { "july",	T_MONTH,  7 },
  { "august",	T_MONTH,  8 },
  { "september",T_MONTH,  9 },
  { "sept",	T_MONTH,  9 },
  { "october",	T_MONTH, 10 },
  { "november",	T_MONTH, 11 },
  { "december",	T_MONTH, 12 },
  { "sunday",	T_DAY,   0 },
  { "monday",	T_DAY,   1 },
  { "tuesday",	T_DAY,   2 },
  { "tues",	T_DAY,   2 },
  { "wednesday",T_DAY,   3 },
  { "wednes",	T_DAY,   3 },
  { "thursday",	T_DAY,   4 },
  { "thur",	T_DAY,   4 },
  { "thurs",	T_DAY,   4 },
  { "friday",	T_DAY,   5 },
  { "saturday",	T_DAY,   6 },
  { NULL, 0, 0 }
};

/* Time units table. */
static SYMBOL const units_tab[] = {
  { "year",	T_YEAR_UNIT,	1 },
  { "month",	T_MONTH_UNIT,	1 },
  { "fortnight",T_DAY_UNIT,	14 },
  { "week",	T_DAY_UNIT,	7 },
  { "day",	T_DAY_UNIT,	1 },
  { "hour",	T_HOUR_UNIT,	1 },
  { "minute",	T_MINUTE_UNIT,	1 },
  { "min",	T_MINUTE_UNIT,	1 },
  { "second",	T_SEC_UNIT,	1 },
  { "sec",	T_SEC_UNIT,	1 },
  { NULL, 0, 0 }
};

/* Assorted relative-time words. */
static SYMBOL const other_tab[] = {
  { "tomorrow",	T_MINUTE_UNIT,	1 * 24 * 60 },
  { "yesterday",T_MINUTE_UNIT,	-1 * 24 * 60 },
  { "today",	T_MINUTE_UNIT,	0 },
  { "now",	T_MINUTE_UNIT,	0 },
  { "last",	T_UNUMBER,	-1 },
  { "this",	T_MINUTE_UNIT,	0 },
  { "next",	T_UNUMBER,	1 },
  { "first",	T_UNUMBER,	1 },
/*  { "second",	T_UNUMBER,	2 }, */
  { "third",	T_UNUMBER,	3 },
  { "fourth",	T_UNUMBER,	4 },
  { "fifth",	T_UNUMBER,	5 },
  { "sixth",	T_UNUMBER,	6 },
  { "seventh",	T_UNUMBER,	7 },
  { "eighth",	T_UNUMBER,	8 },
  { "ninth",	T_UNUMBER,	9 },
  { "tenth",	T_UNUMBER,	10 },
  { "eleventh",	T_UNUMBER,	11 },
  { "twelfth",	T_UNUMBER,	12 },
  { "ago",	T_AGO,	        1 },
  { NULL, 0, 0 }
};

/* The timezone table. */
static SYMBOL const tz_tab[] = {
  { "gmt",	T_ZONE,     HOUR ( 0) },	/* Greenwich Mean */
  { "ut",	T_ZONE,     HOUR ( 0) },	/* Universal (Coordinated) */
  { "utc",	T_ZONE,     HOUR ( 0) },
  { "wet",	T_ZONE,     HOUR ( 0) },	/* Western European */
  { "bst",	T_DAYZONE,  HOUR ( 0) },	/* British Summer */
  { "wat",	T_ZONE,     HOUR ( 1) },	/* West Africa */
  { "at",	T_ZONE,     HOUR ( 2) },	/* Azores */
#if	0
  /* For completeness.  BST is also British Summer, and GST is
   * also Guam Standard. */
  { "bst",	T_ZONE,     HOUR ( 3) },	/* Brazil Standard */
  { "gst",	T_ZONE,     HOUR ( 3) },	/* Greenland Standard */
#endif
#if 0
  { "nft",	T_ZONE,     HOUR (3.5) },	/* Newfoundland */
  { "nst",	T_ZONE,     HOUR (3.5) },	/* Newfoundland Standard */
  { "ndt",	T_DAYZONE,  HOUR (3.5) },	/* Newfoundland Daylight */
#endif
  { "ast",	T_ZONE,     HOUR ( 4) },	/* Atlantic Standard */
  { "adt",	T_DAYZONE,  HOUR ( 4) },	/* Atlantic Daylight */
  { "est",	T_ZONE,     HOUR ( 5) },	/* Eastern Standard */
  { "edt",	T_DAYZONE,  HOUR ( 5) },	/* Eastern Daylight */
  { "cst",	T_ZONE,     HOUR ( 6) },	/* Central Standard */
  { "cdt",	T_DAYZONE,  HOUR ( 6) },	/* Central Daylight */
  { "mst",	T_ZONE,     HOUR ( 7) },	/* Mountain Standard */
  { "mdt",	T_DAYZONE,  HOUR ( 7) },	/* Mountain Daylight */
  { "pst",	T_ZONE,     HOUR ( 8) },	/* Pacific Standard */
  { "pdt",	T_DAYZONE,  HOUR ( 8) },	/* Pacific Daylight */
  { "yst",	T_ZONE,     HOUR ( 9) },	/* Yukon Standard */
  { "ydt",	T_DAYZONE,  HOUR ( 9) },	/* Yukon Daylight */
  { "hst",	T_ZONE,     HOUR (10) },	/* Hawaii Standard */
  { "hdt",	T_DAYZONE,  HOUR (10) },	/* Hawaii Daylight */
  { "cat",	T_ZONE,     HOUR (10) },	/* Central Alaska */
  { "ahst",	T_ZONE,     HOUR (10) },	/* Alaska-Hawaii Standard */
  { "nt",	T_ZONE,     HOUR (11) },	/* Nome */
  { "idlw",	T_ZONE,     HOUR (12) },	/* International Date Line West */
  { "cet",	T_ZONE,     -HOUR (1) },	/* Central European */
  { "met",	T_ZONE,     -HOUR (1) },	/* Middle European */
  { "mewt",	T_ZONE,     -HOUR (1) },	/* Middle European Winter */
  { "mest",	T_DAYZONE,  -HOUR (1) },	/* Middle European Summer */
  { "mesz",	T_DAYZONE,  -HOUR (1) },	/* Middle European Summer */
  { "swt",	T_ZONE,     -HOUR (1) },	/* Swedish Winter */
  { "sst",	T_DAYZONE,  -HOUR (1) },	/* Swedish Summer */
  { "fwt",	T_ZONE,     -HOUR (1) },	/* French Winter */
  { "fst",	T_DAYZONE,  -HOUR (1) },	/* French Summer */
  { "eet",	T_ZONE,     -HOUR (2) },	/* Eastern Europe, USSR Zone 1 */
  { "bt",	T_ZONE,     -HOUR (3) },	/* Baghdad, USSR Zone 2 */
#if 0
  { "it",	T_ZONE,     -HOUR (3.5) },/* Iran */
#endif
  { "zp4",	T_ZONE,     -HOUR (4) },	/* USSR Zone 3 */
  { "zp5",	T_ZONE,     -HOUR (5) },	/* USSR Zone 4 */
#if 0
  { "ist",	T_ZONE,     -HOUR (5.5) },/* Indian Standard */
#endif
  { "zp6",	T_ZONE,     -HOUR (6) },	/* USSR Zone 5 */
#if	0
  /* For completeness.  NST is also Newfoundland Standard, and SST is
   * also Swedish Summer. */
  { "nst",	T_ZONE,     -HOUR (6.5) },/* North Sumatra */
  { "sst",	T_ZONE,     -HOUR (7) },	/* South Sumatra, USSR Zone 6 */
#endif	/* 0 */
  { "wast",	T_ZONE,     -HOUR (7) },	/* West Australian Standard */
  { "wadt",	T_DAYZONE,  -HOUR (7) },	/* West Australian Daylight */
#if 0
  { "jt",	T_ZONE,     -HOUR (7.5) },/* Java (3pm in Cronusland!) */
#endif
  { "cct",	T_ZONE,     -HOUR (8) },	/* China Coast, USSR Zone 7 */
  { "jst",	T_ZONE,     -HOUR (9) },	/* Japan Standard, USSR Zone 8 */
#if 0
  { "cast",	T_ZONE,     -HOUR (9.5) },/* Central Australian Standard */
  { "cadt",	T_DAYZONE,  -HOUR (9.5) },/* Central Australian Daylight */
#endif
  { "east",	T_ZONE,     -HOUR (10) },	/* Eastern Australian Standard */
  { "eadt",	T_DAYZONE,  -HOUR (10) },	/* Eastern Australian Daylight */
  { "gst",	T_ZONE,     -HOUR (10) },	/* Guam Standard, USSR Zone 9 */
  { "nzt",	T_ZONE,     -HOUR (12) },	/* New Zealand */
  { "nzst",	T_ZONE,     -HOUR (12) },	/* New Zealand Standard */
  { "nzdt",	T_DAYZONE,  -HOUR (12) },	/* New Zealand Daylight */
  { "idle",	T_ZONE,     -HOUR (12) },	/* International Date Line
						   East */
  {  NULL, 0, 0  }
};

/* Military timezone table. */
static SYMBOL const mil_tz_tab[] = {
  { "a",	T_ZONE,	HOUR (  1) },
  { "b",	T_ZONE,	HOUR (  2) },
  { "c",	T_ZONE,	HOUR (  3) },
  { "d",	T_ZONE,	HOUR (  4) },
  { "e",	T_ZONE,	HOUR (  5) },
  { "f",	T_ZONE,	HOUR (  6) },
  { "g",	T_ZONE,	HOUR (  7) },
  { "h",	T_ZONE,	HOUR (  8) },
  { "i",	T_ZONE,	HOUR (  9) },
  { "k",	T_ZONE,	HOUR ( 10) },
  { "l",	T_ZONE,	HOUR ( 11) },
  { "m",	T_ZONE,	HOUR ( 12) },
  { "n",	T_ZONE,	HOUR (- 1) },
  { "o",	T_ZONE,	HOUR (- 2) },
  { "p",	T_ZONE,	HOUR (- 3) },
  { "q",	T_ZONE,	HOUR (- 4) },
  { "r",	T_ZONE,	HOUR (- 5) },
  { "s",	T_ZONE,	HOUR (- 6) },
  { "t",	T_ZONE,	HOUR (- 7) },
  { "u",	T_ZONE,	HOUR (- 8) },
  { "v",	T_ZONE,	HOUR (- 9) },
  { "w",	T_ZONE,	HOUR (-10) },
  { "x",	T_ZONE,	HOUR (-11) },
  { "y",	T_ZONE,	HOUR (-12) },
  { "z",	T_ZONE,	HOUR (  0) },
  { NULL, 0, 0 }
};




/* ARGSUSED */
static int
pd_yyerror (char *s MU_ARG_UNUSED)
{
  return 0;
}

static int
norm_hour (int hours, MERIDIAN meridian)
{
  switch (meridian)
    {
    case MER24:
      if (hours < 0 || hours > 23)
	return -1;
      return hours;
      
    case MERam:
      if (hours < 1 || hours > 12)
	return -1;
      if (hours == 12)
	hours = 0;
      return hours;
      
    case MERpm:
      if (hours < 1 || hours > 12)
	return -1;
      if (hours == 12)
	hours = 0;
      return hours + 12;
      
    default:
      abort ();
    }
  /* NOTREACHED */
}

static int
norm_year (int year)
{
  if (year < 0)
    year = -year;
  
  /* XPG4 suggests that years 00-68 map to 2000-2068, and
     years 69-99 map to 1969-1999.  */
  if (year < 69)
    year += 2000;
  else if (year < 100)
    year += 1900;

  return year;
}

static int
sym_lookup (char *buff)
{
  register char *p;
  register char *q;
  register const SYMBOL *tp;
  int i;
  int abbrev;
  
  /* Make it lowercase. */
  mu_strlower (buff);
  
  if (strcmp (buff, "am") == 0 || strcmp (buff, "a.m.") == 0)
    {
      pd_yylval.meridian = MERam;
      return T_MERIDIAN;
    }
  if (strcmp (buff, "pm") == 0 || strcmp (buff, "p.m.") == 0)
    {
      pd_yylval.meridian = MERpm;
      return T_MERIDIAN;
    }
  
  /* See if we have an abbreviation for a month. */
  if (strlen (buff) == 3)
    abbrev = 1;
  else if (strlen (buff) == 4 && buff[3] == '.')
    {
      abbrev = 1;
      buff[3] = '\0';
    }
  else
    abbrev = 0;

  for (tp = month_day_tab; tp->name; tp++)
    {
      if (abbrev)
	{
	  if (strncmp (buff, tp->name, 3) == 0)
	    {
	      pd_yylval.number = tp->value;
	      return tp->type;
	    }
	}
      else if (strcmp (buff, tp->name) == 0)
	{
	  pd_yylval.number = tp->value;
	  return tp->type;
	}
    }

  for (tp = tz_tab; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
        pd_yylval.tz.name = tp->name;
	pd_yylval.tz.delta = tp->value;
	return tp->type;
      }

  if (strcmp (buff, "dst") == 0)
    return T_DST;

  for (tp = units_tab; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
	pd_yylval.number = tp->value;
	return tp->type;
      }

  /* Strip off any plural and try the units table again. */
  i = strlen (buff) - 1;
  if (buff[i] == 's')
    {
      buff[i] = '\0';
      for (tp = units_tab; tp->name; tp++)
	if (strcmp (buff, tp->name) == 0)
	  {
	    pd_yylval.number = tp->value;
	    return tp->type;
	  }
      buff[i] = 's';		/* Put back for "this" in other_tab. */
    }

  for (tp = other_tab; tp->name; tp++)
    if (strcmp (buff, tp->name) == 0)
      {
	pd_yylval.number = tp->value;
	return tp->type;
      }

  /* Military timezones. */
  if (buff[1] == '\0' && ISALPHA ((unsigned char) *buff))
    {
      for (tp = mil_tz_tab; tp->name; tp++)
	if (strcmp (buff, tp->name) == 0)
	  {
            pd_yylval.tz.name = tp->name;
	    pd_yylval.tz.delta = tp->value;
	    return tp->type;
	  }
    }

  /* Drop out any periods and try the timezone table again. */
  for (i = 0, p = q = buff; *q; q++)
    if (*q != '.')
      *p++ = *q;
    else
      i++;
  *p = '\0';
  if (i)
    for (tp = tz_tab; tp->name; tp++)
      if (strcmp (buff, tp->name) == 0)
	{
	  pd_yylval.number = tp->value;
	  return tp->type;
	}

  return T_ID;
}

static int
pd_yylex (void)
{
  register unsigned char c;
  register char *p;
  char buff[20];
  int count;
  int sign;

  for (;;)
    {
      while (ISSPACE ((unsigned char) *yyinput))
	yyinput++;

      if (ISDIGIT (c = *yyinput) || c == '-' || c == '+')
	{
	  if (c == '-' || c == '+')
	    {
	      sign = c == '-' ? -1 : 1;
	      if (!ISDIGIT (*++yyinput))
		/* skip the '-' sign */
		continue;
	    }
	  else
	    sign = 0;
	  for (pd_yylval.number = 0; ISDIGIT (c = *yyinput++);)
	    pd_yylval.number = 10 * pd_yylval.number + c - '0';
	  yyinput--;
	  if (sign < 0)
	    pd_yylval.number = -pd_yylval.number;
	  return sign ? T_SNUMBER : T_UNUMBER;
	}
      if (ISALPHA (c))
	{
	  for (p = buff; (c = *yyinput++, ISALPHA (c)) || c == '.';)
	    if (p < &buff[sizeof buff - 1])
	      *p++ = c;
	  *p = '\0';
	  yyinput--;
	  return sym_lookup (buff);
	}
      if (c != '(')
	return *yyinput++;
      count = 0;
      do
	{
	  c = *yyinput++;
	  if (c == '\0')
	    return c;
	  if (c == '(')
	    count++;
	  else if (c == ')')
	    count--;
	}
      while (count > 0);
    }
}

#define TM_YEAR_ORIGIN 1900

/* Yield A - B, measured in seconds.  */
static long
difftm (struct tm *a, struct tm *b)
{
  int ay = a->tm_year + (TM_YEAR_ORIGIN - 1);
  int by = b->tm_year + (TM_YEAR_ORIGIN - 1);
  long days = (
  /* difference in day of year */
		a->tm_yday - b->tm_yday
  /* + intervening leap days */
		+ ((ay >> 2) - (by >> 2))
		- (ay / 100 - by / 100)
		+ ((ay / 100 >> 2) - (by / 100 >> 2))
  /* + difference in years * 365 */
		+ (long) (ay - by) * 365
  );
  return (60 * (60 * (24 * days + (a->tm_hour - b->tm_hour))
		+ (a->tm_min - b->tm_min))
	  + (a->tm_sec - b->tm_sec));
}

int
mu_parse_date_dtl (const char *p, const time_t *now, 
		   time_t *rettime,
		   struct tm *rettm,
		   struct mu_timezone *rettz,
		   int *retflags)
{
  struct tm tm, tm0, *tmp;
  time_t start;

  yyinput = p;
  start = now ? *now : time ((time_t *) NULL);
  tmp = localtime (&start);
  if (!tmp)
    return -1;

  memset (&tm, 0, sizeof tm);
  tm.tm_isdst = tmp->tm_isdst;

  if (pd_yyparse ())
    return -1;
  
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_YEAR))
    __SET_YEAR (pd.date, tmp->tm_year + TM_YEAR_ORIGIN, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_MONTH))
    __SET_MONTH (pd.date, tmp->tm_mon + 1, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_DAY))
    __SET_DAY (pd.date, tmp->tm_mday, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_HOUR))
    __SET_HOUR (pd.date, tmp->tm_hour, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_MERIDIAN))
    __SET_MERIDIAN (pd.date, MER24, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_MINUTE))
    __SET_MINUTE (pd.date, tmp->tm_min, return -1);
  if (!MASK_IS_SET (pd.date.mask, MU_PD_MASK_SECOND))
    __SET_SECOND (pd.date, tmp->tm_sec, return -1);
  
  tm.tm_year = norm_year (pd.date.year) - TM_YEAR_ORIGIN + pd.rel.year;
  tm.tm_mon = pd.date.month - 1 + pd.rel.month;
  tm.tm_mday = pd.date.day + pd.rel.day;
  if (MASK_TEST (pd.date.mask, MU_PD_MASK_TIME)
      || (pd.rel.mask && !MASK_TEST (pd.date.mask, MU_PD_MASK_DATE)
	  && !MASK_TEST (pd.date.mask, MU_PD_MASK_DOW)))
    {
      tm.tm_hour = norm_hour (pd.date.hour, pd.date.meridian);
      if (tm.tm_hour < 0)
	return -1;
      tm.tm_min = pd.date.minute;
      tm.tm_sec = pd.date.second;
    }
  else
    {
      tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    }
  tm.tm_hour += pd.rel.hour;
  tm.tm_min += pd.rel.minute;
  tm.tm_sec += pd.rel.second;

  /* Let mktime deduce tm_isdst if we have an absolute timestamp,
     or if the relative timestamp mentions days, months, or years.  */
  if (MASK_TEST (pd.date.mask, MU_PD_MASK_DATE | MU_PD_MASK_DOW | MU_PD_MASK_TIME)
      || MASK_TEST (pd.rel.mask, MU_PD_MASK_DOW | MU_PD_MASK_MONTH | MU_PD_MASK_YEAR))
    tm.tm_isdst = -1;

  tm0 = tm;

  start = mktime (&tm);

  if (start == (time_t) -1)
    {

      /* Guard against falsely reporting errors near the time_t boundaries
         when parsing times in other time zones.  For example, if the min
         time_t value is 1970-01-01 00:00:00 UTC and we are 8 hours ahead
         of UTC, then the min localtime value is 1970-01-01 08:00:00; if
         we apply mktime to 1970-01-01 00:00:00 we will get an error, so
         we apply mktime to 1970-01-02 08:00:00 instead and adjust the time
         zone by 24 hours to compensate.  This algorithm assumes that
         there is no DST transition within a day of the time_t boundaries.  */
      if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
	{
	  tm = tm0;
	  if (tm.tm_year <= EPOCH - TM_YEAR_ORIGIN)
	    {
	      tm.tm_mday++;
	      pd.date.tz -= 24 * 60;
	    }
	  else
	    {
	      tm.tm_mday--;
	      pd.date.tz += 24 * 60;
	    }
	  start = mktime (&tm);
	}

      if (start == (time_t) -1)
	return -1;
    }

  if (MASK_TEST (pd.date.mask, MU_PD_MASK_DOW)
      && !MASK_TEST (pd.date.mask, MU_PD_MASK_DATE))
    {
      tm.tm_mday += ((pd.date.number - tm.tm_wday + 7) % 7
		     + 7 * (pd.date.ordinal - (0 < pd.date.ordinal)));
      start = mktime (&tm);
      if (start == (time_t) -1)
	return -1;
    }
  
  if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
    {
      long delta;
      struct tm *gmt = gmtime (&start);
      if (gmt)
	{
	  delta = pd.date.tz * 60L + difftm (&tm, gmt);
	  if ((start + delta < start) != (delta < 0))
	    return -1;		/* time_t overflow */
	  start += delta;
	}
    }

  if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
    {
      pd.date.tz = - pd.date.tz * 60L;
      if (!pd.date.tzname)
	pd.date.tzname = tm.tm_isdst != -1 ? tzname[tm.tm_isdst] : NULL;
#if HAVE_STRUCT_TM_TM_GMTOFF
      tm.tm_gmtoff = pd.date.tz;
#endif
#if HAVE_STRUCT_TM_TM_ZONE 	
      tm.tm_zone = pd.date.tzname;
#endif
    }
  if (rettime)
    *rettime = start;
  if (rettm)
    *rettm = tm;
  if (rettz)
    {
      if (MASK_TEST (pd.date.mask, MU_PD_MASK_TZ))
	{
	  rettz->utc_offset = pd.date.tz;
	  rettz->tz_name = pd.date.tzname;
	}
      else
	{
	  mu_datetime_tz_local (rettz);
	}
    }
  if (retflags)
    *retflags = pd.date.mask;
  return 0;
}

int
mu_parse_date (const char *p, time_t *rettime, const time_t *now)
{
  return mu_parse_date_dtl (p, now, rettime, NULL, NULL, NULL);
}

#ifdef STANDALONE
int
main (int argc, char *argv[])
{
  char buff[MAX_BUFF_LEN + 1];
  time_t d;

  if (argc > 1 && strcmp (argv[1], "-d") == 0)
    pd_yydebug++;
  printf ("Enter date, or blank line to exit.\n\t> ");
  fflush (stdout);

  buff[MAX_BUFF_LEN] = 0;
  while (fgets (buff, MAX_BUFF_LEN, stdin) && buff[0])
    {
      if (mu_parse_date (buff, &d, NULL))
	printf ("Bad format - couldn't convert.\n");
      else
	printf ("%s", ctime (&d));
      printf ("\t> ");
      fflush (stdout);
    }
  exit (0);
  /* NOTREACHED */
}

#endif

