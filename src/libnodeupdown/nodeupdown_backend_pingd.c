/*****************************************************************************\
 *  $Id: nodeupdown_backend_pingd.c,v 1.2 2006-08-30 17:10:00 chu11 Exp $
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
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
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
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_module.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

#include "fd.h"

#define PINGD_BACKEND_DEFAULT_PORT        9125
#define PINGD_BACKEND_DEFAULT_TIMEOUT_LEN 60
#define PINGD_BACKEND_CONNECT_LEN         5 
#define PINGD_BACKEND_BUFLEN              1024

char pingd_default_hostname[NODEUPDOWN_MAXHOSTNAMELEN+1];

/*
 * pingd_backend_default_hostname
 *
 * pingd backend module default_hostname function
 */
static char *
pingd_backend_default_hostname(nodeupdown_t handle)
{
  memset(pingd_default_hostname, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
  if (gethostname(pingd_default_hostname, NODEUPDOWN_MAXHOSTNAMELEN) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "gethostname: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      return NULL;
    }
  return &pingd_default_hostname[0];
}

/*
 * pingd_backend_default_port
 *
 * pingd backend module default_port function
 */
static int 
pingd_backend_default_port(nodeupdown_t handle)
{
  return PINGD_BACKEND_DEFAULT_PORT;
}

/*
 * pingd_backend_default_timeout_len
 *
 * pingd backend module default_timeout_len function
 */
static int 
pingd_backend_default_timeout_len(nodeupdown_t handle)
{
  return PINGD_BACKEND_DEFAULT_TIMEOUT_LEN;
}

/*
 * pingd_backend_setup
 *
 * pingd backend module setup function
 */
static int 
pingd_backend_setup(nodeupdown_t handle)
{
  /* nothing to do */
  return 0;
}

/*
 * pingd_backend_cleanup
 *
 * pingd backend module cleanup function
 */
static int
pingd_backend_cleanup(nodeupdown_t handle)
{
  /* nothing to do */
  return 0;
}

/*
 * _low_timeout_connect
 *
 * Setup a tcp connection to 'hostname' and 'port' using a connection
 * timeout of 'connect_timeout'.
 *
 * Return file descriptor on success, -1 on error.
 */
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

/*
 * pingd_backend_get_updown_data
 *
 * pingd backend module get_updown_data function
 */
static int 
pingd_backend_get_updown_data(nodeupdown_t handle, 
                              const char *hostname,
                              unsigned int port,
                              unsigned int timeout_len,
                              char *reserved) 
{
  struct timeval tv;
  int fd, rv = -1;

  if ((fd = _low_timeout_connect(handle,
                                 hostname,
                                 port,
                                 PINGD_BACKEND_CONNECT_LEN)) < 0)
    goto cleanup;

  /* Call gettimeofday at the latest point right before XML stuff. */
  if (gettimeofday(&tv, NULL) < 0) 
    {
#ifndef NDEBUG
      fprintf(stderr, "gettimeofday: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      goto cleanup;
    } 

  while (1)
    {
      char buf[PINGD_BACKEND_BUFLEN];
      char hostname[NODEUPDOWN_MAXHOSTNAMELEN+1];
      unsigned long int localtime;
      int len, num;

      if ((len = fd_read_line(fd, buf, PINGD_BACKEND_BUFLEN)) < 0)
        {
#ifndef NDEBUG
	  fprintf(stderr, "fd_read_line: %s\n", strerror(errno));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
          goto cleanup;
        }

      if (!len)
        break;
      
      num = sscanf(buf, "%s %lu\n", hostname, &localtime);
      if (num != 2)
        {
#ifndef NDEBUG
	  fprintf(stderr, "sscanf: parse error\n");
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
          goto cleanup;
        }

      if (abs(localtime - tv.tv_sec) < timeout_len)
        nodeupdown_add_up_node(handle, hostname);
      else
        nodeupdown_add_down_node(handle, hostname);
    }

  rv = 0;
 cleanup:
  close(fd);
  return rv;
}

struct nodeupdown_backend_module_info backend_module_info = 
  {
    "pingd",
    &pingd_backend_default_hostname,
    &pingd_backend_default_port,
    &pingd_backend_default_timeout_len,
    &pingd_backend_setup,
    &pingd_backend_cleanup,
    &pingd_backend_get_updown_data
  };
