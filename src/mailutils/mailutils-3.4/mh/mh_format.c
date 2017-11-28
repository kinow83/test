/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2003, 2005-2007, 2009-2012, 2014-2017 Free
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

/* This module implements execution of MH format strings. */

#include <mh.h>
#include <mh_format.h>
#include <mailutils/mime.h>
#include <mailutils/opool.h>

#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "mbiter.h"
#include "mbchar.h"
#include "mbswidth.h"


/* String functions */

#define MH_STRING_INITIALIZER { 0, NULL }

static inline void
mh_string_init (struct mh_string *s)
{
  s->size = 0;
  s->ptr = NULL;
}

static void
mh_string_free (struct mh_string *s)
{
  free (s->ptr);
  s->size = 0;
  s->ptr = NULL;
}

static void
mh_string_realloc (struct mh_string *s, size_t length)
{
  if (length > s->size)
    {
      s->ptr = mu_realloc (s->ptr, length);
      s->ptr[length-1] = 0;
      s->size = length;
    }
}

static inline int
mh_string_is_null (struct mh_string *s)
{
  return s->ptr == NULL || s->ptr[0] == 0;
}

static inline size_t
mh_string_length (struct mh_string *s)
{
  return mh_string_is_null (s) ? 0 : strlen (s->ptr);
}

static inline char const *
mh_string_value (struct mh_string *s)
{
  return mh_string_is_null (s) ? "" : s->ptr;
}

static inline void
mh_string_clear (struct mh_string *s)
{
  if (s->ptr)
    s->ptr[0] = 0;
}

static void
mh_string_load (struct mh_string *s, char const *str)
{
  if (!str)
    mh_string_clear (s);
  else
    {
      mh_string_realloc (s, strlen (str) + 1);
      strcpy (s->ptr, str);
    }
}

static void
mh_string_copy (struct mh_fvm *mach, enum regid dst, enum regid src)
{
  mh_string_load (&mach->str[dst], mach->str[src].ptr);
}


static char *_get_builtin_name (mh_builtin_fp ptr);

static inline size_t
output_width (struct mh_fvm *mach)
{
  if (mach->width < mach->ind)
    return 0;
  return mach->width - mach->ind;
}

/* Return the length (number of octets) of a substring of
   string STR of length LEN, such that it contains NCOL
   multibyte characters. */
int
mbsubstrlen (char const *str, size_t len, size_t ncol)
{
  int ret = 0;
  mbi_iterator_t iter;

  if (ncol <= 0)
    return 0;
  
  for (mbi_init (iter, str, len);
       ncol && mbi_avail (iter);
       ncol--, mbi_advance (iter))
    ret += mb_len (mbi_cur (iter));
  return ret;
}

/* Return the number of multibyte characters in the first LEN bytes
   of character string STRING.  */
size_t
mbsnlen (char const *str, size_t len)
{
  int ret = 0;
  mbi_iterator_t iter;

  for (mbi_init (iter, str, len); mbi_avail (iter); mbi_advance (iter))
    ret++;
  return ret;
}

/* Compress whitespace in a string (multi-byte) */
static void
str_compress_ws (char *str)
{
  unsigned char *p, *q;
  size_t size = strlen (str);
  mbi_iterator_t iter;
  int space = 0;
  
  for (p = q = (unsigned char*) str,
	 mbi_init (iter, str, size);
       mbi_avail (iter);
       mbi_advance (iter))
    {
      if (mb_isspace (mbi_cur (iter)))
	{
	  if (!space)
	    *p++ = ' ';
	  space++;
	  continue;
	}
      else if (space)
	space = 0;

      if (mb_isprint (mbi_cur (iter)))
	{
	  size_t len = mb_len (mbi_cur (iter));
	  memcpy (p, mb_ptr (mbi_cur (iter)), len);
	  p += len;
	}
    }
  *p = 0;
}

static inline void
compress_ws (struct mh_fvm *mach, char *str)
{
  if (mach->fmtflags & MH_FMT_COMPWS)
    str_compress_ws (str);
}

static void
put_string (struct mh_fvm *mach, char const *str, int len)
{
  if (len == 0)
    return;
  mu_stream_write (mach->output, str, len, NULL);
  mach->ind += mbsnwidth (str, len, 0);
}

static void
print_hdr_segment (struct mh_fvm *mach, char const *str, size_t len)
{
  if (!len)
    len = strlen (str);

  if (mbsnlen (str, len) < mach->width)
    put_string (mach, str, len);
  else
    {
      while (1)
	{
	  mbi_iterator_t iter;
	  size_t rest = output_width (mach);
	  size_t width = mbsnlen (str, len);
	  size_t off, size;
	  
	  if (width <= rest)
	    {
	      put_string (mach, str, len);
	      break;
	    }

	  size = off = 0;
	  for (mbi_init (iter, str, len);
	       mbi_avail (iter);
	       mbi_advance (iter))
	    {
	      if (mb_isspace (mbi_cur (iter)))
		off = size;
	      size += mb_len (mbi_cur (iter));
	    }

	  if (off > 0)
	    {
	      put_string (mach, str, off);
	      put_string (mach, "\n        ", 9);
	      mach->ind = 8;
	      str += off;
	      len -= off;
	    }
	  else
	    {
	      size = mbsubstrlen (str, len, rest);
	      put_string (mach, str, len);
	      break;
	    }
	}
    }
}

static void
print_hdr_string (struct mh_fvm *mach, char const *str)
{
  char const *p;
  
  if (!str)
    str = "";

  p = strchr (str, '\n');
  while (p)
    {
      print_hdr_segment (mach, str, p - str + 1);
      mach->ind = 0;
      str = p + 1;
      p = strchr (str, '\n');
    }
    
  if (str[0])
    print_hdr_segment (mach, str, 0);
}

static void
print_simple_segment (struct mh_fvm *mach, size_t width,
		      char const *str, size_t len)
{
  size_t rest;

  if (!str)
    str = "";

  if (!len)
    len = strlen (str);

  if (!width)
    width = mach->width;

  rest = output_width (mach);
  if (rest == 0)
    {
      if (len == 1 && str[0] == '\n')
	put_string (mach, str, len);
      return;
    }
  
  put_string (mach, str, mbsubstrlen (str, len, rest));
}

static void
print_string (struct mh_fvm *mach, size_t width, char const *str)
{
  char *p;
  
  if (!str)
    str = "";

  if (!width)
    width = mach->width;

  p = strchr (str, '\n');
  while (p)
    {
      print_simple_segment (mach, width, str, p - str + 1);
      mach->ind = 0;
      str = p + 1;
      p = strchr (str, '\n');
    }
    
  if (str[0])
    print_simple_segment (mach, width, str, 0);
}
  
static void
print_fmt_segment (struct mh_fvm *mach, size_t fmtwidth, char const *str,
		   size_t len)
{
  size_t width = mbsnlen (str, len);

  if (fmtwidth && width > fmtwidth)
    {
      len = mbsubstrlen (str, len, fmtwidth);
      width = fmtwidth;
    }
  else
    len = mbsubstrlen (str, len, output_width (mach));
  
  put_string (mach, str, len);

  if (fmtwidth > width)
    {
      fmtwidth -= width;
      mach->ind += fmtwidth;
      while (fmtwidth--)
	mu_stream_write (mach->output, " ", 1, NULL);
    }
}

static void
print_fmt_string (struct mh_fvm *mach, size_t fmtwidth, char const *str)
{
  char *p;
  while ((p = strchr (str, '\n')))
    {
      print_fmt_segment (mach, fmtwidth, str, p - str);
      mu_stream_write (mach->output, "\n", 1, NULL);
      mach->ind = 0;
      str = p + 1;
    }
  if (str[0])
    print_fmt_segment (mach, fmtwidth, str, strlen (str));
}

static void
reset_fmt_defaults (struct mh_fvm *mach)
{
  const char *p;
  
  mach->fmtflags = 0;
  p = mh_global_profile_get ("Compress-WS", "yes");
  if (p && (mu_c_strcasecmp (p, "yes") == 0
	    || mu_c_strcasecmp (p, "true") == 0))
    mach->fmtflags |= MH_FMT_COMPWS;
}

