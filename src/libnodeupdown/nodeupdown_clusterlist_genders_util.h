/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_genders_util.h,v 1.1 2005-04-05 23:54:50 achu Exp $
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

#ifndef _NODEUPDOWN_CLUSTERLIST_GENDERS_UTIL_H
#define _NODEUPDOWN_CLUSTERLIST_GENDERS_UTIL_H

#include "nodeupdown.h"

int genders_util_clusterlist_init(nodeupdown_t handle,
                                  genders_t *genders_handle);

int genders_util_clusterlist_cleanup(nodeupdown_t handle,
                                     genders_t *genders_handle); 

int genders_util_clusterlist_complete_loading(nodeupdown_t handle, 
                                              genders_t genders_handle); 

int genders_util_clusterlist_compare_to_clusterlist(nodeupdown_t handle,
                                                    genders_t genders_handle) ;

#endif /* _NODEUPDOWN_CLUSTERLIST_GENDERS_UTIL_H */
