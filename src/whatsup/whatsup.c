/*
 * $Id: whatsup.c,v 1.74 2003-11-14 02:14:50 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/whatsup/whatsup.c,v $
 *    
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#if HAVE_GENDERS
#include <genders.h>
#elif HAVE_GENDERSLLNL
#include <gendersllnl.h>
#endif

#include "hostlist.h"
#include "nodeupdown.h"

/* External Variables */

extern char *optarg;
extern int optind, opterr, optopt;

/* Definitions */
 
typedef enum {WHATSUP_TRUE, WHATSUP_FALSE} whatsup_bool_t;
typedef enum {UP_NODES, DOWN_NODES, UP_AND_DOWN} whatsup_output_t;
typedef enum {WHATSUP_HOSTLIST = '\0', /* anything not ',', '\n', or ' ' */
              WHATSUP_COMMA = ',',
              WHATSUP_NEWLINE = '\n',
              WHATSUP_SPACE = ' '} whatsup_list_t;

#define WHATSUP_BUFFERLEN    65536
#define WHATSUP_FORMATLEN    64

/* struct winfo
 * - carries information for the entire program
 */
struct winfo {
  nodeupdown_t handle;        /* nodeupdown handle */
  char *hostname;             /* hostname of gmond server */
  char *ip;                   /* ip of gmond server */
  int port;                   /* port of gmond server */  
  whatsup_output_t output;    /* output type */ 
  whatsup_list_t list;        /* list type */
  whatsup_bool_t count;       /* list count? */
  hostlist_t nodes;           /* nodes entered at command line */
#if (HAVE_HOSTSFILE || HAVE_GENDERS || HAVE_GENDERSLLNL)
  char *filename;             /* filename */
#endif
#if HAVE_GENDERSLLNL
  whatsup_bool_t altnames;    /* list altnames? */
#endif
};

/* _log10
 * - a simple log 10 function for ints
 */
static int _log10(int num) {
  int count = 0;

  if (num > 0) {
    while ((num /= 10) > 0)
      count++;
  }

  return count;
} 

/* usage
 * - output usage
 */
static void usage(void) {
  fprintf(stderr,
    "Usage: whatsup [OPTIONS]... [NODES]...\n"
    "  -h         --help              Print help and exit\n"
    "  -V         --version           Print version and exit\n"
    "  -o STRING  --hostname=STRING   gmond server hostname\n"
    "  -i STRING  --ip=STRING         gmond server IP address\n"
    "  -p INT     --port=INT          gmond server port\n"
    "  -b         --updown            List both up and down nodes\n"
    "  -u         --up                List only up nodes\n"
    "  -d         --down              List only down nodes\n"
    "  -t         --count             List only node counts\n"
    "  -l         --hostlist          List nodes in hostlist format\n"
    "  -c         --comma             List nodes in comma separated list\n"
    "  -n         --newline           List nodes in newline separated list\n"
    "  -s         --space             List nodes in space separated list\n");
#if HAVE_HOSTSFILE
  fprintf(stderr,
    "  -f STRING  --filename=STRING   Location of master list file\n");
#endif
#if (HAVE_GENDERS || HAVE_GENDERSLLNL)
  fprintf(stderr,
    "  -f STRING  --filename=STRING   Location of genders file\n");
#endif
#if HAVE_GENDERSLLNL
  fprintf(stderr,
    "  -a         --altnames          List nodes by alternate name\n");
#endif
    fprintf(stderr, "\n");
}

/* version
 * - output version
 */
static void version(void) {
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
}

/* cmdline_parse
 * - parse command line arguments
 * - store info in a struct winfo strcuture
 */