static void
format_num (struct mh_fvm *mach, long num)
{
  int n;
  char buf[64];
  char *ptr;
  int fmtwidth = mach->fmtflags & MH_WIDTH_MASK;
  char padchar = mach->fmtflags & MH_FMT_ZEROPAD ? '0' : ' ';
	    
  n = snprintf (buf, sizeof buf, "%ld", num);

  if (fmtwidth)
    {
      if (n > fmtwidth)
	{
	  ptr = buf + n - fmtwidth;
	  *ptr = '?';
	}
      else
	{
	  int i;
	  ptr = buf;
	  for (i = n; i < fmtwidth && mach->ind < mach->width;
	       i++, mach->ind++)
	    mu_stream_write (mach->output, &padchar, 1, NULL);
	}
    }
  else
    ptr = buf;

  print_string (mach, 0, ptr);
  reset_fmt_defaults (mach);
}

static void
format_str (struct mh_fvm *mach, char const *str)
{
  if (!str)
    str = "";
  if (mach->fmtflags)
    {
      int len = strlen (str);
      int fmtwidth = mach->fmtflags & MH_WIDTH_MASK;
      char padchar = ' ';
				
      if (mach->fmtflags & MH_FMT_RALIGN)
	{
	  int i, n;
	  
	  n = fmtwidth - len;
	  for (i = 0; i < n && mach->ind < mach->width;
	       i++, mach->ind++, fmtwidth--)
	    mu_stream_write (mach->output, &padchar, 1, NULL);
	}
	      
      print_fmt_string (mach, fmtwidth, str);
      reset_fmt_defaults (mach);
    }
  else
    print_string (mach, 0, str);
}

static int
addr_cmp (void *item, void *data)
{
  mu_address_t a = item;
  mu_address_t b = data;
  size_t i, count;
  int rc = 0;
  
  mu_address_get_count (a, &count);
  for (i = 1; rc == 0 && i <= count; i++)
    {
      const char *str;
      if (mu_address_sget_email (a, i, &str) || str)
	continue;
      rc = mu_address_contains_email (b, str);
    }
  return rc ? MU_ERR_USER0 : 0;
}

static int
addrlist_lookup (mu_list_t list, mu_address_t addr)
{
  return mu_list_foreach (list, addr_cmp, addr);
}

static int
addr_free (void *item, void *data)
{
  mu_address_t addr = item;
  mu_address_destroy (&addr);
  return 0;
}

static void
addrlist_destroy (mu_list_t *list)
{
  mu_list_foreach (*list, addr_free, NULL);
  mu_list_destroy (list);
}

/* Execute pre-compiled format on message msg with number msgno.
 */
void
mh_fvm_run (mh_fvm_t mach, mu_message_t msg)
{
  mach->message = msg;

  reset_fmt_defaults (mach);
  mu_list_clear (mach->addrlist);
  mh_string_init (&mach->str[R_REG]);
  mh_string_init (&mach->str[R_ARG]);
  mh_string_init (&mach->str[R_ACC]);

  mach->pc = 1;
  mach->stop = 0;
  mach->ind = 0;
  mach->tos = 0;
  mach->maxstack = 0;
  mach->numstack = 0;
  while (!mach->stop)
    {
      mh_opcode_t opcode;
      
      switch (opcode = MHI_OPCODE (mach->prog[mach->pc++]))
	{
	case mhop_stop:
	  mach->stop = 1;
	  break;

	case mhop_branch:
	  mach->pc += MHI_NUM (mach->prog[mach->pc]);
	  break;

	case mhop_brzn:
	  {
	    int res = mach->num[R_REG] == 0;
	    if (res)
	      mach->pc += MHI_NUM (mach->prog[mach->pc]);
	    else
	      mach->pc++;
	    mach->num[R_REG] = !res;
	  }
	  break;

	case mhop_brzs:
	  {
	    int res = mh_string_is_null (&mach->str[R_REG]);
	    if (res)
	      mach->pc += MHI_NUM (mach->prog[mach->pc]);
	    else
	      mach->pc++;
	    mach->num[R_REG] = !res;
	  }
	  break;
	  
	case mhop_setn:
	  {
	    long reg = MHI_NUM (mach->prog[mach->pc++]);
	    mach->num[reg] = MHI_NUM (mach->prog[mach->pc++]);
	  }
	  break;

	case mhop_sets:
	  {
	    long reg = MHI_NUM (mach->prog[mach->pc++]);
	    size_t skip = MHI_NUM (mach->prog[mach->pc++]);
	    char const *str = MHI_STR (mach->prog[mach->pc]);

	    mach->pc += skip;
	    
	    mh_string_load (&mach->str[reg], str);
	  }
	  break;

	case mhop_movn:
	  {
	    long dst = MHI_NUM (mach->prog[mach->pc++]);
	    long src = MHI_NUM (mach->prog[mach->pc++]);
	    mach->num[dst] = mach->num[src];
	  }
	  break;

	case mhop_movs:
	  {
	    long dst = MHI_NUM (mach->prog[mach->pc++]);
	    long src = MHI_NUM (mach->prog[mach->pc++]);
	    mh_string_copy (mach, dst, src);
	    /* FIXME: perhaps copy, not move? */
	  }
	  break;

	case mhop_pushn:
	  if (mach->tos == mach->maxstack)
	    mach->numstack = mu_2nrealloc (mach->numstack, &mach->maxstack,
					   sizeof (mach->numstack[0]));
	  mach->numstack[mach->tos++] = mach->num[R_REG];
	  break;

	case mhop_popn:
	  assert (mach->tos > 0);
	  mach->num[R_REG] = mach->numstack[--mach->tos];
	  break;

	case mhop_xchgn:
	  assert (mach->tos > 0);
	  {
	    long t = mach->numstack[mach->tos-1];
	    mach->numstack[mach->tos-1] = mach->num[R_REG];
	    mach->num[R_REG] = t;
	  }
	  break;
	  
	case mhop_ldcomp:
	  {
	    long reg = MHI_NUM (mach->prog[mach->pc++]);
	    size_t skip = MHI_NUM (mach->prog[mach->pc++]);
	    char const *comp = MHI_STR (mach->prog[mach->pc]);
	    mu_header_t hdr = NULL;
	    char *value = NULL;

	    mach->pc += skip;

	    mu_message_get_header (mach->message, &hdr);
	    mu_header_aget_value_unfold (hdr, comp, &value);
	    mh_string_clear (&mach->str[reg]);
	    if (value)
	      {
      		compress_ws (mach, value);
		mh_string_load (&mach->str[reg], value);
		free (value);
	      }
	  }
	  break;

	case mhop_ldbody:
	  {
	    long reg = MHI_NUM (mach->prog[mach->pc++]);
	    mu_body_t body = NULL;
	    mu_stream_t stream = NULL;
	    size_t size = 0;
	    size_t rest = output_width (mach);

	    mh_string_clear (&mach->str[reg]);

	    mu_message_get_body (mach->message, &body);
	    mu_body_size (body, &size);
	    if (size == 0)
	      break;
	    mu_body_get_streamref (body, &stream);
	    if (stream)
	      {
		if (size > rest)
		  size = rest;

		mh_string_realloc (&mach->str[reg], size + 1);
		mu_stream_read (stream, mach->str[reg].ptr, size, NULL);
		mach->str[reg].ptr[size] = 0;
		compress_ws (mach, mach->str[reg].ptr);

		mu_stream_destroy (&stream);
	      }
	  }
	  break;

	case mhop_call:
	  MHI_BUILTIN (mach->prog[mach->pc++]) (mach);
	  break;

	  /* Convert string register to number */
	case mhop_atoi:
	  {
	    if (mh_string_is_null (&mach->str[R_REG]))
	      mach->num[R_REG] = 0;
	    else
	      mach->num[R_REG] = strtoul (mach->str[R_REG].ptr, NULL, 0);
	  }
	  break;
  
	  /* Convert numeric register to string */
	case mhop_itoa:
	  {
	    mu_asnprintf (&mach->str[R_REG].ptr, &mach->str[R_REG].size,
			  "%lu", mach->num[R_REG]);
	  }
	  break;

	case mhop_printn:
	  format_num (mach, mach->num[R_REG]);
	  break;

	case mhop_prints:
	  format_str (mach, mach->str[R_REG].ptr);
	  break;

	case mhop_printlit:
	  {
	    size_t skip = MHI_NUM (mach->prog[mach->pc++]);
	    char const *str = MHI_STR (mach->prog[mach->pc]);
	    format_str (mach, str);
	    mach->pc += skip;
	  }
	  break;
	  
	case mhop_fmtspec:
	  mach->fmtflags = MHI_NUM (mach->prog[mach->pc++]);
	  break;

	default:
	  mu_error (_("INTERNAL ERROR: Unknown opcode: %x"), opcode);
	  abort ();
	}
    }
  if ((mach->flags & MH_FMT_FORCENL) && mach->ind != 0)
    put_string (mach, "\n", 1);
}

