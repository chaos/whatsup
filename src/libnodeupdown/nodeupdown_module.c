/*****************************************************************************\
 *  $Id: nodeupdown_module.c,v 1.12 2005-05-10 23:30:03 achu Exp $
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

#if WITH_STATIC_MODULES
extern struct nodeupdown_backend_module_info ganglia_backend_module_info;
#if WITH_CEREBRO
extern struct nodeupdown_backend_module_info cerebro_backend_module_info;
#endif /* WITH_CEREBRO */

#if WITH_GENDERSLLNL
extern struct nodeupdown_clusterlist_module_info gendersllnl_clusterlist_module_info;
extern struct nodeupdown_config_module_info gendersllnl_config_module_info;
#endif /* WITH_GENDERSLLNL */
#if WITH_GENDERS
extern struct nodeupdown_clusterlist_module_info genders_clusterlist_module_info;
#endif /* WITH_GENDERS */
#if WITH_HOSTSFILE
extern struct nodeupdown_clusterlist_module_info hostsfile_clusterlist_module_info;
#endif /* WITH_HOSTSFILE */

static struct nodeupdown_backend_module_info *backend_modules[] =
  {
#if WITH_CEREBRO
    &cerebro_backend_module_info,
#endif /* WITH_CEREBRO */
    &ganglia_backend_module_info,
    NULL
  };

static struct nodeupdown_clusterlist_module_info *clusterlist_modules[] =
  {
#if WITH_GENDERSLLNL
    &gendersllnl_clusterlist_module_info,
#endif /* WITH_GENDERSLLNL */
#if WITH_GENDERS
    &genders_clusterlist_module_info,
#endif /* WITH_GENDERS */
#if WITH_HOSTSFILE
    &hostsfile_clusterlist_module_info,
#endif /* WITH_HOSTSFILE */
    NULL
  };

static struct nodeupdown_config_module_info *config_modules[] =
  {
#if WITH_GENDERSLLNL
    &gendersllnl_config_module_info,
#endif /* WITH_GENDERSLLNL */
    NULL
  };

#else  /* !WITH_STATIC_MODULES */
static char *backend_modules[] = {
  "nodeupdown_backend_cerebro.la",
  "nodeupdown_backend_ganglia.la",
  NULL
};
static int backend_modules_len = 1;

static char *clusterlist_modules[] = {
  "nodeupdown_clusterlist_gendersllnl.la",
  "nodeupdown_clusterlist_genders.la",
  "nodeupdown_clusterlist_hostsfile.la",
  NULL
};
static int clusterlist_modules_len = 3;

static char *config_modules[] = {
  "nodeupdown_config_gendersllnl.la",
  NULL
};
static int config_modules_len = 1;

static lt_dlhandle backend_module_dl_handle = NULL;
static lt_dlhandle clusterlist_module_dl_handle = NULL;
static lt_dlhandle config_module_dl_handle = NULL;

#endif /* !WITH_STATIC_MODULES */

static struct nodeupdown_backend_module_info *backend_module_info = NULL;

static struct nodeupdown_clusterlist_module_info *clusterlist_module_info = NULL;

static struct nodeupdown_config_module_info *config_module_info = NULL;

extern struct nodeupdown_clusterlist_module_info default_clusterlist_module_info;

extern struct nodeupdown_config_module_info default_config_module_info;

static int clusterlist_module_found = 0;

static int config_module_found = 0;

/* 
 * Nodeupdown_util_load_module
 *
 * Define a load module function to be passed to
 * nodeupdown_util_lookup_module().
 *
 * Returns 1 if module is found and loaded successfully, 0 if module
 * cannot be found, -1 on fatal error.
 */
typedef int (*Nodeupdown_util_load_module)(nodeupdown_t, char *);

/* 
 * _nodeupdown_util_lookup_module
 *
 * Search the directory 'search_dir' for any one of the modules listed
 * in 'modules_list'.  Call 'load_module' on any discovered module.
 *
 * Returns 1 if a module is found and loaded, 0 if a module is not
 * found, -1 on fatal error.
 */
static int
_nodeupdown_util_lookup_module(nodeupdown_t handle, 
			       char *search_dir,
			       char **modules_list,
			       int modules_list_len,
			       Nodeupdown_util_load_module load_module)
{
  DIR *dir;
  int i = 0, found = 0, rv = -1;
 
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
              char filebuf[NODEUPDOWN_MAXPATHLEN+1];
              int flag;

              memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
              snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
                       search_dir, modules_list[i]);

              if ((flag = load_module(handle, filebuf)) < 0)
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
 * _nodeupdown_util_search_for_module
 *
 * Search the directory 'search_dir' for any module with the given signature.
 *
 * Returns 1 when a module is found, 0 when one is not, -1 on fatal error
 */
