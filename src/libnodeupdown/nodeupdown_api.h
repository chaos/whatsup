/*****************************************************************************\
 *  $Id: nodeupdown_api.h,v 1.1 2005-05-05 21:23:51 achu Exp $
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

#ifndef _NODEUPDOWN_API_H
#define _NODEUPDOWN_API_H

#include "nodeupdown.h"
#include "hostlist.h"

#define NODEUPDOWN_MAGIC_NUM             0xfeedbeef

#define NODEUPDOWN_BUFFERLEN             65536

/* 
 * struct nodeupdown
 *
 * nodeupdown handle used throughout the nodeupdown library
 */
struct nodeupdown {
  int magic;
  int errnum;
  int is_loaded;
  hostlist_t up_nodes;
  hostlist_t down_nodes;
  int max_nodes;
};

#endif /* _NODEUPDOWN_API_H */
