/*
 * $Id: whatsup.c,v 1.66 2003-11-06 00:53:30 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/whatsup/whatsup.c,v $
 *    
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#if HAVE_GENDERS
#include <genders.h>
#include <gendersllnl.h>
#endif /* HAVE_GENDERS */

#include "hostlist.h"
#include "nodeupdown.h"

/********************************
 * External Variables           *
 ********************************/

/* getopt */
extern char *optarg;
extern int optind, opterr, optopt;

/********************************
 * Definitions                  *
 ********************************/

#define WHATSUP_OFF          0
#define WHATSUP_ON           1

#define WHATSUP_BUFFERLEN    65536

#define WHATSUP_FORMATLEN    64

#define RANGED_STRING        0
#define DERANGED_STRING      1

#define UP_AND_DOWN          0
#define UP_NODES             1
#define DOWN_NODES           2

#define HOSTLIST             '\0' /* any char that is not ',', '\n', or ' ' */  
#define COMMA                ','
#define NEWLINE              '\n'
#define SPACE                ' '

/* struct winfo
 * - carries information for the entire program
 */
struct winfo {
  nodeupdown_t handle;        /* nodeupdown handle */
  char *hostname;             /* hostname of gmond server */
  char *ip;                   /* ip of gmond server */
  int port;                   /* port of gmond server */  
  int output;                 /* output type */ 
  char list_type;             /* list type */
  int count;                  /* list count? */
  hostlist_t nodes;           /* nodes entered at command line */
#if HAVE_GENDERS
  char *filename;             /* genders filename */
  int list_altnames;          /* list altnames? */
#endif /* HAVE_GENDERS */
};

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
#if HAVE_GENDERS
  fprintf(stderr,
    "  -f STRING  --filename=STRING   Location of genders file\n"
    "  -a         --altnames          List nodes by alternate name\n");
#endif /* HAVE_GENDERS */
    fprintf(stderr, "\n");
}

/* version
 * - output version
 */
static void version(void) {
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
}

/* err_msg
 * - output error statement
 */
static void err_msg(char *msg, char *errno_msg) {
  if (errno_msg == NULL)
    fprintf(stderr, "whatsup error: %s\n", msg);
  else
    fprintf(stderr, "whatsup error: %s, %s\n", msg, errno_msg);
}

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

/* initialize_struct_winfo
 * - initialize struct winfo structure
 */
static int initialize_struct_winfo(struct winfo *winfo) {
  winfo->hostname = NULL;
  winfo->ip = NULL;
  winfo->port = 0;
  winfo->output = UP_AND_DOWN;
  winfo->list_type = HOSTLIST;
  winfo->count = WHATSUP_OFF;
  winfo->nodes = NULL;
#if HAVE_GENDERS
  winfo->filename = NULL;
  winfo->list_altnames = WHATSUP_OFF;
#endif /* HAVE_GENDERS */
  return 0;
}

/* cmdline_parse
 * - parse commandline arguments
 * - store info in a struct winfo strcuture
 */
static int cmdline_parse(struct winfo *winfo, int argc, char **argv) {
  int c, index, oopt = 0, iopt = 0;

#if HAVE_GENDERS
  char *options = "hVo:i:p:budtlcnsf:a";
#else
  char *options = "hVo:i:p:budtlcns";
#endif /* HAVE_GENDERS */
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
#if HAVE_GENDERS
    {"filename",  1, NULL, 'f'},
    {"altnames",  0, NULL, 'a'},
#endif /* HAVE_GENDERS */
    {0, 0, 0, 0}
  };

  /* turn off output messages printed by getopt_long */
  opterr = 0;

  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1) {
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
      winfo->count = WHATSUP_ON;
      break;
    case 'l':
      winfo->list_type = HOSTLIST;
      break;
    case 'c':
      winfo->list_type = COMMA;
      break;
    case 'n':
      winfo->list_type = NEWLINE;
      break;
    case 's':
      winfo->list_type = SPACE;
      break;
#if HAVE_GENDERS
    case 'f':
      winfo->filename = optarg;
      break;
    case 'a':
      winfo->list_altnames = WHATSUP_ON;
      break;
#endif /* HAVE_GENDERS */
    case '?':
      err_msg("invalid command line option entered", NULL);
      return -1;
      break;
    default:
      err_msg("getopt() error", NULL);
      return -1;
      break;
    }
  }

  if (oopt && iopt) {
    err_msg("you cannot specify --hostname and --ip", NULL);
    return -1;
  }

  if ((winfo->nodes = hostlist_create(NULL)) == NULL) {
    err_msg("hostlist_create() error", NULL);
    return -1;
  }

  /* store any nodes listed on the command line */
  index = optind;
  while (index < argc) {
    /* search for periods.  If there are periods, these are non-short hostname
     * machine names. Output error 
     */
    if (strchr(argv[index], '.') != NULL) {
      err_msg("nodes must be listed in short hostname format", NULL);
      return -1;
    }

    if (hostlist_push(winfo->nodes, argv[index]) == 0) {
      err_msg("hostlist_push_host() error", "nodes listed incorrectly");
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
    err_msg("hostlist_create() error", NULL);
    return -1;
  }

  if ((iter = hostlist_iterator_create(winfo->nodes)) == NULL) {
    err_msg("hostlist_iterator_create() error", NULL);
    return -1;
  }

  while ((str = hostlist_next(iter)) != NULL) {
    if (which == UP_NODES) {
      if ((ret = nodeupdown_is_node_up(winfo->handle, str)) < 0) {
        err_msg("nodeupdown_is_node_up()", nodeupdown_errormsg(winfo->handle)); 
        goto cleanup;
      }
    }
    else {
      if ((ret = nodeupdown_is_node_down(winfo->handle, str)) < 0) {
        err_msg("nodeupdown_is_node_down()", nodeupdown_errormsg(winfo->handle)); 
        goto cleanup;
      }
    }

    if (ret == 1) {
      if (hostlist_push_host(hl, str) == 0) {
        err_msg("hostlist_push_host() error", NULL);
        goto cleanup;
      }
    }
    free(str);
  }
  str = NULL;

  hostlist_sort(hl);

  if (hostlist_ranged_string(hl, buflen, buf) < 0) {
    err_msg("hostlist_ranged_string() error", NULL);
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
      err_msg("nodeupdown_get_up_nodes_string()", nodeupdown_errormsg(winfo->handle));
      return -1;
    }
  }
  else {
    if (nodeupdown_get_down_nodes_string(winfo->handle, buf, buflen) < 0) {
      err_msg("nodeupdown_get_down_nodes_string()", nodeupdown_errormsg(winfo->handle));
      return -1;
    }
  }

  return 0;
}

