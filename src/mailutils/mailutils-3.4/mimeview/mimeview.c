/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2005, 2007-2012, 2014-2017 Free Software Foundation,
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <mimeview.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "mailutils/cli.h"
#include "mailutils/argcv.h"

#include "mailcap.h"

static int dry_run;    /* Dry run mode */
static int lint;       /* Syntax check mode */
static int identify;   /* Print only the file's type */
static char *metamail; /* Name of metamail program, if requested */
static char *mimetypes_config = DEFAULT_CUPS_CONFDIR;
static char *no_ask_types;  /* List of MIME types for which no questions
			       should be asked */
static int interactive = -1; 
char const *mimeview_file;      /* Name of the file to view */
mu_stream_t mimeview_stream;    /* The corresponding stream */


static void
cli_no_ask (struct mu_parseopt *po, struct mu_option *opt, char const *arg)
{
  no_ask_types = mu_strdup (arg ? arg : "*");
  setenv ("MM_NOASK", arg, 1); /* In case we are given --metamail option */
}

static void
cli_no_interactive (struct mu_parseopt *po, struct mu_option *opt,
		    char const *arg)
{
  interactive = 0;
}

static void
cli_debug (struct mu_parseopt *po, struct mu_option *opt,
	   char const *arg)
{
  mu_debug_level_t lev;
  if (!arg)
    lev = MU_DEBUG_LEVEL_UPTO (MU_DEBUG_TRACE2);
  else
    {
      mu_debug_get_category_level (MU_DEBCAT_APP, &lev);
      for (; *arg; arg++)
	{
	  switch (*arg)
	    {
	    case 'l':
	      lev |= MU_DEBUG_LEVEL_MASK (MU_DEBUG_TRACE4);
	      break;
	      
	    case 'g':
	      lev |= MU_DEBUG_LEVEL_MASK (MU_DEBUG_TRACE3);
	      break;

	    default:
	      if (mu_isdigit (*arg))
		lev |= MU_DEBUG_LEVEL_UPTO (MU_DEBUG_TRACE0 + *arg - '0');
	      else
		mu_parseopt_error (po, _("ignoring invalid debug flag: %c"),
				   *arg);
	  }
	}
    }
  mu_debug_set_category_level (MU_DEBCAT_APP, lev);
}

static void
cli_metamail (struct mu_parseopt *po, struct mu_option *opt,
	      char const *arg)
{
  if (!arg)
    arg = "metamail";
  metamail = mu_strdup (arg);
}

static struct mu_option mimeview_options[] = {
  { "no-ask", 'a', N_("TYPE-LIST"), MU_OPTION_ARG_OPTIONAL,
    N_("do not ask for confirmation before displaying files, or, if TYPE-LIST is given, do not ask for confirmation before displaying such files whose MIME type matches one of the patterns from TYPE-LIST"),
    mu_c_string, NULL, cli_no_ask },
  { "no-interactive", 'h', NULL, MU_OPTION_DEFAULT,
   N_("disable interactive mode"),
    mu_c_string, NULL, cli_no_interactive },
  { "print",  0, NULL, MU_OPTION_ALIAS },
  { "debug", 'd', N_("FLAGS"),  MU_OPTION_ARG_OPTIONAL,
    N_("enable debugging output"),
    mu_c_string, NULL, cli_debug },
  { "mimetypes", 'f', N_("FILE"), MU_OPTION_DEFAULT,
    N_("use this mime.types file"),
    mu_c_string, &mimetypes_config },
  
  { "dry-run",   'n', NULL, MU_OPTION_DEFAULT,
    N_("do nothing, just print what would have been done"),
    mu_c_bool, &dry_run },

  { "lint",      't', NULL, MU_OPTION_DEFAULT,
    N_("test mime.types syntax and exit"),
    mu_c_bool, &lint },

  { "identify",  'i', NULL, MU_OPTION_DEFAULT,
    N_("identify MIME type of each file"),
    mu_c_bool, &identify },
  
  { "metamail",    0, N_("FILE"), MU_OPTION_ARG_OPTIONAL,
    N_("use metamail to display files"),
    mu_c_string, NULL, cli_metamail },

