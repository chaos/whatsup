/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist.c,v 1.7 2005-04-07 17:16:34 achu Exp $
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
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_clusterlist.h"
#include "nodeupdown_util.h"
#include "ltdl.h"

#if WITH_STATIC_MODULES
#if WITH_GENDERSLLNL
extern struct nodeupdown_clusterlist_module_info gendersllnl_clusterlist_module_info;
#endif /* !WITH_GENDERSLLNL */
#if WITH_GENDERS
extern struct nodeupdown_clusterlist_module_info genders_clusterlist_module_info;
#endif /* !WITH_GENDERS */
extern struct nodeupdown_clusterlist_module_info hostsfile_clusterlist_module_info;

extern struct nodeupdown_clusterlist_module_info none_clusterlist_module_info;
 
static struct nodeupdown_clusterlist_module_info *clusterlist_modules[] =
  {
#if WITH_GENDERSLLNL
    &gendersllnl_clusterlist_module_info,
#endif /* !WITH_GENDERSLLNL */
#if WITH_GENDERS
    &genders_clusterlist_module_info,
#endif /* !WITH_GENDERS */
    &hostsfile_clusterlist_module_info,
    &none_clusterlist_module_info,
    NULL
  };
#else  /* !WITH_STATIC_MODULES */
static char *clusterlist_modules[] = {
  "nodeupdown_clusterlist_gendersllnl.la",
  "nodeupdown_clusterlist_genders.la",
  "nodeupdown_clusterlist_none.la",
  "nodeupdown_clusterlist_hostsfile.la",
  NULL
};
static int clusterlist_modules_len = 4;

static lt_dlhandle clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */

static struct nodeupdown_clusterlist_module_info *clusterlist_module_info = NULL;

/* 
 * _load_module
 *
 * Load the specified clusterlist module
 * 
 * Returns 1 if module is found and loaded successfully, 0 if module
 * cannot be found, -1 on fatal error.
 */
static int
_load_module(nodeupdown_t handle, 
#if WITH_STATIC_MODULES
             struct nodeupdown_clusterlist_module_info *module_info
#else  /* !WITH_STATIC_MODULES */
             char *module_path
#endif /* !WITH_STATIC_MODULES */
	     )
{
#if !WITH_STATIC_MODULES
  struct stat buf;
#endif /* !WITH_STATIC_MODULES */

#if WITH_STATIC_MODULES
  clusterlist_module_info = module_info;
#else  /* !WITH_STATIC_MODULES */
  if (stat(module_path, &buf) < 0)
    return 0;

  if (!(clusterlist_module_dl_handle = lt_dlopen(module_path)))
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_INTERNAL;
      goto cleanup;
    }

  if (!(clusterlist_module_info = (struct nodeupdown_clusterlist_module_info *)lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_info")))
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_INTERNAL;
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!clusterlist_module_info->clusterlist_module_name
      || !clusterlist_module_info->init
      || !clusterlist_module_info->complete_loading
      || !clusterlist_module_info->cleanup
      || !clusterlist_module_info->compare_to_clusterlist
      || !clusterlist_module_info->is_node_in_cluster
      || !clusterlist_module_info->is_node_discovered
      || !clusterlist_module_info->get_nodename
      || !clusterlist_module_info->increase_max_nodes)
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_INTERNAL;
      goto cleanup;
    }

  return 1;

 cleanup:
#if !WITH_STATIC_MODULES
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = NULL;
  return -1;
}

