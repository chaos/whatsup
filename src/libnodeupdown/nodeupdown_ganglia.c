/*****************************************************************************\
 *  $Id: nodeupdown_ganglia.c,v 1.2 2005-04-01 21:45:25 achu Exp $
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

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h> 

#include "hostlist.h"
#include "xmlparse.h"
#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_ganglia_clusterlist.h"

/* to pass multiple variables as one during XML parsing */
struct parse_vars 
{
  nodeupdown_t handle;
  int timeout_len;
  unsigned long localtime;
};

/* xml start function for use with expat XML parsing library
 * - parse beginning tags like <FOO attr1=X attr2=Y> 
 */
static void 
_xml_parse_start(void *data, const char *e1, const char **attr) 
{
  nodeupdown_t handle = ((struct parse_vars *)data)->handle;
  int timeout_len = ((struct parse_vars *)data)->timeout_len;
  unsigned long localtime = ((struct parse_vars *)data)->localtime;
  char shorthostname[MAXHOSTNAMELEN+1];
  char buffer[MAXHOSTNAMELEN+1];
  unsigned long reported;
  char *ptr;
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

      /* shorten hostname if necessary */
      memset(shorthostname, '\0', MAXHOSTNAMELEN+1);
      strncpy(shorthostname, attr[1], MAXHOSTNAMELEN);
      if ((ptr = strchr(shorthostname, '.')) != NULL)
        *ptr = '\0';
      
      if (nodeupdown_ganglia_clusterlist_is_node_in_cluster(handle, shorthostname) <= 0)
        return;
      
      if (nodeupdown_ganglia_clusterlist_get_nodename(handle, shorthostname, 
                                                      buffer, MAXHOSTNAMELEN+1) < 0)
        return;
      
      /* store as up or down */
      reported = atol(attr[5]);
      if (abs(localtime - reported) < timeout_len)
        ret = hostlist_push(handle->up_nodes, buffer);
      else
        ret = hostlist_push(handle->down_nodes, buffer);
      
      if (ret == 0)
        return;
      
      if (nodeupdown_ganglia_clusterlist_increase_max_nodes(handle) < 0)
        return;
    }
}

/* xml end function for use with expat XML parsing library
 * - parse end tags like </FOO>
 */
static void 
_xml_parse_end(void *data, const char *e1) 
{
  /* nothing to do at this time */
}

int 
nodeupdown_ganglia_get_gmond_data(nodeupdown_t handle, int fd, int timeout_len) 
{
  XML_Parser xml_parser = NULL;
  struct parse_vars pv;
  struct timeval tv;
  int retval = -1;

  /* Setup parse vars to pass to _xml_parse_start */

  pv.handle = handle;

  if (timeout_len <= 0)
    pv.timeout_len = NODEUPDOWN_TIMEOUT_LEN;
  else
    pv.timeout_len = timeout_len;

  /* Call gettimeofday at the latest point right before XML stuff. */
  if (gettimeofday(&tv, NULL) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    } 
  pv.localtime = tv.tv_sec;

  /* Following XML parsing loop more or less ripped from libganglia by
   * Matt Massie <massie@CS.Berkeley.EDU>
   */

  xml_parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xml_parser, _xml_parse_start, _xml_parse_end);
  XML_SetUserData(xml_parser, (void *)&pv);

  while (1) 
    {
      int bytes_read;
      void *buff;
      
      if ((buff = XML_GetBuffer(xml_parser, BUFSIZ)) == NULL) 
        {
          handle->errnum = NODEUPDOWN_ERR_EXPAT;
          goto cleanup;
        }

      if ((bytes_read = read(fd, buff, BUFSIZ)) < 0) 
        {
          handle->errnum = NODEUPDOWN_ERR_NETWORK;
          goto cleanup;
        }

      if (XML_ParseBuffer(xml_parser, bytes_read, bytes_read == 0) == 0) 
        {
          handle->errnum = NODEUPDOWN_ERR_EXPAT;
          goto cleanup;
        }
      
      if (bytes_read == 0)
        break;
    }
  
  retval = 0;
 cleanup:
  if (xml_parser != NULL)
    XML_ParserFree(xml_parser);
  return retval;
}

void
nodeupdown_ganglia_free_data(nodeupdown_t handle)
{
  nodeupdown_ganglia_clusterlist_cleanup(handle);
  nodeupdown_ganglia_clusterlist_unload_module(handle);
}
