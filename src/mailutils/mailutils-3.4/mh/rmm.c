/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2002, 2005, 2007-2012, 2014-2017 Free Software
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

/* MH rmm command */

#include <mh.h>

static char prog_doc[] = N_("Remove messages");
static char args_doc[] = N_("[MSGLIST]");

static int
rmm (size_t num, mu_message_t msg, void *data)
{
  mu_attribute_t attr;
  mu_message_get_attribute (msg, &attr);
  mu_attribute_set_deleted (attr);
  return 0;
}

int
main (int argc, char **argv)
{
  mu_mailbox_t mbox;
  mu_msgset_t msgset;
  int status;
  
  mh_getopt (&argc, &argv, NULL, MH_GETOPT_DEFAULT_FOLDER,
	     args_doc, prog_doc, NULL);

  mbox = mh_open_folder (mh_current_folder (), MU_STREAM_RDWR);

  mh_msgset_parse (&msgset, mbox, argc, argv, "cur");

  status = mu_msgset_foreach_message (msgset, rmm, NULL);

  mh_sequences_elim (msgset);
  
  mu_mailbox_expunge (mbox);
  mu_mailbox_close (mbox);
  mu_mailbox_destroy (&mbox);
  mh_global_save_state ();
  return !!status;
}

