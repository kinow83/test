/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2010-2012, 2014-2017 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <mailutils/types.h>
#include <mailutils/errno.h>
#include <mailutils/cctype.h>
#include <mailutils/cstr.h>
#include <mailutils/log.h>

#include <mailutils/nls.h>
#include <mailutils/stream.h>
#include <mailutils/debug.h>
#include <mailutils/locus.h>
#include <mailutils/sys/logstream.h>

char *_mu_severity_str[] = {
  N_("debug"),
  N_("info"),
  N_("notice"),
  N_("warning"),
  N_("error"),
  N_("crit"),
  N_("alert"),
  N_("emerg"),
};
int _mu_severity_num = MU_ARRAY_SIZE (_mu_severity_str);

int
mu_severity_from_string (const char *str, unsigned *pn)
{
  int i;

  for (i = 0; i < _mu_severity_num; i++)
    {
      if (mu_c_strcasecmp (_mu_severity_str[i], str) == 0)
	{
	  *pn = i;
	  return 0;
	}
    }
  return MU_ERR_NOENT;
}

int
mu_severity_to_string (unsigned n, const char **pstr)
{
  if (n >= _mu_severity_num)
    return MU_ERR_NOENT;
  *pstr = _mu_severity_str[n];
  return 0;
}

static void
lr_set_line (struct mu_locus_range *loc, unsigned val, int end)
{
  if (end)
    loc->end.mu_line = val;
  else
    loc->beg.mu_line = val;
}

static void
lr_set_col (struct mu_locus_range *loc, unsigned val, int end)
{
  if (end)
    loc->end.mu_col = val;
  else
    loc->beg.mu_col = val;
}

static int
lr_set_file (struct mu_locus_range *loc, char const *fname, unsigned len,
	     int end)
{
  char const *refname;
  struct mu_locus_point *pt = end ? &loc->end : &loc->beg;
  int rc;
  
  if (fname == NULL)
    {
      refname = NULL;
      rc = 0;
    }
  else if (len == 0)
    rc = mu_ident_ref (fname, &refname);
  else
    {
      char *name;

      name = malloc (len + 1);
      if (!name)
	return errno;
      memcpy (name, fname, len);
      name[len] = 0;
      rc = mu_ident_ref (name, &refname);
      free (name);
    }
  if (rc)
    return rc;
  mu_ident_deref (pt->mu_file);
  pt->mu_file = refname;
  return 0;
}

/* Field modification map (binary):

     FfLlCc

   The bits f, l, and c (file, line, and column) are toggled cyclically.
   The value 0 means locus beg, 1 meand locus end.
   The bits F, L, and C are set once and indicate that the corresponding
   bit was toggled at least once.
 */
#define FMM_COL  0
#define FMM_LINE 1
#define FMM_FILE 2

#define FMM_SHIFT(n) ((n)<<1)
#define FMM_MASK(n) (0x3 << FMM_SHIFT (n))
#define FMM_VAL(m,n) (((m) >> FMM_SHIFT (n)) & 0x1)
#define FMM_SET(m,n,v) ((m) = ((m) & ~FMM_MASK (n)) | (((v) << FMM_SHIFT (n))|0x2))
#define FMM_CYCLE(m, n) \
  FMM_SET ((m), (n), ((FMM_VAL ((m), (n)) + 1) % 2))

