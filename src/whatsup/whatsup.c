/*
 * $Id: whatsup.c,v 1.40 2003-05-23 01:03:07 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/whatsup/whatsup.c,v $
 *    
 */

#include <errno.h>
#include <ganglia.h>
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

/* whatsup_output_type
 * - indicates if output should be nodes that are up or down
 */
enum whatsup_output_type {WHATSUP_UP_AND_DOWN, WHATSUP_UP, WHATSUP_DOWN};

/* whatsup_list_type
 * - indicates how output should be listed, in hostlist, comma separated, 
 *   neweline separated, or space separated listing.
 */
enum whatsup_list_type {WHATSUP_HOSTLIST, 
                        WHATSUP_COMMA, 
                        WHATSUP_NEWLINE, 
                        WHATSUP_SPACE};

/* struct arginfo
 * - carries information about args passed in from the command line
 * genders_filename - filename of genders file
 * gmond_hostname - hostname of gmond server
 * gmond_ip - ip address of gmond server
 * gmond_port - port of gmond server
 * output_type - what output should be dumped to screen (up/down)
 * list_type - how nodes should be outputted (hostlist/comma/newline/space)
 * list_altnames - indicates if alternate names should be listed instead
 * nodes - stores the nodes (if any) were input in the command line
 */
struct arginfo {
  char *genders_filename; 
  char *gmond_hostname;
  char *gmond_ip;         
  int gmond_port;         
  enum whatsup_output_type output_type;
  enum whatsup_list_type list_type;    
  int list_altnames;
  hostlist_t nodes;
};

/********************************
 * Function Declarations        *
 ********************************/

static void usage(void);

static void version(void);

static void err_usage(char *);

static void output_error(char *, char *);

static void cleanup_struct_arginfo(struct arginfo *);

static int cmdline_parse(struct arginfo *, int, char **);

static char * get_hostlist_string(hostlist_t, int);

static int check_if_nodes_are_up_or_down(struct arginfo *, 
                                         enum whatsup_output_type,
                                         nodeupdown_t, 
                                         char **); 

static int get_all_up_or_down_nodes(struct arginfo *, 
                                    enum whatsup_output_type,
                                    nodeupdown_t, 
                                    char **);

static int convert_to_altnames(struct arginfo *, char **);

static int get_up_or_down_nodes(struct arginfo *, 
                                enum whatsup_output_type, 
                                nodeupdown_t, 
                                char **);

static int output_nodes(struct arginfo *, char *nodes);

/* usage
 * - output usage and exit
 */
static void usage(void) {
  fprintf(stderr,
          "Usage: whatsup [OPTIONS]... [NODES]...\n"
          "  -h         --help              Print help and exit\n"
          "  -V         --version           Print version and exit\n"
          "  -f STRING  --filename=STRING   Location of genders file (default=%s)\n"
          "  -o STRING  --hostname=STRING   gmond server hostname (default=localhost)\n"
          "  -i STRING  --ip=STRING         gmond server IP address (default=127.0.0.1)\n"
          "  -p INT     --port=INT          gmond server port (default=%d)\n"
          "  -b         --updown            List both up and down nodes (default)\n"
          "  -u         --up                List only up nodes\n"
          "  -d         --down              List only down nodes\n"
          "  -l         --hostlist          List nodes in hostlist format (default)\n"
          "  -c         --comma             List nodes in comma separated list\n"
          "  -n         --newline           List nodes in newline separated list\n"
          "  -s         --space             List nodes in space separated list\n"
          "  -a         --altnames          List nodes by alternate name (default=off)\n"
          "\n",
          GENDERS_DEFAULT_FILE, GANGLIA_DEFAULT_XML_PORT);
  exit(1);
}

/* version
 * - output version and exit
 */
static void version(void) {
  fprintf(stderr, "whatsup 1.0-5\n");
  exit(1);
}

/* err_usage
 * - print error statement and output usage and exit
 */
