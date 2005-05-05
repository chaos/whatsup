/*****************************************************************************\
 *  $Id: nodeupdown.c,v 1.129 2005-05-05 18:09:56 achu Exp $
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

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_backend.h"
#include "nodeupdown_backend_module.h"
#include "nodeupdown_clusterlist.h"
#include "nodeupdown_clusterlist_module.h"
#include "nodeupdown_config.h"
#include "nodeupdown_config_module.h"
#include "nodeupdown_util.h"
#include "conffile.h"
#include "hostlist.h"
#include "list.h"
#include "ltdl.h"

/* 
 * nodeupdown_errmsg
 * 
 * error messages array
 */
static char * nodeupdown_errmsg[] = 
  {
    "success",
    "nodeupdown handle is null",
    "connection to server error",
    "connection to server timeout",
    "improper hostname error",
    "improper address error",
    "network error",
    "data already loaded",
    "data not loaded",
    "array or string not large enough to store result",
    "incorrect parameters",
    "null pointer reached in list",
    "out of memory",
    "node not found",
    "open clusterlist database error",
    "read clusterlist database error",
    "parse clusterlist database error",
    "internal clusterlist module error",
    "internal backend module error",
    "internal config module error",
    "open config file error",
    "read config file error",
    "parse config file error",
    "invalid config file input error",
    "internal config file error",
    "internal XML parse error",
    "internal hostlist error",
    "illegal nodeupdown handle",
    "internal system error",
    "error number out of range",
  };

/* 
 * _handle_error_check
 * 
 * standard handle error checker
 *
 * Returns -1 on error, 0 on success
 */
static int 
_handle_error_check(nodeupdown_t handle) 
{
  if (!handle || handle->magic != NODEUPDOWN_MAGIC_NUM)
    return -1;

  return 0;
}

/* 
 * _unloaded_handle_error_check
 * 
 * standard unloaded handle error checker
 *
 * Returns -1 on error, 0 on success
 */
static int 
_unloaded_handle_error_check(nodeupdown_t handle) 
{
  if (_handle_error_check(handle) < 0)
    return -1;

  if (handle->is_loaded) 
    {
      handle->errnum = NODEUPDOWN_ERR_ISLOADED;
      return -1;
    }

  return 0;
}

/* 
 * _loaded_handle_error_check
 * 
 * standard loaded handle error checker
 *
 * Returns -1 on error, 0 on success
 */
static int 
_loaded_handle_error_check(nodeupdown_t handle) 
{
  if (_handle_error_check(handle) < 0)
    return -1;

  if (!handle->is_loaded) 
    {
      handle->errnum = NODEUPDOWN_ERR_NOTLOADED;
      return -1;
    }

  return 0;
}

/* 
 * _initialize_handle
 *
 * initialize nodeupdown handle 
 */
static void 
_initialize_handle(nodeupdown_t handle) 
{
  handle->magic = NODEUPDOWN_MAGIC_NUM;
  handle->is_loaded = 0;
  handle->up_nodes = NULL;
  handle->down_nodes = NULL;
  handle->max_nodes = 0;
}

nodeupdown_t 
nodeupdown_handle_create() 
{
  nodeupdown_t handle;

  if (!(handle = (nodeupdown_t)malloc(sizeof(struct nodeupdown))))
    return NULL;
  
  _initialize_handle(handle);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle;
}

/* 
 * _free_handle_data
 *
 * free nodeupdown handle data
 */
static void 
_free_handle_data(nodeupdown_t handle) 
{
  nodeupdown_backend_cleanup(handle);
  nodeupdown_backend_unload_module(handle);
  nodeupdown_clusterlist_cleanup(handle);
  nodeupdown_clusterlist_unload_module(handle);
  nodeupdown_config_cleanup(handle);
  nodeupdown_config_unload_module(handle);
  hostlist_destroy(handle->up_nodes);
  hostlist_destroy(handle->down_nodes);
  _initialize_handle(handle);
#if !WITH_STATIC_MODULES
  lt_dlexit();
#endif /* !WITH_STATIC_MODULES */
}

int 
nodeupdown_handle_destroy(nodeupdown_t handle) 
{
  if (_handle_error_check(handle) < 0)
    return -1;

  _free_handle_data(handle);

  /* "clean" magic number */ 
  handle->magic = ~NODEUPDOWN_MAGIC_NUM;

  free(handle);
  return 0;
}  

