/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999, 2001-2012, 2014-2017 Free Software Foundation,
   Inc.

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

#include "mail.h"
#include <mailutils/mime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int isfilename (const char *);
static int msg_to_pipe (const char *cmd, mu_message_t msg);

int multipart_alternative;


/* Additional message headers */
struct add_header
{
  int mode;
  char *name;
  char *value;
};

static mu_list_t add_header_list;

static int
seed_headers (void *item, void *data)
{
  struct add_header *hp = item;
  compose_env_t *env = data;

  compose_header_set (env, hp->name, hp->value, hp->mode);
  return 0;
}

static int
list_headers (void *item, void *data)
{
  struct add_header *hp = item;
  char *name = data;

  if (!name || strcmp (name, hp->name) == 0)
    {
      mu_printf ("%s: %s\n", hp->name, hp->value);
    }
  return 0;
}
    
static void
add_header (char *name, char *value, int mode)
{
  struct add_header *hp;
  
  if (!add_header_list)
    {
      int rc = mu_list_create (&add_header_list);
      if (rc)
	{
	  mu_error (_("Cannot create header list: %s"), mu_strerror (rc));
	  exit (1);
	}
    }

  hp = mu_alloc (sizeof (*hp));
  hp->mode = mode;
  hp->name = name;
  hp->value = value;
  mu_list_append (add_header_list, hp);
}

void
send_append_header (char const *text)
{
  char *p;
  size_t len;
  char *name;

  p = strchr (text, ':');
  if (!p)
    {
      mu_error (_("Invalid header: %s"), text);
      return;
    }
  len = p - text;
  name = mu_alloc (len + 1);
  memcpy (name, text, len);
  name[len] = 0;
  for (p++; *p && mu_isspace (*p); p++)
    ;

  add_header (name, mu_strdup (p), COMPOSE_APPEND);
}

void
send_append_header2 (char const *name, char const *value, int mode)
{
  add_header (mu_strdup (name), mu_strdup (value), mode);
}

int
mail_sendheader (int argc, char **argv)
{
  if (argc == 1)
    mu_list_foreach (add_header_list, list_headers, NULL);
  else if (argc == 2)
    {
      if (strchr (argv[1], ':'))
	send_append_header (argv[1]);
      else
	mu_list_foreach (add_header_list, list_headers, argv[1]);
    }
  else
    {
      size_t len = strlen (argv[1]);
      if (len > 0 && argv[1][len - 1] == ':') 
	argv[1][len - 1] = 0;
      add_header (mu_strdup (argv[1]), mu_strdup (argv[2]), COMPOSE_APPEND);
    }
  return 0;
}

/* Attachments */
struct atchinfo
{
  char *id;
  char *encoding;
  char *content_type;
  char *name;
  char *filename;
  mu_stream_t source;
  int skip_empty;
};

static void
atchinfo_free (void *p)
{
  struct atchinfo *ap = p;
  free (ap->id);
  free (ap->encoding);
  free (ap->content_type);
  free (ap->name);
  free (ap->filename);
  mu_stream_destroy (&ap->source);
  free (ap);
}

static mu_list_t
attlist_new (void)
{
  mu_list_t lst;
  int rc = mu_list_create (&lst);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_list_create", NULL, rc);
      exit (1);
    }
  mu_list_set_destroy_item (lst, atchinfo_free);
  return lst;
}

static void
attach_set_content_type (struct atchinfo *aptr, char const *content_type)
{
  char *charset;

  if (!content_type)
    content_type = "text/plain";
  if (strncmp (content_type, "text/", 5) == 0
      && !strstr (content_type, "charset=")
      && (charset = util_get_charset ()))
    {
      mu_asprintf (&aptr->content_type, "%s; charset=%s",
		   content_type, charset);
      free (charset);
    }
  else
    aptr->content_type = mu_strdup (content_type);
}

static void
attlist_add (mu_list_t attlist, char *id, char const *encoding,
	     char const *content_type, char const *content_name,
	     char const *content_filename,
	     mu_stream_t stream, int skip_empty)
{
  struct atchinfo *aptr;
  int rc;
  
  aptr = mu_alloc (sizeof (*aptr));

  aptr->id = id ? mu_strdup (id) : id;
  aptr->encoding = mu_strdup (encoding);
  attach_set_content_type (aptr,
			   content_type
			     ? content_type : "application/octet-stream");
  aptr->name = content_name ? mu_strdup (content_name) : NULL;
  aptr->filename = content_filename ? mu_strdup (content_filename) : NULL;
  aptr->source = stream;
  if (stream)
    mu_stream_ref (stream);
  aptr->skip_empty = skip_empty;
  rc = mu_list_append (attlist, aptr);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_list_append", NULL, rc);
      exit (1);
    }
}

