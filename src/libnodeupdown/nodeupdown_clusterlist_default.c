/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_default.c,v 1.3 2005-05-05 18:20:41 achu Exp $
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

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_clusterlist.h"
#include "nodeupdown_clusterlist_util.h"
#include "nodeupdown/nodeupdown_clusterlist_module.h"
#include "hostlist.h"

/*
 * default_clusterlist_setup
 *
 * default clusterlist module setup function
 */
int 
default_clusterlist_setup(nodeupdown_t handle) 
{
  return 0;
}

/*
 * default_clusterlist_cleanup
 *
 * default clusterlist module cleanup function
 */
int 
default_clusterlist_cleanup(nodeupdown_t handle) 
{
  return 0;
}

/*
 * default_clusterlist_get_numnodes
 *
 * default clusterlist module get_numnodes function
 */
int 
default_clusterlist_get_numnodes(nodeupdown_t handle) 
{
  int count = 0;
  
  count += hostlist_count(handle->up_nodes);
  count += hostlist_count(handle->down_nodes);

  handle->max_nodes = count;
  return 0;
}

/*
 * default_clusterlist_is_node_in_cluster
 *
 * default clusterlist module is_node_in_cluster function
 */
int 
default_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  /* Have to assume it is */
  return 1;
}

/*
 * default_clusterlist_is_node_discovered
 *
 * default clusterlist module is_node_discovered function
 */
int 
default_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node) 
{
  /* Without a clusterlist_ of some sort, this is the best we can do */
  if (hostlist_find(handle->up_nodes, node) == -1 
      && hostlist_find(handle->down_nodes, node) == -1)
    return 0;
  else
    return 1;
}

/*
 * default_clusterlist_get_nodename
 *
 * default clusterlist module get_nodename function
 */
int 
default_clusterlist_get_nodename(nodeupdown_t handle, 
				 const char *node, 
				 char *buffer, 
				 int buflen) 
{
  return nodeupdown_clusterlist_copy_nodename(handle, node, buffer, buflen);
}

/*
 * default_clusterlist_compare_to_clusterlist
 *
 * default clusterlist module compare_to_clusterlist function
 */
int 
default_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  return 0;
}

struct nodeupdown_clusterlist_module_info default_clusterlist_module_info =
  {
    "default",
    &default_clusterlist_setup,
    &default_clusterlist_cleanup,
    &default_clusterlist_get_numnodes,
    &default_clusterlist_is_node_in_cluster,
    &default_clusterlist_is_node_discovered,
    &default_clusterlist_get_nodename,
    &default_clusterlist_compare_to_clusterlist,
  };