static int cmdline_parse(struct winfo *winfo, int argc, char **argv) {
  int c, index, oopt = 0, iopt = 0;
  char options[100];

  strcpy(options, "hVo:i:p:budtlcns");
#if (HAVE_HOSTSFILE || HAVE_GENDERS)
  strcat(options, "f:");
#elif HAVE_GENDERSLLNL
  strcat(options, "f:a");
#endif
#if HAVE_GETOPT_LONG
  struct option long_options[] = {
    {"help",      0, NULL, 'h'},
    {"version",   0, NULL, 'V'},
    {"hostname",  1, NULL, 'o'},
    {"ip",        1, NULL, 'i'},
    {"port",      1, NULL, 'p'},
    {"updown",    0, NULL, 'b'},
    {"up",        0, NULL, 'u'},
    {"down",      0, NULL, 'd'},
    {"count",     0, NULL, 't'},
    {"hostlist",  0, NULL, 'l'},
    {"comma",     0, NULL, 'c'},
    {"newline",   0, NULL, 'n'},
    {"space",     0, NULL, 's'},
#if (HAVE_HOSTSFILE || HAVE_GENDERS || HAVE_GENDERSLLNL)
    {"filename",  1, NULL, 'f'},
#endif
#if HAVE_GENDERSLLNL
    {"altnames",  0, NULL, 'a'},
#endif 
    {0, 0, 0, 0}
  };
#endif /* HAVE_GETOPT_LONG */

  /* turn off output messages printed by getopt_long */
  opterr = 0;

#if HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1) {
#else
  while ((c = getopt(argc, argv, options)) != -1) { 
#endif
    switch(c) {
    case 'h':
      usage();
      return -1;
      break;
    case 'V':
      version();
      return -1;
      break;
    case 'o':
      oopt++;
      winfo->hostname = optarg;
      break;
    case 'i':
      iopt++;
      winfo->ip = optarg;
      break;
    case 'p':
      winfo->port = atoi(optarg);
      break;
    case 'b':
      winfo->output = UP_AND_DOWN;
      break;
    case 'u':
      winfo->output = UP_NODES;
      break;
    case 'd':
      winfo->output = DOWN_NODES;
      break;
    case 't':
      winfo->count = WHATSUP_TRUE;
      break;
    case 'l':
      winfo->list = WHATSUP_HOSTLIST;
      break;
    case 'c':
      winfo->list = WHATSUP_COMMA;
      break;
    case 'n':
      winfo->list = WHATSUP_NEWLINE;
      break;
    case 's':
      winfo->list = WHATSUP_SPACE;
      break;
#if (HAVE_HOSTSFILE || HAVE_GENDERS || HAVE_GENDERSLLNL)
    case 'f':
      winfo->filename = optarg;
      break;
#endif
#if HAVE_GENDERSLLNL
    case 'a':
      winfo->altnames = WHATSUP_TRUE;
      break;
#endif 
    case '?':
      fprintf(stderr, "Usage: invalid command line option entered\n");
      return -1;
      break;
    default:
      fprintf(stderr, "Error: getopt() error\n");
      return -1;
      break;
    }
  }

  if (oopt && iopt) {
    fprintf(stderr, "Usage: you cannot specify --hostname and --ip\n");
    return -1;
  }

  if ((winfo->nodes = hostlist_create(NULL)) == NULL) {
    fprintf(stderr, "cmdline_parse: hostlist_create()\n");
    return -1;
  }

  /* store any nodes listed on the command line */
  index = optind;
  while (index < argc) {
    /* search for periods.  If there are periods, these are non-short hostname
     * machine names. Output error 
     */
    if (strchr(argv[index], '.') != NULL) {
      fprintf(stderr, "Usage: nodes must be listed in short hostname format\n");
      return -1;
    }

    if (hostlist_push(winfo->nodes, argv[index]) == 0) {
      fprintf(stderr, "Usage: nodes listed incorrectly\n");
      return -1;
    }

    index++;
  }

  /* remove any duplicate nodes listed */
  hostlist_uniq(winfo->nodes);
  return 0;
}

/* check_arg_nodes
 * - determine if specific nodes passed in at the command line are up or down 
 */
