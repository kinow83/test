/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2010-2012, 2014-2017 Free Software Foundation, Inc.

   GNU Mailutils is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Mailutils is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Mailutils.  If not, see <http://www.gnu.org/licenses/>. */

#if defined(HAVE_CONFIG_H)
# include <config.h>
#endif
#include <mailutils/mailutils.h>
#include <sysexits.h>
#include "mu.h"

char stat_docstring[] = N_("display mailbox status");
char stat_args_doc[] = N_("[MAILBOX]");

char *format_string = "%:t\n\
%:p\n\
%:U\n\
%:s\n\
%:c\n\
%:r\n\
%:u\n\
%:v\n\
%:n\n\
%:A\n";

static struct mu_option stat_options[] = {
  { "format", 'c', N_("STRING"), MU_OPTION_DEFAULT,
    N_("defines output format"),
    mu_c_string, &format_string },
  MU_OPTION_END
};

#define prcat2(a, b) a ## b
#define PR_C(field, fmt)			\
  static void					\
  prcat2(pr_,field) (mu_c_storage_t *stor)	\
  {						\
    mu_printf (fmt, stor->field);		\
  }						\

PR_C(c_string, "%s")
PR_C(c_short, "%hd")
PR_C(c_ushort, "%hu")
PR_C(c_int, "%d")
PR_C(c_uint, "%u")
PR_C(c_long, "%ld")
PR_C(c_ulong, "%lu")
PR_C(c_size, "%zu")
PR_C(c_off, "%" MU_PRI_OFF_T)

static void
pr_c_time (mu_c_storage_t *stor)
{
  mu_printf ("%lu", (unsigned long)stor->c_time);
}

static void
pr_c_time_h (mu_c_storage_t *stor)
{
  struct tm *tm = localtime (&stor->c_time);
  mu_c_streamftime (mu_strout, "%Y-%m-%d %H:%M:%S %Z", tm, NULL);
}

static void (*c_fmt[]) (mu_c_storage_t *stor) = {
#define D(t) [prcat2(mu_,t)] = prcat2(pr_,t)
  D(c_string),
  D(c_short),
  D(c_ushort),
  D(c_int),
  D(c_uint),
  D(c_long),
  D(c_ulong),
  D(c_size),
  D(c_off),
  D(c_time)
#undef D
};

static void
mu_c_output (mu_c_type_t type, mu_c_storage_t *cstor)
{
  if (c_fmt[type])
    c_fmt[type] (cstor);
  else
    abort ();
}

struct mbox_property
{
  char fmt;
  char *title;
  mu_c_type_t type;
  int (*fun) (mu_mailbox_t, char const *, mu_c_storage_t *);
  void (*prt) (mu_c_storage_t *stor);
};

