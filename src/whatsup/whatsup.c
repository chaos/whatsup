/*
 * $Id: whatsup.c,v 1.46 2003-05-28 16:23:11 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/whatsup/whatsup.c,v $
 *    
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <gendersllnl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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

#define RANGED_STRING        0
#define DERANGED_STRING      1

#define UP_AND_DOWN          0
#define UP_NODES             1
#define DOWN_NODES           2

#define HOSTLIST             '\0' /* any char that is not ',', '\n', or ' ' */  
#define COMMA                ','
#define NEWLINE              '\n'
#define SPACE                ' '

/* struct arginfo
 * - carries information about args passed in from the command line
 * genders_filename - filename of genders file
 * gmond_hostname - hostname of gmond server
 * gmond_ip - ip address of gmond server
 * gmond_port - port of gmond server
 * output - what output should be dumped to screen (up/down)
 * list_type - how nodes should be outputted (hostlist/comma/newline/space)
 * list_altnames - indicates if alternate names should be listed instead
 * nodes - stores the nodes (if any) were input in the command line
 */
struct arginfo {
  char *genders_filename; 
  char *gmond_hostname;
  char *gmond_ip;         
  int gmond_port;         
  int output;
  char list_type;    
  int list_altnames;
  hostlist_t nodes;
};

/********************************
 * Function Declarations        *
 ********************************/

static void usage(void);
static void version(void);
static void err_msg(char *, char *);
static void cleanup_struct_arginfo(struct arginfo *);
static int cmdline_parse(struct arginfo *, int, char **);
static char *get_hostlist_string(hostlist_t, int);
static int get_arg_nodes_common(struct arginfo *, int, nodeupdown_t, char **); 
static int get_all_nodes_common(struct arginfo *, int, nodeupdown_t, char **);
static int convert_to_altnames(struct arginfo *, char **);
static int get_nodes_common(struct arginfo *, int, nodeupdown_t, char **);
static int output_nodes(struct arginfo *, char *nodes);

/* usage
 * - output usage
 */
static void usage(void) {
  fprintf(stderr,
    "Usage: whatsup [OPTIONS]... [NODES]...\n"
    "  -h         --help              Print help and exit\n"
    "  -V         --version           Print version and exit\n"
    "  -f STRING  --filename=STRING   Location of genders file\n"
    "  -o STRING  --hostname=STRING   gmond server hostname\n"
    "  -i STRING  --ip=STRING         gmond server IP address\n"
    "  -p INT     --port=INT          gmond server port\n"
    "  -b         --updown            List both up and down nodes\n"
    "  -u         --up                List only up nodes\n"
    "  -d         --down              List only down nodes\n"
    "  -l         --hostlist          List nodes in hostlist format\n"
    "  -c         --comma             List nodes in comma separated list\n"
    "  -n         --newline           List nodes in newline separated list\n"
    "  -s         --space             List nodes in space separated list\n"
    "  -a         --altnames          List nodes by alternate name\n"
    "\n");
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

  fprintf(stderr, "whatsup error: %s, %s\n", msg, errno_msg);
}

/* initialize_struct_arginfo
 * - initialize struct arginfo structure
 */
static int initialize_struct_arginfo(struct arginfo *arginfo) {
  arginfo->genders_filename = NULL;
  arginfo->gmond_hostname = NULL;
  arginfo->gmond_ip = NULL;
  arginfo->gmond_port = 0;
  arginfo->output = UP_AND_DOWN;
  arginfo->list_type = HOSTLIST;
  arginfo->list_altnames = WHATSUP_OFF;
  arginfo->nodes = NULL;
  return 0;
}

/* cleanup_struct_arginfo
 * - free memory allocated in a struct arginfo structure
 */
static void cleanup_struct_arginfo(struct arginfo *arginfo) {
  free(arginfo->genders_filename);
  free(arginfo->gmond_ip);
  hostlist_destroy(arginfo->nodes);
  free(arginfo);
}

/* cmdline_parse
 * - parse commandline arguments
 * - store info in a struct arginfo strcuture
 */