static int
_log_write (struct _mu_stream *str, const char *buf, size_t size,
	    size_t *pnwrite)
{
  struct _mu_log_stream *sp = (struct _mu_log_stream *)str;
  unsigned severity = sp->severity;
  int logmode = sp->logmode;
  struct mu_locus_range loc;
  int fmm = 0;
  unsigned flen = 0;
  int save_locus = 0;
  int rc;
  int escape_error = 0;
  
#define NEXT do { if (size == 0) return EINVAL; buf++; size--; } while (0)
#define READNUM(n) do {				\
    unsigned __x = 0;				\
    if (*buf != '<')				\
      return EINVAL;				\
    NEXT;					\
    while (*buf != '>')				\
      {						\
        if (!mu_isdigit (*buf))			\
	  return EINVAL;			\
	__x = __x * 10 + (*buf - '0');		\
	NEXT;					\
      }						\
    NEXT;					\
    n = __x;					\
} while (0)

  loc = sp->locrange;
  mu_ident_ref (loc.beg.mu_file, &loc.beg.mu_file);
  mu_ident_ref (loc.end.mu_file, &loc.end.mu_file);
  
  /* Tell them we've consumed everything */
  *pnwrite = size;
  
  /* Process escapes */
  while (!escape_error && *buf == '\033')
    {
      int code;
      unsigned n;
      
      NEXT;
      code = *buf;
      NEXT;
      switch (code)
	{
	case 'S':
	  /* Save locus */
	  save_locus = 1;
	  break;

	case 's':
	  /* Severity code in decimal (single digit) */
	  if (*buf == '<')
	    READNUM (severity);
	  else if (mu_isdigit (*buf))
	    {
	      severity = *buf - '0';
	      NEXT;
	    }
	  else
	    return EINVAL;
	  break;
	  
	case 'O':
	  /* Flags on.  Followed by a bitmask of MU_LOGMODE_*, in decimal  */
	  READNUM (n);
	  logmode |= n;
	  break;
	  
	case 'X':
	  /* Flags off.  Followed by a bitmask of MU_LOGMODE_*, in decimal  */
	  READNUM (n);
	  logmode &= ~n;
	  break;
	  
	case 'l':
	  /* Input line (decimal) */
	  READNUM (n);
	  lr_set_line (&loc, n, FMM_VAL (fmm, FMM_LINE));
	  FMM_CYCLE (fmm, FMM_LINE);
	  logmode |= MU_LOGMODE_LOCUS;
	  break;

	case 'c':
	  /* Column in input line (decimal) */
	  READNUM (n);
	  lr_set_col (&loc, n, FMM_VAL (fmm, FMM_COL));
	  FMM_CYCLE (fmm, FMM_COL);
	  logmode |= MU_LOGMODE_LOCUS;
	  break;
	  
	case 'f':
	  /* File name. Format: <N>S */
	  READNUM (flen);
	  lr_set_file (&loc, buf, flen, FMM_VAL (fmm, FMM_FILE));
	  FMM_CYCLE (fmm, FMM_FILE);
	  buf += flen;
	  size -= flen;
	  logmode |= MU_LOGMODE_LOCUS;
	  break;

	default:
	  buf -= 2;
	  size += 2;
	  escape_error = 1;
	}
    }

  if (severity >= _mu_severity_num)
    severity = MU_LOG_EMERG;

  if (save_locus)
    mu_locus_range_copy (&sp->locrange, &loc);
  
  if (severity < sp->threshold)
    rc = 0;
  else
    {
      mu_stream_ioctl (sp->transport, MU_IOCTL_LOGSTREAM,
		       MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  
      if ((logmode & MU_LOGMODE_LOCUS) && loc.beg.mu_file)
	{
	  mu_stream_print_locus_range (sp->transport, &loc);
  	  mu_stream_write (sp->transport, ": ", 2, NULL);
	}
      
      if ((logmode & MU_LOGMODE_SEVERITY) &&
	  !(sp->sevmask & MU_DEBUG_LEVEL_MASK (severity)))
	{
	  char *s = gettext (_mu_severity_str[severity]);
	  rc = mu_stream_write (sp->transport, s, strlen (s), NULL);
	  if (rc)
	    return rc;
	  mu_stream_write (sp->transport, ": ", 2, NULL);
	}
      rc = mu_stream_write (sp->transport, buf, size, NULL);
    }
  
  mu_ident_deref (loc.beg.mu_file);
  mu_ident_deref (loc.end.mu_file);
  return rc;
}

