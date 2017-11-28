/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999, 2001, 2005, 2007-2012, 2014-2017 Free Software
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

#include "imap4d.h"
#include <mailutils/assoc.h>

struct namespace namespace[NS_MAX] = {
  [NS_PERSONAL] = { "personal" },
  [NS_OTHER]    = { "other" },
  [NS_SHARED]   = { "shared" }
};
  
static mu_assoc_t prefixes;

struct namespace *
namespace_lookup (char const *name)
{
  size_t i;
  
  for (i = 0; i < NS_MAX; i++)
    if (strcmp (namespace[i].name, name) == 0)
      {
	if (!namespace[i].prefixes)
	  {
	    int rc = mu_list_create (&namespace[i].prefixes);
	    if (rc)
	      {
		mu_diag_funcall (MU_DIAG_ERROR, "mu_list_create", NULL, rc);
		abort ();
	      }
	  }
	return &namespace[i];
      }
  return NULL;
}

static int
cmplen (const char *aname, const void *adata,
	const char *bname, const void *bdata,
	void *closure)
{
  return strlen (bname) - strlen (aname);
}

void
translate_delim (char *dst, char const *src, int dst_delim, int src_delim)
{
  do
    *dst++ = *src == src_delim ? dst_delim : *src;
  while (*src++);
}  

static void
trim_delim (char *str, int delim)
{
  size_t len = strlen (str);
  while (len && str[len-1] == '/')
    len--;
  str[len] = 0;
}

void
namespace_init (void)
{
  int i;
  int rc;
  struct namespace_prefix *pfx;
  
  if (mu_assoc_create (&prefixes, 0))
    imap4d_bye (ERR_NO_MEM);
  for (i = 0; i < NS_MAX; i++)
    {
      mu_iterator_t itr;
      
      if (mu_list_is_empty (namespace[i].prefixes))
	continue;
      
      if (mu_list_get_iterator (namespace[i].prefixes, &itr))
	imap4d_bye (ERR_NO_MEM);
      	
      for (mu_iterator_first (itr);
	   !mu_iterator_is_done (itr); mu_iterator_next (itr))
	{
	  rc = mu_iterator_current (itr, (void **)&pfx);
	  if (rc)
	    {
	      mu_diag_funcall (MU_DIAG_ERROR, "mu_iterator_current", NULL, rc);
	      continue;
	    }
	  pfx->ns = i;

	  trim_delim (pfx->dir, '/');
	  
	  if (!pfx->scheme)
	    mu_registrar_get_default_record (&pfx->record);
	  
	  rc = mu_assoc_install (prefixes, pfx->prefix, pfx);
	  if (rc == MU_ERR_EXISTS)
	    {
	      mu_error (_("%s: namespace prefix appears twice"), pfx->prefix);
	      exit (EX_CONFIG);
	    }
	  else if (rc)
	    {
	      mu_error (_("%s: can't install prefix: %s"),
			pfx->prefix, mu_strerror (rc));
	      exit (EX_CONFIG);
	    }
	}
    }

  pfx = mu_assoc_get (prefixes, "");
  if (pfx)
    {
      if (pfx->ns != NS_PERSONAL)
	{
	  mu_error (_("empty prefix not allowed in the namespace %s"),
		    namespace[pfx->ns].name);
	  exit (EX_CONFIG);
	}
    }
  else
    {
      struct namespace *priv;
      
      pfx = mu_zalloc (sizeof (*pfx));
      pfx->prefix = mu_strdup ("");
      pfx->dir = mu_strdup ("$home");
      pfx->delim = '/';
      mu_registrar_get_default_record (&pfx->record);
      priv = namespace_lookup ("personal");
      mu_list_prepend (priv->prefixes, pfx);
      rc = mu_assoc_install (prefixes, pfx->prefix, pfx);
      if (rc)
	{
	  mu_error (_("%s: can't install prefix: %s"),
		    pfx->prefix, mu_strerror (rc));
	  exit (EX_CONFIG);
	}
    }
  
  mu_assoc_sort_r (prefixes, cmplen, NULL);
}
      
static char *
prefix_translate_name (struct namespace_prefix const *pfx, char const *name,
		       size_t namelen)
{
  size_t pfxlen = strlen (pfx->prefix);
  int delim = 0;
  
  if (pfxlen && pfx->prefix[pfxlen-1] == pfx->delim)
    {
      pfxlen--;
      delim = pfx->delim;
    }
  
  if ((pfxlen <= namelen && memcmp (pfx->prefix, name, pfxlen) == 0)
      && (delim == 0 || (name[pfxlen] == delim || name[pfxlen] == 0)))
    {
      char *tmpl, *p;

      name += pfxlen;

      if (pfx->ns == NS_PERSONAL && mu_c_strcasecmp (name, "INBOX") == 0)
	{
	  tmpl = mu_strdup (auth_data->mailbox);
	  return tmpl;//FIXME
	}
      
      tmpl = mu_alloc (namelen - pfxlen + strlen (pfx->dir) + 2);
      p = tmpl;
      
      p = mu_stpcpy (p, pfx->dir);
      if (*name)
	{
	  if (pfx->ns == NS_OTHER)
	    {
	      if (pfx->prefix[strlen (pfx->prefix) - 1] == pfx->delim)
		++name;

	      while (*name && *name != pfx->delim)
		name++;
	    }
	  else if (*name != pfx->delim)
	    *p++ = '/';
	  translate_delim (p, name, '/', pfx->delim);
	}
      
      return tmpl;
    }
  return NULL;
}

