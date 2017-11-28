/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999-2003, 2005-2007, 2009-2012, 2014-2017 Free
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

#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>
#include <mailutils/io.h>
#include <mailutils/nls.h>
#include <mailutils/assoc.h>
#include <mailutils/cstr.h>
#include <mailutils/cctype.h>
#include <mailutils/wordsplit.h>

static int
exp_getvar (char **ret, const char *vptr, size_t vlen, void *data)
{
  int rc;
  char *varname, *s = NULL;
  mu_assoc_t assoc = data;
  
  if (mu_assoc_is_empty (assoc))
    return MU_WRDSE_UNDEF;

  varname = malloc (vlen + 1);
  if (!varname)
    return MU_WRDSE_NOSPACE;
  memcpy (varname, vptr, vlen);
  varname[vlen] = 0;

  rc = mu_assoc_lookup (assoc, varname, &s);
  free (varname);

  switch (rc)
    {
    case 0:
      rc = MU_WRDSE_OK;
      break;
      
    case MU_ERR_NOENT:
      rc = MU_WRDSE_UNDEF;
      break;

    case MU_ERR_BUFSPACE:
    case ENOMEM:
      rc = MU_WRDSE_NOSPACE;
      break;

    default:
      s = (char*) mu_strerror (rc);
      rc = MU_WRDSE_USERERR;
    }

  if (s)
    {
      s = strdup (s);
      if (!s)
	return MU_WRDSE_NOSPACE;
      *ret = s;
    }
  else
    rc = MU_WRDSE_UNDEF;
  return rc;
}

static int
exp_localpart (int argc, char **argv, char **result)
{
  size_t len = strcspn (argv[1], "@");
  char *s;

  s = malloc (len + 1);
  if (!s)
    return MU_WRDSE_NOSPACE;
  memcpy (s, argv[1], len);
  s[len] = 0;

  *result = s;
  return MU_WRDSE_OK;
}

static int
exp_domainpart (int argc, char **argv, char **result)
{
  char *s = strchr (argv[1], '@');

  if (s)
    s++;
  else
    s = "";

  s = strdup (s);
  if (!s)
    return MU_WRDSE_NOSPACE;
  
  *result = s;
  
  return MU_WRDSE_OK;
}

static int
exp_shell (char **ret, char const *str, size_t len, void *closure)
{
  FILE *fp;
  char *cmd;
  int c, lastc;
  char *buffer = NULL;
  size_t bufsize = 0;
  size_t buflen = 0;
  
  cmd = malloc (len + 1);
  if (!cmd)
    return MU_WRDSE_NOSPACE;
  memcpy (cmd, str, len);
  cmd[len] = 0;

  fp = popen (cmd, "r");
  if (!fp)
    {
      ret = NULL;
      if (mu_asprintf (ret, "can't run %s: %s", cmd, mu_strerror (errno)))
	return MU_WRDSE_NOSPACE;
      else
	return MU_WRDSE_USERERR;
    }

  while ((c = fgetc (fp)) != EOF)
    {
      lastc = c;
      if (c == '\n')
	c = ' ';
      if (buflen == bufsize)
	{
	  char *p;
	  
	  if (bufsize == 0)
	    bufsize = 80;
	  else
	    bufsize *= 2;
	  p = realloc (buffer, bufsize);
	  if (!p)
	    {
	      free (buffer);
	      free (cmd);
	      return MU_WRDSE_NOSPACE;
	    }
	  buffer = p;
	}
      buffer[buflen++] = c;
    }

  if (buffer)
    {
      if (lastc == '\n')
	--buflen;
      buffer[buflen] = 0;
    }

  pclose (fp);
  free (cmd);

  *ret = buffer;
  return MU_WRDSE_OK;  
}

struct exp_command
{
  char *name;
  int minarg;
  int maxarg;
  int (*exp) (int argc, char **argv, char **ret);
};

static struct exp_command exp_command_tab[] = {
  { "localpart", 2, 2, exp_localpart },
  { "domainpart", 2, 2, exp_domainpart },
  { NULL }
};

static struct exp_command *
findcom (char const *name)
{
  struct exp_command *cp;

  for (cp = exp_command_tab; cp->name; cp++)
    if (strcmp (name, cp->name) == 0)
      return cp;

  return NULL;
}

