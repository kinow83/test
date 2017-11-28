#include <mailutils/mailutils.h>

int owner_option;
int mode_option;
int force_option;
int deref_option;

static struct mu_option copy_options[] = {
  { "owner", 'u', NULL, MU_OPTION_DEFAULT,
    "copy ownership",
    mu_c_bool, &owner_option },
  { "mode", 'm', NULL, MU_OPTION_DEFAULT,
    "copy mode",
    mu_c_bool, &mode_option },
  { "force", 'f', NULL, MU_OPTION_DEFAULT,
    "force overwriting the destination file if it exists",
    mu_c_bool, &force_option },
  { "overwrite", 0, NULL, MU_OPTION_ALIAS },
  { "dereference", 'h', NULL, MU_OPTION_DEFAULT,
    "dereference symbolic links",
    mu_c_bool, &deref_option },
  MU_OPTION_END
}, *options[] = { copy_options, NULL };
  
struct mu_cli_setup cli = {
  options,
  NULL,
  "copy file",
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
  int flags;
  
  mu_cli (argc, argv, &cli, capa, NULL, &argc, &argv);

  if (argc != 2)
    {
      mu_error ("wrong number of arguments");
      return 1;
    }

  flags = (owner_option ? MU_COPY_OWNER : 0)
        | (mode_option ? MU_COPY_MODE : 0)
        | (force_option ? MU_COPY_OVERWRITE : 0)
        | (deref_option ? MU_COPY_DEREF : 0);
  rc = mu_copy_file (argv[0], argv[1], flags);

  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_copy_file", NULL, rc);

  return !!rc;
}

      
