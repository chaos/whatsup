/*****************************************************************************\
 *  $Id: whatsup_options.h,v 1.2 2005-04-04 16:17:58 achu Exp $
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
 * Definitions for Whatsup options modules
 *
 * Whatsup_options_output_usage
 * - Output additional usage lines for options specified by this module
 *
 * Whatsup_options_add_options
 * - Add additional options to the options array
 *
 * Whatsup_options_add_long_options
 * - Add addtional long options to the long options array
 *
 * Whatsup_options_check_option
 * - Check if the option is for this module, and take appropriate action
 * 
 * Whatsup_options_convert_nodenames
 * - Conversion function for node names
 */

typedef int (*Whatsup_options_output_usage)(void);
typedef int (*Whatsup_options_add_options)(char *options, int maxlen);
typedef int (*Whatsup_options_add_long_options)(struct option *long_options, int maxlen);
typedef int (*Whatsup_options_check_option)(int c, char *optarg);
typedef int (*Whatsup_options_convert_nodenames)(char *nodes, char *buf, int buflen);

struct whatsup_options_module_info
{
  char *options_module_name;
  Whatsup_options_output_usage output_usage;
  Whatsup_options_add_options add_options;
  Whatsup_options_add_long_options add_long_options;
  Whatsup_options_check_option check_option;
  Whatsup_options_convert_nodenames convert_nodenames;
};

#endif /* _WHATSUP_OPTIONS_H */