int
attlist_attach_file (mu_list_t *attlist_ptr,
		     int fd,
		     const char *realname,
		     const char *content_filename, const char *content_name,
		     const char *content_type, const char *encoding)
{
  int rc;
  struct stat st;
  mu_list_t list;
  mu_stream_t stream = NULL;
  char *id = NULL;
  mu_list_t attlist;
  
  if (fd >= 0)
    {
      rc = mu_fd_stream_create (&stream, NULL, fd, MU_STREAM_READ);
      if (rc)
	{
	  mu_error (_("can't open descriptor %d: %s"), fd, mu_strerror (rc));
	  return 1;
	}
      mu_asprintf (&id, "fd %d", fd);
      if (fd == 0)
	{
	  mu_stream_destroy (&mu_strin);
	  mu_nullstream_create (&mu_strin, MU_STREAM_READ);
	  mu_stream_ioctl (mu_strin, MU_IOCTL_NULLSTREAM,
			   MU_IOCTL_NULLSTREAM_SET_PATTERN, NULL);
	  util_do_command ("set nullbody nullbodymsg");
	}
    }
  else if (realname)
    {
      if (!content_filename)
	content_filename = realname;
      if (stat (realname, &st))
	{
	  if (errno == ENOENT)
	    {
	      mu_error (_("%s: file does not exist"), realname);
	      return 1;
	    }
	  else
	    {
	      mu_error (_("%s: cannot stat: %s"), realname,
			mu_strerror (errno));
	      return 1;
	    }
	}
      
      if (!S_ISREG (st.st_mode))
	{
	  mu_error (_("%s: not a regular file"), realname);
	  return 1;
	}

      rc = mu_mapfile_stream_create (&stream, realname, MU_STREAM_READ);
      if (rc)
	{
	  mu_error (_("can't open file %s: %s"),
		    realname, mu_strerror (rc));
	  return 1;
	}
      mu_asprintf (&id, "\"%s\"", realname);
    }
  else
    abort ();
  
  if (!encoding)
    encoding = "base64";
  mu_filter_get_list (&list);
  rc = mu_list_locate (list, (void*) encoding, NULL);
  if (rc)
    {
      mu_error (_("unsupported encoding: %s"), encoding);
      free (id);
      mu_stream_destroy (&stream);
      return 1;
    }
  
  if (!*attlist_ptr)
    {
      attlist = attlist_new ();
      *attlist_ptr = attlist;
    }
  else
    attlist = *attlist_ptr;
    
  attlist_add (attlist, id, encoding, content_type,
	       content_name, content_filename,
	       stream, skip_empty_attachments);
  if (stream)
    mu_stream_unref (stream);
  free (id);

  return 0;
}

static int
attlist_helper (void *item, void *data)
{
  struct atchinfo *aptr = item;
  mu_list_t list = data;
  attlist_add (list, aptr->id, aptr->encoding, aptr->content_type,
	       aptr->name, aptr->filename, aptr->source, aptr->skip_empty);
  return 0;
}

static mu_list_t
attlist_copy (mu_list_t src)
{
  mu_list_t dst;

  if (!src)
    return NULL;
  dst = attlist_new ();
  mu_list_foreach (src, attlist_helper, dst);
  return dst;
}

static mu_list_t attachment_list;

int
send_attach_file (int fd,
		  const char *realname,
		  const char *content_filename, const char *content_name,
		  const char *content_type, const char *encoding)
{
  return attlist_attach_file (&attachment_list,
			      fd,
			      realname,
			      content_filename,
			      content_name,
			      content_type,
			      encoding);
}

static void
report_multipart_type (compose_env_t *env)
{
  mu_printf ("multipart/%s\n", env->alt ? "alternative" : "mixed");
}

/* ~/ - toggle between multipart/mixed and multipart/alternative */
int
escape_toggle_multipart_type (int argc, char **argv, compose_env_t *env)
{
  env->alt = !env->alt;
  report_multipart_type (env);
  return 0;
}

int
escape_list_attachments (int argc, char **argv, compose_env_t *env)
{
  mu_iterator_t itr;
  int i;
  
  report_multipart_type (env);

  if (mu_list_is_empty (env->attlist) ||
      mu_list_get_iterator (env->attlist, &itr))
    {
      mu_printf ("%s\n", _("No attachments"));
      return 0;
    }
  
  for (mu_iterator_first (itr), i = 1; !mu_iterator_is_done (itr);
       mu_iterator_next (itr), i++)
    {
      struct atchinfo *aptr;
      if (mu_iterator_current (itr, (void**)&aptr))
	continue;
	  
      mu_printf ("%3d %-12s %-30s %-s\n",
		 i, aptr->id, aptr->content_type, aptr->encoding);
    }
  mu_iterator_destroy (&itr);

  return 0;
}

