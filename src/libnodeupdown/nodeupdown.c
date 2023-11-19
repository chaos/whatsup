/*****************************************************************************\
 *  $Id: nodeupdown.c,v 1.161 2010-02-02 00:01:58 chu11 Exp $
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

#include "nodeupdown.h"
#include "nodeupdown_api.h"
#include "nodeupdown_module.h"
#include "nodeupdown_util.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_clusterlist_module.h"
#include "nodeupdown/nodeupdown_config.h"
#include "nodeupdown/nodeupdown_config_module.h"
#include "nodeupdown/nodeupdown_constants.h"

#include "conffile.h"
#include "hash.h"
#include "hostlist.h"
#include "list.h"
#include "ltdl.h"

extern int clusterlist_module_found;

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
    "data already loaded",
    "data not loaded",
    "array or string not large enough to store result",
    "incorrect parameters",
    "null pointer reached in list",
    "out of memory",
    "node not found",
    "not supported",
    "internal backend module error",
    "internal clusterlist module error",
    "internal config module error",
    "parse config file error",
    "invalid config file input",
    "internal config file error",
    "illegal nodeupdown handle",
    "internal system error",
    "error number out of range",
  };

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
  if (_nodeupdown_handle_error_check(handle) < 0)
    return -1;

  if (handle->load_state == NODEUPDOWN_LOAD_STATE_LOADED)
    {
      handle->errnum = NODEUPDOWN_ERR_ISLOADED;
      return -1;
    }

  if (handle->load_state != NODEUPDOWN_LOAD_STATE_UNLOADED)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
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
  if (_nodeupdown_handle_error_check(handle) < 0)
    return -1;

  if (handle->load_state == NODEUPDOWN_LOAD_STATE_UNLOADED)
    {
      handle->errnum = NODEUPDOWN_ERR_NOTLOADED;
      return -1;
    }

  if (handle->load_state != NODEUPDOWN_LOAD_STATE_LOADED)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
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
  handle->load_state = NODEUPDOWN_LOAD_STATE_UNLOADED;
  handle->up_nodes = NULL;
  handle->down_nodes = NULL;
  handle->numnodes = 0;
  handle->last_up_times = NULL;
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
  backend_module_cleanup(handle);
  backend_module_unload(handle);
  clusterlist_module_cleanup(handle);
  clusterlist_module_unload(handle);
  config_module_cleanup(handle);
  config_module_unload(handle);
  hostlist_destroy(handle->up_nodes);
  hostlist_destroy(handle->down_nodes);
  hash_destroy(handle->last_up_times);
  _initialize_handle(handle);
  lt_dlexit();
}

int
nodeupdown_handle_destroy(nodeupdown_t handle)
{
  if (_nodeupdown_handle_error_check(handle) < 0)
    return -1;

  _free_handle_data(handle);

  /* "clean" magic number */
  handle->magic = ~NODEUPDOWN_MAGIC_NUM;

  free(handle);
  return 0;
}

/*
 * _destroy_last_up_time
 *
 * callback for hash_destroy to destroy last_up_time structure.
 */
static void
_destroy_last_up_time(void *x)
{
  struct last_up_time *lut;

  lut = (struct last_up_time *)x;

  if (lut->node)
    free(lut->node);
  free(lut);
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

  if (data->stringlist_len > NODEUPDOWN_CONFIG_HOSTNAMES_MAX)
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
      /*
       * Current configuration options
       */
      {"hostnames", CONFFILE_OPTION_LIST_STRING, -1,
       _cb_hostnames, 1, 0, &(conf->hostnames_flag),
       conf, 0},
      {"port", CONFFILE_OPTION_INT, 0,
       conffile_int, 1, 0, &(conf->port_flag),
       &(conf->port), 0},
      {"timeout_len", CONFFILE_OPTION_INT, 0,
       conffile_int, 1, 0, &(conf->timeout_len_flag),
       &(conf->timeout_len), 0},
      /*
       * Older options to be ignored by conffile library
       */
      {"gmond_hostname", CONFFILE_OPTION_IGNORE, 0,
       conffile_empty, 0, 0, NULL, NULL, 0},
      {"gmond_hostnames", CONFFILE_OPTION_IGNORE, 0,
       conffile_empty, 0, 0, NULL, NULL, 0},
      {"gmond_ip", CONFFILE_OPTION_IGNORE, 0,
       conffile_empty, 0, 0, NULL, NULL, 0},
      {"gmond_port", CONFFILE_OPTION_IGNORE, 0,
       conffile_empty, 0, 0, NULL, NULL, 0},
      {"hostsfile", CONFFILE_OPTION_IGNORE, 0,
       conffile_empty, 0, 0, NULL, NULL, 0},
      {"gendersfile", CONFFILE_OPTION_IGNORE, 0,
       conffile_empty, 0, 0, NULL, NULL, 0},
    };
  conffile_t cf = NULL;
  int num, rv = -1;

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
        if (CONFFILE_IS_PARSE_ERR(errnum))
          handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
        else if (errnum == CONFFILE_ERR_OUTMEM)
          handle->errnum = NODEUPDOWN_ERR_OUTMEM;
        else
          handle->errnum = NODEUPDOWN_ERR_CONF_INTERNAL;
        goto cleanup;
      }
    }

  rv = 0;
 cleanup:
  (void)conffile_handle_destroy(cf);
  return rv;
}

