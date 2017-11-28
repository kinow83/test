/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2010-2012, 2014-2017 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mailutils/mailutils.h>

int
main (int argc, char **argv)
{
  int rc;
  mu_folder_t f;
  
  if (argc != 3)
    {
      fprintf (stderr, "usage: %s URL NAME\n", argv[0]);
      return 1;
    }

  mu_register_all_mbox_formats ();
  
  MU_ASSERT (mu_folder_create (&f, argv[1]));
  rc = mu_folder_delete (f, argv[2]);
  if (rc)
    {
      if (rc == ENOTEMPTY)
	{
	  printf ("mailbox removed, but has subfolders\n");
	  rc = 0;
	}
      else
	fprintf (stderr, "%s\n", mu_strerror (rc));
    }
  mu_folder_destroy (&f);
  
  return rc != 0;
}