static int check_arg_nodes(struct winfo *winfo, int which, char *buf, int buflen) {
  hostlist_t hl = NULL;
  hostlist_iterator_t iter = NULL;
  char *str = NULL;
  int ret, exit_value = -1;

  if ((hl = hostlist_create(NULL)) == NULL) {
    fprintf(stderr, "check_arg_nodes: hostlist_create()\n");
    return -1;
  }

  if ((iter = hostlist_iterator_create(winfo->nodes)) == NULL) {
    fprintf(stderr, "check_arg_nodes: hostlist_iterator_create()\n");
    return -1;
  }

  while ((str = hostlist_next(iter)) != NULL) {
    if (which == UP_NODES) {
      if ((ret = nodeupdown_is_node_up(winfo->handle, str)) < 0) {
        fprintf(stderr, "check_arg_nodes: nodeupdown_is_node_up(): %s\n", 
		nodeupdown_errormsg(winfo->handle)); 
        goto cleanup;
      }
    }
    else {
      if ((ret = nodeupdown_is_node_down(winfo->handle, str)) < 0) {
        fprintf(stderr, "check_arg_nodes: nodeupdown_is_node_down(): %s\n", 
		nodeupdown_errormsg(winfo->handle)); 
        goto cleanup;
      }
    }

    if (ret == 1) {
      if (hostlist_push_host(hl, str) == 0) {
        fprintf(stderr, "check_arg_nodes: hostlist_push_host()\n");
        goto cleanup;
      }
    }
    free(str);
  }
  str = NULL;

  hostlist_sort(hl);

  if (hostlist_ranged_string(hl, buflen, buf) < 0) {
    fprintf(stderr, "check_arg_nodes: hostlist_ranged_string()\n");
    goto cleanup;
  }

  exit_value = 0;

 cleanup:
  free(str);
  hostlist_iterator_destroy(iter);
  hostlist_destroy(hl);
  return exit_value;
}

/* get_all_nodes
 * - get all up or down nodes
 */
static int get_all_nodes(struct winfo *winfo, int which, char *buf, int buflen) {
  int ret;

  if (which == UP_NODES) {
    if (nodeupdown_get_up_nodes_string(winfo->handle, buf, buflen) < 0) {
      fprintf(stderr, "get_all_nodes: nodeupdown_get_up_nodes_string(): %s\n", 
	      nodeupdown_errormsg(winfo->handle));
      return -1;
    }
  }
  else {
    if (nodeupdown_get_down_nodes_string(winfo->handle, buf, buflen) < 0) {
      fprintf(stderr, "get_all_nodes: nodeupdown_get_down_nodes_string(): %s\n",
	      nodeupdown_errormsg(winfo->handle));
      return -1;
    }
  }

  return 0;
}

#if HAVE_GENDERSLLNL
/* convert_to_altnames
 * - convert nodes in buf to alternate node names
 */
static int convert_to_altnames(struct winfo *winfo, char *buf, int buflen) {
  genders_t handle = NULL;
  char tbuf[WHATSUP_BUFFERLEN];
  int retval = -1;
  int tbuflen = WHATSUP_BUFFERLEN;

  memset(tbuf, '\0', WHATSUP_BUFFERLEN);

  if ((handle = genders_handle_create()) == NULL) {
    fprintf(stderr, "convert_to_altnames: genders_handle_create()\n");
    goto cleanup;
  }

  if (genders_load_data(handle, winfo->filename) == -1) {
    fprintf(stderr, "convert_to_altnames: genders_load_data(): %s\n", 
	    genders_errormsg(handle));
    goto cleanup;
  }

  if (genders_string_to_altnames_preserve(handle, buf, tbuf, tbuflen) < 0) {
    fprintf(stderr, 
	    "convert_to_altnames: genders_string_to_altnames_preserve(): %s\n", 
	    genders_errormsg(handle));
    goto cleanup;
  }

  if (strlen(tbuf) < buflen)
    strcpy(buf, tbuf);
  else {
    fprintf(stderr, "convert_to_altnames: Internal buffer overflow\n");
    goto cleanup;
  }
    
  retval = 0;

 cleanup:
  (void)genders_handle_destroy(handle);
  return retval;
}
#endif /* HAVE_GENDERSLLNL */

