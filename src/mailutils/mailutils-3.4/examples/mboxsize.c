/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2005, 2007, 2010-2012, 2014-2017 Free Software
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

#include <mailutils/mailutils.h>

struct mu_cli_setup cli = {
  NULL,
  NULL,
  "compute mailbox size",
  "MBOX"
};

static char *capa[] = {
  "debug",
  NULL
};

int
main (int argc, char **argv)
{
  int rc;
  mu_mailbox_t mbox;
  mu_off_t size;
  char *name;
  
  mu_register_all_mbox_formats ();

  mu_cli (argc, argv, &cli, capa, NULL, &argc, &argv);

  if (argc != 1)
    {
      mu_error ("wrong number of arguments");
      return 1;
    }
  name = argv[0];
  
  rc = mu_mailbox_create_default (&mbox, name);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mailbox_create_default",
		       name, rc);
      return 1;
    }

  rc = mu_mailbox_open (mbox, MU_STREAM_READ);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mailbox_open", name, rc);
      return 1;
    }

  rc = mu_mailbox_get_size (mbox, &size);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mailbox_get_size", name, rc);
      return 1;
    }

  mu_printf ("%" MU_PRI_OFF_T "\n", size);

  return 0;
}

  

      
