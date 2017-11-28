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
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <mailutils/stream.h>
#include <mailutils/util.h>
#include <mailutils/diag.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/nls.h>

/* Rename file OLDPATH to NEWPATH.  Prefer rename(2), unless OLDPATH
   and NEWPATH reside on different devices.  In the latter case, fall
   back to recursive copying.

   FLAGS controls what to do if NEWPATH exists.  If it has MU_COPY_OVERWRITE
   bit set, the NEWPATH will be overwritten (if possible, atomically).
   Otherwise EEXIST will be returned.
*/
int
mu_rename_file (const char *oldpath, const char *newpath, int flags)
{
  int rc;
  struct stat st;
  
  if (access (oldpath, F_OK))
    return errno;
  
  if (stat (newpath, &st) == 0)
    {
      if (flags & MU_COPY_OVERWRITE)
	{
	  if (S_ISDIR (st.st_mode))
	    {
	      if (mu_remove_file (newpath))
		return MU_ERR_REMOVE_DEST;
	    }
	}
      else
	return EEXIST;
    }

  if (rename (oldpath, newpath) == 0)
    return 0;

  if (errno == EXDEV)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		(_("cannot rename %s to %s: %s"),
		 oldpath, newpath, mu_strerror (errno)));
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_TRACE1,
		(_("attempting copy")));
      
      rc = mu_copy_file (oldpath, newpath, flags|MU_COPY_MODE|MU_COPY_OWNER);
      if (rc == 0)
	{
	  rc = mu_remove_file (oldpath);
	  if (rc)
	    {
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			(_("copied %s to %s, but failed to remove the source: %s"),
			 oldpath, newpath, mu_strerror (rc)));
	      rc = MU_ERR_REMOVE_SOURCE;
	    }
	}
    }
  else
    rc = errno;
  return rc;
}
