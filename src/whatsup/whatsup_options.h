/*****************************************************************************\
 *  $Id: whatsup_options.h,v 1.4 2005-04-06 17:24:04 achu Exp $
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
 
#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#define _GNU_SOURCE
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */


/*
 * Whatsup_options_options_list
 * Return string of options desired from module
 * 
 * Whatsup_options_add_options
 * - Add additional options to the options array
 *
 * Whatsup_options_add_long_option
 * - Add addtional long options to the long options array
 *
 * Whatsup_options_check_option
 * - Check if the option is for this module, and take appropriate action
 * 
 * Whatsup_options_convert_nodenames
 * - Conversion function for node names
 */

/*
 * Whatsup_options_output_usage
 *
 * Output additional usage lines for options specified by this module
 *
 * Return 0 on success, -1 on error.
 */
typedef int (*Whatsup_options_output_usage)(void);

/*
 * Whatsup_options_options_string
 *
 * Return a string of the characters this module would like to
 * register.  The string only contains characters, none of the special
 * characters such as ':' that can be passed to getopt().
 *
 * Returns string on success, NULL on error.
 */
typedef char *(*Whatsup_options_options_string)(void);

/*
 * Whatsup_options_register_option
 *
 * Registers an option in the module and updates the options array
 * appropriately.  Module is responsible for copying both the
 * character and any additional characters (i.e. ':' for an argument)
 * into the options arrays.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Whatsup_options_register_option)(char c, char *options);

/*
 * Whatsup_options_add_long_option
 *
 * If long options are supported, add the long options values into the
 * structure pointed to by long_option.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Whatsup_options_add_long_option)(char c, struct option *long_option);

/*  
 * Whatsup_options_check_option
 *
 * Handle the option 'c' and possibly the option argument
 * appropriately for a particular module.
 *
 * Returns 0 on success, -1 on error
 */
typedef int (*Whatsup_options_check_option)(char c, char *optarg);

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
 * struct whatsup_options_module_info
 * 
 * contains options module information and operations.  Required to be
 * defined in each options module.
 */
struct whatsup_options_module_info
{
  char *options_module_name;
  Whatsup_options_output_usage output_usage;
  Whatsup_options_options_string options_string;
  Whatsup_options_register_option register_option;
  Whatsup_options_add_long_option add_long_option;
  Whatsup_options_check_option check_option;
  Whatsup_options_convert_nodenames convert_nodenames;
};

#endif /* _WHATSUP_OPTIONS_H */
