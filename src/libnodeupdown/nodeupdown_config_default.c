/*****************************************************************************\
 *  $Id: nodeupdown_config_default.c,v 1.4 2008-03-28 17:06:38 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "nodeupdown.h"
#include "nodeupdown/nodeupdown_config.h"
#include "nodeupdown/nodeupdown_config_module.h"

/* 
 * default_config_setup
 *
 * default config module setup function
 */
int 
default_config_setup(nodeupdown_t handle) 
{
  return 0;
}

/* 
 * default_config_cleanup
 *
 * default config module cleanup function
 */
int 
default_config_cleanup(nodeupdown_t handle) 
{
  return 0;
}

/* 
 * default_config_load_default
 *
 * default config module load_default function
 */
int 
default_config_load_default(nodeupdown_t handle, 
			    struct nodeupdown_config *conf)
{
  return 0;
}

struct nodeupdown_config_module_info default_config_module_info =
  {
    "default",
    &default_config_setup,
    &default_config_cleanup,
    &default_config_load_default,
  };
