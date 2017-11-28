/* This file is part of GNU Mailutils testsuite.
   Copyright (C) 2017 Free Software Foundation, Inc.

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

/*

NAME

   cwdrepl - replace occurrences of CWD with .

SYNOPSIS

   COMMAND | cwdrepl [DIR REPL]...

DESCRIPTION

   Some testcases operate programs that produce full file names as part
   of their output.  To make this output independent of the actual file
   location, this tool replaces every occurrence of the current working
   directory with dot.  Both logical (as given by the PWD environment
   variable) and physical (as returned by getcwd(3)) locations are replaced.

   The same effect could have been achieved by using "pwd -P", "pwd -L"
   and sed, but this would pose portability problems.

   Additionally, any number of DIR REPL pairs can be supplied in the command
   line.  Each pair instructs the tool to replace every occurrence of DIR
   with REPL on output.  Note that these pairs take precedence over the
   default ones, so running "cwdrepl $PWD 'PWD'" will replace occurrences
   of the logical current working directory name with the string PWS, instead
   of the default dot.

*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <mailutils/mailutils.h>

struct dirtrans
{
  char *dir;
  size_t dirlen;
  char const *trans;
  ssize_t translen;
};

mu_list_t translist;

static int
transcmp (const void *a, const void *b)
{
  struct dirtrans const *trans1 = a;
  struct dirtrans const *trans2 = b;
  return strcmp (trans1->dir, trans2->dir);
}

static void
newdir (char const *dir, char const *trans)
{
  if (dir)
    {
      size_t dirlen = strlen (dir);
      size_t translen = strlen (trans);
      struct dirtrans *dt = mu_alloc (sizeof *dt);
      
      while (dirlen > 0 && dir[dirlen-1] == '/')
	dirlen--;

      dt->dir = mu_alloc (dirlen + 1);
      memcpy (dt->dir, dir, dirlen);
      dt->dir[dirlen] = 0;
      dt->dirlen = dirlen;
      dt->trans = trans;
      dt->translen = translen;

      if (!translist)
	{
	  MU_ASSERT (mu_list_create (&translist));
	  mu_list_set_comparator (translist, transcmp);
	}
      else if (mu_list_locate (translist, dt, NULL) == 0)
	{
	  free (dt->dir);
	  free (dt);
	  return;
	}

      MU_ASSERT (mu_list_append (translist, dt));
    }
}

static inline int
isbnd (int c)
{
  return mu_c_is_class (c, MU_CTYPE_CNTRL|MU_CTYPE_PUNCT|MU_CTYPE_SPACE);
}

int
main (int argc, char **argv)
{
  int i;
  int rc;
  char *buf = NULL;
  size_t size, n;
  mu_iterator_t itr;
  
  mu_set_program_name (argv[0]);
  mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);

  for (i = 1; i < argc; i += 2)
    newdir (argv[i], (i + 1 < argc) ? argv[i + 1] : "");

  newdir (getenv ("PWD"), ".");
  newdir (mu_getcwd (), ".");

  MU_ASSERT (mu_list_get_iterator (translist, &itr));
  while ((rc = mu_stream_getline (mu_strin, &buf, &size, &n)) == 0 && n > 0)
    {
      n = mu_rtrim_class (buf, MU_CTYPE_SPACE);
      for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
	   mu_iterator_next (itr))
	{
	  struct dirtrans *dt;
	  size_t start = 0;
	  char *p;

	  mu_iterator_current (itr, (void**) &dt);
	  while ((p = strstr (buf + start, dt->dir)))
	    {
	      if (isbnd (p[dt->dirlen]))
		{
		  size_t off = p - buf;
		  size_t rest = n - start;
		  ssize_t d = (ssize_t)dt->translen - dt->dirlen;
		  
		  if (d > 0)
		    {
		      if (n + d + 1 > size)
			{
			  size = n + d + 1;
			  buf = mu_realloc (buf, size);
			  p = buf + off;
			}
		    }
		  
		  memmove (p + dt->translen, p + dt->dirlen,
			   rest - dt->dirlen + 1);
		  memcpy (p, dt->trans, dt->translen);
		  
		  n += d;
		  start = off + dt->translen;
		}
	      else
		start++;
	    }
	}
      mu_stream_write (mu_strout, buf, n, NULL);
      mu_stream_write (mu_strout, "\n", 1, NULL);
    }
  return 0;
}