static int
_nodeupdown_util_search_for_module(nodeupdown_t handle,
				   char *search_dir,
				   char *signature,
				   Nodeupdown_util_load_module load_module)
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
          char filebuf[NODEUPDOWN_MAXPATHLEN+1];
          int flag;
 
          /*
           * Don't bother trying to load this file unless its a shared
           * object file or libtool file.
           */
          ptr = strchr(dirent->d_name, '.');
          if (!ptr || !(!strcmp(ptr, ".la") || !strcmp(ptr, ".so")))
            continue;
 
          memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
          snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
                   search_dir, dirent->d_name);
           
          if ((flag = load_module(handle, filebuf)) < 0)
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
 * _backend_module_load
 *
 * Load the specified backend module
 * 
 * Returns 1 if module is found and loaded successfully, 0 if module
 * cannot be found, -1 on fatal error.
 */
static int
_backend_module_load(nodeupdown_t handle, 
#if WITH_STATIC_MODULES
	     struct nodeupdown_backend_module_info *module_info
#else  /* !WITH_STATIC_MODULES */
	     char *module_path
#endif /* !WITH_STATIC_MODULES */
	     )
{
#if !WITH_STATIC_MODULES
  struct stat buf;
#endif /* !WITH_STATIC_MODULES */

#if WITH_STATIC_MODULES
  backend_module_info = module_info;
#else  /* !WITH_STATIC_MODULES */
  if (stat(module_path, &buf) < 0)
    return 0;

  if (!(backend_module_dl_handle = lt_dlopen(module_path)))
    {
      handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
      goto cleanup;
    }

  if (!(backend_module_info = (struct nodeupdown_backend_module_info *)lt_dlsym(backend_module_dl_handle, "backend_module_info")))
    {
      handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!backend_module_info->backend_module_name
      || !backend_module_info->default_hostname
      || !backend_module_info->default_port
      || !backend_module_info->default_timeout_len
      || !backend_module_info->setup
      || !backend_module_info->cleanup
      || !backend_module_info->get_updown_data)
    {
      handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
      goto cleanup;
    }

  return 1;

 cleanup:
#if !WITH_STATIC_MODULES
  if (backend_module_dl_handle)
    lt_dlclose(backend_module_dl_handle);
  backend_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  backend_module_info = NULL;
  return -1;
}

int 
_nodeupdown_backend_module_load(nodeupdown_t handle)
{
#if WITH_STATIC_MODULES
  struct nodeupdown_backend_module_info **ptr;
  int i = 0;
  
  ptr = &backend_modules[0];
  while (ptr[i] != NULL)
    {
      int rv;
      
      if (!ptr[i]->backend_module_name)
	continue;
      
      if ((rv = _backend_module_load(handle, backend_modules[i])) < 0)
	goto cleanup;
      
      if (rv)
	goto found;
      
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  int rv;
  
  if ((rv = _nodeupdown_util_lookup_module(handle,
					   NODEUPDOWN_MODULE_BUILDDIR,
					   backend_modules,
					   backend_modules_len,
					   _backend_module_load)) < 0)
    goto cleanup;
  
  if (rv)
    goto found;
  
  if ((rv = _nodeupdown_util_lookup_module(handle,
					   NODEUPDOWN_MODULE_DIR,
					   backend_modules,
					   backend_modules_len,
					   _backend_module_load)) < 0)
    goto cleanup;
  
  if (rv)
    goto found;

  if ((rv = _nodeupdown_util_search_for_module(handle,
					       NODEUPDOWN_MODULE_DIR,
					       "nodeupdown_backend_",
					       _backend_module_load)) < 0)
    goto cleanup;
  
  handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
  goto cleanup;

#endif /* !WITH_STATIC_MODULES */

 found:
  return 0;

 cleanup:
  return -1;
}

int
_nodeupdown_backend_module_unload(nodeupdown_t handle)
{
#if !WITH_STATIC_MODULES
  /* May have not been loaded, so can't close */
  if (backend_module_dl_handle)
    lt_dlclose(backend_module_dl_handle);
  backend_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  backend_module_info = NULL;
  return 0;
}
 
char *
_nodeupdown_backend_module_name(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }
  return backend_module_info->backend_module_name;
}

char *
_nodeupdown_backend_module_default_hostname(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }
  return (*backend_module_info->default_hostname)(handle);
}

