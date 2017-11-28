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
#include <mailutils/stream.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>
#include <mailutils/property.h>
#include <mailutils/mu_auth.h>

#include <gnutls/gnutls.h>
#include <mailutils/sys/tls-stream.h>

static void
_mu_gtls_logger(int level, const char *text)
{
  int len = strlen (text);
  if (text[len-1] == '\n')
    len--;
  mu_diag_output (MU_DIAG_DEBUG, "GnuTLS(%d): %.*s", level, len, text);
}

void
mu_deinit_tls_libs (void)
{
  if (mu_tls_enable)
    gnutls_global_deinit ();
  mu_tls_enable = 0;
}

static void
_onexit_deinit (void *ptr MU_ARG_UNUSED)
{
  mu_deinit_tls_libs ();
}

int
mu_init_tls_libs (void)
{
  if (!mu_tls_enable)
    {
      int rc = gnutls_global_init ();
      if (rc == GNUTLS_E_SUCCESS)
	{
	  mu_tls_enable = 1;
	  mu_onexit (_onexit_deinit, NULL);
	  if (mu_debug_level_p (MU_DEBCAT_TLS, MU_DEBUG_PROT))
	    {
	      gnutls_global_set_log_function (_mu_gtls_logger);
	      gnutls_global_set_log_level (110);
	    }
	}
      else
	{
	  mu_error ("gnutls_global_init: %s", gnutls_strerror (rc));
	}
    }
  return mu_tls_enable;
}

/* TLS push & pull functions */

static ssize_t
_tls_stream_pull (gnutls_transport_ptr_t fd, void *buf, size_t size)
{
  mu_stream_t stream = fd;
  int rc;
  size_t rdbytes;
	
  while ((rc = mu_stream_read (stream, buf, size, &rdbytes)) == EAGAIN)
    ;
  
  if (rc)
    return -1;
  return rdbytes;
}

static ssize_t
_tls_stream_push (gnutls_transport_ptr_t fd, const void *buf, size_t size)
{
  mu_stream_t stream = fd;
  int rc;

  rc = mu_stream_write (stream, buf, size, &size);
  if (rc)
    {
      mu_error ("_tls_stream_push: %s",
		mu_stream_strerror (stream, rc)); /* FIXME */
      return -1;
    }

  mu_stream_flush (stream);
  return size;
}

static char default_priority_string[] = "NORMAL";

