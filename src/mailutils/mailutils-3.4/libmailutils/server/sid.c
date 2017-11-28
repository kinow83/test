/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 2017 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General
   Public License along with this library; If not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <mailutils/errno.h>
#include <mailutils/alloc.h>

#define SID_ABC "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define SID_LEN (sizeof (SID_ABC) - 1)
#define SID_LEN_SQR (SID_LEN * SID_LEN)

/* Generate (almost) unique session ID.  The idea borrowed from Sendmail
   queue ID.
 */
int
mu_sid (char **res)
{
  static int init;
  static unsigned int seqno = 0;
  static char abc[] = SID_ABC;
  
  struct timeval t;
  struct tm *tm;
  unsigned int n;
  char sidbuf[9];
  char *p;

  if (!res)
    return MU_ERR_OUT_PTR_NULL;

  gettimeofday (&t, NULL);
  
  if (!init)
    {
      seqno = getpid () + t.tv_sec + t.tv_usec;
      init = 1;
    }
  
  n = seqno++ % SID_LEN_SQR;
  tm = gmtime (&t.tv_sec);
  sidbuf[0] = abc[tm->tm_year % SID_LEN];
  sidbuf[1] = abc[tm->tm_mon];
  sidbuf[2] = abc[tm->tm_mday];
  sidbuf[3] = abc[tm->tm_hour];
  sidbuf[4] = abc[tm->tm_min % SID_LEN];
  sidbuf[5] = abc[tm->tm_sec % SID_LEN];
  sidbuf[6] = abc[n / SID_LEN];
  sidbuf[7] = abc[n % SID_LEN];
  sidbuf[8] = 0;

  p = strdup (sidbuf);
  if (!p)
    return errno;
  *res = p;
  return 0;
}
