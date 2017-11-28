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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <mailutils/stream.h>
#include <mailutils/util.h>
#include <mailutils/diag.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/nls.h>

static int copy_regular_file (const char *srcpath, const char *dstpath,
			      int flags, struct stat *st);
static int copy_symlink (const char *srcpath, const char *dstpath);
static int copy_dir (const char *srcpath, const char *dstpath, int flags);

/* Copy SRCPATH to DSTPATH.  SRCPATH can be any kind of file.  If it is
   a directory, its content will be copied recursively.

   FLAGS:

   MU_COPY_OVERWRITE      Overwrite destination file, if it exists.
   MU_COPY_MODE           Preserve file mode
   MU_COPY_OWNER          Preserve file ownership 
   MU_COPY_DEREF          Dereference symbolic links: operate on files they
                          refer to.
*/
int
mu_copy_file (const char *srcpath, const char *dstpath, int flags)
{
  int rc = 0;
  struct stat st;

  if (((flags & MU_COPY_DEREF) ? stat : lstat) (srcpath, &st))
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("can't stat file %s: %s"),
		 srcpath, mu_strerror (errno)));
      return errno;
    }

  if (access (dstpath, F_OK) == 0)
    {
      if (flags & MU_COPY_OVERWRITE)
	{
	  rc = mu_remove_file (dstpath);
	  if (rc)
	    {
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			(_("can't remove destination %s: %s"),
			 dstpath, mu_strerror (rc)));
	      return rc;
	    }
	}
      else
	return EEXIST;
    }
  
  switch (st.st_mode & S_IFMT)
    {
    case S_IFREG:
      return copy_regular_file (srcpath, dstpath, flags, &st);

    case S_IFLNK:
      return copy_symlink (srcpath, dstpath);

    case S_IFDIR:
      return copy_dir (srcpath, dstpath, flags);

    case S_IFBLK:
    case S_IFCHR:
      if (mknod (dstpath, st.st_mode & 0777, st.st_dev))
	{
	  rc = errno;
	  mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		    (_("%s: cannot create node: %s"),
		     dstpath,
		     mu_strerror (rc)));
	}
      break;
      
    case S_IFIFO:
      if (mkfifo (dstpath, st.st_mode & 0777))
	{
	  rc = errno;
	  mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		    (_("%s: cannot create node: %s"),
		     dstpath,
		     mu_strerror (rc)));
	}
      break;

    default:
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("%s: don't know how to copy file of that type"),
		 srcpath));
      return ENOTSUP;
    }

  return rc;
}
      
static int
copy_regular_file (const char *srcpath, const char *dstpath, int flags,
		   struct stat *st)
{
  int rc;
  mu_stream_t src, dst;
  mode_t mask, mode;

  rc = mu_file_stream_create (&src, srcpath, MU_STREAM_READ);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("cannot open source file %s: %s"),
		 srcpath, mu_strerror (rc)));
      return rc;
    }

  mask = umask (077);
  mode = ((flags & MU_COPY_MODE) ? st->st_mode : (0666 & ~mask)) & 0777;

  rc = mu_file_stream_create (&dst, dstpath, MU_STREAM_CREAT|MU_STREAM_WRITE);
  umask (mask);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("cannot open destination file %s: %s"),
		 dstpath, mu_strerror (rc)));
      mu_stream_destroy (&src);
      return rc;
    }

  rc = mu_stream_copy (dst, src, 0, NULL);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("failed to copy %s to %s: %s"),
		 srcpath, dstpath, mu_strerror (rc)));
    }
  else 
    {
      mu_transport_t trans[2];

      rc = mu_stream_ioctl (dst, MU_IOCTL_TRANSPORT, MU_IOCTL_OP_GET, trans);
      if (rc == 0)
	{	    
	  if (fchmod ((int) (intptr_t) trans[0], mode))
	    {
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			(_("%s: cannot chmod: %s"),
			 dstpath, mu_strerror (errno)));
	      rc = MU_ERR_RESTORE_META;
	    }
	  else if (flags & MU_COPY_OWNER)
	    {
	      uid_t uid;
	      gid_t gid;

	      if (getuid () == 0)
		{
		  uid = st->st_uid;
		  gid = st->st_gid;
		}
	      else if (getuid () == st->st_uid)
		{
		  uid = -1;
		  gid = st->st_gid;
		}
	      else
		{
		  uid = -1;
		  gid = -1;
		}

	      if (gid != -1)
		{
		  if (fchown ((int) (intptr_t) trans[0], uid, gid))
		    {
		      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
				(_("%s: cannot chown to %lu.%lu: %s"),
				 dstpath,
				 (unsigned long) uid,
				 (unsigned long) gid,
				 mu_strerror (errno)));
		      rc = MU_ERR_RESTORE_META;
		    }
		}
	    }
	}
      else
	{
	  mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		    (_("can't change file mode and ownership after copying %s to %s;"
		       " cannot get file handle: %s"),
		     srcpath, dstpath,
		     mu_strerror (rc)));
	}      
    }
  
  mu_stream_destroy (&src);
  mu_stream_destroy (&dst);
  
  return rc;
}