static int
_log_flush (struct _mu_stream *str)
{
  struct _mu_log_stream *sp = (struct _mu_log_stream *)str;
  return mu_stream_flush (sp->transport);
}

static void
_log_done (struct _mu_stream *str)
{
  struct _mu_log_stream *sp = (struct _mu_log_stream *)str;
  mu_locus_range_deinit (&sp->locrange);
  mu_stream_destroy (&sp->transport);
}

static int
_log_close (struct _mu_stream *str)
{
  struct _mu_log_stream *sp = (struct _mu_log_stream *)str;
  return mu_stream_close (sp->transport);
}

static int
_log_setbuf_hook (mu_stream_t str, enum mu_buffer_type type, size_t size)
{
  if (type != mu_buffer_line)
    return EACCES;
  return 0;
}

static int
_log_ctl (struct _mu_stream *str, int code, int opcode, void *arg)
{
  struct _mu_log_stream *sp = (struct _mu_log_stream *)str;
  int status;
  
  switch (code)
    {
    case MU_IOCTL_TRANSPORT:
      if (!arg)
	return EINVAL;
      else
	{
	  mu_transport_t *ptrans = arg;
	  switch (opcode)
	    {
	    case MU_IOCTL_OP_GET:
	      ptrans[0] = (mu_transport_t) sp->transport;
	      ptrans[1] = NULL;
	      break;

	    case MU_IOCTL_OP_SET:
	      ptrans = arg;
	      if (ptrans[0])
		sp->transport = (mu_stream_t) ptrans[0];
	      break;

	    default:
	      return EINVAL;
	    }
	}
      break;

    case MU_IOCTL_SUBSTREAM:
      if (sp->transport &&
          ((status = mu_stream_ioctl (sp->transport, code, opcode, arg)) == 0 ||
           status != ENOSYS))
        return status;
      /* fall through */

    case MU_IOCTL_TOPSTREAM:
      if (!arg)
	return EINVAL;
      else
	{
	  mu_stream_t *pstr = arg;
	  switch (opcode)
	    {
	    case MU_IOCTL_OP_GET:
	      pstr[0] = sp->transport;
	      mu_stream_ref (pstr[0]);
	      pstr[1] = NULL;
	      break;

	    case MU_IOCTL_OP_SET:
	      mu_stream_unref (sp->transport);
	      sp->transport = pstr[0];
	      mu_stream_ref (sp->transport);
	      break;

	    default:
	      return EINVAL;
	    }
	}
      break;
      
    case MU_IOCTL_LOGSTREAM:
      switch (opcode)
	{
	case MU_IOCTL_LOGSTREAM_GET_SEVERITY:
	  if (!arg)
	    return EINVAL;
	  *(unsigned*)arg = sp->severity;
	  break;
      
	case MU_IOCTL_LOGSTREAM_SET_SEVERITY:
	  if (!arg)
	    return EINVAL;
	  if (*(unsigned*)arg >= _mu_severity_num)
	    return EINVAL;
	  sp->severity = *(unsigned*)arg;
	  break;

	case MU_IOCTL_LOGSTREAM_GET_MODE:
	  if (!arg)
	    return EINVAL;
	  *(int*)arg = sp->logmode;
	  break;

	case MU_IOCTL_LOGSTREAM_SET_MODE:
	  if (!arg)
	    return EINVAL;
	  sp->logmode = *(int*)arg;
	  break;
      
	case MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE:
	  {
	    struct mu_locus_range *lr = arg;
	    if (!arg)
	      {
		mu_ident_deref (sp->locrange.beg.mu_file);
		mu_ident_deref (sp->locrange.end.mu_file);
		memset (&sp->locrange, 0, sizeof sp->locrange);
	      }
	    else
	      {
		char const *begname, *endname;
		
		status = mu_ident_ref (lr->beg.mu_file, &begname);
		if (status)
		  return status;
		status = mu_ident_ref (lr->end.mu_file, &endname);
		if (status)
		  {
		    mu_ident_deref (begname);
		    return status;
		  }
		mu_ident_deref (sp->locrange.beg.mu_file);
		sp->locrange.beg.mu_file = begname;
		sp->locrange.beg.mu_line = lr->beg.mu_line;
		sp->locrange.beg.mu_col = lr->beg.mu_col;

		mu_ident_deref (sp->locrange.end.mu_file);
		sp->locrange.end.mu_file = endname;
		sp->locrange.end.mu_line = lr->end.mu_line;
		sp->locrange.end.mu_col = lr->end.mu_col;
	      }
	  }
	  break;

	case MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE:
	  if (!arg)
	    return EINVAL;
	  else
	    {
	      struct mu_locus_range *lr = arg;
	      char const *begname, *endname;

	      status = mu_ident_ref (sp->locrange.beg.mu_file, &begname);
	      if (status)
		return status;
	      status = mu_ident_ref (sp->locrange.end.mu_file, &endname);
	      if (status)
		{
		  mu_ident_deref (begname);
		  return status;
		}
	      lr->beg.mu_file = begname;
	      lr->beg.mu_line = sp->locrange.beg.mu_line;
	      lr->beg.mu_col = sp->locrange.beg.mu_col;
	      lr->end.mu_file = endname;
	      lr->end.mu_line = sp->locrange.end.mu_line;
	      lr->end.mu_col = sp->locrange.end.mu_col;
	    }
	  break;
	  
	case MU_IOCTL_LOGSTREAM_GET_LOCUS_DEPRECATED:
	  if (!arg)
	    return EINVAL;
	  else
	    {
	      struct mu_locus_DEPRECATED *ploc = arg;
	      if (sp->locrange.beg.mu_file)
		{
		  ploc->mu_file = strdup (sp->locrange.beg.mu_file);
		  if (!ploc->mu_file)
		    return ENOMEM;
		}
	      else
		ploc->mu_file = NULL;
	      ploc->mu_line = sp->locrange.beg.mu_line;
	      ploc->mu_col = sp->locrange.beg.mu_col;
	    }
	  break;
	
	case MU_IOCTL_LOGSTREAM_SET_LOCUS_DEPRECATED:
	  {
	    struct mu_locus_DEPRECATED *ploc = arg;

	    mu_ident_deref (sp->locrange.end.mu_file);
	    sp->locrange.end.mu_file = NULL;
	    if (arg)
	      {
		status = lr_set_file (&sp->locrange, ploc->mu_file, 0, 0);
		if (status)
		  return status;
		lr_set_line (&sp->locrange, ploc->mu_line, 0);
		lr_set_col (&sp->locrange, ploc->mu_col, 0);
	      }
	    else
	      {
		mu_ident_deref (sp->locrange.beg.mu_file);
		sp->locrange.beg.mu_file = NULL;
	      }
	    
	    break;
	  }

	case MU_IOCTL_LOGSTREAM_SET_LOCUS_LINE:
	  if (!arg)
	    return EINVAL;
	  sp->locrange.beg.mu_line = *(unsigned*)arg;
	  break;
	  
	case MU_IOCTL_LOGSTREAM_SET_LOCUS_COL:
	  if (!arg)
	    return EINVAL;
	  sp->locrange.beg.mu_col = *(unsigned*)arg;
	  break;
	  
	case MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_LINE:
	  if (!arg)
	    sp->locrange.beg.mu_line++;
	  else
	    sp->locrange.beg.mu_line += *(int*)arg;
	  break;

	case MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_COL:
	  if (!arg)
	    sp->locrange.beg.mu_col++;
	  else
	    sp->locrange.beg.mu_col += *(int*)arg;
	  break;

	case MU_IOCTL_LOGSTREAM_SUPPRESS_SEVERITY:
	  if (!arg)
	    sp->threshold = MU_LOG_DEBUG;
	  else if (*(unsigned*)arg >= _mu_severity_num)
	    return MU_ERR_NOENT;
	  sp->threshold = *(unsigned*)arg;
	  break;
	  
	case MU_IOCTL_LOGSTREAM_SUPPRESS_SEVERITY_NAME:
	  if (!arg)
	    sp->threshold = MU_LOG_DEBUG;
	  else
	    return mu_severity_from_string ((const char *) arg, &sp->threshold);

	case MU_IOCTL_LOGSTREAM_GET_SEVERITY_MASK:
	  if (!arg)
	    return EINVAL;
	  *(int*)arg = sp->sevmask;
	  break;

	case MU_IOCTL_LOGSTREAM_SET_SEVERITY_MASK:
	  if (!arg)
	    return EINVAL;
	  sp->sevmask = *(int*)arg;
	  break;

	case MU_IOCTL_LOGSTREAM_CLONE:
	  if (!arg)
	    return EINVAL;
	  else
	    {
	      mu_stream_t str;
	      struct _mu_log_stream *newp;
	      int rc = mu_log_stream_create (&str, sp->transport);
	      if (rc)
		return rc;
	      newp = (struct _mu_log_stream *) str;
	      newp->severity = sp->severity;
	      newp->threshold = sp->threshold;
	      newp->logmode = sp->logmode;
	      newp->sevmask = sp->sevmask;
	      newp->locrange = sp->locrange;
	      mu_ident_ref (sp->locrange.beg.mu_file,
			    &sp->locrange.beg.mu_file);
	      mu_ident_ref (sp->locrange.end.mu_file,
			    &sp->locrange.end.mu_file);
	      
	      *(mu_stream_t*) arg = str;
	    }
	  break;
	  
	default:
	  return EINVAL;
	}
      break;

    case MU_IOCTL_FILTER:
    case MU_IOCTL_SYSLOGSTREAM:
      return mu_stream_ioctl (sp->transport, code, opcode, arg);
            
    default:
      return ENOSYS;
    }
  return 0;
}

