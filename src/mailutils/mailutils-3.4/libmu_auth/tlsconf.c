/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003-2004, 2007-2012, 2014-2017 Free Software
   Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library.  If not, see
   <http://www.gnu.org/licenses/>. */

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <mailutils/tls.h>
#include <mailutils/nls.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/mu_auth.h>


struct safety_check_closure
{
  int defval;
  int *data;
};
  
static int
cb_safety_checks (const char *name, void *data)
{
  struct safety_check_closure *cp = data;
    
  if (mu_file_safety_compose (cp->data, name, cp->defval))
    mu_error (_("unknown keyword: %s"), name);
  return 0;
}

static int
cb_cert_safety_checks (void *data, mu_config_value_t *arg)
{
  struct safety_check_closure clos;
  clos.defval = MU_TLS_CERT_FILE_CHECKS;
  clos.data = data;
  return mu_cfg_string_value_cb (arg, cb_safety_checks, &clos);
}

static int
cb_key_safety_checks (void *data, mu_config_value_t *arg)
{
  struct safety_check_closure clos;
  clos.defval = MU_TLS_KEY_FILE_CHECKS;
  clos.data = data;
  return mu_cfg_string_value_cb (arg, cb_safety_checks, &clos);
}

static int
cb_ca_safety_checks (void *data, mu_config_value_t *arg)
{
  struct safety_check_closure clos;
  clos.defval = MU_TLS_CA_FILE_CHECKS;
  clos.data = data;
  return mu_cfg_string_value_cb (arg, cb_safety_checks, &clos);
}

static struct mu_cfg_param mu_tls_global_param[] = {
  { "key-file", mu_cfg_callback,
    &mu_tls_key_file_checks, 0,
    cb_key_safety_checks,
    N_("Configure safety checks for SSL key file.  Argument is a list or "
       "sequence of check names optionally prefixed with '+' to enable or "
       "'-' to disable the corresponding check.  Valid check names are:\n"
       "\n"
       "  none          disable all checks\n"
       "  all           enable all checks\n"
       "  gwrfil        forbid group writable files\n"
       "  awrfil        forbid world writable files\n"
       "  grdfil        forbid group readable files\n"
       "  ardfil        forbid world writable files\n"
       "  linkwrdir     forbid symbolic links in group or world writable directories\n"
       "  gwrdir        forbid files in group writable directories\n"
       "  awrdir        forbid files in world writable directories\n"),
    N_("arg: list") },  
  { "cert-file", mu_cfg_callback,
    &mu_tls_cert_file_checks, 0,
    cb_cert_safety_checks,
    N_("Configure safety checks for SSL certificate.  See above for a description of <arg>."),
    N_("arg: list") },  
  { "ca-file", mu_cfg_callback,
    &mu_tls_ca_file_checks, 0,
    cb_ca_safety_checks,
    N_("Configure safety checks for SSL certificate authority file.  See above for a description of <arg>."),
    N_("arg: list") },  
  { NULL }
}; 

static struct mu_cfg_param tls_canned_param[] = {
  { "ssl-certificate-file", mu_c_string,
    NULL, mu_offsetof(struct mu_tls_config, cert_file), NULL,
    N_("Specify SSL certificate file."),
    N_("file") },
  { "ssl-key-file", mu_c_string,
    NULL, mu_offsetof(struct mu_tls_config, key_file), NULL,
    N_("Specify SSL certificate key file."),
    N_("file") },
  { "ssl-ca-file", mu_c_string,
    NULL, mu_offsetof(struct mu_tls_config, ca_file), NULL,
    N_("Specify trusted CAs file."),
    N_("file") },
  { "ssl-priorities", mu_c_string,
    NULL, mu_offsetof(struct mu_tls_config, priorities), NULL,
    N_("Set the priorities to use on the ciphers, key exchange methods, "
       "macs and compression methods."),
    NULL },
  { NULL }
}; 

void
mu_tls_cfg_init (void)
{
  struct mu_cfg_section *section;

  if (mu_create_canned_section ("tls", &section))
    abort ();
  section->docstring = N_("Configure TLS");
  section->label = NULL;
  mu_cfg_section_add_params (section, tls_canned_param);
}

struct mu_auth_module mu_auth_tls_module = {
  .name = "tls-file-checks",
  .cfg = mu_tls_global_param,
};
