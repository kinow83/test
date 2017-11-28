/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2005, 2007, 2009-2012, 2014-2017 Free Software
   Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <mailutils/assoc.h>
#include <mailutils/header.h>
#include <mailutils/message.h>
#include <mailutils/mime.h>
#include <mailutils/iterator.h>
#include <mailutils/stream.h>
#include <mailutils/stdstream.h>
#include <mailutils/util.h>
#include <mailutils/cstr.h>
#include <mailutils/cctype.h>
#include <mailutils/error.h>
#include <mailutils/errno.h>

static int
sort_names (char const *aname, void const *adata,
	    char const *bname, void const *bdata, void *data)
{
  return mu_c_strcasecmp (aname, bname);
}

static int
print_param (const char *name, void *item, void *data)
{
  struct mu_mime_param *param = item;
  
  mu_printf ("%s", name);
  if (param->lang)
    mu_printf ("(lang:%s/%s)", param->lang, param->cset);
  mu_printf ("=%s\n", param->value);
  return 0;
}

int
main (int argc, char **argv)
{
  int i;
  mu_stream_t tmp;
  mu_transport_t trans[2];
  char *value;
  mu_assoc_t assoc;
  char *charset = NULL;
  char *header_name = NULL;
  unsigned long width = 76;
  
  mu_set_program_name (argv[0]);
  for (i = 1; i < argc; i++)
    {
      char *opt = argv[i];

      if (strncmp (opt, "-debug=", 7) == 0)
	mu_debug_parse_spec (opt + 7);
      else if (strncmp (opt, "-charset=", 9) == 0)
	charset = opt + 9;
      else if (strcmp (opt, "-h") == 0 || strcmp (opt, "-help") == 0)
	{
	  mu_printf ("usage: %s [-charset=cs] [-debug=SPEC] [-header=NAME] [-width=N]", mu_program_name);
	  return 0;
	}
      else if (strncmp (opt, "-header=", 8) == 0)
	header_name = opt + 8;
      else if (strncmp (opt, "-width=", 7) == 0)
	width = strtoul (opt + 7, NULL, 10);
      else
	{
	  mu_error ("unknown option %s", opt);
	  return 1;
	}
    }

  if (i != argc)
    {
      mu_error ("too many arguments");
      return 1;
    }
	    
  MU_ASSERT (mu_memory_stream_create (&tmp, MU_STREAM_RDWR));
  MU_ASSERT (mu_stream_copy (tmp, mu_strin, 0, NULL));
  MU_ASSERT (mu_stream_write (tmp, "", 1, NULL));
  MU_ASSERT (mu_stream_ioctl (tmp, MU_IOCTL_TRANSPORT, MU_IOCTL_OP_GET,
			      trans));
      
  MU_ASSERT (mu_mime_header_parse ((char*)trans[0], charset, &value, &assoc));

  if (header_name)
    {
      mu_header_t hdr;
      mu_stream_t hstr;
      
      MU_ASSERT (mu_header_create (&hdr, NULL, 0));
      MU_ASSERT (mu_mime_header_set_w (hdr, header_name, value, assoc, width));
      MU_ASSERT (mu_header_get_streamref (hdr, &hstr));
      MU_ASSERT (mu_stream_copy (mu_strout, hstr, 0, NULL));
    }
  else
    {
      mu_printf ("%s\n", value);
      mu_assoc_sort_r (assoc, sort_names, NULL);
      mu_assoc_foreach (assoc, print_param, NULL);
    }
  
  return 0;
}
  
