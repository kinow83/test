/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003, 2009-2012, 2014-2017 Free Software Foundation,
   Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see 
   <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mailutils/nls.h>

static int
_parse_lc_all (const char *arg, struct mu_lc_all *str, int flags)
{
  char *s;
  size_t n;
  
  str->flags = 0;
  
  n = strcspn (arg, "_.@");
  if (flags & MU_LC_LANG)
    {
      s = malloc (n + 1);
      if (!s)
	return ENOMEM;
      memcpy (s, arg, n);
      s[n] = 0;
      str->language = s;
      str->flags |= MU_LC_LANG;
    }
  else
    str->language = NULL;
  arg += n;

  if (arg[0] == '_')
    {
      arg++;
  
      n = strcspn (arg, ".@");
      if (flags & MU_LC_TERR)
	{
	  s = malloc (n + 1);
	  if (!s)
	    return ENOMEM;
	  memcpy (s, arg, n);
	  s[n] = 0;
	  str->territory = s;
	  str->flags |= MU_LC_TERR;
	}
      else
	str->territory = NULL;
      arg += n;
    }

  if (arg[0] == '.')
    {
      arg++;

      n = strcspn (arg, "@");
      if (flags & MU_LC_CSET)
	{
	  s = malloc (n + 1);
	  if (!s)
	    return ENOMEM;
	  memcpy (s, arg, n);
	  s[n] = 0;
	  str->charset = s;
	  str->flags |= MU_LC_CSET;
	}
      else
	str->charset = NULL;
      arg += n;
    }

  if (arg[0])
    {
      arg++;
      if (flags & MU_LC_MOD)
	{
	  str->modifier = strdup (arg);
	  if (!str->modifier)
	    return ENOMEM;
	  str->flags |= MU_LC_MOD;
	}
    }
  return 0;
}

void
mu_lc_all_free (struct mu_lc_all *str)
{
  if (str->flags & MU_LC_LANG)
    free (str->language);
  if (str->flags & MU_LC_TERR)
    free (str->territory);
  if (str->flags & MU_LC_CSET)
    free (str->charset);
  if (str->flags & MU_LC_MOD)
    free (str->modifier);
  str->flags = 0;
}

int
mu_parse_lc_all (const char *arg, struct mu_lc_all *str, int flags)
{
  int rc;
  
  memset (str, 0, sizeof (str[0]));
  if (!arg)
    {
      if (flags & MU_LC_LANG)
	{
	  str->language = strdup ("C");
	  if (!str->language)
	    return ENOMEM;
	}
      return 0;
    }

  /* If charset is requested (MU_LC_CSET), request also language and
     territory.  These will be used if ARG doesn't provide the charset
     information.  In any case, the surplus data will be discarded before
     returning. */
  rc = _parse_lc_all (arg, str,
		      (flags & MU_LC_CSET)
		        ? (flags | MU_LC_LANG | MU_LC_TERR)
		        : flags); 
  if (rc == 0 && (flags & MU_LC_CSET))
    {
      if (!str->charset)
	{
	  /* The caller requested charset, but we're unable to satisfy
	     the request based on the ARG only.  Try the charset table
	     lookup. */
	  const char *charset =
	    mu_charset_lookup (str->language, str->territory);
	  if (charset)
	    {
	      /* Found it.  Fill in the charset field. */
	      str->charset = strdup (charset);
	      if (!str->charset)
		{
		  rc = ENOMEM;
		  goto err;
		}
	      str->flags |= MU_LC_CSET;
	    }
	}

      /* The STR struct most probably contains data not requested by
	 the caller.  First, see what these are.  The following leaves
	 in FLAGS only those bits that weren't requested, but were filled
	 in just in case: */
      flags = ~flags & str->flags;

      /* Free the surplus data and clear the corresponding flag bits. */
      if (flags & MU_LC_LANG)
	{
	  free (str->language);
	  str->language = NULL;
	  str->flags &= ~MU_LC_LANG;
	}
      
      if (flags & MU_LC_TERR)
	{
	  free (str->territory);
	  str->territory = NULL;
	  str->flags &= ~MU_LC_TERR;
	}
    }

 err:
  if (rc)
    mu_lc_all_free (str);
  return rc;
}
