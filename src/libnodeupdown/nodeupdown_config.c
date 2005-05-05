/*****************************************************************************\
 *  $Id: nodeupdown_config.c,v 1.7 2005-05-05 18:20:41 achu Exp $
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
#include "nodeupdown_config.h"
#include "nodeupdown_util.h"
#include "nodeupdown/nodeupdown_config_module.h"
#include "ltdl.h"

#if WITH_STATIC_MODULES

extern struct nodeupdown_config_module_info gendersllnl_config_module_info;

static struct nodeupdown_config_module_info *config_modules[] =
  {
    &gendersllnl_config_module_info,
    NULL
  };

#else  /* !WITH_STATIC_MODULES */
static char *config_modules[] = {
  "nodeupdown_config_gendersllnl.la",
  NULL
};
static int config_modules_len = 1;

static lt_dlhandle config_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */

static struct nodeupdown_config_module_info *config_module_info = NULL;

/* 
 * _load_module
 *
 * Load the specified config module
 * 
 * Returns 1 if module is found and loaded successfully, 0 if module
 * cannot be found, -1 on fatal error.
 */
static int
_load_module(nodeupdown_t handle, 
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
nodeupdown_config_load_module(nodeupdown_t handle)
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
      
      if ((rv = _load_module(handle, config_modules[i])) < 0)
        goto cleanup;
      
      if (rv)
        goto done;
      
      i++;
    }
#else  /* !WITH_STATIC_MODULES */
  int rv;
  
  if ((rv = nodeupdown_util_lookup_module(handle,
					  NODEUPDOWN_MODULE_BUILDDIR,
					  config_modules,
					  config_modules_len,
					  _load_module)) < 0)
    goto cleanup;
  
  if (rv)
    goto done;
  
  if ((rv = nodeupdown_util_lookup_module(handle,
					  NODEUPDOWN_MODULE_DIR,
					  config_modules,
					  config_modules_len,
					  _load_module)) < 0)
    goto cleanup;
  
  
  if ((rv = nodeupdown_util_search_for_module(handle,
                                              NODEUPDOWN_MODULE_DIR,
                                              "nodeupdown_config_",
                                              _load_module)) < 0)
    goto cleanup;

  if (rv)
    goto done;
  
  handle->errnum = NODEUPDOWN_ERR_CONFIG_MODULE;
  goto cleanup;
#endif /* !WITH_STATIC_MODULES */

 done:
  return 0;

 cleanup:
  return -1;
}

int
nodeupdown_config_unload_module(nodeupdown_t handle)
{
#if !WITH_STATIC_MODULES
  /* May have not been loaded, so can't close */
  if (config_module_dl_handle)
    lt_dlclose(config_module_dl_handle);
  config_module_dl_handle = NULL;
#endif /* !WITH_STATIC_MODULES */
  config_module_info = NULL;
  return 0;
}
 
int 
nodeupdown_config_setup(nodeupdown_t handle)
{
  return (*config_module_info->setup)(handle);
}

int 
nodeupdown_config_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!config_module_info)
    return 0;

  return (*config_module_info->cleanup)(handle);
}
 
int 
nodeupdown_config_load_default(nodeupdown_t handle, 
                               struct nodeupdown_config *conf)
{
  return (*config_module_info->load_default)(handle, conf);
}
