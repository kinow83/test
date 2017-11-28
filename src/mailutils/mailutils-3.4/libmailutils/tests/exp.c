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

#include <mailutils/mailutils.h>

int
main (int argc, char **argv)
{
  mu_assoc_t assc;
  char *p;
  int i;
  
  MU_ASSERT (mu_assoc_create (&assc, 0));

  for (i = 1; i < argc; i++)
    {
      p = strchr (argv[i], '=');
      if (p)
	{
	  *p++ = 0;
	  MU_ASSERT (mu_assoc_install (assc, argv[i], p));
	}
      else if (strcmp (argv[i], "--") == 0)
	{
	  i++;
	  break;
	}
      else
	break;
    }

  for (; i < argc; i++)
    {
      int rc = mu_str_expand (&p, argv[i], assc);
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
	}
    }

  return 0;
}

