#include <mailutils/mailutils.h>

int force_option;

static struct mu_option rename_options[] = {
  { "force", 'f', NULL, MU_OPTION_DEFAULT,
    "force overwriting the destination file if it exists",
    mu_c_bool, &force_option },
  { "overwrite", 0, NULL, MU_OPTION_ALIAS },
  MU_OPTION_END
}, *options[] = { rename_options, NULL };
  
struct mu_cli_setup cli = {
  options,
  NULL,
  "rename file",
  "SRC DST"
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

  if (argc != 2)
    {
      mu_error ("wrong number of arguments");
      return 1;
    }

  if (!mu_file_name_is_safe (argv[0])
      || (argv[0][0] == '/' && mu_str_count (argv[0], "/", NULL) < 2))
    {
      mu_error ("%s: unsafe file name", argv[0]);
      return 1;
    }
  if (!mu_file_name_is_safe (argv[1])
      || (argv[1][0] == '/' && mu_str_count (argv[1], "/", NULL) < 2))
    {
      mu_error ("%sunsafe file name", argv[0]);
      return 1;
    }
  
  rc = mu_rename_file (argv[0], argv[1], force_option ? MU_COPY_OVERWRITE : 0);

  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_rename_file", NULL, rc);

  return !!rc;
}

      
