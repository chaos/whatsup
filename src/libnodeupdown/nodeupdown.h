/*****************************************************************************\
 *  $Id: nodeupdown.h,v 1.44 2008-03-28 17:06:37 chu11 Exp $
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

#ifndef _NODEUPDOWN_H
#define _NODEUPDOWN_H

#include <stdio.h>

/*
 * Nodeupdown Error Codes
 */

#define NODEUPDOWN_ERR_SUCCESS               0
#define NODEUPDOWN_ERR_NULLHANDLE            1
#define NODEUPDOWN_ERR_CONNECT               2
#define NODEUPDOWN_ERR_CONNECT_TIMEOUT       3
#define NODEUPDOWN_ERR_HOSTNAME              4
#define NODEUPDOWN_ERR_ISLOADED              5
#define NODEUPDOWN_ERR_NOTLOADED             6
#define NODEUPDOWN_ERR_OVERFLOW              7
#define NODEUPDOWN_ERR_PARAMETERS            8
#define NODEUPDOWN_ERR_NULLPTR               9
#define NODEUPDOWN_ERR_OUTMEM               10
#define NODEUPDOWN_ERR_NOTFOUND             11
#define NODEUPDOWN_ERR_NOTSUPPORTED         12
#define NODEUPDOWN_ERR_BACKEND_MODULE       13
#define NODEUPDOWN_ERR_CLUSTERLIST_MODULE   14
#define NODEUPDOWN_ERR_CONFIG_MODULE        15
#define NODEUPDOWN_ERR_CONF_PARSE           16
#define NODEUPDOWN_ERR_CONF_INPUT           17
#define NODEUPDOWN_ERR_CONF_INTERNAL        18
#define NODEUPDOWN_ERR_MAGIC                19
#define NODEUPDOWN_ERR_INTERNAL             20
#define NODEUPDOWN_ERR_ERRNUMRANGE          21

typedef struct nodeupdown *nodeupdown_t;

/* 
 * nodeupdown_handle_create
 * 
 * Create a nodeupdown handle.
 *
 * Returns handle on success, NULL on error
 */
nodeupdown_t nodeupdown_handle_create(void);

/* nodeupdown_handle_destroy
 *
 * Destroy a nodeupdown handle.
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_handle_destroy(nodeupdown_t handle);

/* 
 * nodeupdown_load_data
 *
 * Loads data from the backend tool used to evaluate up and down
 * nodes.
 *
 * If 'hostname' is NULL, 'port' is <= 0, or 'timeout_len' <=0, the
 * backend tool's respective defaults will be used.
 *
 * 'module' can be used to select a specific backend module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_load_data(nodeupdown_t handle, 
                         const char *hostname, 
                         int port,
                         int timeout_len,
                         char *module); 

/* 
 * nodeupdown_errnum
 *
 * Return the most recent error number.
 *
 * Returns error number on success
 */
int nodeupdown_errnum(nodeupdown_t handle);

/* 
 * nodeupdown_strerror
 *
 * Return a string message describing an error number.
 *
 * Returns pointer to message on success
 */
char *nodeupdown_strerror(int errnum);

/* 
 * nodeupdown_errormsg
 * 
 * Return a string message describing the most recent error.
 *
 * Returns pointer to message on success
 */
char *nodeupdown_errormsg(nodeupdown_t handle);

/* 
 * nodeupdown_perror
 *
 * Output a message to standard error 
 */
void nodeupdown_perror(nodeupdown_t handle, const char *msg);

/* 
 * nodeupdown_get_up_nodes_string
 *
 * Retrieve a ranged string of up nodes and store it in the buffer
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_get_up_nodes_string(nodeupdown_t handle, 
                                   char *buf, 
                                   int buflen);

/* 
 * nodeupdown_get_down_nodes_string
 *
 * Retrieve a ranged string of down nodes and store it in the buffer
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_get_down_nodes_string(nodeupdown_t handle, 
                                     char *buf, 
                                     int buflen);

/* 
 * nodeupdown_get_up_nodes_list
 *
 * Retrieve a list of up nodes and store them in the list
 *
 * Returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list, int len);

/* 
 * nodeupdown_get_down_nodes_list
 *
 * Retrieve a list of down nodes and store them in the list
 *
 * Returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list, int len);

/* 
 * nodeupdown_is_node_up
 *
 * Check if a node is up
 *
 * Returns 1 if up, 0 if down, -1 on error
 */
int nodeupdown_is_node_up(nodeupdown_t handle, const char *node);

/* 
 * nodeupdown_is_node_down
 *
 * Check if a node is down
 *
 * Returns 1 if down, 0 if up, -1 on error
 */
int nodeupdown_is_node_down(nodeupdown_t handle, const char *node);

/* 
 * nodeupdown_up_count
 *
 * Returns number of nodes that are up, -1 on error
 */
int nodeupdown_up_count(nodeupdown_t handle);

/* 
 * nodeupdown_down_count
 *
 * Returns number of nodes that are down, -1 on error
 */
int nodeupdown_down_count(nodeupdown_t handle);

/*
 * nodeupdown_last_up_time
 *
 * Retrieve time since epoch of the last known up time for a node.
 * May not be available from all backend modules supported by
 * nodeupdown.
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_last_up_time(nodeupdown_t handle, 
                            const char *node,
                            unsigned int *last_up_time);

/* 
 * nodeupdown_nodelist_create
 *
 * Allocate an array to store node names in
 *
 * Returns number of node entries created on success, -1 on error
 */
int nodeupdown_nodelist_create(nodeupdown_t handle, char ***list);

/* 
 * nodeupdown_nodelist_clear
 *
 * Clear a previously allocated nodelist
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_nodelist_clear(nodeupdown_t handle, char **list);

/* 
 * nodeupdown_nodelist_destroy
 *
 * Destroy a previously allocated nodelist
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_nodelist_destroy(nodeupdown_t handle, char **list);

#endif /* _NODEUPDOWN_H */
