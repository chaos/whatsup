/*****************************************************************************\
 *  $Id: nodeupdown_backend_cerebro.c,v 1.25 2010-02-02 00:01:58 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155699
 *
 *  This file is part of Whatsup, tools and libraries for determining up and
 *  down nodes in a cluster. For details, see https://github.com/chaos/whatsup.
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
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <sys/types.h>
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_module.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

#include <cerebro.h>

static cerebro_t cerebro_handle = NULL;

char cerebro_default_hostname[NODEUPDOWN_MAXHOSTNAMELEN+1];

/*
 * cerebro_backend_default_hostname
 *
 * cerebro backend module default_hostname function
 */
static char *
cerebro_backend_default_hostname(nodeupdown_t handle)
{
  memset(cerebro_default_hostname, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
  if (gethostname(cerebro_default_hostname, NODEUPDOWN_MAXHOSTNAMELEN) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "gethostname: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      return NULL;
    }
  return &cerebro_default_hostname[0];
}

/*
 * cerebro_backend_default_port
 *
 * cerebro backend module default_port function
 */
static int
cerebro_backend_default_port(nodeupdown_t handle)
{
  return CEREBRO_METRIC_SERVER_PORT;
}

/*
 * cerebro_backend_default_timeout_len
 *
 * cerebro backend module default_timeout_len function
 */
static int
cerebro_backend_default_timeout_len(nodeupdown_t handle)
{
  /* Let libcerebro or the cerebro metric server pick the default */
  return 0;
}

/*
 * cerebro_backend_preferred_clusterlist_module
 *
 * cerebro backend preferred_clusterlist_module function
 */
static char *
cerebro_backend_preferred_clusterlist_module(nodeupdown_t handle)
{
  return NULL;
}

/*
 * cerebro_backend_setup
 *
 * cerebro backend module setup function
 */
static int
cerebro_backend_setup(nodeupdown_t handle)
{
  if (cerebro_handle)
    return 0;

  if (!(cerebro_handle = cerebro_handle_create()))
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_handle_create: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      goto cleanup;
    }

  return 0;
 cleanup:
  if (cerebro_handle)
    cerebro_handle_destroy(cerebro_handle);
  cerebro_handle = NULL;
  return -1;
}

/*
 * cerebro_backend_cleanup
 *
 * cerebro backend module cleanup function
 */
static int
cerebro_backend_cleanup(nodeupdown_t handle)
{
  if (!cerebro_handle)
    return 0;

  if (cerebro_handle_destroy(cerebro_handle) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_handle_destroy: %s\n",
	      cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      return -1;
    }

  return 0;
}

/*
 * cerebro_backend_get_updown_data
 *
 * cerebro backend module get_updown_data function
 */
static int
cerebro_backend_get_updown_data(nodeupdown_t handle,
                                const char *hostname,
                                unsigned int port,
                                unsigned int timeout_len,
                                char *reserved)
{
  cerebro_nodelist_t nodelist = NULL;
  cerebro_nodelist_iterator_t itr = NULL;
  int flag, rv = -1;

  if (cerebro_set_hostname(cerebro_handle, hostname) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_set_hostname: %s\n",
	      cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      return -1;
    }

  if (cerebro_set_port(cerebro_handle, port) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_set_port: %s\n",
	      cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      return -1;
    }

  if (cerebro_set_timeout_len(cerebro_handle, timeout_len) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_set_timeout_len: %s\n",
	      cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      return -1;
    }

  if (!(nodelist = cerebro_get_metric_data(cerebro_handle,
                                           CEREBRO_METRIC_UPDOWN_STATE)))
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_get_metric_data: %s\n",
	      cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
      if (cerebro_errnum(cerebro_handle) == CEREBRO_ERR_CONNECT)
        nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONNECT);
      else if (cerebro_errnum(cerebro_handle) == CEREBRO_ERR_CONNECT_TIMEOUT)
        nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONNECT_TIMEOUT);
      else if (cerebro_errnum(cerebro_handle) == CEREBRO_ERR_HOSTNAME)
        nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_HOSTNAME);
      else
        nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      return -1;
    }

  if (!(itr = cerebro_nodelist_iterator_create(nodelist)))
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_nodelist_iterator_create: %s\n",
	      cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      goto cleanup;
    }

  while (!(flag = cerebro_nodelist_iterator_at_end(itr)))
    {
      char *nodename;
      unsigned int mtime, mtype, mlen;
      void *mvalue;
      u_int32_t updown_state;

      if (cerebro_nodelist_iterator_nodename(itr, &nodename) < 0)
        {
#ifndef NDEBUG
	  fprintf(stderr, "cerebro_nodelist_iterator_nodename: %s\n",
		  cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
          goto cleanup;
        }

      if (!nodename)
        {
#ifndef NDEBUG
	  fprintf(stderr, "cerebro_nodelist_iterator_create: null nodename\n");
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
          goto cleanup;
        }

      if (cerebro_nodelist_iterator_metric_value(itr,
                                                 &mtime,
                                                 &mtype,
                                                 &mlen,
                                                 &mvalue) < 0)
        {
#ifndef NDEBUG
	  fprintf(stderr, "cerebro_nodelist_iterator_metric_value: %s\n",
		  cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
          goto cleanup;
        }

#ifdef CEREBRO_METRIC_VALUE_TYPE_U_INT32
      if (mtype != CEREBRO_METRIC_VALUE_TYPE_U_INT32)
#else  /* !CEREBRO_METRIC_VALUE_TYPE_U_INT32 */
      if (mtype != CEREBRO_DATA_VALUE_TYPE_U_INT32)
#endif /* !CEREBRO_METRIC_VALUE_TYPE_U_INT32 */
        {
#ifndef NDEBUG
	  fprintf(stderr, "cerebro_nodelist_iterator_metric_value: invalid mtype: %u\n", mtype);
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
          goto cleanup;
        }

      if (mlen != sizeof(u_int32_t))
        {
#ifndef NDEBUG
	  fprintf(stderr, "cerebro_nodelist_iterator_metric_value: invalid mlen: %u\n", mlen);
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
          goto cleanup;
        }

      updown_state = *((u_int32_t *)mvalue);

      if (updown_state)
        {
          if (nodeupdown_add_up_node(handle, nodename) < 0)
            goto cleanup;
        }
      else
        {
          if (nodeupdown_add_down_node(handle, nodename) < 0)
            goto cleanup;
        }

      if (nodeupdown_add_last_up_time(handle, nodename, mtime) < 0)
        goto cleanup;

      if (cerebro_nodelist_iterator_next(itr) < 0)
        {
#ifndef NDEBUG
	  fprintf(stderr, "cerebro_nodelist_iterator_next: %s\n",
		  cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
          goto cleanup;
        }
    }

  if (flag < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "cerebro_nodelist_iterator_at_end: %s\n",
	      cerebro_strerror(cerebro_errnum(cerebro_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_BACKEND_MODULE);
      goto cleanup;
    }

  rv = 0;
 cleanup:
  if (itr)
    cerebro_nodelist_iterator_destroy(itr);
  if (nodelist)
    cerebro_nodelist_destroy(nodelist);
  return rv;
}

struct nodeupdown_backend_module_info backend_module_info =
  {
    "cerebro",
    &cerebro_backend_default_hostname,
    &cerebro_backend_default_port,
    &cerebro_backend_default_timeout_len,
    &cerebro_backend_preferred_clusterlist_module,
    &cerebro_backend_setup,
    &cerebro_backend_cleanup,
    &cerebro_backend_get_updown_data
  };
