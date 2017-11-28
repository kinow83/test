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

#include <config.h>
#include <mailutils/mailutils.h>
#include <mailutils/nls.h>
#include <sysexits.h>
#include "mu.h"

char cflags_docstring[] = N_("show compiler options");

int
main (int argc, char **argv)
{
  mu_action_getopt (&argc, &argv, NULL, cflags_docstring, NULL);
  if (argc)
    {
      mu_error (_("too many arguments"));
      return EX_USAGE;
    }
  mu_printf ("%s\n", COMPILE_FLAGS);
  return 0;
}