int 
nodeupdown_clusterlist_load_module(nodeupdown_t handle, char *clusterlist_module)
{
#if WITH_STATIC_MODULES
  if (clusterlist_module)
    {
      int i = 0;

      while (clusterlist_modules[i])
        {
          if (!clusterlist_modules[i]->clusterlist_module_name)
            continue;

          if (!strcmp(clusterlist_modules[i]->clusterlist_module_name, clusterlist_module))
            {
              int rv;

              if ((rv = _load_module(handle, clusterlist_modules[i])) < 0)
                goto cleanup;

              if (rv)
                goto done;
            }

          i++;
        }

      handle->errnum = NODEUPDOWN_ERR_CONF_INPUT;
      goto cleanup;
    }
  else
    {
      struct nodeupdown_clusterlist_module_info **ptr;
      int i = 0;

      ptr = &clusterlist_modules[0];
      while (ptr[i] != NULL)
        {
          int rv;

          if (!ptr[i]->clusterlist_module_name)
	    continue;

          if ((rv = _load_module(handle, clusterlist_modules[i])) < 0)
            goto cleanup;

          if (rv)
            goto done;

          i++;
        }
    }
#else  /* !WITH_STATIC_MODULES */
  if (clusterlist_module)
    {
      char filebuf[NODEUPDOWN_MAXPATHLEN+1];
      
      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
               NODEUPDOWN_MODULE_BUILDDIR, clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;
      
      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/nodeupdown_clusterlist_%s.la",
               NODEUPDOWN_MODULE_BUILDDIR, clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;
      
      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
               NODEUPDOWN_MODULE_DIR, clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/nodeupdown_clusterlist_%s.la",
               NODEUPDOWN_MODULE_DIR, clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "./nodeupdown_clusterlist_%s.la",
               clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;
      
      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "./%s", clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      handle->errnum = NODEUPDOWN_ERR_CONF_INPUT;
      goto cleanup;
    }
  else
    {
      if ((rv = nodeupdown_util_search_dir_for_module(handle,
						      NODEUPDOWN_MODULE_DIR,
						      clusterlist_modules,
						      clusterlist_modules_len,
						      _load_module)) < 0)
        goto cleanup;
                     
      if (rv)
        goto done;
                                                                          
      if ((rv = nodeupdown_util_search_dir_for_module(handle,
						      ".",
						      clusterlist_modules,
						      clusterlist_modules_len,
						      _load_module)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      if ((rv = nodeupdown_util_search_dir_for_module(handle,
						      NODEUPDOWN_MODULE_BUILDDIR,
						      clusterlist_modules,
						      clusterlist_modules_len,
						      _load_module)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_INTERNAL;
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

 done:
  return 0;

 cleanup:
  return -1;
}

int
nodeupdown_clusterlist_unload_module(nodeupdown_t handle)
{
#if !WITH_STATIC_MODULES
  /* May have not been loaded, so can't close */
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = NULL;
  return 0;
}
 
int 
nodeupdown_clusterlist_init(nodeupdown_t handle)
{
  return (*clusterlist_module_info->init)(handle);
}
 
int 
nodeupdown_clusterlist_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!clusterlist_module_info)
    return 0;

  return (*clusterlist_module_info->cleanup)(handle);
}
 
int 
nodeupdown_clusterlist_complete_loading(nodeupdown_t handle)
{
  return (*clusterlist_module_info->complete_loading)(handle);
}

int 
nodeupdown_clusterlist_compare_to_clusterlist(nodeupdown_t handle)
{
  return (*clusterlist_module_info->compare_to_clusterlist)(handle);
}
 
int 
nodeupdown_clusterlist_is_node_in_cluster(nodeupdown_t handle, 
                                          const char *node)
{
  return (*clusterlist_module_info->is_node_in_cluster)(handle, 
                                                        node);
}
 
int 
nodeupdown_clusterlist_is_node_discovered(nodeupdown_t handle, 
                                          const char *node)
{
  return (*clusterlist_module_info->is_node_discovered)(handle, 
                                                        node);
}
 
int 
nodeupdown_clusterlist_get_nodename(nodeupdown_t handle, 
                                    const char *node, 
                                    char *buffer, 
                                    int buflen)
{
  return (*clusterlist_module_info->get_nodename)(handle, 
                                                  node, 
                                                  buffer, 
                                                  buflen);
}
 
int 
nodeupdown_clusterlist_increase_max_nodes(nodeupdown_t handle)
{
  return (*clusterlist_module_info->increase_max_nodes)(handle);
}


