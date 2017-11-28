/* Content-Type (RFC 2045) parser for GNU Mailutils
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <mailutils/types.h>
#include <mailutils/mime.h>
#include <mailutils/assoc.h>
#include <mailutils/util.h>
#include <mailutils/errno.h>

  
static int
content_type_parse (const char *input, const char *charset,
		    mu_content_type_t ct)
{
  int rc;
  char *value, *p;
  
  rc = mu_mime_header_parse (input, charset, &value, &ct->param);
  if (rc)
    return rc;
  p = strchr (value, '/');
  if (p)
    {
      size_t len = p - value;
      ct->type = malloc (len + 1);
      if (!ct->type)
	{
	  rc = errno;
	  free (value);
	  return rc;
	}
      memcpy (ct->type, value, len);
      ct->type[len] = 0;

      ct->subtype = strdup (p + 1);
      if (!ct->subtype)
	{
	  rc = errno;
	  free (value);
	  return rc;
	}
    }
  else
    {
      ct->type = value;
      ct->subtype = NULL;
    }
  return 0;
}

int
mu_content_type_parse (const char *input, const char *charset,
		       mu_content_type_t *retct)
{
  int rc;
  mu_content_type_t ct;

  if (!input)
    return EINVAL;
  if (!retct)
    return MU_ERR_OUT_PTR_NULL;
  
  ct = calloc (1, sizeof (*ct));
  if (!ct)
    return errno;
  rc = content_type_parse (input, charset, ct);
  if (rc)
    mu_content_type_destroy (&ct);
  else
    *retct = ct;

  return rc;
}

void
mu_content_type_destroy (mu_content_type_t *pptr)
{
  if (pptr && *pptr)
    {
      mu_content_type_t ct = *pptr;
      free (ct->type);
      free (ct->subtype);
      free (ct->trailer);
      mu_assoc_destroy (&ct->param);
      free (ct);
      *pptr = NULL;
    }
}
