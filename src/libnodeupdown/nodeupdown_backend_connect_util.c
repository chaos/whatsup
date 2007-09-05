/*****************************************************************************\
 *  $Id: nodeupdown_backend_connect_util.c,v 1.2 2007-09-05 17:29:25 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2003 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155699
 *  
 *  This file is part of Whatsup, tools and libraries for determining up and
 *  down nodes in a cluster. For details, see http://www.llnl.gov/linux/.
 *  
 *  Whatsup is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your 
 *  option) any later version.
 *  
 *  Whatsup is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
 *  for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with Whatsup; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_devel.h"

int
_low_timeout_connect(nodeupdown_t handle,
                     const char *hostname,
                     int port,
                     int connect_timeout)
{
  int rv, old_flags, fd = -1;
  struct sockaddr_in servaddr;
  struct hostent *hptr;
  
  /* valgrind will report a mem-leak in gethostbyname() */
  if (!(hptr = gethostbyname(hostname)))
    {
#ifndef NDEBUG
      fprintf(stderr, "gethostbyname: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_HOSTNAME);
      return -1;
    }

  /* Alot of this code is from Unix Network Programming, by Stevens */
  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "socket: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      goto cleanup;
    }
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr = *((struct in_addr *)hptr->h_addr);

  if ((old_flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "fcntl: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      goto cleanup;
    }

  if (fcntl(fd, F_SETFL, old_flags | O_NONBLOCK) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "fcntl: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      goto cleanup;
    }

  rv = connect(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
  if (rv < 0 && errno != EINPROGRESS)
    {
#ifndef NDEBUG
      fprintf(stderr, "connect: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONNECT);
      goto cleanup;
    }
  else if (rv < 0 && errno == EINPROGRESS)
    {
      fd_set rset, wset;
      struct timeval tval;

      FD_ZERO(&rset);
      FD_SET(fd, &rset);
      FD_ZERO(&wset);
      FD_SET(fd, &wset);
      tval.tv_sec = connect_timeout;
      tval.tv_usec = 0;

      if ((rv = select(fd+1, &rset, &wset, NULL, &tval)) < 0)
        {
#ifndef NDEBUG
	  fprintf(stderr, "select: %s\n", strerror(errno));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
          goto cleanup;
        }

      if (!rv)
        {
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONNECT_TIMEOUT);
          goto cleanup;
        }
      else
        {
          if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset))
            {
              int len, error;

              len = sizeof(int);

              if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
                {
#ifndef NDEBUG
		  fprintf(stderr, "getsockopt: %s\n", strerror(errno));
#endif /* NDEBUG */
                  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
                  goto cleanup;
                }

              if (error != 0)
                {
                  if (error == ECONNREFUSED)
                    nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONNECT);
                  else if (error == ETIMEDOUT)
                    nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONNECT_TIMEOUT);
                  else
                    nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
                  goto cleanup;
                }
              /* else no error, connected within timeout length */
            }
          else
            {
#ifndef NDEBUG
	      fprintf(stderr, "select: invalid state\n");
#endif /* NDEBUG */
              nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
              goto cleanup;
            }
        }
    }

  /* reset flags */
  if (fcntl(fd, F_SETFL, old_flags) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "fcntl: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      goto cleanup;
    }

  return fd;

 cleanup:
  close(fd);
  return -1;
}
