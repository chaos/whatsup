/*****************************************************************************\
 *  $Id: nodeupdown_backend.h,v 1.2 2005-04-06 00:56:23 achu Exp $
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

#ifndef _NODEUPDOWN_BACKEND_H
#define _NODEUPDOWN_BACKEND_H

#include "nodeupdown.h"

typedef char *(*Nodeupdown_backend_default_hostname)(nodeupdown_t);
typedef int (*Nodeupdown_backend_default_port)(nodeupdown_t);
typedef int (*Nodeupdown_backend_default_timeout_len)(nodeupdown_t);
typedef int (*Nodeupdown_backend_init)(nodeupdown_t);
typedef int (*Nodeupdown_backend_cleanup)(nodeupdown_t);
typedef int (*Nodeupdown_backend_get_updown_data)(nodeupdown_t, const char *, int, int, char *);

struct nodeupdown_backend_module_info
{
  char *backend_module_name;
  Nodeupdown_backend_default_hostname default_hostname;
  Nodeupdown_backend_default_port default_port;
  Nodeupdown_backend_default_timeout_len default_timeout_len;
  Nodeupdown_backend_init init;
  Nodeupdown_backend_cleanup cleanup;
  Nodeupdown_backend_get_updown_data get_updown_data;
};

char *nodeupdown_backend_default_hostname(nodeupdown_t handle);

int nodeupdown_backend_default_port(nodeupdown_t handle);

int nodeupdown_backend_default_timeout_len(nodeupdown_t handle);

int nodeupdown_backend_init(nodeupdown_t handle);

int nodeupdown_backend_cleanup(nodeupdown_t handle);

int nodeupdown_backend_get_updown_data(nodeupdown_t handle, 
                                       const char *hostname,
                                       int port,
                                       int timeout_len,
                                       char *reserved);

#endif /* _NODEUPDOWN_BACKEND_H  */