/* get_nodes
 * - a wrapper function used to avoid duplicate code.
 */
int get_nodes(struct winfo *winfo, int which, char *buf, int buflen, int *count) {
  int ret, retval = -1;
  hostlist_t hl = NULL;
  
  if (hostlist_count(winfo->nodes) > 0)
    ret = check_arg_nodes(winfo, which, buf, buflen);
  else
    ret = get_all_nodes(winfo, which, buf, buflen);
  
  if (ret != 0)
    goto cleanup;

#if HAVE_GENDERSLLNL
  if (winfo->altnames == WHATSUP_TRUE) {
    if (convert_to_altnames(winfo, buf, buflen) == -1)
      goto cleanup;
  }
#endif /* HAVE_GENDERSLLNL */

  /* can't use nodeupdown_up/down_count, b/c we may be counting the
   * nodes specified by the user 
   */
  if ((hl = hostlist_create(buf)) == NULL) {
    fprintf(stderr, "get_nodes: hostlist_create()\n");
    goto cleanup;
  }

  *count = hostlist_count(hl);
  retval = 0;

 cleanup:
  hostlist_destroy(hl);
  return retval;
}

/* output_nodes
 * - output the nodes indicated in the buffer
 */
int output_nodes(struct winfo *winfo, char *buf) {
  char *ptr;
  char tbuf[WHATSUP_BUFFERLEN];
  hostlist_t hl = NULL;
  int retval = -1;

  if (winfo->list == WHATSUP_HOSTLIST)
    fprintf(stdout, "%s\n", buf);
  else {
    /* output nodes separated by some break type */
    memset(tbuf, '\0', WHATSUP_BUFFERLEN);
    
    if ((hl = hostlist_create(buf)) == NULL) {
      fprintf(stderr, "output_nodes: hostlist_create() error\n");
      goto cleanup;
    }

    if (hostlist_deranged_string(hl, WHATSUP_BUFFERLEN, tbuf) < 0) {
      fprintf(stderr, "output_nodes: hostlist_deranged_string() error\n");
      goto cleanup;
    }

    /* convert commas to appropriate break types */
    if (winfo->list != WHATSUP_COMMA) {
      while ((ptr = strchr(tbuf, ',')) != NULL)
        *ptr = (char)winfo->list;
    }

    /* start on the next line */
    if (winfo->output == UP_AND_DOWN && winfo->list == WHATSUP_NEWLINE)
      fprintf(stdout, "\n");

    fprintf(stdout,"%s\n", tbuf);
  }

  retval = 0;

 cleanup:
  hostlist_destroy(hl);
  return retval;
}