static int
prep_session (mu_stream_t stream)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;
  gnutls_certificate_credentials_t cred = NULL;
  mu_transport_t transport[2];
  int rc;
  const char *errp;

  if (!sp->cred)
    {
      rc = gnutls_certificate_allocate_credentials (&cred);
      if (rc)
	{
	  mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		    ("gnutls_certificate_allocate_credentials: %s",
		     gnutls_strerror (rc)));
	  sp->tls_err = rc;
	  return MU_ERR_FAILURE;
	}
  
      if (sp->conf.ca_file)
	{
	  rc = gnutls_certificate_set_x509_trust_file (cred, sp->conf.ca_file,
						       GNUTLS_X509_FMT_PEM);
	  if (rc < 0)
	    {
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			("can't use X509 CA file %s: %s",
			 sp->conf.ca_file,
			 gnutls_strerror (rc)));
	      goto cred_err;
	    }
	}
  
      if (sp->conf.cert_file && sp->conf.key_file)
	{
	  rc = gnutls_certificate_set_x509_key_file (cred,
						     sp->conf.cert_file, 
						     sp->conf.key_file,
						     GNUTLS_X509_FMT_PEM);
	  if (rc != GNUTLS_E_SUCCESS)
	    {
	      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
			("can't use X509 cert/key pair (%s,%s): %s",
			 sp->conf.cert_file,
			 sp->conf.key_file,
			 gnutls_strerror (rc)));
	      goto cred_err;
	    }
	}
      sp->cred = cred;
    }
  
  rc = gnutls_init (&sp->session, sp->session_type);
  if (rc != GNUTLS_E_SUCCESS)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("failed to initialize session: %s", gnutls_strerror (rc)));
      goto cred_err;
    }

  rc = gnutls_priority_set_direct (sp->session,
				   sp->conf.priorities
				     ? sp->conf.priorities
				     : default_priority_string,
				   &errp);
  if (rc != GNUTLS_E_SUCCESS)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("error setting priorities near %s: %s",
		 errp, gnutls_strerror (rc)));
      goto cred_err;
    }
  
  rc = gnutls_credentials_set (sp->session, GNUTLS_CRD_CERTIFICATE, sp->cred);
  if (rc)
    {
      mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		("gnutls_credentials_set: %s", gnutls_strerror (rc)));
      goto sess_err;
    }
  
  if (sp->session_type == GNUTLS_SERVER)
    gnutls_certificate_server_set_request (sp->session, GNUTLS_CERT_REQUEST);
  
  rc = mu_stream_ioctl (stream, MU_IOCTL_TRANSPORT, MU_IOCTL_OP_GET,
			transport);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_stream_ioctl", NULL, rc);
      abort (); /* should not happen */
    }
  
  gnutls_transport_set_ptr2 (sp->session,
			     (gnutls_transport_ptr_t) transport[0],
			     (gnutls_transport_ptr_t) transport[1]);
  gnutls_transport_set_pull_function (sp->session, _tls_stream_pull);
  gnutls_transport_set_push_function (sp->session, _tls_stream_push);

  return 0;
  
 sess_err:
  gnutls_deinit (sp->session);
 cred_err:
  if (sp->cred)
    {
      gnutls_certificate_free_credentials (sp->cred);
      sp->cred = NULL;
    }
  sp->tls_err = rc;
  return MU_ERR_FAILURE;
}

static int
_tls_open (mu_stream_t stream)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;
  int rc = 0;
  
  switch (sp->state)
    {
    case state_closed:
      if (sp->session)
	{
	  gnutls_deinit (sp->session);
	  sp->session = NULL;
	}
      /* FALLTHROUGH */

    case state_init:
      rc = prep_session (stream);
      if (rc)
	break;
      rc = gnutls_handshake (sp->session);
      if (rc != GNUTLS_E_SUCCESS)
	{
	  mu_debug (MU_DEBCAT_STREAM, MU_DEBUG_ERROR,
		    ("gnutls_handshake: %s", gnutls_strerror (rc)));
	  sp->tls_err = rc;
	  gnutls_deinit (sp->session);
	  sp->session = NULL;
	  sp->state = state_init;
	}
      else
	/* FIXME: if (ssl_cafile) verify_certificate (s->session); */
	sp->state = state_open;
      break;

    default:
      rc = MU_ERR_BADOP;
    }
  return rc;
}

/* TLS stream */
static int
_tls_read (struct _mu_stream *str, char *buf, size_t bufsize,
	   size_t *pnread)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *)str;
  return mu_stream_read (sp->transport[0], buf, bufsize, pnread);
}

static int
_tls_write (struct _mu_stream *str, const char *buf, size_t bufsize,
	    size_t *pnwrite)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *)str;
  return mu_stream_write (sp->transport[1], buf, bufsize, pnwrite);
}

static int
get_cipher_info (gnutls_session_t session, mu_property_t *pprop)
{
  mu_property_t prop;
  const char *s;
  int rc;

  if (!pprop)
    return EINVAL;

  rc = mu_property_create_init (&prop, mu_assoc_property_init, NULL);
  if (rc)
    return rc;

  s = gnutls_protocol_get_name (gnutls_protocol_get_version (session));
  mu_property_set_value (prop, "protocol", s, 1);

  s = gnutls_cipher_get_name (gnutls_cipher_get (session));
  mu_property_set_value (prop, "cipher", s, 1);

  s = gnutls_mac_get_name (gnutls_mac_get (session));
  mu_property_set_value (prop, "mac", s, 1);

  *pprop = prop;

  return 0;
}

