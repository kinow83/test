/* This file is part of GNU Mailutils test suite
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

#include <mailutils/mailutils.h>
#include <mailutils/locus.h>

/* Check normal operation.
   Expected output: hello world
*/
static void
simple_print (mu_stream_t str)
{
  mu_stream_printf (str, "hello world\n");
}

/* Check severity control.
   Expected output:
     info: one
     emerg: two
     mode was: 0x0001
*/
static void
check_severity (mu_stream_t str)
{
  unsigned severity = MU_DIAG_INFO;
  int mode = MU_LOGMODE_SEVERITY, old_mode;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "one\n");

  severity = MU_DIAG_EMERG;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "two\n");

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_GET_MODE, &old_mode);

  mode = 0;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  mu_stream_printf (str, "mode was: 0x%04X\n", old_mode);
}

/* Check severity suppression mechanism.
   Expected output:
     info: this message is seen
     emerg: and this one as well
*/
static void
check_suppress (mu_stream_t str)
{
  unsigned severity;
  int mode = MU_LOGMODE_SEVERITY;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);

  severity = MU_DIAG_INFO;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SUPPRESS_SEVERITY, 
		   &severity);

  severity = MU_DIAG_DEBUG;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);

  mu_stream_printf (str, "this message is not seen\n");

  severity = MU_DIAG_INFO;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "this message is seen\n");
  
  severity = MU_DIAG_EMERG;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "and this one as well\n");
}

/* Check suppression by severity name.
   Expected output:
     info: this message is seen
     emerg: and this one as well
*/
static void
check_suppress_name (mu_stream_t str)
{
  int mode = MU_LOGMODE_SEVERITY, severity;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SUPPRESS_SEVERITY_NAME, 
		   "info");

  severity = MU_DIAG_DEBUG;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);

  mu_stream_printf (str, "this message is not seen\n");

  severity = MU_DIAG_INFO;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "this message is seen\n");
  
  severity = MU_DIAG_EMERG;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "and this one as well\n");
}

static void
comp_range (mu_stream_t str, struct mu_locus_range *lr1,
	    char const *file, int line)
{
  struct mu_locus_range lr2;
  
  MU_ASSERT (mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
			      MU_IOCTL_LOGSTREAM_GET_LOCUS_RANGE, &lr2));
  if (strcmp (lr1->beg.mu_file, lr2.beg.mu_file))
    mu_error ("%s:%d: beg.mu_file differs", file, line);
  if (lr1->beg.mu_line != lr2.beg.mu_line)
    mu_error ("%s:%d: beg.mu_line differs", file, line);
  if (lr1->beg.mu_col != lr2.beg.mu_col)
    mu_error ("%s:%d: beg.mu_col differs", file, line);
  mu_ident_deref (lr2.beg.mu_file);

  if (strcmp (lr1->end.mu_file, lr2.end.mu_file))
    mu_error ("%s:%d: end.mu_file differs", __FILE__, __LINE__);
  if (lr1->end.mu_line != lr2.end.mu_line)
    mu_error ("%s:%d: end.mu_line differs", __FILE__, __LINE__);
  if (lr1->end.mu_col != lr2.end.mu_col)
    mu_error ("%s:%d: end.mu_col differs", __FILE__, __LINE__);

  mu_ident_deref (lr2.end.mu_file);
}

