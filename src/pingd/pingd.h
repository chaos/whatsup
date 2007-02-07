/*****************************************************************************\
 *  $Id: pingd.h,v 1.2 2007-02-07 17:21:36 chu11 Exp $
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

#ifndef _PINGD_H
#define _PINGD_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <limits.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif /* MAXHOSTNAMELEN */

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif /* MAXPATHLEN */

/*
 * Pingd_clusterlist_setup
 *
 * function prototype for clusterlist module function to setup the
 * module.  Required to be defined by each clusterlist module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Pingd_clusterlist_setup)(void);

/*
 * Pingd_clusterlist_cleanup
 *
 * function prototype for clusterlist module function to
 * cleanup. Required to be defined by each clusterlist module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Pingd_clusterlist_cleanup)(void);

/*
 * Pingd_clusterlist_get_nodes
 *
 * function prototype for clusterlist module function to get all
 * cluster nodes.  Caller is responsible for freeing the created
 * char ** array of nodes.  The returned array of strings will
 * be NULL terminated.   Required to be defined by each clusterlist
 * module.
 *
 * - nodes - pointer to return char ** array of nodes
 *
 * Returns number of cluster nodes retrieved on success, -1
 * on error
 */
typedef int (*Pingd_clusterlist_get_nodes)(char ***nodes);

/*
 * struct pingd_clusterlist_module_info
 *
 * contains clusterlist module information and operations.  Required
 * to be defined in each clusterlist module.
 */
struct pingd_clusterlist_module_info
{
  char *clusterlist_module_name;
  Pingd_clusterlist_setup setup;
  Pingd_clusterlist_cleanup cleanup;
  Pingd_clusterlist_get_nodes get_nodes;
};

#endif /* _PINGD_H */
