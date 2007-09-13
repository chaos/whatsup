/*****************************************************************************\
 *  $Id: nodeupdown_api.h,v 1.6 2007-09-13 23:01:29 chu11 Exp $
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

#ifndef _NODEUPDOWN_API_H
#define _NODEUPDOWN_API_H

#include "nodeupdown.h"
#include "hostlist.h"
#include "hash.h"

#define NODEUPDOWN_MAGIC_NUM           0xfeedbeef

#define NODEUPDOWN_UP_NODES            1
#define NODEUPDOWN_DOWN_NODES          0

#define NODEUPDOWN_LOAD_STATE_UNLOADED 0
#define NODEUPDOWN_LOAD_STATE_SETUP    1
#define NODEUPDOWN_LOAD_STATE_LOADED   2

#define NODEUPDOWN_LAST_UP_TIMES_SIZE  2048

/* 
 * struct last_up_time
 *
 * store node string and node last up time
 */
struct last_up_time {
  char *node;
  unsigned int last_up_time;
};

/* 
 * struct nodeupdown
 *
 * nodeupdown handle used throughout the nodeupdown library
 */
struct nodeupdown {
  int magic;
  int errnum;
  int load_state;
  hostlist_t up_nodes;
  hostlist_t down_nodes;
  hash_t last_up_times;
  int numnodes;
};

#endif /* _NODEUPDOWN_API_H */