static int
msg_uid_1 (mu_message_t msg MU_ARG_UNUSED, size_t *ret)
{
  if (!ret)
    return MU_ERR_OUT_PTR_NULL;
  *ret = 1;
  return 0;
}
    
int
mh_format_str (mh_format_t fmt, char *str, size_t width, char **pstr)
{
  mu_message_t msg = NULL;
  mu_header_t hdr = NULL;
  int rc = 0;
  mu_stream_t outstr;
  char *buf;
  mu_off_t size;

  mh_fvm_t fvm;
  
  MU_ASSERT (mu_message_create (&msg, NULL));
  MU_ASSERT (mu_message_get_header (msg, &hdr));
  MU_ASSERT (mu_header_set_value (hdr, "text", str, 1));
  MU_ASSERT (mu_memory_stream_create (&outstr, MU_STREAM_RDWR));
  MU_ASSERT (mu_message_set_uid (msg, msg_uid_1, NULL));
    
  mh_fvm_create (&fvm, 0);
  mh_fvm_set_output (fvm, outstr);
  mh_fvm_set_width (fvm, width);
  mh_fvm_set_format (fvm, fmt);
  mh_fvm_run (fvm, msg);
  mh_fvm_destroy (&fvm);
	      
  MU_ASSERT (mu_stream_size (outstr, &size));
  buf = mu_alloc (size + 1);
  MU_ASSERT (mu_stream_seek (outstr, 0, MU_SEEK_SET, NULL));
  MU_ASSERT (mu_stream_read (outstr, buf, size, NULL));

  *pstr = buf;
  
  mu_message_destroy (&msg, NULL);
  mu_stream_destroy (&outstr);
  
  return rc;
}

/* Built-in functions */

/* Handler for unimplemented functions */
static void
builtin_not_implemented (char *name)
{
  mu_error ("%s is not yet implemented.", name);
}

static void
builtin_msg (struct mh_fvm *mach)
{
  size_t msgno;
  mh_message_number (mach->message, &msgno);
  mach->num[R_REG] = msgno;
}

static void
builtin_cur (struct mh_fvm *mach)
{
  size_t msgno;
  size_t cur;
  int rc;
  mu_mailbox_t mbox;
  
  rc = mu_message_get_mailbox (mach->message, &mbox);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_message_get_mailbox", NULL, rc);
      exit (1);
    }
  mh_message_number (mach->message, &msgno);
  mh_mailbox_get_cur (mbox, &cur); /* FIXME: Cache this */
  mach->num[R_REG] = msgno == cur;
}

static void
builtin_size (struct mh_fvm *mach)
{
  size_t size;
  if (mu_message_size (mach->message, &size) == 0)
    mach->num[R_REG] = size;
  else
    mach->num[R_REG] = 0;
}

static void
builtin_strlen (struct mh_fvm *mach)
{
  mach->num[R_REG] = mh_string_length (&mach->str[R_REG]);
}

static void
builtin_width (struct mh_fvm *mach)
{
  mach->num[R_REG] = mach->width;
}

static void
builtin_charleft (struct mh_fvm *mach)
{
  mach->num[R_REG] = output_width (mach);
}

static void
builtin_timenow (struct mh_fvm *mach)
{
  time_t t;
  
  time (&t);
  mach->num[R_REG] = t;
}

static void
builtin_me (struct mh_fvm *mach)
{
  mh_string_load (&mach->str[R_REG], mh_get_my_user_name ());
}

static void
builtin_myhost (struct mh_fvm *mach)
{
  mh_string_load (&mach->str[R_REG], mh_my_host ());
}

static void
builtin_myname (struct mh_fvm *mach)
{
  mh_string_load (&mach->str[R_REG], mh_get_my_real_name ());
}

static void
builtin_localmbox (struct mh_fvm *mach)
{
  mh_string_load (&mach->str[R_REG], mh_my_email ());
}

static void
builtin_eq (struct mh_fvm *mach)
{
  mach->num[R_REG] = mach->num[R_REG] == mach->num[R_ARG];
}

static void
builtin_ne (struct mh_fvm *mach)
{
  mach->num[R_REG] = mach->num[R_REG] != mach->num[R_ARG];
}

static void
builtin_gt (struct mh_fvm *mach)
{
  mach->num[R_REG] = mach->num[R_REG] > mach->num[R_ARG];
}

static void
builtin_match (struct mh_fvm *mach)
{
  mach->num[R_REG] = strstr (mh_string_value (&mach->str[R_REG]),
			     mh_string_value (&mach->str[R_ARG])) != NULL;
}

static void
builtin_amatch (struct mh_fvm *mach)
{
  char const *arg = mh_string_value (&mach->str[R_ARG]);
  size_t len = strlen (arg);
  mach->num[R_REG] =
    strncmp (mh_string_value (&mach->str[R_REG]), arg, len) == 0;
}

static void
builtin_plus (struct mh_fvm *mach)
{
  mach->num[R_REG] += mach->num[R_ARG];
}

static void
builtin_minus (struct mh_fvm *mach)
{
  mach->num[R_REG] -= mach->num[R_ARG];
}

static void
builtin_divide (struct mh_fvm *mach)
{
  if (mach->num[R_ARG] == 0)
    {
      /* TRANSLATORS: Do not translate the word 'format'! */
      mu_error (_("format: divide by zero"));
      mach->stop = 1;
    }
  else
    mach->num[R_REG] /= mach->num[R_ARG];
}

static void
builtin_modulo (struct mh_fvm *mach)
{
  if (mach->num[R_ARG] == 0)
    {
      mu_error (_("format: divide by zero"));
      mach->stop = 1;
    }
  else
    mach->num[R_REG] %= mach->num[R_ARG];
}

static void
builtin_getenv (struct mh_fvm *mach)
{
  char const *val = getenv (mh_string_value (&mach->str[R_ARG]));
  mh_string_load (&mach->str[R_REG], val);
}

static void
builtin_profile (struct mh_fvm *mach)
{
  char const *val = mh_global_profile_get (mh_string_value (&mach->str[R_ARG]),
					   "");
  mh_string_load (&mach->str[R_REG], val);
}

static void
builtin_nonzero (struct mh_fvm *mach)
{
  mach->num[R_REG] = mach->num[R_ARG] != 0;
}

static void
builtin_zero (struct mh_fvm *mach)
{
  mach->num[R_REG] = mach->num[R_ARG] == 0;
}

static void
builtin_null (struct mh_fvm *mach)
{
  mach->num[R_REG] = mh_string_is_null (&mach->str[R_ARG]);
}

static void
builtin_nonnull (struct mh_fvm *mach)
{
  mach->num[R_REG] = !mh_string_is_null (&mach->str[R_ARG]);
}

/*     comp       comp     string   Set str to component text*/
static void
builtin_comp (struct mh_fvm *mach)
{
  /* FIXME: Check this */
  mh_string_copy (mach, R_REG, R_ARG);
}

/*     compval    comp     integer  num set to "atoi(comp)"*/
static void
builtin_compval (struct mh_fvm *mach)
{
  /* FIXME: Check this */
  mach->num[R_REG] = strtol (mh_string_value (&mach->str[R_ARG]), NULL, 0);
}

/*     trim       expr              trim trailing white-space from str*/
static void
builtin_trim (struct mh_fvm *mach)
{
  if (!mh_string_is_null (&mach->str[R_REG]))
    mu_rtrim_class (mach->str[R_REG].ptr, MU_CTYPE_SPACE);
}

static void
_parse_date (struct mh_fvm *mach, struct tm *tm, struct mu_timezone *tz,
	     int *pflags)
{
  char const *date = mh_string_value (&mach->str[R_ARG]);
  int flags;
  
  if (!(mu_parse_date_dtl (date, NULL, NULL, tm, tz, &flags) == 0
	&& (flags & (MU_PD_MASK_DATE|MU_PD_MASK_TIME))))
    {
      
      /*mu_error ("can't parse date: [%s]", date);*/
      if (tm)
	{
	  time_t t;
	  time (&t);
	  *tm = *localtime (&t);
	}
      
      if (tz)
	mu_datetime_tz_local (tz);
      flags = 0;
    }
  if (pflags)
    *pflags = flags;
}