/* Check MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE.
   Passed: file, line.
   Expected output:
     input:1: file, line
*/
static void
lr_file_line (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 0;

  lr.end.mu_file = "input";
  lr.end.mu_line = 1;
  lr.end.mu_col = 0;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mu_stream_printf (str, "file, line\n");
  
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE.
   Passed: file, line, col.
   Expected output:
     input:1.1-10: file, line, col
*/
static void
lr_file_line_col (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "input";
  lr.end.mu_line = 1;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mu_stream_printf (str, "file, line, col\n");
  
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE.
   Passed: file, line-range
   Expected output:
     input:1-2: file, line-range
*/
static void
lr_file_line2 (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 0;

  lr.end.mu_file = "input";
  lr.end.mu_line = 2;
  lr.end.mu_col = 0;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mu_stream_printf (str, "file, line-range\n");
  
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE.
   Passed: file, line-range, column
   Expected output:
     input:1.1-2.10: file, line-range, col
*/
static void
lr_file_line2_col (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "input";
  lr.end.mu_line = 2;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mu_stream_printf (str, "file, line-range, col\n");
  
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE.
   Passed: file-range, line-range, column-range
   Expected output:
     input:1.1-next:2.10: file-range, line-range, col-range
*/
static void
lr_file2_line_col (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 2;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mu_stream_printf (str, "file-range, line-range, col-range\n");
  
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_SET_LOCUS_LINE.
   Expected output:
     input:1.1-next:2.10: initial
     input:8.1-next:2.10: locus line changed
*/
static void
set_locus_line (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  unsigned line;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 2;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);
  mu_stream_printf (str, "initial\n");
  line = 8;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_LINE, &line);
  mu_stream_printf (str, "locus line changed\n");
  lr.beg.mu_line = line;
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_LINE.
   Expected output:
     input:1.1-next:5.10: initial
     input:3.1-next:5.10: locus line advanced
*/
static void
advance_locus_line (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  unsigned line;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 5;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);
  mu_stream_printf (str, "initial\n");
  line = 2;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_LINE, &line);
  mu_stream_printf (str, "locus line advanced\n");

  lr.beg.mu_line += line;
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_SET_LOCUS_COL.
   Expected output:
     input:1.1-next:2.10: initial
     input:1.8-next:2.10: locus column changed
*/
static void
set_locus_col (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  unsigned col;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 2;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);
  mu_stream_printf (str, "initial\n");
  col = 8;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_COL, &col);
  mu_stream_printf (str, "locus column changed\n");
  lr.beg.mu_col = col;
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_COL.
   Expected output:
     input:1.1-next:5.10: initial
     input:1.5-next:5.10: locus line advanced
*/
static void
advance_locus_col (mu_stream_t str)
{
  int mode = MU_LOGMODE_LOCUS;
  struct mu_locus_range lr;
  unsigned col;
  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 5;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);
  mu_stream_printf (str, "initial\n");
  col = 4;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_ADVANCE_LOCUS_COL, &col);
  mu_stream_printf (str, "locus line advanced\n");

  lr.beg.mu_col += col;
  comp_range (str, &lr, __FILE__, __LINE__);
}

/* Check severity mask.
   Expected output:
     one
     two
     emerg: two
 */
static void
check_severity_mask (mu_stream_t str)
{
  unsigned severity;
  int mode = MU_LOGMODE_SEVERITY, mask;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);

  mask = MU_DEBUG_LEVEL_UPTO (MU_DIAG_NOTICE);
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY_MASK, &mask);

  severity = MU_DIAG_INFO;  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "one\n");

  severity = MU_DIAG_NOTICE;  
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "two\n");

  severity = MU_DIAG_EMERG;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_SEVERITY, &severity);
  mu_stream_printf (str, "three\n");
}

/* Check ESC-s & ESC-O format specifiers.
   Expected output:
     info: severity
*/
static void
fmt_severity (mu_stream_t str)
{
  mu_stream_printf (str, "\033s<%d>\033O<%d>severity\n",
		    MU_DIAG_INFO, MU_LOGMODE_SEVERITY);
}

/* Check ESC-f and ESC-l format specifiers.
   Expected output:
     a:10: one
*/
static void
fmt_locus1 (mu_stream_t str)
{
  char *file = "a";
  mu_stream_printf (str, "\033f<%d>%s\033l<%d>one\n",
		    (int) strlen (file), file, 10);
}

/* Check ESC-f, ESC-l, and ESC-c format specifiers.
   Expected output:
     a:10.5: one
*/
static void
fmt_locus2 (mu_stream_t str)
{
  char *file = "a";
  mu_stream_printf (str, "\033f<%d>%s\033l<%d>\033c<%d>one\n",
		    (int) strlen (file), file, 10, 5);
}

/* Check setting range with ESC-f, ESC-l, and ESC-c format specifiers.
   Expected output:
     a:10.5-b:14.8: one
*/   
static void
fmt_locus3 (mu_stream_t str)
{
  char *file[] = { "a", "b" };
  mu_stream_printf (str, "\033f<%d>%s\033l<%d>\033c<%d>"
		    "\033f<%d>%s\033l<%d>\033c<%d>one\n",
		    (int) strlen (file[0]), file[0], 10, 5,
		    (int) strlen (file[1]), file[1], 14, 8);
}

/* Check that ESC-f, ESC-l, and ESC-c format specifiers don't clobber
   default stream settings.
   Expected output:
     a:10.5-b:14.8: one
     default
*/
static void
fmt_locus4 (mu_stream_t str)
{
  char *file[] = { "a", "b" };
  struct mu_locus_range lr;
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 5;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mu_stream_printf (str, "\033f<%d>%s\033l<%d>\033c<%d>"
		    "\033f<%d>%s\033l<%d>\033c<%d>one\n",
		    (int) strlen (file[0]), file[0], 10, 5,
		    (int) strlen (file[1]), file[1], 14, 8);
  mu_stream_printf (str, "default\n");
}

