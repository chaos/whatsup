/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist_gendersllnl.c,v 1.1 2005-03-31 22:44:22 achu Exp $
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
#include <gendersllnl.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_ganglia_clusterlist.h"

static genders_t gendersllnl_handle = NULL;
static char *gendersllnl_file = NULL;

int
gendersllnl_ganglia_clusterlist_parse_options(char **options)
{
  return 0;
}

static int
_load_gendersllnl_data(nodeupdown_t handle)
{
  int numnodes;
  char *file = NULL;

  if (gendersllnl_file)
    file = gendersllnl_file;

  if (!(gendersllnl_handle = genders_handle_create())) 
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
 
  if (genders_load_data(gendersllnl_handle, file) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST_OPEN;
      return -1;
    }
 
#if HAVE_GENDERS_INDEX_ATTRVALS
  /* This is for performance improvements if the indexing API
   * functions are available.  Don't fail and return -1, since the
   * rest of libnodeupdown is not dependent on this section of code.
   */
  genders_index_attrvals(gendersllnl_handle, GENDERS_ALTNAME_ATTRIBUTE);
#endif /* HAVE_GENDERS_INDEX_ATTRVALS */
 
  return 0;

 cleanup:
  genders_handle_destroy(gendersllnl_handle);
  gendersllnl_handle = NULL;
  return -1;
}

int 
gendersllnl_ganglia_clusterlist_init(nodeupdown_t handle, void *ptr) 
{
  return _load_gendersllnl_data(handle);
}

int 
gendersllnl_ganglia_clusterlist_finish(nodeupdown_t handle) 
{
  handle->max_nodes = genders_numnodes(gendersllnl_handle);
  return 0;
}

int 
gendersllnl_ganglia_clusterlist_cleanup(nodeupdown_t handle) 
{
  genders_handle_destroy(gendersllnl_handle);
  gendersllnl_handle = NULL;
  return 0;
}

int 
gendersllnl_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  int i, ret, num;
  char **nlist = NULL;
 
  if ((num = genders_nodelist_create(gendersllnl_handle, &nlist)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      goto cleanup;
    }
   
  if (genders_getnodes(gendersllnl_handle, nlist, num, NULL, NULL) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      goto cleanup;
    }
   
  for (i = 0; i < num; i++) 
    {
      if ((hostlist_find(handle->up_nodes, nlist[i]) < 0)
          && (hostlist_find(handle->down_nodes, nlist[i]) < 0)) 
        {
          
          /* This node must also be down */
          if (hostlist_push_host(handle->down_nodes, nlist[i]) == 0) 
            {
              handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
              goto cleanup;
            }
        }
    }
 
  if (genders_nodelist_destroy(gendersllnl_handle, nlist) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      goto cleanup;
    }
 
  hostlist_sort(handle->down_nodes);
  return 0;
 
 cleanup:
  (void)genders_nodelist_destroy(gendersllnl_handle, nlist);
  return -1;
}

int 
gendersllnl_ganglia_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  int ret;
  if ((ret = genders_isnode_or_altnode(handle->gendersllnl_handle, node)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      return -1;
    }
  return ret;
}

int 
gendersllnl_ganglia_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node) 
{
  int ret;
  if ((ret = genders_isnode_or_altnode(handle->gendersllnl_handle, node)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
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
  if (genders_to_gendname(handle->genders_handle, node, buffer, buflen) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      return -1;
    }
  return 0;
}
    
int 
gendersllnl_ganglia_clusterlist_increase_max_nodes(nodeupdown_t handle) 
{
  handle->max_nodes++;
  return 0;
}

struct nodeupdown_ganglia_clusterlist_module_info ganglia_clusterlist_module_info =
  {
    "gendersllnl";
    &gendersllnl_ganglia_clusterlist_parse_options;
    &gendersllnl_ganglia_clusterlist_init;
    &gendersllnl_ganglia_clusterlist_finish;
    &gendersllnl_ganglia_clusterlist_cleanup;
    &gendersllnl_ganglia_clusterlist_compare_to_clusterlist;
    &gendersllnl_ganglia_clusterlist_is_node_in_cluster;
    &gendersllnl_ganglia_clusterlist_is_node_discovered;
    &gendersllnl_ganglia_clusterlist_get_nodename;
  };