int
escape_attach (int argc, char **argv, compose_env_t *env)
{
  const char *encoding = default_encoding;
  const char *content_type = default_content_type;
  
  switch (argc)
    {
    case 4:
      encoding = argv[3];
    case 3:
      content_type = argv[2];
    case 2:
      return attlist_attach_file (&env->attlist,
				  -1, argv[1], argv[1], argv[1],
				  content_type, encoding);
    default:
      return escape_check_args (argc, argv, 2, 4);
    }
  return 1;
}

int
escape_remove_attachment (int argc, char **argv, compose_env_t *env)
{
  size_t count;
  unsigned long n;
  char *p;
  
  if (escape_check_args (argc, argv, 2, 2))
    return 1;
  n = strtoul (argv[1], &p, 10);
  if (*p)
    {
      mu_error (_("not a valid number: %s"), argv[1]);
      return 1;
    }
  
  mu_list_count (env->attlist, &count);
  if (n == 0 || n > count)
    {
      mu_error (_("index out of range"));
      return 1;
    }

  return mu_list_remove_nth (env->attlist, n - 1);
}

static int
saveatt (void *item, void *data)
{
  struct atchinfo *aptr = item;
  compose_env_t *env = data;
  mu_message_t part;
  mu_header_t hdr;
  int rc;
  size_t nparts;
  char *p;

  rc = mu_attachment_create (&part, aptr->content_type, aptr->encoding,
			     aptr->name,
			     aptr->filename);
  if (rc)
    {
      mu_error (_("can't create attachment %s: %s"),
		aptr->id, mu_strerror (rc));
      return 1;
    }

  rc = mu_attachment_copy_from_stream (part, aptr->source);
  if (rc)
    {
      mu_error (_("cannot attach %s: %s"), aptr->id, mu_strerror (rc));
      return 1;
    }

  if (aptr->skip_empty)
    {
      mu_body_t body;
      size_t size;
      
      rc = mu_message_get_body (part, &body);
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_message_get_body", aptr->id, rc);
	  return 1;
	}
      rc = mu_body_size (body, &size);
      if (rc)
	{
	  mu_diag_funcall (MU_DIAG_ERROR, "mu_body_size", aptr->id, rc);
	  return 1;
	}
      if (size == 0)
	return 0;
    }
      
  mu_mime_get_num_parts	(env->mime, &nparts);
  mu_message_get_header (part, &hdr);
  mu_rfc2822_msg_id (nparts, &p);
  mu_header_set_value (hdr, MU_HEADER_CONTENT_ID, p, 1);
  free (p);

  rc = mu_mime_add_part (env->mime, part);
  mu_message_unref (part);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mime_add_part", aptr->filename, rc);
      return 1;
    }

  return 0;
}

static int
add_body (mu_message_t inmsg, compose_env_t *env)
{
  int rc;
  mu_body_t body;
  mu_stream_t str;
  struct atchinfo *aptr;
  
  mu_message_get_body (inmsg, &body);
  mu_body_get_streamref (body, &str);

  aptr = mu_alloc (sizeof (*aptr));
  aptr->id = NULL;
  aptr->encoding = default_encoding ? mu_strdup (default_encoding) : NULL;
  attach_set_content_type (aptr, default_content_type);
  aptr->name = NULL;
  aptr->filename = NULL;
  aptr->source = str;
  if (str)
    mu_stream_ref (str);
  aptr->skip_empty = skip_empty_attachments || multipart_alternative;
  if (!env->attlist)
    env->attlist = attlist_new ();
  rc = mu_list_prepend (env->attlist, aptr);
  if (rc)
    mu_diag_funcall (MU_DIAG_ERROR, "mu_list_prepend", NULL, rc);
  return rc;
}
  
