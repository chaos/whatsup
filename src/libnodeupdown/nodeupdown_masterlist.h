/*****************************************************************************\
 *  $Id: nodeupdown_masterlist.h,v 1.3 2004-01-15 01:09:36 achu Exp $
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
 * - Typically, this should be called after a check using "is_node_legit"
 *   or "is_node_in_cluster".
 */
int nodeupdown_masterlist_get_nodename(nodeupdown_t, const char *, char *, int);

/* Increase the number of nodes in the system */
int nodeupdown_masterlist_increase_max_nodes(nodeupdown_t);

#endif /* _NODEUPDOWN_MASTERLIST_H */
