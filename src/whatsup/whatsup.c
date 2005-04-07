/*****************************************************************************\
 *  $Id: whatsup.c,v 1.97 2005-04-07 06:12:28 achu Exp $
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
#include <assert.h>
#include <errno.h>

#include "whatsup_options.h"
#include "nodeupdown.h"
#include "hostlist.h"
#include "error.h"
#include "ltdl.h"
#include "fd.h"

/* 
 * External variables for getopt 
 */
extern char *optarg;
extern int optind, opterr, optopt;

/* 
 * Definitions 
 */
#define WHATSUP_TRUE              1
#define WHATSUP_FALSE             0

#define WHATSUP_UP_NODES          0 
#define WHATSUP_DOWN_NODES        1
#define WHATSUP_UP_AND_DOWN       2

#define WHATSUP_HOSTLIST          '\0'
#define WHATSUP_COMMA             ','
#define WHATSUP_NEWLINE           '\n'
#define WHATSUP_SPACE             ' '

#define WHATSUP_BUFFERLEN         65536
#define WHATSUP_FORMATLEN         64
#define WHATSUP_OPTIONS_LEN       64
#define WHATSUP_LONG_OPTIONS_LEN  32
#define WHATSUP_MODULES_LEN       16
#define WHATSUP_MAXPATHLEN        256

/* 
 * struct whatsup_data
 * 
 * carries information for the entire program
 */
struct whatsup_data 
{
  char *hostname;
  int port;
  int updown_output;
  int list_output;
  int count_output;
  nodeupdown_t handle;
  hostlist_t cmdline_nodes;
#if !WITH_STATIC_MODULES
  lt_dlhandle module_handles[WHATSUP_MODULES_LEN];
#endif /* !WITH_STATIC_MODULES */
  struct whatsup_options_module_info *module_info[WHATSUP_MODULES_LEN];
  int module_loaded_count;
};

/*  
 * whatsup_options_modules
 *
 * list of options modules to search for
 */
#if WITH_STATIC_MODULES

#if WITH_GENDERSLLNL
extern struct whatsup_options_module_info gendersllnl_options_module_info;
#endif /* WITH_GENDERSLLNL */

static struct whatsup_options_module_info *whatsup_options_modules[] =
  {
#if WITH_GENDERSLLNL
    &gendersllnl_options_module_info,
#endif /* WITH_GENDERSLLNL */
    NULL
  };

#else /* !WITH_STATIC_MODULES */

static char *whatsup_options_modules[] = 
  {
    "whatsup_options_gendersllnl.la",
    NULL
  };
#endif /* !WITH_STATIC_MODULES */

/* 
 * _init_whatsup_data
 *
 * initialize whatsup_data struct with defaults
 */
static void
_init_whatsup_data(struct whatsup_data *w)
{
  assert(w);

  memset(w, '\0', sizeof(struct whatsup_data));

  w->hostname = NULL;
  w->port = 0;
  w->updown_output = WHATSUP_UP_AND_DOWN;
  w->list_output = WHATSUP_HOSTLIST;
  w->count_output = WHATSUP_FALSE;

  if (!(w->handle = nodeupdown_handle_create()))
    err_exit("_init_whatsup_data: nodeupdown_handle_create()");

  if (!(w->cmdline_nodes = hostlist_create(NULL)))
    err_exit("_init_whatsup_data: hostlist_create()");

  w->module_loaded_count = 0;
}

/* 
 * _cleanup_whatsup_data
 *
 * cleanup whatsup_data struct
 */
static void
_cleanup_whatsup_data(struct whatsup_data *w)
{
  assert(w);

  (void)nodeupdown_handle_destroy(w->handle);
  hostlist_destroy(w->cmdline_nodes);
}

/*  
 * _load_options_module
 *
 * load a whatsup options module
 */
