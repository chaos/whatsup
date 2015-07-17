/*****************************************************************************\
 *  $Id: nodeupdown_devel.h,v 1.7 2010-02-02 00:01:59 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
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

#ifndef _NODEUPDOWN_DEVEL_H
#define _NODEUPDOWN_DEVEL_H

#include <nodeupdown.h>

/*
 * nodeupdown_add_up_node
 *
 * Add up node into the handle
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_add_up_node(nodeupdown_t handle, const char *node);

/*
 * nodeupdown_add_down_node
 *
 * Add down node into the handle
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_add_down_node(nodeupdown_t handle, const char *node);

/* 
 * nodeupdown_add_last_up_time
 *
 * Add node time into the handle
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_add_last_up_time(nodeupdown_t handle, 
                                const char *node,
                                unsigned int last_up_time);

/*
 * nodeupdown_not_discovered_check
 *
 * Determines if a node has already been added into the handle.  If it
 * has not, add the node to the down nodes.
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_not_discovered_check(nodeupdown_t handle, const char *node);

/* 
 * nodeupdown_set_errnum
 * 
 * Set the errnum for a nodeupdown handle.
 */      
void nodeupdown_set_errnum(nodeupdown_t handle, int errnum);

#endif /* _NODEUPDOWN_DEVEL_H */
