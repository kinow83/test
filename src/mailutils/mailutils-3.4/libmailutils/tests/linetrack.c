#include <mailutils/mailutils.h>
#include <mailutils/locus.h>

int
getnum (char const *arg, unsigned *ret)
{
  char *end;
  unsigned long x = strtoul (arg, &end, 10);
  if (*end)
    {
      mu_error ("bad number: %s", arg);
      return -1;
    }
  *ret = x;
  return 0;
}

static void
com_retreat (mu_linetrack_t trk, size_t argc, char **argv)
{
  unsigned x;
  if (getnum (argv[1], &x) == 0)
    {
      int rc = mu_linetrack_retreat (trk, x);
      if (rc == ERANGE)
	mu_error ("retreat count too big");
      else if (rc)
	mu_diag_funcall (MU_DIAG_ERROR, "mu_linetrack_retreat", argv[1], rc);
    }
}

static void
com_origin (mu_linetrack_t trk, size_t argc, char **argv)
{
  int rc;
  struct mu_locus_point pt;

  pt.mu_file = argv[1];
  if (getnum (argv[2], &pt.mu_line))
    return;
  if (getnum (argv[3], &pt.mu_col))
    return;
  rc = mu_linetrack_origin (trk, &pt);
  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_linetrack_origin", NULL, rc);
}

static void
com_line (mu_linetrack_t trk, size_t argc, char **argv)
{
  int rc;
  struct mu_locus_point pt = MU_LOCUS_POINT_INITIALIZER;

  if (getnum (argv[1], &pt.mu_line))
    return;
  rc = mu_linetrack_origin (trk, &pt);
  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_linetrack_origin", NULL, rc);
}

static void
com_rebase (mu_linetrack_t trk, size_t argc, char **argv)
{
  int rc;
  struct mu_locus_point pt;

  pt.mu_file = argv[1];
  if (getnum (argv[2], &pt.mu_line))
    return;
  if (getnum (argv[3], &pt.mu_col))
    return;
  rc = mu_linetrack_rebase (trk, &pt);
  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_linetrack_rebase", NULL, rc);
}

static void
com_point (mu_linetrack_t trk, size_t argc, char **argv)
{
  struct mu_locus_range lr = MU_LOCUS_RANGE_INITIALIZER;
  int rc;
  
  rc = mu_linetrack_locus (trk, &lr.beg);
  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_linetrack_locus", NULL, rc);
  else
    {
      mu_stream_lprintf (mu_strout, &lr, "%s\n", argv[0]);
      mu_locus_range_deinit (&lr);
    }
}

static void
com_bol_p (mu_linetrack_t trk, size_t argc, char **argv)
{
  mu_printf ("%d\n", mu_linetrack_at_bol (trk));
}

static void
com_stat (mu_linetrack_t trk, size_t argc, char **argv)
{
  int rc;
  struct mu_linetrack_stat st;
  
  rc = mu_linetrack_stat (trk, &st);
  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_linetrack_stat", NULL, rc);
  else
    {
      mu_printf ("n_files=%zu\n", st.n_files);
      mu_printf ("n_lines=%zu\n", st.n_lines);
      mu_printf ("n_chars=%zu\n", st.n_chars);
    }
}

struct command
{
  char *name;
  size_t argc;
  void (*fun) (mu_linetrack_t trk, size_t argc, char **argv);
};

static struct command comtab[] = {
  { "retreat",   2, com_retreat },
  { "origin",    4, com_origin },
  { "line",      2, com_line },
  { "point",     1, com_point },
  { "rebase",    4, com_rebase },
  { "bol",       1, com_bol_p },
  { "stat",      1, com_stat },
  { NULL }
};

int
main (int argc, char **argv)
{
  unsigned long max_lines;
  char *end;
  mu_linetrack_t trk;
  int rc;
  char *buf = NULL;
  size_t size, n;
  struct mu_wordsplit ws;
  int wsf = MU_WRDSF_NOVAR | MU_WRDSF_NOCMD
            | MU_WRDSF_SHOWERR | MU_WRDSF_ENOMEMABRT;
  
  mu_set_program_name (argv[0]);
  mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);

  if (argc != 3)
    {
      mu_error ("usage: %s FILE LINES", mu_program_name);
      return 1;
    }
  max_lines = strtoul (argv[2], &end, 10);
  if (*end || max_lines == 0)
    {
      mu_error ("invalid number of lines");
      return 1;
    }

  MU_ASSERT (mu_linetrack_create (&trk, argv[1], max_lines));
  while ((rc = mu_stream_getline (mu_strin, &buf, &size, &n)) == 0 && n > 0)
    {
      char *tok;
      
      n = mu_rtrim_class (buf, MU_CTYPE_SPACE);
      if (n == 0)
	continue;
      if (buf[0] == '#')
	{
	  struct command *com;

	  mu_wordsplit (buf+1, &ws, wsf);
	  wsf |= MU_WRDSF_REUSE;

	  for (com = comtab; com->name; com++)
	    if (strcmp (com->name, ws.ws_wordv[0]) == 0
		&& com->argc == ws.ws_wordc)
	      break;
	  if (com->name)
	    com->fun (trk, ws.ws_wordc, ws.ws_wordv);
	  else
	    mu_error ("unrecognized command");
	}
      else
	{
	  struct mu_locus_range lr = MU_LOCUS_RANGE_INITIALIZER;
	  
	  mu_c_str_unescape (buf, "\\\n", "\\n", &tok);
	  mu_linetrack_advance (trk, &lr, tok, strlen (tok));
	  free (tok);
	  mu_stream_lprintf (mu_strout, &lr, "%s\n", buf);
	  mu_locus_range_deinit (&lr);
	}
    }
  mu_linetrack_destroy (&trk);
  return 0;
}

  
