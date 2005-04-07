/*****************************************************************************\
 *  $Id: nodeupdown_backend_ganglia.c,v 1.2 2005-04-06 21:50:19 achu Exp $
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
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_backend.h"
#include "nodeupdown_clusterlist.h"
#include "nodeupdown_util.h"
#include "hostlist.h"
#include "xmlparse.h"

/* Used to pass multiple variables as one during XML parsing */
struct parse_vars 
{
  nodeupdown_t handle;
  int timeout_len;
  unsigned long localtime;
};

#define GANGLIA_BACKEND_DEFAULT_PORT        8649
#define GANGLIA_BACKEND_DEFAULT_TIMEOUT_LEN 60
#define GANGLIA_BACKEND_CONNECT_LEN         5 

char ganglia_default_hostname[NODEUPDOWN_MAXHOSTNAMELEN+1];

/*
 * ganglia_backend_default_hostname
 *
 * ganglia backend module default_hostname function
 */
char *
ganglia_backend_default_hostname(nodeupdown_t handle)
{
  memset(ganglia_default_hostname, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
  if (gethostname(ganglia_default_hostname, NODEUPDOWN_MAXHOSTNAMELEN) < 0)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return NULL;
    }
  return &ganglia_default_hostname[0];
}

/*
 * ganglia_backend_default_port
 *
 * ganglia backend module default_port function
 */
int 
ganglia_backend_default_port(nodeupdown_t handle)
{
  return GANGLIA_BACKEND_DEFAULT_PORT;
}

/*
 * ganglia_backend_default_timeout_len
 *
 * ganglia backend module default_timeout_len function
 */
int 
ganglia_backend_default_timeout_len(nodeupdown_t handle)
{
  return GANGLIA_BACKEND_DEFAULT_TIMEOUT_LEN;
}

/*
 * ganglia_backend_init
 *
 * ganglia backend module init function
 */
int 
ganglia_backend_init(nodeupdown_t handle)
{
  /* nothing to do */
  return 0;
}

/*
 * ganglia_backend_cleanup
 *
 * ganglia backend module cleanup function
 */
int
ganglia_backend_cleanup(nodeupdown_t handle)
{
  /* nothing to do */
  return 0;
}

/* 
 * _xml_start
 *
 * Callback for xml parser.  Parses beginning tags like <FOO attr1=X
 * attr2=Y>
 */
static void 
_xml_parse_start(void *data, const char *e1, const char **attr) 
{
  nodeupdown_t handle = ((struct parse_vars *)data)->handle;
  int timeout_len = ((struct parse_vars *)data)->timeout_len;
  unsigned long localtime = ((struct parse_vars *)data)->localtime;
  char buffer[NODEUPDOWN_MAXHOSTNAMELEN+1];
  unsigned long reported;
  int ret;

  if (strcmp("HOST", e1) == 0) 
    {
      /* attributes of XML HOST tag
       * attr[0] - "NAME"
       * attr[1] - hostname
       * attr[2] - "IP"
       * attr[3] - ip address of host
       * attr[4] - "REPORTED"
       * attr[5] - time gmond received a multicast message from the host
       * - remaining attributes aren't needed 
       */

      if (nodeupdown_clusterlist_is_node_in_cluster(handle, attr[1]) <= 0)
        return;
      
      if (nodeupdown_clusterlist_get_nodename(handle, 
                                                      attr[1], 
                                                      buffer, 
                                                      NODEUPDOWN_MAXHOSTNAMELEN+1) < 0)
        return;
      
      /* store as up or down */
      reported = atol(attr[5]);
      if (abs(localtime - reported) < timeout_len)
        ret = hostlist_push(handle->up_nodes, buffer);
      else
        ret = hostlist_push(handle->down_nodes, buffer);
      
      if (ret == 0)
        return;
      
      if (nodeupdown_clusterlist_increase_max_nodes(handle) < 0)
        return;
    }
}

/* 
 * _xml_start
 *
 * Callback for xml parser.  Parses end tags like </FOO>
 */
static void 
_xml_parse_end(void *data, const char *e1) 
{
  /* nothing to parse at this time */
}

/*
 * ganglia_backend_get_updown_data
 *
 * ganglia backend module get_updown_data function
 */
int 
ganglia_backend_get_updown_data(nodeupdown_t handle, 
                                const char *hostname,
                                int port,
                                int timeout_len,
                                char *reserved) 
{
  XML_Parser xml_parser = NULL;
  struct parse_vars pv;
  struct timeval tv;
  int fd, retval = -1;

  if ((fd = nodeupdown_util_low_timeout_connect(handle,
                                                hostname,
                                                port,
                                                GANGLIA_BACKEND_CONNECT_LEN)) < 0)
    goto cleanup;

  /* Setup parse vars to pass to _xml_parse_start */
  pv.handle = handle;
  pv.timeout_len = timeout_len;

  /* Call gettimeofday at the latest point right before XML stuff. */
  if (gettimeofday(&tv, NULL) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    } 
  pv.localtime = tv.tv_sec;

  /* Following XML parsing loop more or less ripped from libganglia by
   * Matt Massie <massie at CS.Berkeley.EDU>
   */

  xml_parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xml_parser, _xml_parse_start, _xml_parse_end);
  XML_SetUserData(xml_parser, (void *)&pv);

  while (1) 
    {
      int bytes_read;
      void *buff;
      
      if (!(buff = XML_GetBuffer(xml_parser, BUFSIZ))) 
        {
          handle->errnum = NODEUPDOWN_ERR_XML;
          goto cleanup;
        }

      if ((bytes_read = read(fd, buff, BUFSIZ)) < 0) 
        {
          handle->errnum = NODEUPDOWN_ERR_NETWORK;
          goto cleanup;
        }

      if (!XML_ParseBuffer(xml_parser, bytes_read, bytes_read == 0)) 
        {
          handle->errnum = NODEUPDOWN_ERR_XML;
          goto cleanup;
        }
      
      if (bytes_read == 0)
        break;
    }
  
  if (nodeupdown_clusterlist_compare_to_clusterlist(handle) < 0)
    goto cleanup;

  if (nodeupdown_clusterlist_complete_loading(handle) < 0)
    goto cleanup;

  retval = 0;

 cleanup:
  close(fd);
  if (xml_parser != NULL)
    XML_ParserFree(xml_parser);
  return retval;
}

struct nodeupdown_backend_module_info backend_module_info = 
  {
    "ganglia",
    &ganglia_backend_default_hostname,
    &ganglia_backend_default_port,
    &ganglia_backend_default_timeout_len,
    &ganglia_backend_init,
    &ganglia_backend_cleanup,
    &ganglia_backend_get_updown_data
  };