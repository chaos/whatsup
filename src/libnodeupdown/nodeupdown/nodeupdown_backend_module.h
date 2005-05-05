/*****************************************************************************\
 *  $Id: nodeupdown_backend_module.h,v 1.1 2005-05-05 18:20:41 achu Exp $
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

#ifndef _NODEUPDOWN_BACKEND_MODULE_H
#define _NODEUPDOWN_BACKEND_MODULE_H

/*
 * Nodeupdown_backend_default_hostname
 *
 * Return pointer to default backend hostname
 */
typedef char *(*Nodeupdown_backend_default_hostname)(nodeupdown_t);

/*
 * Nodeupdown_backend_default_port
 *
 * Return default port of backend tool
 */
typedef int (*Nodeupdown_backend_default_port)(nodeupdown_t);

/*
 * Nodeupdown_backend_default_timeout_len
 *
 * Return default timeout_len of backend tool
 */
typedef int (*Nodeupdown_backend_default_timeout_len)(nodeupdown_t);

/*
 * Nodeupdown_backend_setup
 *
 * Setup the backend module
 *
 * Return 0 on success, -1 on error
 */
typedef int (*Nodeupdown_backend_setup)(nodeupdown_t);

/*
 * Nodeupdown_backend_cleanup
 *
 * Cleanup backend module allocations
 *
 * Return 0 on success, -1 on error
 */
typedef int (*Nodeupdown_backend_cleanup)(nodeupdown_t);

/* 
 * Nodeupdown_backend_get_updown_data
 *
 * Get all updown data and store in the nodeupdown handle
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Nodeupdown_backend_get_updown_data)(nodeupdown_t, const char *, int, int, char *);

/*
 * struct nodeupdown_backend_module_info
 *
 * contains backend module information and operations.  Required to be
 * defined in each backend module.
 */
struct nodeupdown_backend_module_info
{
  char *backend_module_name;
  Nodeupdown_backend_default_hostname default_hostname;
  Nodeupdown_backend_default_port default_port;
  Nodeupdown_backend_default_timeout_len default_timeout_len;
  Nodeupdown_backend_setup setup;
  Nodeupdown_backend_cleanup cleanup;
  Nodeupdown_backend_get_updown_data get_updown_data;
};

#endif /* _NODEUPDOWN_BACKEND_MODULE_H  */