static int
add_attachments (compose_env_t *env, mu_message_t *pmsg)
{
  mu_message_t inmsg, outmsg;
  mu_header_t inhdr, outhdr;
  mu_iterator_t itr;
  int rc;
  
  inmsg = *pmsg;

  if (mailvar_is_true ("mime") && add_body (inmsg, env))
    return 1;
  
  if (mu_list_is_empty (env->attlist))
    return 0;
  
  /* Create a mime object */
  rc = mu_mime_create (&env->mime, NULL,
		       env->alt ?
		         MU_MIME_MULTIPART_ALT : MU_MIME_MULTIPART_MIXED);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mime_create", NULL, rc);
      return 1;
    }

  mu_message_get_header (inmsg, &inhdr);

  /* Add the respective attachments */
  rc = mu_list_foreach (env->attlist, saveatt, env);
  if (rc)
    return 1;

  /* Get the resulting message */
  rc = mu_mime_get_message (env->mime, &outmsg);

  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_mime_get_message", NULL, rc);
      return 1;
    }

  /* Copy rest of headers from the original message */
  rc = mu_message_get_header (outmsg, &outhdr);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_message_get_header", NULL, rc);
      return 1;
    }

  rc = mu_header_get_iterator (inhdr, &itr);
  if (rc)
    {
      mu_diag_funcall (MU_DIAG_ERROR, "mu_header_get_iterator", NULL, rc);
      return 1;
    }
  for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
       mu_iterator_next (itr))
    {
      const char *name, *value;
      
      if (mu_iterator_current_kv (itr, (const void **)&name,
				  (void**)&value) == 0)
	{
	  if (mu_c_strcasecmp (name, MU_HEADER_RECEIVED) == 0
	      || mu_c_strncasecmp (name, "X-", 2) == 0)
	    mu_header_append (outhdr, name, value);
	  else if (mu_c_strcasecmp (name, MU_HEADER_MIME_VERSION) == 0 ||
		   mu_c_strncasecmp (name, "Content-", 8) == 0)
	    {
	      mu_error (_("%s: not setting header"), name);
	      continue;
	    }
	  else
	    mu_header_set_value (outhdr, name, value, 1);
	}
    }
  mu_iterator_destroy (&itr);

  mu_message_unref (outmsg);
  mu_message_unref (inmsg);

  *pmsg = outmsg;
  return 0;
}



/* Send-related commands */

static void
read_cc_bcc (compose_env_t *env)
{
  if (mailvar_is_true ("askcc"))
    compose_header_set (env, MU_HEADER_CC,
			ml_readline_with_intr ("Cc: "), COMPOSE_REPLACE);
  if (mailvar_is_true ("askbcc"))
    compose_header_set (env, MU_HEADER_BCC,
			ml_readline_with_intr ("Bcc: "), COMPOSE_REPLACE);
}

/*
 * m[ail] address...
 if address is starting with
 
    '/'        it is considered a file and the message is saveed to a file;
    '.'        it is considered to be a relative path;
    '|'        it is considered to be a pipe, and the message is written to
               there;
	       
 example:
 
   mail joe '| cat >/tmp/save'

 mail will be sent to joe and the message saved in /tmp/save. */

int
mail_send (int argc, char **argv)
{
  compose_env_t env;
  int status;
  int save_to = mu_isupper (argv[0][0]);
  compose_init (&env);

  if (argc < 2)
    {
      if (interactive)
	compose_header_set (&env, MU_HEADER_TO, ml_readline_with_intr ("To: "),
			    COMPOSE_REPLACE);
      else if (mailvar_is_true ("editheaders"))
	{
	  if (parse_headers (mu_strin, &env) != parse_headers_ok)
	    {
	      mu_error ("%s", _("Errors parsing message"));
	      exit (EXIT_FAILURE);
	    }
	  if (add_header_list)
	    {
	      mu_iterator_t itr;
	      mu_list_get_iterator (add_header_list, &itr);
	      for (mu_iterator_first (itr); !mu_iterator_is_done (itr);
		   mu_iterator_next (itr))
		{
		  struct add_header *hp;
		  int mode;
		  if (mu_iterator_current (itr, (void**) &hp))
		    break; 
		  mode = hp->mode;
		  if (mu_header_sget_value (env.header, hp->name, NULL) == 0)
		    mode = COMPOSE_REPLACE;
		  compose_header_set (&env, hp->name, hp->value, mode);
		}
	      mu_iterator_destroy (&itr);
	    }
	}
      else
	{
	  mu_error ("%s", _("No recipients specified"));
	  exit (EXIT_FAILURE);
	}
    }
  else
    {
      while (--argc)
	{
	  char *p = *++argv;
	  if (isfilename (p))
	    {
	      env.outfiles = realloc (env.outfiles,
				      (env.nfiles + 1) *
				      sizeof (*(env.outfiles)));
	      if (env.outfiles)
		{
		  env.outfiles[env.nfiles] = p;
		  env.nfiles++;
		}
	    }
	  else
	    compose_header_set (&env, MU_HEADER_TO, p, COMPOSE_SINGLE_LINE);
	}
    }

  if (interactive)
    {
      if (!mailvar_is_true ("mailx"))
	read_cc_bcc (&env);

      if (mailvar_is_true ("asksub"))
	compose_header_set (&env, MU_HEADER_SUBJECT,
			    ml_readline_with_intr ("Subject: "),
			    COMPOSE_REPLACE);
    }
  
  status = mail_send0 (&env, save_to);
  compose_destroy (&env);
  return status;
}

