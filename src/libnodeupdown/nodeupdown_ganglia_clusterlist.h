/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist.h,v 1.6 2005-04-05 23:13:01 achu Exp $
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

#ifndef _NODEUPDOWN_GANGLIA_CLUSTERLIST_H
#define _NODEUPDOWN_GANGLIA_CLUSTERLIST_H

#include "nodeupdown.h"

/*  
 * Clusterlist function prototypes
 */

typedef int (*Nodeupdown_ganglia_clusterlist_init)(nodeupdown_t);
typedef int (*Nodeupdown_ganglia_clusterlist_cleanup)(nodeupdown_t);
typedef int (*Nodeupdown_ganglia_clusterlist_complete_loading)(nodeupdown_t);
typedef int (*Nodeupdown_ganglia_clusterlist_compare_to_clusterlist)(nodeupdown_t);
typedef int (*Nodeupdown_ganglia_clusterlist_is_node_in_cluster)(nodeupdown_t, const char *);
typedef int (*Nodeupdown_ganglia_clusterlist_is_node_discovered)(nodeupdown_t, const char *);
typedef int (*Nodeupdown_ganglia_clusterlist_get_nodename)(nodeupdown_t, const char *, char *, int);
typedef int (*Nodeupdown_ganglia_clusterlist_increase_max_nodes)(nodeupdown_t);

/* 
 * Define all module information. 
 */

struct nodeupdown_ganglia_clusterlist_module_info
{
  char *ganglia_clusterlist_module_name;
  Nodeupdown_ganglia_clusterlist_init init;
  Nodeupdown_ganglia_clusterlist_cleanup cleanup;
  Nodeupdown_ganglia_clusterlist_complete_loading complete_loading;
  Nodeupdown_ganglia_clusterlist_compare_to_clusterlist compare_to_clusterlist;
  Nodeupdown_ganglia_clusterlist_is_node_in_cluster is_node_in_cluster;
  Nodeupdown_ganglia_clusterlist_is_node_discovered is_node_discovered;
  Nodeupdown_ganglia_clusterlist_get_nodename get_nodename;
  Nodeupdown_ganglia_clusterlist_increase_max_nodes increase_max_nodes;
};

/*
 * Load the clusterlist module
 */
int nodeupdown_ganglia_clusterlist_load_module(nodeupdown_t handle, char *clusterlist_module);

/*  
 * Unload the clusterlist module
 */
int nodeupdown_ganglia_clusterlist_unload_module(nodeupdown_t handle);

/* 
 * Initialize any clusterlist info, for example, loading data from a file 
 */
int nodeupdown_ganglia_clusterlist_init(nodeupdown_t handle);

/* 
 * cleanup up clusterlist
 */
int nodeupdown_ganglia_clusterlist_cleanup(nodeupdown_t handle);

/* 
 * complete_loading up clusterlist work
 */
int nodeupdown_ganglia_clusterlist_complete_loading(nodeupdown_t handle);

/* 
 * Compare all nodes retrieved with nodes from the clusterlist 
 * - Adds nodes not found from gmond into the down nodes hostlist
 */
int nodeupdown_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle);

/* 
 * Returns 1 if the specified node is in the cluster, 0 if not, -1 on error 
 */
int nodeupdown_ganglia_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node);

/* 
 * Returns 1 if the specified node name is discovered
 * - Usually identical to nodeupdown_ganglia_clusterlist_is_node_in_cluster.
 *   Only necessary when no clusterlist is available.
 */
int nodeupdown_ganglia_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node);

/* 
 * Returns the appropriate nodename to use in the specified buffer
 * - Used if multiple node names may be used to specify a single node
 *   and node names need to be resolved to a common one.  Most of the time
 *   this just copies the nodename straight into the buffer.
 * - Typically, this should be called after a check using
 *   "is_node_discovered" or "is_node_in_cluster".
 */
int nodeupdown_ganglia_clusterlist_get_nodename(nodeupdown_t handle, const char *node, char *buffer, int buflen);

/* 
 * Increase the number of nodes in the system found 
 */
int nodeupdown_ganglia_clusterlist_increase_max_nodes(nodeupdown_t handle);

#endif /* _NODEUPDOWN_GANGLIA_CLUSTERLIST_H */