static int cmdline_parse(struct arginfo *arginfo, int argc, char **argv) {
  int c, index, fopt = 0, oopt = 0, iopt = 0;
  char *filename, *hostname, *ip;

  char *options = "hVf:o:i:p:budlcnsag:";
  struct option long_options[] = {
    {"help",      0, NULL, 'h'},
    {"version",   0, NULL, 'V'},
    {"filename",  1, NULL, 'f'},
    {"hostname",  1, NULL, 'o'},
    {"ip",        1, NULL, 'i'},
    {"port",      1, NULL, 'p'},
    {"updown",    0, NULL, 'b'},
    {"up",        0, NULL, 'u'},
    {"down",      0, NULL, 'd'},
    {"altnames",  0, NULL, 'a'},
    {"hostlist",  0, NULL, 'l'},
    {"comma",     0, NULL, 'c'},
    {"newline",   0, NULL, 'n'},
    {"space",     0, NULL, 's'},
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
    case 'f':
      fopt++;
      filename = optarg;
      break;
    case 'o':
      oopt++;
      hostname = optarg;
      break;
    case 'i':
      iopt++;
      ip = optarg;
      break;
    case 'p':
      arginfo->gmond_port = atoi(optarg);
      break;
    case 'b':
      arginfo->output = UP_AND_DOWN;
      break;
    case 'u':
      arginfo->output = UP_NODES;
      break;
    case 'd':
      arginfo->output = DOWN_NODES;
      break;
    case 'l':
      arginfo->list_type = HOSTLIST;
      break;
    case 'c':
      arginfo->list_type = COMMA;
      break;
    case 'n':
      arginfo->list_type = NEWLINE;
      break;
    case 's':
      arginfo->list_type = SPACE;
      break;
    case 'a':
      arginfo->list_altnames = WHATSUP_ON;
      break;
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

  if (fopt > 0) { 
    if ((arginfo->genders_filename = strdup(filename)) == NULL) {
      err_msg("out of memory", NULL);
      return -1;
    }
  }

  if ((oopt + iopt) > 1) {
    err_msg("you cannot specify --gmond_hostname and --gmond_ip", NULL);
    return -1;
  }
  else if (oopt > 0) {
    if ((arginfo->gmond_hostname = strdup(hostname)) == NULL) {
      err_msg("out of memory", NULL);
      return -1;
    }
  }
  else if (iopt > 0) {
    if ((arginfo->gmond_ip = strdup(ip)) == NULL) {
      err_msg("out of memory", NULL);
      return -1;
    }
  }

  if ((arginfo->nodes = hostlist_create(NULL)) == NULL) {
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

    if (hostlist_push(arginfo->nodes, argv[index]) == 0) {
      err_msg("hostlist_push_host() error", "nodes listed incorrectly");
      return -1;
    }

    index++;
  }

  /* remove any duplicate nodes listed */
  hostlist_uniq(arginfo->nodes);

  return 0;
}

/* get_hostlist_string
 * - get a hostlist ranged/deranged string 
 */
char * get_hostlist_string(hostlist_t hl, int which) {
  char *str = NULL;
  int ret, str_len = 0;

  do {
    free(str);
    str_len += WHATSUP_BUFFERLEN;
    if ((str = (char *)malloc(str_len)) == NULL) {
      err_msg("out of memory", NULL);
      return NULL;
    }
    memset(str, '\0', str_len);

    if (which == RANGED_STRING)
      ret = hostlist_ranged_string(hl, str_len, str);
    else
      ret = hostlist_deranged_string(hl, str_len, str);
  } while (ret == -1);
  
  return str;
}

/* get_arg_nodes_common
 * - determine if specific nodes passed in at the command line are up or down 
 */
int get_arg_nodes_common(struct arginfo *arginfo, 
                         int output,
                         nodeupdown_t handle, 
                         char **nodes) {
  hostlist_t hl = NULL;
  hostlist_iterator_t iter = NULL;
  char *str = NULL;
  int ret;

  if ((hl = hostlist_create(NULL)) == NULL) {
    err_msg("hostlist_create() error", NULL);
    return -1;
  }

  if ((iter = hostlist_iterator_create(arginfo->nodes)) == NULL) {
    err_msg("hostlist_iterator_create() error", NULL);
    return -1;
  }

  while ((str = hostlist_next(iter)) != NULL) {
    if (output == UP_NODES)
      ret = nodeupdown_is_node_up(handle, str);
    else
      ret = nodeupdown_is_node_down(handle, str);

    if (ret == -1) {
      if (output == UP_NODES)
        err_msg("nodeupdown_is_node_up()", nodeupdown_errormsg(handle)); 
      else
        err_msg("nodeupdown_is_node_down()", nodeupdown_errormsg(handle)); 
      goto cleanup;
    }
    else if (ret == 1) {
      if (hostlist_push_host(hl, str) == 0) {
        err_msg("hostlist_push_host() error", NULL);
        goto cleanup;
      }
    }
    free(str);
  }
  str = NULL;

  hostlist_sort(hl);

  if ((str = get_hostlist_string(hl, RANGED_STRING)) == NULL)
    goto cleanup;

  hostlist_iterator_destroy(iter);
  hostlist_destroy(hl);

  *nodes = str;
  
  return 0;

 cleanup:

  free(str);
  
  hostlist_iterator_destroy(iter);
  hostlist_destroy(hl);

  return -1;
}

/* get_all_nodes_common
 * - get all up or down nodes
 */
int get_all_nodes_common(struct arginfo *arginfo, 
                         int output,
                         nodeupdown_t handle, 
                         char **nodes) {
  int ret, str_len = 0;
  char *str = NULL;

  do {
    free(str);
    str_len += WHATSUP_BUFFERLEN;
    if ((str = (char *)malloc(str_len)) == NULL) {
      err_msg("out of memory", NULL);
      return -1;
    }
    memset(str, '\0', str_len);
    
    if (output == UP_NODES)
      ret = nodeupdown_get_up_nodes_string(handle, str, str_len);
    else
      ret = nodeupdown_get_down_nodes_string(handle, str, str_len);

    if (ret == -1) {
      if (nodeupdown_errnum(handle) != NODEUPDOWN_ERR_OVERFLOW) {
        if (output == UP_NODES)
          err_msg("nodeupdown_get_up_nodes_string()", 
                  nodeupdown_errormsg(handle));
        else 
          err_msg("nodeupdown_get_down_nodes_string()", 
                  nodeupdown_errormsg(handle));
        free(str);
        return -1;
      }
    }
  } while (nodeupdown_errnum(handle) == NODEUPDOWN_ERR_OVERFLOW); 
  
  *nodes = str;
  return 0;
}

int convert_to_altnames(struct arginfo *arginfo, char **nodes) {
  genders_t handle = NULL;
  char *buf = NULL;
  int ret, buflen = 0;

  if ((handle = genders_handle_create()) == NULL) {
    err_msg("genders_handle_create() error", NULL);
    goto cleanup;
  }

  if (genders_load_data(handle, arginfo->genders_filename) == -1) {
    err_msg("genders_load_data()", genders_errormsg(handle));
    goto cleanup;
  }

  do {
    free(buf);
    buflen += WHATSUP_BUFFERLEN;
    if ((buf = (char *)malloc(buflen)) == NULL) {
      err_msg("out of memory", NULL);
      goto cleanup;
    }
    memset(buf, '\0', buflen);

    ret = genders_string_to_altnames_preserve(handle, *nodes, buf, buflen);
  } while (ret == -1 && genders_errnum(handle) == GENDERS_ERR_OVERFLOW);

  if (ret == -1) {
    err_msg("genders_string_to_altnames_preserve()", genders_errormsg(handle));
    goto cleanup;
  }
  
  if (genders_handle_destroy(handle) == -1) {
    err_msg("genders_handle_destroy()", genders_errormsg(handle));
    goto cleanup;
  }

  free(*nodes);

  *nodes = buf;

  return 0;

 cleanup:

  (void)genders_handle_destroy(handle);
  free(buf);
  return -1;
}

/* get_nodes_common
 * - a wrapper function used to avoid duplicate code.
 */
int get_nodes_common(struct arginfo *arginfo, 
                     int output, 
                     nodeupdown_t handle, 
                     char **nodes) {
  int ret;
  char *str = NULL;
  
  if (hostlist_count(arginfo->nodes) > 0)
    ret = get_arg_nodes_common(arginfo, output, handle, &str);
  else
    ret = get_all_nodes_common(arginfo, output, handle, &str);
  
  if (ret != 0)
    goto cleanup;

  if (arginfo->list_altnames == WHATSUP_ON) {
    if (convert_to_altnames(arginfo, &str) == -1)
      goto cleanup;
  }

  *nodes = str;

  return 0;

 cleanup:

  free(str);
  return -1;
}

/* output_nodes
 * - output the nodes indicated in nodes
 */
int output_nodes(struct arginfo *arginfo, char *nodes) {
  char *str = NULL;
  char *ptr;
  hostlist_t hl = NULL;

  if (arginfo->list_type == HOSTLIST)
    fprintf(stdout, "%s\n", nodes);
  else {
    /* output nodes separated by some break type */
    
    if ((hl = hostlist_create(nodes)) == NULL) {
      err_msg("hostlist_create() error", NULL);
      goto cleanup;
    }

    if ((str = get_hostlist_string(hl, DERANGED_STRING)) == NULL)
      goto cleanup;

    /* convert commas to appropriate break types */
    if (arginfo->list_type != COMMA) {
      while ((ptr = strchr(str, ',')) != NULL)
        *ptr = arginfo->list_type;
    }

    fprintf(stdout,"%s\n", str);

    free(str);
    hostlist_destroy(hl);
  }

  return 0;

 cleanup:

  free(str);
  hostlist_destroy(hl);
  return -1;
}

int main(int argc, char **argv) {
  struct arginfo *arginfo = NULL;
  nodeupdown_t handle = NULL;
  char *up_nodes = NULL;
  char *down_nodes = NULL;

  if ((arginfo = (struct arginfo *)malloc(sizeof(struct arginfo))) == NULL) {
    err_msg("out of memory", NULL);
    goto cleanup;
  }

  if (initialize_struct_arginfo(arginfo) != 0) {
    err_msg("initialize_struct_arginfo() error", NULL);
    goto cleanup;
  }
  
  if (cmdline_parse(arginfo, argc, argv) != 0)
    goto cleanup;
  
  if ((handle = nodeupdown_handle_create()) == NULL) {
    err_msg("nodeupdown_handle_create() error", NULL);
    goto cleanup;
  }

  if (nodeupdown_load_data(handle, 
                           arginfo->genders_filename, 
                           arginfo->gmond_hostname, 
                           arginfo->gmond_ip, 
                           arginfo->gmond_port,
                           0) == -1) {
    err_msg("nodeupdown_load_data()", nodeupdown_errormsg(handle)); 
    goto cleanup;
  }

  /* get up nodes */
  if (arginfo->output == UP_NODES || arginfo->output == UP_AND_DOWN) {
    if (get_nodes_common(arginfo, UP_NODES, handle, &up_nodes) == -1)
      goto cleanup;
  }

  /* get down nodes */
  if (arginfo->output == DOWN_NODES || arginfo->output == UP_AND_DOWN) {
    if (get_nodes_common(arginfo, DOWN_NODES, handle, &down_nodes) == -1)
      goto cleanup;
  }

  /* output up, down, or both up and down nodes */
  if (arginfo->output == UP_AND_DOWN) {
    fprintf(stdout, "up:\t");

    /* handle odd situation with output formatting */
    if (arginfo->list_type == NEWLINE)
      fprintf(stdout, "\n");

    if (output_nodes(arginfo, up_nodes) != 0)
      goto cleanup;

    /* handle odd situation with output formatting */
    if (arginfo->list_type == NEWLINE)
      fprintf(stdout, "\n");
 
    fprintf(stdout, "down:\t");

    /* handle odd situation with output formatting */
    if (arginfo->list_type == NEWLINE)
      fprintf(stdout, "\n");

    if (output_nodes(arginfo, down_nodes) != 0)
      goto cleanup;
  }
  else if (arginfo->output == UP_NODES) {
    if (output_nodes(arginfo, up_nodes) != 0)
      goto cleanup;
  }
  else if (arginfo->output == DOWN_NODES) {
    if (output_nodes(arginfo, down_nodes) != 0)
      goto cleanup;
  }

  cleanup_struct_arginfo(arginfo);
  (void)nodeupdown_handle_destroy(handle);
  free(up_nodes);
  free(down_nodes);

  exit(0);

 cleanup:

  cleanup_struct_arginfo(arginfo);
  (void)nodeupdown_handle_destroy(handle);
  free(up_nodes);
  free(down_nodes);

  exit(1);
}
