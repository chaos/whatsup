/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist_hostsfile.c,v 1.1 2005-03-31 22:44:22 achu Exp $
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
#include <ctype.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_ganglia_clusterlist.h"
#include "hostlist.h"
#include "fd.h"
#include "list.h"

static List hosts = NULL;
static char *hostsfile_file = NULL;

int
hostsfile_ganglia_clusterlist_parse_options(char **options)
{
  return 0;
}

static int
_readline(nodeupdown_t handle, int fd, char *buf, int buflen)
{
  int ret;
                                                                                     
  if ((ret = fd_read_line(fd, buf, buflen)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST_READ;
      return -1;
    }
                                                                                     
  /* buflen - 1 b/c fd_read_line guarantees null termination */
  if (ret >= (buflen-1)) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST_PARSE;
      return -1;
    }
                                                                                     
  return ret;
}

static int
_remove_comments(char *buf, int buflen)
{
  int i, comment_flag, retlen;
                                                                                     
  if (strchr(buf, '#') == NULL)
    return buflen;
                                                                                     
  i = 0;
  comment_flag = 0;
  retlen = buflen;
  while (i < buflen)
    {
      if (comment_flag)
        {
          buf[i] = '\0';
          retlen--;
        }
                                                                                     
      if (buf[i] == '#')
        {
          buf[i] = '\0';
          comment_flag++;
          retlen--;
        }
      i++;
    }
                                                                                     
  return retlen;
}

static int
_remove_trailing_whitespace(char *buf, int buflen)
{
  char *temp;
                                                                                     
  temp = buf + buflen;
  for (--temp; temp >= buf; temp--)
    {
      if (isspace(*temp))
        *temp = '\0';
      else
        break;
      buflen--;
    }
                                                                                     
  return buflen;
}

static char *
_move_past_whitespace(char *buf)
{
  assert(buf);
                                                                                     
  while (*buf != '\0' && isspace(*buf))
    buf++;
                                                                                     
  if (*buf == '\0')
    return NULL;
                                                                                     
  return buf;
}

static int
_load_hostsfile_data(nodeupdown_t handle)
{
  int fd = -1, len;
  char buf[NODEUPDOWN_BUFFERLEN];
  char *file;

  if (!(hosts = list_create((ListDelF)free)))
    {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
                                                                                     
  if (hostsfile_file)
    file = hostsfile_file;
  else
    file = NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT;
                                                                                     
  if ((fd = open(file, O_RDONLY)) < 0)
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST_OPEN;
      goto cleanup;
    }
                                                                                     
  while ((len = _readline(fd, buf, NODEUPDOWN_BUFFERLEN)) > 0)
    {
      char *hostPtr;
      char *str;
                                                                                     
      if ((len = _remove_comments(buf, len)) == 0)
        continue;
                                                                                     
      if ((len = _remove_trailing_whitespace(buf, len)) == 0)
        continue;
                                                                                     
      if ((hostPtr = _move_past_whitespace(buf)) == NULL)
        continue;
                                                                                     
      if (hostPtr[0] == '\0')
        continue;
                                                                                     
      if (strchr(hostPtr, ' ') || strchr(hostPtr, '\t'))
        {
          handle->errnum = NODEUPDOWN_ERR_MASTERLIST_PARSE;
          goto cleanup;
        }
                                                                                     
      if (strlen(hostPtr) > CEREBRO_MAXNODENAMELEN)
        {
          handle->errnum = NODEUPDOWN_ERR_MASTERLIST_PARSE;
          goto cleanup;
        }
                                                                                     
      if (!(str = strdup(hostPtr)))
        {
          handle->errnum = NODEUPDOWN_ERR_OUTMEM;
          goto cleanup;
        }

      if (!list_append(hosts, str))
        {
          handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
          goto cleanup;
        }
    }
                                                                                     
  close(fd);
  return 0;

 cleanup:
  close(fd);
  return -1;
}

int 
hostsfile_ganglia_clusterlist_init(nodeupdown_t handle, void *ptr) 
{
  return _load_hostsfile_data(handle);
}

int 
hostsfile_ganglia_clusterlist_finish(nodeupdown_t handle) 
{
  handle->max_nodes = list_count(hosts);
  return 0;
}

int 
hostsfile_ganglia_clusterlist_cleanup(nodeupdown_t handle) 
{
  list_destroy(hosts);
  hosts = NULL;
  return 0;
}

int 
hostsfile_ganglia_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  ListIterator itr = NULL;
  char *nodename;
                                                                                     
  if ((itr = list_iterator_create(hosts)) == NULL) 
    {
      handle->errnum = NODEUPDOWN_ERR_MASTERLIST;
      return -1;
    }
                                                                                     
  while (nodename = list_next(itr)) 
    {
      if ((hostlist_find(handle->up_nodes, nodename) == -1)
          && (hostlist_find(handle->down_nodes, nodename) == -1)) 
        {
          /* This node must also be down */
          if (hostlist_push_host(handle->down_nodes, nodename) == 0) 
            {
              handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
              goto cleanup;
            }
      }
    }
  
  list_iterator_destroy(itr);
  hostlist_sort(handle->down_nodes);
  return 0;
  
 cleanup:
  list_iterator_destroy(itr);
  return -1;
}

static int
_find_str(void *x, void *key)
{
  if (strcmp((char *)x, (char *)key) == 0)
    return 1;
  return 0;
}

int 
hostsfile_ganglia_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  void *ptr;
  
  ptr = list_find_first(hosts, _find_str, (void *)node);
  if (ptr != NULL)
    return 1;
  else
    return 0;
}

int 
hostsfile_ganglia_clusterlist_is_node_discovered(nodeupdown_t handle, const char *node) 
{
  void *ptr;

  ptr = list_find_first(hosts, _find_str, (void *)node);
  if (ptr != NULL)
    return 1;
  else
    return 0;
}

int 
hostsfile_ganglia_clusterlist_get_nodename(nodeupdown_t handle, 
                                           const char *node, 
                                           char *buffer, 
                                           int buflen) 
{
  if ((strlen(node) + 1) > buflen) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  strcpy(buffer, node);
  return 0;
}
    
int 
hostsfile_ganglia_clusterlist_increase_max_nodes(nodeupdown_t handle) 
{
  handle->max_nodes++;
  return 0;
}

struct nodeupdown_ganglia_clusterlist_module_info ganglia_clusterlist_module_info =
  {
    "hostsfile";
    &hostsfile_ganglia_clusterlist_parse_options;
    &hostsfile_ganglia_clusterlist_init;
    &hostsfile_ganglia_clusterlist_finish;
    &hostsfile_ganglia_clusterlist_cleanup;
    &hostsfile_ganglia_clusterlist_compare_to_clusterlist;
    &hostsfile_ganglia_clusterlist_is_node_in_cluster;
    &hostsfile_ganglia_clusterlist_is_node_discovered;
    &hostsfile_ganglia_clusterlist_get_nodename;
  };
