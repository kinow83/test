/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2016-2017 Free Software Foundation, Inc.

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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <mailutils/cstr.h>
#include <mailutils/errno.h>

/* Examines STR for occurrences of characters from CHR.  Returns in RET_STR
   a malloc'ed string where each occurrence of CHR[i] is replaced by a
   backslash, followed by XTAB[i].

   If CHR is NULL, RET_STR contains a malloc'ed copy of STR (XTAB is
   ignored).

   If XTAB is NULL, XTAB = CHR is assumed.

   Callers are advised to include backslash into both CHR and XTAB.

   Returns 0 on success, error code if an error occurred.
   
   Example:

   Escape each occurrence of backslash and double quote:
   
     mu_c_str_escape (str, "\\\"", NULL, &ret_str)
*/
int
mu_c_str_escape (char const *str, char const *chr, char const *xtab,
		 char **ret_str)
{
  char *newstr;
  size_t n;
  int c;
  
  if (!ret_str)
    return MU_ERR_OUT_PTR_NULL;
  
  if (!str)
    {
      *ret_str = NULL;
      return 0;
    }

  if (!chr)
    {
      newstr = strdup (str);
      if (!newstr)
	return errno;
      *ret_str = newstr;
      return 0;
    }

  n = strlen (chr);

  if (xtab)
    {
      if (strlen (xtab) != n)
	return EINVAL;
    }
  else
    xtab = chr;
  
  n = mu_str_count (str, chr, NULL);
  
  newstr = malloc (strlen (str) + n + 1);
  if (!newstr)
    return errno;
  *ret_str = newstr;
  
  if (n == 0)
    {
      strcpy (newstr, str);
      return 0;
    }

  while ((c = *str++) != 0)
    {
      char *p = strchr (chr, c);

      if (p)
	{
	  *newstr++ = '\\';
          *newstr++ = xtab[p - chr];
	}
      else
	*newstr++ = c;
    }
  *newstr = 0;

  return 0;
}

/* Escape certain characters in STR.  Return allocated string in RET_STR.

   Escapable characters are defined by the array TRANS, which consists of an
   even number of elements.  Each pair of characters in this array contains:

     TRANS[i+1]  - character to be escaped
     TRANS[i]    - character to use in escape sequence for TRANS[i+1].

   Each TRANS[i+1] is replaced by backslash + TRANS[i].

   Returns 0 on success, or error code if an error occurred.

   E.g., to escape control characters, backslash and double-quote using
   C convention:

      mu_c_str_escape_trans (str, "\\\\\"\"a\ab\bf\fn\nr\rt\tv\v", &ret)

   See also mu_wordsplit_c_escape_tab in wordsplit.c
*/
int
mu_c_str_escape_trans (char const *str, char const *trans, char **ret_str)
{
  char *chr, *xtab;
  size_t n, i;
  int rc;
  
  if (trans)
    {
      n = strlen (trans);
      if (n % 2)
	return EINVAL;
      chr = malloc (n + 2);
      if (!chr)
	return errno;
      xtab = chr + n / 2 + 1;
      for (i = 0; i < n; i += 2)
	{
	  chr[i / 2] = trans[i + 1];
	  xtab[i / 2] = trans[i];
	}
      chr[i / 2] = xtab[i / 2] = 0;
    }
  else
    {
      chr = xtab = NULL;
    }

  rc = mu_c_str_escape (str, chr, xtab, ret_str);

  free (chr);

  return rc;
}
	
    
