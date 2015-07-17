/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_default.c,v 1.15 2010-02-02 00:01:58 chu11 Exp $
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
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "nodeupdown.h"
#include "nodeupdown_api.h"
#include "nodeupdown_clusterlist_util.h"
#include "nodeupdown/nodeupdown_clusterlist_module.h"
#include "nodeupdown/nodeupdown_devel.h"
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

  return count;
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
 * default_clusterlist_get_nodename
 *
 * default clusterlist module get_nodename function
 */
int 
default_clusterlist_get_nodename(nodeupdown_t handle, 
				 const char *node, 
				 char *buf, 
				 unsigned int buflen) 
{
  int len;
  
  len = strlen(node);
  
  if ((len + 1) > buflen)
    {
#ifndef NDEBUG
      fprintf(stderr, "default_clusterlist_get_nodename: invalid buffer length\n");
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      return -1;
    }
  
  strcpy(buf, node);
 
  return 0;
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
    &default_clusterlist_get_nodename,
    &default_clusterlist_compare_to_clusterlist,
  };
