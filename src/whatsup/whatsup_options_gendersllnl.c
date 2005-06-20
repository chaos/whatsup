/*****************************************************************************\
 *  $Id: whatsup_options_gendersllnl.c,v 1.11 2005-06-20 21:58:09 achu Exp $
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

/* 
 * gendersllnl_options_setup
 * 
 * gendersllnl setup func
 */
int 
gendersllnl_options_setup(void)
{
  return 0;
}

/* 
 * gendersllnl_options_cleanup
 * 
 * gendersllnl cleanup func
 */
int 
gendersllnl_options_cleanup(void)
{
  return 0;
}

/* 
 * gendersllnl_options_output_usage
 * 
 * gendersllnl output_usage func
 */
int 
gendersllnl_options_output_usage(void)
{
  fprintf(stderr,
	  "  -a         --altnames          List nodes by alternate name\n");
  return 0;
}

/* 
 * gendersllnl_options_options_string
 * 
 * gendersllnl options_string func
 */
char * 
gendersllnl_options_options_string(void)
{
  return "a";
}

/* 
 * gendersllnl_options_register_option
 * 
 * gendersllnl register_option func
 */
int 
gendersllnl_options_register_option(char c, char *options)
{
  if (c == 'a')
    {
      strcat(options, "a");
      gendersllnl_option_a_registered++;
    }

  return 0;
}

/* 
 * gendersllnl_options_add_long_option
 * 
 * gendersllnl add_long_option func
 */
int 
gendersllnl_options_add_long_option(char c, struct option *long_option)
{
#if HAVE_GETOPT_LONG
  if (c == 'a' && gendersllnl_option_a_registered)
    {
      long_option->name = GENDERSLLNL_OPTION_ALTNAME;
      long_option->has_arg = 0;
      long_option->flag = NULL;
      long_option->val = 'a';
    }
#endif /* HAVE_GETOPT_LONG */
  return 0;
}

/* 
 * gendersllnl_options_handle_option
 * 
 * gendersllnl handle_option func
 */
int
gendersllnl_options_handle_option(char c, char *optarg)
{
  if (c != 'a' || !gendersllnl_option_a_registered)
    return -1;

  if (gendersllnl_option_a_registered)
    gendersllnl_list_altnames_flag++;

  return 1;
}

/* 
 * gendersllnl_options_convert_nodenames
 * 
 * gendersllnl convert_nodenames func
 */
int
gendersllnl_options_convert_nodenames(char *nodes, char *buf, int buflen)
{
  genders_t handle = NULL;
                                                                                     
  if (!gendersllnl_list_altnames_flag)
    return 0;

  memset(buf, '\0', buflen);

  if (!(handle = genders_handle_create()))
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
    &gendersllnl_options_setup,
    &gendersllnl_options_cleanup,
    &gendersllnl_options_output_usage,
    &gendersllnl_options_options_string,
    &gendersllnl_options_register_option,
    &gendersllnl_options_add_long_option,
    &gendersllnl_options_handle_option,
    &gendersllnl_options_convert_nodenames
  };
