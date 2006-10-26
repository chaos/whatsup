/*****************************************************************************\
 *  $Id: whatsup.c,v 1.122 2006-10-26 22:06:20 chu11 Exp $
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
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <assert.h>
#include <errno.h>

#include "whatsup_options.h"
#include "nodeupdown.h"
#include "hostlist.h"
#include "error.h"
#include "list.h"
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
#define WHATSUP_UP_NODES          0 
#define WHATSUP_DOWN_NODES        1
#define WHATSUP_UP_AND_DOWN       2

#define WHATSUP_BUFFERLEN         65536
#define WHATSUP_FORMATLEN         64
#define WHATSUP_OPTIONS_LEN       64
#define WHATSUP_LONG_OPTIONS_LEN  32
#define WHATSUP_MAXPATHLEN        256

#define WHATSUP_MODULE_SIGNATURE  "whatsup_options_"
#define WHATSUP_MODULE_INFO_SYM   "whatsup_module_info"
#define WHATSUP_MODULE_DEVEL_DIR  WHATSUP_MODULE_BUILDDIR "/.libs"

#define WHATSUP_LOG_POLL          30

/* 
 * Whatsup Data
 *
 * hostname - hostname to connect to
 * port - port to connect o
 * updown_output - up, down, or both up and down output
 * output_type - hostlist, comma, newline, or space output
 * count_only_output - count output only or not
 * handle - nodeupdown handle
 * inputted_nodes - nodes input via the cmdline or stdin
 * module - a specific backend module to use
 * log_mode - flag to indicate log mode
 * log_file - file to log to, NULL for stdout
 * log_poll - logging polling interval in seconds
 */
static char *hostname = NULL;
static int port = 0;
static int updown_output = WHATSUP_UP_AND_DOWN;
static char output_type = 0;
static int count_only_output = 0;
static nodeupdown_t handle;
static hostlist_t inputted_nodes = NULL;
static char *module = NULL;
static int log_mode = 0;
static char *log_file = NULL;
static int log_poll = WHATSUP_LOG_POLL;

struct whatsup_module_loadinfo 
{
  lt_dlhandle handle;
  struct whatsup_options_module_info *module_info;
  char options_towatch[WHATSUP_OPTIONS_LEN];
  char options_processed[WHATSUP_OPTIONS_LEN];
};

static List modules_list = NULL;
static ListIterator modules_list_itr = NULL;

/* 
 * Whatsup output data
 */
static char up_nodes[WHATSUP_BUFFERLEN];
static char down_nodes[WHATSUP_BUFFERLEN];
static char upfmt[WHATSUP_FORMATLEN];
static char downfmt[WHATSUP_FORMATLEN];
static int up_count = -1;
static int down_count = -1;

/* 
 * _init_whatsup
 *
 * initialize globals
 */
static void
_init_whatsup(void)
{
  if (!(inputted_nodes = hostlist_create(NULL)))
    err_exit("%s: hostlist_create", __FUNCTION__);
}

/* 
 * _cleanup_whatsup
 *
 * cleanup globals
 */
static void
_cleanup_whatsup(void)
{
  (void)nodeupdown_handle_destroy(handle);
  hostlist_destroy(inputted_nodes);
}

/*  
 * _load_options_module
 *
 * load a whatsup options module
 */