int
_nodeupdown_backend_module_default_port(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  return (*backend_module_info->default_port)(handle);
}

int
_nodeupdown_backend_module_default_timeout_len(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  return (*backend_module_info->default_timeout_len)(handle);
}

int 
_nodeupdown_backend_module_setup(nodeupdown_t handle)
{
  if (!backend_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  return (*backend_module_info->setup)(handle);
}

int 
_nodeupdown_backend_module_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!backend_module_info)
    return 0;

  return (*backend_module_info->cleanup)(handle);
}
 
int 
_nodeupdown_backend_module_get_updown_data(nodeupdown_t handle, 
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
 * _clusterlist_module_load
 *
 * Load the specified clusterlist module
 *
 * Returns 1 if module is found and loaded successfully, 0 if module
 * cannot be found, -1 on fatal error.
 */
static int
_clusterlist_module_load(nodeupdown_t handle,
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
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }

  if (!(clusterlist_module_info = (struct nodeupdown_clusterlist_module_info *)lt_dlsym(clusterlist_module_dl_handle, "clusterlist_module_info")))
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_MODULE;
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  if (!clusterlist_module_info->clusterlist_module_name
      || !clusterlist_module_info->setup
      || !clusterlist_module_info->get_numnodes
      || !clusterlist_module_info->cleanup
      || !clusterlist_module_info->is_node_in_cluster
      || !clusterlist_module_info->get_nodename
      || !clusterlist_module_info->compare_to_clusterlist)
    {
      handle->errnum = NODEUPDOWN_ERR_CLUSTERLIST_MODULE;
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
_nodeupdown_clusterlist_module_load(nodeupdown_t handle)
{
#if WITH_STATIC_MODULES
  struct nodeupdown_clusterlist_module_info **ptr;
  int i = 0;

  ptr = &clusterlist_modules[0];
  while (ptr[i] != NULL)
    {
      int rv;

      if (!ptr[i]->clusterlist_module_name)
        continue;

      if ((rv = _clusterlist_module_load(handle, clusterlist_modules[i])) < 0)
        goto cleanup;

      if (rv)
        goto found;

      i++;
    }

  if (!ptr[i])
    clusterlist_module_info = &default_clusterlist_module_info;

#else  /* !WITH_STATIC_MODULES */
  int rv;

  if ((rv = _nodeupdown_util_lookup_module(handle,
					   NODEUPDOWN_MODULE_BUILDDIR,
					   clusterlist_modules,
					   clusterlist_modules_len,
					   _clusterlist_module_load)) < 0)
    goto cleanup;

  if (rv)
    goto found;

  if ((rv = _nodeupdown_util_lookup_module(handle,
					   NODEUPDOWN_MODULE_DIR,
					   clusterlist_modules,
					   clusterlist_modules_len,
					   _clusterlist_module_load)) < 0)
    goto cleanup;
                      
  if (rv)
    goto found;

  if ((rv = _nodeupdown_util_search_for_module(handle,
					       NODEUPDOWN_MODULE_DIR,
					       "nodeupdown_clusterlist_",
					       _clusterlist_module_load)) < 0)
    goto cleanup;

  if (rv)
    goto found;

  clusterlist_module_info = &default_clusterlist_module_info;

#endif /* !WITH_STATIC_MODULES */

  return 0;
  
 found:
  clusterlist_module_found++;
  return 0;

 cleanup:
  return -1;
}

int
_nodeupdown_clusterlist_module_unload(nodeupdown_t handle)
{
#if !WITH_STATIC_MODULES
  /* May have not been loaded, so can't close */
  if (clusterlist_module_dl_handle)
    lt_dlclose(clusterlist_module_dl_handle);
  clusterlist_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  clusterlist_module_info = NULL;
  clusterlist_module_found = 0;
  return 0;
}

int 
_nodeupdown_clusterlist_module_found(nodeupdown_t handle)
{
  return (clusterlist_module_found) ? 1 : 0;
}

char *
_nodeupdown_clusterlist_module_name(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }

  return clusterlist_module_info->clusterlist_module_name;
}

int
_nodeupdown_clusterlist_module_setup(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->setup)(handle);
}

int
_nodeupdown_clusterlist_module_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!clusterlist_module_info)
    return 0;

  return (*clusterlist_module_info->cleanup)(handle);
}

int
_nodeupdown_clusterlist_module_get_numnodes(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->get_numnodes)(handle);
}

int
_nodeupdown_clusterlist_module_compare_to_clusterlist(nodeupdown_t handle)
{
  if (!clusterlist_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*clusterlist_module_info->compare_to_clusterlist)(handle);
}

