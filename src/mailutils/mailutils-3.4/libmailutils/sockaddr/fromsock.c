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
   Public License along with this library.  If not, see 
   <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <mailutils/sockaddr.h>
#include <mailutils/errno.h>
#include <mailutils/error.h>

int
mu_sockaddr_from_socket (struct mu_sockaddr **retval, int fd)
{
  int rc;
  struct sockaddr addr;
  socklen_t len = sizeof (addr);
  rc = getsockname (fd, &addr, &len);
  if (rc)
    return rc;
  return mu_sockaddr_create (retval, &addr, len);
}
