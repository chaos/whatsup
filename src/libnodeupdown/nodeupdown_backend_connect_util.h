/*****************************************************************************\
 *  $Id: nodeupdown_backend_connect_util.h,v 1.5 2010-02-02 00:01:58 chu11 Exp $
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

#ifndef _NODEUPDOWN_BACKEND_CONNECT_UTIL_H
#define _NODEUPDOWN_BACKEND_CONNECT_UTIL_H

#include "nodeupdown.h"

/*
 * _low_timeout_connect
 *
 * Setup a tcp connection to 'hostname' and 'port' using a connection
 * timeout of 'connect_timeout'.
 *
 * Return file descriptor on success, -1 on error.
 */
int
_low_timeout_connect(nodeupdown_t handle,
                     const char *hostname,
                     int port,
                     int connect_timeout);


#endif /* _NODEUPDOWN_BACKEND_CONNECT_UTIL_H */
