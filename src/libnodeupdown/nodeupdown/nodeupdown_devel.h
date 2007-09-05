/*****************************************************************************\
 *  $Id: nodeupdown_devel.h,v 1.3 2007-09-05 17:29:26 chu11 Exp $
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
 *  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
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
