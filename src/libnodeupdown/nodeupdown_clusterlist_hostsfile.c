/*****************************************************************************\
 *  $Id: nodeupdown_clusterlist_hostsfile.c,v 1.19 2005-05-07 17:34:42 achu Exp $
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
#include <ctype.h>
#endif /* STDC_HEADERS */
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#include "nodeupdown.h"
#include "nodeupdown_clusterlist_util.h"
#include "nodeupdown/nodeupdown_clusterlist_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"
#include "fd.h"
#include "list.h"

/* 
 * hosts
 *
 * Stores list of all hostnames
 */
static List hosts = NULL;

/*
 * _readline
 *
 * read a line from the hostsfile.  Buffer guaranteed to be null
 * terminated.
 *
 * - fd - file descriptor to read from
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return amount of data read into the buffer
 */
static int
_readline(nodeupdown_t handle, int fd, char *buf, int buflen)
{
  int len;
                                                                                     
  if ((len = fd_read_line(fd, buf, buflen)) < 0) 
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      return -1;
    }
                                                                                     
  /* buflen - 1 b/c fd_read_line guarantees null termination */
  if (len >= (buflen-1)) 
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      return -1;
    }
                                                                                     
  return len;
}

/*
 * _remove_comments
 *
 * remove comments from the buffer
 *
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return length of buffer left after comments were removed
 */
static int
_remove_comments(char *buf, int buflen)
{
  int i, comment_flag, retlen;
                                                                                     
  if (!strchr(buf, '#'))
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

/*
 * _remove_trailing_whitespace
 *
 * remove trailing whitespace from the buffer
 *
 * - buf - buffer pointer
 * - buflen - buffer length
 *
 * Return length of buffer left after trailing whitespace was removed
 */
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

/*
 * _move_past_whitespace
 *
 * move past whitespace at the beginning of the buffer
 *
 * - buf - buffer pointer
 *
 * Return pointer to beginning of first non-whitespace char
 */
static char *
_move_past_whitespace(char *buf)
{
  while (*buf != '\0' && isspace(*buf))
    buf++;
                                                                                     
  if (*buf == '\0')
    return NULL;
                                                                                     
  return buf;
}

/*
 * hostsfile_clusterlist_setup
 *
 * hostsfile clusterlist module setup function
 */
int 
hostsfile_clusterlist_setup(nodeupdown_t handle) 
{
  int fd = -1, len;
  char buf[NODEUPDOWN_BUFFERLEN];
  char *p;

  if (!(hosts = list_create((ListDelF)free)))
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_OUTMEM);
      goto cleanup;
    }
                                                                                    
  if ((fd = open(NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT, O_RDONLY)) < 0)
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      goto cleanup;
    }
                                                                                     
  while ((len = _readline(handle, fd, buf, NODEUPDOWN_BUFFERLEN)) > 0)
    {
      char *hostPtr;
      char *str;
                                                                                     
      if ((len = _remove_comments(buf, len)) == 0)
        continue;
                                                                                     
      if ((len = _remove_trailing_whitespace(buf, len)) == 0)
        continue;
                                                                                     
      if (!(hostPtr = _move_past_whitespace(buf)))
        continue;
                                                                                     
      if (hostPtr[0] == '\0')
        continue;
                                                                                     
      if (strchr(hostPtr, ' ') || strchr(hostPtr, '\t'))
        {
	  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
          goto cleanup;
        }
                                                                                     
      if (strlen(hostPtr) > NODEUPDOWN_MAXHOSTNAMELEN)
        {
	  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
          goto cleanup;
        }
          
      /* Shorten hostname if necessary */
      if ((p = strchr(hostPtr, '.')))
        *p = '\0';
      
      if (!(str = strdup(hostPtr)))
        {
	  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_OUTMEM);
          goto cleanup;
        }

      if (!list_append(hosts, str))
        {
	  nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
          goto cleanup;
        }
    }
                                                                                     
  close(fd);
  return 0;

 cleanup:
  close(fd);
  return -1;
}