static int get_type (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_path (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_url (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_size (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_count (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_recent (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_unseen (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_uidvalidity (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_uidnext (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_atime (mu_mailbox_t, char const *, mu_c_storage_t *);
static int get_name (mu_mailbox_t, char const *, mu_c_storage_t *);

static struct mbox_property proptab[] = {
  { 't', N_("type"),               mu_c_string, get_type },
  { 'p', N_("path"),               mu_c_string, get_path },
  { 'U', N_("URL"),                mu_c_string, get_url },
  { 's', N_("size"),               mu_c_off, get_size },
  { 'c', N_("messages"),           mu_c_size, get_count },
  { 'r', N_("recent messages"),    mu_c_size, get_recent },
  { 'u', N_("first unseen"),       mu_c_size, get_unseen },
  { 'v', N_("uidvalidity"),        mu_c_ulong, get_uidvalidity },
  { 'n', N_("next uid"),           mu_c_size, get_uidnext },
  { 'a', N_("access"),             mu_c_time, get_atime },
  { 'A', N_("access"),             mu_c_time, get_atime, pr_c_time_h },
  { 'f', N_("name"),               mu_c_string, get_name },
  { 0 }
};

static struct mbox_property *
propfmt (int fmt)
{
  struct mbox_property *p;
  for (p = proptab; p->fmt; p++)
    if (p->fmt == fmt)
      return p;
  return NULL;
}

static char const *
fmtspec (char const *fmt, mu_mailbox_t mbx, const char *name)
{
  int c;
  int title = 0;
  struct mbox_property *prop;

  if (!*++fmt)
    {
      mu_stream_write (mu_strout, fmt - 1, 1, NULL);
      return fmt;
    }
  
  c = *fmt++;

  if (c == '%')
    {
      mu_stream_write (mu_strout, fmt - 1, 1, NULL);
      return fmt;
    }

  if (c == ':')
    {
      if (*fmt == 0)
	{
	  mu_stream_write (mu_strout, fmt - 2, 2, NULL);
	  return fmt;
	}
      c = *fmt++;
      title = 1;
    }
  prop = propfmt (c);
  if (prop)
    {
      int rc;
      mu_c_storage_t cstor;
      if (title)
	mu_printf ("%s: ", gettext (prop->title));
      rc = prop->fun (mbx, name, &cstor);
      switch (rc)
	{
	case 0:
	  if (prop->prt)
	    prop->prt (&cstor);
	  else
	    mu_c_output (prop->type, &cstor);
	  if (prop->type == mu_c_string)
	    free (cstor.c_string);
	  break;

	case MU_ERR_EMPTY_VFN:
	case ENOSYS:
	  mu_printf (_("N/A"));
	  break;

	default:
	  mu_printf ("[%s]", mu_strerror (rc));
	}
    }
  else
    mu_stream_write (mu_strout, "?", 1, NULL);
  return fmt;
}

void
format_stat (char const *fmt, mu_mailbox_t mbx, const char *name)
{
  int c;
  
  while (*fmt)
    {
      switch (*fmt)
	{
	case '%':
	  fmt = fmtspec (fmt, mbx, name);
	  break;
	  
	case '\\':
	  if (fmt[1] && (c = mu_wordsplit_c_unquote_char (fmt[1])))
	    {
	      mu_printf ("%c", c);
	      fmt += 2;
	      break;
	    }
	  /* fall through */
	default:
	  mu_stream_write (mu_strout, fmt, 1, NULL);
	  if (*fmt == '\n' && fmt[1] == 0)
	    return;
	  fmt++;
	}
    }
  mu_printf ("\n");
}

int
main (int argc, char **argv)
{
  int rc;
  mu_mailbox_t mbox;
  const char *name;
    
  mu_register_all_mbox_formats ();
  
  mu_action_getopt (&argc, &argv, stat_options, stat_docstring, stat_args_doc);
  if (argc > 1)
    {
      mu_error (_("too many arguments"));
      return EX_USAGE;
    }
  name = argv[0];
  
  rc = mu_mailbox_create_default (&mbox, name);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mailbox_create_default", name, rc);
      return EX_UNAVAILABLE;
    }

  rc = mu_mailbox_open (mbox, MU_STREAM_READ);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mailbox_open", name, rc);
      return EX_UNAVAILABLE;
    }

  if (!name)
    {
      mu_url_t url;
      mu_mailbox_get_url (mbox, &url);
      name = mu_url_to_string (url);
    }

  format_stat (format_string, mbox, name);

  mu_mailbox_close (mbox);
  mu_mailbox_destroy (&mbox);
  return 0;
}

static int
get_type (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  mu_url_t url;
  int rc;
  
  rc = mu_mailbox_get_url (mbox, &url);
  if (rc == 0)
    rc = mu_url_aget_scheme (url, &cstor->c_string);
  return rc;
}

static int
get_path (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  int rc;
  mu_url_t url;

  rc = mu_mailbox_get_url (mbox, &url);
  if (rc == 0)
    rc = mu_url_aget_path (url, &cstor->c_string);
  return rc;
}

static int
get_url (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  mu_url_t url;
  int rc;
  
  rc = mu_mailbox_get_url (mbox, &url);
  if (rc == 0)
    cstor->c_string = mu_strdup (mu_url_to_string (url));
  return rc;
}

static int
get_size (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  return mu_mailbox_get_size (mbox, &cstor->c_off);
}

static int
get_count (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  return mu_mailbox_messages_count (mbox, &cstor->c_size);
}

static int
get_recent (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  return mu_mailbox_messages_recent (mbox, &cstor->c_size);
}

static int
get_unseen (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  return mu_mailbox_message_unseen (mbox, &cstor->c_size);
}

static int
get_uidvalidity (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  return mu_mailbox_uidvalidity (mbox, &cstor->c_ulong);
}

static int
get_uidnext (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  return mu_mailbox_uidnext (mbox, &cstor->c_size);
}

static int
get_atime (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  return mu_mailbox_access_time (mbox, &cstor->c_time);
}

static int
get_name (mu_mailbox_t mbox, char const *mbname, mu_c_storage_t *cstor)
{
  cstor->c_string = mu_strdup (mbname);
  return 0;
}