void
_mu_log_stream_setup (struct _mu_log_stream *sp, mu_stream_t transport)
{
  mu_stream_t stream;
  
  sp->base.write = _log_write;
  sp->base.flush = _log_flush;
  sp->base.close = _log_close;
  sp->base.done = _log_done;
  sp->base.setbuf_hook = _log_setbuf_hook;
  sp->base.ctl = _log_ctl;
  sp->transport = transport;
  mu_stream_ref (transport);
  sp->severity = MU_LOG_ERROR;
  sp->logmode = 0;
  
  stream = (mu_stream_t) sp;
  mu_stream_set_buffer (stream, mu_buffer_line, 0);
}

int
mu_log_stream_create (mu_stream_t *pstr, mu_stream_t transport)
{
  struct _mu_log_stream *sp;
  
  sp = (struct _mu_log_stream *)
    _mu_stream_create (sizeof (*sp), MU_STREAM_WRITE);
  if (!sp)
    return ENOMEM;
  _mu_log_stream_setup (sp, transport);
  *pstr = (mu_stream_t) sp;
  
  return 0;
}

static char wiki_url[] = "http://mailutils.org/wiki/Source_location_API#Deprecated_interface";

int
mu_ioctl_logstream_get_locus_deprecated (void)
{
  static int warned;
  if (!warned)
    {
      mu_error (_("the program uses MU_IOCTL_LOGSTREAM_GET_LOCUS, which is deprecated"));
      mu_error (_("please see %s, for detailed guidelines"), wiki_url);
      warned = 1;
    }
  return MU_IOCTL_LOGSTREAM_GET_LOCUS_DEPRECATED;
}

int
mu_ioctl_logstream_set_locus_deprecated (void)
{
  static int warned;
  if (!warned)
    {
      mu_error (_("program uses MU_IOCTL_LOGSTREAM_SET_LOCUS, which is deprecated"));
      mu_error (_("please see %s, for detailed guidelines"), wiki_url);
      warned = 1;
    }
  return MU_IOCTL_LOGSTREAM_SET_LOCUS_DEPRECATED;
}


