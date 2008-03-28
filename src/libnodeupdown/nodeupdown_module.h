/*****************************************************************************\
 *  $Id: nodeupdown_module.h,v 1.17 2008-03-28 17:06:38 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
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

#ifndef _NODEUPDOWN_MODULE_H
#define _NODEUPDOWN_MODULE_H

#include "nodeupdown.h"
#include "nodeupdown/nodeupdown_config.h"

/*
 * backend_module_load
 *
 * Find and load a nodeupdown backend module
 *
 * Returns 0 on success, -1 on error
 */
int backend_module_load(nodeupdown_t handle, char *module);
 
/*
 * backend_module_unload
 *
 * unload the nodeupdown backend module
 *
 * Returns 0 on success, -1 on error
 */
int backend_module_unload(nodeupdown_t handle);

/* 
 * backend_module_name
 *
 * Return pointer to backend module name
 */
char *backend_module_name(nodeupdown_t handle);

/* 
 * backend_module_default_hostname
 *
 * call backend module default_hostname function
 */
char *backend_module_default_hostname(nodeupdown_t handle);

/* 
 * backend_module_default_port
 *
 * call backend module default_port function
 */
int backend_module_default_port(nodeupdown_t handle);

/* 
 * backend_module_default_timeout_len
 *
 * call backend module default_timeout_len function
 */
int backend_module_default_timeout_len(nodeupdown_t handle);

/* 
 * backend_module_preferred_clusterlist_module
 *
 * call backend module preferred_clusterlist_module function
 */
char *backend_module_preferred_clusterlist_module(nodeupdown_t handle);

/* 
 * backend_module_setup
 *
 * call backend module setup function
 */
int backend_module_setup(nodeupdown_t handle);

/* 
 * backend_module_cleanup
 *
 * call backend module cleanup function
 */
int backend_module_cleanup(nodeupdown_t handle);

/* 
 * backend_module_get_updown_data
 *
 * call backend module get_updown_data function
 */
int backend_module_get_updown_data(nodeupdown_t handle, 
                                   const char *hostname,
                                   unsigned int port,
                                   unsigned int timeout_len,
                                   char *reserved);

/*
 * clusterlist_module_load
 *
 * Find and load a nodeupdown clusterlist module.  If none is
 * found a default one will still be loaded.
 *
 * Returns 0 on success, -1 on error
 */
int clusterlist_module_load(nodeupdown_t handle, char *module);
 
/*
 * clusterlist_module_unload
 *
 * unload the nodeupdown clusterlist module
 *
 * Returns 0 on success, -1 on error
 */
int clusterlist_module_unload(nodeupdown_t handle);

/* 
 * clusterlist_module_name
 *
 * Return pointer to clusterlist module name
 */
char *clusterlist_module_name(nodeupdown_t handle);

/* 
 * clusterlist_module_setup
 *
 * call clusterlist module setup function
 */
int clusterlist_module_setup(nodeupdown_t handle);

/* 
 * clusterlist_module_cleanup
 *
 * call clusterlist module cleanup function
 */
int clusterlist_module_cleanup(nodeupdown_t handle);

/* 
 * clusterlist_module_get_numnodes
 *
 * call clusterlist module get_numnodes function
 */
int clusterlist_module_get_numnodes(nodeupdown_t handle);

/* 
 * clusterlist_module_is_node_in_cluster
 *
 * call clusterlist module is_node_in_cluster function
 */
int clusterlist_module_is_node_in_cluster(nodeupdown_t handle, const char *node);

/* 
 * clusterlist_module_get_nodename
 *
 * call clusterlist module get_nodename function
 */
int clusterlist_module_get_nodename(nodeupdown_t handle, 
				    const char *node, 
				    char *buffer, 
				    unsigned int buflen);

/* 
 * clusterlist_module_compare_to_clusterlist
 *
 * call clusterlist module compare_to_clusterlist function
 */
int clusterlist_module_compare_to_clusterlist(nodeupdown_t handle);

/*
 * config_module_load
 *
 * Find and load a nodeupdown config module.  If none is found a
 * default one will still be loaded.
 *
 * Returns 0 on success, -1 on error
 */
int config_module_load(nodeupdown_t handle);
 
/*
 * config_module_unload
 *
 * unload the nodeupdown config module
 *
 * Returns 0 on success, -1 on error
 */
int config_module_unload(nodeupdown_t handle);

/* 
 * config_module_name
 *
 * Return pointer to config module name
 */
char *config_module_name(nodeupdown_t handle);

/*
 * config_module_setup
 *
 * call config module setup function
 */
int config_module_setup(nodeupdown_t handle);
 
/*
 * config_module_cleanup
 *
 * call config module cleanup function
 */
int config_module_cleanup(nodeupdown_t handle);
 
/*
 * config_module_load_config
 *
 * call config module load_config function
 */
int config_module_load_config(nodeupdown_t handle,
			      struct nodeupdown_config *conf);

#endif /* _NODEUPDOWN_MODULE_H */