static void
_load_options_module(struct whatsup_data *w, 
#if WITH_STATIC_MODULES
		     struct whatsup_options_module_info *module_info
#else /* !WITH_STATIC_MODULES */
		     char *module_path
#endif /* !WITH_STATIC_MODULES */
		     )
{
  int count = w->module_loaded_count;
  int i;

  assert(w);
#if WITH_STATIC_MODULES
  assert(module_info);
#else /* !WITH_STATIC_MODULES */
  assert(module_path);
#endif /* !WITH_STATIC_MODULES */

  if (count >= WHATSUP_MODULES_LEN)
    return;

#if WITH_STATIC_MODULES
  w->module_info[count] = module_info;
#else /* !WITH_STATIC_MODULES */
  if (!(w->module_handles[count] = lt_dlopen(module_path)))
    err_exit("_load_options_module: lt_dlopen: module_path=%s: %s",
	     module_path, lt_dlerror());

  if (!(w->module_info[count] = lt_dlsym(w->module_handles[count],
					 "options_module_info")))
    err_exit("_load_options_module: lt_dlsym: module_path=%s: %s",
	     module_path, lt_dlerror());
#endif /* !WITH_STATIC_MODULES */

  if (!w->module_info[count]->options_module_name
      || !w->module_info[count]->output_usage
      || !w->module_info[count]->register_option
      || !w->module_info[count]->add_long_option
      || !w->module_info[count]->check_option
      || !w->module_info[count]->convert_nodenames)
#if WITH_STATIC_MODULES
    err_exit("_load_options_module: invalid module options");
#else /* !WITH_STATIC_MODULES */
    err_exit("_load_options_module: module_path=%s: invalid module options",
	     module_path);
#endif /* !WITH_STATIC_MODULES */
  
  for (i = 0; i < count; i++)
    {
      if (!strcmp(w->module_info[i]->options_module_name,
		  w->module_info[count]->options_module_name))
	goto already_loaded;
    }

  if ((*w->module_info[i]->init)() < 0)
    err_exit("_load_options_module:: %s init error", 
             w->module_info[i]->options_module_name);

  w->module_loaded_count++;
  return;

 already_loaded:
#if !WITH_STATIC_MODULES
  lt_dlclose(w->module_handles[count]);
  w->module_handles[count] = NULL;
#endif /* !WITH_STATIC_MODULES */
  w->module_info[count] = NULL;
  return;
}

#if !WITH_STATIC_MODULES
/*  
 * _load_options_modules_in_dir
 *
 * Search for whatsup options modules in the specified directory.  If
 * one is found, load it.
 */
static void
_load_options_modules_in_dir(struct whatsup_data *w, char *search_dir)
{
  DIR *dir;
  int i = 0;

  assert(w);
  assert(search_dir);

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

              _load_options_module(w, filebuf);
            }
        }
      rewinddir(dir);
      i++;
    }
}
#endif /* !WITH_STATIC_MODULES */

/*  
 * _load_options_modules
 *
 * Load whatsup options modules
 */
static void
_load_options_modules(struct whatsup_data *w)
{
  assert(w);

#if WITH_STATIC_MODULES
  int i = 0;

  while (whatsup_options_modules[i])
    {
      _load_options_module(w, whatsup_options_modules[i]);
    }
#else /* !WITH_STATIC_MODULES */
  if (lt_dlinit() != 0)
    err_exit("_load_options_modules: lt_dlinit: %s", lt_dlerror());

  _load_options_modules_in_dir(w, WHATSUP_MODULE_BUILDDIR);
  _load_options_modules_in_dir(w, WHATSUP_MODULE_DIR);
  _load_options_modules_in_dir(w, ".");
#endif /* !WITH_STATIC_MODULES */
}

/*  
 * _unload_options_modules
 *
 * Unload whatsup options modules
 */
static void
_unload_options_modules(struct whatsup_data *w)
{
  int i;

  assert(w);

  for (i = 0; i < w->module_loaded_count; i++)
    {
      (*w->module_info[i]->cleanup)();
#if !WITH_STATIC_MODULES
      lt_dlclose(w->module_handles[i]);
#endif /* !WITH_STATIC_MODULES */
      i++;
    }
#if !WITH_STATIC_MODULES
  lt_dlexit();
#endif /* !WITH_STATIC_MODULES */
}

/* 
 * _usage
 *
 * output usage and exit
 */
