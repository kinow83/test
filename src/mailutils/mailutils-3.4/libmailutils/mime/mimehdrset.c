/* Functions for formatting RFC-2231-compliant mail headers fields.
   GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2001, 2004-2005, 2007, 2009-2012, 2014-2017 Free
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
   Public License along with this library.  If not,
   see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mailutils/mime.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/header.h>
#include <mailutils/stream.h>
#include <mailutils/filter.h>
#include <mailutils/assoc.h>
#include <mailutils/errno.h>

struct header_buffer
{
  mu_stream_t str;     /* Output stream */
  size_t line_len;     /* Length of current line */
  size_t line_max;     /* Max. line length */
};

static int
mime_store_parameter (char const *name, void *value, void *data)
{
  struct mu_mime_param *p = value;
  struct header_buffer *hbuf = data;
  size_t nlen;   /* Length of parameter name
		    (eq. sign, eventual seqence no. and language info mark
		    included) */
  size_t vlen;   /* Length of lang'charset' part */
  int langinfo;  /* True if language info is available */ 
  int quote = 0; /* 2 if the value should be quoted, 0 otherwise */
  int segment = -1; /* Segment sequence number */
  mu_stream_t valstr;  /* Value stream (properly encoded) */
  mu_off_t valsize;    /* Number of octets left in valstr */
  char const *filter_name = NULL; /* Name of the filter for the value */
  int rc;
    
  rc = mu_static_memory_stream_create (&valstr, p->value, strlen (p->value));
  if (rc)
    return rc;
  
  nlen = strlen (name);
  if (p->lang || p->cset)
    {
      vlen = 2;
      if (p->lang)
	vlen += strlen (p->lang);
      if (p->cset)
	vlen += strlen (p->cset);
      langinfo = 1;
      filter_name = "percent";
    }
  else
    {
      if (*mu_str_skip_class_comp (p->value, MU_CTYPE_TSPEC|MU_CTYPE_BLANK))
	{
	  /* Must be in quoted-string, to use within parameter values */
	  quote = 2;
	  filter_name = "dq";
	}
      else
	quote = 0;
      vlen = 0;
      langinfo = 0;
    }

  if (filter_name)
    {
      mu_stream_t tmp;
      
      rc = mu_filter_create (&tmp, valstr, filter_name, MU_FILTER_ENCODE,
			     MU_STREAM_READ | MU_STREAM_SEEK);
      if (rc)
	goto err;
      mu_stream_unref (valstr);
      valstr = tmp;
      rc = mu_memory_stream_create (&tmp, MU_STREAM_RDWR);
      if (rc == 0)
	{
	  rc = mu_stream_copy (tmp, valstr, 0, &valsize);
	  mu_stream_destroy (&tmp);
	}
    }
  else
    rc = mu_stream_size (valstr, &valsize);

  if (rc)
    goto err;

  nlen += langinfo;
  
  rc = mu_stream_seek (valstr, 0, MU_SEEK_SET, NULL);

  if (hbuf->line_max == 0)
    {
      /* No line wrapping requested.  Store the value as it is */
      mu_stream_printf (hbuf->str, "%s", name);
      if (langinfo)
	mu_stream_write (hbuf->str, "*", 1, NULL);
      mu_stream_write (hbuf->str, "=", 1, NULL);
      if (vlen)
	{
	  mu_stream_printf (hbuf->str, "%s'%s'",
			    mu_prstr (p->lang),
			    mu_prstr (p->cset));
	  vlen = 0;
	}
      else if (quote)
	mu_stream_write (hbuf->str, "\"", 1, NULL);
      mu_stream_copy (hbuf->str, valstr, 0, NULL);
      if (quote)
	mu_stream_write (hbuf->str, "\"", 1, NULL);
      if (mu_stream_err (hbuf->str))
	rc = mu_stream_last_error (hbuf->str);
    }
  else
    {
      /* Split the value into sequentially indexed segments, each one no
	 wider than the requested line width.

	 Without special precautions, an encoded character occurring at
	 the end of a segment can be split between this and the following
	 segment to satisfy line width requirements.  To avoid this, the
	 following approach is used:

	 1. The value stream is put to unbuffered mode.
	 2. Before each write, the size of the transcoder output buffer
	    in valstr is set to the number of bytes left in the current
	    line.

	 This way the transcoder will write as many bytes as possible
	 without breaking the encoded constructs while the unbuffered mode
	 will ensure that it will not be called again to fill up the stream
	 buffer.

	 If the line width is insufficient, MU_ERR_BUFSPACE will be returned.
      */
      char *iobuf;

      iobuf = malloc (hbuf->line_max + 1);
      if (!iobuf)
	{
	  rc = errno;
	  goto err;
	}
      
      mu_stream_set_buffer (valstr, mu_buffer_none, 0);

      while (rc == 0 && valsize)
	{
	  mu_off_t start, nr; /* Start and end positions in stream */
	  size_t sz, n;
      
	  mu_stream_write (hbuf->str, ";", 1, NULL);
	  mu_stream_seek (hbuf->str, 0, MU_SEEK_CUR, &start);
      
	  if (segment >= 0)
	    {
	      mu_stream_write (hbuf->str, "\n", 1, NULL);
	      hbuf->line_len = 0;
	      segment++;
	    }
	  else if (hbuf->line_len + valsize + quote + vlen + nlen + 1 >
		   hbuf->line_max)
	    {
	      mu_stream_write (hbuf->str, "\n", 1, NULL);
	      hbuf->line_len = 0;
	      if (hbuf->line_len + valsize + quote + vlen + nlen + 1 >
		   hbuf->line_max)
		segment++;
	    }
	  
	  mu_stream_write (hbuf->str, " ", 1, NULL);
	  
	  if (segment >= 0)
	    mu_stream_printf (hbuf->str, "%s*%d", name, segment);
	  else
	    mu_stream_printf (hbuf->str, "%s", name);
	  if (langinfo)
	    mu_stream_write (hbuf->str, "*", 1, NULL);
	  mu_stream_write (hbuf->str, "=", 1, NULL);
	  mu_stream_seek (hbuf->str, 0, MU_SEEK_CUR, &nr);
	  nlen = nr - start;
	  hbuf->line_len += nlen;
	  start = nr;

	  /* Compute the number of octets to put into the current line.
	     If the requested line width is not enough to accomodate
	     the line, signal the error */
	  if (hbuf->line_max <= (hbuf->line_len + quote + vlen))
	    {
	      rc = MU_ERR_BUFSPACE;
	      break;
	    }

	  sz = hbuf->line_max - (hbuf->line_len + quote + vlen);
	  mu_stream_ioctl (valstr, MU_IOCTL_FILTER,
			   MU_IOCTL_FILTER_SET_OUTBUF_SIZE, &sz);
	  
	  rc = mu_stream_read (valstr, iobuf, sz, &n);
	  if (rc || n == 0)
	    break;
	  
	  if (vlen)
	    {
	      mu_stream_printf (hbuf->str, "%s'%s'",
				mu_prstr (p->lang),
				mu_prstr (p->cset));
	      vlen = 0;
	    }
	  else if (quote)
	    mu_stream_write (hbuf->str, "\"", 1, NULL);

	  mu_stream_write (hbuf->str, iobuf, n, NULL);
	  
	  if (quote)
	    mu_stream_write (hbuf->str, "\"", 1, NULL);
	  mu_stream_seek (hbuf->str, 0, MU_SEEK_CUR, &nr);
	  nr -= start;
	  hbuf->line_len += nr;
	  valsize -= n;
	  
	  if (mu_stream_err (hbuf->str))
	    rc = mu_stream_last_error (hbuf->str);
	}
      free (iobuf);
    }
 err:
  mu_stream_destroy (&valstr);

  return rc;
}

