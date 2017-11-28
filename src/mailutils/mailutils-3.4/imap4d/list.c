/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999, 2001-2002, 2005-2012, 2014-2017 Free Software
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
#include <dirent.h>
#include <pwd.h>

struct refinfo
{
  char const *refptr;   /* Original reference */
  size_t reflen;        /* Length of the original reference */
  struct namespace_prefix const *pfx;
  size_t dirlen;        /* Length of the current directory prefix */
  char *buf;
  size_t bufsize;
};

static int
list_fun (mu_folder_t folder, struct mu_list_response *resp, void *data)
{
  char *name;
  struct refinfo *refinfo = data;
  size_t size;
  char *p;

  if ((resp->type & MU_FOLDER_ATTRIBUTE_FILE)
      && refinfo->pfx->record
      && refinfo->pfx->record != resp->format)
    return 0;
  
  name = resp->name + refinfo->dirlen;

  /* There can be only one INBOX */
  if (refinfo->reflen == 0 &&  mu_c_strcasecmp (name, "INBOX") == 0)
    return 0;

  /* Ignore mailboxes that contain delimiter as part of their name */
  if (refinfo->pfx->delim != resp->separator
      && strchr (name, refinfo->pfx->delim))
    return 0;
  
  io_sendf ("* %s", "LIST (");
  if ((resp->type & (MU_FOLDER_ATTRIBUTE_FILE|MU_FOLDER_ATTRIBUTE_DIRECTORY))
       == (MU_FOLDER_ATTRIBUTE_FILE|MU_FOLDER_ATTRIBUTE_DIRECTORY))
    /* nothing */;
  else if (resp->type & MU_FOLDER_ATTRIBUTE_FILE)
    io_sendf ("\\NoInferiors");
  else if (resp->type & MU_FOLDER_ATTRIBUTE_DIRECTORY)
    io_sendf ("\\NoSelect");
  
  io_sendf (") \"%c\" ", refinfo->pfx->delim);

  size = strlen (name) + refinfo->reflen + 2;
  if (size > refinfo->bufsize)
    {
      if (refinfo->buf == NULL)
	{
	  refinfo->bufsize = size;
	  refinfo->buf = mu_alloc (refinfo->bufsize);
	}
      else
	{
	  refinfo->buf = mu_realloc (refinfo->buf, size);
	  refinfo->bufsize = size;
	}
    }

  if (refinfo->refptr[0])
    {
      memcpy (refinfo->buf, refinfo->refptr, refinfo->reflen);
      p = refinfo->buf + refinfo->reflen;
    }
  else
    p = refinfo->buf;
  if (*name)
    translate_delim (p, name, refinfo->pfx->delim, resp->separator);

  name = refinfo->buf;
  
  if (strpbrk (name, "\"{}"))
    io_sendf ("{%lu}\n%s\n", (unsigned long) strlen (name), name);
  else if (is_atom (name))
    io_sendf ("%s\n", name);
  else
    {
      io_send_astring (name);
      io_sendf ("\n");
    }
  return 0;
}

static int
match_pfx (struct namespace_prefix const *pfx, char const *ref)
{
  char const *p = pfx->prefix, *q = ref;

  for (; *q; p++, q++)
    {
      if (*p == 0 || *p != *q)
	return 0;
    }
  if (*p == pfx->delim)
    p++;
  return *p == 0;
}

static int
list_ref (char const *ref, char const *wcard, char const *cwd,
	  struct namespace_prefix const *pfx)
{
  int rc;
  struct refinfo refinfo;
  mu_folder_t folder;
  char const *dir;
  mu_url_t url;

  if (!wcard[0])
    {
      /* An empty ("" string) mailbox name argument is a special request to
	 return the hierarchy delimiter and the root name of the name given
	 in the reference.
      */ 
      io_sendf ("* LIST (\\NoSelect) ");
      if (mu_c_strcasecmp (ref, "INBOX") == 0)
	{
	  io_sendf ("NIL \"\"");
	}
      else
	{
	  io_sendf ("\"%c\" ", pfx->delim);
	  io_send_astring (pfx->prefix);
	}
      io_sendf ("\n");
      return RESP_OK;
    }
  
  if (pfx->ns == NS_OTHER && match_pfx (pfx, ref) && strpbrk (wcard, "*%"))
    {
      /* [A] server MAY return NO to such a LIST command, requiring that a
	 user name be included with the Other Users' Namespace prefix
	 before listing any other user's mailboxes */
      return RESP_NO;
    }	  
	
  rc = mu_folder_create (&folder, cwd);
  if (rc)
    return RESP_NO;

  /* Force the right matcher */
  mu_folder_set_match (folder, mu_folder_imap_match);

  memset (&refinfo, 0, sizeof refinfo);

  refinfo.pfx = pfx;
  /* Note: original reference would always coincide with the pfx->prefix,
     if it weren't for the special handling of NS_OTHER namespace, where
     the part between the prefix and the first delimiter is considered to
     be a user name and is handled as part of the actual prefix. */
  refinfo.refptr = ref;
  refinfo.reflen = strlen (ref);

