/*****************************************************************************\
 *  $Id: whatsup.c,v 1.90 2005-04-02 05:51:16 achu Exp $
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
#include <stdarg.h>
#endif /* STDC_HEADERS */

#define _GNU_SOURCE
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <dirent.h>
#include <errno.h>

#include "nodeupdown.h"
#include "hostlist.h"
#include "error.h"
#include "ltdl.h"
#include "fd.h"

#include "whatsup_options.h"

/* External variables for getopt */
extern char *optarg;
extern int optind, opterr, optopt;

/* Definitions */
typedef enum 
  {
    WHATSUP_TRUE, 
    WHATSUP_FALSE
  } 
whatsup_bool_t;

typedef enum 
  {
    UP_NODES, 
    DOWN_NODES, 
    UP_AND_DOWN
  } 
whatsup_output_t;

typedef enum 
  {
    WHATSUP_HOSTLIST = '\0', /* anything not ',', '\n', or ' ' */
    WHATSUP_COMMA = ',',
    WHATSUP_NEWLINE = '\n',
    WHATSUP_SPACE = ' '
  } 
whatsup_list_t;

#define WHATSUP_BUFFERLEN         65536
#define WHATSUP_FORMATLEN         64
#define WHATSUP_OPTIONS_LEN       64
#define WHATSUP_LONG_OPTIONS_LEN  32
#define WHATSUP_MODULES_LEN       16
#define WHATSUP_MAXPATHLEN        256

/* struct whatsup_data
 * - carries information for the entire program
 */
struct whatsup_data 
{
  nodeupdown_t handle;
  char *hostname;
  int port;
  whatsup_output_t output;
  whatsup_list_t list;
  whatsup_bool_t count;
  hostlist_t input_nodes;
  lt_dlhandle module_handles[WHATSUP_MODULES_LEN];
  struct whatsup_options_module_info *module_info[WHATSUP_MODULES_LEN];
  int module_loaded_count;
};

static char *whatsup_options_modules[] = {
  "whatsup_options_gendersllnl.la",
  NULL
};

static void 
_usage(void) 
{
  fprintf(stderr,
	  "Usage: whatsup [OPTIONS]... [NODES]...\n"
	  "  -h         --help              Print help and exit\n"
	  "  -V         --version           Print version and exit\n"
	  "  -o STRING  --hostname=STRING   Server hostname\n"
	  "  -p INT     --port=INT          Server port\n"
	  "  -b         --updown            List both up and down nodes\n"
	  "  -u         --up                List only up nodes\n"
	  "  -d         --down              List only down nodes\n"
	  "  -t         --count             List only node counts\n"
	  "  -l         --hostlist          List nodes in hostlist format\n"
	  "  -c         --comma             List nodes in comma separated list\n"
	  "  -n         --newline           List nodes in newline separated list\n"
	  "  -s         --space             List nodes in space separated list\n");
  /* Call output */
  fprintf(stderr, "\n");
  exit(1);
}

static void 
_version(void) 
{
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
  exit(1);
}

static void
_push_nodes_on_hostlist(struct whatsup_data *whatsup_data, char *string) 
{
  /* Error if nodes aren't short hostnames */
  if (strchr(string, '.') != NULL)
    err_exit("nodes must be listed in short hostname format");
        
  if (hostlist_push(whatsup_data->input_nodes, string) == 0)
    err_exit("nodes improperly formatted");
}

static void 
_read_nodes_from_stdin(struct whatsup_data *whatsup_data) 
{
  int n;
  char buf[WHATSUP_BUFFERLEN];
  
  if ((n = fd_read_n(STDIN_FILENO, buf, WHATSUP_BUFFERLEN)) < 0)
    err_exit("error reading from standard input: %s", strerror(errno));
  
  if (n == WHATSUP_BUFFERLEN)
    err_exit("Overflow standard input buffer"); 

  if (n > 0) 
    {
      char *ptr = strtok(buf, " \t\n\0"); 
      while (ptr != NULL) 
        {
          _push_nodes_on_hostlist(whatsup_data, ptr);
          ptr = strtok(NULL, " \t\n\0");
        }
    }
}

