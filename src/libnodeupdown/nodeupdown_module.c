/*****************************************************************************\
 *  $Id: nodeupdown_module.c,v 1.29 2008-03-28 17:06:38 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
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
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <dirent.h>
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_api.h"
#include "nodeupdown_module.h"
#include "nodeupdown_util.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_clusterlist_module.h"
#include "nodeupdown/nodeupdown_config.h"
#include "nodeupdown/nodeupdown_config_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "ltdl.h"

static char *backend_modules[] = {
  "nodeupdown_backend_cerebro.so",
  "nodeupdown_backend_ganglia.so",
  "nodeupdown_backend_pingd.so",
  "nodeupdown_backend_openib.so",
  NULL
};

static char *clusterlist_modules[] = {
  "nodeupdown_clusterlist_gendersllnl.so",
  "nodeupdown_clusterlist_genders.so",
  "nodeupdown_clusterlist_hostsfile.so",
  NULL
};

static char *config_modules[] = {
  "nodeupdown_config_chaos.so",
  NULL
};

#define BACKEND_MODULE_SIGNATURE     "nodeupdown_backend_"
#define CLUSTERLIST_MODULE_SIGNATURE "nodeupdown_clusterlist_"
#define CONFIG_MODULE_SIGNATURE      "nodeupdown_config_"

#define BACKEND_MODULE_INFO_SYM      "backend_module_info"
#define CLUSTERLIST_MODULE_INFO_SYM  "clusterlist_module_info"
#define CONFIG_MODULE_INFO_SYM       "config_module_info"

#define BACKEND_MODULE_BUFLEN        1024
#define CLUSTERLIST_MODULE_BUFLEN    1024

static lt_dlhandle backend_module_dl_handle = NULL;
static lt_dlhandle clusterlist_module_dl_handle = NULL;
static lt_dlhandle config_module_dl_handle = NULL;

static struct nodeupdown_backend_module_info *backend_module_info = NULL;

static struct nodeupdown_clusterlist_module_info *clusterlist_module_info = NULL;

static struct nodeupdown_config_module_info *config_module_info = NULL;

extern struct nodeupdown_clusterlist_module_info default_clusterlist_module_info;

extern struct nodeupdown_config_module_info default_config_module_info;

/* externed from outside this file */
int clusterlist_module_found = 0;

/* 
 * Nodeupdown_module_callback
 *
 * Define a load module function to be passed to a module finder
 * function.
 *
 * Returns 1 if module is loaded, 0 if not, -1 on fatal error.
 */
typedef int (*Nodeupdown_module_callback)(nodeupdown_t, void *, void *);

/* 
 * _load_module
 *
 * Load module indicated by the parameters and call module callback
 *
 * Returns 1 if module loaded, 0 if not, -1 on error
 */
static int
_load_module(nodeupdown_t handle,
             char *search_dir,
             char *filename,
             Nodeupdown_module_callback module_callback,
             char *module_info_sym)
{
  char filebuf[NODEUPDOWN_MAXPATHLEN+1];
  struct stat buf;
  lt_dlhandle dl_handle;
  void *module_info;
  int flag;

  memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
  snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s", search_dir, filename);

  if (stat(filebuf, &buf) < 0)
    return 0;

  if (!(dl_handle = lt_dlopen(filebuf)))
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
  
  if (!(module_info = lt_dlsym(dl_handle, module_info_sym)))
    {
      lt_dlclose(dl_handle);
      return 0;
    }

  if ((flag = module_callback(handle, dl_handle, module_info)) < 0)
    goto cleanup;

  if (!flag)
    lt_dlclose(dl_handle);

  return flag;

 cleanup:
  return -1;
}

/* 
 * _find_known_module
 *
 * Search the directory 'search_dir' for any one of the modules listed
 * in 'modules_list'.  Call 'module_callback' on any discovered module.
 *
 * Returns 1 if a module is found and loaded, 0 if a module is not
 * found, -1 on fatal error.
 */