/*     sec        date     integer  seconds of the minute*/
static void
builtin_sec (struct mh_fvm *mach)
{
  struct tm tm;
  
  _parse_date (mach, &tm, NULL, NULL);
  mach->num[R_REG] = tm.tm_sec;
}

/*     min        date     integer  minutes of the hour*/
static void
builtin_min (struct mh_fvm *mach)
{
  struct tm tm;
  
  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_min;
}

/*     hour       date     integer  hours of the day (0-23)*/
static void
builtin_hour (struct mh_fvm *mach)
{
  struct tm tm;
  
  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_hour;
}

/*     wday       date     integer  day of the week (Sun=0)*/
static void
builtin_wday (struct mh_fvm *mach)
{
  struct tm tm;
  
  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_wday;
}

/*     day        date     string   day of the week (abbrev.)*/
static void
builtin_day (struct mh_fvm *mach)
{
  struct tm tm;
  char buf[80];
  
  _parse_date (mach, &tm, NULL, NULL);

  strftime (buf, sizeof buf, "%a", &tm);
  mh_string_load (&mach->str[R_REG], buf);
}

/*     weekday    date     string   day of the week */
static void
builtin_weekday (struct mh_fvm *mach)
{
  struct tm tm;
  char buf[80];
  
  _parse_date (mach, &tm, NULL, NULL);
  
  strftime (buf, sizeof buf, "%A", &tm);
  mh_string_load (&mach->str[R_REG], buf);
}

/*      sday       date     integer  day of the week known?
	(1=explicit,0=implicit,-1=unknown) */
static void
builtin_sday (struct mh_fvm *mach)
{
  int flags;
  _parse_date (mach, NULL, NULL, &flags);
  mach->num[R_REG] = !!(flags & MU_PD_MASK_DOW); /* FIXME: how about unknown? */
}

/*     mday       date     integer  day of the month*/
static void
builtin_mday (struct mh_fvm *mach)
{
  struct tm tm;
  
  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_mday;
}

/*      yday       date     integer  day of the year */
static void
builtin_yday (struct mh_fvm *mach)
{
  struct tm tm;
  
  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_yday;
}

/*     mon        date     integer  month of the year*/
static void
builtin_mon (struct mh_fvm *mach)
{
  struct tm tm;
   
  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_mon + 1;
}

/*     month      date     string   month of the year (abbrev.) */
static void
builtin_month (struct mh_fvm *mach)
{
  struct tm tm;
  char buf[80];
  
  _parse_date (mach, &tm, NULL, NULL);

  strftime (buf, sizeof buf, "%b", &tm);
  mh_string_load (&mach->str[R_REG], buf);
}

/*      lmonth     date     string   month of the year*/
static void
builtin_lmonth (struct mh_fvm *mach)
{
  struct tm tm;
  char buf[80];
  
  _parse_date (mach, &tm, NULL, NULL);

  strftime (buf, sizeof buf, "%B", &tm);
  mh_string_load (&mach->str[R_REG], buf);
}

/*     year       date     integer  year (may be > 100)*/
static void
builtin_year (struct mh_fvm *mach)
{
  struct tm tm;
  
  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_year + 1900;
}

/*     zone       date     integer  timezone in hours
   FIXME: mh and nmh return the value as given, e.g. 0300 is returned as 300 */
static void
builtin_zone (struct mh_fvm *mach)
{
  struct mu_timezone tz;
  
  _parse_date (mach, NULL, &tz, NULL);

  mach->num[R_REG] = tz.utc_offset / 3600;
}

/*     tzone      date     string   timezone string */
static void
builtin_tzone (struct mh_fvm *mach)
{
  struct mu_timezone tz;
  char buf[6];
  int s;
  unsigned hrs;
  
  _parse_date (mach, NULL, &tz, NULL);

#if 0
  /* FIXME: If symbolic tz representation is needed, we'd do: */
  if (tz.tz_name)
    mh_string_load (&mach->str[R_REG], tz.tz_name);
  else
    /* .... */
  /* However, MH's tzone function simply formats the timezone */
#endif
      
  if (tz.utc_offset < 0)
    {
      s = '-';
      tz.utc_offset = - tz.utc_offset;
    }
  else
    s = '+';
  hrs = tz.utc_offset / 3600;
  snprintf (buf, sizeof buf, "%c%02u%02u", s,
	    hrs, (tz.utc_offset - hrs * 3600) / 60);
  mh_string_load (&mach->str[R_REG], buf);
}

/*      szone      date     integer  timezone explicit?
	(0=implicit,-1=unknown) */
static void
builtin_szone (struct mh_fvm *mach)
{
  int flags;
  
  _parse_date (mach, NULL, NULL, &flags);
  mach->num[R_REG] = !!(flags & MU_PD_MASK_TZ);
}

static void
builtin_str_noop (struct mh_fvm *mach)
{
  mh_string_copy (mach, R_REG, R_ARG);
}

/*     date2local date              coerce date to local timezone*/
static void
builtin_date2local (struct mh_fvm *mach)
{
  /*FIXME: Noop*/
  builtin_str_noop (mach);
}

/*     date2gmt   date              coerce date to GMT*/
static void
builtin_date2gmt (struct mh_fvm *mach)
{
  /*FIXME: Noop*/
  builtin_str_noop (mach);
}

/*     dst        date     integer  daylight savings in effect?*/
static void
builtin_dst (struct mh_fvm *mach)
{
#ifdef HAVE_STRUCT_TM_TM_ISDST  
  struct tm tm;

  _parse_date (mach, &tm, NULL, NULL);

  mach->num[R_REG] = tm.tm_isdst;
#else
  mach->num[R_REG] = 0;
#endif
}

/*     clock      date     integer  seconds since the UNIX epoch*/
static void
builtin_clock (struct mh_fvm *mach)
{
  struct tm tm;
  struct mu_timezone tz;

  _parse_date (mach, &tm, &tz, NULL);

  mach->num[R_REG] = mu_datetime_to_utc (&tm, &tz);
}

/*     rclock     date     integer  seconds prior to current time*/
void
builtin_rclock (struct mh_fvm *mach)
{
  struct tm tm;
  struct mu_timezone tz;
  time_t now = time (NULL);
  
  _parse_date (mach, &tm, &tz, NULL);

  mach->num[R_REG] = now - mu_datetime_to_utc (&tm, &tz);
}

struct
{
  const char *std;
  const char *dst;
  int utc_offset;     /* offset from GMT (hours) */
} tzs[] = {
  { "GMT", "BST", 0 },
  { "EST", "EDT", -5 },
  { "CST", "CDT", -6 },
  { "MST", "MDT", -7 },
  { "PST", "PDT", -8 },
  { "EET", "EEST", -2 },
  { NULL, 0}
};

static void
date_cvt (struct mh_fvm *mach, int pretty)
{
  struct tm tm;
  struct mu_timezone tz;
  char buf[80];
  int i, len;
  const char *tzname = NULL;
  
  _parse_date (mach, &tm, &tz, NULL);

  if (pretty)
    {
      for (i = 0; tzs[i].std; i++)
	{
	  int offset = tzs[i].utc_offset;
	  int dst = 0;
	  
#ifdef HAVE_STRUCT_TM_TM_ISDST
	  if (tm.tm_isdst)
	    dst = -1;
#endif

	  if (tz.utc_offset == (offset + dst) * 3600)
	    {
	      if (dst)
		tzname = tzs[i].dst;
	      else
		tzname = tzs[i].std;
	      break;
	    }
	}
    }
  
  len = strftime (buf, sizeof buf,
		  "%a, %d %b %Y %H:%M:%S ", &tm);
  if (tzname)
    snprintf (buf + len, sizeof(buf) - len, "%s", tzname);
  else
    {
      int min, hrs, sign;
      int offset = tz.utc_offset;
      
      if (offset < 0)
	{
	  sign = '-';
	  offset = - offset;
	}
      else
	sign = '+';
      min = offset / 60;
      hrs = min / 60;
      min %= 60;
      snprintf (buf + len, sizeof(buf) - len, "%c%02d%02d", sign, hrs, min);
    }
  mh_string_load (&mach->str[R_REG], buf);
}

/*      tws        date     string   official 822 rendering */
static void
builtin_tws (struct mh_fvm *mach)
{
  date_cvt (mach, 0);
}