static void
_load_options_module(char *module_path)
{
  struct whatsup_module_loadinfo *loadinfo;
  struct whatsup_module_loadinfo *loadinfoPtr;
  struct whatsup_option *optionsPtr;
  ListIterator itr = NULL;

  assert(module_path && modules_list);

  if (!(loadinfo = malloc(sizeof(struct whatsup_module_loadinfo))))
    err_exit("%s: malloc: %s", __FUNCTION__, strerror(errno));
  memset(loadinfo, '\0', sizeof(struct whatsup_module_loadinfo));

  if (!(loadinfo->handle = lt_dlopen(module_path)))
    goto cleanup;

  if (!(loadinfo->module_info = lt_dlsym(loadinfo->handle, WHATSUP_MODULE_INFO_SYM)))
    goto cleanup;

  if (!loadinfo->module_info->module_name
      || !loadinfo->module_info->options
      || !loadinfo->module_info->setup
      || !loadinfo->module_info->cleanup
      || !loadinfo->module_info->process_option
      || !loadinfo->module_info->convert_nodenames
      || !loadinfo->module_info->get_nodenames)
    goto cleanup;
  
  optionsPtr = loadinfo->module_info->options;
  while (optionsPtr->option) 
    {
      if (optionsPtr->option_type != WHATSUP_OPTION_TYPE_CONVERT_NODENAMES
          && optionsPtr->option_type != WHATSUP_OPTION_TYPE_GET_NODENAMES)
        goto cleanup;
      optionsPtr++;
    }

  if (list_count(modules_list) > 0) 
    {
      char *module_name;
      
      if (!(itr = list_iterator_create(modules_list)))
        err_exit("%s: list_iterator_create: %s", __FUNCTION__, strerror(errno));
      
      module_name = loadinfo->module_info->module_name;
      while ((loadinfoPtr = list_next(itr))) 
        {
          if (!strcmp(loadinfoPtr->module_info->module_name, module_name))
            goto cleanup;           /* module already loaded */
        }
    }

  if ((*loadinfo->module_info->setup)() < 0)
    goto cleanup;

  if (!list_append(modules_list, loadinfo))
    err_exit("%s: list_append: %s", __FUNCTION__, strerror(errno));

  if (itr)
    list_iterator_destroy(itr);
  return;

 cleanup:
  if (loadinfo) 
    {
      if (loadinfo->handle)
        lt_dlclose(loadinfo->handle);
    }
  if (itr)
    list_iterator_destroy(itr);
  return;
}

/*  
 * _load_options_modules_in_dir
 *
 * Search for whatsup options modules in the specified directory.  If
 * one is found, load it.
 */
static void
_load_options_modules_in_dir(char *search_dir)
{
  DIR *dir;
  struct dirent *dirent;

  assert(search_dir);

  /* Can't open the directory? we assume it doesn't exit, so its not
   * an error.
   */
  if (!(dir = opendir(search_dir)))
    return;

  while ((dirent = readdir(dir)))
    {
      char *ptr = strstr(dirent->d_name, WHATSUP_MODULE_SIGNATURE);
 
      if (ptr && ptr == &dirent->d_name[0])
        {
          char filebuf[WHATSUP_MAXPATHLEN+1];
          char *filename = dirent->d_name;

          /* Don't bother unless its a shared object file. */
          ptr = strchr(filename, '.');
          if (!ptr || strcmp(ptr, ".so"))
            continue;
  
          memset(filebuf, '\0', WHATSUP_MAXPATHLEN+1);
          snprintf(filebuf, WHATSUP_MAXPATHLEN, "%s/%s", search_dir, filename);
            
	  _load_options_module(filebuf);
        }
    }
}

/*  
 * _delete_whatsup_module_loadinfo
 *
 * Destroy loadinfo data
 */
static void
_delete_whatsup_module_loadinfo(void *x)
{
  struct whatsup_module_loadinfo *info = (struct whatsup_module_loadinfo *)x;

  (*info->module_info->cleanup)();
  lt_dlclose(info->handle);
}


/*  
 * _load_options_modules
 *
 * Load whatsup options modules
 */
static void
_load_options_modules(void)
{
  if (lt_dlinit() != 0)
    err_exit("%s: lt_dlinit: %s", __FUNCTION__, lt_dlerror());
  
  if (!(modules_list = list_create((ListDelF)_delete_whatsup_module_loadinfo)))
    err_exit("%s: list_create: %s", __FUNCTION__, strerror(errno));
  
#ifndef NDEBUG
  _load_options_modules_in_dir(WHATSUP_MODULE_DEVEL_DIR);
#endif /* !NDEBUG */
  _load_options_modules_in_dir(WHATSUP_MODULE_DIR);
  
  if (list_count(modules_list) > 0) 
    {
      if (!(modules_list_itr = list_iterator_create(modules_list)))
        err_exit("%s: list_iterator_create: %s", __FUNCTION__, strerror(errno));
    }
}

/*  
 * _unload_options_modules
 *
 * Unload whatsup options modules
 */
static void
_unload_options_modules(void)
{
  list_destroy(modules_list);
  modules_list = NULL;
  lt_dlexit();
}

/* 
 * _usage
 *
 * output usage and exit
 */
