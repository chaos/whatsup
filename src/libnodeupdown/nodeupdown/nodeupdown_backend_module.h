/*****************************************************************************\
 *  $Id: nodeupdown_backend_module.h,v 1.12 2010-02-02 00:01:59 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155699
 *
 *  This file is part of Whatsup, tools and libraries for determining up and
 *  down nodes in a cluster. For details, see https://github.com/chaos/whatsup.
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

#ifndef _NODEUPDOWN_BACKEND_MODULE_H
#define _NODEUPDOWN_BACKEND_MODULE_H

#include <nodeupdown.h>

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
 * Nodeupdown_backend_preferred_clusterlist_module
 *
 * Return preferred clusterlist module name or NULL if none
 */
typedef char *(*Nodeupdown_backend_preferred_clusterlist_module)(nodeupdown_t);

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
typedef int (*Nodeupdown_backend_get_updown_data)(nodeupdown_t,
                                                  const char *,
                                                  unsigned int,
                                                  unsigned int,
                                                  char *);

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
  Nodeupdown_backend_preferred_clusterlist_module preferred_clusterlist_module;
  Nodeupdown_backend_setup setup;
  Nodeupdown_backend_cleanup cleanup;
  Nodeupdown_backend_get_updown_data get_updown_data;
};

#endif /* _NODEUPDOWN_BACKEND_MODULE_H  */
