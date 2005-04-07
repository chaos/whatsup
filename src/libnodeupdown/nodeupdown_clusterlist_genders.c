/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_genders.c,v 1.2 2005-04-06 21:50:19 achu Exp $
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
#include <genders.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_clusterlist.h"
#include "nodeupdown_clusterlist_util.h"
#include "nodeupdown_clusterlist_genders_util.h"

static genders_t genders_handle = NULL;

/*
 * genders_clusterlist_init
 *
 * genders clusterlist module init function
 */
int 
genders_clusterlist_init(nodeupdown_t handle) 
{
  return genders_util_clusterlist_init(handle, &genders_handle);
}

/*
 * genders_clusterlist_cleanup
 *
 * genders clusterlist module cleanup function
 */
int 
genders_clusterlist_cleanup(nodeupdown_t handle) 
{
  return genders_util_clusterlist_cleanup(handle, &genders_handle);
}

/*
 * genders_clusterlist_complete_loading
 *
 * genders clusterlist module complete_loading function
 */
int 
genders_clusterlist_complete_loading(nodeupdown_t handle) 
{
  return genders_util_clusterlist_complete_loading(handle, genders_handle);
}

/*
 * genders_clusterlist_compare_to_clusterlist
 *
 * genders clusterlist module compare_to_clusterlist function
 */
int 
genders_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  return genders_util_clusterlist_compare_to_clusterlist(handle, genders_handle);
}

/*
 * genders_clusterlist_is_node_in_cluster
 *
 * genders clusterlist module is_node_in_cluster function
 */
int 
genders_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  int ret;
  char nodebuf[NODEUPDOWN_MAXHOSTNAMELEN+1];
  char *nodePtr = NULL;

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
 
      memset(nodebuf, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
      strncpy(nodebuf, node, NODEUPDOWN_MAXHOSTNAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  if ((ret = genders_isnode(genders_handle, nodePtr)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_INTERNAL;
      return -1;
    }
  return ret;
}

/*
 * genders_clusterlist_is_node_discovered
 *
 * genders clusterlist module is_node_discovered function
 */
int 
genders_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node) 
{
  int ret;
  char nodebuf[NODEUPDOWN_MAXHOSTNAMELEN+1];
  char *nodePtr = NULL;

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
 
      memset(nodebuf, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
      strncpy(nodebuf, node, NODEUPDOWN_MAXHOSTNAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  if ((ret = genders_isnode(genders_handle, nodePtr)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_INTERNAL;
      return -1;
    }
  return ret;
}

/*
 * genders_clusterlist_get_nodename
 *
 * genders clusterlist module get_nodename function
 */
int 
genders_clusterlist_get_nodename(nodeupdown_t handle, 
                                         const char *node, 
                                         char *buffer, 
                                         int buflen) 
{
  char nodebuf[NODEUPDOWN_MAXHOSTNAMELEN+1];
  char *nodePtr = NULL;

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
 
      memset(nodebuf, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
      strncpy(nodebuf, node, NODEUPDOWN_MAXHOSTNAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  return nodeupdown_clusterlist_copy_nodename(handle, nodePtr, buffer, buflen);
}
    
/*
 * genders_clusterlist_increase_max_nodes
 *
 * genders clusterlist module increase_max_nodes function
 */
int 
genders_clusterlist_increase_max_nodes(nodeupdown_t handle) 
{
  return 0;
}

struct nodeupdown_clusterlist_module_info clusterlist_module_info =
  {
    "genders",
    &genders_clusterlist_init,
    &genders_clusterlist_cleanup,
    &genders_clusterlist_complete_loading,
    &genders_clusterlist_compare_to_clusterlist,
    &genders_clusterlist_is_node_in_cluster,
    &genders_clusterlist_is_node_discovered,
    &genders_clusterlist_get_nodename,
    &genders_clusterlist_increase_max_nodes,
  };