static char *
translate_name (char const *name, struct namespace_prefix const **return_pfx)
{
  mu_iterator_t itr;
  int rc;
  char *res = NULL;
  size_t namelen = strlen (name);

  rc = mu_assoc_get_iterator (prefixes, &itr);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_assoc_get_iterator", NULL, rc);
      return NULL;
    }
  for (mu_iterator_first (itr);
       !mu_iterator_is_done (itr); mu_iterator_next (itr))
    {
      struct namespace_prefix *pfx;
      
      rc = mu_iterator_current (itr, (void **)&pfx);
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_iterator_current", NULL, rc);
	  continue;
	}

      res = prefix_translate_name (pfx, name, namelen);
      if (res)
	{
	  if (return_pfx)
	    *return_pfx = pfx;
	  break;
	}
    }
  mu_iterator_destroy (&itr);

  return res;
}

static char *
extract_username (char const *name, struct namespace_prefix const *pfx)
{
  char const *p;
  char *end;
  char *user;
  size_t len;

  if (strlen (name) < strlen (pfx->prefix))
    return NULL;
  p = name + strlen (pfx->prefix);
  end = strchr (p, pfx->delim);
  
  if (end)
    len = end - p;
  else
    len = strlen (p);

  if (len == 0)
    return mu_strdup (auth_data->name);

  user = mu_alloc (len + 1);
  memcpy (user, p, len);
  user[len] = 0;
  return user;
}

char *
namespace_translate_name (char const *name,
			  struct namespace_prefix const **return_pfx)
{
  char *res = NULL;
  struct namespace_prefix const *pfx;
  
  res = translate_name (name, &pfx);

  if (res)
    {
      mu_assoc_t assoc;
      int rc;
      char *dir;
      
      rc = mu_assoc_create (&assoc, 0);
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_assoc_create", NULL, rc);
	  free (res);
	  imap4d_bye (ERR_NO_MEM);
	}
      
      switch (pfx->ns)
	{
	case NS_PERSONAL:
	  mu_assoc_install (assoc, "user", auth_data->name);
	  mu_assoc_install (assoc, "home", real_homedir);
	  break;

	case NS_OTHER:
	  {
	    struct mu_auth_data *adata;
	    char *user = extract_username (name, pfx);
	    mu_assoc_install (assoc, "user", user);
	    adata = mu_get_auth_by_name (user);
	    if (adata)
	      {
		mu_assoc_install (assoc, "home", mu_strdup (adata->dir));
		mu_auth_data_free (adata);
	      }
	    mu_assoc_set_destroy_item (assoc, mu_list_free_item);
	  }
	  break;

	case NS_SHARED:
	  break;
	}

      rc = mu_str_expand (&dir, res, assoc);
      free (res);
      mu_assoc_destroy (&assoc);
      if (rc)
	{
	  if (rc == MU_ERR_FAILURE)
	    {
	      mu_error (_("cannot expand line `%s': %s"), res, dir);
	      free (dir);
	    }
	  else
	    {
	      mu_error (_("cannot expand line `%s': %s"), res,
			mu_strerror (rc));
	    }
	  imap4d_bye (ERR_NO_MEM);
	}
      
      res = dir;
      trim_delim (res, '/');
    }
  else if (mu_c_strcasecmp (name, "INBOX") == 0 && auth_data->change_uid)
    {
      res = mu_strdup (auth_data->mailbox);
      pfx = mu_assoc_get (prefixes, "");
    }
      
  if (res && return_pfx)
    *return_pfx = pfx;

  return res;
}

char *
namespace_get_name (char const *name, mu_record_t *rec, int *mode)
{
  struct namespace_prefix const *pfx;
  char *path = namespace_translate_name (name, &pfx);
  if (strcmp (name, pfx->prefix) == 0)
    {
      free (path);
      return NULL;
    }
  if (rec)
    *rec = pfx->record;
  if (mode)
    *mode = namespace[pfx->ns].mode;
  return path;
}

static int
prefix_printer(void *item, void *data)
{
  struct namespace_prefix *pfx = item;
  int *first = data;

  if (*first)
    *first = 0;
  else
    io_sendf (" ");
  io_sendf ("(\"%s\" \"%c\")", pfx->prefix, pfx->delim);
  return 0;
}

static void
print_namespace (int ns)
{
  if (mu_list_is_empty (namespace[ns].prefixes))
    io_sendf ("NIL");
  else
    {
      int first = 1;
      io_sendf ("(");
      mu_list_foreach (namespace[ns].prefixes, prefix_printer, &first);
      io_sendf (")");
    }
}

/*
5. NAMESPACE Command

   Arguments: none

   Response:  an untagged NAMESPACE response that contains the prefix
                 and hierarchy delimiter to the server's Personal
                 Namespace(s), Other Users' Namespace(s), and Shared
                 Namespace(s) that the server wishes to expose. The
                 response will contain a NIL for any namespace class
                 that is not available. Namespace_Response_Extensions
                 MAY be included in the response.
                 Namespace_Response_Extensions which are not on the IETF
                 standards track, MUST be prefixed with an "X-".
*/

int
imap4d_namespace (struct imap4d_session *session,
                  struct imap4d_command *command, imap4d_tokbuf_t tok)
{
  if (imap4d_tokbuf_argc (tok) != 2)
    return io_completion_response (command, RESP_BAD, "Invalid arguments");

  io_sendf ("* NAMESPACE ");

  print_namespace (NS_PERSONAL);
  io_sendf (" ");
  print_namespace (NS_OTHER);
  io_sendf (" ");
  print_namespace (NS_SHARED);
  io_sendf ("\n");

  return io_completion_response (command, RESP_OK, "Completed");
}
