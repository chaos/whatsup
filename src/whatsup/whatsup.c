/*****************************************************************************\
 *  $Id: whatsup.c,v 1.89 2005-04-02 01:42:00 achu Exp $
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

#include <errno.h>

#if HAVE_GENDERS
#include <genders.h>
#elif HAVE_GENDERSLLNL
#include <gendersllnl.h>
#endif

#include "nodeupdown.h"
#include "hostlist.h"
#include "fd.h"

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

#define WHATSUP_BUFFERLEN    65536
#define WHATSUP_FORMATLEN    64

/* struct whatsup_info
 * - carries information for the entire program
 */
struct whatsup_info 
{
  nodeupdown_t handle;        /* nodeupdown handle */
  char *hostname;             /* hostname of gmond server */
  int port;                   /* port of gmond server */  
  whatsup_output_t output;    /* output type */ 
  whatsup_list_t list;        /* list type */
  whatsup_bool_t count;       /* list count? */
  hostlist_t nodes;           /* nodes entered at command line */
#if HAVE_GENDERSLLNL
  whatsup_bool_t altnames;    /* list altnames? */
#endif /* HAVE_GENDERSLLNL */
};

/* program name */
char *err_progname;

static void
_err_init(char *progname)
{
  char *ptr = strrchr(progname, '/');
  err_progname = (ptr == NULL) ? progname : ptr + 1;
}

static void 
_err_exit(char *fmt, ...) 
{
  char buffer[WHATSUP_BUFFERLEN];
  va_list ap;
  va_start(ap, fmt);
  snprintf(buffer, WHATSUP_BUFFERLEN, "%s: %s\n", err_progname, fmt);
  vfprintf(stderr, buffer, ap);
  va_end(ap);
  exit(1);
}

static void 
_usage(void) 
{
  fprintf(stderr,
    "Usage: whatsup [OPTIONS]... [NODES]...\n"
    "  -h         --help              Print help and exit\n"
    "  -V         --version           Print version and exit\n"
    "  -o STRING  --hostname=STRING   Gmond server hostname\n"
    "  -p INT     --port=INT          Gmond server port\n"
    "  -b         --updown            List both up and down nodes\n"
    "  -u         --up                List only up nodes\n"
    "  -d         --down              List only down nodes\n"
    "  -t         --count             List only node counts\n"
    "  -l         --hostlist          List nodes in hostlist format\n"
    "  -c         --comma             List nodes in comma separated list\n"
    "  -n         --newline           List nodes in newline separated list\n"
    "  -s         --space             List nodes in space separated list\n");
#if HAVE_GENDERSLLNL
  fprintf(stderr,
    "  -a         --altnames          List nodes by alternate name\n");
#endif /* HAVE_GENDERSLLNL */
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
_push_nodes_on_hostlist(struct whatsup_info *whatsup_info, char *string) 
{
  /* Error if nodes aren't short hostnames */
  if (strchr(string, '.') != NULL)
    _err_exit("nodes must be listed in short hostname format");
        
  if (hostlist_push(whatsup_info->nodes, string) == 0)
    _err_exit("nodes improperly formatted");
}

static void 
_read_nodes_from_stdin(struct whatsup_info *whatsup_info) 
{
  int n;
  char buf[WHATSUP_BUFFERLEN];
  
  if ((n = fd_read_n(STDIN_FILENO, buf, WHATSUP_BUFFERLEN)) < 0)
    _err_exit("error reading from standard input: %s", strerror(errno));
  
  if (n == WHATSUP_BUFFERLEN)
    _err_exit("Overflow standard input buffer"); 

  if (n > 0) 
    {
      char *ptr = strtok(buf, " \t\n\0"); 
      while (ptr != NULL) 
        {
          _push_nodes_on_hostlist(whatsup_info, ptr);
          ptr = strtok(NULL, " \t\n\0");
        }
    }
}

static void
_cmdline_parse(struct whatsup_info *whatsup_info, int argc, char **argv) 
{
  int c, index;
  char options[100];

#if HAVE_GETOPT_LONG
  struct option long_options[] = 
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
#if HAVE_GENDERSLLNL
      {"altnames",  0, NULL, 'a'},
#endif /* HAVE_GENDERSLLNL */
      {0, 0, 0, 0}
  };
#endif /* HAVE_GETOPT_LONG */

  strcpy(options, "hVo:p:budtlcns");
#if HAVE_GENDERSLLNL
  strcat(options, "a");
#endif /* HAVE_GENDERSLLNL */

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
        whatsup_info->hostname = optarg;
        break;
      case 'p':
        whatsup_info->port = atoi(optarg);
        break;
      case 'b':
        whatsup_info->output = UP_AND_DOWN;
        break;
      case 'u':
        whatsup_info->output = UP_NODES;
        break;
      case 'd':
        whatsup_info->output = DOWN_NODES;
        break;
      case 't':
        whatsup_info->count = WHATSUP_TRUE;
        break;
      case 'l':
        whatsup_info->list = WHATSUP_HOSTLIST;
        break;
      case 'c':
        whatsup_info->list = WHATSUP_COMMA;
        break;
      case 'n':
        whatsup_info->list = WHATSUP_NEWLINE;
        break;
      case 's':
        whatsup_info->list = WHATSUP_SPACE;
        break;
#if HAVE_GENDERSLLNL
      case 'a':
        whatsup_info->altnames = WHATSUP_TRUE;
        break;
#endif /* HAVE_GENDERSLLNL */
      default:
        fprintf(stderr, "command line option error\n");
        _usage();
      }
    }

  if ((whatsup_info->nodes = hostlist_create(NULL)) == NULL)
    _err_exit("_cmdline_parse: hostlist_create()");

  index = optind;
  
  /* Read nodes in via command line or standard input */
  if (index < argc) 
    {
      if (strcmp(argv[index], "-") == 0)
        _read_nodes_from_stdin(whatsup_info);
      else 
        {
          while (index < argc) {
            _push_nodes_on_hostlist(whatsup_info, argv[index]);
            index++;
          }
        } 
      
      /* remove any duplicate nodes listed */
      hostlist_uniq(whatsup_info->nodes);
    }
}

