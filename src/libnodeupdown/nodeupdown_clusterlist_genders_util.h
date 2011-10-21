/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_genders_util.h,v 1.8 2010-02-02 00:01:58 chu11 Exp $
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

#ifndef _NODEUPDOWN_CLUSTERLIST_GENDERS_UTIL_H
#define _NODEUPDOWN_CLUSTERLIST_GENDERS_UTIL_H

#include "nodeupdown.h"

/* 
 * genders_util_clusterlist_get_numnodes
 *
 * common genders clusterlist get_numnodes function
 */
int genders_util_clusterlist_get_numnodes(nodeupdown_t handle, 
					  genders_t genders_handle); 

/* 
 * genders_util_clusterlist_compare_to_clusterlist
 *
 * common genders clusterlist compare_to_clusterlist function
 */
int genders_util_clusterlist_compare_to_clusterlist(nodeupdown_t handle,
                                                    genders_t genders_handle) ;

#endif /* _NODEUPDOWN_CLUSTERLIST_GENDERS_UTIL_H */
