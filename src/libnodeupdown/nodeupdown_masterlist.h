/*
 *  $Id: nodeupdown_masterlist.h,v 1.1 2003-11-24 16:13:19 achu Exp $
 *  $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown_masterlist.h,v $
 *    
 */

#ifndef _NODEUPDOWN_MASTERLIST_H
#define _NODEUPDOWN_MASTERLIST_H

/* masterlist functions hide most of the master list
 * mechanism specifics from nodeupdown.c
 */

/* Initialize masterlist items in the nodeupdown_t handle */
void nodeupdown_masterlist_initialize_handle(nodeupdown_t);

/* Free masterlist handle data from the nodeupdown_t handle */
void nodeupdown_masterlist_free_handle_data(nodeupdown_t);

/* Initialize any masterlist info, for example, loading data from a file */
int nodeupdown_masterlist_init(nodeupdown_t, void *);

/* finish up any masterlist needs */
int nodeupdown_masterlist_finish(nodeupdown_t);

/* Compare all nodes retrieved from gmond with nodes from the master list 
 * - Adds nodes not found from gmond into the down nodes hostlist
 */
int nodeupdown_masterlist_compare_gmond_to_masterlist(nodeupdown_t);

/* Returns 1 if the specified node name is legitimate 
 * - Identical to nodeupdown_masterlist_is_node_in_cluster except
 *   when no masterlist is available.
 */
int nodeupdown_masterlist_is_node_legit(nodeupdown_t, const char *);

/* Returns 1 if the specified node is in the cluster, 0 if not, -1 on error */
int nodeupdown_masterlist_is_node_in_cluster(nodeupdown_t, const char *);

/* Returns the appropriate nodename to use in the specified buffer
 * - Used if multiple node names may be used to specify a single node
 *   and node names need to be resolved to a common one.  Most of the time
 *   this just copies the nodename straight into the buffer.
 */
int nodeupdown_masterlist_get_nodename(nodeupdown_t, const char *, char *, int);

/* Increase the number of nodes in the system */
int nodeupdown_masterlist_increase_max_nodes(nodeupdown_t);

#endif /* _NODEUPDOWN_MASTERLIST_H */