#if HAVE_GENDERS
/* convert_to_altnames
 * - convert nodes in buf to alternate node names
 */
static int convert_to_altnames(struct winfo *winfo, char *buf, int buflen) {
  genders_t handle = NULL;
  char tbuf[WHATSUP_BUFFERLEN];
  int exit_value = -1;
  int tbuflen = WHATSUP_BUFFERLEN;

  memset(tbuf, '\0', WHATSUP_BUFFERLEN);

  if ((handle = genders_handle_create()) == NULL) {
    err_msg("genders_handle_create() error", NULL);
    goto cleanup;
  }

  if (genders_load_data(handle, winfo->filename) == -1) {
    err_msg("genders_load_data()", genders_errormsg(handle));
    goto cleanup;
  }

  if (genders_string_to_altnames_preserve(handle, buf, tbuf, tbuflen) < 0) {
    err_msg("genders_string_to_altnames_preserve()", genders_errormsg(handle));
    goto cleanup;
  }

  if (strlen(tbuf) < buflen)
    strcpy(buf, tbuf);
  else {
    err_msg("Internal buffer overflow", NULL);
    goto cleanup;
  }
    
  exit_value = 0;

 cleanup:

  (void)genders_handle_destroy(handle);
  return exit_value;
}
#endif /* HAVE_GENDERS */

/* get_nodes
 * - a wrapper function used to avoid duplicate code.
 */
int get_nodes(struct winfo *winfo, int which, char *buf, int buflen, int *count) {
  int ret, exit_value = -1;
  hostlist_t hl = NULL;
  
  if (hostlist_count(winfo->nodes) > 0)
    ret = check_arg_nodes(winfo, which, buf, buflen);
  else
    ret = get_all_nodes(winfo, which, buf, buflen);
  
  if (ret != 0)
    goto cleanup;

#if HAVE_GENDERS
  if (winfo->list_altnames == WHATSUP_ON) {
    if (convert_to_altnames(winfo, buf, buflen) == -1)
      goto cleanup;
  }
#endif /* HAVE_GENDERS */

  /* can't use nodeupdown_up/down_count, b/c we may be counting the
   * nodes specified by the user 
   */
  if ((hl = hostlist_create(buf)) == NULL) {
    err_msg("hostlist_create() error", NULL);
    goto cleanup;
  }

  *count = hostlist_count(hl);
  exit_value = 0;

 cleanup:

  hostlist_destroy(hl);
  return exit_value;
}

/* output_nodes
 * - output the nodes indicated in the buffer
 */
int output_nodes(struct winfo *winfo, char *buf) {
  char *ptr;
  char tbuf[WHATSUP_BUFFERLEN];
  hostlist_t hl = NULL;
  int exit_value = -1;

  if (winfo->list_type == HOSTLIST)
    fprintf(stdout, "%s\n", buf);
  else {
    /* output nodes separated by some break type */
    memset(tbuf, '\0', WHATSUP_BUFFERLEN);
    
    if ((hl = hostlist_create(buf)) == NULL) {
      err_msg("hostlist_create() error", NULL);
      goto cleanup;
    }

    if (hostlist_deranged_string(hl, WHATSUP_BUFFERLEN, tbuf) < 0) {
      err_msg("hostlist_deranged_string() error", NULL);
      goto cleanup;
    }

    /* convert commas to appropriate break types */
    if (winfo->list_type != COMMA) {
      while ((ptr = strchr(tbuf, ',')) != NULL)
        *ptr = winfo->list_type;
    }

    fprintf(stdout,"%s\n", tbuf);
  }

  exit_value = 0;

 cleanup:

  hostlist_destroy(hl);
  return exit_value;
}