/* _check_input_nodes
 * - check if nodes input on command line or stdin are up or down
 */
static void 
_check_input_nodes(struct whatsup_info *whatsup_info, 
                   int up_or_down, 
                   char *buf, 
                   int buflen) 
{
  hostlist_t hl = NULL;
  hostlist_iterator_t iter = NULL;
  char *str = NULL;
  int ret;

  if ((hl = hostlist_create(NULL)) == NULL)
    _err_exit("_check_input_nodes: hostlist_create()");

  if ((iter = hostlist_iterator_create(whatsup_info->nodes)) == NULL)
    _err_exit("_check_input_nodes: hostlist_iterator_create()");

  while ((str = hostlist_next(iter)) != NULL) 
    {
      if (up_or_down == UP_NODES) 
        {
          if ((ret = nodeupdown_is_node_up(whatsup_info->handle, str)) < 0) 
            {
              if (nodeupdown_errnum(whatsup_info->handle) == NODEUPDOWN_ERR_NOTFOUND)
                _err_exit("Unknown node \"%s\"", str);
              else
                _err_exit("_check_input_nodes: nodeupdown_is_node_up(): %s",
                          nodeupdown_errormsg(whatsup_info->handle));
            }
        }
      else 
        {
          if ((ret = nodeupdown_is_node_down(whatsup_info->handle, str)) < 0) 
            {
              if (nodeupdown_errnum(whatsup_info->handle) == NODEUPDOWN_ERR_NOTFOUND)
                _err_exit("Unknown node \"%s\"", str);
              else
                _err_exit("_check_input_nodes: nodeupdown_is_node_down(): %s",
                          nodeupdown_errormsg(whatsup_info->handle));
            }
        }
      
      if (ret == 1) 
        {
          if (hostlist_push_host(hl, str) == 0)
            _err_exit("_check_input_nodes: hostlist_push_host()");
        }
      free(str);
    }
  
  hostlist_sort(hl);
  
  if (hostlist_ranged_string(hl, buflen, buf) < 0)
    _err_exit("_check_input_nodes: hostlist_ranged_string()");
  
  hostlist_iterator_destroy(iter);
  hostlist_destroy(hl);
}
 
/* _get_all_nodes
 * - get all up or down nodes
 */
static void
_get_all_nodes(struct whatsup_info *whatsup_info, 
               int up_or_down, 
               char *buf, 
               int buflen) 
{
  if (up_or_down == UP_NODES) 
    {
      if (nodeupdown_get_up_nodes_string(whatsup_info->handle, buf, buflen) < 0)
        _err_exit("_get_all_nodes: nodeupdown_get_up_nodes_string(): %s", 
                  nodeupdown_errormsg(whatsup_info->handle));
    }
  else 
    {
      if (nodeupdown_get_down_nodes_string(whatsup_info->handle, buf, buflen) < 0)
        _err_exit("_get_all_nodes: nodeupdown_get_down_nodes_string(): %s",
                  nodeupdown_errormsg(whatsup_info->handle));
    }
}