static void 
_usage(struct whatsup_data *w) 
{
  int i;

  assert(w);

  fprintf(stderr,
	  "Usage: whatsup [OPTIONS]... [NODES]...\n"
	  "  -h         --help              Print help and exit\n"
	  "  -V         --version           Print version and exit\n"
	  "  -o STRING  --hostname=STRING   Server hostname\n"
	  "  -p INT     --port=INT          Server port\n"
	  "  -b         --updown            Output up and down nodes\n"
	  "  -u         --up                Output only up nodes\n"
	  "  -d         --down              Output only down nodes\n"
	  "  -t         --count             Output only node counts\n"
	  "  -q         --hostlist          Output in hostlist format\n"
	  "  -c         --comma             Output in comma separated list\n"
	  "  -n         --newline           Output in newline separated list\n"
	  "  -s         --space             Output in space separated list\n");

  for (i = 0; i < w->module_loaded_count; i++)
    {
      if ((*w->module_info[i]->output_usage)() < 0)
        err_exit("_usage: %s output_usage error",
                 w->module_info[i]->options_module_name);
    }

  fprintf(stderr, "\n");
  exit(1);
}

/*  
 * _version
 *
 * output version and exit
 */
static void 
_version(void) 
{
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
  exit(1);
}

/* 
 * _push_nodes_on_hostlist
 *
 * push nodes onto a hostlist structure
 */
static void
_push_nodes_on_hostlist(struct whatsup_data *w, char *nodes) 
{
  assert(w);
  assert(nodes);

  /* Error if nodes aren't short hostnames */
  if (strchr(nodes, '.') != NULL)
    err_exit("nodes must be listed in short hostname format");
        
  if (!hostlist_push(w->cmdline_nodes, nodes))
    err_exit("nodes improperly formatted");
}

/* 
 * _read_nodes_from_stdin
 *
 * read nodes off of standard input rather than the command line
 */
static void 
_read_nodes_from_stdin(struct whatsup_data *w) 
{
  int n;
  char buf[WHATSUP_BUFFERLEN];
  
  assert(w);

  if ((n = fd_read_n(STDIN_FILENO, buf, WHATSUP_BUFFERLEN)) < 0)
    err_exit("error reading from standard input: %s", strerror(errno));
  
  if (n == WHATSUP_BUFFERLEN)
    err_exit("overflow in standard input buffer"); 

  if (n > 0) 
    {
      char *ptr = strtok(buf, " \t\n\0"); 
      while (ptr != NULL) 
        {
          _push_nodes_on_hostlist(w, ptr);
          ptr = strtok(NULL, " \t\n\0");
        }
    }
}

/* 
 * _cmdline_parse
 *
 * parse all cmdline input
 */
static void
_cmdline_parse(struct whatsup_data *w, int argc, char **argv) 
{
  int i, c, index, used_option;
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
      {"hostlist",  0, NULL, 'q'},
      {"comma",     0, NULL, 'c'},
      {"newline",   0, NULL, 'n'},
      {"space",     0, NULL, 's'},
      {0, 0, 0, 0},
  };
  int long_options_count = 12;
