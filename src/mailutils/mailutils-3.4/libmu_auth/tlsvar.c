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

#include <mailutils/tls.h>
#include <mailutils/errno.h>

int mu_tls_enable = 0;
int mu_tls_cert_file_checks = MU_TLS_CERT_FILE_CHECKS;
int mu_tls_key_file_checks  = MU_TLS_KEY_FILE_CHECKS;
int mu_tls_ca_file_checks   = MU_TLS_CA_FILE_CHECKS;

static int
check_err(int rc)
{
  switch (rc)
    {
    case MU_ERR_PERM_OWNER_MISMATCH:
    case MU_ERR_PERM_GROUP_WRITABLE:
    case MU_ERR_PERM_WORLD_WRITABLE:
    case MU_ERR_PERM_GROUP_READABLE:
    case MU_ERR_PERM_WORLD_READABLE:
    case MU_ERR_PERM_LINKED_WRDIR:
    case MU_ERR_PERM_DIR_IWGRP:
    case MU_ERR_PERM_DIR_IWOTH:
      return MU_TLS_CONFIG_UNSAFE;
    default:
      return MU_TLS_CONFIG_FAIL;
    }
}

int
mu_tls_config_check (struct mu_tls_config const *conf, int verbose)
{
  int rc;
  int res = MU_TLS_CONFIG_NULL;
  
  if (conf->cert_file)
    {
      rc = mu_file_safety_check (conf->cert_file, mu_tls_cert_file_checks,
				 -1, NULL);
      if (rc)
	{
	  if (verbose)
	    mu_error ("%s: %s", conf->cert_file, mu_strerror (rc));
	  return check_err (rc);
	}
      res = MU_TLS_CONFIG_OK;
    }

  if (conf->key_file)
    {
      rc = mu_file_safety_check (conf->key_file, mu_tls_key_file_checks,
				 -1, NULL);
      if (rc)
	{
	  if (verbose)
	    mu_error ("%s: %s", conf->key_file, mu_strerror (rc));
	  return check_err (rc);
	}
      res = MU_TLS_CONFIG_OK;
    }

  if (conf->ca_file)
    {
      rc = mu_file_safety_check (conf->ca_file, mu_tls_ca_file_checks,
				 -1, NULL);
      if (rc)
	{
	  if (verbose)
	    mu_error ("%s: %s", conf->ca_file, mu_strerror (rc));
	  return check_err (rc);
	}
      res = MU_TLS_CONFIG_OK;
    }

  if (conf->priorities)
    res = MU_TLS_CONFIG_OK;
  
  return res;

}