  MU_OPTION_END
}, *options[] = { mimeview_options, NULL };

struct mu_cfg_param mimeview_cfg_param[] = {
  { "mimetypes", mu_c_string, &mimetypes_config, 0, NULL,
    N_("Use this mime.types file."),
    N_("file") },
  { "metamail", mu_c_string, &metamail, 0, NULL,
    N_("Use this program to display files."),
    N_("prog") },
  { NULL }
};

struct mu_cli_setup cli = {
  options,
  mimeview_cfg_param,
  N_("GNU mimeview -- display files, using mailcap mechanism."),
  N_("FILE [FILE ...]"),
  NULL,
  N_("Debug flags are:\n\
  g - Mime.types parser traces\n\
  l - Mime.types lexical analyzer traces\n\
  0-9 - Set debugging level\n")
};

static char *capa[] = {
  "debug",
  NULL
};

static int
open_file (char const *name)
{
  int rc;
  struct stat st;
  
  if (stat (name, &st))
    {
      mu_error (_("cannot stat `%s': %s"), name, mu_strerror (errno));
      return -1;
    }
  if (!S_ISREG (st.st_mode) && !S_ISLNK (st.st_mode))
    {
      mu_error (_("not a regular file or symbolic link: `%s'"), name);
      return -1;
    }

  mimeview_file = name;
  rc = mu_file_stream_create (&mimeview_stream, mimeview_file, MU_STREAM_READ);
  if (rc)
    {
      mu_error (_("Cannot open `%s': %s"), name, mu_strerror (rc));
      return -1;
    }
  return 0;
}

void
close_file ()
{
  mu_stream_close (mimeview_stream);
}

void
display_file (const char *file, const char *type)
{
  int status;

  if (identify)
    {
      printf ("%s: %s\n", file, type ? type : "unknown");
      return;
    }

  if (!type)
    return;
  
  if (metamail)
    {
      char *argv[7];
      
      argv[0] = "metamail";
      argv[1] = "-b";

      argv[2] = interactive ? "-p" : "-h";
      
      argv[3] = "-c";
      argv[4] = (char*) type;
      argv[5] = (char*) mimeview_file;
      argv[6] = NULL;
      
      if (mu_debug_level_p (MU_DEBCAT_APP, MU_DEBUG_TRACE0))
	{
	  char *string;
	  mu_argcv_string (6, argv, &string);
	  mu_debug (MU_DEBCAT_APP, MU_DEBUG_TRACE0,
		    (_("executing %s...\n"), string));
	  free (string);
	}
      
      if (!dry_run)
	mu_spawnvp (metamail, argv, &status);
    }
  else
    {
      mu_header_t hdr;
      char *text;

      mu_asprintf (&text, "Content-Type: %s\n", type);
      status = mu_header_create (&hdr, text, strlen (text));
      if (status)
	mu_error (_("cannot create header: %s"), mu_strerror (status));
      else
	{
	  display_stream_mailcap (mimeview_file, mimeview_stream, hdr,
				  no_ask_types, interactive, dry_run,
				  MU_DEBCAT_APP);
	  mu_header_destroy (&hdr);
	}
    }
}

int
main (int argc, char **argv)
{
  MU_APP_INIT_NLS ();

  interactive = isatty (fileno (stdin));
  
  mu_cli (argc, argv, &cli, capa, NULL, &argc, &argv);
  if (dry_run)
    {
      mu_debug_level_t lev;
      mu_debug_get_category_level (MU_DEBCAT_APP, &lev);
      lev |= MU_DEBUG_LEVEL_UPTO (MU_DEBUG_TRACE2);
      mu_debug_set_category_level (MU_DEBCAT_APP, lev);
    }

  if (argc == 0 && !lint)
    {
      mu_error (_("no files given"));
      return 1;
    }

  if (mimetypes_parse (mimetypes_config))
    return 1;
  if (lint)
    return 0;
  
  while (argc--)
    {
      const char *type;
      char const *file = *argv++;
      if (open_file (file))
	continue;
      type = get_file_type ();
      display_file (file, type);
      close_file ();
    }
  
  return 0;
}
