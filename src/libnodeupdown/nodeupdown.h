/*****************************************************************************\
 *  $Id: nodeupdown.h,v 1.26 2005-03-31 22:44:22 achu Exp $
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

#ifndef _NODEUPDOWN_H
#define _NODEUPDOWN_H

#include <stdio.h>

/**************************************
 * Definitions                        *
 **************************************/

#define NODEUPDOWN_ERR_SUCCESS            0 /* success */
#define NODEUPDOWN_ERR_NULLHANDLE         1 /* nodeupdown handle is null */
#define NODEUPDOWN_ERR_CONNECT            2 /* network connection error */
#define NODEUPDOWN_ERR_TIMEOUT            3 /* network connect timeout */
#define NODEUPDOWN_ERR_HOSTNAME           4 /* hostname invalid */
#define NODEUPDOWN_ERR_ADDRESS            5 /* network address error */
#define NODEUPDOWN_ERR_NETWORK            6 /* network error */
#define NODEUPDOWN_ERR_ISLOADED           7 /* data already loaded */
#define NODEUPDOWN_ERR_NOTLOADED          8 /* data not loaded */
#define NODEUPDOWN_ERR_OVERFLOW           9 /* overflow on list/string passed in */
#define NODEUPDOWN_ERR_PARAMETERS        10 /* incorrect parameters passed in */
#define NODEUPDOWN_ERR_NULLPTR           11 /* null pointer in list */
#define NODEUPDOWN_ERR_OUTMEM            12 /* out of memory */
#define NODEUPDOWN_ERR_NOTFOUND          13 /* node not found */ 
#define NODEUPDOWN_ERR_MASTERLIST        14 /* internal master list error */
#define NODEUPDOWN_ERR_MASTERLIST_OPEN   15 /* open masterlist file error */
#define NODEUPDOWN_ERR_MASTERLIST_READ   16 /* read masterlist file error */
#define NODEUPDOWN_ERR_MASTERLIST_PARSE  17 /* parse masterlist error */ 
#define NODEUPDOWN_ERR_CONF              18 /* internal conf file error */
#define NODEUPDOWN_ERR_CONF_OPEN         19 /* open conf file error */
#define NODEUPDOWN_ERR_CONF_READ         20 /* read conf file error */
#define NODEUPDOWN_ERR_CONF_PARSE        21 /* parse conf file error */
#define NODEUPDOWN_ERR_EXPAT             22 /* internal expat error */
#define NODEUPDOWN_ERR_HOSTLIST          23 /* internal hostlist error */
#define NODEUPDOWN_ERR_MAGIC             24 /* magic number error */
#define NODEUPDOWN_ERR_INTERNAL          25 /* internal system error */
#define NODEUPDOWN_ERR_ERRNUMRANGE       26 /* error number out of range */ 

#define NODEUPDOWN_TIMEOUT_LEN           60

typedef struct nodeupdown *nodeupdown_t;

/* nodeupdown_handle_create
 * - create a nodeupdown handle
 * - returns handle on success, NULL on error
 */
nodeupdown_t nodeupdown_handle_create(void);

/* nodeupdown_handle_destroy
 * - destroy a nodeupdown handle
 * - returns 0 on success, -1 on error
 */
int nodeupdown_handle_destroy(nodeupdown_t handle);

/* nodeupdown_load_data
 * - loads data from ganglia and a msterlist
 * - if hostname is NULL, localhost is assumed.
 * - if port is <= 0, default port is used
 * - if timeout_len is <= 0, default timeout is used
 * - 'reserved' is used for backwards compatability
 * - returns 0 on success, -1 on error
 */
int nodeupdown_load_data(nodeupdown_t handle, 
                         const char *gmond_hostname, 
                         int gmond_port,
                         int timeout_len,
                         char *reserved); 

/* nodeupdown_errnum
 * - return the most recent error number
 * - returns error number on success
 */
int nodeupdown_errnum(nodeupdown_t handle);

/* nodeupdown_strerror
 * - return a string message describing an error number
 * - returns pointer to message on success
 */
char *nodeupdown_strerror(int errnum);

/* nodeupdown_errormsg
 * - return a string message describing the most recently
 *   occurred error
 * - returns pointer to message on success
 */
char *nodeupdown_errormsg(nodeupdown_t handle);

/* nodeupdown_perror
 * - output a message to standard error 
 */
void nodeupdown_perror(nodeupdown_t handle, const char *msg);

/* nodeupdown_get_up_nodes_string
 * - retrieve a ranged string of up nodes
 * - buffer space assumed to be preallocated with length buflen
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_up_nodes_string(nodeupdown_t handle, 
                                   char *buf, int buflen);

/* nodeupdown_get_down_nodes_string
 * - retrieve a ranged string of down nodes
 * - buffer space assumed to be preallocated with length buflen
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_down_nodes_string(nodeupdown_t handle, 
                                     char *buf, int buflen);

/* nodeupdown_get_up_nodes_list
 * - retrieve a list of up nodes
 * - list assumed to be preallocated with len elements
 * - returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list, int len);

/* nodeupdown_get_down_nodes_list
 * - retrieve a list of down nodes
 * - list assumed to be preallocated with len elements
 * - returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list, int len);

/* nodeupdown_is_node_up
 * - check if a node is up
 * - node must be shortened name of hostname
 * - returns 1 if up, 0 if down, -1 on error
 */
int nodeupdown_is_node_up(nodeupdown_t handle, const char *node);

/* nodeupdown_is_node_down
 * - check if a node is down
 * - node must be shortened name of hostname
 * - returns 1 if down, 0 if up, -1 on error
 */
int nodeupdown_is_node_down(nodeupdown_t handle, const char *node);

/* nodeupdown_up_count
 * - returns number of nodes that are up, -1 on error
 */
int nodeupdown_up_count(nodeupdown_t handle);

/* nodeupdown_down_count
 * - returns number of nodes that are down, -1 on error
 */
int nodeupdown_down_count(nodeupdown_t handle);

/* nodeupdown_nodelist_create
 * - allocate an array to store node names in
 * - returns number of node entries created on success, -1 on error
 */
int nodeupdown_nodelist_create(nodeupdown_t handle, char ***list);

/* nodeupdown_nodelist_clear
 * - clear a previously allocated nodelist
 * - returns 0 on success, -1 on error
 */
int nodeupdown_nodelist_clear(nodeupdown_t handle, char **list);

/* nodeupdown_nodelist_destroy
 * - destroy the previously allocated nodelist
 * - returns 0 on success, -1 on error
 */
int nodeupdown_nodelist_destroy(nodeupdown_t handle, char **list);

/* Set the errnum for a nodeupdown handle
 * - used for development of local libraries 
 */      
void nodeupdown_set_errnum(nodeupdown_t handle, int errnum);

#endif /* _NODEUPDOWN_H */