int main(int argc, char **argv) {
  struct winfo winfo;
  char up_nodes[WHATSUP_BUFFERLEN];
  char down_nodes[WHATSUP_BUFFERLEN];
  char upfmt[WHATSUP_FORMATLEN];
  char downfmt[WHATSUP_FORMATLEN];
  int up_count, down_count, max, retval = 1;
  int buflen = WHATSUP_BUFFERLEN;

  if (argc == 2) {
    if (strcasecmp(argv[1],"doc") == 0)
      fprintf(stderr,"Shhhhhh.  Be very very quiet.  I'm hunting wabbits.\n");
    if (strcasecmp(argv[1],"dude") == 0)
      fprintf(stderr,"Surfs up man! Cowabunga!\n");
    if (strcasecmp(argv[1],"man") == 0)
      fprintf(stderr, "Nothin much, just chillin ...\n");
  }

  /* Initialize winfo structure with defaults */
  winfo.hostname = NULL;
  winfo.ip = NULL;
  winfo.port = 0;
  winfo.output = UP_AND_DOWN;
  winfo.list = WHATSUP_HOSTLIST;
  winfo.count = WHATSUP_FALSE;
  winfo.nodes = NULL;
#if (HAVE_HOSTSFILE || HAVE_GENDERS || HAVE_GENDERSLLNL)
  winfo.filename = NULL;
#endif
#if HAVE_GENDERSLLNL
  winfo.altnames = WHATSUP_FALSE;
#endif
  
  if (cmdline_parse(&winfo, argc, argv) != 0)
    goto cleanup;
  
  if ((winfo.handle = nodeupdown_handle_create()) == NULL) {
    fprintf(stderr, "main: nodeupdown_handle_create() error\n");
    goto cleanup;
  }

  if (nodeupdown_load_data(winfo.handle, 
#if (HAVE_HOSTSFILE || HAVE_GENDERS || HAVE_GENDERSLLNL)
                           winfo.filename, 
#else
                           NULL,
#endif
                           winfo.hostname, winfo.ip, winfo.port, 0) == -1) {
    fprintf(stderr, "main: nodeupdown_load_data(): %s\n", 
	    nodeupdown_errormsg(winfo.handle)); 
    goto cleanup;
  }

  /* get up nodes */
  if (winfo.output == UP_NODES || winfo.output == UP_AND_DOWN) {
    if (get_nodes(&winfo, UP_NODES, up_nodes, buflen, &up_count) == -1)
      goto cleanup;
  }
  
  /* get down nodes */
  if (winfo.output == DOWN_NODES || winfo.output == UP_AND_DOWN) {
    if (get_nodes(&winfo, DOWN_NODES, down_nodes, buflen, &down_count) == -1)
      goto cleanup;
  }

  /* only output count */
  if (winfo.count == WHATSUP_TRUE) {
    if (winfo.output == UP_AND_DOWN) {
      /* hacks to get the numbers to align */
      max = (up_count > down_count) ? _log10(up_count) : _log10(down_count);
      snprintf(upfmt,   WHATSUP_FORMATLEN, "up:   %%%dd\n", ++max);
      snprintf(downfmt, WHATSUP_FORMATLEN, "down: %%%dd\n", max);

      fprintf(stdout, upfmt, up_count);
      fprintf(stdout, downfmt, down_count);
    }
    else if (winfo.output == UP_NODES)
      fprintf(stdout, "%d\n", up_count);
    else
      fprintf(stdout, "%d\n", down_count);
  }
  else {    /* output up, down, or both up and down nodes */
    if (winfo.output == UP_AND_DOWN) {
      /* hacks to get the numbers to align */
      if (winfo.list != WHATSUP_NEWLINE) {
        max = (up_count > down_count) ? _log10(up_count) : _log10(down_count);
        snprintf(upfmt,   WHATSUP_FORMATLEN, "up:   %%%dd: ", ++max);
        snprintf(downfmt, WHATSUP_FORMATLEN, "down: %%%dd: ", max);
      }
      else {
        /* newline format would have output numbers in a funny way */
        snprintf(upfmt,   WHATSUP_FORMATLEN, "up %d:");
        snprintf(downfmt, WHATSUP_FORMATLEN, "down %d:");
      }

      fprintf(stdout, upfmt, up_count);
      
      if (output_nodes(&winfo, up_nodes) != 0)
        goto cleanup;

      /* handle odd situation with output formatting */
      if (winfo.list == WHATSUP_NEWLINE)
        fprintf(stdout, "\n");

      fprintf(stdout, downfmt, down_count);

      if (output_nodes(&winfo, down_nodes) != 0)
        goto cleanup;
    }
    else if (winfo.output == UP_NODES) {
      if (output_nodes(&winfo, up_nodes) != 0)
        goto cleanup;
    }
    else {
      if (output_nodes(&winfo, down_nodes) != 0)
        goto cleanup;
    }
  }

  retval = 0;

 cleanup:
  hostlist_destroy(winfo.nodes);
  (void)nodeupdown_handle_destroy(winfo.handle);
  exit(retval);
}
