/*****************************************************************************\
 *  $Id: nodeupdown_ganglia_clusterlist_util.c,v 1.2 2005-04-01 16:19:32 achu Exp $
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
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_ganglia_clusterlist.h"

int 
nodeupdown_ganglia_clusterlist_parse_filename(nodeupdown_t handle, char **options, char *filename, int filename_len)
{
  int i = 0;
 
  memset(filename, '\0', filename_len);
  while (options[i] != NULL)
    {
      if (strstr(options[i], "filename"))
        {
          char *p = strchr(options[i], '=');
 
          if (!p)
            {
              handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
              return -1;
            }
 
          p++;
          if (p == '\0')
            {
              handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
              return -1;
            }
 
          if (strlen(p) > filename_len)
            {
              handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
              return -1;
            }
          strncpy(filename, p, filename_len);
        }
      else
        {
          handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
          return -1;
        }
 
      i++;
    }
 
  if (filename != NULL)
    {
      struct stat buf;
 
      if (stat(filename, &buf) < 0)
        {
          handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
          return -1;
        }
    }
 
  return 0;
}

int 
nodeupdown_ganglia_clusterlist_copy_nodename(nodeupdown_t handle, const char *node, char *buf, unsigned int buflen)
{
  int len;
 
  len = strlen(node);
 
  if ((len + 1) > buflen)
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
 
  strcpy(buf, node);
 
  return 0;
}