/*     pretty     date     string   user-friendly rendering*/
static void
builtin_pretty (struct mh_fvm *mach)
{
  date_cvt (mach, 1);
}

/*     nodate     date     integer  str not a date string */
static void
builtin_nodate (struct mh_fvm *mach)
{
  char const *date = mh_string_value (&mach->str[R_ARG]);
  
  mach->num[R_REG] =
    mu_parse_date_dtl (date, NULL, NULL, NULL, NULL, NULL) != 0;
}

/*     proper     addr     string   official 822 rendering */
static void
builtin_proper (struct mh_fvm *mach)
{
  int rc;
  char const *str;
  mu_address_t addr;
  
  rc = mu_address_create (&addr, mh_string_value (&mach->str[R_ARG]));
  if (rc)
    {
      mh_string_copy (mach, R_REG, R_ARG);
      return;
    }
  if (mu_address_sget_printable (addr, &str) == 0 && str)
    mh_string_load (&mach->str[R_REG], str);
  else
    mh_string_copy (mach, R_REG, R_ARG);
  mu_address_destroy (&addr);
}

/*     friendly   addr     string   user-friendly rendering*/
static void
builtin_friendly (struct mh_fvm *mach)
{
  mu_address_t addr;
  const char *str;
  int rc;
  
  rc = mu_address_create (&addr, mh_string_value (&mach->str[R_ARG]));
  if (rc)
    return;

  if (mu_address_sget_personal (addr, 1, &str) == 0 && str)
    mh_string_load (&mach->str[R_REG], str);
  else
    mh_string_copy (mach, R_REG, R_ARG);

  mu_address_destroy (&addr);
}

/*     addr       addr     string   mbox@host or host!mbox rendering*/
static void
builtin_addr (struct mh_fvm *mach)
{
  const char *arg = mh_string_value (&mach->str[R_ARG]);
  mu_address_t addr;
  const char *str;
  int rc;
  
  rc = mu_address_create (&addr, arg);
  if (rc == 0)
    {
      int rc = mu_address_sget_email (addr, 1, &str);
      if (rc == 0)
	mh_string_load (&mach->str[R_REG], mu_prstr (str));
      mu_address_destroy (&addr);
      if (rc == 0)
	return;
    }
  mh_string_load (&mach->str[R_REG], arg);  
}

/*     pers       addr     string   the personal name**/
static void
builtin_pers (struct mh_fvm *mach)
{
  char const *arg = mh_string_value (&mach->str[R_ARG]);
  mu_address_t addr;
  const char *str;
  int rc;
  
  rc = mu_address_create (&addr, arg);
  mh_string_clear (&mach->str[R_REG]);
  if (rc)
    return;

  if (mu_address_sget_personal (addr, 1, &str) == 0 && str)
    mh_string_load (&mach->str[R_REG], str);
  mu_address_destroy (&addr);
}

/* FIXME: mu_address_get_comments never returns any comments. */
/*     note       addr     string   commentary text*/
static void
builtin_note (struct mh_fvm *mach)
{
  mu_address_t addr;
  const char *str;
  int rc;
  
  rc = mu_address_create (&addr, mh_string_value (&mach->str[R_ARG]));
  mh_string_clear (&mach->str[R_REG]);
  if (rc)
    return;

  if (mu_address_sget_comments (addr, 1, &str) == 0 && str)
    mh_string_load (&mach->str[R_REG], str);
  mu_address_destroy (&addr);
}

/*     mbox       addr     string   the local mailbox**/
static void
builtin_mbox (struct mh_fvm *mach)
{
  mu_address_t addr;
  char *str;
  int rc;
  
  rc = mu_address_create (&addr, mh_string_value (&mach->str[R_ARG]));
  mh_string_clear (&mach->str[R_REG]);
  if (rc)
    return;

  if (mu_address_aget_email (addr, 1, &str) == 0 && str)
    {
      char *p = strchr (str, '@');
      if (p)
	*p = 0;
      mh_string_load (&mach->str[R_REG], str);
      free (str);
    }
  mu_address_destroy (&addr);
}

/*     mymbox     addr     integer  the user's addresses? (0=no,1=yes)*/
static void
builtin_mymbox (struct mh_fvm *mach)
{
  mu_address_t addr;
  const char *str;
  
  if (mu_address_create (&addr, mh_string_value (&mach->str[R_ARG])))
    return;

  if (mu_address_sget_email (addr, 1, &str) == 0 && str)
    mach->num[R_REG] = mh_is_my_name (str);
  else
    mach->num[R_REG] = 0;
  mu_address_destroy (&addr);
}

/*     host       addr     string   the host domain**/
static void
builtin_host (struct mh_fvm *mach)
{
  mu_address_t addr;
  char *str;
  int rc;
  
  rc = mu_address_create (&addr, mh_string_value (&mach->str[R_ARG]));
  mh_string_clear (&mach->str[R_REG]);
  if (rc)
    return;

  if (mu_address_aget_email (addr, 1, &str) == 0 && str)
    {
      char *p = strchr (str, '@');
      if (p)
	mh_string_load (&mach->str[R_REG], p + 1);
      free (str);
    }
  mu_address_destroy (&addr);
}

/*     nohost     addr     integer  no host was present**/
static void
builtin_nohost (struct mh_fvm *mach)
{
  struct mu_address hint;
  mu_address_t addr;
  const char *dom;
  int rc;

  hint.domain = NULL;
  rc = mu_address_create_hint (&addr, mh_string_value (&mach->str[R_ARG]),
			       &hint, MU_ADDR_HINT_DOMAIN);
  mh_string_clear (&mach->str[R_REG]);
  if (rc)
    mach->num[R_REG] = 1;
  else
    {
      mach->num[R_REG] = !(mu_address_sget_domain (addr, 1, &dom) == 0 && dom);
      mu_address_destroy (&addr);
    }
}

/*     type       addr     integer  host type* (0=local,1=network,
       -1=uucp,2=unknown)*/
static void
builtin_type (struct mh_fvm *mach)
{
  mu_address_t addr;
  int rc;
  const char *str;
  
  rc = mu_address_create (&addr, mh_string_value (&mach->str[R_ARG]));
  mh_string_clear (&mach->str[R_REG]);
  if (rc)
    return;

  if (mu_address_sget_email (addr, 1, &str) == 0 && str)
    {
      if (strchr (str, '@'))
	mach->num[R_REG] = 1;
      else if (strchr (str, '!'))
	mach->num[R_REG] = -1;
      else
	mach->num[R_REG] = 0; /* assume local */
    }
  else
    mach->num[R_REG] = 2;
  mu_address_destroy (&addr);
}

/*     path       addr     string   any leading host route**/
static void
builtin_path (struct mh_fvm *mach)
{
  mu_address_t addr;
  const char *str;
  int rc = mu_address_create (&addr, mh_string_value (&mach->str[R_ARG]));
  mh_string_clear (&mach->str[R_REG]);
  if (rc)
    return;
  if (mu_address_sget_route (addr, 1, &str) && str)
    mh_string_load (&mach->str[R_REG], str);
  mu_address_destroy (&addr);
}

/*     ingrp      addr     integer  address was inside a group**/
static void
builtin_ingrp (struct mh_fvm *mach)
{
  /*FIXME:*/
  builtin_not_implemented ("ingrp");
  mach->num[R_REG] = 0;
}

/*     gname      addr     string   name of group**/
static void
builtin_gname (struct mh_fvm *mach)
{
  /*FIXME:*/
  builtin_not_implemented ("gname");
  builtin_str_noop (mach);
}

/*     formataddr expr              append arg to str as a
       (comma separated) address list */
static void
builtin_formataddr (struct mh_fvm *mach)
{
  mu_address_t addr, dest;
  int i;
  size_t num;
  const char *buf;
  
  if (mh_string_is_null (&mach->str[R_ACC]))
    dest = NULL;
  else if (mu_address_create (&dest, mh_string_value (&mach->str[R_ACC])))
    return;

  if (!mh_string_is_null (&mach->str[R_ARG])
      && mu_address_create (&addr, mh_string_value (&mach->str[R_ARG])) == 0)
    {
      mu_address_get_count (addr, &num);
      for (i = 1; i <= num; i++)
	{
	  if (mu_address_sget_email (addr, i, &buf) == 0 && buf)
	    {
	      if ((rcpt_mask & RCPT_ME) || !mh_is_my_name (buf))
		{
		  mu_address_t subaddr;
		  mu_address_get_nth (addr, i, &subaddr);
		  if (!addrlist_lookup (mach->addrlist, subaddr))
		    {
		      mu_list_append (mach->addrlist, subaddr);
		      mu_address_union (&dest, subaddr);
		    }
		  else
		    mu_address_destroy (&subaddr);
		}
	    }
	}
    }
  
  if (mu_address_sget_printable (dest, &buf) == 0 && buf)
    mh_string_load (&mach->str[R_REG], buf);
  else
    mh_string_clear (&mach->str[R_REG]);
  mu_address_destroy (&dest);
}

