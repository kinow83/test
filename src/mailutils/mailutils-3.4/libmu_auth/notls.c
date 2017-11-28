/* Declare fail-only TLS interfaces in the absense of GNU TLS */
#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <mailutils/tls.h>
#include <mailutils/errno.h>

int
mu_tls_stream_create (mu_stream_t *pstream,
		      mu_stream_t strin, mu_stream_t strout,
		      struct mu_tls_config const *conf,
		      enum mu_tls_type type,
		      int flags)
{
  return ENOSYS;
}

void
mu_deinit_tls_libs (void)
{
}

int
mu_init_tls_libs (void)
{
  mu_tls_enable = 0;
  return 0;
}