/* 
 * _cb_hostnames
 *
 * callback function for configuration parsing of hostnames option.
 *
 * Returns -1 on error, 0 on success
 */
static int 
_cb_hostnames(conffile_t cf, struct conffile_data *data, char *optionname,
              int option_type, void *option_ptr, int option_data,
              void *app_ptr, int app_data) 
{
  struct nodeupdown_config *conf = option_ptr;
  int i;

  if (!option_ptr)
    {
      conffile_seterrnum(cf, CONFFILE_ERR_PARAMETERS);
      return -1;
    }
  
  if (data->stringlist_len > NODEUPDOWN_CONF_HOSTNAMES_MAX)
    return -1;

  for (i = 0; i < data->stringlist_len; i++) 
    {
      if (strlen(data->stringlist[i]) > NODEUPDOWN_MAXHOSTNAMELEN)
        return -1;
      strcpy(conf->hostnames[i], data->stringlist[i]);
    }
  conf->hostnames_len = data->stringlist_len;
  return 0;
}

/*  
 * _init_nodeupdown_config
 *
 * initialize nodeupdown_config structure with defaults
 *
 * Return 0 on success, -1 on error
 */
static int
_init_nodeupdown_config(nodeupdown_t handle, struct nodeupdown_config *conf)
{
  memset(conf, '\0', sizeof(struct nodeupdown_config));
  return 0;
}

/* 
 * _read_conffile
 *
 * read and parse the nodeupdown configuration file
 *
 * Returns 0 on success, -1 on error
 */
static int 
_read_conffile(nodeupdown_t handle, struct nodeupdown_config *conf) 
{
  struct conffile_option options[] = 
    {
      {NODEUPDOWN_CONF_HOSTNAMES, CONFFILE_OPTION_LIST_STRING, -1, 
       _cb_hostnames, 1, 0, &(conf->hostnames_flag),
       conf, 0},
      {NODEUPDOWN_CONF_PORT, CONFFILE_OPTION_INT, 0, 
       conffile_int, 1, 0, &(conf->port_flag), &(conf->port), 0},
      {NODEUPDOWN_CONF_TIMEOUT_LEN, CONFFILE_OPTION_INT, 0, 
       conffile_int, 1, 0, &(conf->timeout_len_flag), &(conf->timeout_len), 0},
      /* 
       * Older options to be ignored by conffile library
       */
      {NODEUPDOWN_CONF_GMOND_HOSTNAME, CONFFILE_OPTION_IGNORE, 0, 
       conffile_empty, 0, 0, NULL, NULL, 0},
      {NODEUPDOWN_CONF_GMOND_HOSTNAMES, CONFFILE_OPTION_IGNORE, 0, 
       conffile_empty, 0, 0, NULL, NULL, 0},
      {NODEUPDOWN_CONF_GMOND_IP, CONFFILE_OPTION_IGNORE, 0, 
       conffile_empty, 0, 0, NULL, NULL, 0},
      {NODEUPDOWN_CONF_GMOND_PORT, CONFFILE_OPTION_IGNORE, 0, 
       conffile_empty, 0, 0, NULL, NULL, 0},
      {NODEUPDOWN_CONF_HOSTSFILE, CONFFILE_OPTION_IGNORE, 0, 
       conffile_empty, 0, 0, NULL, NULL, 0},
      {NODEUPDOWN_CONF_GENDERSFILE, CONFFILE_OPTION_IGNORE, 0, 
       conffile_empty, 0, 0, NULL, NULL, 0},

    };
  conffile_t cf = NULL;
  int num, ret = -1;
  
  if (!(cf = conffile_handle_create())) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

  /* NODEUPDOWN_CONF_FILE defined in config.h */
  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, NODEUPDOWN_CONF_FILE, options, num, NULL, 0, 0) < 0) 
    {
      /* Not an error if the configuration file does not exist */
      if (conffile_errnum(cf) != CONFFILE_ERR_EXIST) {
        int errnum = conffile_errnum(cf);
        if (errnum == CONFFILE_ERR_OPEN)
          handle->errnum = NODEUPDOWN_ERR_CONF_OPEN;
        else if (errnum == CONFFILE_ERR_READ)
          handle->errnum = NODEUPDOWN_ERR_CONF_READ;
        else if (CONFFILE_IS_PARSE_ERR(errnum))
          handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
        else if (errnum == CONFFILE_ERR_OUTMEM)
          handle->errnum = NODEUPDOWN_ERR_OUTMEM;
        else
          handle->errnum = NODEUPDOWN_ERR_CONF_INTERNAL;
        goto cleanup;
      }
    }

   ret = 0;
 cleanup:
  (void)conffile_handle_destroy(cf);
  return ret;
}

