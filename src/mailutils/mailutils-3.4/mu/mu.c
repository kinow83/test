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

#if defined(HAVE_CONFIG_H)
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <mailutils/mailutils.h>
#include "mailutils/cli.h"
#include <assert.h>
#include <glob.h>

char *mailutilsdir;

struct mu_tool
{
  char *name;
  char cmd[1];
};

#define MUTOOL_PREFIX "mailutils-"

mu_list_t
find_tools (char *pat)
{
  mu_list_t tool_list;
  char *fpat, *pattern;
  glob_t gbuf;
  
  mu_list_create (&tool_list);
  mu_list_set_destroy_item (tool_list, mu_list_free_item);
  
  fpat = mu_alloc (sizeof MUTOOL_PREFIX + strlen (pat));
  strcat (strcpy (fpat, MUTOOL_PREFIX), pat);
  pattern = mu_make_file_name (mailutilsdir, fpat);
  free (fpat);
  
  if (glob (pattern, 0, NULL, &gbuf) == 0)
    {
      int i;
      for (i = 0; i < gbuf.gl_pathc; i++)
	{
	  char *p;
	  struct mu_tool *tp = mu_alloc (sizeof (*tp) + strlen (gbuf.gl_pathv[i]));
	  strcpy (tp->cmd, gbuf.gl_pathv[i]);
	  p = strrchr (tp->cmd, '/');
	  assert (p != NULL);
	  tp->name = p + sizeof MUTOOL_PREFIX;
	  mu_list_push (tool_list, tp);
	}
      globfree (&gbuf);
    }

  return tool_list;
}  

static int
mutool_comp (const void *a, const void *b)
{
  struct mu_tool const *pa = a;
  struct mu_tool const *pb = b;
  return strcmp (pa->name, pb->name);
}

static int
show_help (void *item, void *data)
{
  struct mu_tool *t = item;
  mu_stream_t ostr = data;
  mu_stream_t istr;
  char *argv[3];
  int rc;
  
  argv[0] = t->cmd;
  argv[1] = "--describe";
  argv[2] = NULL;
  rc = mu_prog_stream_create (&istr, t->cmd,
			      2, argv,
			      0, NULL, MU_STREAM_READ);
  if (rc == 0)
    {
      unsigned margin;
  
      margin = 2;
      mu_stream_ioctl (ostr, MU_IOCTL_WORDWRAPSTREAM,
		       MU_IOCTL_WORDWRAP_SET_MARGIN,
		       &margin);
      mu_stream_printf (ostr, "%s %s",  mu_program_name, t->name);

      margin = 29;
      mu_stream_ioctl (ostr, MU_IOCTL_WORDWRAPSTREAM,
		       MU_IOCTL_WORDWRAP_SET_MARGIN,
		       &margin);
      rc = mu_stream_copy (ostr, istr, 0, NULL);
      if (rc)
	mu_diag_funcall (MU_DIAG_ERR, "mu_stream_copy", t->cmd, rc);

      mu_stream_destroy (&istr);
    }
  return 0;
}

void
subcommand_help (mu_stream_t str)
{
  mu_list_t tool_list = find_tools ("*");
  if (mu_list_is_empty (tool_list))
    {
      mu_stream_printf (str, _("No commands found.\n"));
    }
  else
    {
      mu_list_sort (tool_list, mutool_comp);
      mu_list_foreach (tool_list, show_help, str);
      mu_list_destroy (&tool_list);
    }
}

struct mu_cli_setup cli = {
  .prog_doc = N_("GNU Mailutils multi-purpose tool."),
  .prog_args = N_("COMMAND [CMDOPTS]"),
  .inorder = 1,
  .prog_doc_hook = subcommand_help
};

