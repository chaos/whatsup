/*****************************************************************************\
 *  $Id: nodeupdown_config_module.h,v 1.9 2010-02-02 00:01:59 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.
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

#ifndef _NODEUPDOWN_CONFIG_MODULE_H
#define _NODEUPDOWN_CONFIG_MODULE_H

#include <nodeupdown.h>
#include <nodeupdown/nodeupdown_config.h>

/*
 * Nodeupdown_config_setup
 *
 * Setup the config module
 *
 * Return 0 on success, -1 on error
 */
typedef int (*Nodeupdown_config_setup)(nodeupdown_t);

/*
 * Nodeupdown_config_cleanup
 *
 * Cleanup config module allocations
 *
 * Return 0 on success, -1 on error
 */
typedef int (*Nodeupdown_config_cleanup)(nodeupdown_t);

/*
 * Nodeupdown_config_load_config
 *
 * change default configuration values
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Nodeupdown_config_load_config)(nodeupdown_t, 
					      struct nodeupdown_config *);
 
/*
 * struct nodeupdown_config_module_info
 *
 * contains config module information and operations.  Required to be
 * defined in each config module.
 */
struct nodeupdown_config_module_info
{
  char *config_module_name;
  Nodeupdown_config_setup setup;
  Nodeupdown_config_cleanup cleanup;
  Nodeupdown_config_load_config load_config;
};

#endif /* _NODEUPDOWN_CONFIG_MODULE_H  */
