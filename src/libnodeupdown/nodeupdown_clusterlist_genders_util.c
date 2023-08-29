/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_genders_util.c,v 1.16 2010-02-02 00:01:58 chu11 Exp $
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <genders.h>

#include "nodeupdown.h"
#include "nodeupdown_clusterlist_genders_util.h"
#include "nodeupdown/nodeupdown_devel.h"

int
genders_util_clusterlist_get_numnodes(nodeupdown_t handle, genders_t genders_handle)
{
  int count;

  if ((count = genders_getnumnodes(genders_handle)) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "genders_getnumnodes: %s\n",
              genders_strerror(genders_errnum(genders_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      return -1;
    }

  return count;
}

int
genders_util_clusterlist_compare_to_clusterlist(nodeupdown_t handle,
                                                genders_t genders_handle)
{
  int i, num;
  char **nlist = NULL;

  /* get all genders nodes */
  if ((num = genders_nodelist_create(genders_handle, &nlist)) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "genders_nodelist_create: %s\n",
              genders_strerror(genders_errnum(genders_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      goto cleanup;
    }

  if (genders_getnodes(genders_handle, nlist, num, NULL, NULL) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "genders_getnodes: %s\n",
              genders_strerror(genders_errnum(genders_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      goto cleanup;
    }

  for (i = 0; i < num; i++)
    {
      if (nodeupdown_not_discovered_check(handle, nlist[i]) < 0)
	goto cleanup;
    }

  if (genders_nodelist_destroy(genders_handle, nlist) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "genders_nodelist_destroy: %s\n",
              genders_strerror(genders_errnum(genders_handle)));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      goto cleanup;
    }
  return 0;

 cleanup:
  (void)genders_nodelist_destroy(genders_handle, nlist);
  return -1;
}
