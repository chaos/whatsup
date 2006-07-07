/*****************************************************************************\
 *  $Id: pingd.h,v 1.1 2006-07-07 18:14:16 chu11 Exp $
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