static void
_cmdline_parse(struct whatsup_data *whatsup_data, int argc, char **argv) 
{
  int c, index;
  char options[WHATSUP_OPTIONS_LEN+1];

#if HAVE_GETOPT_LONG
  struct option long_options[WHATSUP_LONG_OPTIONS_LEN+1] = 
    {
      {"help",      0, NULL, 'h'},
      {"version",   0, NULL, 'V'},
      {"hostname",  1, NULL, 'o'},
      {"port",      1, NULL, 'p'},
      {"updown",    0, NULL, 'b'},
      {"up",        0, NULL, 'u'},
      {"down",      0, NULL, 'd'},
      {"count",     0, NULL, 't'},
      {"hostlist",  0, NULL, 'l'},
      {"comma",     0, NULL, 'c'},
      {"newline",   0, NULL, 'n'},
      {"space",     0, NULL, 's'},
      {0, 0, 0, 0},
  };
#endif /* HAVE_GETOPT_LONG */

  memset(options, '\0', WHATSUP_OPTIONS_LEN+1);
  strncpy(options, "hVo:p:budtlcns", WHATSUP_OPTIONS_LEN);

  /* turn off output messages printed by getopt_long */
  opterr = 0;

#if HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1)
#else
  while ((c = getopt(argc, argv, options)) != -1)
#endif
    {
    switch(c) 
      {
      case 'h':
        _usage();
      case 'V':
        _version();
      case 'o':
        whatsup_data->hostname = optarg;
        break;
      case 'p':
        whatsup_data->port = atoi(optarg);
        break;
      case 'b':
        whatsup_data->output = UP_AND_DOWN;
        break;
      case 'u':
        whatsup_data->output = UP_NODES;
        break;
      case 'd':
        whatsup_data->output = DOWN_NODES;
        break;
      case 't':
        whatsup_data->count = WHATSUP_TRUE;
        break;
      case 'l':
        whatsup_data->list = WHATSUP_HOSTLIST;
        break;
      case 'c':
        whatsup_data->list = WHATSUP_COMMA;
        break;
      case 'n':
        whatsup_data->list = WHATSUP_NEWLINE;
        break;
      case 's':
        whatsup_data->list = WHATSUP_SPACE;
        break;
      case '?':
	/* XXX */
      default:
        fprintf(stderr, "command line option error\n");
        _usage();
      }
    }

  if ((whatsup_data->input_nodes = hostlist_create(NULL)) == NULL)
    err_exit("_cmdline_parse: hostlist_create()");

  index = optind;
  
  /* Read nodes in via command line or standard input */
  if (index < argc) 
    {
      if (strcmp(argv[index], "-") == 0)
        _read_nodes_from_stdin(whatsup_data);
      else 
        {
          while (index < argc) {
            _push_nodes_on_hostlist(whatsup_data, argv[index]);
            index++;
          }
        } 
      
      /* remove any duplicate nodes listed */
      hostlist_uniq(whatsup_data->input_nodes);
    }
}

/* _check_input_nodes
 * - check if nodes input on command line or stdin are up or down
 */
static void 
_check_input_nodes(struct whatsup_data *whatsup_data, 
                   int up_or_down, 
                   char *buf, 
                   int buflen) 
{
  hostlist_t hl = NULL;
  hostlist_iterator_t iter = NULL;
  char *str = NULL;
  int ret;

  if ((hl = hostlist_create(NULL)) == NULL)
    err_exit("_check_input_nodes: hostlist_create()");

  if ((iter = hostlist_iterator_create(whatsup_data->input_nodes)) == NULL)
    err_exit("_check_input_nodes: hostlist_iterator_create()");

  while ((str = hostlist_next(iter)) != NULL) 
    {
      if (up_or_down == UP_NODES) 
        {
          if ((ret = nodeupdown_is_node_up(whatsup_data->handle, str)) < 0) 
            {
              if (nodeupdown_errnum(whatsup_data->handle) == NODEUPDOWN_ERR_NOTFOUND)
                err_exit("Unknown node \"%s\"", str);
              else
                err_exit("_check_input_nodes: nodeupdown_is_node_up(): %s",
                          nodeupdown_errormsg(whatsup_data->handle));
            }
        }
      else 
        {
          if ((ret = nodeupdown_is_node_down(whatsup_data->handle, str)) < 0) 
            {
              if (nodeupdown_errnum(whatsup_data->handle) == NODEUPDOWN_ERR_NOTFOUND)
                err_exit("Unknown node \"%s\"", str);
              else
                err_exit("_check_input_nodes: nodeupdown_is_node_down(): %s",
                          nodeupdown_errormsg(whatsup_data->handle));
            }
        }
      
      if (ret == 1) 
        {
          if (hostlist_push_host(hl, str) == 0)
            err_exit("_check_input_nodes: hostlist_push_host()");
        }
      free(str);
    }
  
  hostlist_sort(hl);
  
  if (hostlist_ranged_string(hl, buflen, buf) < 0)
    err_exit("_check_input_nodes: hostlist_ranged_string()");
  
  hostlist_iterator_destroy(iter);
  hostlist_destroy(hl);
}
 
