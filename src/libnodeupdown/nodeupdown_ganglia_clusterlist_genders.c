/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist_genders.c,v 1.1 2005-04-01 00:56:53 achu Exp $
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
#include "hostlist.h"

static genders_t genders_handle = NULL;
static char genders_file[MAXPATHLEN+1];

int
genders_ganglia_clusterlist_parse_options(nodeupdown_t handle, char **options)
{
  return nodeupdown_ganglia_clusterlist_parse_filename(handle, options, genders_file, MAXPATHLEN);
}

static int
_load_genders_data(nodeupdown_t handle)
{
  char *file = NULL;

  if (genders_file)
    file = genders_file;

  if (!(genders_handle = genders_handle_create())) 
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
 
  if (genders_load_data(genders_handle, file) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST_OPEN;
      goto cleanup;
    }
 
  return 0;

 cleanup:
  genders_handle_destroy(genders_handle);
  genders_handle = NULL;
  return -1;
}

int 
genders_ganglia_clusterlist_init(nodeupdown_t handle, void *ptr) 
{
  return _load_genders_data(handle);
}

int 
genders_ganglia_clusterlist_finish(nodeupdown_t handle) 
{
  handle->max_nodes = genders_getnumnodes(genders_handle);
  return 0;
}

int 
genders_ganglia_clusterlist_cleanup(nodeupdown_t handle) 
{
  genders_handle_destroy(genders_handle);
  genders_handle = NULL;
  return 0;
}

int 
genders_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  int i, num;
  char **nlist = NULL;
 
  /* get all genders nodes */
  if ((num = genders_nodelist_create(genders_handle, &nlist)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      goto cleanup;
    }
   
  if (genders_getnodes(genders_handle, nlist, num, NULL, NULL) < 0) 
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
 
  if (genders_nodelist_destroy(genders_handle, nlist) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      goto cleanup;
    }
 
  hostlist_sort(handle->down_nodes);
  return 0;
 
 cleanup:
  (void)genders_nodelist_destroy(genders_handle, nlist);
  return -1;
}

int 
genders_ganglia_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  int ret;
  if ((ret = genders_isnode(genders_handle, node)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
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
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
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
  handle->max_nodes++;
  return 0;
}

struct nodeupdown_ganglia_clusterlist_module_info ganglia_clusterlist_module_info =
  {
    "genders",
    &genders_ganglia_clusterlist_parse_options,
    &genders_ganglia_clusterlist_init,
    &genders_ganglia_clusterlist_finish,
    &genders_ganglia_clusterlist_cleanup,
    &genders_ganglia_clusterlist_compare_to_clusterlist,
    &genders_ganglia_clusterlist_is_node_in_cluster,
    &genders_ganglia_clusterlist_is_node_discovered,
    &genders_ganglia_clusterlist_get_nodename,
  };