int
nodeupdown_load_data(nodeupdown_t handle,
                     const char *hostname,
                     int port,
                     int timeout_len,
                     char *module)
{
  struct nodeupdown_config conffile_config;
  struct nodeupdown_config module_config;
  char *preferred_clusterlist_module;

  if (_unloaded_handle_error_check(handle) < 0)
    goto cleanup;

  memset(&conffile_config, '\0', sizeof(struct nodeupdown_config));
  memset(&module_config, '\0', sizeof(struct nodeupdown_config));

  if (lt_dlinit() != 0)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

  /* Read configuration before loading a potential backend or
   * clusterlist module to use. The configuration file may indicate
   * which module to load.
   */
  if (_read_conffile(handle, &conffile_config) < 0)
    goto cleanup;

  /*
   * Load backend module
   */
  if (backend_module_load(handle, module) < 0)
    goto cleanup;

  if (backend_module_setup(handle) < 0)
    goto cleanup;

  preferred_clusterlist_module = backend_module_preferred_clusterlist_module(handle);

  /*
   * Load clusterlist module
   */
  if (clusterlist_module_load(handle, preferred_clusterlist_module) < 0)
    goto cleanup;

  if (clusterlist_module_setup(handle) < 0)
    goto cleanup;

  /*
   * Load config module
   */
  if (config_module_load(handle) < 0)
    goto cleanup;

  if (config_module_setup(handle) < 0)
    goto cleanup;

  if (config_module_load_config(handle, &module_config) < 0)
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

  if (!(handle->last_up_times = hash_create(NODEUPDOWN_LAST_UP_TIMES_SIZE,
                                            (hash_key_f)hash_key_string,
                                            (hash_cmp_f)strcmp,
                                            (hash_del_f)_destroy_last_up_time)))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

  handle->load_state = NODEUPDOWN_LOAD_STATE_SETUP;

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
          if (backend_module_default_port(handle) <= 0)
            {
              handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
              goto cleanup;
            }
          port = backend_module_default_port(handle);
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
          if (backend_module_default_timeout_len(handle) < 0)
            {
              handle->errnum = NODEUPDOWN_ERR_BACKEND_MODULE;
              goto cleanup;
            }
          timeout_len = backend_module_default_timeout_len(handle);
        }
    }

  if (hostname)
    {
      if (backend_module_get_updown_data(handle,
                                         hostname,
                                         port,
                                         timeout_len,
                                         module) < 0)
        goto cleanup;
    }
  else if (conffile_config.hostnames_flag || module_config.hostnames_flag)
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
              if (backend_module_get_updown_data(handle,
                                                 hostnames[i],
                                                 port,
                                                 timeout_len,
                                                 module) < 0)
                continue;
              else
                break;
            }
        }

      if (i >= hostnames_len)
        {
          handle->errnum = NODEUPDOWN_ERR_CONNECT;
          goto cleanup;
        }
    }
  else
    {
      char *hostnamePtr;

      if (!(hostnamePtr = backend_module_default_hostname(handle)))
        goto cleanup;

      if (backend_module_get_updown_data(handle,
                                         hostnamePtr,
                                         port,
                                         timeout_len,
                                         module) < 0)
        goto cleanup;
    }

  if (clusterlist_module_compare_to_clusterlist(handle) < 0)
    goto cleanup;

  hostlist_sort(handle->up_nodes);
  hostlist_sort(handle->down_nodes);

  if ((handle->numnodes = clusterlist_module_get_numnodes(handle)) < 0)
    goto cleanup;

  /* loading complete */
  handle->load_state = NODEUPDOWN_LOAD_STATE_LOADED;

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
  hostlist_iterator_t itr;
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

  if (!(itr = hostlist_iterator_create(hl)))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }

  while ((nodename = hostlist_next(itr)) != NULL)
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

  hostlist_iterator_destroy(itr);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return count;

 cleanup:
  free(nodename);
  hostlist_iterator_destroy(itr);
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
  char *nodePtr;
  int temp, rv = -1;

  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!node)
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }

  if (clusterlist_module_found)
    {
      if ((rv = clusterlist_module_is_node_in_cluster(handle, node)) < 0)
        return -1;

      if (!rv)
        {
          handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
          return -1;
        }

      if (clusterlist_module_get_nodename(handle,
                                          node,
                                          buffer,
                                          NODEUPDOWN_MAXNODENAMELEN+1) < 0)
        return -1;

      nodePtr = buffer;
    }
  else
    {
      /* Special case: We can do better when the default module is
       * loaded.
       */
      if (hostlist_find(handle->up_nodes, node) < 0
          && hostlist_find(handle->down_nodes, node) < 0)
        {
          handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
          return -1;
        }

      nodePtr = (char *)node;
    }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    temp = hostlist_find(handle->up_nodes, nodePtr);
  else
    temp = hostlist_find(handle->down_nodes, nodePtr);

  if (temp != -1)
    rv = 1;
  else
    rv = 0;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return rv;
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
  int rv;

  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (up_or_down == NODEUPDOWN_UP_NODES)
    rv = hostlist_count(handle->up_nodes);
  else
    rv = hostlist_count(handle->down_nodes);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return rv;
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
nodeupdown_last_up_time(nodeupdown_t handle,
                        const char *node,
                        unsigned int *last_up_time)
{
  struct last_up_time *lut;

  if (_loaded_handle_error_check(handle) < 0)
    return -1;

  if (!node || !last_up_time)
    {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      return -1;
    }

  if (hash_is_empty(handle->last_up_times))
    {
      handle->errnum = NODEUPDOWN_ERR_NOTSUPPORTED;
      return -1;
    }

  if (!(lut = hash_find(handle->last_up_times, node)))
    {
      handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
      return -1;
    }

  *last_up_time = lut->last_up_time;
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
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

  if (!(nodes = (char **)malloc(sizeof(char *) * handle->numnodes)))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }

  for (i = 0; i < handle->numnodes; i++)
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
  return handle->numnodes;
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

  for (i = 0; i < handle->numnodes; i++)
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

  for (i = 0; i < handle->numnodes; i++)
    free(list[i]);
  free(list);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}
