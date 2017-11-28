#include <mailutils/mailutils.h>

int owner_option;
int mode_option;
int force_option;

static struct mu_option *options[] = { NULL };
  
struct mu_cli_setup cli = {
  options,
  NULL,
  "delete file",
  "FILE"
};

static char *capa[] = {
  "debug",
  NULL
};

int
main (int argc, char **argv)
{
  int rc;
  
  mu_cli (argc, argv, &cli, capa, NULL, &argc, &argv);

  if (argc != 1)
    {
      mu_error ("wrong number of arguments");
      return 1;
    }

  if (!mu_file_name_is_safe (argv[0])
      || (argv[0][0] == '/' && mu_str_count (argv[0], "/", NULL) < 2))
    {
      mu_error ("unsafe file name");
      return 1;
    }
  
  rc = mu_remove_file (argv[0]);

  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_remove_file", NULL, rc);

  return !!rc;
}

      
