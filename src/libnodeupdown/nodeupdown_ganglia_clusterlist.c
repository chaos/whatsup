/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist.c,v 1.4 2005-04-01 18:48:32 achu Exp $
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_ganglia_clusterlist.h"
#include "ltdl.h"

char *clusterlist_modules[] = {
  "nodeupdown_ganglia_clusterlist_gendersllnl.la",
  "nodeupdown_ganglia_clusterlist_genders.la",
  "nodeupdown_ganglia_clusterlist_none.la",
  "nodeupdown_ganglia_clusterlist_hostsfile.la",
  NULL
};
int clusterlist_modules_len = 4;

static lt_dlhandle ganglia_clusterlist_module_dl_handle = NULL;
static struct nodeupdown_ganglia_clusterlist_module_info *ganglia_clusterlist_module_info = NULL;

static int
_load_module(nodeupdown_t handle, char *module_path)
{
  if (!(ganglia_clusterlist_module_dl_handle = lt_dlopen(module_path)))
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      goto cleanup;
    }

  if (!(ganglia_clusterlist_module_info = (struct nodeupdown_ganglia_clusterlist_module_info *)lt_dlsym(ganglia_clusterlist_module_dl_handle, "ganglia_clusterlist_module_info")))
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      goto cleanup;
    }

  if (!ganglia_clusterlist_module_info->ganglia_clusterlist_module_name
      || !ganglia_clusterlist_module_info->parse_options
      || !ganglia_clusterlist_module_info->init
      || !ganglia_clusterlist_module_info->finish
      || !ganglia_clusterlist_module_info->cleanup
      || !ganglia_clusterlist_module_info->compare_to_clusterlist
      || !ganglia_clusterlist_module_info->is_node_in_cluster
      || !ganglia_clusterlist_module_info->is_node_discovered
      || !ganglia_clusterlist_module_info->get_nodename
      || !ganglia_clusterlist_module_info->increase_max_nodes)
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
    }

  return 1;

 cleanup:
  if (ganglia_clusterlist_module_dl_handle)
    lt_dlclose(ganglia_clusterlist_module_dl_handle);
  ganglia_clusterlist_module_info = NULL;
  ganglia_clusterlist_module_dl_handle = NULL;
  return -1;
}

int
_search_dir_for_module(nodeupdown_t handle, 
                       char *search_dir,
                       char **modules_list,
                       int modules_list_len)
{
  DIR *dir;
  int i = 0, found = 0;
 
  /* Can't open the directory? we assume it doesn't exit, so its not
   * an error.
   */
  if (!(dir = opendir(search_dir)))
    return 0;

  for (i = 0; i < modules_list_len; i++)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, modules_list[i]))
            {
              char filebuf[MAXPATHLEN+1];
              int ret;

              memset(filebuf, '\0', MAXPATHLEN+1);
              snprintf(filebuf, MAXPATHLEN, "%s/%s",
                       search_dir, modules_list[i]);

              if ((ret = _load_module(handle, filebuf)) < 0)
                  goto cleanup;

              if (ret)
                {
                  found++;
                  goto done;
                }
            }
        }
      rewinddir(dir);
    }

 done:
  closedir(dir);

  return (found) ? 1 : 0;

 cleanup:
  return -1;
}

int 
nodeupdown_ganglia_clusterlist_load_module(nodeupdown_t handle, char *clusterlist_module)
{
  int rv;

  if (lt_dlinit() != 0)
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      return -1;
    }

  if (clusterlist_module)
    {
      char filebuf[MAXPATHLEN+1];

      memset(filebuf, '\0', MAXPATHLEN+1);
      snprintf(filebuf, MAXPATHLEN, "%s/%s",
               NODEUPDOWN_MODULE_DIR, clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto done;

      if (rv)
        goto done;

      memset(filebuf, '\0', MAXPATHLEN+1);
      snprintf(filebuf, MAXPATHLEN, "./%s", clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto done;

      if (rv)
        goto done;

      memset(filebuf, '\0', MAXPATHLEN+1);
      snprintf(filebuf, MAXPATHLEN, "%s/cerebrod_config_%s.la",
               NODEUPDOWN_MODULE_DIR, clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto done;

      if (rv)
        goto done;

      memset(filebuf, '\0', MAXPATHLEN+1);
      snprintf(filebuf, MAXPATHLEN, "./cerebrod_config_%s.la",
               clusterlist_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto done;
      
      if (rv)
        goto done;

      handle->errnum = NODEUPDOWN_ERR_CONF_OPEN;
      goto cleanup;
    }
  else
    {
      if ((rv = _search_dir_for_module(handle,
                                       NODEUPDOWN_MODULE_DIR,
                                       clusterlist_modules,
                                       clusterlist_modules_len)) < 0)
        goto cleanup;
                     
      if (rv)
        goto done;
                                                                          
      if ((rv = _search_dir_for_module(handle,
                                       ".",
                                       clusterlist_modules,
                                       clusterlist_modules_len)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      printf("dir=%s\n", NODEUPDOWN_MODULE_BUILDDIR);
      if ((rv = _search_dir_for_module(handle,
                                       NODEUPDOWN_MODULE_BUILDDIR,
                                       clusterlist_modules,
                                       clusterlist_modules_len)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST;
      goto cleanup;
    }

 done:
  return 0;

 cleanup:
  return -1;
}

int
nodeupdown_ganglia_clusterlist_unload_module(nodeupdown_t handle)
{
  /* May have not been loaded, so can't close */
  if (ganglia_clusterlist_module_dl_handle)
    lt_dlclose(ganglia_clusterlist_module_dl_handle);
  lt_dlexit();
  ganglia_clusterlist_module_info = NULL;
  ganglia_clusterlist_module_dl_handle = NULL;
  return 0;
}
 
int 
nodeupdown_ganglia_clusterlist_parse_options(nodeupdown_t handle, char **options)
{
  return (*ganglia_clusterlist_module_info->parse_options)(handle, options);
}

int 
nodeupdown_ganglia_clusterlist_init(nodeupdown_t handle)
{
  return (*ganglia_clusterlist_module_info->init)(handle);
}
 
int 
nodeupdown_ganglia_clusterlist_finish(nodeupdown_t handle)
{
  return (*ganglia_clusterlist_module_info->finish)(handle);
}

int 
nodeupdown_ganglia_clusterlist_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!ganglia_clusterlist_module_info)
    return 0;

  return (*ganglia_clusterlist_module_info->cleanup)(handle);
}
 
int 
nodeupdown_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle)
{
  return (*ganglia_clusterlist_module_info->compare_to_clusterlist)(handle);
}
 
int 
nodeupdown_ganglia_clusterlist_is_node_in_cluster(nodeupdown_t handle, 
                                                  const char *node)
{
  return (*ganglia_clusterlist_module_info->is_node_in_cluster)(handle, 
                                                                node);
}
 
int 
nodeupdown_ganglia_clusterlist_is_node_discovered(nodeupdown_t handle, 
                                                  const char *node)
{
  return (*ganglia_clusterlist_module_info->is_node_discovered)(handle, 
                                                                node);
}
 
int 
nodeupdown_ganglia_clusterlist_get_nodename(nodeupdown_t handle, 
                                            const char *node, 
                                            char *buffer, 
                                            int buflen)
{
  return (*ganglia_clusterlist_module_info->get_nodename)(handle, 
                                                          node, 
                                                          buffer, 
                                                          buflen);
}
 
int 
nodeupdown_ganglia_clusterlist_increase_max_nodes(nodeupdown_t handle)
{
  return (*ganglia_clusterlist_module_info->increase_max_nodes)(handle);
}


