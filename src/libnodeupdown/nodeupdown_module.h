/*****************************************************************************\
 *  $Id: nodeupdown_module.h,v 1.2 2005-05-05 21:36:34 achu Exp $
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

#ifndef _NODEUPDOWN_MODULE_H
#define _NODEUPDOWN_MODULE_H

#include "nodeupdown.h"

/*
 * nodeupdown_backend_load_module
 *
 * Find and load the nodeupdown backend module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_backend_load_module(nodeupdown_t handle);
 
/*
 * nodeupdown_backend_unload_module
 *
 * unload the nodeupdown backend module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_backend_unload_module(nodeupdown_t handle);

/* 
 * nodeupdown_backend_default_hostname
 *
 * call backend module default_hostname function
 */
char *nodeupdown_backend_default_hostname(nodeupdown_t handle);

/* 
 * nodeupdown_backend_default_port
 *
 * call backend module default_port function
 */
int nodeupdown_backend_default_port(nodeupdown_t handle);

/* 
 * nodeupdown_backend_default_timeout_len
 *
 * call backend module default_timeout_len function
 */
int nodeupdown_backend_default_timeout_len(nodeupdown_t handle);

/* 
 * nodeupdown_backend_setup
 *
 * call backend module setup function
 */
int nodeupdown_backend_setup(nodeupdown_t handle);

/* 
 * nodeupdown_backend_cleanup
 *
 * call backend module cleanup function
 */
int nodeupdown_backend_cleanup(nodeupdown_t handle);

/* 
 * nodeupdown_backend_get_updown_data
 *
 * call backend module get_updown_data function
 */
int nodeupdown_backend_get_updown_data(nodeupdown_t handle, 
                                       const char *hostname,
                                       int port,
                                       int timeout_len,
                                       char *reserved);

/*
 * nodeupdown_clusterlist_load_module
 *
 * Find and load the nodeupdown clusterlist module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_clusterlist_load_module(nodeupdown_t handle);
 
/*
 * nodeupdown_clusterlist_unload_module
 *
 * unload the nodeupdown clusterlist module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_clusterlist_unload_module(nodeupdown_t handle);

/* 
 * nodeupdown_clusterlist_setup
 *
 * call clusterlist module setup function
 */
int nodeupdown_clusterlist_setup(nodeupdown_t handle);

/* 
 * nodeupdown_clusterlist_cleanup
 *
 * call clusterlist module cleanup function
 */
int nodeupdown_clusterlist_cleanup(nodeupdown_t handle);

/* 
 * nodeupdown_clusterlist_get_numnodes
 *
 * call clusterlist module get_numnodes function
 */
int nodeupdown_clusterlist_get_numnodes(nodeupdown_t handle);

/* 
 * nodeupdown_clusterlist_is_node_in_cluster
 *
 * call clusterlist module is_node_in_cluster function
 */
int nodeupdown_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node);

/* 
 * nodeupdown_clusterlist_is_node_discovered
 *
 * call clusterlist module is_node_discovered function
 */
int nodeupdown_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node);

/* 
 * nodeupdown_clusterlist_get_nodename
 *
 * call clusterlist module get_nodename function
 */
int nodeupdown_clusterlist_get_nodename(nodeupdown_t handle, const char *node, char *buffer, int buflen);

/* 
 * nodeupdown_clusterlist_compare_to_clusterlist
 *
 * call clusterlist module compare_to_clusterlist function
 */
int nodeupdown_clusterlist_compare_to_clusterlist(nodeupdown_t handle);

/*
 * nodeupdown_config_load_module
 *
 * Find and load the nodeupdown config module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_config_load_module(nodeupdown_t handle);
 
/*
 * nodeupdown_config_unload_module
 *
 * unload the nodeupdown config module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_config_unload_module(nodeupdown_t handle);

/*
 * nodeupdown_config_setup
 *
 * call config module setup function
 */
int nodeupdown_config_setup(nodeupdown_t handle);
 
/*
 * nodeupdown_config_cleanup
 *
 * call config module cleanup function
 */
int nodeupdown_config_cleanup(nodeupdown_t handle);
 
/*
 * nodeupdown_config_load_default
 *
 * call config module load_default function
 */
int nodeupdown_config_load_default(nodeupdown_t handle,
				   struct nodeupdown_config *conf);

#endif /* _NODEUPDOWN_MODULE_H */
