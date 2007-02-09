/*****************************************************************************\
 *  $Id: nodeupdown_genders_util.c,v 1.8 2007-02-09 05:51:16 chu11 Exp $
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
#include <genders.h>

#include "nodeupdown.h"
#include "nodeupdown_genders_util.h"
#include "nodeupdown/nodeupdown_devel.h"

int 
genders_util_setup(nodeupdown_t handle, 
                   genders_t *genders_handle,
                   char *filename)
{
  if (!(*genders_handle = genders_handle_create()))
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_OUTMEM);
      goto cleanup;
    }

  if (genders_load_data(*genders_handle, filename) < 0)
    {
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      goto cleanup;
    }

  return 0;

 cleanup:
  genders_handle_destroy(*genders_handle);
  *genders_handle = NULL;
  return -1;
}

int 
genders_util_cleanup(nodeupdown_t handle, genders_t *genders_handle) 
{
  genders_handle_destroy(*genders_handle);
  *genders_handle = NULL;
  return 0;
}
