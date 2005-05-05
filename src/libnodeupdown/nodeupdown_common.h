/*****************************************************************************\
 *  $Id: nodeupdown_common.h,v 1.24 2005-05-05 18:09:56 achu Exp $
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
#endif /* HAVE_CONFIG_H */

#include "nodeupdown.h"
#include "hostlist.h"
#include "list.h"

#define NODEUPDOWN_MAXHOSTNAMELEN        64

#define NODEUPDOWN_MAXNODENAMELEN        NODEUPDOWN_MAXHOSTNAMELEN

#define NODEUPDOWN_MAXPATHLEN            256

#define NODEUPDOWN_UP_NODES              1
#define NODEUPDOWN_DOWN_NODES            0

#define NODEUPDOWN_MAGIC_NUM             0xfeedbeef

#define NODEUPDOWN_BUFFERLEN             65536

/* 
 * Older options to be ignored by conffile library
 */
#define NODEUPDOWN_CONF_GMOND_HOSTNAME            "gmond_hostname"
#define NODEUPDOWN_CONF_GMOND_HOSTNAMES           "gmond_hostnames"
#define NODEUPDOWN_CONF_GMOND_IP                  "gmond_ip"
#define NODEUPDOWN_CONF_GMOND_PORT                "gmond_port"
#define NODEUPDOWN_CONF_HOSTSFILE                 "hostsfile"
#define NODEUPDOWN_CONF_GENDERSFILE               "gendersfile"

#define NODEUPDOWN_CONF_HOSTNAMES                  "hostnames"
#define NODEUPDOWN_CONF_HOSTNAMES_MAX              8
#define NODEUPDOWN_CONF_PORT                       "port"
#define NODEUPDOWN_CONF_TIMEOUT_LEN                "timeout_len"

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

/* 
 * struct nodeupdown_config
 *
 * stores configuration file data
 */
struct nodeupdown_config 
{
  char hostnames[NODEUPDOWN_CONF_HOSTNAMES_MAX+1][NODEUPDOWN_MAXHOSTNAMELEN+1];
  int hostnames_len;
  int hostnames_flag;
  int port;
  int port_flag;
  int timeout_len;
  int timeout_len_flag;
};

#endif /* _NODEUPDOWN_COMMON_H */