int
parse_headers (mu_stream_t input, compose_env_t *env)
{
  int status;
  mu_header_t header;
  char *name = NULL;
  char *value = NULL;
  enum { STATE_INIT, STATE_READ, STATE_BODY } state = STATE_INIT;
  char *buf = NULL;
  size_t size = 0, n;
  int errcnt = 0, line = 0;
  
  if ((status = mu_header_create (&header, NULL, 0)) != 0)
    {
      mu_error (_("Cannot create header: %s"), mu_strerror (status));
      return parse_headers_fatal;
    }
  
  while (state != STATE_BODY &&
	 errcnt == 0 &&
	 mu_stream_getline (input, &buf, &size, &n) == 0 && n > 0)
    {
      mu_rtrim_class (buf, MU_CTYPE_SPACE);

      line++;
      switch (state)
	{
	case STATE_INIT:
	  if (!buf[0] || mu_isspace (buf[0]))
	    continue;
	  else
	    state = STATE_READ;
	  /*FALLTHRU*/
	  
	case STATE_READ:
	  if (buf[0] == 0)
	    state = STATE_BODY;
	  else if (mu_isspace (buf[0]))
	    {
	      /* A continuation line */
	      if (name)
		{
		  char *p = NULL;
		  mu_asprintf (&p, "%s\n%s", value, buf);
		  free (value);
		  value = p;
		}
	      else
		{
		  mu_error (_("%d: not a header line"), line);
		  errcnt++;
		}
	    }
	  else
	    {
	      char *p;
	      
	      if (name)
		{
		  mu_header_append (header, name, value[0] ? value : NULL);
		  free (name);
		  free (value);
		  name = value = NULL;
		}
	      p = strchr (buf, ':');
	      if (p)
		{
		  *p++ = 0;
		  while (*p && mu_isspace (*p))
		    p++;
		  value = mu_strdup (p);
		  name = mu_strdup (buf);
		}
	      else
		{
		  mu_error (_("%d: not a header line"), line);
		  errcnt++;
		}
	    }
	  break;
	  
	default:
	  abort ();
	}
    }
  
  free (buf);
  if (name)
    {
      mu_header_append (header, name, value);
      free (name);
      free (value);
    }     

  if (errcnt)
    {
      mu_header_destroy (&header);
      return parse_headers_error;
    }

  mu_header_destroy (&env->header);
  env->header = header;
  return parse_headers_ok;
}


void
compose_init (compose_env_t *env)
{
  memset (env, 0, sizeof (*env));
  env->alt = multipart_alternative;
  env->attlist = attlist_copy (attachment_list);
  mu_list_foreach (add_header_list, seed_headers, env);
}

int
compose_header_set (compose_env_t *env, const char *name,
		    const char *value, int mode)
{
  int status;
  char *old_value;

  if (!value || value[0] == 0)
    return EINVAL;

  if (!env->header
      && (status = mu_header_create (&env->header, NULL, 0)) != 0)
    {
      mu_error (_("Cannot create header: %s"), mu_strerror (status));
      return status;
    }

  switch (mode)
    {
    case COMPOSE_REPLACE:
      if (is_address_field (name) && mailvar_is_true ("inplacealiases"))
	{
	  char *exp = alias_expand (value);
	  status = mu_header_set_value (env->header, name, exp ? exp : value, 1);
	  free (exp);
	}
      else
	status = mu_header_set_value (env->header, name, value, 1);
      break;

    case COMPOSE_APPEND:
      if (is_address_field (name) && mailvar_is_true ("inplacealiases"))
	{
	  char *exp = alias_expand (value);
	  status = mu_header_append (env->header, name, exp ? exp : value);
	  free (exp);
	}
      else
	status = mu_header_append (env->header, name, value);
      break;
      
    case COMPOSE_SINGLE_LINE:
      if (mu_header_aget_value (env->header, name, &old_value) == 0
	  && old_value[0])
	{
	  if (is_address_field (name) && mailvar_is_true ("inplacealiases"))
	    {
	      char *exp = alias_expand (value);
	      status = util_merge_addresses (&old_value, exp ? exp : value);
	      if (status == 0)
		status = mu_header_set_value (env->header, name, old_value, 1);
	      free (exp);
	    }
	  else
	    {
	      size_t size = strlen (old_value) + strlen (value) + 2;
	      char *p = realloc (old_value, size);
	      if (!p)
		status = ENOMEM;
	      else
		{
		  old_value = p;
		  strcat (old_value, ",");
		  strcat (old_value, value);
		  status = mu_header_set_value (env->header, name, old_value,
						1);
		}
	    }
	  free (old_value);
	}
      else
	status = compose_header_set (env, name, value, COMPOSE_REPLACE);
    }

  return status;
}

char *
compose_header_get (compose_env_t *env, char *name, char *defval)
{
  char *p;

  if (mu_header_aget_value (env->header, name, &p))
    p = defval;
  return p;
}

void
compose_destroy (compose_env_t *env)
{
  mu_header_destroy (&env->header);
  free (env->outfiles);
  mu_mime_destroy (&env->mime);
  mu_list_destroy (&env->attlist);
  mu_stream_destroy (&env->compstr);
}