#if HAVE_GENDERSLLNL
static void 
_convert_to_altnames(struct whatsup_info *whatsup_info, 
                     char *buf, 
                     int buflen) 
{
  genders_t handle = NULL;
  char tbuf[WHATSUP_BUFFERLEN];
  int retval = -1;
  int tbuflen = WHATSUP_BUFFERLEN;

  memset(tbuf, '\0', WHATSUP_BUFFERLEN);

  if ((handle = genders_handle_create()) == NULL)
    _err_exit("_convert_to_altnames: genders_handle_create()");

  if (genders_load_data(handle, whatsup_info->filename) < 0)
    _err_exit("_convert_to_altnames: genders_load_data(): %s", 
              genders_errormsg(handle));

  if (genders_string_to_altnames_preserve(handle, buf, tbuf, tbuflen) < 0)
    _err_exit("_convert_to_altnames: genders_string_to_altnames_preserve(): %s",
              genders_errormsg(handle));

  if (strlen(tbuf) < buflen)
    strcpy(buf, tbuf);
  else
    _err_exit("_convert_to_altnames: overflow altname buffer");
    
  (void)genders_handle_destroy(handle);
}
#endif /* HAVE_GENDERSLLNL */

/* _get_nodes
 * - get up or down nodes
 */
static void 
_get_nodes(struct whatsup_info *whatsup_info, 
           int up_or_down, 
           char *buf, 
           int buflen, 
           int *count) 
{
  hostlist_t hl = NULL;
  
  if (hostlist_count(whatsup_info->nodes) > 0)
    _check_input_nodes(whatsup_info, up_or_down, buf, buflen);
  else
    _get_all_nodes(whatsup_info, up_or_down, buf, buflen);
  
#if HAVE_GENDERSLLNL
  if (whatsup_info->altnames == WHATSUP_TRUE)
    _convert_to_altnames(whatsup_info, buf, buflen);
#endif /* HAVE_GENDERSLLNL */

  /* can't use nodeupdown_up/down_count, b/c we may be counting the
   * nodes specified by the user 
   */
  if ((hl = hostlist_create(buf)) == NULL)
    _err_exit("_get_nodes: hostlist_create()");

  *count = hostlist_count(hl);

  hostlist_destroy(hl);
}

