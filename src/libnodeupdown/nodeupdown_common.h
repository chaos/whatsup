/*****************************************************************************\
 *  $Id: nodeupdown_common.h,v 1.13 2005-03-31 23:59:28 achu Exp $
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

#ifndef _NODEUPDOWN_COMMON_H
#define _NODEUPDOWN_COMMON_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_HOSTSFILE
#include "list.h"
#elif HAVE_GENDERS
#include <genders.h>
#elif HAVE_GENDERSLLNL
#include <gendersllnl.h>
#endif 

#include <sys/param.h>
#include "hostlist.h"
#include "nodeupdown.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN 255
#endif

#define NODEUPDOWN_UP_NODES              1
#define NODEUPDOWN_DOWN_NODES            0

#define NODEUPDOWN_MAGIC_NUM             0xfeedbeef

#define NODEUPDOWN_BUFFERLEN             65536

#define NODEUPDOWN_CONNECT_LEN           5 

/* Configuration file keys */
#define NODEUPDOWN_CONF_HOSTNAME                   "hostnames"
#define NODEUPDOWN_CONF_HOSTNAME_MAX               8
#define NODEUPDOWN_CONF_PORT                       "port"
#define NODEUPDOWN_CONF_TIMEOUT_LEN                "timeout_len"
#define NODEUPDOWN_CONF_CLUSTERLIST_MODULE         "clusterlist_module"
#define NODEUPDOWN_CONF_CLUSTERLIST_MODULE_OPTIONS "clusterlist_module_options"

struct nodeupdown {
  int magic;                  /* magic number */
  int errnum;                 /* error code */
  int is_loaded;              /* nodeupdown data loaded? */
  hostlist_t up_nodes;        /* up nodes */
  hostlist_t down_nodes;      /* down nodes */
  int max_nodes;              /* max nodes in genders file */
};

#endif /* _NODEUPDOWN_COMMON_H */