int main(int argc, char **argv) {
  struct winfo *winfo = NULL;
  char up_nodes[WHATSUP_BUFFERLEN];
  char down_nodes[WHATSUP_BUFFERLEN];
  int up_count, down_count, max, exit_value = 1;
  int buflen = WHATSUP_BUFFERLEN;
  char ufmt[WHATSUP_FORMATLEN];
  char dfmt[WHATSUP_FORMATLEN];

  if (argc == 2) {
    if (strcasecmp(argv[1],"doc") == 0)
      fprintf(stderr,"Shhhhhh.  Be very very quiet.  I'm hunting wabbits.\n");
    if (strcasecmp(argv[1],"dude") == 0)
      fprintf(stderr,"Surfs up man! Cowabunga!\n");
    if (strcasecmp(argv[1],"man") == 0)
      fprintf(stderr, "Nothin much, just chillin ...\n");
  }

  memset(up_nodes, '\0', buflen);
  memset(down_nodes, '\0', buflen);

  if ((winfo = (struct winfo *)malloc(sizeof(struct winfo))) == NULL) {
    err_msg("out of memory", NULL);
    goto cleanup;
  }

  if (initialize_struct_winfo(winfo) != 0) {
    err_msg("initialize_struct_winfo() error", NULL);
    goto cleanup;
  }
  
  if (cmdline_parse(winfo, argc, argv) != 0)
    goto cleanup;
  
  if ((winfo->handle = nodeupdown_handle_create()) == NULL) {
    err_msg("nodeupdown_handle_create() error", NULL);
    goto cleanup;
  }

  if (nodeupdown_load_data(winfo->handle, 
#if HAVE_GENDERS
                           winfo->filename, 
#else
                           NULL,
#endif /* HAVE_GENDERS */
                           winfo->hostname, winfo->ip, winfo->port, 0) == -1) {
    err_msg("nodeupdown_load_data()", nodeupdown_errormsg(winfo->handle)); 
    goto cleanup;
  }

  /* get up nodes */
  if (winfo->output == UP_NODES || winfo->output == UP_AND_DOWN) {
    if (get_nodes(winfo, UP_NODES, up_nodes, buflen, &up_count) == -1)
      goto cleanup;
  }
  
  /* get down nodes */
  if (winfo->output == DOWN_NODES || winfo->output == UP_AND_DOWN) {
    if (get_nodes(winfo, DOWN_NODES, down_nodes, buflen, &down_count) == -1)
      goto cleanup;
  }

  if (winfo->count == WHATSUP_ON) {
    if (winfo->output == UP_AND_DOWN) {
      /* hacks to get the numbers to align */
      max = (up_count > down_count) ? _log10(up_count) : _log10(down_count);
      snprintf(ufmt, WHATSUP_FORMATLEN, "up:   %%%dd\n", ++max);
      snprintf(dfmt, WHATSUP_FORMATLEN, "down: %%%dd\n", max);

      fprintf(stdout, ufmt, up_count);
      fprintf(stdout, dfmt, down_count);
    }
    else if (winfo->output == UP_NODES)
      fprintf(stdout, "%d\n", up_count);
    else
      fprintf(stdout, "%d\n", down_count);
  }
  else {
    /* output up, down, or both up and down nodes */
    if (winfo->output == UP_AND_DOWN) {

      /* hacks to get the numbers to align */
      if (winfo->list_type != NEWLINE) {
        max = (up_count > down_count) ? _log10(up_count) : _log10(down_count);
        snprintf(ufmt, WHATSUP_FORMATLEN, "up:   %%%dd: ", ++max);
        snprintf(dfmt, WHATSUP_FORMATLEN, "down: %%%dd: ", max);
      }
      else {
        /* newline format would have output numbers in a funny way */
        snprintf(ufmt, WHATSUP_FORMATLEN, "up %d:");
        snprintf(dfmt, WHATSUP_FORMATLEN, "down %d:");
      }

      fprintf(stdout, ufmt, up_count);
      
      /* handle odd situation with output formatting */
      if (winfo->list_type == NEWLINE)
        fprintf(stdout, "\n");

      if (output_nodes(winfo, up_nodes) != 0)
        goto cleanup;

      /* handle odd situation with output formatting */
      if (winfo->list_type == NEWLINE)
        fprintf(stdout, "\n");

      fprintf(stdout, dfmt, down_count);

      /* handle odd situation with output formatting */
      if (winfo->list_type == NEWLINE)
        fprintf(stdout, "\n");

      if (output_nodes(winfo, down_nodes) != 0)
        goto cleanup;
    }
    else if (winfo->output == UP_NODES) {
      if (output_nodes(winfo, up_nodes) != 0)
        goto cleanup;
    }
    else {
      if (output_nodes(winfo, down_nodes) != 0)
        goto cleanup;
    }
  }

  exit_value = 0;

 cleanup:

  hostlist_destroy(winfo->nodes);
  if (winfo)
    (void)nodeupdown_handle_destroy(winfo->handle);
  free(winfo);
  exit(exit_value);
}