/*      putaddr    literal        print str address list with
                                  arg as optional label;
                                  get line width from num
   FIXME: Currently it's the same as puthdr. Possibly it should do
   some address-checking as well.
*/
static void
builtin_putaddr (struct mh_fvm *mach)
{
  if (!mh_string_is_null (&mach->str[R_ARG]))
    print_hdr_string (mach, mh_string_value (&mach->str[R_ARG]));
  if (!mh_string_is_null (&mach->str[R_REG]))
    print_hdr_string (mach, mh_string_value (&mach->str[R_REG]));
}

/* GNU extension: Strip leading whitespace and eventual Re: (or Re\[[0-9]+\]:)
   prefix from the argument */
static void
builtin_unre (struct mh_fvm *mach)
{
  char const *arg = mh_string_value (&mach->str[R_ARG]);
  char const *p;
  int rc = mu_unre_subject (arg, &p);

  if (rc == 0 && p != arg)
    {
      char *q = mu_strdup (p); /* Create a copy, since mh_string_load can
			          destroy p */
      mh_string_load (&mach->str[R_REG], q);
      free (q);
    }
  else
    mh_string_load (&mach->str[R_REG], arg);
}  

static void
builtin_isreply (struct mh_fvm *mach)
{
  int rc;
  
  if (mh_string_is_null (&mach->str[R_ARG]))
    {
      mu_header_t hdr = NULL;
      char *value = NULL;
      mu_message_get_header (mach->message, &hdr);
      
      mu_header_aget_value (hdr, MU_HEADER_SUBJECT, &value);
      rc = mu_unre_subject (value, NULL);
      free (value);
    }
  else
    rc = mu_unre_subject (mh_string_value (&mach->str[R_ARG]), NULL);

  mach->num[R_REG] = !rc;
}

static void
builtin_decode (struct mh_fvm *mach)
{
  char *tmp;
  
  if (mh_string_is_null (&mach->str[R_ARG]))
    return;

  if (mh_decode_2047 (mh_string_value (&mach->str[R_ARG]), &tmp) == 0)
    {
      mh_string_load (&mach->str[R_REG], tmp);
      free (tmp);
    }
}

static void
builtin_reply_regex (struct mh_fvm *mach)
{
  mh_set_reply_regex (mh_string_value (&mach->str[R_ARG]));
}

int
mh_decode_rcpt_flag (const char *arg)
{
  if (strcmp (arg, "to") == 0)
    return RCPT_TO;
  else if (strcmp (arg, "cc") == 0)
    return RCPT_CC;
  else if (strcmp (arg, "me") == 0)
    return RCPT_ME;
  else if (strcmp (arg, "all") == 0)
    return RCPT_ALL;

  return RCPT_NONE;
}

static void
builtin_rcpt (struct mh_fvm *mach)
{
  int rc = mh_decode_rcpt_flag (mh_string_value (&mach->str[R_ARG]));
  if (rc == RCPT_NONE)
    {
      mu_error (_("invalid recipient mask"));
      /* try to continue anyway */
    }
  mach->num[R_REG] = !!(rc & rcpt_mask);
}

static void
builtin_printhdr (struct mh_fvm *mach)
{
  char *tmp = NULL;
  size_t s = 0;
  
  if (!mh_string_is_null (&mach->str[R_ARG]))
    {
      s = mh_string_length (&mach->str[R_ARG]);
      tmp = mu_strdup (mh_string_value (&mach->str[R_ARG]));
    }
  
  if (!mh_string_is_null (&mach->str[R_REG]))
    {
      s += mh_string_length (&mach->str[R_REG]) + 1;
      tmp = mu_realloc (tmp, s);
      strcat (tmp, mh_string_value (&mach->str[R_REG]));
    }

  if (tmp)
    {
      print_hdr_string (mach, tmp);
      free (tmp);
    }
}

static void
builtin_in_reply_to (struct mh_fvm *mach)
{
  char *value;

  mh_string_clear (&mach->str[R_REG]);
  if (mu_rfc2822_in_reply_to (mach->message, &value) == 0)
    {
      mh_string_load (&mach->str[R_REG], value);
      free (value);
    }
}

static void
builtin_references (struct mh_fvm *mach)
{
  char *value;

  mh_string_clear (&mach->str[R_REG]);
  if (mu_rfc2822_references (mach->message, &value) == 0)
    {
      mh_string_load (&mach->str[R_REG], value);
      free (value);
    }
}

static void
builtin_package (struct mh_fvm *mach)
{
  mh_string_load (&mach->str[R_REG], PACKAGE);
}

static void
builtin_package_string (struct mh_fvm *mach)
{
  mh_string_load (&mach->str[R_REG], PACKAGE_STRING);
}

static void
builtin_version (struct mh_fvm *mach)
{
  mh_string_load (&mach->str[R_REG], VERSION);
}

/* Builtin function table */

