/*****************************************************************************\
 *  $Id: nodeupdown_config_gendersllnl.c,v 1.16 2005-06-28 22:55:59 achu Exp $
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <gendersllnl.h>

#include "nodeupdown.h"
#include "nodeupdown_genders_util.h"
#include "nodeupdown/nodeupdown_config.h"
#include "nodeupdown/nodeupdown_config_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

static genders_t gendersllnl_handle = NULL;

/* 
 * gendersllnl_config_setup
 *
 * gendersllnl config module setup function
 */
int 
gendersllnl_config_setup(nodeupdown_t handle) 
{
  int rv;

  rv = genders_util_setup(handle, &gendersllnl_handle);

#if HAVE_GENDERS_INDEX_ATTRVALS
  if (!rv)
    {
      /* This is for performance improvements if the indexing API
       * functions are available.  Don't fail and return -1, since the
       * rest of libnodeupdown is not dependent on this section of code.
       */
      genders_index_attrvals(gendersllnl_handle, GENDERS_ALTNAME_ATTRIBUTE);
    }
#endif /* HAVE_GENDERS_INDEX_ATTRVALS */

  if (rv < 0)
    nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONFIG_MODULE);

  return rv;
}

/* 
 * gendersllnl_config_cleanup
 *
 * gendersllnl config module cleanup function
 */
int 
gendersllnl_config_cleanup(nodeupdown_t handle) 
{
  return genders_util_cleanup(handle, &gendersllnl_handle);
}

/* 
 * gendersllnl_config_load_default
 *
 * gendersllnl config module load_default function
 */
int 
gendersllnl_config_load_default(nodeupdown_t handle, 
                                struct nodeupdown_config *conf)
{
  int flag;

  if ((flag = genders_testattr(gendersllnl_handle, NULL, "mgmt", NULL, 0)) < 0)
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONFIG_MODULE);
      return -1;
    }

  if (!flag)
    {
      char **altnodelist;
      int altnodelistlen, mgmt_len;

      if ((altnodelistlen = genders_altnodelist_create(gendersllnl_handle, 
                                                       &altnodelist)) < 0)
        {
	  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONFIG_MODULE);
          return -1;
        }

      if ((mgmt_len = genders_getaltnodes_preserve(gendersllnl_handle, 
                                                   altnodelist, 
                                                   altnodelistlen, 
                                                   "mgmt", 
                                                   NULL)) < 0)
        {
          (void)genders_altnodelist_destroy(gendersllnl_handle, altnodelist);
	  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONFIG_MODULE);
          return -1;
        }

      if (mgmt_len)
        {
          int i;

          if (mgmt_len > NODEUPDOWN_CONFIG_HOSTNAMES_MAX)
            mgmt_len = NODEUPDOWN_CONFIG_HOSTNAMES_MAX;

          for (i = 0; i < mgmt_len; i++)
            {
              if (strlen(altnodelist[i]) > NODEUPDOWN_MAXHOSTNAMELEN)
                {
                  (void)genders_altnodelist_destroy(gendersllnl_handle, altnodelist);
		  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONFIG_MODULE);
                  return -1;
                }
              strcpy(conf->hostnames[i], altnodelist[i]);
            }
	  conf->hostnames_len = mgmt_len;
          conf->hostnames_flag++;
        }
      
      if (genders_altnodelist_destroy(gendersllnl_handle, altnodelist) < 0)
        {
	  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CONFIG_MODULE);
          return -1;
        }
    }
  
  return 0;
}

struct nodeupdown_config_module_info config_module_info =
  {
    "gendersllnl",
    &gendersllnl_config_setup,
    &gendersllnl_config_cleanup,
    &gendersllnl_config_load_default,
  };