static int
copy_symlink (const char *srcpath, const char *dstpath)
{
  int rc;
  char *buf = NULL;
  size_t size = 0;
  
  rc = mu_readlink (srcpath, &buf, &size, NULL);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("%s: cannot read link: %s"),
		 srcpath, mu_strerror (rc)));
      return rc;
    }

  if (symlink (buf, dstpath))
    {
      rc = errno;
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("%s: can't link %s to %s: %s"),
		 srcpath, buf, dstpath, mu_strerror (rc)));
    }
  free (buf);
  return rc;
}
  
static int
copy_dir (const char *srcpath, const char *dstpath, int flags)
{
  DIR *dirp;
  struct dirent *dp;
  struct stat st;
  int rc;
  mode_t mode, mask;
  
  if (stat (srcpath, &st))
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("can't stat file %s: %s"),
		 srcpath, mu_strerror (errno)));
      return errno;
    }

  mask = umask (077);
  mode = ((flags & MU_COPY_MODE) ? st.st_mode : (0777 & ~mask)) & 0777;
  
  rc = mkdir (dstpath, 0700);
  umask (mask);
	  
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("can't create directory %s: %s"),
		 dstpath, mu_strerror (errno)));
      return errno;
    }
  
  dirp = opendir (srcpath);
  if (dirp == NULL)
    {
      rc = errno;
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("cannot open directory %s: %s",
		 srcpath, mu_strerror (errno)));
      return rc;
    }

  while ((dp = readdir (dirp)))
    {
      char const *ename = dp->d_name;
      char *src, *dst;
      
      if (ename[ename[0] != '.' ? 0 : ename[1] != '.' ? 1 : 2] == 0)
	continue;

      src = mu_make_file_name (srcpath, ename);
      dst = mu_make_file_name (dstpath, ename);
      rc = mu_copy_file (src, dst, flags);
      free (dst);
      free (src);

      if (rc)
	break;
    }
  closedir (dirp);

  if (chmod (dstpath, mode))
    {
      rc = errno;
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("%s: cannot chmod: %s"),
		 dstpath, mu_strerror (rc)));
    }
  else if (flags & MU_COPY_OWNER)
    {
      uid_t uid;
      gid_t gid;
	      
      if (getuid () == 0)
	{
	  uid = st.st_uid;
	  gid = st.st_gid;
	}
      else if (getuid () == st.st_uid)
	{
	  uid = -1;
	  gid = st.st_gid;
	}
      else
	{
	  uid = -1;
	  gid = -1;
	}
      
      if (gid != -1)
	{
	  if (chown (dstpath, uid, gid))
	    {
	      rc = errno;
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			(_("%s: cannot chown to %lu.%lu: %s"),
			 dstpath,
			 (unsigned long) uid,
			 (unsigned long) gid,
			 mu_strerror (rc)));
	    }
	}
    }
  return rc;
}