/*
 * hostsfile_clusterlist_cleanup
 *
 * hostsfile clusterlist module cleanup function
 */
int 
hostsfile_clusterlist_cleanup(nodeupdown_t handle) 
{
  if (hosts)
    list_destroy(hosts);
  hosts = NULL;
  return 0;
}

/*
 * hostsfile_clusterlist_get_numnodes
 *
 * hostsfile clusterlist module get_numnodes function
 */
int 
hostsfile_clusterlist_get_numnodes(nodeupdown_t handle) 
{
  return list_count(hosts);
}

static int
_find_str(void *x, void *key)
{
  if (strcmp((char *)x, (char *)key) == 0)
    return 1;
  return 0;
}

/*
 * hostsfile_clusterlist_is_node_in_cluster
 *
 * hostsfile clusterlist module is_node_in_cluster function
 */
int 
hostsfile_clusterlist_is_node_in_cluster(nodeupdown_t handle, const char *node) 
{
  char nodebuf[NODEUPDOWN_MAXNODENAMELEN+1];
  char *nodePtr = NULL;
  void *ptr;
  
  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
 
      memset(nodebuf, '\0', NODEUPDOWN_MAXNODENAMELEN+1);
      strncpy(nodebuf, node, NODEUPDOWN_MAXNODENAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  ptr = list_find_first(hosts, _find_str, (void *)nodePtr);
  if (ptr != NULL)
    return 1;
  else
    return 0;
}

/*
 * hostsfile_clusterlist_get_nodename
 *
 * hostsfile clusterlist module get_nodename function
 */
int 
hostsfile_clusterlist_get_nodename(nodeupdown_t handle, 
                                   const char *node, 
                                   char *buf, 
                                   unsigned int buflen) 
{
  char nodebuf[NODEUPDOWN_MAXNODENAMELEN+1];
  char *nodePtr = NULL;

  /* Shorten hostname if necessary */
  if (strchr(node, '.'))
    {
      char *p;
 
      memset(nodebuf, '\0', NODEUPDOWN_MAXNODENAMELEN+1);
      strncpy(nodebuf, node, NODEUPDOWN_MAXNODENAMELEN);
      p = strchr(nodebuf, '.');
      *p = '\0';
      nodePtr = nodebuf;
    }
  else
    nodePtr = (char *)node;

  return nodeupdown_clusterlist_copy_nodename(handle, nodePtr, buf, buflen);
}
    
/*
 * hostsfile_clusterlist_compare_to_clusterlist
 *
 * hostsfile clusterlist module compare_to_clusterlist function
 */
int 
hostsfile_clusterlist_compare_to_clusterlist(nodeupdown_t handle) 
{
  ListIterator itr = NULL;
  char *nodename;
                                                                                     
  if (!(itr = list_iterator_create(hosts))) 
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_CLUSTERLIST_MODULE);
      return -1;
    }
                                                                                     
  while ((nodename = list_next(itr)))
    {
      if (nodeupdown_not_discovered_check(handle, nodename) < 0)
	goto cleanup;
    }
  
  list_iterator_destroy(itr);
  return 0;
  
 cleanup:
  list_iterator_destroy(itr);
  return -1;
}

#if WITH_STATIC_MODULES
struct nodeupdown_clusterlist_module_info hostsfile_clusterlist_module_info =
#else  /* !WITH_STATIC_MODULES */
struct nodeupdown_clusterlist_module_info clusterlist_module_info =
#endif /* !WITH_STATIC_MODULES */
  {
    "hostsfile",
    &hostsfile_clusterlist_setup,
    &hostsfile_clusterlist_cleanup,
    &hostsfile_clusterlist_get_numnodes,
    &hostsfile_clusterlist_is_node_in_cluster,
    &hostsfile_clusterlist_get_nodename,
    &hostsfile_clusterlist_compare_to_clusterlist,
  };