/* _get_all_nodes
 * - get all up or down nodes
 */
static void
_get_all_nodes(struct whatsup_data *whatsup_data, 
               int up_or_down, 
               char *buf, 
               int buflen) 
{
  if (up_or_down == UP_NODES) 
    {
      if (nodeupdown_get_up_nodes_string(whatsup_data->handle, buf, buflen) < 0)
        err_exit("_get_all_nodes: nodeupdown_get_up_nodes_string(): %s", 
                  nodeupdown_errormsg(whatsup_data->handle));
    }
  else 
    {
      if (nodeupdown_get_down_nodes_string(whatsup_data->handle, buf, buflen) < 0)
        err_exit("_get_all_nodes: nodeupdown_get_down_nodes_string(): %s",
                  nodeupdown_errormsg(whatsup_data->handle));
    }
}

/* _get_nodes
 * - get up or down nodes
 */
static void 
_get_nodes(struct whatsup_data *whatsup_data, 
           int up_or_down, 
           char *buf, 
           int buflen, 
           int *count) 
{
  hostlist_t hl = NULL;
#if 0
  char tempbuf[WHATSUP_BUFFERLEN];
#endif
  
  if (hostlist_count(whatsup_data->input_nodes) > 0)
    _check_input_nodes(whatsup_data, up_or_down, buf, buflen);
  else
    _get_all_nodes(whatsup_data, up_or_down, buf, buflen);

  /* 
     convert - use a tbuf then copy back in
   */

  /* can't use nodeupdown_up/down_count, b/c we may be counting the
   * nodes specified by the user 
   */
  if ((hl = hostlist_create(buf)) == NULL)
    err_exit("_get_nodes: hostlist_create()");

  *count = hostlist_count(hl);

  hostlist_destroy(hl);
}

static void
_output_nodes(struct whatsup_data *whatsup_data, char *nodebuf) 
{
  char *ptr;
  char tnodebuf[WHATSUP_BUFFERLEN];
  hostlist_t hl = NULL;

  if (whatsup_data->list == WHATSUP_HOSTLIST)
    fprintf(stdout, "%s\n", nodebuf);
  else 
    {
      /* output nodes separated by some break type */
      memset(tnodebuf, '\0', WHATSUP_BUFFERLEN);
    
      if ((hl = hostlist_create(nodebuf)) == NULL)
        err_exit("_output_nodes: hostlist_create() error");

      if (hostlist_deranged_string(hl, WHATSUP_BUFFERLEN, tnodebuf) < 0)
        err_exit("_output_nodes: hostlist_deranged_string() error");

      /* convert commas to appropriate break types */
      if (whatsup_data->list != WHATSUP_COMMA) 
        {
          while ((ptr = strchr(tnodebuf, ',')) != NULL)
            *ptr = (char)whatsup_data->list;
        }

      /* start on the next line */
      if (whatsup_data->output == UP_AND_DOWN && whatsup_data->list == WHATSUP_NEWLINE)
        fprintf(stdout, "\n");

      fprintf(stdout,"%s\n", tnodebuf);
      hostlist_destroy(hl);
    }
}

static int 
_log10(int num) 
{
  int count = 0;

  if (num > 0) 
    {
      while ((num /= 10) > 0)
        count++;
    }

  return count;
} 

static void
_create_formats(char *upfmt, int upfmt_len, int up_count,
                char *downfmt, int downfmt_len, int down_count,
                char *endstr)
{
  int max = (up_count > down_count) ? _log10(up_count) : _log10(down_count);
  max++;
  snprintf(upfmt,   upfmt_len,   "up:   %%%dd%s", max, endstr);
  snprintf(downfmt, downfmt_len, "down: %%%dd%s", max, endstr);
}