#endif /* HAVE_GETOPT_LONG */

  assert(w);
  assert(argv);

  memset(options, '\0', WHATSUP_OPTIONS_LEN+1);
  strncpy(options, "hVo:p:budtqcns", WHATSUP_OPTIONS_LEN);

  /* 
   * Load additional option arguments
   */
  for (i = 0; i < w->module_loaded_count; i++)
    {
      char *module_options;
      char *c;

      if (!(module_options = (*w->module_info[i]->options_string)()))
	err_exit("_cmdline_parse: %s options_string failure",
		 w->module_info[i]->options_module_name);
      
      c = module_options;
      while (*c != '\0')
	{
          /* Don't install this option if it already exists */
	  if (strchr(options, *c))
	    continue;

	  if ((*w->module_info[i]->register_option)(*c, options) < 0)
	    err_exit("_cmdline_parse: %s register_option failure",
		     w->module_info[i]->options_module_name);
	  
	  if ((*w->module_info[i]->add_long_option)(*c, 
                                                    &long_options[long_options_count]) < 0)
	    err_exit("_cmdline_parse: %s add_long_options failure",
		     w->module_info[i]->options_module_name);

	  long_options_count++;
	  long_options[long_options_count].name = NULL;
	  long_options[long_options_count].has_arg = 0;
	  long_options[long_options_count].flag = NULL;
	  long_options[long_options_count].val = 0;

          /* If we're out of space for more options, this is
           * all the user gets.
           */

          /* We take into account the -1 b/c the module may want the
           * option to take an argument.  In other words, the next
           * register_option call may copy in two chars rather than
           * one.
           */
	  if (strlen(options) >= (WHATSUP_OPTIONS_LEN - 1))
	    break;

	  if (long_options_count >= WHATSUP_LONG_OPTIONS_LEN)
	    break;
          
          c++;
	}
    }
  
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
        _usage(w);
      case 'V':
        _version();
      case 'o':
        w->hostname = optarg;
        break;
      case 'p':
        w->port = atoi(optarg);
        break;
      case 'b':
        w->updown_output = WHATSUP_UP_AND_DOWN;
        break;
      case 'u':
        w->updown_output = WHATSUP_UP_NODES;
        break;
      case 'd':
        w->updown_output = WHATSUP_DOWN_NODES;
        break;
      case 't':
        w->count_output = WHATSUP_TRUE;
        break;
      case 'q':
        w->list_output = WHATSUP_HOSTLIST;
        break;
      case 'c':
        w->list_output = WHATSUP_COMMA;
        break;
      case 'n':
        w->list_output = WHATSUP_NEWLINE;
        break;
      case 's':
        w->list_output = WHATSUP_SPACE;
        break;
      default:
	used_option = 0;

	for (i = 0; i < w->module_loaded_count; i++)
	  {
	    int rv;
	    
	    if ((rv = (*w->module_info[i]->check_option)(c, optarg)) < 0)
	      continue;
	    
	    if (rv)
	      {
		used_option = 1;
		break;
	      }
	  }

	if (used_option)
	  break;
	/* else fall through */
      case '?':
        fprintf(stderr, "command line option error\n");
        _usage(w);
      }
    }

  index = optind;
  
  /* Read nodes in from the command line or standard input */
  if (index < argc) 
    {
      if (!strcmp(argv[index], "-"))
        _read_nodes_from_stdin(w);
      else 
        {
          while (index < argc) {
            _push_nodes_on_hostlist(w, argv[index]);
            index++;
          }
        } 
      
      /* remove any duplicate nodes listed */
      hostlist_uniq(w->cmdline_nodes);
    }
}

/* 
 * _get_cmdline_nodes
 *
 * Get the up or down status of nodes specified on the cmdline or
 * standard input.
 */
static void 
_get_cmdline_nodes(struct whatsup_data *w, 
		   int up_or_down, 
		   char *buf, 
		   int buflen) 
{
  hostlist_t hl = NULL;
  hostlist_iterator_t iter = NULL;
  char *node = NULL;
  int ret;

  assert(w);
  assert(up_or_down == WHATSUP_UP_NODES || up_or_down == WHATSUP_DOWN_NODES);
  assert(buf);
  assert(buflen > 0);

  if (!(hl = hostlist_create(NULL)))
    err_exit("_get_cmdline_nodes: hostlist_create()");

  if (!(iter = hostlist_iterator_create(w->cmdline_nodes)))
    err_exit("_get_cmdline_nodes: hostlist_iterator_create()");

  while ((node = hostlist_next(iter)) != NULL) 
    {
      if (up_or_down == WHATSUP_UP_NODES) 
        {
          if ((ret = nodeupdown_is_node_up(w->handle, node)) < 0) 
            {
              if (nodeupdown_errnum(w->handle) == NODEUPDOWN_ERR_NOTFOUND)
                err_exit("Unknown node \"%s\"", node);
              else
                err_exit("_get_cmdline_nodes: nodeupdown_is_node_up(): %s",
			 nodeupdown_errormsg(w->handle));
            }
        }
      else 
        {
          if ((ret = nodeupdown_is_node_down(w->handle, node)) < 0) 
            {
              if (nodeupdown_errnum(w->handle) == NODEUPDOWN_ERR_NOTFOUND)
                err_exit("Unknown node \"%s\"", node);
              else
                err_exit("_get_cmdline_nodes: nodeupdown_is_node_down(): %s",
			 nodeupdown_errormsg(w->handle));
            }
        }
      
      if (ret) 
        {
          if (!hostlist_push_host(hl, node))
            err_exit("_get_cmdline_nodes: hostlist_push_host()");
        }

      free(node);
    }
  
  hostlist_sort(hl);
  
  if (hostlist_ranged_string(hl, buflen, buf) < 0)
    err_exit("_get_cmdline_nodes: hostlist_ranged_string()");
  
  hostlist_iterator_destroy(iter);
  hostlist_destroy(hl);
}
 
