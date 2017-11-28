/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2006-2007, 2010-2012, 2014-2017 Free Software
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

#define MH_FMT_DEFAULT 0
#define MH_FMT_RALIGN  0x1000
#define MH_FMT_ZEROPAD 0x2000
#define MH_FMT_COMPWS  0x4000
#define MH_WIDTH_MASK  0x0fff

enum mh_opcode
{
  /* Stop. Format: mhop_stop */
  mhop_stop,
  /* Unconditional branch
     Format: mhop_branch offset */
  mhop_branch,
  /* Branch if num reg is zero.
     Format: mhop_brz_num dest-off */
  mhop_brzn,
  /* Branch if str reg is zero.
     Format: mhop_brz_str dest-off */
  mhop_brzs,

  /* Set numeric register
     Format: mhop_setn val */
  mhop_setn,
  
  /* Set string register
     Format: mhop_sets reg length string */
  mhop_sets,

  /* Move value bewtween two numeric registers
     Format: mhop_movn dest src */
  mhop_movn,

  /* Move value bewtween two string registers
     Format: mhop_movs dest src */
  mhop_movs,
  
  /* Load component value into a string register 
     Format: mhop_load reg string */
  mhop_ldcomp,

  /* Load first width bytes of message body contents into a string register.
     Format: mhop_body reg */
  mhop_ldbody,
  
  /* Call a function.
     Format: mhop_call function-pointer */
  mhop_call,

  /* Convert string register to number reg
     Format: mhop_atoi
   */
  mhop_atoi,
  
  /* Convert numeric register to string
     Format: mhop_itoa */
  mhop_itoa,

  /* Print num reg */
  mhop_printn,

  /* Print str reg */
  mhop_prints,

  /* Print literal
     Format: mhop_printlit length string */
  mhop_printlit,
  
  /* Set format specification.
     Format: mhop_fmtspec number */
  mhop_fmtspec,

  /* Push numeric register */
  mhop_pushn,
  /* Pop numeric register */
  mhop_popn,
  /* Exchange top of stack value and numeric register */
  mhop_xchgn,
};    

enum regid { R_REG, R_ARG, R_ACC };
#define MH_NREG 3

enum mh_type
{
  mhtype_none,
  mhtype_num,
  mhtype_str
};

typedef enum mh_opcode mh_opcode_t;

struct mh_machine;
typedef void (*mh_builtin_fp) (struct mh_fvm *);

typedef union {
  mh_opcode_t opcode;
  mh_builtin_fp builtin;
  long num;
  void *ptr;
  size_t size;
  char str[1]; /* Any number of characters follows */
} mh_instr_t;

#define MHI_OPCODE(m) (m).opcode
#define MHI_BUILTIN(m) (m).builtin
#define MHI_NUM(m) (m).num
#define MHI_PTR(m) (m).ptr
#define MHI_STR(m) (m).str

struct mh_format
{
  size_t progmax;          /* Size of allocated program*/
  size_t progcnt;          /* Actual number of elements used */
  mh_instr_t *prog;        /* Program itself */
  /* The tree and pool members are filled only if mh_format_parse
     was called with MH_FMT_PARSE_TREE flag */
  struct node *tree;       /* Parse tree */  
  mu_opool_t pool;         /* Literal pool */
};

#define MHA_DEFAULT       0
#define MHA_IGNOREFMT     0x001
#define MHA_NOPRINT       0x002 
#define MHA_PRINT_MASK    0x003

#define MHA_OPTARG        0x004
#define MHA_OPTARG_NIL    0x008
#define MHA_LITERAL       0x010
#define MHA_VOID          0x020
#define MHA_SPECIAL       0x040
#define MHA_ACC           0x080

typedef struct mh_builtin mh_builtin_t;

struct mh_builtin
{
  char *name;
  mh_builtin_fp fun;
  enum mh_type type;
  enum mh_type argtype;
  int flags;
};

struct mh_string
{
  size_t size;
  char *ptr;
};
  
struct mh_fvm
{
  long num[MH_NREG];              /* numeric registers */
  struct mh_string str[MH_NREG];  /* string registers */

  long *numstack;           /* Stack of numeric value */
  size_t maxstack;          /* Stack capacity */
  size_t tos;               /* Top of stack (next free slot) */
  
  size_t pc;                /* Program counter */
  size_t progcnt;           /* Size of allocated program*/
  mh_instr_t *prog;         /* Program itself */
  int stop;                 /* Stop execution immediately */

  size_t width;             /* Output line width */
  size_t ind;               /* Output line index */
  mu_stream_t output;       /* Output stream */
  int flags;
  
  mu_list_t addrlist;       /* The list of email addresses output this far */
  int fmtflags;             /* Current formatting flags */

  mu_message_t message;     /* Current message */
};

mh_builtin_t *mh_lookup_builtin (char *name, size_t len);
void mh_print_fmtspec (int fmtspec);