int
_nodeupdown_clusterlist_module_is_node_in_cluster(nodeupdown_t handle,
						  const char *node)
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
_nodeupdown_clusterlist_module_get_nodename(nodeupdown_t handle,
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
 * _config_module_load
 *
 * Load the specified config module
 *
 * Returns 1 if module is found and loaded successfully, 0 if module
 * cannot be found, -1 on fatal error.
 */
static int
_config_module_load(nodeupdown_t handle,
#if WITH_STATIC_MODULES
             struct nodeupdown_config_module_info *module_info
#else  /* !WITH_STATIC_MODULES */
             char *module_path
#endif /* !WITH_STATIC_MODULES */
             )
{
#if !WITH_STATIC_MODULES
  struct stat buf;
#endif /* !WITH_STATIC_MODULES */
 
#if WITH_STATIC_MODULES
  config_module_info = module_info;
#else  /* !WITH_STATIC_MODULES */
  if (stat(module_path, &buf) < 0)
    return 0;
 
  if (!(config_module_dl_handle = lt_dlopen(module_path)))
    {
      handle->errnum = NODEUPDOWN_ERR_CONFIG_MODULE;
      goto cleanup;
    }
 
  if (!(config_module_info = (struct nodeupdown_config_module_info *)lt_dlsym(config_module_dl_handle, "config_module_info")))
    {
      handle->errnum = NODEUPDOWN_ERR_CONFIG_MODULE;
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */
 
  if (!config_module_info->config_module_name
      || !config_module_info->setup
      || !config_module_info->cleanup
      || !config_module_info->load_default)
    {
      handle->errnum = NODEUPDOWN_ERR_CONFIG_MODULE;
      goto cleanup;
    }
 
  return 1;
 
 cleanup:
#if !WITH_STATIC_MODULES
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
  config_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  config_module_info = NULL;
  return -1;
}
 
int
_nodeupdown_config_module_load(nodeupdown_t handle)
{
#if WITH_STATIC_MODULES
  struct nodeupdown_config_module_info **ptr;
  int i = 0;
   
  ptr = &config_modules[0];
  while (ptr[i] != NULL)
    {
      int rv;
       
      if (!ptr[i]->config_module_name)
        continue;
       
      if ((rv = _config_module_load(handle, config_modules[i])) < 0)
        goto cleanup;
       
      if (rv)
        goto found;
       
      i++;
    }

  if (!ptr[i])
    config_module_info = &default_config_module_info;

#else  /* !WITH_STATIC_MODULES */
  int rv;
   
  if ((rv = _nodeupdown_util_lookup_module(handle,
					   NODEUPDOWN_MODULE_BUILDDIR,
					   config_modules,
					   config_modules_len,
					   _config_module_load)) < 0)
    goto cleanup;
   
  if (rv)
    goto found;
   
  if ((rv = _nodeupdown_util_lookup_module(handle,
					   NODEUPDOWN_MODULE_DIR,
					   config_modules,
					   config_modules_len,
					   _config_module_load)) < 0)
    goto cleanup;
   
  if (rv)
    goto found;
   
  if ((rv = _nodeupdown_util_search_for_module(handle,
					       NODEUPDOWN_MODULE_DIR,
					       "nodeupdown_config_",
					       _config_module_load)) < 0)
    goto cleanup;
 
  if (rv)
    goto found;

  config_module_info = &default_config_module_info;

#endif /* !WITH_STATIC_MODULES */

  return 0;

 found:
  config_module_found++;
  return 0;
 
 cleanup:
  return -1;
}
 
int
_nodeupdown_config_module_unload(nodeupdown_t handle)
{
#if !WITH_STATIC_MODULES
  /* May have not been loaded, so can't close */
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
  config_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  config_module_info = NULL;
  config_module_found = 0;
  return 0;
}
 
int 
_nodeupdown_config_module_found(nodeupdown_t handle)
{
  return (config_module_found) ? 1 : 0;
}
 
char *
_nodeupdown_config_module_name(nodeupdown_t handle)
{
  if (!config_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }

  return config_module_info->config_module_name;
}

int
_nodeupdown_config_module_setup(nodeupdown_t handle)
{
  if (!config_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*config_module_info->setup)(handle);
}
 
int
_nodeupdown_config_module_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!config_module_info)
    return 0;
 
  return (*config_module_info->cleanup)(handle);
}
  
int
_nodeupdown_config_module_load_default(nodeupdown_t handle,
				       struct nodeupdown_config *conf)
{
  if (!config_module_info)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  return (*config_module_info->load_default)(handle, conf);
}