static int
_tls_ioctl (struct _mu_stream *stream, int code, int opcode, void *arg)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;

  switch (code)
    {
    case MU_IOCTL_TRANSPORT:
      switch (opcode)
	{
	case MU_IOCTL_OP_GET:
	  if (!arg)
	    return EINVAL;
	  else
	    {
	      mu_transport_t *ptrans, trans[2];

	      ptrans = arg;
	      mu_stream_ioctl (sp->transport[0], MU_IOCTL_TRANSPORT,
			       MU_IOCTL_OP_GET, trans);
	      ptrans[0] = trans[0];
	      mu_stream_ioctl (sp->transport[1], MU_IOCTL_TRANSPORT,
			       MU_IOCTL_OP_GET, trans);
	      ptrans[1] = trans[0];
	    }
	  break;

	case MU_IOCTL_OP_SET:
	  return ENOSYS;

	default:
	  return EINVAL;
	}
      break;

    case MU_IOCTL_TRANSPORT_BUFFER:
      if (!arg)
	return EINVAL;
      else
	{
	  struct mu_buffer_query *qp = arg;
	  switch (opcode)
	    {
	    case MU_IOCTL_OP_GET:
	      if (!MU_TRANSPORT_VALID_TYPE (qp->type) ||
		  !sp->transport[qp->type])
		return EINVAL;
	      return mu_stream_get_buffer (sp->transport[qp->type], qp);

	    case MU_IOCTL_OP_SET:
	      if (!MU_TRANSPORT_VALID_TYPE (qp->type) ||
		  !sp->transport[qp->type])
		return EINVAL;
	      return mu_stream_set_buffer (sp->transport[qp->type],
					   qp->buftype, qp->bufsize);

	    default:
	      return EINVAL;
	    }
	}
      break;
      
    case MU_IOCTL_TLSSTREAM:
      switch (opcode)
	{
	case MU_IOCTL_TLS_GET_CIPHER_INFO:
	  return get_cipher_info (sp->session, arg);

	default:
	  return EINVAL;
	}
      break;
      
    default:
      return ENOSYS;
    }
  return 0;
}

static int
_tls_wait (struct _mu_stream *stream, int *pflags, struct timeval *tvp)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;
  int rc = EINVAL;
  
  if (*pflags == MU_STREAM_READY_RD)
    rc = mu_stream_wait (sp->transport[0], pflags, tvp);
  else if (*pflags == MU_STREAM_READY_WR)
    rc = mu_stream_wait (sp->transport[1], pflags, tvp);
  return rc;
}

static int
_tls_flush (struct _mu_stream *stream)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;
  return mu_stream_flush (sp->transport[1]);
}

static int
_tls_close (mu_stream_t stream)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;
  
  if (sp->session && sp->state == state_open)
    {
      gnutls_bye (sp->session, GNUTLS_SHUT_RDWR);
      sp->state = state_closed;
    }
  
  mu_stream_close (sp->transport[0]);
  mu_stream_close (sp->transport[1]);
  return 0;
}

static void free_conf (struct mu_tls_config *conf);

static void
_tls_done (struct _mu_stream *stream)
{
  struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;
  
  if (sp->session)
    gnutls_deinit (sp->session);
  if (sp->cred)
    gnutls_certificate_free_credentials (sp->cred);
  free_conf (&sp->conf);
  mu_stream_destroy (&sp->transport[0]);
  mu_stream_destroy (&sp->transport[1]);
}

static const char *
_tls_error_string (struct _mu_stream *stream, int rc)
{
  if (rc == EIO)
    {
      struct _mu_tls_stream *sp = (struct _mu_tls_stream *) stream;
      return gnutls_strerror (sp->tls_err);
    }
  return mu_strerror (rc);
}

static void
free_conf (struct mu_tls_config *conf)
{
  free (conf->cert_file);
  free (conf->key_file);
  free (conf->ca_file);
}