/* Check that ESC-f, ESC-l, and ESC-c format specifiers don't clobber
   default stream settings and locus.
   Expected output:
     a:10.5-b:14.8: one
     input:1.1-next:5.10: default
*/
static void
fmt_locus5 (mu_stream_t str)
{
  char *file[] = { "a", "b" };
  struct mu_locus_range lr;
  int mode;
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 5;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mode = MU_LOGMODE_LOCUS;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  mu_stream_printf (str, "\033f<%d>%s\033l<%d>\033c<%d>"
		    "\033f<%d>%s\033l<%d>\033c<%d>one\n",
		    (int) strlen (file[0]), file[0], 10, 5,
		    (int) strlen (file[1]), file[1], 14, 8);
  mu_stream_printf (str, "default\n");
}

/* Check the ESC-S specifier (store locus).
   Expected output:
     a:10.5-b:14.8: one
     a:10.5-b:14.8: default
*/
static void
fmt_locus6 (mu_stream_t str)
{
  char *file[] = { "a", "b" };
  struct mu_locus_range lr;
  int mode;
  
  lr.beg.mu_file = "input";
  lr.beg.mu_line = 1;
  lr.beg.mu_col = 1;

  lr.end.mu_file = "next";
  lr.end.mu_line = 5;
  lr.end.mu_col = 10;

  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, &lr);

  mode = MU_LOGMODE_LOCUS;
  mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
		   MU_IOCTL_LOGSTREAM_SET_MODE, &mode);
  mu_stream_printf (str, "\033S\033f<%d>%s\033l<%d>\033c<%d>"
		    "\033f<%d>%s\033l<%d>\033c<%d>one\n",
		    (int) strlen (file[0]), file[0], 10, 5,
		    (int) strlen (file[1]), file[1], 14, 8);
  mu_stream_printf (str, "default\n");
}


struct testcase
{
  char const *id;
  void (*handler) (mu_stream_t);
  int enabled;
};

struct testcase testcases[] = {
  { "simple print", simple_print },
  { "severity", check_severity },
  { "suppress severity", check_suppress },
  { "suppress severity name", check_suppress_name },
  { "severity mask", check_severity_mask },
  { "locus: file, line", lr_file_line },
  { "locus: file, line, col", lr_file_line_col },
  { "locus: file, line-range", lr_file_line2 },
  { "locus: file, line-range, col", lr_file_line2_col },
  { "locus: file-range, line-range, col-range", lr_file2_line_col },
  { "set locus line", set_locus_line },
  { "advance locus line", advance_locus_line },
  { "set locus column", set_locus_col },
  { "advance locus column", advance_locus_col },
  { "fmt: severity", fmt_severity },
  { "fmt: locus (file, line)", fmt_locus1 },
  { "fmt: locus (file, line, column)", fmt_locus2 },
  { "fmt: locus (range)", fmt_locus3 },
  { "fmt: locus; restore defaults", fmt_locus4 },
  { "fmt: locus; restore defaults, display locus", fmt_locus5 },
  { "fmt: set locus", fmt_locus6 },
  { NULL }
};

static mu_stream_t
create_log (void)
{
  mu_stream_t str, transport;
  int yes = 1;
  
  MU_ASSERT (mu_stdio_stream_create (&transport, MU_STDOUT_FD, 0));
  mu_stream_ioctl (transport, MU_IOCTL_FD, MU_IOCTL_FD_SET_BORROW, &yes);
  MU_ASSERT (mu_log_stream_create (&str, transport));
  mu_stream_unref (transport);
  return str;
}

static void
log_reset (mu_stream_t str)
{
  int mode = 0;
  MU_ASSERT (mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
			      MU_IOCTL_LOGSTREAM_SET_MODE, &mode));
  MU_ASSERT (mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
			      MU_IOCTL_LOGSTREAM_SET_LOCUS_RANGE, NULL));
  mode = 0;
  MU_ASSERT (mu_stream_ioctl (str, MU_IOCTL_LOGSTREAM,
			      MU_IOCTL_LOGSTREAM_SET_SEVERITY_MASK,
			      &mode));
}

int
main (int argc, char **argv)
{
  mu_stream_t log;
  struct testcase *tp;
  int i;
  int ena = 0;
  
  mu_set_program_name (argv[0]);
  mu_stdstream_setup (MU_STDSTREAM_RESET_NONE);

  if (argc > 1)
    {
      ena = 1;
      for (i = 1; i < argc; i++)
	{
	  char *p;
	  int n = strtol (argv[i], &p, 10);
	  if (! (*p == 0 && n >= 0) )
	    {
	      mu_error ("erroneous argument %s\n", argv[i]);
	      return 1;
	    }
	  testcases[n].enabled = ena;
	}
    }

  log = create_log ();

  for (i = 0, tp = testcases; tp->id; tp++, i++)
    {
      if (tp->enabled == ena)
	{
	  mu_stream_printf (log, "%02d. %s\n", i, tp->id);
	  tp->handler (log);
	  log_reset (log);
	}
    }
  
  return 0;
}

  