static void err_usage(char *msg) {
  if (msg)
    fprintf(stderr, "command line error: %s\n\n", msg);

  usage();
}

/* output_error
 * - output error statement
 */
static void output_error(char *msg, char *errno_msg) {
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
  arginfo->output_type = WHATSUP_UP_AND_DOWN;
  arginfo->list_type = WHATSUP_HOSTLIST;
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
  int hopt = 0, Vopt = 0, fopt = 0, oopt = 0, iopt = 0, popt = 0, bopt = 0;
  int uopt = 0, dopt = 0, lopt = 0, copt = 0, nopt = 0, sopt = 0, aopt = 0;
  int c, index, i, ret;

  char *filename;
  char *hostname;
  char *ip;
  int port;

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
      hopt++;
      break;
    case 'V':
      Vopt++;
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
      popt++;
      port = atoi(optarg);
      break;
    case 'b':
      bopt++;
      break;
    case 'u':
      uopt++;
      break;
    case 'd':
      dopt++;
      break;
    case 'l':
      lopt++;
      break;
    case 'c':
      copt++;
      break;
    case 'n':
      nopt++;
      break;
    case 's':
      sopt++;
      break;
    case 'a':
      aopt++;
      break;
    case '?':
      err_usage("invalid command line option entered");
      break;
    default:
      output_error("getopt() error", NULL);
      return -1;
      break;
    }
  }

  if (hopt > 0)
    usage();

  if (Vopt > 0)
    version();

  if (fopt > 1)
    err_usage("you can only specify the --filename ('-f') option once");
  else if (fopt == 1) { 
    if ((arginfo->genders_filename = strdup(filename)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
  }

  if ((oopt + iopt) > 1) {
    err_usage("you can only specify one of the"
              " --gmond_hostname ('o') or --gmond_ip ('i') options");
  }
  else if (oopt == 1) {
    if ((arginfo->gmond_hostname = strdup(hostname)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
  }
  else if (iopt == 1) {
    if ((arginfo->gmond_ip = strdup(ip)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
  }

  if (popt > 1)
    err_usage("you can only specify --gmond_port ('-p') once");
  else  if (popt == 1)
    arginfo->gmond_port = port;

  if (bopt == 1  && (uopt == 1 || dopt == 1))
    err_usage("you cannot specify --upanddown ('b') and --up ('u') or --down ('d')");
  else if (bopt == 1 || (uopt == 1 && dopt == 1))
    arginfo->output_type = WHATSUP_UP_AND_DOWN;
  else if (uopt == 1)
    arginfo->output_type = WHATSUP_UP;
  else if (dopt == 1)
    arginfo->output_type = WHATSUP_DOWN;

  if ((lopt + copt + nopt + sopt) > 1)
    err_usage("you can only specify one of the --hostlist ('-l'), --comma ('-c'), "
              "--newline ('-n'), or --space ('-s') options once");
  else if (lopt == 1)
    arginfo->list_type = WHATSUP_HOSTLIST;
  else if (copt == 1)
    arginfo->list_type = WHATSUP_COMMA;
  else if (nopt == 1)
    arginfo->list_type = WHATSUP_NEWLINE;
  else if (sopt == 1)
    arginfo->list_type = WHATSUP_SPACE;

  if (aopt > 1)
    err_usage("you can only specify --altnames ('-a') once");
  else if (aopt == 1)
    arginfo->list_altnames = WHATSUP_ON;

  if ((arginfo->nodes = hostlist_create(NULL)) == NULL) {
    output_error("hostlist_create() error", NULL);
    return -1;
  }

  /* store any nodes listed on the command line */
  index = optind;
  while (index < argc) {
    char *bracket1 = NULL;
    char *bracket2 = NULL;

    /* search for periods.  If there are periods, these are non-short hostname
     * machine names. Output error 
     */
    if (strchr(argv[index], '.') != NULL) {
      output_error("nodes must be listed in short hostname format", NULL);
      return -1;
    }

    /* search for brackets.  Assume nodes are listed in hostlist format */
    bracket1 = strchr(argv[index], '[');
    bracket2 = strchr(argv[index], ']');
    
    if (bracket1 != NULL && bracket2 != NULL) {
      /* best situation, found both ends of a hostlist list of nodes */
      
      if ((ret = hostlist_push(arginfo->nodes, argv[index])) == 0) {
        output_error("hostlist_push() error", 
                     "hosts may have been input incorrectly");
        return -1;
      }
    }
    else if (bracket1 != NULL && bracket2 == NULL) {
      /* hardest situation, found beginning bracket, but not end bracket */
      /* end of hostlist is in a later token */

      int index_begin = index;
      int strlen_count = strlen(argv[index]);
      char *buffer;
      index++;

      /* find ending token with end bracket */
      while (index < optind) {
        if ((bracket2 = strchr(argv[index], ']')) != NULL)
          break;

        strlen_count += strlen(argv[index]);
        index++;
      }
      
      /* ending token not found */
      if (bracket2 == NULL) {
        output_error("'[' found, but not ']'", 
                     "hosts may have been input incorrectly");
        return -1;
      }

      /* create a single string out of all the tokens */
      buffer = (char *)malloc(strlen_count + 1);
      if (buffer == NULL) {
        output_error("out of memory", NULL);
        return -1;
      }
      memset(buffer, '\0', strlen_count + 1);
      
      strcpy(buffer, argv[index_begin]);
      for (i = (index_begin+1); i <= index; i++)
        strcat(buffer, argv[i]);

      if (hostlist_push(arginfo->nodes, buffer) == 0) {
        free(buffer);
        output_error("hostlist_push() error", 
                     "hosts may have been input incorrectly");
        return -1;
      }

      free(buffer);
    }
    else if (bracket1 == NULL && bracket2 != NULL) {
      /* error situation, found end bracket before begin bracket */

      output_error("']' found before '['", 
                   "hosts may have been input incorrectly");
      return -1;
    }
    else {
      /* found no brackets, node input is comma or space separated */

      char *temp_char;

      if (strchr(argv[index],',') != NULL) {
        temp_char = strtok(argv[index],",");
        while (temp_char != NULL) {
          if (hostlist_push_host(arginfo->nodes, temp_char) == 0) {
            output_error("hostlist_push_host() error", 
                         "hosts may have been input incorrectly");
            return -1;
          }
          temp_char = strtok(NULL,",");
        }
      }
      else {
        if (hostlist_push_host(arginfo->nodes, argv[index]) == 0) {
          output_error("hostlist_push_host() error", 
                       "hosts may have been input incorrectly");
          return -1;
        }
      }
    }

    index++;
  }

  /* remove any duplicate nodes listed */
  hostlist_uniq(arginfo->nodes);

  return 0;
}

/* get_hostlist_string
 * - get a hostlist ranged string 
 */
char * get_hostlist_string(hostlist_t hl, int which) {
  char *str = NULL;
  int ret, str_len = 0;

  do {
    free(str);
    str_len += WHATSUP_BUFFERLEN;
    if ((str = (char *)malloc(str_len)) == NULL) {
      output_error("out of memory", NULL);
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

/* check_if_nodes_are_up_or_down
 * - determine if specific nodes passed in at the command line are up or down 
 */
int check_if_nodes_are_up_or_down(struct arginfo *arginfo, 
                                  enum whatsup_output_type output_type,
                                  nodeupdown_t handle, 
                                  char **nodes) {
  hostlist_t hl = NULL;
  hostlist_iterator_t iter = NULL;
  char *str = NULL;
  int ret;

  if ((hl = hostlist_create(NULL)) == NULL) {
    output_error("hostlist_create() error", NULL);
    return -1;
  }

  if ((iter = hostlist_iterator_create(arginfo->nodes)) == NULL) {
    output_error("hostlist_iterator_create() error", NULL);
    return -1;
  }

  while ((str = hostlist_next(iter)) != NULL) {
    if (output_type == WHATSUP_UP) {
      if ((ret = nodeupdown_is_node_up(handle, str)) == -1) {
        output_error("nodeupdown_is_node_up() error",
                     nodeupdown_strerror(nodeupdown_errnum(handle))); 
        goto cleanup;
      }
    }
    else {
      if ((ret = nodeupdown_is_node_down(handle, str)) == -1) {
        output_error("nodeupdown_is_node_down() error",
                     nodeupdown_strerror(nodeupdown_errnum(handle))); 
        goto cleanup;
      }
    }

    if (ret == 1) {
      if (hostlist_push_host(hl, str) == 0) {
        output_error("hostlist_push_host() error", NULL);
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

/* get_all_up_or_down_nodes
 * - get all up or down nodes
 */
int get_all_up_or_down_nodes(struct arginfo *arginfo, 
                             enum whatsup_output_type output_type,
                             nodeupdown_t handle, 
                             char **nodes) {
  int str_len = 0;
  char *str = NULL;

  do {
    free(str);
    str_len += WHATSUP_BUFFERLEN;
    if ((str = (char *)malloc(str_len)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
    memset(str, '\0', str_len);
    
    if (output_type == WHATSUP_UP) {
      if (nodeupdown_get_up_nodes_string(handle, str, str_len) == -1) {
        if (nodeupdown_errnum(handle) != NODEUPDOWN_ERR_OVERFLOW) {
          output_error("nodeupdown_get_up_nodes_string() error",
                       nodeupdown_strerror(nodeupdown_errnum(handle)));
          free(str);
          return -1;
        }
      }
    }
    else {
      if (nodeupdown_get_down_nodes_string(handle, str, str_len) == -1) {
        if (nodeupdown_errnum(handle) != NODEUPDOWN_ERR_OVERFLOW) {
          output_error("nodeupdown_get_up_nodes_string() error",
                       nodeupdown_strerror(nodeupdown_errnum(handle)));
          free(str);
          return -1;
        }
      }
    }
  } while (nodeupdown_errnum(handle) == NODEUPDOWN_ERR_OVERFLOW); 
  
  *nodes = str;
  return 0;
}

int convert_to_altnames(struct arginfo *arginfo, char **nodes) {
  genders_t handle = NULL;
  char *buffer = NULL;
  int ret, buflen = 0;

  if ((handle = genders_handle_create()) == NULL) {
    output_error("genders_handle_create() error", NULL);
    goto cleanup;
  }

  if (genders_load_data(handle, arginfo->genders_filename) == -1) {
    output_error("genders_load_data() error",
                 genders_errormsg(handle));
    goto cleanup;
  }

  do {
    free(buffer);
    buflen += WHATSUP_BUFFERLEN;
    if ((buffer = (char *)malloc(buflen)) == NULL) {
      output_error("out of memory", NULL);
      goto cleanup;
    }
    memset(buffer, '\0', buflen);

    ret = genders_string_to_altnames_preserve(handle, 
                                              *nodes,
                                              buffer,
                                              buflen);
  } while (ret == -1 && genders_errnum(handle) == GENDERS_ERR_OVERFLOW);

  if (ret == -1) {
    output_error("genders_string_to_altnames_preserve() error",
                 genders_errormsg(handle));
    goto cleanup;
  }
  
  if (genders_handle_destroy(handle) == -1) {
    output_error("genders_handle_destroy() error",
                 genders_errormsg(handle));
    goto cleanup;
  }

  free(*nodes);

  *nodes = buffer;

  return 0;

 cleanup:

  (void)genders_handle_destroy(handle);

  free(buffer);

  return -1;
}

/* get_up_or_down_nodes
 * - a wrapper function used to avoid duplicate code.
 */
int get_up_or_down_nodes(struct arginfo *arginfo, 
                         enum whatsup_output_type output_type, 
                         nodeupdown_t handle, 
                         char **nodes) {
  char *str = NULL;
  
  if (hostlist_count(arginfo->nodes) > 0) {
    if (check_if_nodes_are_up_or_down(arginfo, 
                                      output_type, 
                                      handle, 
                                      &str) != 0)
      goto cleanup;
  }
  else {
    if (get_all_up_or_down_nodes(arginfo, 
                                 output_type, 
                                 handle, 
                                 &str) != 0)
      goto cleanup;
  }

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
  char break_type;
  hostlist_t hl = NULL;

  if (arginfo->list_type == WHATSUP_HOSTLIST)
    fprintf(stdout, "%s\n", nodes);
  else {
    /* output nodes separated by some break type */
    
    if (arginfo->list_type == WHATSUP_COMMA)
      break_type = ',';
    else if (arginfo->list_type == WHATSUP_NEWLINE)
      break_type = '\n';
    else
      break_type = ' ';

    if ((hl = hostlist_create(nodes)) == NULL) {
      output_error("hostlist_create() error", NULL);
      goto cleanup;
    }

    if ((str = get_hostlist_string(hl, DERANGED_STRING)) == NULL)
      goto cleanup;

    /* convert commas to appropriate break types */
    if (break_type != ',') {
      while ((ptr = strchr(str, ',')) != NULL)
        *ptr = break_type;
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
    output_error("out of memory", NULL);
    goto cleanup;
  }

  if (initialize_struct_arginfo(arginfo) != 0) {
    output_error("initialize_struct_arginfo() error", NULL);
    goto cleanup;
  }
  
  if (cmdline_parse(arginfo, argc, argv) != 0)
    goto cleanup;
  
  if ((handle = nodeupdown_handle_create()) == NULL) {
    output_error("nodeupdown_handle_create() error", NULL);
    goto cleanup;
  }

  if (nodeupdown_load_data(handle, 
                           arginfo->genders_filename, 
                           arginfo->gmond_hostname, 
                           arginfo->gmond_ip, 
                           arginfo->gmond_port,
                           0) == -1) {
    output_error("nodeupdown_load_data() error", 
                 nodeupdown_strerror(nodeupdown_errnum(handle))); 
    goto cleanup;
  }

  /* get up nodes */
  if (arginfo->output_type == WHATSUP_UP || 
      arginfo->output_type == WHATSUP_UP_AND_DOWN) {
    if (get_up_or_down_nodes(arginfo, 
                             WHATSUP_UP, 
                             handle, 
                             &up_nodes) == -1)
      goto cleanup;
  }

  /* get down nodes */
  if (arginfo->output_type == WHATSUP_DOWN || 
      arginfo->output_type == WHATSUP_UP_AND_DOWN) {
    if (get_up_or_down_nodes(arginfo, 
                             WHATSUP_DOWN, 
                             handle, 
                             &down_nodes) == -1)
      goto cleanup;
  }

  /* output up, down, or both up and down nodes */
  if (arginfo->output_type == WHATSUP_UP_AND_DOWN) {
    fprintf(stdout, "up:\t");

    /* handle odd situation with output formatting */
    if (arginfo->list_type == WHATSUP_NEWLINE)
      fprintf(stdout, "\n");

    if (output_nodes(arginfo, up_nodes) != 0)
      goto cleanup;

    /* handle odd situation with output formatting */
    if (arginfo->list_type == WHATSUP_NEWLINE)
      fprintf(stdout, "\n");
 
    fprintf(stdout, "down:\t");

    /* handle odd situation with output formatting */
    if (arginfo->list_type == WHATSUP_NEWLINE)
      fprintf(stdout, "\n");

    if (output_nodes(arginfo, down_nodes) != 0)
      goto cleanup;
  }
  else if (arginfo->output_type == WHATSUP_UP) {
    if (output_nodes(arginfo, up_nodes) != 0)
      goto cleanup;
  }
  else if (arginfo->output_type == WHATSUP_DOWN) {
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