  mu_folder_get_url (folder, &url);
  mu_url_sget_path (url, &dir);
  refinfo.dirlen = strlen (dir);

  if (refinfo.refptr[refinfo.reflen-1] == pfx->delim)
    refinfo.reflen--;
  else if (strcmp (ref, pfx->prefix) == 0)
    refinfo.dirlen++;
  
  /* The special name INBOX is included in the output from LIST, if
     INBOX is supported by this server for this user and if the
     uppercase string "INBOX" matches the interpreted reference and
     mailbox name arguments with wildcards as described above.  The
     criteria for omitting INBOX is whether SELECT INBOX will return
     failure; it is not relevant whether the user's real INBOX resides
     on this or some other server. */
  
  if (!*ref &&
      mu_imap_wildmatch_ci (wcard, "INBOX", MU_HIERARCHY_DELIMITER) == 0)
    io_untagged_response (RESP_NONE, "LIST (\\NoInferiors) NIL INBOX");
  
  mu_folder_enumerate (folder, NULL, (void*) wcard, 0, 0, NULL,
		       list_fun, &refinfo);
  mu_folder_destroy (&folder);
  free (refinfo.buf);
  return RESP_OK;
}

/*
6.3.8.  LIST Command

   Arguments:  reference name
               mailbox name with possible wildcards

   Responses:  untagged responses: LIST

   Result:     OK - list completed
               NO - list failure: can't list that reference or name
               BAD - command unknown or arguments invalid
*/

/*
  1- IMAP4 insists: the reference argument present in the
  interpreted form SHOULD prefix the interpreted form.  It SHOULD
  also be in the same form as the reference name argument.  This
  rule permits the client to determine if the returned mailbox name
  is in the context of the reference argument, or if something about
  the mailbox argument overrode the reference argument.
  
  ex:
  Reference         Mailbox         -->  Interpretation
  ~smith/Mail        foo.*          -->  ~smith/Mail/foo.*
  archive            %              --> archive/%
  #news              comp.mail.*     --> #news.comp.mail.*
  ~smith/Mail        /usr/doc/foo   --> /usr/doc/foo
  archive            ~fred/Mail     --> ~fred/Mail/ *

  2- The character "*" is a wildcard, and matches zero or more characters
  at this position.  The character "%" is similar to "*",
  but it does not match the hierarchy delimiter.  */

int
imap4d_list (struct imap4d_session *session,
             struct imap4d_command *command, imap4d_tokbuf_t tok)
{
  char *ref;
  char *wcard;
  int status = RESP_OK;
  static char *resp_text[] = {
    [RESP_OK]  = "Completed",
    [RESP_NO]  = "The requested item could not be found",
    [RESP_BAD] = "System error"
  };
  char *cwd = NULL;
  struct namespace_prefix const *pfx = NULL;
      
  if (imap4d_tokbuf_argc (tok) != 4)
    return io_completion_response (command, RESP_BAD, "Invalid arguments");
  
  ref = imap4d_tokbuf_getarg (tok, IMAP4_ARG_1);
  wcard = imap4d_tokbuf_getarg (tok, IMAP4_ARG_2);

  if (ref[0] == 0)
    {
      cwd = namespace_translate_name (wcard, &pfx);
      if (cwd)
	{
	  size_t p_len = strlen (pfx->prefix);
	  size_t w_len = strlen (wcard);
	  
	  if (p_len <= w_len)
	    {
	      memmove (wcard, wcard + p_len, w_len - p_len + 1);
	      ref = mu_strdup (pfx->prefix);
	    }
	  else
	    ref = mu_strdup (ref);
	  free (cwd);
	}
      else
	ref = mu_strdup (ref);
    }
  else
    ref = mu_strdup (ref);
  
  if (!pfx)
    {
      cwd = namespace_translate_name (ref, &pfx);
      if (cwd)
	free (cwd);
    }
  
  if (pfx)
    {
      /* Find the longest directory prefix */
      size_t i = strcspn (wcard, "%*");
      if (wcard[i])
	{
	  while (i > 0 && wcard[i - 1] != pfx->delim)
	    i--;
	  /* Append it to the reference */
	  if (i)
	    {
	      size_t reflen = strlen (ref);
	      size_t len = i + reflen;
	      
	      ref = mu_realloc (ref, len);
	      memcpy (ref + reflen, wcard, i - 1); /* omit the trailing / */
	      ref[len-1] = 0;
	      
	      wcard += i;
	    }
	}
    }
	  
  cwd = namespace_translate_name (ref, &pfx);
  if (cwd)
    status = list_ref (ref, wcard, cwd, pfx);
  else
    status = RESP_NO;

  free (cwd);
  free (ref);
  
  return io_completion_response (command, status, "%s", resp_text[status]);
}

