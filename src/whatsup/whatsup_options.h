/*****************************************************************************\
 *  $Id: whatsup_options.h,v 1.10.2.2 2006-11-09 06:58:55 chu11 Exp $
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

#ifndef _WHATSUP_OPTIONS_H
#define _WHATSUP_OPTIONS_H
 
#define WHATSUP_OPTION_TYPE_GET_NODENAMES     0
#define WHATSUP_OPTION_TYPE_CONVERT_NODENAMES 1
#define WHATSUP_OPTION_TYPE_MONITOR           2

/*
 * struct whatsup_option
 *
 * Describes a whatsup module option
 */
struct whatsup_option {
  char    option;               /* option character */
  char   *option_arg;           /* option argument description if option takes one */
  char   *option_long;          /* optional long option */
  char   *description;          /* description of option */
  int     option_type;          /* which function to call */
};

/*
 * Whatsup_options_setup
 *
 * Setup options module
 *
 * Return 0 on success, -1 on error.
 */
typedef int (*Whatsup_options_setup)(void);

/*
 * Whatsup_options_cleanup
 *
 * Cleanup options module allocations
 *
 * Return 0 on success, -1 on error.
 */
typedef int (*Whatsup_options_cleanup)(void);

/*  
 * Whatsup_options_process_option
 *
 * Handle the option 'c' and possibly the option argument
 * appropriately for a particular module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Whatsup_options_process_option)(char c, char *optarg);

/* 
 * Whatsup_options_get_nodenames
 *
 * Retrieve nodenames specified by the user
 *
 * Returns 0 on success, -1 on error.
 */
typedef int (*Whatsup_options_get_nodenames)(char *buf, int buflen);

/* 
 * Whatsup_options_convert_nodenames
 *
 * Convert the nodes appropriately for the option module and store the
 * result into the buffer.
 *
 * Returns 0 on success, -1 on error.
 */
typedef int (*Whatsup_options_convert_nodenames)(char *nodes, char *buf, int buflen);

/* 
 * Whatsup_options_monitor
 *
 * Monitor up/down node status.
 *
 * Returns 0 on success, -1 on error.
 */
typedef int (*Whatsup_options_monitor)(char *hostname, int port);

/* 
 * struct whatsup_options_module_info
 * 
 * contains options module information and operations.  Required to be
 * defined in each options module.
 */
struct whatsup_options_module_info
{
  char *module_name;
  struct whatsup_option *options;
  Whatsup_options_setup setup;
  Whatsup_options_cleanup cleanup;
  Whatsup_options_process_option process_option;
  Whatsup_options_get_nodenames get_nodenames;
  Whatsup_options_convert_nodenames convert_nodenames;
  Whatsup_options_monitor monitor;
};

#endif /* _WHATSUP_OPTIONS_H */