static int
fill_body (mu_message_t msg, mu_stream_t instr)
{
  int rc;
  mu_body_t body = NULL;
  mu_stream_t stream = NULL;
  mu_off_t n;
  
  rc = mu_message_get_body (msg, &body);
  if (rc)
    {
      mu_error (_("cannot get message body: %s"), mu_strerror (rc));
      return 1;
    }
  rc = mu_body_get_streamref (body, &stream);
  if (rc)
    {
      mu_error (_("cannot get body: %s"), mu_strerror (rc));
      return 1;
    }

  rc = mu_stream_copy (stream, instr, 0, &n);
  mu_stream_destroy (&stream);

  if (rc)
    {
      mu_error (_("cannot copy temporary stream: %s"), mu_strerror (rc));
      return 1;
    }
  
  if (n == 0)
    {
      if (mailvar_is_true ("nullbody"))
	{
	  char *str;
	  if (mailvar_get (&str, "nullbodymsg", mailvar_type_string, 0) == 0)
	    mu_error ("%s", _(str));
	}
      else
	return 1;
    }

  return 0;
}

static int
save_dead_message_env (compose_env_t *env)
{
  if (mailvar_is_true ("save"))
    {
      mu_stream_t dead_letter, str;
      int rc;
      time_t t;
      struct tm *tm;
      const char *name = getenv ("DEAD");
      char *sender;
      
      /* FIXME: Use MU_STREAM_APPEND if appenddeadletter, instead of the
	 stream manipulations below */
      rc = mu_file_stream_create (&dead_letter, name,
				  MU_STREAM_CREAT|MU_STREAM_WRITE);
      if (rc)
	{
	  mu_error (_("Cannot open file %s: %s"), name, strerror (rc));
	  return 1;
	}
      if (mailvar_is_true ("appenddeadletter"))
	mu_stream_seek (dead_letter, 0, MU_SEEK_END, NULL);
      else
	mu_stream_truncate (dead_letter, 0);

      time (&t);
      tm = gmtime (&t);
      sender = mu_get_user_email (NULL);
      if (!sender)
	sender = mu_strdup ("UNKNOWN");
      mu_stream_printf (dead_letter, "From %s ", sender);
      free (sender);
      mu_c_streamftime (dead_letter, "%c%n", tm, NULL);

      if (mu_header_get_streamref (env->header, &str) == 0)
	{
	  mu_stream_copy (dead_letter, str, 0, NULL);
	  mu_stream_unref (str);
	}
      else
	mu_stream_write (dead_letter, "\n", 1, NULL);
      
      mu_stream_seek (env->compstr, 0, MU_SEEK_SET, NULL);
      mu_stream_copy (dead_letter, env->compstr, 0, NULL);
      mu_stream_write (dead_letter, "\n", 1, NULL);
      mu_stream_destroy (&dead_letter);
    }
  return 0;
}

static int
save_dead_message (mu_message_t msg)
{
  if (mailvar_is_true ("save"))
    {
      mu_stream_t dead_letter, str;
      int rc;
      time_t t;
      struct tm *tm;
      const char *name = getenv ("DEAD");
      char *sender;
      
      /* FIXME: Use MU_STREAM_APPEND if appenddeadletter, instead of the
	 stream manipulations below */
      rc = mu_file_stream_create (&dead_letter, name,
				  MU_STREAM_CREAT|MU_STREAM_WRITE);
      if (rc)
	{
	  mu_error (_("Cannot open file %s: %s"), name, strerror (rc));
	  return 1;
	}
      if (mailvar_is_true ("appenddeadletter"))
	mu_stream_seek (dead_letter, 0, MU_SEEK_END, NULL);
      else
	mu_stream_truncate (dead_letter, 0);

      time (&t);
      tm = gmtime (&t);
      sender = mu_get_user_email (NULL);
      if (!sender)
	sender = mu_strdup ("UNKNOWN");
      mu_stream_printf (dead_letter, "From %s ", sender);
      free (sender);
      mu_c_streamftime (dead_letter, "%c%n", tm, NULL);

      if (mu_message_get_streamref (msg, &str) == 0)
	{
	  mu_stream_copy (dead_letter, str, 0, NULL);
	  mu_stream_unref (str);
	}
      mu_stream_write (dead_letter, "\n", 1, NULL);
      mu_stream_destroy (&dead_letter);
    }
  return 0;
}