mh_builtin_t builtin_tab[] = {
  /* Name       Handling function Return type  Arg type      Flags */ 
  { "msg",      builtin_msg,      mhtype_num,  mhtype_none },
  { "cur",      builtin_cur,      mhtype_num,  mhtype_none },
  { "size",     builtin_size,     mhtype_num,  mhtype_none },
  { "strlen",   builtin_strlen,   mhtype_num,  mhtype_none },
  { "width",    builtin_width,    mhtype_num,  mhtype_none },
  { "charleft", builtin_charleft, mhtype_num,  mhtype_none },
  { "timenow",  builtin_timenow,  mhtype_num,  mhtype_none },
  { "me",       builtin_me,       mhtype_str,  mhtype_none },
  { "myhost",   builtin_myhost,   mhtype_str,  mhtype_none },
  { "myname",   builtin_myname,   mhtype_str,  mhtype_none },
  { "localmbox",builtin_localmbox,mhtype_str,  mhtype_none },
  { "eq",       builtin_eq,       mhtype_num,  mhtype_num,  MHA_LITERAL },
  { "ne",       builtin_ne,       mhtype_num,  mhtype_num,  MHA_LITERAL },
  { "gt",       builtin_gt,       mhtype_num,  mhtype_num,  MHA_LITERAL },
  { "match",    builtin_match,    mhtype_num,  mhtype_str,  MHA_LITERAL },
  { "amatch",   builtin_amatch,   mhtype_num,  mhtype_str,  MHA_LITERAL },
  { "plus",     builtin_plus,     mhtype_num,  mhtype_num,  MHA_LITERAL },
  { "minus",    builtin_minus,    mhtype_num,  mhtype_num,  MHA_LITERAL },
  { "divide",   builtin_divide,   mhtype_num,  mhtype_num,  MHA_LITERAL },
  { "modulo",   builtin_modulo,   mhtype_num,  mhtype_num,  MHA_LITERAL },
  { "num",      NULL,             mhtype_num,  mhtype_num,  MHA_LITERAL|MHA_OPTARG|MHA_OPTARG_NIL|MHA_SPECIAL },
  { "lit",      NULL,             mhtype_str,  mhtype_str,  MHA_LITERAL|MHA_OPTARG|MHA_OPTARG_NIL|MHA_SPECIAL },
  { "getenv",   builtin_getenv,   mhtype_str,  mhtype_str,  MHA_LITERAL },
  { "profile",  builtin_profile,  mhtype_str,  mhtype_str,  MHA_LITERAL },
  { "nonzero",  builtin_nonzero,  mhtype_num,  mhtype_num,  MHA_OPTARG },
  { "zero",     builtin_zero,     mhtype_num,  mhtype_num,  MHA_OPTARG },
  { "null",     builtin_null,     mhtype_num,  mhtype_str,  MHA_OPTARG },
  { "nonnull",  builtin_nonnull,  mhtype_num,  mhtype_str,  MHA_OPTARG },
  { "comp",     builtin_comp,     mhtype_str,  mhtype_str },
  { "compval",  builtin_compval,  mhtype_num,  mhtype_str },	   
  { "trim",     builtin_trim,     mhtype_none, mhtype_str, MHA_OPTARG },
  { "putstr",   NULL,             mhtype_str,  mhtype_str, MHA_SPECIAL|MHA_OPTARG|MHA_OPTARG|MHA_IGNOREFMT },
  { "putstrf",  NULL,             mhtype_str,  mhtype_str, MHA_SPECIAL|MHA_OPTARG },
  { "putnum",   NULL,             mhtype_num,  mhtype_num, MHA_SPECIAL|MHA_OPTARG|MHA_IGNOREFMT },
  { "putnumf",  NULL,             mhtype_num,  mhtype_num, MHA_SPECIAL|MHA_OPTARG },
  { "sec",      builtin_sec,      mhtype_num,  mhtype_str },
  { "min",      builtin_min,      mhtype_num,  mhtype_str },
  { "hour",     builtin_hour,     mhtype_num,  mhtype_str },
  { "wday",     builtin_wday,     mhtype_num,  mhtype_str },
  { "day",      builtin_day,      mhtype_str,  mhtype_str },
  { "weekday",  builtin_weekday,  mhtype_str,  mhtype_str },
  { "sday",     builtin_sday,     mhtype_num,  mhtype_str },
  { "mday",     builtin_mday,     mhtype_num,  mhtype_str },
  { "yday",     builtin_yday,     mhtype_num,  mhtype_str },
  { "mon",      builtin_mon,      mhtype_num,  mhtype_str },
  { "month",    builtin_month,    mhtype_str,  mhtype_str },
  { "lmonth",   builtin_lmonth,   mhtype_str,  mhtype_str },
  { "year",     builtin_year,     mhtype_num,  mhtype_str },
  { "zone",     builtin_zone,     mhtype_num,  mhtype_str },
  { "tzone",    builtin_tzone,    mhtype_str,  mhtype_str },
  { "szone",    builtin_szone,    mhtype_num,  mhtype_str },
  { "date2local", builtin_date2local, mhtype_none, mhtype_str },
  { "date2gmt", builtin_date2gmt, mhtype_none,  mhtype_str },
  { "dst",      builtin_dst,      mhtype_num,  mhtype_str },
  { "clock",    builtin_clock,    mhtype_num,  mhtype_str },
  { "rclock",   builtin_rclock,   mhtype_num,  mhtype_str },
  { "tws",      builtin_tws,      mhtype_str,  mhtype_str },
  { "pretty",   builtin_pretty,   mhtype_str,  mhtype_str },
  { "nodate",   builtin_nodate,   mhtype_num,  mhtype_str },
  { "proper",   builtin_proper,   mhtype_str,  mhtype_str },
  { "friendly", builtin_friendly, mhtype_str,  mhtype_str },
  { "addr",     builtin_addr,     mhtype_str,  mhtype_str },
  { "pers",     builtin_pers,     mhtype_str,  mhtype_str },
  { "note",     builtin_note,     mhtype_str,  mhtype_str },
  { "mbox",     builtin_mbox,     mhtype_str,  mhtype_str },
  { "mymbox",   builtin_mymbox,   mhtype_num,  mhtype_str },
  { "host",     builtin_host,     mhtype_str,  mhtype_str },
  { "nohost",   builtin_nohost,   mhtype_num,  mhtype_str },
  { "type",     builtin_type,     mhtype_num,  mhtype_str },
  { "path",     builtin_path,     mhtype_str,  mhtype_str },
  { "ingrp",    builtin_ingrp,    mhtype_num,  mhtype_str },
  { "gname",    builtin_gname,    mhtype_str,  mhtype_str},
  { "formataddr", builtin_formataddr, mhtype_none, mhtype_str, MHA_ACC },
  { "putaddr",  builtin_putaddr,  mhtype_none, mhtype_str, MHA_LITERAL },
  { "unre",     builtin_unre,     mhtype_str,  mhtype_str },
  { "rcpt",     builtin_rcpt,     mhtype_num,  mhtype_str, MHA_LITERAL },
  { "printhdr", builtin_printhdr, mhtype_none, mhtype_str, MHA_LITERAL },
  { "in_reply_to", builtin_in_reply_to, mhtype_str,  mhtype_none },
  { "references", builtin_references, mhtype_str,  mhtype_none },
  { "package",  builtin_package,  mhtype_str, mhtype_none },
  { "package_string",  builtin_package_string,  mhtype_str, mhtype_none },
  { "version",  builtin_version,  mhtype_str, mhtype_none },
  { "reply_regex", builtin_reply_regex, mhtype_none, mhtype_str },
  { "isreply", builtin_isreply, mhtype_num, mhtype_str, MHA_OPTARG },
  { "decode", builtin_decode, mhtype_str, mhtype_str },
  { "void",   NULL, mhtype_none, mhtype_str, MHA_VOID },
  { 0 }
};

mh_builtin_t *
mh_lookup_builtin (char *name, size_t len)
{
  mh_builtin_t *bp;
  
  for (bp = builtin_tab; bp->name; bp++)
    {
      if (strlen (bp->name) == len && memcmp (name, bp->name, len) == 0)
	return bp;
    }
  return NULL;
}

char *
_get_builtin_name (mh_builtin_fp ptr)
{
  mh_builtin_t *bp;

  for (bp = builtin_tab; bp->name; bp++)
    if (bp->fun == ptr)
      return bp->name;
  return NULL;
}

/* Label array is used when disassembling the code, in order to create.
   meaningful label names.  The array elements starting from index 1 keep
   the code addesses to which branch instructions point, in ascending order.
   Element 0 keeps the number of addresses stored.  Thus, the label
   array LAB with contents { 3, 2, 5, 7 } declares three labels: "L1" on
   address 2, "L2", on address 5, and "L3" on address 7.
*/

/* Find in LAB the index of the label corresponding to the given PC.  Return
   0 if no label found. */
size_t
find_label (size_t *lab, size_t pc)
{
  if (lab)
    {
      size_t i;
      for (i = 1; i <= lab[0]; i++)
	{
	  if (lab[i] == pc)
	    return i;
	}
    }
  return 0;
}

static int
comp_pc (const void *a, const void *b)
{
  size_t pca = *(size_t*)a;
  size_t pcb = *(size_t*)b;
  if (pca < pcb)
    return -1;
  else if (pca > pcb)
    return 1;
  return 0;
}

/* Extract a label array from a compiled format FMT. */
static size_t *
extract_labels (mh_format_t fmt)
{
  size_t *lab;
  size_t pc;
  long n;
  
  lab = mu_calloc (fmt->progcnt, sizeof (lab[0]));
  lab[0] = 0;
  for (pc = 1; pc < fmt->progcnt; )
    {
      mh_opcode_t opcode = MHI_OPCODE (fmt->prog[pc++]);
      if (opcode == mhop_stop)
	break;
      switch (opcode)
	{
	case mhop_branch:
	case mhop_brzn:
	case mhop_brzs:
	  n = MHI_NUM (fmt->prog[pc++]);
	  if (!find_label (lab, pc + n - 1))
	    lab[++lab[0]] = pc + n - 1;
	  break;
	  
	case mhop_setn:
	  pc += 2;
	  break;

	case mhop_sets:
	case mhop_ldcomp:
	  pc += 2 + MHI_NUM (fmt->prog[pc + 1]);
	  break;

	case mhop_movn:
	case mhop_movs:
	  pc += 2;
	  break;

	case mhop_ldbody:
	case mhop_call:
	case mhop_fmtspec:
	  pc++;
	  break;

 	case mhop_printlit:
	  pc += 1 + MHI_NUM (fmt->prog[pc]);
	  break;

	case mhop_atoi:
	case mhop_itoa:
	case mhop_printn:
	case mhop_prints:
	case mhop_pushn:
	case mhop_popn:
	case mhop_xchgn:
	  break;

	default:
	  abort ();
	}
    }
  if (lab[0] > 0)
    qsort (lab + 1, lab[0], sizeof lab[0], comp_pc);
  return lab;
}

/* Print to *PBUF (of size *PSZ) the label corresponding to the address PC.
   If there's no label having this address (in particular, if LAB==NULL),
   format the address itself to *PBUF.
   Reallocate *PBUF, updating *PSZ, if necessary.
*/
void
format_label (size_t *lab, size_t pc, char **pbuf, size_t *psz)
{
  size_t ln = find_label (lab, pc);
  if (ln)
    mu_asnprintf (pbuf, psz, "L%ld", (long) ln);
  else
    mu_asnprintf (pbuf, psz, "%ld", (long) pc);
}

