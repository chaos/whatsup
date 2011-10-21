/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_genders.c,v 1.30 2010-02-02 00:01:58 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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
#include <genders.h>

#include "nodeupdown.h"
#include "nodeupdown_clusterlist_util.h"
#include "nodeupdown_clusterlist_genders_util.h"
#include "nodeupdown_genders_util.h"
#include "nodeupdown/nodeupdown_clusterlist_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

static genders_t genders_handle = NULL;

/*
 * genders_clusterlist_setup
 *
 * genders clusterlist module setup function
 */
static int 
genders_clusterlist_setup(nodeupdown_t handle) 
{
  int rv;

  rv = genders_util_setup(handle, &genders_handle, NULL);
  if (rv < 0)
    nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
  return rv;
}

/*
 * genders_clusterlist_cleanup
 *
 * genders clusterlist module cleanup function
 */
static int 
genders_clusterlist_cleanup(nodeupdown_t handle) 
{
  return genders_util_cleanup(handle, &genders_handle);
}

/*
 * genders_clusterlist_get_numnodes
 *
 * genders clusterlist module get_numnodes function
 */
static int 
genders_clusterlist_get_numnodes(nodeupdown_t handle) 
{
  return genders_util_clusterlist_get_numnodes(handle, genders_handle);
}

/*
 * genders_clusterlist_is_node_in_cluster
 *
 * genders clusterlist module is_node_in_cluster function
 */
static int 
genders_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
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

  if ((rv = genders_isnode(genders_handle, nodePtr)) < 0) 
    {
#ifndef NDEBUG
      fprintf(stderr, "genders_isnode: %s\n", 
              genders_strerror(genders_errnum(genders_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      return -1;
    }
  return rv;
}

/*
 * genders_clusterlist_get_nodename
 *
 * genders clusterlist module get_nodename function
 */
static int 
genders_clusterlist_get_nodename(nodeupdown_t handle, 
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

  return _nodeupdown_clusterlist_copy_nodename(handle, nodePtr, buf, buflen);
}
    
/*
 * genders_clusterlist_compare_to_clusterlist
 *
 * genders clusterlist module compare_to_clusterlist function
 */
static int 
genders_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  return genders_util_clusterlist_compare_to_clusterlist(handle, genders_handle);
}

struct nodeupdown_clusterlist_module_info clusterlist_module_info =
  {
    "genders",
    &genders_clusterlist_setup,
    &genders_clusterlist_cleanup,
    &genders_clusterlist_get_numnodes,
    &genders_clusterlist_is_node_in_cluster,
    &genders_clusterlist_get_nodename,
    &genders_clusterlist_compare_to_clusterlist,
  };