static int
_find_known_module(nodeupdown_t handle, 
                   char *search_dir,
                   char **modules_list,
                   Nodeupdown_module_callback module_callback,
                   char *module_info_sym)
{
  DIR *dir;
  int i = 0, found = 0, rv = -1;
 
  /* Can't open the directory? we assume it doesn't exit, so its not
   * an error.
   */
  if (!(dir = opendir(search_dir)))
    return 0;

  for (i = 0; modules_list[i]; i++)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, modules_list[i]))
            {
              int flag;

              if ((flag = _load_module(handle,
                                       search_dir,
                                       modules_list[i],
                                       module_callback,
                                       module_info_sym)) < 0)
                goto cleanup;
                                       
              if (flag)
                {
                  found++;
                  goto done;
                }
            }
        }
      rewinddir(dir);
    }

 done:
  rv = (found) ? 1 : 0;
 cleanup:
  closedir(dir);
  return rv;
}

/*
 * _find_unknown_module
 *
 * Search the directory 'search_dir' for any module with the given signature.
 *
 * Returns 1 when a module is found, 0 when one is not, -1 on fatal error
 */
static int
_find_unknown_module(nodeupdown_t handle,
                     char *search_dir,
                     char *signature,
                     Nodeupdown_module_callback module_callback,
                     char *module_info_sym)
{
  DIR *dir;
  struct dirent *dirent;
  int rv = -1, found = 0;
 
  if (!(dir = opendir(search_dir)))
    return 0;
 
  while ((dirent = readdir(dir)))
    {
      char *ptr = strstr(dirent->d_name, signature);

      if (ptr && ptr == &dirent->d_name[0])
        {
          int flag;

          /* Don't bother unless its a shared object */
          ptr = strchr(dirent->d_name, '.');
          if (!ptr || strcmp(ptr, ".so"))
            continue;

          if ((flag = _load_module(handle,
                                   search_dir,
                                   dirent->d_name,
                                   module_callback,
                                   module_info_sym)) < 0)
            goto cleanup;

          if (flag)
            {
              found++;
              goto done;
            }
        }
    }
   
 done:
  rv = (found) ? 1 : 0;
 cleanup:
  closedir(dir);
  return rv;
}

/*
 * _find_module
 *
 * Find and load a module
 *
 * Returns 1 when a module is found, 0 when one is not, -1 on fatal error
 */
static int
_find_module(nodeupdown_t handle,
             char **modules_list,
             char *signature,
             Nodeupdown_module_callback module_callback,
             char *module_info_sym)
{
  int rv;

  if (modules_list)
    {
#ifndef NDEBUG
      if ((rv = _find_known_module(handle,
                                   NODEUPDOWN_MODULE_BUILDDIR "/.libs",
                                   modules_list,
                                   module_callback,
                                   module_info_sym)) < 0)
        return -1;
      
      if (rv)
        return 1;
#endif /* !NDEBUG */
      
      if ((rv = _find_known_module(handle,
                                   NODEUPDOWN_MODULE_DIR,
                                   modules_list,
                                   module_callback,
                                   module_info_sym)) < 0)
        return -1;
      
      if (rv)
        return 1;
    }
  
  if ((rv = _find_unknown_module(handle,
                                 NODEUPDOWN_MODULE_DIR,
                                 signature,
                                 module_callback,
                                 module_info_sym)) < 0)
    return -1;
  
  if (rv)
    return 1;

  return 0;
}

/* 
 * _backend_module_callback
 *
 * Check and setup the backend module
 * 
 * Returns 1 if is loaded, 0 if not, -1 on fatal error
 */
