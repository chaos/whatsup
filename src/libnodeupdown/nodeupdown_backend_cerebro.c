/*****************************************************************************\
 *  $Id: nodeupdown_backend_cerebro.c,v 1.2 2005-05-10 22:47:22 achu Exp $
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
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_module.h"
#include "nodeupdown_backend_util.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

#include <cerebro.h>
#include <cerebro/cerebro_updown_protocol.h>

char cerebro_default_hostname[NODEUPDOWN_MAXHOSTNAMELEN+1];

/*
 * cerebro_backend_default_hostname
 *
 * cerebro backend module default_hostname function
 */
char *
cerebro_backend_default_hostname(nodeupdown_t handle)
{
  memset(cerebro_default_hostname, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
  if (gethostname(cerebro_default_hostname, NODEUPDOWN_MAXHOSTNAMELEN) < 0)
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      return NULL;
    }
  return &cerebro_default_hostname[0];
}

/*
 * cerebro_backend_default_port
 *
 * cerebro backend module default_port function
 */
int 
cerebro_backend_default_port(nodeupdown_t handle)
{
  return CEREBRO_UPDOWN_SERVER_PORT;
}

/*
 * cerebro_backend_default_timeout_len
 *
 * cerebro backend module default_timeout_len function
 */
int 
cerebro_backend_default_timeout_len(nodeupdown_t handle)
{
  return CEREBRO_UPDOWN_TIMEOUT_LEN_DEFAULT;
}

/*
 * cerebro_backend_setup
 *
 * cerebro backend module setup function
 */
int 
cerebro_backend_setup(nodeupdown_t handle)
{
  return 0;
}

/*
 * cerebro_backend_cleanup
 *
 * cerebro backend module cleanup function
 */
int
cerebro_backend_cleanup(nodeupdown_t handle)
{
  return 0;
}

/*
 * cerebro_backend_get_updown_data
 *
 * cerebro backend module get_updown_data function
 */
int 
cerebro_backend_get_updown_data(nodeupdown_t handle, 
                                const char *hostname,
                                unsigned int port,
                                unsigned int timeout_len,
                                char *reserved) 
{
  int fd, rv = -1;

  if ((fd = _nodeupdown_util_low_timeout_connect(handle,
						 hostname,
						 port,
						 CEREBRO_UPDOWN_PROTOCOL_CONNECT_TIMEOUT_LEN)) < 0)
    goto cleanup;
  
  rv = 0;

 cleanup:
  close(fd);
  return rv;
}

#if WITH_STATIC_MODULES
struct nodeupdown_backend_module_info cerebro_backend_module_info = 
#else  /* !WITH_STATIC_MODULES */
struct nodeupdown_backend_module_info backend_module_info = 
#endif /* !WITH_STATIC_MODULES */
  {
    "cerebro",
    &cerebro_backend_default_hostname,
    &cerebro_backend_default_port,
    &cerebro_backend_default_timeout_len,
    &cerebro_backend_setup,
    &cerebro_backend_cleanup,
    &cerebro_backend_get_updown_data
  };