static int
checkargc (struct exp_command *cmd, int argc)
{
  if (cmd->minarg && argc < cmd->minarg)
    return 1;
  else if (cmd->maxarg && argc > cmd->maxarg)
    return 1;
  return 0;
}

#define SHELL_CMD "shell"

static int
exp_runcmd (char **ret, const char *str, size_t len, char **argv, void *closure)
{
  int argc;
  struct exp_command *cmd;
  char *result = NULL;
  int rc;

  if (strcmp (argv[0], SHELL_CMD) == 0)
    {
      len -= sizeof SHELL_CMD - 1;
      str += sizeof SHELL_CMD - 1;
      while (len > 0 && mu_isspace (*str))
	{
	  len--;
	  str++;
	}
      
      if (len == 0)
	{
	  if (mu_asprintf (ret, _("%s: bad number of arguments"), argv[0]))
	    return MU_WRDSE_NOSPACE;
	  return MU_WRDSE_USERERR;
	}
      return exp_shell (ret, str, len, closure);
    }
  
  cmd = findcom (argv[0]);
  if (!cmd)
    {
      if (mu_asprintf (ret, _("%s: unknown function"), argv[0]))
	return MU_WRDSE_NOSPACE;
      return MU_WRDSE_USERERR;
    }
  
  for (argc = 0; argv[argc]; argc++)
    ;

  if (checkargc (cmd, argc))
    {
      if (mu_asprintf (ret, _("%s: bad number of arguments"), argv[0]))
	return MU_WRDSE_NOSPACE;
      return MU_WRDSE_USERERR;
    }

  rc = cmd->exp (argc, argv, &result);
  if (rc == MU_WRDSE_USERERR && result == NULL)
    {
      if (mu_asprintf (ret, _("%s: command expansion error"), argv[0]))
	return MU_WRDSE_NOSPACE;
      return MU_WRDSE_USERERR;
    }

  if (rc == MU_WRDSE_OK || rc == MU_WRDSE_USERERR)
    *ret = result;
  
  return rc;
}
  
int
mu_str_expand (char **output, char const *input, mu_assoc_t assoc)
{
  struct mu_wordsplit ws;
  int rc = 0;
  
  ws.ws_getvar = exp_getvar;
  ws.ws_command = exp_runcmd;
  ws.ws_closure = assoc;
  ws.ws_options = MU_WRDSO_ARGV;

  if (mu_wordsplit (input, &ws,
		    MU_WRDSF_NOSPLIT | MU_WRDSF_GETVAR | MU_WRDSF_CLOSURE
		    | MU_WRDSF_OPTIONS))
    {
      if (ws.ws_errno == MU_WRDSE_NOSPACE)
	rc = ENOMEM;
      else
	{
	  char *p = strdup (mu_wordsplit_strerror (&ws));
	  if (!p)
	    rc = ENOMEM;
	  else
	    {
	      *output = p;
	      rc = MU_ERR_FAILURE;
	    }
	}
    }
  else if (ws.ws_wordc == 0)
    {
      *output = strdup ("");
      if (!*output)
	rc = ENOMEM;
    }
  else
    {
      size_t wordc;
      char **wordv;
      
      mu_wordsplit_get_words (&ws, &wordc, &wordv);
      *output = wordv[0];
    }
  mu_wordsplit_free (&ws);
  return rc;
}

int
mu_str_vexpand (char **output, char const *input, ...)
{
  int rc;
  mu_assoc_t assoc;
  char *p[2];
  int i;
  va_list ap;
  
  rc = mu_assoc_create (&assoc, 0);
  if (rc)
    return rc;

  va_start (ap, input);
  i = 0;
  while ((p[i] = va_arg (ap, char *)) != NULL)
    {
      if (i == 1)
	{
	  rc = mu_assoc_install (assoc, p[0], p[1]);
	  if (rc)
	    {
	      mu_assoc_destroy (&assoc);
	      return rc;
	    }
	}
      i = (i + 1) % 2;
    }
  va_end (ap);

  rc = mu_str_expand (output, input, assoc);
  mu_assoc_destroy (&assoc);
  return rc;
}
