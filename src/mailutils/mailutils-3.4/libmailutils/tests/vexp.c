/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2017 Free Software Foundation, Inc.

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
#include <mailutils/mailutils.h>
#include <assert.h>

int
main (int argc, char **argv)
{
  int rc;
  char *p;

  assert (argc == 2);

  rc = mu_str_vexpand (&p, argv[1],
		       "user", "somebody",
		       "host", "localhost",
		       "name", "tests",
		       "package", PACKAGE,
		       NULL);
  switch (rc)
    {
    case 0:
      printf ("%s\n", p);
      free (p);
      break;
      
    case MU_ERR_FAILURE:
      mu_error ("%s", p);
      free (p);
      break;
      
    default:
      mu_error ("%s", mu_strerror (rc));
      return 1;
    }
  return 0;
}