static int
mime_header_format (const char *value, mu_assoc_t params,
		    struct header_buffer *hbuf)
{
  size_t l = strlen (value);
  
  mu_stream_write (hbuf->str, value, l, NULL);
  hbuf->line_len += l;
  return mu_assoc_foreach (params, mime_store_parameter, hbuf);
}

/* Store a header in accordance with RFC 2231, Section 3,
   "Parameter Value Continuations"

   HDR    -  Message header object
   NAME   -  Header name
   VALUE  -  Header value part
   PARAMS -  Named parameters (assoc of struct mu_mime_param *)
   LINE_WIDTH - Maximum line width.
*/

int
mu_mime_header_set_w (mu_header_t hdr, const char *name,
		      const char *value, mu_assoc_t params, size_t line_width)
{
  struct header_buffer hbuf;
  int rc;
  
  rc = mu_memory_stream_create (&hbuf.str, MU_STREAM_RDWR);
  if (rc)
    return rc;
  hbuf.line_len = strlen (name) + 2;
  hbuf.line_max = line_width;
  rc = mime_header_format (value, params, &hbuf);
  if (rc == 0)
    {
      mu_off_t pos;
      char *fmtval;
      
      mu_stream_seek (hbuf.str, 0, MU_SEEK_CUR, &pos);
      fmtval = malloc (pos + 1);
      mu_stream_seek (hbuf.str, 0, MU_SEEK_SET, NULL);
      mu_stream_read (hbuf.str, fmtval, pos, NULL);
      fmtval[pos] = 0;
      rc = mu_header_set_value (hdr, name, fmtval, 1);
      free (fmtval);
    }
  mu_stream_destroy (&hbuf.str);
  return rc;
}

int
mu_mime_header_set (mu_header_t hdr, const char *name,
		    const char *value, mu_assoc_t params)
{
  return mu_mime_header_set_w (hdr, name, value, params, 76);
}

    
  
