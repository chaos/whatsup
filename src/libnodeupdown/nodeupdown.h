/*
 *  $Id: nodeupdown.h,v 1.3 2003-02-26 19:38:37 achu Exp $
 *  $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown.h,v $
 *    
 */

#ifndef _NODEUPDOWN_H
#define _NODEUPDOWN_H

#include <genders.h>
#include <ganglia.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "hostlist.h"

/**************************************
 * Definitions                        *
 **************************************/

#define NODEUPDOWN_ERR_SUCCESS            0 /* success */
#define NODEUPDOWN_ERR_OPEN               1 /* open file error */
#define NODEUPDOWN_ERR_READ               2 /* read file error */
#define NODEUPDOWN_ERR_CONNECT            3 /* network connection error */
#define NODEUPDOWN_ERR_ADDRESS            4 /* network address error */
#define NODEUPDOWN_ERR_NETWORK            5 /* network error */
#define NODEUPDOWN_ERR_LOAD               6 /* data not loaded */
#define NODEUPDOWN_ERR_OVERFLOW           7 /* overflow on list passed in */
#define NODEUPDOWN_ERR_PARAMETERS         8 /* incorrect parameters passed in */
#define NODEUPDOWN_ERR_OUTMEM             9 /* out of memory */
#define NODEUPDOWN_ERR_NODE_CONFLICT     10 /* conflict between gmond & genders */
#define NODEUPDOWN_ERR_NOTFOUND          11 /* node not found */ 
#define NODEUPDOWN_ERR_GENDERS           12 /* internal genders error */
#define NODEUPDOWN_ERR_GANGLIA           13 /* internal ganglia error */
#define NODEUPDOWN_ERR_HOSTLIST          14 /* internal hostlist error */
#define NODEUPDOWN_ERR_INTERNAL          15 /* internal system error */

#define NODEUPDOWN_ERR_MIN               NODEUPDOWN_ERR_SUCCESS
#define NODEUPDOWN_ERR_MAX               NODEUPDOWN_ERR_INTERNAL

#define NODEUPDOWN_MAX_NODES_GUESS       4096

#define NODEUPDOWN_GANGLIA_NAME          "nodeupdown_ganglia"

#define NODEUPDOWN_GANGLIA_CLUSTER_NAME  "nodeupdown_ganglia_cluster"

#define NODEUPDOWN_BUFFERLEN             65536

#ifndef GENDERS_ALTNAME_ATTRIBUTE
#define GENDERS_ALTNAME_ATTRIBUTE        "altname"
#endif

typedef struct nodeupdown *nodeupdown_t;

/* nodeupdown_create
 * - create a nodeupdown handle
 * - returns handle on success, NULL on error
 */
nodeupdown_t nodeupdown_create(void);

/* nodeupdown_load_data
 * - loads data from genders and ganglia
 * - sorts and processes information to determine up and down nodes
 * - user may pass in hostname, or IP address (presentable format), or both.  
 * - if both hostname and IP are passed in, IP address is used.
 * - if hostname and IP address is NULL, default IP is used.
 * - returns 0 on success, -1 on error
 */
int nodeupdown_load_data(nodeupdown_t handle, 
			 char *genders_filename, 
			 char *gmond_hostname, 
			 char *gmond_ip, 
			 int gmond_port);

/* nodeupdown_destroy
 * - destroy a nodeupdown handle
 * - returns 0 on success, -1 on error
 */
int nodeupdown_destroy(nodeupdown_t handle);

/* nodeupdown_errnum
 * - return the most recent error number
 * - returns error number on success, -1 on error
 */
int nodeupdown_errnum(nodeupdown_t handle);

/* nodeupdown_strerror
 * - return a string message describing an error number
 * - returns pointer to message on success, NULL on error
 */
char *nodeupdown_strerror(int errnum);

/* nodeupdown_perror
 * - output a message to standard error 
 */
void nodeupdown_perror(nodeupdown_t handle, char *msg);

/* nodeupdown_dump
 * - output contents stored in the handle
 * - returns 0 on success, -1 on error
 */
int nodeupdown_dump(nodeupdown_t handle, FILE *stream);

/* nodeupdown_get_up_nodes_hostlist
 * - retrieve a hostlist of up nodes
 * - hostlist assumed to be pre-allocated
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_up_nodes_hostlist(nodeupdown_t handle, hostlist_t hl);

/* nodeupdown_get_down_nodes_hostlist
 * - retrieve a hostlist of down nodes
 * - hostlist assumed to be pre-allocated
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_down_nodes_hostlist(nodeupdown_t handle, hostlist_t hl);

/* nodeupdown_get_up_nodes_list
 * - retrieve a list of up nodes
 * - list assumed to be preallocated
 * - returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list, int len);

/* nodeupdown_get_down_nodes_list
 * - retrieve a list of down nodes
 * - list assumed to be preallocated
 * - returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list, int len);

/* nodeupdown_is_node_up
 * - check if a node is up
 * - returns 1 if up, 0 if down, -1 on error
 */
int nodeupdown_is_node_up(nodeupdown_t handle, char *node);

/* nodeupdown_is_node_down
 * - check if a node is down
 * - returns 1 if down, 0 if up, -1 on error
 */
int nodeupdown_is_node_down(nodeupdown_t handle, char *node);

/* nodeupdown_get_hostlist_alternate_names
 * - converts the hostlist to alternate names
 * - hostlists assumed to be preallocated
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_hostlist_alternate_names(nodeupdown_t handle, 
					    hostlist_t src, 
					    hostlist_t dest);

/* nodeupdown_get_list_alternate names
 * - converts the list to alternate names
 * - lists assumed to be preallocated
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_list_alternate_names(nodeupdown_t handle, 
					char **src, 
					char **dest);

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

#endif /* _NODEUPDOWN_H */ 
