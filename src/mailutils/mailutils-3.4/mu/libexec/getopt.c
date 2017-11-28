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

static void
describe (struct mu_parseopt *po, struct mu_option *opt, char const *unused)
{
  int len = strcspn (po->po_prog_doc, "\n");
  mu_printf ("%.*s\n", len, po->po_prog_doc);
  exit (0);
}

struct mu_option common_options[] = {
  { "describe", 0, NULL, MU_OPTION_HIDDEN,
    "describe the program",
    mu_c_string, NULL, describe },
  MU_OPTION_END  
};

void
mu_action_getopt (int *pargc, char ***pargv, struct mu_option *opt,
		  char const *docstring, char const *argdoc)
{
  static struct mu_parseopt pohint = {
    .po_flags = MU_PARSEOPT_PACKAGE_NAME
              | MU_PARSEOPT_PACKAGE_URL
              | MU_PARSEOPT_BUG_ADDRESS
              | MU_PARSEOPT_VERSION_HOOK,
    .po_package_name = PACKAGE_NAME,
    .po_package_url = PACKAGE_URL,
    .po_bug_address = PACKAGE_BUGREPORT,
    .po_version_hook = mu_version_hook
  };
  static char *defcapa[] = { "debug", NULL };
  struct mu_cfg_parse_hints cfhint = { .flags = 0 };
  struct mu_option *options[3] = { common_options, opt, NULL };
  struct mu_cli_setup cli = {
    .prog_doc = (char*) docstring,
    .prog_args = (char*) argdoc,
    .optv = options
  };
  char *p;

  p = getenv ("MAILUTILS_PROGNAME");
  if (p)
    {
      pohint.po_flags |= MU_PARSEOPT_PROG_NAME;
      pohint.po_prog_name = p;
    }
  
  MU_APP_INIT_NLS ();
  mu_cli_ext (*pargc, *pargv, &cli, &pohint, &cfhint, defcapa, NULL,
	      pargc, pargv);
}
  