static void
_load_module(struct whatsup_data *whatsup_data, char *module_path)
{
  int count = whatsup_data->module_loaded_count;
  int i;

  if (count >= WHATSUP_MODULES_LEN)
    return;

  if (!(whatsup_data->module_handles[count] = lt_dlopen(module_path)))
    err_exit("_load_module: lt_dlopen: module_path=%s: %s",
	     module_path, lt_dlerror());

  if (!(whatsup_data->module_info[count] = lt_dlsym(whatsup_data->module_handles[count],
						    "options_module_info")))
    err_exit("_load_module: lt_dlsym: module_path=%s: %s",
	     module_path, lt_dlerror());

  if (!whatsup_data->module_info[count]->options_module_name
      || whatsup_data->module_info[count]->output_usage
      || whatsup_data->module_info[count]->add_options
      || whatsup_data->module_info[count]->add_long_options
      || whatsup_data->module_info[count]->check_option
      || whatsup_data->module_info[count]->convert_nodenames)
    err_exit("_load_module: module_path=%s: invalid module options");
  
  for (i = 0; i < count; i++)
    {
      if (!strcmp(whatsup_data->module_info[i]->options_module_name,
		  whatsup_data->module_info[count]->options_module_name))
	goto already_loaded;
    }

  whatsup_data->module_loaded_count++;
  return;

 already_loaded:
  whatsup_data->module_info[count] = NULL;
  lt_dlclose(whatsup_data->module_handles[count]);
  return;
}

static void
_load_modules_in_dir(struct whatsup_data *whatsup_data, char *search_dir)
{
  DIR *dir;
  int i = 0;
                                                                                                
  /* Can't open the directory? we assume it doesn't exit, so its not
   * an error.
   */
  if (!(dir = opendir(search_dir)))
    return;

  while (whatsup_options_modules[i])
    {
      struct dirent *dirent;
                                                                                                
      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, whatsup_options_modules[i]))
            {
              char filebuf[WHATSUP_MAXPATHLEN+1];
                                                                                                
              memset(filebuf, '\0', WHATSUP_MAXPATHLEN+1);
              snprintf(filebuf, WHATSUP_MAXPATHLEN, "%s/%s",
                       search_dir, whatsup_options_modules[i]);
                                                                                                
              _load_module(whatsup_data, filebuf);
            }
        }
      rewinddir(dir);
      i++;
    }
}

static void
_load_options_modules(struct whatsup_data *whatsup_data)
{
  if (lt_dlinit() != 0)
    err_exit("_load_modules: lt_dlinit: %s", lt_dlerror());

  _load_modules_in_dir(whatsup_data, WHATSUP_MODULE_BUILDDIR);
  _load_modules_in_dir(whatsup_data, WHATSUP_MODULE_DIR);
  _load_modules_in_dir(whatsup_data, ".");
}

