/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist_genders_util.c,v 1.2 2005-04-01 17:59:01 achu Exp $
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
#include "hostlist.h"

int 
genders_util_ganglia_clusterlist_init(nodeupdown_t handle,
                                      genders_t *genders_handle,
                                      char *genders_file)
{
  char *file = NULL;

  if (strlen(genders_file))
    file = genders_file;

  if (!(*genders_handle = genders_handle_create()))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

  if (genders_load_data(*genders_handle, file) < 0)
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_OPEN;
      goto cleanup;
    }

  return 0;

 cleanup:
  genders_handle_destroy(*genders_handle);
  *genders_handle = NULL;
  return -1;
}

int 
genders_util_ganglia_clusterlist_finish(nodeupdown_t handle, 
                                        genders_t genders_handle) 
{
  handle->max_nodes = genders_getnumnodes(genders_handle);
  return 0;
}

int 
genders_util_ganglia_clusterlist_cleanup(nodeupdown_t handle,
                                         genders_t *genders_handle) 
{
  genders_handle_destroy(*genders_handle);
  *genders_handle = NULL;
  return 0;
}

int 
genders_util_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle,
                                                        genders_t genders_handle) 
{
  int i, num;
  char **nlist = NULL;
 
  /* get all genders nodes */
  if ((num = genders_nodelist_create(genders_handle, &nlist)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      goto cleanup;
    }
   
  if (genders_getnodes(genders_handle, nlist, num, NULL, NULL) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
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
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      goto cleanup;
    }
 
  hostlist_sort(handle->down_nodes);
  return 0;
 
 cleanup:
  (void)genders_nodelist_destroy(genders_handle, nlist);
  return -1;
}