int 
nodeupdown_load_data(nodeupdown_t handle, 
                     const char *hostname, 
                     int port, 
                     int timeout_len, 
                     char *reserved)
{
  struct nodeupdown_config conffile_config;
  struct nodeupdown_config module_config;

  if (_unloaded_handle_error_check(handle) < 0)
    goto cleanup;

#if !WITH_STATIC_MODULES
  if (lt_dlinit() != 0)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
#endif /* !WITH_STATIC_MODULES */

  /* Read configuration before loading a potential backend or
   * clusterlist module to use. The configuration file may indicate
   * which module to load.
   */

  _init_nodeupdown_config(handle, &conffile_config);
  if (_read_conffile(handle, &conffile_config) < 0)
    goto cleanup;

  /* 
   * Load backend module
   */
  if (nodeupdown_backend_load_module(handle) < 0)
    goto cleanup;

  if (nodeupdown_backend_setup(handle) < 0)
    goto cleanup;

  /* 
   * Load clusterlist module
   */
  if (nodeupdown_clusterlist_load_module(handle) < 0)
    goto cleanup;

  if (nodeupdown_clusterlist_setup(handle) < 0)
    goto cleanup;

  /* 
   * Load config module
   */
  if (nodeupdown_config_load_module(handle) < 0)
    goto cleanup;

  if (nodeupdown_config_setup(handle) < 0)
    goto cleanup;

  _init_nodeupdown_config(handle, &module_config);
  if (nodeupdown_config_load_default(handle, &module_config) < 0)
    goto cleanup;

  if (!(handle->up_nodes = hostlist_create(NULL))) 
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

  if (!(handle->down_nodes = hostlist_create(NULL))) 
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

  if (port <= 0)
    {
      if (conffile_config.port_flag)
	{
	  if (conffile_config.port <= 0)
	    {
	      handle->errnum = NODEUPDOWN_ERR_CONF_INPUT;
	      goto cleanup;
	    }
	  port = conffile_config.port;
	}
      else if (module_config.port_flag)
	{
	  if (module_config.port <= 0)
	    {
	      handle->errnum = NODEUPDOWN_ERR_CONFIG_MODULE;
	      goto cleanup;
	    }
	  port = module_config.port;
	}
      else
	{
	  if (nodeupdown_backend_default_port(handle) <= 0)
	    {
	      handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
	      goto cleanup;
	    }
	  port = nodeupdown_backend_default_port(handle);
	}
    }
  
  if (timeout_len <= 0)
    {
      if (conffile_config.timeout_len_flag)
	{
	  if (conffile_config.timeout_len <= 0)
	    {
	      handle->errnum = NODEUPDOWN_ERR_CONF_INPUT;
	      goto cleanup;
	    }
	  timeout_len = conffile_config.timeout_len;
	}
      else if (module_config.timeout_len_flag)
	{
	  if (module_config.timeout_len <= 0)
	    {
	      handle->errnum = NODEUPDOWN_ERR_CONFIG_MODULE;
	      goto cleanup;
	    }
	  timeout_len = module_config.timeout_len;
	}
      else
	{
	  if (nodeupdown_backend_default_timeout_len(handle) <= 0)
	    {
	      handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
	      goto cleanup;
	    }
	  timeout_len = nodeupdown_backend_default_timeout_len(handle);
	}
    }

  if (hostname)
    {
      if (nodeupdown_backend_get_updown_data(handle, 
                                             hostname,
                                             port, 
                                             timeout_len,
                                             reserved) < 0)
        goto cleanup;
    }
  else if (conffile_config.hostnames_flag 
           || module_config.hostnames_flag)
    {
      char (*hostnames)[NODEUPDOWN_MAXHOSTNAMELEN+1];
      int i, hostnames_len;
      
      if (conffile_config.hostnames_flag)
	{
	  hostnames = conffile_config.hostnames;
	  hostnames_len = conffile_config.hostnames_len;
	}
      else
	{
	  hostnames = module_config.hostnames;
	  hostnames_len = module_config.hostnames_len;
	}
      
      for (i = 0; i < hostnames_len; i++)
        {
          if (strlen(hostnames[i]) > 0)
            {
              if (nodeupdown_backend_get_updown_data(handle, 
                                                     hostnames[i],
                                                     port,
                                                     timeout_len,
                                                     reserved) < 0)
                continue;
              else
                break;
            }
        }
      
      if (i > hostnames_len)
        {
          handle->errnum = NODEUPDOWN_ERR_CONNECT;
          goto cleanup;
        }
    }
  else
    {
      char *hostnamePtr;

      if (!(hostnamePtr = nodeupdown_backend_default_hostname(handle)))
        goto cleanup;

      if (nodeupdown_backend_get_updown_data(handle, 
                                             hostnamePtr,
                                             port, 
                                             timeout_len,
                                             reserved) < 0)
        goto cleanup;
    }

  hostlist_sort(handle->up_nodes);
  hostlist_sort(handle->down_nodes);

  if ((handle->max_nodes = nodeupdown_clusterlist_get_numnodes(handle)) < 0)
    goto cleanup;

  /* loading complete */
  handle->is_loaded++;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

 cleanup:
  _free_handle_data(handle);
  return -1;
}

