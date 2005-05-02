/*****************************************************************************\
 *  $Id: nodeupdown_backend.h,v 1.7 2005-05-02 23:00:28 achu Exp $
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

#ifndef _NODEUPDOWN_BACKEND_H
#define _NODEUPDOWN_BACKEND_H

#include "nodeupdown.h"

/* 
 * nodeupdown_backend_load_module
 *
 * Find and load the nodeupdown backend module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_backend_load_module(nodeupdown_t handle);

/* 
 * nodeupdown_backend_unload_module
 *
 * unload the nodeupdown backend module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_backend_unload_module(nodeupdown_t handle);

/* 
 * nodeupdown_backend_default_hostname
 *
 * call backend module default_hostname function
 */
char *nodeupdown_backend_default_hostname(nodeupdown_t handle);

/* 
 * nodeupdown_backend_default_port
 *
 * call backend module default_port function
 */
int nodeupdown_backend_default_port(nodeupdown_t handle);

/* 
 * nodeupdown_backend_default_timeout_len
 *
 * call backend module default_timeout_len function
 */
int nodeupdown_backend_default_timeout_len(nodeupdown_t handle);

/* 
 * nodeupdown_backend_setup
 *
 * call backend module setup function
 */
int nodeupdown_backend_setup(nodeupdown_t handle);

/* 
 * nodeupdown_backend_cleanup
 *
 * call backend module cleanup function
 */
int nodeupdown_backend_cleanup(nodeupdown_t handle);

/* 
 * nodeupdown_backend_get_updown_data
 *
 * call backend module get_updown_data function
 */
int nodeupdown_backend_get_updown_data(nodeupdown_t handle, 
                                       const char *hostname,
                                       int port,
                                       int timeout_len,
                                       char *reserved);

#endif /* _NODEUPDOWN_BACKEND_H  */