/* Dump disassembled code of FMT to stdout.  If ADDR is 0, print label names
   where necessary, otherwise, prefix each line of output with its program
   counter in decimal.
*/
void
mh_format_dump_disass (mh_format_t fmt, int addr)
{
  mh_instr_t *prog = fmt->prog;
  size_t pc = 1;
  int stop = 0;
  static char *regname[] = {
    [R_REG] = "reg",
    [R_ARG] = "arg",
    [R_ACC] = "acc"
  };
  static char c_trans[] = "\\\\\"\"a\ab\bf\fn\nr\rt\tv\v";
  size_t *lab;
  size_t lc;
  char *lbuf = NULL;
  size_t lsz = 0;
  
  if (!prog)
    return;

  if (!addr)
    lab = extract_labels (fmt);
  else
    lab = NULL;
  lc = lab ? 1 : 0;

  while (!stop)
    {
      mh_opcode_t opcode;

      if (addr)
	printf ("% 4.4ld: ", (long) pc);
      else
	{
	  int w = 0;
	  if (lc <= lab[0] && lab[lc] == pc)
	    {
	      w = printf ("L%ld:", (long) lc);
	      lc++;
	    }
	  if (w > 8)
	    {
	      putchar ('\n');
	      w = 0;
	    }
	  while (w < 8)
	    {
	      putchar (' ');
	      w++;
	    }
	}
      
      switch (opcode = MHI_OPCODE (prog[pc++]))
	{
	case mhop_stop:
	  printf ("stop");
	  stop = 1;
	  break;

	case mhop_branch:
	  {
	    long n =  MHI_NUM (prog[pc++]);
	    format_label (lab, pc + n - 1, &lbuf, &lsz);
	    printf ("branch %s", lbuf);
	  }
	  break;

	case mhop_brzn:
	  {
	    long n =  MHI_NUM (prog[pc++]);
	    format_label (lab, pc + n - 1, &lbuf, &lsz);
	    printf ("brzn %s", lbuf);
	  }
	  break;

	case mhop_brzs:
	  {
	    long n =  MHI_NUM (prog[pc++]);
	    format_label (lab, pc + n - 1, &lbuf, &lsz);
	    printf ("brzs %s", lbuf);
	  }
	  break;

	case mhop_setn:
	  {
	    long reg = MHI_NUM (prog[pc++]);
	    long n = MHI_NUM (prog[pc++]);
	    printf ("setn %s, %ld", regname[reg], n);
	  }
	  break;
	  
	case mhop_sets:
	  {
	    long reg = MHI_NUM (prog[pc++]);
	    size_t skip = MHI_NUM (prog[pc++]);
	    char const *str = MHI_STR (prog[pc]);
	    char *prt;
	    
	    MU_ASSERT (mu_c_str_escape_trans (str, c_trans, &prt));
	    
	    pc += skip;
	    printf ("sets %s, \"%s\"", regname[reg], prt);
	    free (prt);
	  }
	  break;

	case mhop_movn:
	  {
	    long dst = MHI_NUM (prog[pc++]);
	    long src = MHI_NUM (prog[pc++]);
	    printf ("movn %s, %s", regname[dst], regname[src]);
	  }
	  break;
	  
	case mhop_movs:
	  {
	    long dst = MHI_NUM (prog[pc++]);
	    long src = MHI_NUM (prog[pc++]);
	    printf ("movs %s, %s", regname[dst], regname[src]);
	  }
	  break;

	case mhop_pushn:
	  printf ("pushn");
	  break;
	  
	case mhop_popn:
	  printf ("popn");
	  break;

	case mhop_xchgn:
	  printf ("xchgn");
	  break;
	  
	case mhop_ldcomp:
	  {
	    long reg = MHI_NUM (prog[pc++]);
	    size_t skip = MHI_NUM (prog[pc++]);
	    char const *comp = MHI_STR (prog[pc]);
	    pc += skip;
	    printf ("ldcomp %s, \"%s\"", regname[reg], comp);
	  }
	  break;

	case mhop_ldbody:
	  {
	    long reg = MHI_NUM (prog[pc++]);
	    printf ("ldbody %s", regname[reg]);
	  }
	  break;

	case mhop_call:
	  {
	    char *name = _get_builtin_name (MHI_BUILTIN (prog[pc++]));
	    printf ("call %s", name ? name : "UNKNOWN");
	  }
	  break;

	case mhop_atoi:
	  printf ("atoi");
	  break;
	  
	case mhop_itoa:
	  printf ("itoa");
	  break;

	case mhop_printn:
	  printf ("printn");
	  break;
	  
	case mhop_prints:
	  printf ("prints");
	  break;

	case mhop_printlit:
	  {
	    size_t skip = MHI_NUM (prog[pc++]);
	    char const *str = MHI_STR (prog[pc]);
	    char *prt;
	    pc += skip;
	    MU_ASSERT (mu_c_str_escape_trans (str, c_trans, &prt));
	    printf ("printlit \"%s\"", prt);
	    free (prt);
	  }
	  break;
	  
	case mhop_fmtspec:
	  {
	    int fmtspec = MHI_NUM (prog[pc++]);
	    printf ("fmtspec ");
	    mh_print_fmtspec (fmtspec);
	    printf(", %d", fmtspec & MH_WIDTH_MASK);
	  }
	  break;

	default:
	  abort ();
	}
      printf ("\n");
    }
  free (lbuf);
  free (lab);
}

void
mh_fvm_create (mh_fvm_t *fvmp, int flags)
{
  mh_fvm_t fvm;
  const char *charset;
  
  fvm = mu_zalloc (sizeof *fvm);

  fvm->flags = flags;
  
  fvm->output = mu_strout;
  mu_stream_ref (fvm->output);
  
  MU_ASSERT (mu_list_create (&fvm->addrlist));

  charset = mh_global_profile_get ("Charset", NULL);
  if (charset && strcmp (charset, "auto"))
    {
      /* Try to set LC_CTYPE according to the value of Charset variable.
	 If Charset is `auto', there's no need to do anything, since it
	 is already set. Otherwise, we need to construct a valid locale
	 value with Charset as its codeset part. The problem is, what
	 language and territory to use for that locale.
	 
	 Neither LANG nor any other environment variable is of any use,
	 because if it were, the user would have set "Charset: auto".
	 It would be logical to use 'C' or 'POSIX', but these do not
	 work with '.UTF-8'. So, in the absence of any viable alternative,
	 'en_US' is selected. This choice may be overridden by setting
	 the LC_BASE mh_profile variable to the desired base part.
      */
      const char *lc_base = mh_global_profile_get ("LC_BASE", "en_US");
      char *locale = mu_alloc (strlen (lc_base) + 1 + strlen (charset) + 1);
      strcpy (locale, lc_base);
      strcat (locale, ".");
      strcat (locale, charset);
      if (!setlocale (LC_CTYPE, locale))
        mu_error (_("cannot set LC_CTYPE %s"), locale);
      free (locale);
    }
  //FIXME fvm->charset = charset;

  *fvmp = fvm;
}

void
mh_fvm_destroy (mh_fvm_t *fvmp)
{
  if (fvmp)
    {
      mh_fvm_t fvm = *fvmp;

      free (fvm->prog);
      free (fvm->numstack);
      mh_string_free (&fvm->str[R_REG]);
      mh_string_free (&fvm->str[R_ARG]);
      mh_string_free (&fvm->str[R_ACC]);
      addrlist_destroy (&fvm->addrlist);

      mu_stream_unref (fvm->output);
      
      free (fvm);
      *fvmp = fvm;
    }
}

void
mh_fvm_set_output (mh_fvm_t fvm, mu_stream_t str)
{
  mu_stream_unref (fvm->output);
  fvm->output = str;
  mu_stream_ref (fvm->output);
}

void
mh_fvm_set_width (mh_fvm_t fvm, size_t width)
{
  fvm->width = width - 1;
}

void
mh_fvm_set_format (mh_fvm_t fvm, mh_format_t fmt)
{
  size_t sz = fmt->progcnt * sizeof (fvm->prog[0]);
  fvm->prog = mu_realloc (fvm->prog, sz);
  memcpy (fvm->prog, fmt->prog, sz);
}




