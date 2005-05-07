/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_gendersllnl.c,v 1.22 2005-05-07 17:34:42 achu Exp $
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
#include "nodeupdown_clusterlist_genders_util.h"
#include "nodeupdown_genders_util.h"
#include "nodeupdown/nodeupdown_clusterlist_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

static genders_t gendersllnl_handle = NULL;

/* 
 * gendersllnl_clusterlist_setup
 *
 * gendersllnl clusterlist module setup function
 */
int 
gendersllnl_clusterlist_setup(nodeupdown_t handle) 
{
  int rv;

  rv = genders_util_setup(handle, &gendersllnl_handle);

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

  if (rv < 0)
    nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);

  return rv;
}

/* 
 * gendersllnl_clusterlist_cleanup
 *
 * gendersllnl clusterlist module cleanup function
 */
int 
gendersllnl_clusterlist_cleanup(nodeupdown_t handle) 
{
  return genders_util_cleanup(handle, &gendersllnl_handle);
}

/* 
 * gendersllnl_clusterlist_get_numnodes
 *
 * gendersllnl clusterlist module get_numnodes function
 */
int 
gendersllnl_clusterlist_get_numnodes(nodeupdown_t handle) 
{
  return genders_util_clusterlist_get_numnodes(handle, gendersllnl_handle);
}

/* 
 * gendersllnl_clusterlist_is_node_in_cluster
 *
 * gendersllnl clusterlist module is_node_in_cluster function
 */
int 
gendersllnl_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  char nodebuf[NODEUPDOWN_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  int rv;

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
 
      memset(nodebuf, '\0', NODEUPDOWN_MAXNODENAMELEN+1);
      strncpy(nodebuf, node, NODEUPDOWN_MAXNODENAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;
  
  if ((rv = genders_isnode_or_altnode(gendersllnl_handle, nodePtr)) < 0) 
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      return -1;
    }
  return rv;
}

/* 
 * gendersllnl_clusterlist_get_nodename
 *
 * gendersllnl clusterlist module get_nodename function
 */
int 
gendersllnl_clusterlist_get_nodename(nodeupdown_t handle, 
				     const char *node, 
				     char *buf, 
				     unsigned int buflen) 
{
  char nodebuf[NODEUPDOWN_MAXNODENAMELEN+1];
  char *nodePtr = NULL;

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
 
      memset(nodebuf, '\0', NODEUPDOWN_MAXNODENAMELEN+1);
      strncpy(nodebuf, node, NODEUPDOWN_MAXNODENAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  if (genders_to_gendname(gendersllnl_handle, nodePtr, buf, buflen) < 0) 
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      return -1;
    }
  return 0;
}
    
/* 
 * gendersllnl_clusterlist_compare_to_clusterlist
 *
 * gendersllnl clusterlist module compare_to_clusterlist function
 */
int 
gendersllnl_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  return genders_util_clusterlist_compare_to_clusterlist(handle, gendersllnl_handle);
}

#if WITH_STATIC_MODULES
struct nodeupdown_clusterlist_module_info gendersllnl_clusterlist_module_info =
#else  /* !WITH_STATIC_MODULES */
struct nodeupdown_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    "gendersllnl",
    &gendersllnl_clusterlist_setup,
    &gendersllnl_clusterlist_cleanup,
    &gendersllnl_clusterlist_get_numnodes,
    &gendersllnl_clusterlist_is_node_in_cluster,
    &gendersllnl_clusterlist_get_nodename,
    &gendersllnl_clusterlist_compare_to_clusterlist,
  };