static int
send_message (mu_message_t msg)
{
  char *sendmail;
  int status;
  
  if (mailvar_get (&sendmail, "sendmail", mailvar_type_string, 0) == 0)
    {
      if (sendmail[0] == '/')
	status = msg_to_pipe (sendmail, msg);
      else
	{
	  mu_mailer_t mailer;
	  
	  status = mu_mailer_create (&mailer, sendmail);
	  if (status == 0)
	    {
	      const char *return_address_str;
	      mu_address_t return_address = NULL;
	      
	      if (mailvar_get (&return_address_str, "return-address",
			       mailvar_type_string, 0) == 0)
		{
		  status = mu_address_create (&return_address,
					      return_address_str);
		  if (status)
		    {
		      mu_error (_("invalid return address: %s"),
				mu_strerror (status));
		      mu_mailer_destroy (&mailer);
		      return status;
		    }
		} 

	      if (mailvar_is_true ("verbose"))
		{
		  mu_debug_set_category_level (MU_DEBCAT_MAILER,
					  MU_DEBUG_LEVEL_UPTO (MU_DEBUG_PROT));
		}
	      status = mu_mailer_open (mailer, MU_STREAM_RDWR);
	      if (status == 0)
		{
		  status = mu_mailer_send_message (mailer, msg,
						   return_address, NULL);
		  mu_mailer_close (mailer);
		}
	      else
		mu_error (_("Cannot open mailer: %s"),
			  mu_strerror (status));
	      mu_mailer_destroy (&mailer);
	      mu_address_destroy (&return_address);
	    }
	  else
	    mu_error (_("Cannot create mailer: %s"), mu_strerror (status));
	}
    }
  else
    {
      mu_error (_("Variable sendmail not set: no mailer"));
      status = ENOSYS;
    }
  return status;
}

/* mail_send0(): shared between mail_send() and mail_reply();

   If the variable "record" is set, the outgoing message is
   saved after being sent. If "save_to" argument is non-zero,
   the name of the save file is derived from "to" argument. Otherwise,
   it is taken from the value of "record" variable.

   sendmail

   contains a command, possibly with options, that mailx invokes to send
   mail. You must manually set the default for this environment variable
   by editing ROOTDIR/etc/mailx.rc to specify the mail agent of your
   choice. The default is sendmail, but it can be any command that takes
   addresses on the command line and message contents on standard input. */

