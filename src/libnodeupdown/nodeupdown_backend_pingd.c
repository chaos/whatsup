/*****************************************************************************\
 *  $Id: nodeupdown_backend_pingd.c,v 1.10 2010-02-02 00:01:58 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
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
 *  with Whatsup.  If not, see <http://www.gnu.org/licenses/>.
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
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_backend_connect_util.h"
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
 * pingd_backend_preferred_clusterlist_module
 *
 * pingd backend preferred_clusterlist_module function
 */
static char *
pingd_backend_preferred_clusterlist_module(nodeupdown_t handle)
{
  return NULL;
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

  /* Call gettimeofday at the latest point right before getting data. */
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
  /* ignore potential error, just return error or result */
  close(fd);
  return rv;
}

struct nodeupdown_backend_module_info backend_module_info =
  {
    "pingd",
    &pingd_backend_default_hostname,
    &pingd_backend_default_port,
    &pingd_backend_default_timeout_len,
    &pingd_backend_preferred_clusterlist_module,
    &pingd_backend_setup,
    &pingd_backend_cleanup,
    &pingd_backend_get_updown_data
  };