/* 
 * _get_all_nodes
 *
 * Get the up or down status of all nodes.
 */
static void
_get_all_nodes(struct whatsup_data *w, 
               int up_or_down, 
               char *buf, 
               int buflen) 
{
  assert(w);
  assert(up_or_down == WHATSUP_UP_NODES || up_or_down == WHATSUP_DOWN_NODES);
  assert(buf);
  assert(buflen > 0);

  if (up_or_down == WHATSUP_UP_NODES) 
    {
      if (nodeupdown_get_up_nodes_string(w->handle, buf, buflen) < 0)
        err_exit("_get_all_nodes: nodeupdown_get_up_nodes_string(): %s", 
                  nodeupdown_errormsg(w->handle));
    }
  else 
    {
      if (nodeupdown_get_down_nodes_string(w->handle, buf, buflen) < 0)
        err_exit("_get_all_nodes: nodeupdown_get_down_nodes_string(): %s",
                  nodeupdown_errormsg(w->handle));
    }
}

/*  
 * _get_nodes
 *
 * get the up or down status of nodes.  The appropriate function call
 * to _get_cmdline_nodes or _get_all_nodes will be done.
 */
static void 
_get_nodes(struct whatsup_data *w, 
           int up_or_down, 
           char *buf, 
           int buflen, 
           int *count) 
{
  hostlist_t hl = NULL;
  char tempbuf[WHATSUP_BUFFERLEN];
  int i;
  
  assert(w);
  assert(up_or_down == WHATSUP_UP_NODES || up_or_down == WHATSUP_DOWN_NODES);
  assert(buf);
  assert(buflen > 0);
  assert(count);

  if (hostlist_count(w->cmdline_nodes) > 0)
    _get_cmdline_nodes(w, up_or_down, buf, buflen);
  else
    _get_all_nodes(w, up_or_down, buf, buflen);

  /* convert the nodenames if desired */
  for (i = 0; i < w->module_loaded_count; i++)
    {
      int rv;

      if ((rv = (*w->module_info[i]->convert_nodenames)(buf, 
                                                        tempbuf, 
                                                        WHATSUP_BUFFERLEN)) < 0)
	continue;
      
      if (rv)
	{
	  if (strlen(tempbuf) < buflen)
	    strcpy(buf, tempbuf);
	  else
	    err_exit("_get_nodes: overflow buffer");
	  break;
	}
    }

  /* can't use nodeupdown_up/down_count, b/c we may be counting the
   * nodes specified by the user on the cmdline
   */
  if (!(hl = hostlist_create(buf)))
    err_exit("_get_nodes: hostlist_create()");
  
  *count = hostlist_count(hl);

  hostlist_destroy(hl);
}

/* 
 * _output_nodes
 * 
 * output the nodes specified in the nodebuf to stdout
 */
static void
_output_nodes(struct whatsup_data *w, char *nodebuf) 
{
  assert(w);
  assert(nodebuf);

  if (w->list_output == WHATSUP_HOSTLIST)
    fprintf(stdout, "%s\n", nodebuf);
  else 
    {
      char tempbuf[WHATSUP_BUFFERLEN];
      hostlist_t hl = NULL;
      char *ptr;

      /* output nodes separated by some break type */
      memset(tempbuf, '\0', WHATSUP_BUFFERLEN);
    
      if (!(hl = hostlist_create(nodebuf)))
        err_exit("_output_nodes: hostlist_create() error");
      
      if (hostlist_deranged_string(hl, WHATSUP_BUFFERLEN, tempbuf) < 0)
        err_exit("_output_nodes: hostlist_deranged_string() error");
      
      /* convert commas to appropriate break types */
      if (w->list_output != WHATSUP_COMMA) 
        {
          while ((ptr = strchr(tempbuf, ',')) != NULL)
            *ptr = (char)w->list_output;
        }

      /* start on the next line if its a newline separator */
      if (w->updown_output == WHATSUP_UP_AND_DOWN 
          && w->list_output == WHATSUP_NEWLINE)
        fprintf(stdout, "\n");

      fprintf(stdout,"%s\n", tempbuf);
      hostlist_destroy(hl);
    }
}

