/*****************************************************************************\
 *  $Id: whatsup_options_gendersllnl.c,v 1.4 2005-04-04 20:23:29 achu Exp $
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

#define _GNU_SOURCE
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */

#include <gendersllnl.h>

#include "whatsup_options.h"
#include "error.h"

#define GENDERSLLNL_OPTION_ALTNAME "altnames"

static int gendersllnl_option_a_registered = 0;

static int gendersllnl_list_altnames_flag = 0;

int 
gendersllnl_options_output_usage()
{
  fprintf(stderr,
	  "  -a         --altnames          List nodes by alternate name\n");
  return 0;
}

int 
gendersllnl_options_add_options(char *options, int maxlen)
{
  if (strchr(options, 'a'))
    return 0;
  
  if (strlen(options) >= maxlen)
    return 0;

  strcat(options, "a");
  gendersllnl_option_a_registered++;
  return 0;
}

#if HAVE_GETOPT_LONG
int 
gendersllnl_options_add_long_options(struct option *long_options, int maxlen)
{
  if (gendersllnl_option_a_registered)
    {
      int i = 0;

      while (long_options[i].name)
	i++;

      if (i >= maxlen)
	return 0;

      long_options[i].name = GENDERSLLNL_OPTION_ALTNAME;
      long_options[i].has_arg = 0;
      long_options[i].flag = NULL;
      long_options[i].val = 'a';
      long_options[i+1].name = NULL;
      long_options[i+1].has_arg = 0;
      long_options[i+1].flag = NULL;
      long_options[i+1].val = 0;
    }

  return 0;
}
#endif /* HAVE_GETOPT_LONG */

int
gendersllnl_options_check_option(int c, char *optarg)
{
  if (c != 'a' || !gendersllnl_option_a_registered)
    return -1;

  if (gendersllnl_option_a_registered)
    gendersllnl_list_altnames_flag++;

  return 1;
}

int
gendersllnl_options_convert_nodenames(char *nodes, char *buf, int buflen)
{
  genders_t handle = NULL;
                                                                                     
  if (!gendersllnl_list_altnames_flag)
    return 0;

  memset(buf, '\0', buflen);

  if ((handle = genders_handle_create()) == NULL)
    err_exit("gendersllnl_options_convert_nodenames: genders_handle_create()");

  if (genders_load_data(handle, NULL) < 0)
    err_exit("gendersllnl_options_convert_nodenames: genders_load_data(): %s",
              genders_errormsg(handle));

  if (genders_string_to_altnames_preserve(handle, nodes, buf, buflen) < 0)
    err_exit("gendersllnl_options_convert_nodenames: "
             "genders_string_to_altnames_preserve(): %s",
              genders_errormsg(handle));

  (void)genders_handle_destroy(handle);
  return 1;
}

struct whatsup_options_module_info options_module_info = 
  {
    "gendersllnl",
    &gendersllnl_options_output_usage,
    &gendersllnl_options_add_options,
    &gendersllnl_options_add_long_options,
    &gendersllnl_options_check_option,
    &gendersllnl_options_convert_nodenames
  };