int 
main(int argc, char *argv[]) 
{
  struct whatsup_data whatsup_data;
  char up_nodes[WHATSUP_BUFFERLEN];
  char down_nodes[WHATSUP_BUFFERLEN];
  char upfmt[WHATSUP_FORMATLEN];
  char downfmt[WHATSUP_FORMATLEN];
  int up_count, down_count, exit_val = 1;
  int buflen = WHATSUP_BUFFERLEN;
  int i;

  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);

  /* easter eggs */
  if (argc == 2) 
    {
      if (strcasecmp(argv[1],"doc") == 0)
        fprintf(stderr,"Shhhhhh.  Be very very quiet.  I'm hunting wabbits.\n");
      if (strcasecmp(argv[1],"dude") == 0)
        fprintf(stderr,"Surfs up man! Cowabunga!\n");
      if (strcasecmp(argv[1],"man") == 0)
        fprintf(stderr, "Nothin much, just chillin ...\n");
    }
  
  /* Initialize whatsup_data structure with defaults */
  whatsup_data.hostname = NULL;
  whatsup_data.port = 0;
  whatsup_data.output = UP_AND_DOWN;
  whatsup_data.list = WHATSUP_HOSTLIST;
  whatsup_data.count = WHATSUP_FALSE;
  whatsup_data.input_nodes = NULL;
  whatsup_data.module_loaded_count = 0;
  /* 
   * Load modules before calling cmdline_parse, b/c modules may need
   * additional command line options parsed.
   */
  _load_options_modules(&whatsup_data);

  _cmdline_parse(&whatsup_data, argc, argv);
  
  if ((whatsup_data.handle = nodeupdown_handle_create()) == NULL)
    err_exit("main: nodeupdown_handle_create()");

  if (nodeupdown_load_data(whatsup_data.handle, 
                           whatsup_data.hostname, 
                           whatsup_data.port, 
                           0,
                           NULL) < 0) 
    {
      int errnum = nodeupdown_errnum(whatsup_data.handle);
      
      /* Check for "legit" errors and output appropriate message */
      if (errnum == NODEUPDOWN_ERR_CLUSTERLIST_OPEN)
        err_exit("Cannot open clusterlist file");
      else if (errnum == NODEUPDOWN_ERR_CLUSTERLIST_READ)
        err_exit("Cannot read clusterlist file");
      else if (errnum == NODEUPDOWN_ERR_CLUSTERLIST_PARSE)
        err_exit("Parse error in clusterlist file");
      else if (errnum == NODEUPDOWN_ERR_CONF_OPEN)
        err_exit("Cannot open conf file");
      else if (errnum == NODEUPDOWN_ERR_CONF_READ)
        err_exit("Cannot read conf file");
      else if (errnum == NODEUPDOWN_ERR_CONF_PARSE)
        err_exit("Parse error in conf file");
      else if (errnum == NODEUPDOWN_ERR_CONNECT) 
        err_exit("Cannot connect to server");
      else if (errnum == NODEUPDOWN_ERR_TIMEOUT)
        err_exit("Timeout connecting to server");
      else if (errnum == NODEUPDOWN_ERR_HOSTNAME)
        err_exit("Invalid hostname");
      else if (errnum == NODEUPDOWN_ERR_ADDRESS)
        err_exit("Invalid address");
      else
        err_exit("main: nodeupdown_load_data(): %s", 
		 nodeupdown_errormsg(whatsup_data.handle));
    }
  
  /* regardless of options, need all up and down information for exit val */
  _get_nodes(&whatsup_data, UP_NODES, up_nodes, buflen, &up_count);
  _get_nodes(&whatsup_data, DOWN_NODES, down_nodes, buflen, &down_count);
  
  /* only output count */
  if (whatsup_data.count == WHATSUP_TRUE) 
    {
      if (whatsup_data.output == UP_AND_DOWN) 
        {
          _create_formats(upfmt, WHATSUP_FORMATLEN, up_count,
                          downfmt, WHATSUP_FORMATLEN, down_count, "\n");
          fprintf(stdout, upfmt, up_count);
          fprintf(stdout, downfmt, down_count);
        }
      else if (whatsup_data.output == UP_NODES)
        fprintf(stdout, "%d\n", up_count);
      else
        fprintf(stdout, "%d\n", down_count);
    }
  else 
    {    /* output up, down, or both up and down nodes */
      if (whatsup_data.output == UP_AND_DOWN) 
        {
          if (whatsup_data.list != WHATSUP_NEWLINE)
            _create_formats(upfmt, WHATSUP_FORMATLEN, up_count,
                            downfmt, WHATSUP_FORMATLEN, down_count, ": ");
          else 
            {
              /* newline output is funny, thus special */
              snprintf(upfmt,   WHATSUP_FORMATLEN, "up %d:", up_count);
              snprintf(downfmt, WHATSUP_FORMATLEN, "down %d:", down_count);
            }
          
          fprintf(stdout, upfmt, up_count);
          
          _output_nodes(&whatsup_data, up_nodes);
          
          /* handle odd situation with output formatting */
          if (whatsup_data.list == WHATSUP_NEWLINE)
            fprintf(stdout, "\n");
          
          fprintf(stdout, downfmt, down_count);
          
          _output_nodes(&whatsup_data, down_nodes);
        }
      else if (whatsup_data.output == UP_NODES)
        _output_nodes(&whatsup_data, up_nodes);
      else
        _output_nodes(&whatsup_data, down_nodes);
    }
  
  if (whatsup_data.output == UP_AND_DOWN)
    exit_val = 0;
  else if (whatsup_data.output == UP_NODES)
    exit_val = (down_count == 0) ? 0 : 1;
  else
    exit_val = (up_count == 0) ? 0 : 1;
  
  (void)nodeupdown_handle_destroy(whatsup_data.handle);
  hostlist_destroy(whatsup_data.input_nodes);
  for (i = 0; i < whatsup_data.module_loaded_count; i++)
    {
      lt_dlclose(whatsup_data.module_handles[i]);
      i++;
    }
  lt_dlexit();
  exit(exit_val);
}
