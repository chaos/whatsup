/*****************************************************************************\
 *  $Id: nodeupdown_util.h,v 1.2 2005-04-06 04:24:16 achu Exp $
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

#ifndef _NODEUPDOWN_UTIL_H
#define _NODEUPDOWN_UTIL_H

#include "nodeupdown.h"

typedef int (*Nodeupdown_util_load_module)(nodeupdown_t, char *);

int nodeupdown_util_low_timeout_connect(nodeupdown_t handle, 
                                        const char *hostname, 
                                        int port,
                                        int connect_timeout); 

int nodeupdown_util_search_dir_for_module(nodeupdown_t handle, 
					  char *search_dir,
					  char **modules_list,
					  int modules_list_len,
					  Nodeupdown_util_load_module load_module);

#endif /* _NODEUPDOWN_UTIL_H */
