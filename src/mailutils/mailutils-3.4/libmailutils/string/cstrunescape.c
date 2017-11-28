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

/* Copy characters from IN to OUT.  Replace each occurrences of backslash
   followed by a character XTAB[i] with CHR[i].

   OUT should be large enough to accomodate the translated string (same length
   as IN, in the worst case).  It is OK if IN==OUT.

   Both XTAB and CHR must not be NULL and must contain the same number of
   elements. 
 */
static void
c_str_unescape (char const *in, char *out, char const *chr, char const *xtab)
{
  size_t i, j;
  
  for (i = j = 0; in[i]; i++, j++)
    {
      if (in[i] == '\\')
	break;
      out[j] = in[i];
    }

  if (in[i])
    {
      for (; in[i]; i++, j++)
	{
	  if (in[i] == '\\')
	    {
	      char *p;
	      if (in[i+1] && (p = strchr (xtab, in[i+1])) != NULL)
		{
		  out[j] = chr[p - xtab];
		  i++;
		  continue;
		}
	    }
	  out[j] = in[i];
	}
    }
  out[j] = 0;
}

/* Modifies STR, by replacing each occurrence of \ followed by a charater
   XTAB[i] with CHR[i].

   Either XTAB or CHR can be NULL, in which case XTAB=CHR is assumed.

   If both XTAB and CHR are NULL, STR is unchanged.
   
   STR == NULL is OK.

   Returns 0 success, and EINVAL if lengths of CHR and XTAB differ.
*/
int
mu_c_str_unescape_inplace (char *str, char const *chr, char const *xtab)
{
  if (!str)
    return 0;
  if (!xtab)
    {
      if (chr)
	xtab = chr;
      else
	return 0;
    }
  else if (!chr)
    chr = xtab;
  else if (strlen (chr) != strlen (xtab))
    return EINVAL;
  c_str_unescape (str, str, chr, xtab);
  return 0;
}

/* A counterpart of mu_c_str_escape.  Creates an allocated (using malloc(3))
   copy of STR, where each occurrence of \ followed by a charater XTAB[i]
   is replaced with single character CHR[i].

   Either XTAB or CHR can be NULL, in which case XTAB=CHR is assumed.

   If both XTAB and CHR are NULL, the result is *RET_STR will contain exact
   malloc'ed copy of STR.
   
   STR == NULL is OK: in that case *RET_STR will also be NULL.

   Returns 0 success, EINVAL if lengths of CHR and XTAB differ,
   MU_ERR_OUT_PTR_NULL if RET_STR is NULL, and ENOMEM if memory allocation
   failed.
 */
int
mu_c_str_unescape (char const *str, char const *chr, char const *xtab,
		   char **ret_str)
{
  char *newstr;
  size_t i, size;
  
  if (!ret_str)
    return MU_ERR_OUT_PTR_NULL;
  
  if (!str)
    {
      *ret_str = NULL;
      return 0;
    }

  if (!xtab)
    {
      if (chr)
	xtab = chr;
      else
	{
	  char *p = strdup (str);
	  if (!p)
	    return errno;
	  *ret_str = p;
	}
    }
  else if (!chr)
    chr = xtab;
  else if (strlen (chr) != strlen (xtab))
    return EINVAL;

  size = 0;
  for (i = 0; str[i]; i++)
    {
      if (str[i] == '\\' && str[i + 1] && strchr (xtab, str[i + 1]))
	i++;
      size++;
    }

  newstr = malloc (size + 1);
  if (!newstr)
    return errno;
  *ret_str = newstr;
  
  c_str_unescape (str, newstr, chr, xtab);

  return 0;
}

/* A counterpart of mu_c_str_escape_trans.

   Creates an allocated (using malloc(3)) copy of STR, where each occurrence 
   of \ followed by TRANS[i] is replaced by TRANS[i+1].

   Returns 0 on success, or error code if an error occurred.
   
   See also mu_wordsplit_c_escape_tab in wordsplit.c
 */
int
mu_c_str_unescape_trans (char const *str, char const *trans, char **ret_str)
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

  rc = mu_c_str_unescape (str, chr, xtab, ret_str);

  free (chr);

  return rc;
}
  
