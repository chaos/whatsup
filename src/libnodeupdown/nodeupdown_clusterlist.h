/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist.h,v 1.4 2005-04-25 19:30:10 achu Exp $
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

#ifndef _NODEUPDOWN_CLUSTERLIST_H
#define _NODEUPDOWN_CLUSTERLIST_H

#include "nodeupdown.h"

/*
 * nodeupdown_clusterlist_load_module
 *
 * Find and load the nodeupdown clusterlist module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_clusterlist_load_module(nodeupdown_t handle, char *clusterlist_module);

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

#endif /* _NODEUPDOWN_CLUSTERLIST_H */
