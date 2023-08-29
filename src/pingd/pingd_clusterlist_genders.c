/*****************************************************************************\
 *  $Id: pingd_clusterlist_genders.c,v 1.7 2010-02-02 00:01:59 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2010 Lawrence Livermore National Security, LLC.
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>

#include <genders.h>

#include "pingd.h"

#include "debug.h"

#define GENDERS_MODULE_NAME "genders"

/*
 * gh
 *
 * genders handle
 */
static genders_t gh = NULL;

/*
 * genders_setup
 *
 * genders clusterlist module setup function
 */
static int
genders_setup(void)
{
  if (gh)
    {
      ERR_DEBUG(("gh non-null"));
      return 0;
    }

  if (!(gh = genders_handle_create()))
    {
      ERR_DEBUG(("genders_handle_create"));
      goto cleanup;
    }

  if (genders_load_data(gh, NULL) < 0)
    {
      if (genders_errnum(gh) == GENDERS_ERR_OPEN)
        {
          ERR_OUTPUT(("genders database '%s' cannot be opened", GENDERS_DEFAULT_FILE));
          goto cleanup;
        }
      else
        {
          ERR_DEBUG(("genders_load_data: %s", genders_errormsg(gh)));
          goto cleanup;
        }
    }

  return 0;

 cleanup:
  if (gh)
    genders_handle_destroy(gh);
  gh = NULL;
  return -1;
}

/*
 * genders_cleanup
 *
 * genders clusterlist module cleanup function
 */
static int
genders_cleanup(void)
{
  if (!gh)
    return 0;

  if (genders_handle_destroy(gh) < 0)
    {
      ERR_DEBUG(("genders_handle_destroy: %s", genders_errormsg(gh)));
      return -1;
    }

  gh = NULL;

  return 0;
}

/*
 * genders_get_nodes
 *
 * genders clusterlist module get_nodes function
 */
static int
genders_get_nodes(char ***nodes)
{
  char **nodelist = NULL;
  int nodelistlen, numnodes;

  if (!gh)
    {
      ERR_DEBUG(("gh null"));
      return -1;
    }

  if (!nodes)
    {
      ERR_DEBUG(("invalid parameters"));
      return -1;
    }

  if ((nodelistlen = genders_nodelist_create(gh, &nodelist)) < 0)
    {
      ERR_DEBUG(("genders_nodelist_create: %s", genders_errormsg(gh)));
      goto cleanup;
    }

  if ((numnodes = genders_getnodes(gh,
                                   nodelist,
                                   nodelistlen,
                                   NULL,
                                   NULL)) < 0)
    {
      ERR_DEBUG(("genders_getnodes: %s", genders_errormsg(gh)));
      goto cleanup;
    }

  *nodes = nodelist;
  return numnodes;

 cleanup:
  if (nodelist)
    genders_nodelist_destroy(gh, nodelist);
  return -1;
}

struct pingd_clusterlist_module_info module_info =
  {
    GENDERS_MODULE_NAME,
    &genders_setup,
    &genders_cleanup,
    &genders_get_nodes,
  };
