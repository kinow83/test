/* This file is part of the GNU Mailutils testsuite.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

typedef int (*ACTION) (FILE *, char *);

int
seek_file (FILE *fp, char *arg)
{
  long off;
  int whence;

  switch (arg[0])
    {
    case '+':
      whence = SEEK_CUR;
      arg++;
      break;
      
    case '-':
      whence = SEEK_CUR;
      break;

    case '$':
      whence = SEEK_END;
      arg++;
      break;

    default:
      whence = SEEK_SET;
    }
  
  if (sscanf (arg, "%ld", &off) != 1)
    {
      fprintf (stderr, "bad offset: %s\n", arg);
      abort ();
    }
  return fseek (fp, off, whence);
}

int
write_string (FILE *fp, char *arg)
{
  size_t n = strlen (arg);
  return fwrite (arg, n, 1, fp) != 1;
}

int
write_byte (FILE *fp, char *arg)
{
  int c;
  if (strlen (arg) == 3
      && (arg[0] == '\'' || arg[0] == '"'))
    c = arg[1];
  else
    {
      char *p;
      unsigned long n = strtoul (arg, &p, 0);
      if (*p || n > UCHAR_MAX)
	{
	  errno = EINVAL;
	  return -1;
	}
      c = n;
    }
  return fwrite (&c, 1, 1, fp) != 1;
}

int
write_short (FILE *fp, char *arg)
{
  uint16_t val;
  char *p;
  unsigned long n = strtoul (arg, &p, 0);
  if (*p || n > UINT16_MAX)
    {
      errno = EINVAL;
      return -1;
    }
  val = n;
  return fwrite (&val, sizeof(val), 1, fp) != 1;
}

int
write_int (FILE *fp, char *arg)
{
  uint32_t val;
  char *p;
  unsigned long n = strtoul (arg, &p, 0);
  if (*p || n > UINT32_MAX)
    {
      errno = EINVAL;
      return -1;
    }
  val = n;
  return fwrite (&val, sizeof(val), 1, fp) != 1;
}

static struct dispatch {
  char *opt;
  int (*act) (FILE *, char *);
} dispatch[] = {
  { "-seek",   seek_file },
  { "-string", write_string },
  { "-byte",   write_byte },
  { "-short",  write_short },
  { "-int",    write_int },
  { NULL }
};

static ACTION
find_action (char const *opt)
{
  struct dispatch *p;

  for (p = dispatch; p->opt; p++)
    if (strcmp (p->opt, opt) == 0)
      return p->act;
  return NULL;
}
  
/*
  -seek OFF 
  -string STRING
  -byte BYTE
  -short SHORT
  -int INT
  -repeat N  
 */
int
main (int argc, char **argv)
{
  char *name;
  FILE *fp;
  ACTION action;
  char **pp;
  
  assert (argc > 1);

  name = argv[1];
  fp = fopen (name, "w");
  if (!fp)
    {
      perror (name);
      abort ();
    }

  pp = argv + 2;
  while (*pp)
    {
      char *opt = *pp++, *arg;

      if (opt[0] != '-')
	{
	  fprintf (stderr, "not an option: %s\n", opt);
	  abort ();
	}
      
      action = find_action (opt);
      if (!action)
	{
	  fprintf (stderr, "unknown action: %s\n", opt);
	  abort ();
	}
      
      if (!*pp)
	{
	  fprintf (stderr, "argument to %s missing\n", opt);
	  abort ();
	}
      arg = *pp++;
      if (action (fp, arg))
	{
	  fprintf (stderr, "%s %s: %s\n", opt, arg, strerror (errno));
	  abort ();
	}
    }
  fclose (fp);
  return 0;
}