int 
nodeupdown_errnum(nodeupdown_t handle) 
{
  if (!handle)
    return NODEUPDOWN_ERR_NULLHANDLE;
  else if (handle->magic != NODEUPDOWN_MAGIC_NUM)
    return NODEUPDOWN_ERR_MAGIC;

  return handle->errnum;
}

char *
nodeupdown_strerror(int errnum) 
{
  if (errnum < NODEUPDOWN_ERR_SUCCESS || errnum > NODEUPDOWN_ERR_ERRNUMRANGE)
    return nodeupdown_errmsg[NODEUPDOWN_ERR_ERRNUMRANGE];

  return nodeupdown_errmsg[errnum];
}

char *
nodeupdown_errormsg(nodeupdown_t handle) 
{
  return nodeupdown_strerror(nodeupdown_errnum(handle));
}

void 
nodeupdown_perror(nodeupdown_t handle, const char *msg) 
{
  char *errormsg = nodeupdown_strerror(nodeupdown_errnum(handle));

  if (!msg)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}

/* 
 * _get_nodes_string
 *
 * common function for nodeupdown_get_up_nodes_string and
 * nodeupdown_get_down_nodes_string
 *
 * Returns 0 on success, -1 on error
 */
static int 
_get_nodes_string(nodeupdown_t handle, char *buf, int buflen, int up_or_down) 
{
  hostlist_t hl;
 
  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!buf || buflen <= 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }
  
  if (up_or_down == NODEUPDOWN_UP_NODES)
    hl = handle->up_nodes;
  else
    hl = handle->down_nodes;

  if (hostlist_ranged_string(hl, buflen, buf) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_OVERFLOW;
      return -1;
    }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int 