int
mail_send0 (compose_env_t *env, int save_to)
{
  int done = 0;
  int rc;
  char *savefile = NULL;
  int int_cnt;
  char *escape;

  /* Prepare environment */
  rc = mu_temp_file_stream_create (&env->compstr, NULL, 0);
  if (rc)
    {
      mu_error (_("Cannot open temporary file: %s"), mu_strerror (rc));
      return 1;
    }

  ml_clear_interrupt ();
  int_cnt = 0;
  while (!done)
    {
      char *buf;
      buf = ml_readline (" \b");

      if (ml_got_interrupt ())
	{
	  if (mailvar_is_true ("ignore"))
	    {
	      mu_printf ("@\n");
	    }
	  else
	    {
	      if (buf)
		free (buf);
	      if (++int_cnt == 2)
		break;
	      mu_error (_("\n(Interrupt -- one more to kill letter)"));
	    }
	  continue;
	}

      if (!buf)
	{
	  if (interactive && mailvar_is_true ("ignoreeof"))
	    {
	      mu_error (mailvar_is_true ("dot") 
                          ?  _("Use \".\" to terminate letter.") 
                          : _("Use \"~.\" to terminate letter."));
	      continue;
	    }
	  else
	    break;
	}

      int_cnt = 0;

      if (strcmp (buf, ".") == 0 && mailvar_is_true ("dot"))
	done = 1;
      else if (mailvar_get (&escape, "escape", mailvar_type_string, 0) == 0
	       && buf[0] == escape[0])
	{
	  if (buf[1] == buf[0])
	    mu_stream_printf (env->compstr, "%s\n", buf + 1);
	  else if (buf[1] == '.')
	    done = 1;
	  else if (buf[1] == 'x')
	    {
	      int_cnt = 2;
	      done = 1;
	    }
	  else
	    {
	      struct mu_wordsplit ws;

	      if (mu_wordsplit (buf + 1, &ws, MU_WRDSF_DEFFLAGS) == 0)
		{
		  if (ws.ws_wordc > 0)
		    {
		      const struct mail_escape_entry *entry = 
			mail_find_escape (ws.ws_wordv[0]);

		      if (entry)
			(*entry->escfunc) (ws.ws_wordc, ws.ws_wordv, env);
		      else
			mu_error (_("Unknown escape %s"), ws.ws_wordv[0]);
		    }
		  else
		    mu_error (_("Unfinished escape"));
		  mu_wordsplit_free (&ws);
		}
	      else
		{
		  mu_error (_("Cannot parse escape sequence: %s"),
			      mu_wordsplit_strerror (&ws));
		}
	    }
	}
      else
	mu_stream_printf (env->compstr, "%s\n", buf);
      mu_stream_flush (env->compstr);
      free (buf);
    }

  /* If interrupted, dump the file to dead.letter.  */
  if (int_cnt)
    {
      save_dead_message_env (env);
      return 1;
    }

  /* In mailx compatibility mode, ask for Cc and Bcc after editing
     the body of the message */
  if (mailvar_is_true ("mailx"))
    read_cc_bcc (env);

  /* Prepare the header */
  if (mailvar_is_true ("xmailer"))
    mu_header_set_value (env->header, MU_HEADER_X_MAILER, program_version, 1);

  if (util_header_expand (&env->header) == 0)
    {
      mu_message_t msg = NULL;
      int status = 0;
      int sendit = (compose_header_get (env, MU_HEADER_TO, NULL) ||
		    compose_header_get (env, MU_HEADER_CC, NULL) ||
		    compose_header_get (env, MU_HEADER_BCC, NULL));
      do
	{
	  status = mu_message_create (&msg, NULL);
	  if (status)
	    break;
	  
	  /* Fill the body. */
	  mu_stream_seek (env->compstr, 0, MU_SEEK_SET, NULL);
	  status = fill_body (msg, env->compstr);
	  if (status)
	    break;
	  
	  mu_message_set_header (msg, env->header, NULL);
	  env->header = NULL;

	  status = add_attachments (env, &msg);
	  if (status)
	    break;
	  
	  /* Save outgoing message */
	  if (save_to)
	    {
	      char *tmp = compose_header_get (env, MU_HEADER_TO, NULL);
	      mu_address_t addr = NULL;
	      
	      mu_address_create (&addr, tmp);
	      mu_address_aget_email (addr, 1, &savefile);
	      mu_address_destroy (&addr);
	      if (savefile)
		{
		  char *p = strchr (savefile, '@');
		  if (p)
		    *p = 0;
		}
	    }
	  util_save_outgoing (msg, savefile);
	  if (savefile)
	    free (savefile);

	  /* Check if we need to save the message to files or pipes.  */
	  if (env->outfiles)
	    {
	      int i;
	      for (i = 0; i < env->nfiles; i++)
		{
		  /* Pipe to a cmd.  */
		  if (env->outfiles[i][0] == '|')
		    status = msg_to_pipe (env->outfiles[i] + 1, msg);
		  /* Save to a file.  */
		  else
		    {
		      mu_mailbox_t mbx = NULL;
		      status = mu_mailbox_create_default (&mbx, 
							  env->outfiles[i]);
		      if (status == 0)
			{
			  status = mu_mailbox_open (mbx, MU_STREAM_WRITE
						    | MU_STREAM_CREAT);
			  if (status == 0)
			    {
			      status = mu_mailbox_append_message (mbx, msg);
			      if (status)
				mu_error (_("Cannot append message: %s"),
					    mu_strerror (status));
			      mu_mailbox_close (mbx);
			    }
			  mu_mailbox_destroy (&mbx);
			}
		      if (status)
			mu_error (_("Cannot create mailbox %s: %s"), 
				    env->outfiles[i],
				    mu_strerror (status));
		    }
		}
	    }
	  
	  /* Do we need to Send the message on the wire?  */
	  if (status == 0 && sendit)
	    {
	      status = send_message (msg);
	      if (status)
		{
		  mu_error (_("cannot send message: %s"),
			    mu_strerror (status));
		  save_dead_message (msg);
		}
	    }
	}
      while (0);

      mu_stream_destroy (&env->compstr);
      mu_message_destroy (&msg, NULL);
      return status;
    }
  else
    save_dead_message_env (env);
  return 1;
}

/* Starting with '|' '/' or not consider addresses and we cheat
   by adding '.' in the mix for none absolute path.  */
static int
isfilename (const char *p)
{
  if (p)
    if (*p == '/' || *p == '.' || *p == '|')
      return 1;
  return 0;
}

/* FIXME: Should probably be in util.c.  */
/* Call popen(cmd) and write the message to it.  */
static int
msg_to_pipe (const char *cmd, mu_message_t msg)
{
  mu_stream_t progstream, msgstream;
  int status, rc;
  
  status = mu_command_stream_create (&progstream, cmd, MU_STREAM_WRITE);
  if (status)
    {
      mu_error (_("Cannot pipe to %s: %s"), cmd, mu_strerror (status));
      return status;
    }

  mu_message_get_streamref (msg, &msgstream);
  status = mu_stream_copy (progstream, msgstream, 0, NULL);
  rc = mu_stream_close (progstream);

  if (status == 0 && rc)
    status = rc;

  mu_stream_destroy (&progstream);
  mu_stream_destroy (&msgstream);
  
  if (status)
    {
      mu_error (_("Sending data to %s failed: %s"), cmd,
		  mu_strerror (status));
    }
  return status;
}