static int
_backend_module_callback(nodeupdown_t handle, void *dl_handle, void *module_info)
{
  if (!dl_handle || !module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  backend_module_dl_handle = dl_handle;
  backend_module_info = module_info;

  if (!backend_module_info->backend_module_name
      || !backend_module_info->default_hostname
      || !backend_module_info->default_port
      || !backend_module_info->default_timeout_len
      || !backend_module_info->preferred_clusterlist_module
      || !backend_module_info->setup
      || !backend_module_info->cleanup
      || !backend_module_info->get_updown_data)
    {
      backend_module_dl_handle = NULL;
      backend_module_info = NULL;
      return 0;
    }

  return 1;
}

int 
backend_module_load(nodeupdown_t handle, char *module)
{
  int rv;
  
  if (module)
    {
      char *temp_backend_modules[2];
      char modulebuf[BACKEND_MODULE_BUFLEN];
      int len;
      
      len = snprintf(modulebuf, 
                     BACKEND_MODULE_BUFLEN, 
                     "nodeupdown_backend_%s.so",
                     module);
      if (len < 0 || len >= BACKEND_MODULE_BUFLEN)
        {
          handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
          return -1;
        }

      temp_backend_modules[0] = modulebuf;
      temp_backend_modules[1] = NULL;

      if ((rv = _find_module(handle,
                             temp_backend_modules,
                             BACKEND_MODULE_SIGNATURE,
                             _backend_module_callback,
                             BACKEND_MODULE_INFO_SYM)) < 0)
        return -1;
    }
  else
    {
      if ((rv = _find_module(handle,
                             backend_modules,
                             BACKEND_MODULE_SIGNATURE,
                             _backend_module_callback,
                             BACKEND_MODULE_INFO_SYM)) < 0)
        return -1;
    }

  if (!rv)
    {
      handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
      return -1;
    }

  return 0;
}

int
backend_module_unload(nodeupdown_t handle)
{
  /* May have not been loaded, so can't close */
  if (backend_module_dl_handle)
    lt_dlclose(backend_module_dl_handle);
  backend_module_dl_handle = NULL;
  backend_module_info = NULL;
  return 0;
}
 
char *
backend_module_name(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }
  return backend_module_info->backend_module_name;
}

char *
backend_module_default_hostname(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }
  return (*backend_module_info->default_hostname)(handle);
}

int
backend_module_default_port(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  return (*backend_module_info->default_port)(handle);
}

int
backend_module_default_timeout_len(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  return (*backend_module_info->default_timeout_len)(handle);
}

char *
backend_module_preferred_clusterlist_module(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }
  return (*backend_module_info->preferred_clusterlist_module)(handle);
}

int 
backend_module_setup(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  return (*backend_module_info->setup)(handle);
}

int 
backend_module_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!backend_module_info)
    return 0;

  return (*backend_module_info->cleanup)(handle);
}
 
int 
backend_module_get_updown_data(nodeupdown_t handle, 
			       const char *hostname, 
			       unsigned int port,
			       unsigned int timeout_len,
			       char *reserved)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*backend_module_info->get_updown_data)(handle, 
                                                 hostname,
                                                 port,
                                                 timeout_len,
                                                 reserved);
}

/*
 * _clusterlist_module_callback
 *
 * Check and setup the clusterlist module
 *
 * Returns 1 if is loaded, 0 if not, -1 on fatal error
 */
