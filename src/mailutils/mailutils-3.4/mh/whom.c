/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003, 2007-2012, 2014-2017 Free Software Foundation,
   Inc.

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

#include <mh.h>

static char prog_doc[] = N_("Report to whom a message would go");
static char args_doc[] = "[FILE]";

static int check_recipients;
static char *message;
static const char *draft_folder; /* Use this draft folder */

static void
add_alias (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  mh_alias_read (arg, 1);
}

static void
set_draftmessage (struct mu_parseopt *po, struct mu_option *opt,
		  char const *arg)
{
  if (!draft_folder)
    draft_folder = mh_global_profile_get ("Draft-Folder",
					  mu_folder_directory ());
}

static struct mu_option options[] = {
  { "alias",        0,    N_("FILE"), MU_OPTION_DEFAULT,
    N_("specify additional alias file"),
    mu_c_string, NULL, add_alias },
  { "draft",        0,    NULL, MU_OPTION_DEFAULT,
    N_("use prepared draft"),
    mu_c_string, &message, NULL, "draft" },
  { "draftfolder",   0,      N_("FOLDER"), MU_OPTION_DEFAULT,
    N_("specify the folder for message drafts"),
    mu_c_string, &draft_folder },
  { "nodraftfolder", 0, NULL, MU_OPTION_DEFAULT,
    N_("undo the effect of the last -draftfolder option"),
    mu_c_string, &draft_folder, mh_opt_clear_string },
  { "draftmessage",  0,  NULL, MU_OPTION_DEFAULT,
    N_("treat the arguments as a list of messages from the draftfolder"),
    mu_c_string, NULL, set_draftmessage },
  { "check",         0,  NULL, MU_OPTION_DEFAULT,
    N_("check if addresses are deliverable"),
    mu_c_bool, &check_recipients },
  MU_OPTION_END
};

static struct mh_optinit optinit[] = {
  { "draftfolder", "Draft-Folder" },
  { NULL }
};

int
main (int argc, char **argv)
{
  int rc;
  
  mh_getopt_ext (&argc, &argv, options, 0, optinit, args_doc, prog_doc, NULL);

  if (draft_folder)
    {
      mu_mailbox_t mbox = mh_open_folder (draft_folder, MU_STREAM_READ);
      mu_msgset_t msgset;
      size_t msgno;
      mu_message_t msg;
      
      mh_msgset_parse (&msgset, mbox, argc, argv, "cur");
      if (!mh_msgset_single_message (msgset))
	{
	  mu_error (_("only one message at a time!"));
	  return 1;
	}
      msgno = mh_msgset_first (msgset, RET_MSGNO);
      rc = mu_mailbox_get_message (mbox, msgno, &msg);
      if (rc)
	{
	  mu_error (_("can't read message: %s"), mu_strerror (rc));
	  exit (1);
	}
      rc = mh_whom_message (msg, check_recipients);
    }
  else
    {
      if (argc > 0)
	{
	  if (message || argc > 1)
	    {
	      mu_error (_("only one file at a time!"));
	      exit (1);
	    }
	  message = argv[0];
	}
      else
	message = "draft";
      rc = mh_whom_file (mh_expand_name (draft_folder, message, NAME_ANY), 
			 check_recipients);
    }

  return rc ? 1 : 0;
}