static void 
_usage(void) 
{
  fprintf(stderr,
	  "Usage: whatsup [OPTIONS]... [NODES]...\n"
	  "  -h         --help              Print help and exit\n"
	  "  -v         --version           Print version and exit\n"
	  "  -o STRING  --hostname=STRING   Server hostname\n"
	  "  -p INT     --port=INT          Server port\n"
	  "  -b         --updown            Output up and down nodes\n"
	  "  -u         --up                Output only up nodes\n"
	  "  -d         --down              Output only down nodes\n"
	  "  -t         --count             Output only node counts\n"
	  "  -q         --hostrange         Output in hostrange format\n"
	  "  -c         --comma             Output in comma separated list\n"
	  "  -n         --newline           Output in newline separated list\n"
	  "  -s         --space             Output in space separated list\n"
          "  -m         --module            Specify backend module\n"
          "  -l         --log               Output up/dog state log\n"
          "  -f         --log-file          Specify log file\n"
          "  -e         --log-poll          Specify log polling internval\n");

  if (modules_list_itr) 
    {
      struct whatsup_module_loadinfo *loadinfoPtr;
      
      list_iterator_reset(modules_list_itr);
      while ((loadinfoPtr = list_next(modules_list_itr))) 
        {
          struct whatsup_option *optionsPtr = loadinfoPtr->module_info->options;
          while (optionsPtr->option) 
            {
              if (optionsPtr->option_arg)
                {
                  if (optionsPtr->option_long)
                    fprintf(stderr, 
                            "  -%c %-7.7s --%-17.17s %s\n",
                            optionsPtr->option,
                            optionsPtr->option_arg,
                            optionsPtr->option_long,
                            optionsPtr->description);
                  else
                    fprintf(stderr, 
                            "  -%c %-27.27s %s\n",
                            optionsPtr->option,
                            optionsPtr->option_arg,
                            optionsPtr->description);
                }
              else 
                {
                  if (optionsPtr->option_long)
                    fprintf(stderr, 
                            "  -%c         --%-17.17s %s\n",
                            optionsPtr->option,
                            optionsPtr->option_long,
                            optionsPtr->description);
                  else
                    fprintf(stderr, 
                            "  -%c                             %s\n",
                            optionsPtr->option,
                            optionsPtr->description);
                }
              optionsPtr++;
            }
        }
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
 * _push_inputted_nodes
 *
 * push nodes onto a hostlist structure
 */
static void
_push_inputted_nodes(char *nodes) 
{
  assert(nodes);

  /* Error if nodes aren't short hostnames */
  if (strchr(nodes, '.'))
    err_exit("nodes must be listed in short hostname format");
        
  if (!hostlist_push(inputted_nodes, nodes))
    err_exit("nodes improperly formatted");
}

/* 
 * _read_nodes_from_stdin
 *
 * read nodes off of standard input rather than the command line
 */
static void 
_read_nodes_from_stdin(void) 
{
  char buf[WHATSUP_BUFFERLEN];
  int n;
  
  if ((n = fd_read_n(STDIN_FILENO, buf, WHATSUP_BUFFERLEN)) < 0)
    err_exit("error reading from standard input: %s", strerror(errno));
  
  if (n == WHATSUP_BUFFERLEN)
    err_exit("overflow in standard input buffer"); 

  if (n > 0) 
    {
      char *ptr = strtok(buf, " \t\n\0"); 
      while (ptr) 
        {
          _push_inputted_nodes(ptr);
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
_cmdline_parse(int argc, char **argv) 
{
  int c, index, used_option;
  char soptions[WHATSUP_OPTIONS_LEN+1];
  char *ptr;

#if HAVE_GETOPT_LONG
  struct option loptions[WHATSUP_LONG_OPTIONS_LEN+1] = 
    {
      {"help",         0, NULL, 'h'},
      {"version",      0, NULL, 'v'},
      {"hostname",     1, NULL, 'o'},
      {"port",         1, NULL, 'p'},
      {"updown",       0, NULL, 'b'},
      {"up",           0, NULL, 'u'},
      {"down",         0, NULL, 'd'},
      {"count",        0, NULL, 't'},
      {"hostrange",    0, NULL, 'q'},
      {"comma",        0, NULL, 'c'},
      {"newline",      0, NULL, 'n'},
      {"space",        0, NULL, 's'},
      {"module",       1, NULL, 'm'},
      {"log",          0, NULL, 'l'},
      {"log-file",     1, NULL, 'f'}, 
      {"log-poll",     1, NULL, 'e'},
      {0, 0, 0, 0},
    };
  int loptions_len = 16;
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  /* aegijkrwxyz */
  memset(soptions, '\0', WHATSUP_OPTIONS_LEN+1);
  strncpy(soptions, "hvo:p:budtqcnsm:lf:e:", WHATSUP_OPTIONS_LEN);

  /* 
   * Load additional option arguments
   */
  if (modules_list_itr) 
    {
      struct whatsup_module_loadinfo *loadinfoPtr;
      
      list_iterator_reset(modules_list_itr);
      while ((loadinfoPtr = list_next(modules_list_itr))) 
        {
          struct whatsup_option *optionsPtr = loadinfoPtr->module_info->options;
          
          while (optionsPtr->option) 
            {
              char opt = optionsPtr->option;
              
              /* run out of space?  Then that's all the user gets 
               * -1 to account for possible ':'
               */
              if (strlen(loadinfoPtr->options_towatch) >= (WHATSUP_OPTIONS_LEN - 1))
                break;
              
              if (!strchr(soptions, optionsPtr->option)) 
                {
                  /* run out of space? Then that's all the user gets */
                  if (strlen(soptions) >= WHATSUP_OPTIONS_LEN)
                    break;
                  
#if HAVE_GETOPT_LONG
                  if (loptions_len >= WHATSUP_LONG_OPTIONS_LEN)
                    break;
#endif /* HAVE_GETOPT_LONG */

                  strncat(loadinfoPtr->options_towatch, &opt, 1);
                  strncat(soptions, &opt, 1);

                  if (optionsPtr->option_arg) 
                    {
                      opt = ':';
                      strncat(soptions, &opt, 1);
                    }
                  
#if HAVE_GETOPT_LONG
                  if (optionsPtr->option_long)
                    {
                      loptions[loptions_len].name = optionsPtr->option_long;
                      loptions[loptions_len].has_arg = 1;
                      loptions[loptions_len].flag = NULL;
                      loptions[loptions_len].val = optionsPtr->option;

                      loptions_len++;
                      loptions[loptions_len].name = NULL;
                      loptions[loptions_len].has_arg = 0;
                      loptions[loptions_len].flag = NULL;
                      loptions[loptions_len].val = 0;
                    }
#endif /* HAVE_GETOPT_LONG */
                }
              
              optionsPtr++;
            }
        }
    }
  
  /* turn off output messages printed by getopt_long */
  opterr = 0;

#if HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, soptions, loptions, NULL)) != -1)
#else
  while ((c = getopt(argc, argv, options)) != -1)
#endif
      {
        switch(c) 
          {
          case 'h':
            _usage();
          case 'v':
            _version();
          case 'o':
            hostname = optarg;
            break;
          case 'p':
            port = strtol(optarg, &ptr, 10);
            if (ptr != (optarg + strlen(optarg)))
              err_exit("invalid port specified");
            break;
          case 'b':
            updown_output = WHATSUP_UP_AND_DOWN;
            break;
          case 'u':
            updown_output = WHATSUP_UP_NODES;
            break;
          case 'd':
            updown_output = WHATSUP_DOWN_NODES;
            break;
          case 't':
            count_only_output++;
            break;
          case 'q':
            output_type = 0;
            break;
          case 'c':
            output_type = ',';
            break;
          case 'n':
            output_type = '\n';
            break;
          case 's':
            output_type = ' ';
            break;
          case 'm':
            module = optarg;
            break;
          case 'l':
            log_mode++;
            break;
          case 'f':
            log_file = optarg;
            break;
          case 'e':
            log_poll = strtol(optarg, &ptr, 10);
            if (ptr != (optarg + strlen(optarg))
                || log_poll <= 0)
              err_exit("invalid log_poll specified");
            break;
          default:
            used_option = 0;

            if (modules_list_itr) 
              {
                struct whatsup_module_loadinfo *loadinfoPtr;
                
                list_iterator_reset(modules_list_itr);
                while ((loadinfoPtr = list_next(modules_list_itr))) {
                  char temp;

                  if (strchr(loadinfoPtr->options_towatch, c)) 
                    {
                      if ((*loadinfoPtr->module_info->process_option)(c, optarg) < 0)
                        err_exit("%s: process_option failure", __FUNCTION__);
                    }

                  temp = c;
                  strncat(loadinfoPtr->options_processed, &temp, 1);
                  used_option++;
                  break;
                }
              }
            
            if (used_option)
              break;
            /* else fall through */
          case '?':
            fprintf(stderr, "command line option error\n");
            _usage();
          }
      }

  index = optind;
  
  /* Read nodes in from the command line or standard input */
  if (index < argc) 
    {
      if (!strcmp(argv[index], "-"))
        _read_nodes_from_stdin();
      else 
        {
          while (index < argc) {
            _push_inputted_nodes(argv[index]);
            index++;
          }
        } 
      
      /* remove any duplicate nodes listed */
      hostlist_uniq(inputted_nodes);
    }
  else
    {
      /* get nodenames if desired */
      if (modules_list_itr) 
        {
          struct whatsup_module_loadinfo *loadinfoPtr;
          int break_flag = 0;

          list_iterator_reset(modules_list_itr);

          while ((loadinfoPtr = list_next(modules_list_itr)) && !break_flag) 
            {
              struct whatsup_option *optionsPtr = loadinfoPtr->module_info->options;
              while (optionsPtr->option) 
                {
                  if (optionsPtr->option_type == WHATSUP_OPTION_TYPE_GET_NODENAMES
                      && strchr(loadinfoPtr->options_processed, optionsPtr->option)) 
                    {
                      char buf[WHATSUP_BUFFERLEN];
                      
                      if (!(*loadinfoPtr->module_info->get_nodenames)(buf, WHATSUP_BUFFERLEN)) 
                        {
                          if (!hostlist_push(inputted_nodes, buf))
                            err_exit("%s: hostlist_push", __FUNCTION__);
                          
                          break_flag++;
                          break;
                        }
                    }
                  
                  optionsPtr++;
                }
            }
        }
    }
}

/* 
 * _get_input_nodes
 *
 * Get the up or down status of nodes specified on the cmdline or
 * standard input.
 */
static void 
_get_input_nodes(char *buf, int buflen, int up_or_down)
{
  hostlist_t hl = NULL;
  hostlist_iterator_t iter = NULL;
  char *node = NULL;

  assert(buf && buflen);
  assert(up_or_down == WHATSUP_UP_NODES || up_or_down == WHATSUP_DOWN_NODES);

  if (!(hl = hostlist_create(NULL)))
    err_exit("%s: hostlist_create()", __FUNCTION__);

  if (!(iter = hostlist_iterator_create(inputted_nodes)))
    err_exit("%s: hostlist_iterator_create()", __FUNCTION__);

  while ((node = hostlist_next(iter))) 
    {
      int rv;

      if (up_or_down == WHATSUP_UP_NODES) 
        rv = nodeupdown_is_node_up(handle, node);
      else
        rv = nodeupdown_is_node_down(handle, node);

      if (rv < 0)
        {
          if (nodeupdown_errnum(handle) == NODEUPDOWN_ERR_NOTFOUND)
            err_exit("Unknown node \"%s\"", node);
          else 
            {
              char *msg = nodeupdown_errormsg(handle);
              if (up_or_down == WHATSUP_UP_NODES)
                err_exit("%s: nodeupdown_is_node_up(): %s", __FUNCTION__, msg);
              else
                err_exit("%s: nodeupdown_is_node_down(): %s", __FUNCTION__, msg);
            }
        }

      if (rv) 
        {
          if (!hostlist_push_host(hl, node))
            err_exit("%s: hostlist_push_host()", __FUNCTION__);
        }

      free(node);
    }
  
  hostlist_sort(hl);
  
  if (hostlist_ranged_string(hl, buflen, buf) < 0)
    err_exit("%s: hostlist_ranged_string()", __FUNCTION__);
  
  hostlist_iterator_destroy(iter);
  hostlist_destroy(hl);
}
 
/* 
 * _get_all_nodes
 *
 * Get the up or down status of all nodes.
 */
static void
_get_all_nodes(char *buf, int buflen, int up_or_down)
{
  int rv;

  assert(buf && buflen > 0);
  assert(up_or_down == WHATSUP_UP_NODES || up_or_down == WHATSUP_DOWN_NODES);

  if (up_or_down == WHATSUP_UP_NODES) 
    rv = nodeupdown_get_up_nodes_string(handle, buf, buflen);
  else
    rv = nodeupdown_get_down_nodes_string(handle, buf, buflen);

  if (rv < 0)
    {
      char *msg = nodeupdown_errormsg(handle);
      if (up_or_down == WHATSUP_UP_NODES) 
        err_exit("%s: nodeupdown_get_up_nodes_string(): %s", __FUNCTION__, msg);
      else
        err_exit("%s: nodeupdown_get_down_nodes_string(): %s", __FUNCTION__, msg);
    }
}

/*  
 * _get_nodes
 *
 * get the up or down status of nodes.  The appropriate function call
 * to _get_input_nodes or _get_all_nodes will be done.
 */
static void 
_get_nodes(char *buf, int buflen, int up_or_down, int *count) 
{
  hostlist_t hl = NULL;
  
  assert(buf && buflen > 0 && count);
  assert(up_or_down == WHATSUP_UP_NODES || up_or_down == WHATSUP_DOWN_NODES);

  if (hostlist_count(inputted_nodes) > 0)
    _get_input_nodes(buf, buflen, up_or_down);
  else
    _get_all_nodes(buf, buflen, up_or_down);
  
  /* convert the nodenames if desired */
  if (modules_list_itr) 
    {
      struct whatsup_module_loadinfo *loadinfoPtr;
      int break_flag = 0;
      
      list_iterator_reset(modules_list_itr);
      
      while ((loadinfoPtr = list_next(modules_list_itr)) && !break_flag) 
        {
          struct whatsup_option *optionsPtr = loadinfoPtr->module_info->options;
          while (optionsPtr->option) 
            {
              if (optionsPtr->option_type == WHATSUP_OPTION_TYPE_CONVERT_NODENAMES
                  && strchr(loadinfoPtr->options_processed, optionsPtr->option)) 
                {
                  char tbuf[WHATSUP_BUFFERLEN];
                  
                  if (!(*loadinfoPtr->module_info->convert_nodenames)(buf, tbuf, WHATSUP_BUFFERLEN)) 
                    {
                      if (strlen(tbuf) < buflen)
                        strcpy(buf, tbuf);
                      else
                        err_exit("%s: overflow buffer", __FUNCTION__);
                      
                      break_flag++;
                      break;
                    }
                }
              
              optionsPtr++;
            }
        }
    }
  
  /* can't use nodeupdown_up/down_count, b/c we may be counting the
   * nodes specified by the user on the cmdline
   */
  if (!(hl = hostlist_create(buf)))
    err_exit("%s: hostlist_create()", __FUNCTION__);
  
  *count = hostlist_count(hl);

  hostlist_destroy(hl);
}

/* 
 * _output_nodes
 * 
 * output the nodes specified in the nodebuf to stdout
 */
static void
_output_nodes(char *nodebuf) 
{
  assert(nodebuf);

  if (!output_type)
    fprintf(stdout, "%s\n", nodebuf);
  else 
    {
      char tbuf[WHATSUP_BUFFERLEN];
      hostlist_t hl = NULL;
      char *ptr;

      /* output nodes separated by some break type */
      memset(tbuf, '\0', WHATSUP_BUFFERLEN);
    
      if (!(hl = hostlist_create(nodebuf)))
        err_exit("%s: hostlist_create() error", __FUNCTION__);
      
      if (hostlist_deranged_string(hl, WHATSUP_BUFFERLEN, tbuf) < 0)
        err_exit("%s: hostlist_deranged_string() error", __FUNCTION__);
      
      /* convert commas to appropriate break types */
      if (output_type != ',') 
        {
          while ((ptr = strchr(tbuf, ',')))
            *ptr = (char)output_type;
        }

      /* start on the next line if its a newline separator */
      if (updown_output == WHATSUP_UP_AND_DOWN && output_type == '\n')
        fprintf(stdout, "\n");

      fprintf(stdout,"%s\n", tbuf);
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
_create_formats(char *endstr)
{
  int max;

  assert(up_count >= 0 && down_count >= 0);

  if (up_count > down_count)
    max = _log10(up_count);
  else
    max = _log10(down_count);

  max++;

  snprintf(upfmt,   WHATSUP_FORMATLEN, "up:   %%%dd%s", max, endstr);
  snprintf(downfmt, WHATSUP_FORMATLEN, "down: %%%dd%s", max, endstr);
}

/* 
 * _output_mode
 *
 * Output the current updown state based on inputs from the command line
 *
 * Returns exit_val;
 */
int
_output_mode(void)
{
  int exit_val;

  if (!(handle = nodeupdown_handle_create()))
    err_exit("%s: nodeupdown_handle_create()", __FUNCTION__);

  if (nodeupdown_load_data(handle, hostname, port, 0, module) < 0) 
    {
      int errnum = nodeupdown_errnum(handle);
      char *msg = nodeupdown_errormsg(handle);

      /* Check for "legit" errors and output appropriate message */
      if (errnum == NODEUPDOWN_ERR_CONF_PARSE)
        err_exit("Parse error in conf file");
      else if (errnum == NODEUPDOWN_ERR_CONNECT) 
        err_exit("Cannot connect to server");
      else if (errnum == NODEUPDOWN_ERR_CONNECT_TIMEOUT)
        err_exit("Timeout connecting to server");
      else if (errnum == NODEUPDOWN_ERR_HOSTNAME)
        err_exit("Invalid hostname");
      else
        err_exit("%s: nodeupdown_load_data(): %s", __FUNCTION__, msg);
    }
  
  _get_nodes(up_nodes, WHATSUP_BUFFERLEN, WHATSUP_UP_NODES, &up_count);
  _get_nodes(down_nodes, WHATSUP_BUFFERLEN, WHATSUP_DOWN_NODES, &down_count);
  
  if (count_only_output) 
    {
      if (updown_output == WHATSUP_UP_AND_DOWN) 
        {
          _create_formats("\n");
          fprintf(stdout, upfmt, up_count);
          fprintf(stdout, downfmt, down_count);
        }
      else if (updown_output == WHATSUP_UP_NODES)
        fprintf(stdout, "%d\n", up_count);
      else
        fprintf(stdout, "%d\n", down_count);
    }
  else 
    {    
      /* output up, down, or both up and down nodes */
      if (updown_output == WHATSUP_UP_AND_DOWN) 
        {
          if (output_type == '\n')
            {
              /* newline output is funny, thus special */
              snprintf(upfmt,   WHATSUP_FORMATLEN, "up %d:", up_count);
              snprintf(downfmt, WHATSUP_FORMATLEN, "down %d:", down_count);
            }
          else
            _create_formats(": ");
          
          fprintf(stdout, upfmt, up_count);
          
          _output_nodes(up_nodes);
          
          /* handle odd situation with newline output list */
          if (output_type == '\n')
            fprintf(stdout, "\n");
          
          fprintf(stdout, downfmt, down_count);
          
          _output_nodes(down_nodes);
        }
      else if (updown_output == WHATSUP_UP_NODES)
        _output_nodes(up_nodes);
      else
        _output_nodes(down_nodes);
    }
  
  if (updown_output == WHATSUP_UP_AND_DOWN)
    exit_val = 0;
  else if (updown_output == WHATSUP_UP_NODES)
    exit_val = (!down_count) ? 0 : 1;
  else
    exit_val = (!up_count) ? 0 : 1;
  
  return exit_val;
}

/* 
 * _log_mode
 *
 * Output up/down info as it occurs
 */
int
_log_mode(void)
{
  hostlist_t upnodes, downnodes;
  char upnodesbuf[WHATSUP_BUFFERLEN];
  char downnodesbuf[WHATSUP_BUFFERLEN];
  int nodes_init = 0;
  int exit_val = 0;
  int log_file_fd;

  if (log_file)
    {
      if ((log_file_fd = open (log_file, O_WRONLY | O_CREAT), 600) < 0)
        err_exit("error opening log_file = %s: %s", log_file, strerror(errno));
    }
  else
    log_file_fd = STDOUT_FILENO;

  while (1)
    {
      if (!(handle = nodeupdown_handle_create()))
	err_exit("%s: nodeupdown_handle_create()", __FUNCTION__);

      if (nodeupdown_load_data(handle, hostname, port, 0, module) < 0) 
	{
	  int errnum = nodeupdown_errnum(handle);
	  char *msg = nodeupdown_errormsg(handle);
	  
	  /* Check for "legit" errors and output appropriate message */
	  if (errnum == NODEUPDOWN_ERR_CONF_PARSE)
	    err_exit("Parse error in conf file");
	  else if (errnum == NODEUPDOWN_ERR_CONNECT) 
	    err_exit("Cannot connect to server");
	  else if (errnum == NODEUPDOWN_ERR_CONNECT_TIMEOUT)
	    err_exit("Timeout connecting to server");
	  else if (errnum == NODEUPDOWN_ERR_HOSTNAME)
	    err_exit("Invalid hostname");
	  else
	    err_exit("%s: nodeupdown_load_data(): %s", __FUNCTION__, msg);
	}

      memset(upnodesbuf, '\0', WHATSUP_BUFFERLEN);
      memset(downnodesbuf, '\0', WHATSUP_BUFFERLEN);

      if (nodeupdown_get_up_nodes_string(handle, upnodesbuf, WHATSUP_BUFFERLEN) < 0)
        err_exit("%s: nodeupdown_get_up_nodes_string(): %s", __FUNCTION__, nodeupdown_errormsg(handle));

      if (nodeupdown_get_down_nodes_string(handle, downnodesbuf, WHATSUP_BUFFERLEN) < 0)
        err_exit("%s: nodeupdown_get_down_nodes_string(): %s", __FUNCTION__, nodeupdown_errormsg(handle));

      /* Don't output the first time through */
      if (nodes_init)
	{
	  hostlist_t newupnodes, newdownnodes;
	  hostlist_iterator_t upitr, downitr;
	  char *node;

	  if (!(newupnodes = hostlist_create(upnodesbuf)))
	    err_exit("%s: hostlist_create()", __FUNCTION__);
	  
	  if (!(newdownnodes = hostlist_create(downnodesbuf)))
	    err_exit("%s: hostlist_create()", __FUNCTION__);

	  if (!(upitr = hostlist_iterator_create(newupnodes)))
	    err_exit("%s: hostlist_iterator_create()", __FUNCTION__);

	  if (!(downitr = hostlist_iterator_create(newdownnodes)))
	    err_exit("%s: hostlist_iterator_create()", __FUNCTION__);

	  while ((node = hostlist_next(upitr)))
	    {
	      if (hostlist_find(upnodes, node) < 0)
		{
		  time_t t;
		  struct tm *tt;
		  char timebuf[WHATSUP_BUFFERLEN];
                  char writebuf[WHATSUP_BUFFERLEN];
                  int write_len;

		  t = time(NULL);
		  tt = localtime(&t);
		  strftime(timebuf, WHATSUP_BUFFERLEN, "%Y/%m/%d %T", tt);
                  if ((write_len = snprintf(writebuf, 
                                            WHATSUP_BUFFERLEN,
                                            "%s %s UP\n", 
                                            timebuf, 
                                            node)) < 0)
                    err_exit("snprintf: %s", strerror(errno));

                  if (log_file)
                    {
                      if (fd_write_n (log_file_fd, writebuf, write_len) < 0)
                        err_exit("fd_write_n: %s", strerror(errno));
                      if (fsync(log_file_fd) < 0)
                        err_exit("fsync: %s", strerror(errno));
                    }
                  else
                    {
                      printf("%s", writebuf);
                      fflush(stdout);
                    }
		}
	      free(node);
	    }

	  while ((node = hostlist_next(downitr)))
	    {
	      if (hostlist_find(downnodes, node) < 0)
		{
		  time_t t;
		  struct tm *tt;
		  char timebuf[WHATSUP_BUFFERLEN];
                  char writebuf[WHATSUP_BUFFERLEN];
                  int write_len;

		  t = time(NULL);
		  tt = localtime(&t);
		  strftime(timebuf, WHATSUP_BUFFERLEN, "%Y/%m/%d %T", tt);
                  if ((write_len = snprintf(writebuf, 
                                            WHATSUP_BUFFERLEN,
                                            "%s %s DOWN\n", 
                                            timebuf, 
                                            node)) < 0)
                    err_exit("snprintf: %s", strerror(errno));

                  if (log_file)
                    {
                      if (fd_write_n (log_file_fd, writebuf, write_len) < 0)
                        err_exit("fd_write_n: %s", strerror(errno));
                      if (fsync(log_file_fd) < 0)
                        err_exit("fsync: %s", strerror(errno));
                    }
                  else
                    {
                      printf("%s", writebuf);
                      fflush(stdout);
                    }
		}
	      free(node);
	    }


	  hostlist_destroy(upnodes);
	  hostlist_destroy(downnodes);
	  upnodes = newupnodes;
	  downnodes = newdownnodes;
	}
      else
	{
	  if (!(upnodes = hostlist_create(upnodesbuf)))
	    err_exit("%s: hostlist_create()", __FUNCTION__);

	  if (!(downnodes = hostlist_create(downnodesbuf)))
	    err_exit("%s: hostlist_create()", __FUNCTION__);
  
	  nodes_init++;
	}

      (void)nodeupdown_handle_destroy(handle);
      sleep(log_poll);
    }

  /* NOT REACHED */
  return exit_val;
}

int 
main(int argc, char *argv[]) 
{
  int exit_val;

  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);

  _init_whatsup();

  /* 
   * Load options modules before calling cmdline_parse, b/c modules
   * may be able to parse additional command line options.
   */
  _load_options_modules();

  _cmdline_parse(argc, argv);

  if (log_mode)
    exit_val = _log_mode();
  else
    exit_val = _output_mode();

  _unload_options_modules();
  _cleanup_whatsup();
  exit(exit_val);
}
