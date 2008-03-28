/*****************************************************************************\
 *  $Id: pingd_config.h,v 1.6 2008-03-28 17:06:38 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2008 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
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
 *  with Whatsup.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifndef _PINGD_CONFIG_H
#define _PINGD_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "hostlist.h"

#define PINGD_DEBUG_DEFAULT           0
#define PINGD_PING_PERIOD_DEFAULT     15000
#define PINGD_SERVER_PORT_DEFAULT     9125

struct pingd_config
{
#ifndef NDEBUG
  int debug;
#endif /* NDEBUG */
  char *config_file;

  int ping_period;
  int pingd_server_port;
  hostlist_t hosts;
};

void pingd_config_setup(int argc, char **argv);

#endif /* _PINGD_CONFIG_H */
