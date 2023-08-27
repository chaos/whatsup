/*****************************************************************************\
 *  $Id: nodeupdown_devel.c,v 1.11 2010-02-02 00:01:58 chu11 Exp $
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
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_api.h"
#include "nodeupdown_module.h"
#include "nodeupdown_util.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"
#include "hostlist.h"

/*
 * _setup_handle_error_check
 *
 * standard setup handle error checker
 *
 * Returns -1 on error, 0 on success
 */
static int
_setup_handle_error_check(nodeupdown_t handle)
{
  if (_nodeupdown_handle_error_check(handle) < 0)
    return -1;

  if (handle->load_state != NODEUPDOWN_LOAD_STATE_SETUP)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return 0;
}

/*
 * _add_node
 *
 * Common function for nodeupdown_add_up_node and
 * nodeupdown_add_down_node
 *
 * Returns 0 on success, -1 one error
 */
int
_add_node(nodeupdown_t handle, const char *node, int up_or_down)
{
  char buffer[NODEUPDOWN_MAXNODENAMELEN+1];
  int rv;

  if (_setup_handle_error_check(handle) < 0)
    return -1;

  if (!node)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  if ((rv = clusterlist_module_is_node_in_cluster(handle, node)) < 0)
    goto cleanup;

  if (!rv)
    return 0;

  if (clusterlist_module_get_nodename(handle,
                                      node,
                                      buffer,
                                      NODEUPDOWN_MAXNODENAMELEN+1) < 0)
    goto cleanup;

  if (up_or_down == NODEUPDOWN_UP_NODES)
    rv = hostlist_push(handle->up_nodes, buffer);
  else
    rv = hostlist_push(handle->down_nodes, buffer);

  if (!rv)
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

  return 0;

 cleanup:
  return -1;
}

int
nodeupdown_add_up_node(nodeupdown_t handle, const char *node)
{
  return _add_node(handle, node, NODEUPDOWN_UP_NODES);
}

int
nodeupdown_add_down_node(nodeupdown_t handle, const char *node)
{
  return _add_node(handle, node, NODEUPDOWN_DOWN_NODES);
}

int
nodeupdown_add_last_up_time(nodeupdown_t handle,
                            const char *node,
                            unsigned int last_up_time)
{
  struct last_up_time *lut;

  if (_setup_handle_error_check(handle) < 0)
    return -1;

  if (!node)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  if (!(lut = malloc(sizeof(struct last_up_time))))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

  if (!(lut->node = strdup(node)))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
  lut->last_up_time = last_up_time;

  if (!hash_insert(handle->last_up_times,
                   lut->node,
                   lut))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

  return 0;

 cleanup:
  if (lut)
    {
      if (lut->node)
        free(lut->node);
      free(lut);
    }
  return -1;
}

int
nodeupdown_not_discovered_check(nodeupdown_t handle, const char *node)
{
  if (_setup_handle_error_check(handle) < 0)
    return -1;

  if (!node)
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      return -1;
    }

  if (hostlist_find(handle->up_nodes, node) >= 0
      || hostlist_find(handle->down_nodes, node) >= 0)
    return 0;

  return nodeupdown_add_down_node(handle, node);
}

void
nodeupdown_set_errnum(nodeupdown_t handle, int errnum)
{
  /* Does not need to be setup or loaded */
  if (_nodeupdown_handle_error_check(handle) < 0)
    return;

  if (errnum >= NODEUPDOWN_ERR_SUCCESS && errnum <= NODEUPDOWN_ERR_ERRNUMRANGE)
    handle->errnum = errnum;
  else
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
}