struct mu_parseopt pohint = {
  .po_flags = MU_PARSEOPT_PACKAGE_NAME
            | MU_PARSEOPT_PACKAGE_URL
            | MU_PARSEOPT_BUG_ADDRESS
            | MU_PARSEOPT_EXTRA_INFO
            | MU_PARSEOPT_VERSION_HOOK,
  .po_package_name = PACKAGE_NAME,
  .po_package_url = PACKAGE_URL,
  .po_bug_address = PACKAGE_BUGREPORT,
  .po_extra_info = mu_general_help_text,
  .po_version_hook = mu_version_hook,
};
struct mu_cfg_parse_hints cfhint = { .flags = 0 };

int
main (int argc, char **argv)
{
  size_t len;
  mu_list_t tool_list;
  size_t cnt;
  struct mu_tool *tp;
  int rc;
  char *str;

#define DEVSFX "/.libs/lt-mailutils"
#define DEVSFX_LEN (sizeof (DEVSFX) - 1)
#define SUBDIR "libexec"
#define SUBDIR_LEN (sizeof (SUBDIR) - 1)
  len = strlen (argv[0]);
  if (len > DEVSFX_LEN
      && strcmp (argv[0] + len - DEVSFX_LEN, DEVSFX) == 0)
    {
      len -= DEVSFX_LEN;
      mailutilsdir = mu_alloc (len + 1 + SUBDIR_LEN + 1);
      memcpy (mailutilsdir, argv[0], len);
      mailutilsdir[len++] = '/';
      strcpy (mailutilsdir + len, SUBDIR);
    }
  else
    mailutilsdir = MAILUTILSDIR;
  
  /* Native Language Support */
  MU_APP_INIT_NLS ();

  mu_cli_ext (argc, argv, &cli, &pohint, &cfhint, NULL, NULL, &argc, &argv);

  if (argc < 1)
    {
      mu_error (_("what do you want me to do?"));
      exit (1);
    }

  if (strcmp (argv[0], "help") == 0)
    {
      cli.prog_doc_hook = NULL;
      cli.prog_doc = N_("display help on mailutils subcommands");
      cli.prog_args = N_("[COMMAND]");
      pohint.po_flags |= MU_PARSEOPT_PROG_NAME;
      mu_asprintf (&str, "%s %s", mu_program_name, argv[0]);
      pohint.po_prog_name = str;
      
      mu_cli_ext (argc, argv, &cli, &pohint, &cfhint,
		  NULL, NULL, &argc, &argv);

      if (argc == 0)
	{
	  mu_stream_t str;
	  unsigned margin;
	  
	  if (mu_parseopt_help_stream_create (&str, &pohint, mu_strout))
	    abort ();
	  subcommand_help (str);

	  margin = 0;
	  mu_stream_ioctl (str, MU_IOCTL_WORDWRAPSTREAM,
			   MU_IOCTL_WORDWRAP_SET_MARGIN,
			   &margin);

	  mu_stream_printf (str, "\n"); 
	  mu_stream_printf (str, 
			    _("Run `%s help COMMAND' to get help on a particular mailutils command."),
			    mu_program_name);
	  mu_stream_destroy (&str);
	  exit (0);
	}
      else if (argc == 1)
	{
	  argv--;
	  argv[0] = argv[1];
	  argv[1] = "--help";
	  argv[2] = NULL;
	}
      else
	{
	  mu_error (_("too many arguments"));
	  exit (1);
	}
    }
  
  tool_list = find_tools (argv[0]);
  mu_list_count (tool_list, &cnt);
  if (cnt != 1)
    {
      mu_error (_("don't know what %s is"), argv[0]);
      exit (1);
    }
  rc = mu_list_head (tool_list, (void**) &tp);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_CRIT, "mu_list_head", NULL, rc);
      exit (2);
    }

  mu_asprintf (&str, "%s %s", mu_program_name, tp->name);
  setenv ("MAILUTILS_PROGNAME", str, 1);
  execv (tp->cmd, argv);
  mu_diag_funcall (MU_DIAG_CRIT, "execv", tp->cmd, errno);
  return 2;
}
