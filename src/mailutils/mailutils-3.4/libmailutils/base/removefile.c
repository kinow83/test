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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <mailutils/stream.h>
#include <mailutils/util.h>
#include <mailutils/diag.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/list.h>
#include <mailutils/iterator.h>
#include <mailutils/nls.h>

static int removedir (const char *path);

int
mu_remove_file (const char *path)
{
  int rc = 0;
  struct stat st;

  if (stat (path, &st))
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("can't stat file %s: %s"),
		 path, mu_strerror (errno)));
      return errno;
    }

  if (S_ISDIR (st.st_mode))
    rc = removedir (path);
  else if (unlink (path))
    {
      rc = errno;
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("can't unlink file %s: %s"),
		 path, mu_strerror (rc)));
    }

  return rc;
}

struct nameent
{
  int isdir;
  char name[1];
};

static int
name_add (mu_list_t list, char const *name)
{
  int rc;
  size_t len = strlen (name);
  struct nameent *ent = malloc (sizeof (*ent) + len);
  if (!ent)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("%s", mu_strerror (errno)));
      return 1;
    }
  ent->isdir = -1;
  strcpy (ent->name, name);
  rc = mu_list_append (list, ent);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("mu_list_append: %s", mu_strerror (rc)));
      free (ent);
    }
    
  return rc;
}

static int
lsdir (const char *path, mu_list_t list)
{
  DIR *dirp;
  struct dirent *dp;
  int rc = 0;

  dirp = opendir (path);
  if (dirp == NULL)
    {
      rc = errno;
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("cannot open directory %s: %s",
		 path, mu_strerror (errno)));
      return rc;
    }

  while ((dp = readdir (dirp)))
    {
      char const *ename = dp->d_name;
      char *filename;
      
      if (ename[ename[0] != '.' ? 0 : ename[1] != '.' ? 1 : 2] == 0)
	continue;

      filename = mu_make_file_name (path, ename);
      if (!filename)
	{
	  rc = errno;
	  mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		    ("%s: can't create file name: %s",
		     path, mu_strerror (errno)));
	  break;
	}

      rc = name_add (list, filename);
      free (filename);
      if (rc)
	break;
    }

  closedir (dirp);

  return rc;
}

static int
namecmp (const void *a, const void *b)
{
  struct nameent const *enta = a;
  struct nameent const *entb = b;
  int d = enta->isdir - entb->isdir;
  if (d)
    return d;
  return strcmp (entb->name, enta->name);
}

static int
check_parent_access (const char *path)
{
  int rc;
  char *name, *p;

  name = strdup (path);
  if (!name)
    return errno;
  p = strrchr (name, '/');
  if (p)
    *p = 0;
  else
    strcpy (name, ".");
  rc = access (name, R_OK|W_OK|X_OK);
  free (name);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("not enough privileges to remove files from %s"),
		 name));
      return EACCES;
    }

  return 0;
}

static int
removedir (const char *path)
{
  int rc;
  struct stat st;
  mu_list_t namelist;
  mu_iterator_t itr;
  struct nameent *ent;

  rc = check_parent_access (path);
  if (rc)
    return rc;
  
  rc = mu_list_create (&namelist);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("mu_list_create: %s", mu_strerror (rc)));
      return rc;
    }
  mu_list_set_destroy_item (namelist, mu_list_free_item);
  mu_list_set_comparator (namelist, namecmp);

  rc = name_add (namelist, path);
  if (rc)
    {
      mu_list_destroy (&namelist);
      return rc;
    }
  
  rc = mu_list_get_iterator (namelist, &itr);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("mu_list_get_iterator: %s", mu_strerror (rc)));
      mu_list_destroy (&namelist);
      return rc;
    }

  for (mu_iterator_first (itr);
       !mu_iterator_is_done (itr);
       mu_iterator_next (itr)) 
    {
      mu_iterator_current (itr, (void **)&ent);
      
      if (lstat (ent->name, &st))
	{
	  rc = errno;
	  if (rc == ENOENT)
	    continue;
	  mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		    (_("can't lstat file %s: %s"),
		     ent->name, mu_strerror (rc)));
	  break;
	}

      if (S_ISDIR (st.st_mode))
	{
	  ent->isdir = 1;
	  if (access (ent->name, R_OK|W_OK|X_OK))
	    {
	      rc = EACCES;
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			(_("not enough privileges to remove files from %s"),
			 ent->name));
	    }
	  else 
	    rc = lsdir (ent->name, namelist);
	  if (rc)
	    break;
	}
      else
	ent->isdir = 0;
    }

  if (rc == 0)
    {
      mu_list_sort (namelist, namecmp);
       
      for (mu_iterator_first (itr);
	   !mu_iterator_is_done (itr);
	   mu_iterator_next (itr))
	{
	  mu_iterator_current (itr, (void **)&ent);
	  rc = (ent->isdir ? rmdir : unlink) (ent->name);
	  if (rc)
	    {
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			(_("can't remove %s: %s"),
			 ent->name, mu_strerror (rc)));
	    }
	}
    }
  mu_iterator_destroy (&itr);
  mu_list_destroy (&namelist);

  return rc;
}
  
  