static int
_clusterlist_module_callback(nodeupdown_t handle, void *dl_handle, void *module_info)
{
  if (!dl_handle || !module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  clusterlist_module_dl_handle = dl_handle;
  clusterlist_module_info = module_info;
  if (!clusterlist_module_info->clusterlist_module_name
      || !clusterlist_module_info->setup
      || !clusterlist_module_info->get_numnodes
      || !clusterlist_module_info->cleanup
      || !clusterlist_module_info->is_node_in_cluster
      || !clusterlist_module_info->get_nodename
      || !clusterlist_module_info->compare_to_clusterlist)
    {
      clusterlist_module_dl_handle = NULL;
      clusterlist_module_info = NULL;
      return 0;
    }

  return 1;
}

int
clusterlist_module_load(nodeupdown_t handle, char *module)
{
  int rv;
  
  if (module)
    {
      char *temp_clusterlist_modules[2];
      char modulebuf[CLUSTERLIST_MODULE_BUFLEN];
      int len;
      
      len = snprintf(modulebuf, 
                     CLUSTERLIST_MODULE_BUFLEN, 
                     "nodeupdown_clusterlist_%s.so",
                     module);

      if (len < 0 || len >= CLUSTERLIST_MODULE_BUFLEN)
        {
          handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_MODULE;
          return -1;
        }

      temp_clusterlist_modules[0] = modulebuf;
      temp_clusterlist_modules[1] = NULL;

      if ((rv = _find_module(handle,
                             temp_clusterlist_modules,
                             CLUSTERLIST_MODULE_SIGNATURE,
                             _clusterlist_module_callback,
                             CLUSTERLIST_MODULE_INFO_SYM)) < 0)
        return -1;

      if (!rv)
        {
          handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_MODULE;
          return -1;
        }

      clusterlist_module_found++;
      return 0;
    }

  if ((rv = _find_module(handle,
                         clusterlist_modules,
                         CLUSTERLIST_MODULE_SIGNATURE,
                         _clusterlist_module_callback,
                         CLUSTERLIST_MODULE_INFO_SYM)) < 0)
    return -1;

  if (rv)
    {
      clusterlist_module_found++;
      return 0;
    }
  
  clusterlist_module_info = &default_clusterlist_module_info;
  return 0;
}

int
clusterlist_module_unload(nodeupdown_t handle)
{
  /* May have not been loaded, so can't close */
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
  clusterlist_module_info = NULL;
  clusterlist_module_found = 0;
  return 0;
}

char *
clusterlist_module_name(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }

  return clusterlist_module_info->clusterlist_module_name;
}

int
clusterlist_module_setup(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->setup)(handle);
}

int
clusterlist_module_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!clusterlist_module_info)
    return 0;

  return (*clusterlist_module_info->cleanup)(handle);
}

int
clusterlist_module_get_numnodes(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->get_numnodes)(handle);
}

int
clusterlist_module_compare_to_clusterlist(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->compare_to_clusterlist)(handle);
}

int
clusterlist_module_is_node_in_cluster(nodeupdown_t handle, const char *node)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->is_node_in_cluster)(handle,
                                                        node);
}

int
clusterlist_module_get_nodename(nodeupdown_t handle,
				const char *node,
				char *buffer,
				unsigned int buflen)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->get_nodename)(handle,
                                                  node,
                                                  buffer,
                                                  buflen);
}

/*
 * _config_module_callback
 *
 * Check and setup the config module
 *
 * Returns 1 if is loaded, 0 if not, -1 on fatal error
 */
static int
_config_module_callback(nodeupdown_t handle, void *dl_handle, void *module_info)
{
  if (!dl_handle || !module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  
  config_module_dl_handle = dl_handle;
  config_module_info = module_info;
  if (!config_module_info->config_module_name
      || !config_module_info->setup
      || !config_module_info->cleanup
      || !config_module_info->load_config)
    {
      config_module_dl_handle = NULL;
      config_module_info = NULL;
      return 0;
    }

  return 1;
}
 
int
config_module_load(nodeupdown_t handle)
{
  int rv;
  
  if ((rv = _find_module(handle,
                         config_modules,
                         CONFIG_MODULE_SIGNATURE,
                         _config_module_callback,
                         CONFIG_MODULE_INFO_SYM)) < 0)
    return -1;

  if (rv)
    return 0;

  config_module_info = &default_config_module_info;
  return 0;
}
 
int
config_module_unload(nodeupdown_t handle)
{
  /* May have not been loaded, so can't close */
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
  config_module_dl_handle = NULL;
  config_module_info = NULL;
  return 0;
}
 
char *
config_module_name(nodeupdown_t handle)
{
  if (!config_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }

  return config_module_info->config_module_name;
}

int
config_module_setup(nodeupdown_t handle)
{
  if (!config_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*config_module_info->setup)(handle);
}
 
int
config_module_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!config_module_info)
    return 0;
 
  return (*config_module_info->cleanup)(handle);
}
  
int
config_module_load_config(nodeupdown_t handle, struct nodeupdown_config *conf)
{
  if (!config_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*config_module_info->load_config)(handle, conf);
}