static int
copy_conf (struct mu_tls_config *dst, struct mu_tls_config const *src)
{
  if (src->cert_file)
    {
      dst->cert_file = strdup (src->cert_file);
      if (!dst->cert_file)
	return errno;
    }
  else
    dst->cert_file = NULL;

  if (src->key_file)
    {
      dst->key_file = strdup (src->key_file);
      if (!dst->cert_file)
	{
	  int rc = errno;
	  free (dst->cert_file);
	  return rc;
	}
    }
  else
    dst->key_file = NULL;
  
  if (src->ca_file)
    {
      dst->ca_file = strdup (src->ca_file);
      if (!dst->ca_file)
	{
	  int rc = errno;
	  free (dst->cert_file);
	  free (dst->key_file);
	  return rc;
	}
    }
  else
    dst->ca_file = NULL;

  return 0;
}

int
mu_tls_stream_create (mu_stream_t *pstream,
		      mu_stream_t strin, mu_stream_t strout,
		      struct mu_tls_config const *conf,
		      enum mu_tls_type type,
		      int flags)
{
  struct _mu_tls_stream *sp;
  int rc;
  mu_stream_t stream;
  int session_type;
  
  if (!pstream)
    return MU_ERR_OUT_PTR_NULL;
  if (!conf || !strin || !strout)
    return EINVAL;

  if (!mu_init_tls_libs ())
    return ENOSYS;

  switch (mu_tls_config_check (conf, 1))
    {
    case MU_TLS_CONFIG_OK:
    case MU_TLS_CONFIG_NULL:
      break;
    case MU_TLS_CONFIG_UNSAFE:
      return EACCES;
    case MU_TLS_CONFIG_FAIL:
      return ENOENT;
    }
  
  switch (type)
    {
    case MU_TLS_CLIENT:
      session_type = GNUTLS_CLIENT;
      break;
      
    case MU_TLS_SERVER:
      session_type = GNUTLS_SERVER;
      break;
      
    default:
      return EINVAL;
    }
  
  sp = (struct _mu_tls_stream *)
    _mu_stream_create (sizeof (*sp), MU_STREAM_RDWR);
  if (!sp)
    return ENOMEM;
  sp->session_type = session_type;
  sp->state = state_init;
  sp->session = NULL;
  sp->cred = NULL;

  rc = copy_conf (&sp->conf, conf);
  if (rc)
    {
      free (sp);
      return rc;
    }
	  
  sp->stream.read = _tls_read; 
  sp->stream.write = _tls_write;
  sp->stream.flush = _tls_flush;
  sp->stream.open = _tls_open; 
  sp->stream.close = _tls_close;
  sp->stream.done = _tls_done; 
  sp->stream.ctl = _tls_ioctl;
  sp->stream.wait = _tls_wait;
  sp->stream.error_string = _tls_error_string;

  mu_stream_set_buffer (strin, mu_buffer_none, 0);
  mu_stream_set_buffer (strout, mu_buffer_none, 0);

  stream = (mu_stream_t) sp;
  
  rc = mu_tls_io_stream_create (&sp->transport[0], strin,
				MU_STREAM_READ, sp);
  if (rc)
    goto err;
  
  rc = mu_tls_io_stream_create (&sp->transport[1], strout,
				MU_STREAM_WRITE, sp);
  if (rc)
    goto err;
  
  mu_stream_set_buffer (stream, mu_buffer_line, 0);
  rc = mu_stream_open (stream);
  if (rc)
    goto err;
  *pstream = stream;
  return 0;
  
 err:
  mu_stream_destroy (&stream);
  return rc;
}

int
mu_tls_client_stream_create (mu_stream_t *pstream,
			     mu_stream_t strin, mu_stream_t strout, int flags)
{
  struct mu_tls_config conf = {
    .cert_file = NULL,
    .key_file = NULL,
    .ca_file = NULL,
    .priorities = NULL
  };

  return mu_tls_stream_create (pstream, strin, strout, &conf, MU_TLS_CLIENT,
			       flags);
}

