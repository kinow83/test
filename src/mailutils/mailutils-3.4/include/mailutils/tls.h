/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2003, 2005, 2007-2008, 2010-2012, 2014-2017 Free
   Software Foundation, Inc.

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

#ifndef _MAILUTILS_TLS_H
#define _MAILUTILS_TLS_H

#include <mailutils/types.h>
#include <mailutils/cli.h>
#include <mailutils/util.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mu_tls_config
{
  char *cert_file;
  char *key_file;
  char *ca_file;
  char *priorities;
};

enum mu_tls_type
  {
    MU_TLS_CLIENT,
    MU_TLS_SERVER
  };

extern int mu_tls_enable;
extern int mu_tls_cert_file_checks;
extern int mu_tls_key_file_checks;
extern int mu_tls_ca_file_checks;

#define MU_TLS_CERT_FILE_CHECKS			\
  (MU_FILE_SAFETY_GROUP_WRITABLE		\
   | MU_FILE_SAFETY_GROUP_WRITABLE		\
   | MU_FILE_SAFETY_LINKED_WRDIR)

#define MU_TLS_KEY_FILE_CHECKS			\
  (MU_FILE_SAFETY_ALL & ~MU_FILE_SAFETY_OWNER_MISMATCH)

#define MU_TLS_CA_FILE_CHECKS			\
  (MU_FILE_SAFETY_GROUP_WRITABLE		\
   | MU_FILE_SAFETY_GROUP_WRITABLE		\
   | MU_FILE_SAFETY_LINKED_WRDIR)

void mu_tls_cfg_init (void);
  
int mu_tls_stream_create (mu_stream_t *pstream,
			  mu_stream_t strin, mu_stream_t strout,
			  struct mu_tls_config const *conf,
			  enum mu_tls_type type,
			  int flags);
int mu_tls_client_stream_create (mu_stream_t *pstream,
				 mu_stream_t strin, mu_stream_t strout,
				 int flags);

void mu_deinit_tls_libs (void);
int mu_init_tls_libs (void);

enum mu_tls_config_status
  {
    MU_TLS_CONFIG_OK,         /* Configuration OK */
    MU_TLS_CONFIG_NULL,       /* Configuration is empty */ 
    MU_TLS_CONFIG_UNSAFE,     /* At least one file is considered unsafe */
    MU_TLS_CONFIG_FAIL        /* Some files absent (or other system error) */
  };

int mu_tls_config_check (struct mu_tls_config const *conf, int verbose);

extern struct mu_cli_capa mu_cli_capa_tls;
  
#ifdef __cplusplus
}
#endif

#endif /* _MAILUTILS_TLS_H */

