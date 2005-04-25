/*****************************************************************************\
 *  $Id: nodeupdown_config.h,v 1.3 2005-04-25 19:30:10 achu Exp $
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

#ifndef _NODEUPDOWN_CONFIG_H
#define _NODEUPDOWN_CONFIG_H

#include "nodeupdown.h"

/* 
 * nodeupdown_config_load_module
 *
 * Find and load the nodeupdown config module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_config_load_module(nodeupdown_t handle);

/* 
 * nodeupdown_config_unload_module
 *
 * unload the nodeupdown config module
 *
 * Returns 0 on success, -1 on error
 */
int nodeupdown_config_unload_module(nodeupdown_t handle);

/* 
 * nodeupdown_config_setup
 *
 * call config module setup function
 */
int nodeupdown_config_setup(nodeupdown_t handle);

/* 
 * nodeupdown_config_cleanup
 *
 * call config module cleanup function
 */
int nodeupdown_config_cleanup(nodeupdown_t handle);

/* 
 * nodeupdown_config_load_default
 *
 * call config module load_default function
 */
int nodeupdown_config_load_default(nodeupdown_t handle, 
                                   struct nodeupdown_confdata *conf);

#endif /* _NODEUPDOWN_CONFIG_H  */