/* 
 * _log10
 *
 * portable log10() function that also allows us to avoid linking
 * against the math library.
 */
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

/* 
 * _create_formats
 *
 * create the output formats based on the up and down count.
 */
static void
_create_formats(char *upfmt, int upfmt_len, int up_count,
                char *downfmt, int downfmt_len, int down_count,
                char *endstr)
{
  int max;

  assert(upfmt);
  assert(upfmt_len > 0);
  assert(downfmt);
  assert(downfmt_len > 0);
  assert(endstr);

  if (up_count > down_count)
    max = _log10(up_count);
  else
    max = _log10(down_count);

  max++;

  snprintf(upfmt,   upfmt_len,   "up:   %%%dd%s", max, endstr);
  snprintf(downfmt, downfmt_len, "down: %%%dd%s", max, endstr);
}

int 
main(int argc, char *argv[]) 
{
  struct whatsup_data w;
  char up_nodes[WHATSUP_BUFFERLEN];
  char down_nodes[WHATSUP_BUFFERLEN];
  char upfmt[WHATSUP_FORMATLEN];
  char downfmt[WHATSUP_FORMATLEN];
  int up_count, down_count, exit_val;

  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);

  _init_whatsup_data(&w);

  /* 
   * Load options modules before calling cmdline_parse, b/c modules
   * may need additional command line options parsed.
   */
  _load_options_modules(&w);

  _cmdline_parse(&w, argc, argv);
  
  if (nodeupdown_load_data(w.handle, 
                           w.hostname, 
                           w.port, 
                           0,
                           NULL) < 0) 
    {
      int errnum = nodeupdown_errnum(w.handle);
      
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
		 nodeupdown_errormsg(w.handle));
    }
  
  _get_nodes(&w, 
             WHATSUP_UP_NODES, 
             up_nodes, 
             WHATSUP_BUFFERLEN, 
             &up_count);
  _get_nodes(&w, 
             WHATSUP_DOWN_NODES, 
             down_nodes, 
             WHATSUP_BUFFERLEN, 
             &down_count);
  
  if (w.count_output == WHATSUP_TRUE) 
    {
      /* only output count */
      if (w.updown_output == WHATSUP_UP_AND_DOWN) 
        {
          _create_formats(upfmt, 
                          WHATSUP_FORMATLEN, 
                          up_count,
                          downfmt, 
                          WHATSUP_FORMATLEN, 
                          down_count, 
                          "\n");
          fprintf(stdout, upfmt, up_count);
          fprintf(stdout, downfmt, down_count);
        }
      else if (w.updown_output == WHATSUP_UP_NODES)
        fprintf(stdout, "%d\n", up_count);
      else
        fprintf(stdout, "%d\n", down_count);
    }
  else 
    {    
      /* output up, down, or both up and down nodes */
      if (w.updown_output == WHATSUP_UP_AND_DOWN) 
        {
          if (w.list_output == WHATSUP_NEWLINE)
            {
              /* newline output is funny, thus special */
              snprintf(upfmt,   WHATSUP_FORMATLEN, "up %d:", up_count);
              snprintf(downfmt, WHATSUP_FORMATLEN, "down %d:", down_count);
            }
          else
            _create_formats(upfmt, 
                            WHATSUP_FORMATLEN, 
                            up_count,
                            downfmt, 
                            WHATSUP_FORMATLEN, 
                            down_count, 
                            ": ");
          
          fprintf(stdout, upfmt, up_count);
          
          _output_nodes(&w, up_nodes);
          
          /* handle odd situation with newline output list */
          if (w.list_output == WHATSUP_NEWLINE)
            fprintf(stdout, "\n");
          
          fprintf(stdout, downfmt, down_count);
          
          _output_nodes(&w, down_nodes);
        }
      else if (w.updown_output == WHATSUP_UP_NODES)
        _output_nodes(&w, up_nodes);
      else
        _output_nodes(&w, down_nodes);
    }
  
  if (w.updown_output == WHATSUP_UP_AND_DOWN)
    exit_val = 0;
  else if (w.updown_output == WHATSUP_UP_NODES)
    exit_val = (!down_count) ? 0 : 1;
  else
    exit_val = (!up_count) ? 0 : 1;
  
  _unload_options_modules(&w);
  _cleanup_whatsup_data(&w);
  exit(exit_val);
}
