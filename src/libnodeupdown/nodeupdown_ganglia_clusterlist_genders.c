/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist_genders.c,v 1.5 2005-04-01 21:29:02 achu Exp $
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <genders.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_ganglia_clusterlist.h"
#include "nodeupdown_ganglia_clusterlist_util.h"
#include "nodeupdown_ganglia_clusterlist_genders_util.h"
#include "hostlist.h"

static genders_t genders_handle = NULL;

int 
genders_ganglia_clusterlist_init(nodeupdown_t handle, void *ptr) 
{
  return genders_util_ganglia_clusterlist_init(handle, &genders_handle);
}

int 
genders_ganglia_clusterlist_finish(nodeupdown_t handle) 
{
  return genders_util_ganglia_clusterlist_finish(handle, genders_handle);
}

int 
genders_ganglia_clusterlist_cleanup(nodeupdown_t handle) 
{
  return genders_util_ganglia_clusterlist_cleanup(handle, &genders_handle);
}

int 
genders_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  return genders_util_ganglia_clusterlist_compare_to_clusterlist(handle, genders_handle);
}

int 
genders_ganglia_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  int ret;
  if ((ret = genders_isnode(genders_handle, node)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      return -1;
    }
  return ret;
}

int 
genders_ganglia_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node) 
{
  int ret;
  if ((ret = genders_isnode(genders_handle, node)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      return -1;
    }
  return ret;
}

int 
genders_ganglia_clusterlist_get_nodename(nodeupdown_t handle, 
                                         const char *node, 
                                         char *buffer, 
                                         int buflen) 
{
  return nodeupdown_ganglia_clusterlist_copy_nodename(handle, node, buffer, buflen);
}
    
int 
genders_ganglia_clusterlist_increase_max_nodes(nodeupdown_t handle) 
{
  return 0;
}

struct nodeupdown_ganglia_clusterlist_module_info ganglia_clusterlist_module_info =
  {
    "genders",
    &genders_ganglia_clusterlist_init,
    &genders_ganglia_clusterlist_finish,
    &genders_ganglia_clusterlist_cleanup,
    &genders_ganglia_clusterlist_compare_to_clusterlist,
    &genders_ganglia_clusterlist_is_node_in_cluster,
    &genders_ganglia_clusterlist_is_node_discovered,
    &genders_ganglia_clusterlist_get_nodename,
    &genders_ganglia_clusterlist_increase_max_nodes,
  };
