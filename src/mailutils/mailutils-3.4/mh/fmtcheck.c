/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2003, 2005-2012, 2014-2017 Free Software
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

/* fmtcheck */

#include <mh.h>

static char prog_doc[] = N_("Check MH format string");
static char args_doc[] = N_("[FILE]");

static char *format_str;
static struct mu_locus_point locus = MU_LOCUS_POINT_INITIALIZER;
static mh_format_t format;
static int dump_option;
static int disass_option;
static int debug_option;
static char *input_file;
static size_t width;
static size_t msgno;
static int pc_option;

void
opt_formfile (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  free (format_str);
  if (mh_read_formfile (arg, &format_str))
    exit (1);
  mu_locus_point_set_file (&locus, arg);
  locus.mu_line = 1;
  locus.mu_col = 0;
}

void
opt_format (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  free (format_str);
  format_str = mu_strdup (arg);
}

static struct mu_option options[] = {
  { "form",    0, N_("FILE"),   MU_OPTION_DEFAULT,
    N_("read format from given file"),
    mu_c_string, NULL, opt_formfile },
  
  { "format",  0, N_("FORMAT"), MU_OPTION_DEFAULT,
    N_("use this format string"),
    mu_c_string, NULL, opt_format },
  { "dump",    0, NULL,     MU_OPTION_DEFAULT,
    N_("dump the listing of compiled format code"),
    mu_c_bool,   &dump_option },
  { "disassemble",    0, NULL,     MU_OPTION_DEFAULT,
    N_("dump disassembled format code"),
    mu_c_bool,   &disass_option },
  { "pc",      0, NULL,     MU_OPTION_DEFAULT,
    N_("print program counter along with disassembled code (implies --disassemble)"),
    mu_c_bool,   &pc_option },
  { "debug",   0, NULL,     MU_OPTION_DEFAULT,
    N_("enable parser debugging output"),
    mu_c_bool,   &debug_option },
  { "width",   0, N_("NUMBER"), MU_OPTION_DEFAULT,
    N_("set output width"),
    mu_c_size, &width },
  { "msgno",   0, N_("NUMBER"), MU_OPTION_DEFAULT,
    N_("set message number"),
    mu_c_size, &msgno },

  MU_OPTION_END
};

static int
msg_uid (mu_message_t msg MU_ARG_UNUSED, size_t *ret)
{
  if (!ret)
    return MU_ERR_OUT_PTR_NULL;
  *ret = msgno;
  return 0;
}

static void
run (void)
{
  mu_message_t msg = mh_file_to_message (NULL, input_file);
  mh_fvm_t fvm;

  MU_ASSERT (mu_message_set_uid (msg, msg_uid, mu_message_get_owner (msg)));
  
  mh_fvm_create (&fvm, MH_FMT_FORCENL);
  mh_fvm_set_width (fvm, width ? width : mh_width ());  
  mh_fvm_set_format (fvm, format);
  mh_fvm_run (fvm, msg);
  mh_fvm_destroy (&fvm);
}

int
main (int argc, char **argv)
{
  mh_getopt (&argc, &argv, options, 0, args_doc, prog_doc, NULL);
  if (pc_option)
    disass_option = 1;
  switch (argc)
    {
    case 0:
      if (!disass_option)
	dump_option = 1;
      break;
      
    case 1:
      input_file = argv[0];
      break;

    default:
      mu_error (_("too many arguments"));
      return 1;
    }
  
  if (!format_str)
    {
      mu_error (_("Format string not specified"));
      return 1;
    }
  if (mh_format_string_parse (&format, format_str, &locus,
			      MH_FMT_PARSE_TREE
			      | (debug_option ? MH_FMT_PARSE_DEBUG : 0)))
    return 1;
  
  if (dump_option)
    mh_format_dump_code (format);
  if (disass_option)
    mh_format_dump_disass (format, pc_option);

  if (input_file)
    run ();
  
  return 0;
}
