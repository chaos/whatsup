/*****************************************************************************\
 *  $Id: nodeupdown_backend.c,v 1.3 2005-04-06 05:24:47 achu Exp $
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
#include "nodeupdown_backend.h"
#include "nodeupdown_util.h"
#include "ltdl.h"

static char *backend_modules[] = {
  "nodeupdown_backend_ganglia.la",
  NULL
};
static int backend_modules_len = 1;

static lt_dlhandle backend_module_dl_handle = NULL;
static struct nodeupdown_backend_module_info *backend_module_info = NULL;

static int
_load_module(nodeupdown_t handle, char *module_path)
{
  struct stat buf;

  if (stat(module_path, &buf) < 0)
    return 0;

  if (!(backend_module_dl_handle = lt_dlopen(module_path)))
    {
      handle->errnum = NODEUPDOWN_ERR_BACKEND;
      goto cleanup;
    }

  if (!(backend_module_info = (struct nodeupdown_backend_module_info *)lt_dlsym(backend_module_dl_handle, "backend_module_info")))
    {
      handle->errnum = NODEUPDOWN_ERR_BACKEND;
      goto cleanup;
    }

  if (!backend_module_info->backend_module_name
      || !backend_module_info->default_hostname
      || !backend_module_info->default_port
      || !backend_module_info->default_timeout_len
      || !backend_module_info->init
      || !backend_module_info->cleanup
      || !backend_module_info->get_updown_data)
    {
      handle->errnum = NODEUPDOWN_ERR_BACKEND;
      goto cleanup;
    }

  return 1;

 cleanup:
  if (backend_module_dl_handle)
    lt_dlclose(backend_module_dl_handle);
  backend_module_info = NULL;
  backend_module_dl_handle = NULL;
  return -1;
}

int 
nodeupdown_backend_load_module(nodeupdown_t handle, char *backend_module)
{
  int rv;

  if (backend_module)
    {
      char filebuf[NODEUPDOWN_MAXPATHLEN+1];
      
      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
               NODEUPDOWN_MODULE_BUILDDIR, backend_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;
      
      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/nodeupdown_backend_%s.la",
               NODEUPDOWN_MODULE_BUILDDIR, backend_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;
      
      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
               NODEUPDOWN_MODULE_DIR, backend_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/nodeupdown_backend_%s.la",
               NODEUPDOWN_MODULE_DIR, backend_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "./nodeupdown_backend_%s.la",
               backend_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;
      
      if (rv)
        goto done;

      memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
      snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "./%s", backend_module);

      if ((rv = _load_module(handle, filebuf)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      handle->errnum = NODEUPDOWN_ERR_CONF_OPEN;
      goto cleanup;
    }
  else
    {
      if ((rv = nodeupdown_util_search_dir_for_module(handle,
						      NODEUPDOWN_MODULE_DIR,
						      backend_modules,
						      backend_modules_len,
						      _load_module)) < 0)
        goto cleanup;
                     
      if (rv)
        goto done;
                                                                          
      if ((rv = nodeupdown_util_search_dir_for_module(handle,
						      ".",
						      backend_modules,
						      backend_modules_len,
						      _load_module)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      if ((rv = nodeupdown_util_search_dir_for_module(handle,
						      NODEUPDOWN_MODULE_BUILDDIR,
						      backend_modules,
						      backend_modules_len,
						      _load_module)) < 0)
        goto cleanup;

      if (rv)
        goto done;

      handle->errnum = NODEUPDOWN_ERR_BACKEND;
      goto cleanup;
    }

 done:
  return 0;

 cleanup:
  return -1;
}

int
nodeupdown_backend_unload_module(nodeupdown_t handle)
{
  /* May have not been loaded, so can't close */
  if (backend_module_dl_handle)
    lt_dlclose(backend_module_dl_handle);
  backend_module_info = NULL;
  backend_module_dl_handle = NULL;
  return 0;
}
 
char *
nodeupdown_backend_default_hostname(nodeupdown_t handle)
{
  return (*backend_module_info->default_hostname)(handle);
}

int
nodeupdown_backend_default_port(nodeupdown_t handle)
{
  return (*backend_module_info->default_port)(handle);
}

int
nodeupdown_backend_default_timeout_len(nodeupdown_t handle)
{
  return (*backend_module_info->default_timeout_len)(handle);
}

int 
nodeupdown_backend_init(nodeupdown_t handle)
{
  return (*backend_module_info->init)(handle);
}

int 
nodeupdown_backend_cleanup(nodeupdown_t handle)
{
  /* May have not been loaded, so can't cleanup */
  if (!backend_module_info)
    return 0;

  return (*backend_module_info->cleanup)(handle);
}
 
int 
nodeupdown_backend_get_updown_data(nodeupdown_t handle, 
                                   const char *hostname, 
                                   int port,
                                   int timeout_len,
                                   char *reserved)
{
  return (*backend_module_info->get_updown_data)(handle, 
                                                 hostname,
                                                 port,
                                                 timeout_len,
                                                 reserved);
}