nodeupdown_get_up_nodes_string(nodeupdown_t handle, char *buf, int buflen) 
{
  return _get_nodes_string(handle, buf, buflen, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_get_down_nodes_string(nodeupdown_t handle, char *buf, int buflen) 
{
  return _get_nodes_string(handle, buf, buflen, NODEUPDOWN_DOWN_NODES);
}

/* 
 * _get_nodes_list
 *
 * common function for nodeupdown_get_up_nodes_list and
 * nodeupdown_get_down_nodes_list
 *
 * Returns number of items copied on success, -1 on error
 */
static int 
_get_nodes_list(nodeupdown_t handle, char **list, int len, int up_or_down) 
{
  int count = 0;
  hostlist_t hl;
  hostlist_iterator_t iter;
  char *nodename = NULL;

  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!list || len <= 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    hl = handle->up_nodes;
  else
    hl = handle->down_nodes;

  if (!(iter = hostlist_iterator_create(hl))) 
    {
      handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
      return -1;
    }

  while ((nodename = hostlist_next(iter)) != NULL) 
    {
      if (count >= len) 
        {
          handle->errnum = NODEUPDOWN_ERR_OVERFLOW;
          goto cleanup;
        }

      if (!list[count]) 
        {
          handle->errnum = NODEUPDOWN_ERR_NULLPTR;
          goto cleanup;
        }

      strcpy(list[count], nodename);
      free(nodename);
      count++;
    }
  
  hostlist_iterator_destroy(iter);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return count;

 cleanup:
  free(nodename);
  hostlist_iterator_destroy(iter);
  return -1;
}

int 
nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list, int len) 
{
  return _get_nodes_list(handle, list, len, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list, int len) 
{
  return _get_nodes_list(handle, list, len, NODEUPDOWN_DOWN_NODES);
}

/* 
 * _is_node
 *
 * common function for nodeupdown_is_node_up and
 * nodeupdown_is_node_down
 *
 * Returns bool on success, -1 on error
 */
static int 
_is_node(nodeupdown_t handle, const char *node, int up_or_down) 
{ 
  char buffer[NODEUPDOWN_MAXNODENAMELEN+1];
  int ret, retval = -1;

  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!node) 
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }

  if ((ret = nodeupdown_clusterlist_is_node_discovered(handle, 
                                                       node)) < 0)
    return -1;

  if (!ret) 
    {
      handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
      return -1;
    }

  if (nodeupdown_clusterlist_get_nodename(handle, 
                                          node, 
                                          buffer, 
                                          NODEUPDOWN_MAXNODENAMELEN+1) < 0)
    return -1;

  if (up_or_down == NODEUPDOWN_UP_NODES)
    ret = hostlist_find(handle->up_nodes, buffer);
  else
    ret = hostlist_find(handle->down_nodes, buffer);

  if (ret != -1)
    retval = 1;
  else
    retval = 0;
  
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return retval;
}

int 
nodeupdown_is_node_up(nodeupdown_t handle, const char *node) 
{
  return _is_node(handle, node, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_is_node_down(nodeupdown_t handle, const char *node) 
{
  return _is_node(handle, node, NODEUPDOWN_DOWN_NODES);
}

/* 
 * _node_count
 *
 * common function for nodeupdown_up_count and nodeupdown_down_count
 *
 * Returns count on success, -1 on error
 */
static int 
_node_count(nodeupdown_t handle, int up_or_down) 
{
  int ret;

  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (up_or_down == NODEUPDOWN_UP_NODES)
    ret = hostlist_count(handle->up_nodes);
  else
    ret = hostlist_count(handle->down_nodes);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return ret;
}

int 
nodeupdown_up_count(nodeupdown_t handle) 
{
  return _node_count(handle, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_down_count(nodeupdown_t handle) 
{
  return _node_count(handle, NODEUPDOWN_DOWN_NODES);
}

int 
nodeupdown_nodelist_create(nodeupdown_t handle, char ***list) 
{
  int i, j;
  char **nodes;
  
  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!list) 
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }

  if (!(nodes = (char **)malloc(sizeof(char *) * handle->max_nodes))) 
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }

  for (i = 0; i < handle->max_nodes; i++) 
    {
      if (!(nodes[i] = (char *)malloc(NODEUPDOWN_MAXNODENAMELEN+1))) 
        {
          for (j = 0; j < i; j++) 
            free(nodes[j]);
          free(nodes);
          
          handle->errnum = NODEUPDOWN_ERR_OUTMEM;
          return -1;
        }
      memset(nodes[i], '\0', NODEUPDOWN_MAXNODENAMELEN+1);
    }

  *list = nodes;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle->max_nodes;
}

int 
nodeupdown_nodelist_clear(nodeupdown_t handle, char **list) 
{
  int i;
  
  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!list) 
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }

  for (i = 0; i < handle->max_nodes; i++) 
    {
      if (!list[i]) 
        {
          handle->errnum = NODEUPDOWN_ERR_NULLPTR;
          return -1;
        }
      memset(list[i], '\0', NODEUPDOWN_MAXNODENAMELEN+1);
    }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int 
nodeupdown_nodelist_destroy(nodeupdown_t handle, char **list) 
{
  int i;

  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!list) 
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }

  for (i = 0; i < handle->max_nodes; i++)
    free(list[i]);
  free(list);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

void 
nodeupdown_set_errnum(nodeupdown_t handle, int errnum) 
{
  if (_handle_error_check(handle) < 0)
    return;

  if (errnum >= NODEUPDOWN_ERR_SUCCESS && errnum <= NODEUPDOWN_ERR_ERRNUMRANGE) 
    handle->errnum = errnum;
  else
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
}
