/*
 *  $Id: nodeupdown.h,v 1.11 2003-03-18 01:23:16 achu Exp $
 *  $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown.h,v $
 *    
 */

#ifndef _NODEUPDOWN_H
#define _NODEUPDOWN_H

#include <stdio.h>

/**************************************
 * Definitions                        *
 **************************************/

#define NODEUPDOWN_ERR_SUCCESS            0 /* success */
#define NODEUPDOWN_ERR_OPEN               1 /* open file error */
#define NODEUPDOWN_ERR_READ               2 /* read file error */
#define NODEUPDOWN_ERR_CONNECT            3 /* network connection error */
#define NODEUPDOWN_ERR_TIMEOUT            4 /* network connect timeout */
#define NODEUPDOWN_ERR_ADDRESS            5 /* network address error */
#define NODEUPDOWN_ERR_NETWORK            6 /* network error */
#define NODEUPDOWN_ERR_LOAD               7 /* data not loaded */
#define NODEUPDOWN_ERR_OVERFLOW           8 /* overflow on list passed in */
#define NODEUPDOWN_ERR_PARAMETERS         9 /* incorrect parameters passed in */
#define NODEUPDOWN_ERR_NULLPTR           10 /* null pointer in list */
#define NODEUPDOWN_ERR_OUTMEM            11 /* out of memory */
#define NODEUPDOWN_ERR_NODE_CONFLICT     12 /* conflict between gmond & genders */
#define NODEUPDOWN_ERR_NOTFOUND          13 /* node not found */ 
#define NODEUPDOWN_ERR_GENDERS           14 /* internal genders error */
#define NODEUPDOWN_ERR_GANGLIA           15 /* internal ganglia error */
#define NODEUPDOWN_ERR_HOSTLIST          16 /* internal hostlist error */
#define NODEUPDOWN_ERR_MAGIC             17 /* magic number error */
#define NODEUPDOWN_ERR_INTERNAL          18 /* internal system error */

typedef struct nodeupdown *nodeupdown_t;

/* nodeupdown_create
 * - create a nodeupdown handle
 * - returns handle on success, NULL on error
 */
nodeupdown_t nodeupdown_create(void);

/* nodeupdown_load_data
 * - loads data from genders and ganglia
 * - if filename is null, default genders filename is used.
 * - user may pass in hostname, or IP address (presentable format), or both.  
 * - if both hostname and IP are passed in, IP address is used.
 * - if hostname and IP address is NULL, default IP is used.
 * - if 0 or -1 is passed in for port, default port is used
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

/* nodeupdown_errormsg
 * - return a string message describing the most recently
 *   occurred error
 * - returns pointer to message on success, NULL on error
 */
char *nodeupdown_errormsg(nodeupdown_t handle);

/* nodeupdown_perror
 * - output a message to standard error 
 */
void nodeupdown_perror(nodeupdown_t handle, char *msg);

/* nodeupdown_dump
 * - output contents stored in the handle
 * - returns 0 on success, -1 on error
 */
int nodeupdown_dump(nodeupdown_t handle, FILE *stream);

/* nodeupdown_get_up_nodes_string
 * - retrieve a ranged string of up nodes
 * - buffer space assumed to be preallocated with length buflen
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_up_nodes_string(nodeupdown_t handle, char *buf, int buflen);

/* nodeupdown_get_down_nodes_string
 * - retrieve a ranged string of down nodes
 * - buffer space assumed to be preallocated with length buflen
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_down_nodes_string(nodeupdown_t handle, char *buf, int buflen);

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

#ifdef NODEUPDOWN_HOSTLIST_API
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
#endif

/* nodeupdown_get_up_nodes_string_altnames
 * - retrieve ranged string of up nodes with alternate names
 * - buffer space assumed to be preallocated with length buflen
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_up_nodes_string_altnames(nodeupdown_t handle, 
					    char *buf, 
					    int buflen);

/* nodeupdown_get_down_nodes_string_altnames
 * - retrieve ranged string of down nodes with alternate names
 * - buffer space assumed to be preallocated with length buflen
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_down_nodes_string_altnames(nodeupdown_t handle, 
					      char *buf, 
					      int buflen);

/* nodeupdown_get_up_nodes_list_altnames
 * - retrieve list of up nodes with alternate names
 * - list assumed to be preallocated with len elements
 * - returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_up_nodes_list_altnames(nodeupdown_t handle, 
					  char **list, 
					  int len);

/* nodeupdown_get_down_nodes_list_altnames
 * - retrieve list of down nodes with alternate names
 * - list assumed to be preallocated with len elements
 * - returns number of nodes copied on success, -1 on error
 */
int nodeupdown_get_down_nodes_list_altnames(nodeupdown_t handle, 
					    char **list, 
					    int len);

#ifdef NODEUPDOWN_HOSTLIST_API
/* nodeupdown_get_up_nodes_hostlist_altnames
 * - retrieve a hostlist of up nodes with alternate names
 * - hostlist assumed to be pre-allocated
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_up_nodes_hostlist_altnames(nodeupdown_t handle, 
					      hostlist_t hl);

/* nodeupdown_get_down_nodes_hostlist_altnames
 * - retrieve a hostlist of down nodes with alternate names
 * - hostlist assumed to be pre-allocated
 * - returns 0 on success, -1 on error
 */
int nodeupdown_get_down_nodes_hostlist_altnames(nodeupdown_t handle, 
						hostlist_t hl);
#endif

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

/* nodeupdown_convert_string_to_altnames
 * - converts the ranged string to a string of alternate names
 * - buffer space assumed to be both preallocated with length buflen
 * - returns 0 on success, -1 on error
 */
int nodeupdown_convert_string_to_altnames(nodeupdown_t handle, 
					  char *src,
					  char *dest,
					  int buflen);

/* nodeupdown_convert_list_to_altnames
 * - converts the list to alternate names
 * - num_src_nodes indicates the number of nodes stored in src
 * - lists assumed to be both preallocated lists of atleast 
 *   num_src_nodes elements
 * - returns 0 on success, -1 on error
 */
int nodeupdown_convert_list_to_altnames(nodeupdown_t handle, 
					char **src, 
					char **dest,
					int num_src_nodes);

#ifdef NODEUPDOWN_HOSTLIST_API
/* nodeupdown_convert_hostlist_to_altnames
 * - converts the hostlist to alternate names
 * - hostlists assumed to be preallocated
 * - returns 0 on success, -1 on error
 */
int nodeupdown_convert_hostlist_to_altnames(nodeupdown_t handle, 
					    hostlist_t src, 
					    hostlist_t dest);
#endif

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
