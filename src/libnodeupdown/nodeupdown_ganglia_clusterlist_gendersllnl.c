/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist_gendersllnl.c,v 1.8 2005-04-02 00:57:01 achu Exp $
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
#include <gendersllnl.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_ganglia_clusterlist.h"
#include "nodeupdown_ganglia_clusterlist_util.h"
#include "nodeupdown_ganglia_clusterlist_genders_util.h"

static genders_t gendersllnl_handle = NULL;

int 
gendersllnl_ganglia_clusterlist_init(nodeupdown_t handle) 
{
  int rv;

  rv = genders_util_ganglia_clusterlist_init(handle, &gendersllnl_handle);

#if HAVE_GENDERS_INDEX_ATTRVALS
  if (!rv)
    {
      /* This is for performance improvements if the indexing API
       * functions are available.  Don't fail and return -1, since the
       * rest of libnodeupdown is not dependent on this section of code.
       */
      genders_index_attrvals(gendersllnl_handle, GENDERS_ALTNAME_ATTRIBUTE);
    }
#endif /* HAVE_GENDERS_INDEX_ATTRVALS */

  return rv;
}

int 
gendersllnl_ganglia_clusterlist_finish(nodeupdown_t handle) 
{
  return genders_util_ganglia_clusterlist_finish(handle, gendersllnl_handle);
}

int 
gendersllnl_ganglia_clusterlist_cleanup(nodeupdown_t handle) 
{
  return genders_util_ganglia_clusterlist_cleanup(handle, &gendersllnl_handle);
}

int 
gendersllnl_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  return genders_util_ganglia_clusterlist_compare_to_clusterlist(handle, gendersllnl_handle);
}

int 
gendersllnl_ganglia_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
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

  if ((ret = genders_isnode_or_altnode(gendersllnl_handle, nodePtr)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      return -1;
    }
  return ret;
}

int 
gendersllnl_ganglia_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node) 
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

  if ((ret = genders_isnode_or_altnode(gendersllnl_handle, nodePtr)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      return -1;
    }
  return ret;
}

int 
gendersllnl_ganglia_clusterlist_get_nodename(nodeupdown_t handle, 
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

  if (genders_to_gendname(gendersllnl_handle, nodePtr, buffer, buflen) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      return -1;
    }
  return 0;
}
    
int 
gendersllnl_ganglia_clusterlist_increase_max_nodes(nodeupdown_t handle) 
{
  return 0;
}

struct nodeupdown_ganglia_clusterlist_module_info ganglia_clusterlist_module_info =
  {
    "gendersllnl",
    &gendersllnl_ganglia_clusterlist_init,
    &gendersllnl_ganglia_clusterlist_finish,
    &gendersllnl_ganglia_clusterlist_cleanup,
    &gendersllnl_ganglia_clusterlist_compare_to_clusterlist,
    &gendersllnl_ganglia_clusterlist_is_node_in_cluster,
    &gendersllnl_ganglia_clusterlist_is_node_discovered,
    &gendersllnl_ganglia_clusterlist_get_nodename,
    &gendersllnl_ganglia_clusterlist_increase_max_nodes,
  };
