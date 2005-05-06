/*****************************************************************************\
 *  $Id: nodeupdown_node.c,v 1.3 2005-05-06 17:15:28 achu Exp $
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
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_api.h"
#include "nodeupdown_constants.h"
#include "nodeupdown_module.h"
#include "nodeupdown_node.h"
#include "hostlist.h"

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
      
  if (!node)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  if ((rv = nodeupdown_clusterlist_module_is_node_in_cluster(handle, 
							     node)) < 0)
    goto cleanup;
  
  if (!rv)
    return 0;
  
  if (nodeupdown_clusterlist_module_get_nodename(handle,
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
      handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
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
  return _add_node(handle, node, NODEUPDOWN_UP_NODES);
}

/* 
 * _add_nodes
 *
 * Common function for nodeupdown_add_up_nodes
 * and nodeupdown_add_down_nodes
 *
 * Returns 0 on success, -1 on error
 */
static int
_add_nodes(nodeupdown_t handle, const char *nodes, int up_or_down)
{
  hostlist_t hl = NULL;
  hostlist_iterator_t itr = NULL;
  char *nodename = NULL;

  if (!nodes)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  if (!(hl = hostlist_create(nodes)))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;

    }

  if (!(itr = hostlist_iterator_create(hl)))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
  
  while ((nodename = hostlist_next(itr)))
    {
      char buffer[NODEUPDOWN_MAXNODENAMELEN+1];
      int rv;
      
      if ((rv = nodeupdown_clusterlist_module_is_node_in_cluster(handle, 
								 nodename)) < 0)
	goto cleanup;

      if (!rv)
	goto loop_end;

      if (nodeupdown_clusterlist_module_get_nodename(handle,
						     nodename,
						     buffer,
						     NODEUPDOWN_MAXNODENAMELEN+1) < 0)
	goto cleanup;
      
      if (up_or_down == NODEUPDOWN_UP_NODES)
	rv = hostlist_push(handle->up_nodes, buffer);
      else
	rv = hostlist_push(handle->down_nodes, buffer);
      
      if (!rv)
	{
	  handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
	  goto cleanup;
	}

    loop_end:
      free(nodename);
    }
  nodename = NULL;

  return 0;
 cleanup:
  if (itr)
    hostlist_iterator_destroy(itr);
  if (hl)
    hostlist_destroy(hl);
  if (nodename)
    free(nodename);
  return -1;
}

int 
nodeupdown_add_up_nodes(nodeupdown_t handle, const char *nodes)
{
  return _add_nodes(handle, nodes, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_add_down_nodes(nodeupdown_t handle, const char *nodes)
{
  return _add_nodes(handle, nodes, NODEUPDOWN_DOWN_NODES);
}

int
nodeupdown_not_discovered_check(nodeupdown_t handle, const char *node)
{
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