static void
_output_nodes(struct whatsup_info *whatsup_info, char *nodebuf) 
{
  char *ptr;
  char tnodebuf[WHATSUP_BUFFERLEN];
  hostlist_t hl = NULL;

  if (whatsup_info->list == WHATSUP_HOSTLIST)
    fprintf(stdout, "%s\n", nodebuf);
  else 
    {
      /* output nodes separated by some break type */
      memset(tnodebuf, '\0', WHATSUP_BUFFERLEN);
    
      if ((hl = hostlist_create(nodebuf)) == NULL)
        _err_exit("_output_nodes: hostlist_create() error");

      if (hostlist_deranged_string(hl, WHATSUP_BUFFERLEN, tnodebuf) < 0)
        _err_exit("_output_nodes: hostlist_deranged_string() error");

      /* convert commas to appropriate break types */
      if (whatsup_info->list != WHATSUP_COMMA) 
        {
          while ((ptr = strchr(tnodebuf, ',')) != NULL)
            *ptr = (char)whatsup_info->list;
        }

      /* start on the next line */
      if (whatsup_info->output == UP_AND_DOWN && whatsup_info->list == WHATSUP_NEWLINE)
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

int 
main(int argc, char *argv[]) 
{
  struct whatsup_info whatsup_info;
  char up_nodes[WHATSUP_BUFFERLEN];
  char down_nodes[WHATSUP_BUFFERLEN];
  char upfmt[WHATSUP_FORMATLEN];
  char downfmt[WHATSUP_FORMATLEN];
  int up_count, down_count, exit_val = 1;
  int buflen = WHATSUP_BUFFERLEN;

  _err_init(argv[0]);

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
  
  /* Initialize whatsup_info structure with defaults */
  whatsup_info.hostname = NULL;
  whatsup_info.port = 0;
  whatsup_info.output = UP_AND_DOWN;
  whatsup_info.list = WHATSUP_HOSTLIST;
  whatsup_info.count = WHATSUP_FALSE;
  whatsup_info.nodes = NULL;
#if HAVE_GENDERSLLNL
  whatsup_info.altnames = WHATSUP_FALSE;
#endif /* HAVE_GENDERSLLNL */
  
  _cmdline_parse(&whatsup_info, argc, argv);
  
  if ((whatsup_info.handle = nodeupdown_handle_create()) == NULL)
    _err_exit("main: nodeupdown_handle_create()");

  if (nodeupdown_load_data(whatsup_info.handle, 
                           whatsup_info.hostname, 
                           whatsup_info.port, 
                           0,
                           NULL) < 0) 
    {
      int errnum = nodeupdown_errnum(whatsup_info.handle);
      
      /* Check for "legit" errors and output appropriate message */
      if (errnum == NODEUPDOWN_ERR_CLUSTERLIST_OPEN)
        _err_exit("Cannot open clusterlist file");
      else if (errnum == NODEUPDOWN_ERR_CLUSTERLIST_READ)
        _err_exit("Cannot read clusterlist file");
      else if (errnum == NODEUPDOWN_ERR_CLUSTERLIST_PARSE)
        _err_exit("Parse error in clusterlist file");
      else if (errnum == NODEUPDOWN_ERR_CONF_OPEN)
        _err_exit("Cannot open conf file");
      else if (errnum == NODEUPDOWN_ERR_CONF_READ)
        _err_exit("Cannot read conf file");
      else if (errnum == NODEUPDOWN_ERR_CONF_PARSE)
        _err_exit("Parse error in conf file");
      else if (errnum == NODEUPDOWN_ERR_CONNECT) 
        _err_exit("Cannot connect to gmond server");
      else if (errnum == NODEUPDOWN_ERR_TIMEOUT)
        _err_exit("Timeout connecting to gmond server");
      else if (errnum == NODEUPDOWN_ERR_HOSTNAME)
        _err_exit("Invalid gmond hostname");
      else if (errnum == NODEUPDOWN_ERR_ADDRESS)
        _err_exit("Invalid gmond address");
      else
        _err_exit("main: nodeupdown_load_data(): %s", 
                  nodeupdown_errormsg(whatsup_info.handle));
    }
  
  /* regardless of options, need all up and down information for exit val */
  _get_nodes(&whatsup_info, UP_NODES, up_nodes, buflen, &up_count);
  _get_nodes(&whatsup_info, DOWN_NODES, down_nodes, buflen, &down_count);
  
  /* only output count */
  if (whatsup_info.count == WHATSUP_TRUE) 
    {
      if (whatsup_info.output == UP_AND_DOWN) 
        {
          _create_formats(upfmt, WHATSUP_FORMATLEN, up_count,
                          downfmt, WHATSUP_FORMATLEN, down_count, "\n");
          fprintf(stdout, upfmt, up_count);
          fprintf(stdout, downfmt, down_count);
        }
      else if (whatsup_info.output == UP_NODES)
        fprintf(stdout, "%d\n", up_count);
      else
        fprintf(stdout, "%d\n", down_count);
    }
  else 
    {    /* output up, down, or both up and down nodes */
      if (whatsup_info.output == UP_AND_DOWN) 
        {
          if (whatsup_info.list != WHATSUP_NEWLINE)
            _create_formats(upfmt, WHATSUP_FORMATLEN, up_count,
                            downfmt, WHATSUP_FORMATLEN, down_count, ": ");
          else 
            {
              /* newline output is funny, thus special */
              snprintf(upfmt,   WHATSUP_FORMATLEN, "up %d:", up_count);
              snprintf(downfmt, WHATSUP_FORMATLEN, "down %d:", down_count);
            }
          
          fprintf(stdout, upfmt, up_count);
          
          _output_nodes(&whatsup_info, up_nodes);
          
          /* handle odd situation with output formatting */
          if (whatsup_info.list == WHATSUP_NEWLINE)
            fprintf(stdout, "\n");
          
          fprintf(stdout, downfmt, down_count);
          
          _output_nodes(&whatsup_info, down_nodes);
        }
      else if (whatsup_info.output == UP_NODES)
        _output_nodes(&whatsup_info, up_nodes);
      else
        _output_nodes(&whatsup_info, down_nodes);
    }
  
  if (whatsup_info.output == UP_AND_DOWN)
    exit_val = 0;
  else if (whatsup_info.output == UP_NODES)
    exit_val = (down_count == 0) ? 0 : 1;
  else
    exit_val = (up_count == 0) ? 0 : 1;
  
  (void)nodeupdown_handle_destroy(whatsup_info.handle);
  hostlist_destroy(whatsup_info.nodes);
  exit(exit_val);
}
