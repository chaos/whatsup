/*
 * $Id: whatsup.c,v 1.1 2003-02-24 19:30:42 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/whatsup/whatsup.c,v $
 *    
 */

#include <errno.h>
#include <ganglia.h>
#include <genders.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef WHATSUP_DEBUG
#include <mcheck.h>
#endif

#include "nodeupdown.h"

/********************************
 * External Variables           *
 ********************************/

/* getopt */
extern char *optarg;
extern int optind, opterr, optopt;

/* gethostbyname */
extern int h_errno;

/********************************
 * Definitions                  *
 ********************************/

#define WHATSUP_OFF          0
#define WHATSUP_ON           1

#define WHATSUP_BUFFERLEN    65536

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

#ifdef WHATSUP_DEBUG

#define DEBUG_BUFFERLEN      65536
int debug_int;
int debug_buffer_len;
char *debug_buffer_ptr;

#endif

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
static void err_usage(char *);
static void output_error(char *, char *);
static void cleanup_struct_arginfo(struct arginfo *);
static int cmdline_parse(struct arginfo *, int, char **);
static int check_if_nodes_are_up_or_down(struct arginfo *, 
					 enum whatsup_output_type,
					 nodeupdown_t, 
					 hostlist_t); 
static int get_all_up_or_down_nodes(struct arginfo *, 
				    enum whatsup_output_type,
				    nodeupdown_t, 
				    hostlist_t);
static int output_nodes(struct arginfo *, hostlist_t);
int handle_up_or_down_nodes(struct arginfo *, 
			    enum whatsup_output_type, 
			    nodeupdown_t, 
			    hostlist_t *);


/* usage
 * - output usage and exit
 */
static void usage(void) {
  fprintf(stderr, "Usage: whatsup [OPTIONS] [nodes ...]\n");
  fprintf(stderr,"  -h        --help             Print help and exit\n");
  fprintf(stderr,"  -fSTRING  --filename=STRING  Location of genders file (default=%s)\n", DEFAULT_GENDERS_FILE);
  fprintf(stderr,"  -oSTRING  --hostname=STRING  gmond server hostname (default=localhost)\n");
  fprintf(stderr,"  -iSTRING  --ip=STRING        gmond server IP address (default=127.0.0.1)\n");
  fprintf(stderr,"  -pINT     --port=INT         gmond server port (default=%d)\n",GANGLIA_DEFAULT_XML_PORT);
  fprintf(stderr,"  -b        --updown           List both up and down nodes (default)\n");
  fprintf(stderr,"  -u        --up               List only up nodes\n");
  fprintf(stderr,"  -d        --down             List only down nodes\n");
  fprintf(stderr,"  -l        --hostlist         List nodes in hostlist format (default)\n");
  fprintf(stderr,"  -c        --comma            List nodes in comma separated list\n");
  fprintf(stderr,"  -n        --newline          List nodes in newline separated list\n");
  fprintf(stderr,"  -s        --space            List nodes in space separated list\n");
  fprintf(stderr,"  -a        --altnames         List nodes by alternate names (default=off)\n");
  fprintf(stderr,"  nodes ...                    List of nodes to test (default=all nodes)\n");
  fprintf(stderr,"\n");
  exit(1);
}

/* err_usage
 * - print error statement and output usage and exit
 */
static void err_usage(char *msg) {
  if (msg) {
    fprintf(stderr, "command line error: %s\n\n", msg);
  }
  usage();
}

/* output_error
 * - output error statement
 */
static void output_error(char *msg, char *errno_msg) {
  if (msg != NULL && errno_msg != NULL) {
    fprintf(stderr, "whatsup error: %s, %s\n", msg, errno_msg);
  }

  if (msg != NULL && errno_msg == NULL) {
    fprintf(stderr, "whatsup error: %s\n", msg);
  }

}

/* initialize_struct_arginfo
 * - initialize struct arginfo structure
 */
static int initialize_struct_arginfo(struct arginfo *arginfo) {
  arginfo->genders_filename = NULL;
  arginfo->gmond_hostname = NULL;
  arginfo->gmond_ip = NULL;
  arginfo->gmond_port = GANGLIA_DEFAULT_XML_PORT;
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
  if (arginfo->genders_filename != NULL) {
    free(arginfo->genders_filename);
  }
  if (arginfo->gmond_hostname != NULL) {
    free(arginfo->gmond_hostname);
  }
  if (arginfo->gmond_ip != NULL) {
    free(arginfo->gmond_ip);
  }
  if (arginfo->nodes != NULL) {
    hostlist_destroy(arginfo->nodes);
  }
  free(arginfo);
}

/* cmdline_parse
 * - parse commandline arguments
 * - store info in a struct arginfo strcuture
 */
static int cmdline_parse(struct arginfo *arginfo, int argc, char **argv) {
  int hopt = 0, fopt = 0, oopt = 0, iopt = 0, popt = 0, bopt = 0;
  int uopt = 0, dopt = 0, lopt = 0, copt = 0, nopt = 0, sopt = 0, aopt = 0;
  int c, index, i, ret;

  char *filename = DEFAULT_GENDERS_FILE;
  char *hostname = "localhost";
  char *ip = "127.0.0.1";
  int port = GANGLIA_DEFAULT_XML_PORT;

  char *options = "hf:o:i:p:budlcnsa";
  struct option long_options[] = {
    {"help",     0, NULL, 'h'},
    {"filename", 1, NULL, 'f'},
    {"hostname", 1, NULL, 'o'},
    {"ip",       1, NULL, 'i'},
    {"port",     1, NULL, 'p'},
    {"updown",   0, NULL, 'b'},
    {"up",       0, NULL, 'u'},
    {"down",     0, NULL, 'd'},
    {"altnames", 0, NULL, 'a'},
    {"hostlist", 0, NULL, 'l'},
    {"comma",    0, NULL, 'c'},
    {"newline",  0, NULL, 'n'},
    {"space",    0, NULL, 's'},
    {0, 0, 0, 0}
  };

  /* turn off output messages printed by getopt_long */
  opterr = 0;

  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1) {
    switch(c) {
    case 'h':
      hopt++;
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
      output_error("getopt error", NULL);
      return -1;
      break;
    }
  }

  if (hopt > 0) {
    usage();
  }

  if (fopt > 1) {
    err_usage("you can only specify the --filename ('-f') option once");
  }

  if ((arginfo->genders_filename = strdup(filename)) == NULL) {
    output_error("out of memory", NULL);
    return -1;
  }

  if ((oopt + iopt) > 1) {
    err_usage("you can only specify one of the --gmond_hostname ('o') or --gmond_ip ('i') options");
  }
  else if (oopt == 1) {
    /* gmond hostname was provided, must also determine IP address */

    struct hostent *hptr;

    if ((arginfo->gmond_hostname = strdup(hostname)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
    
    if ((hptr = gethostbyname(hostname)) == NULL) {
      output_error("gethostbyname error", (char *)hstrerror(h_errno));
      return -1;
    }

    if ((arginfo->gmond_ip = (char *)malloc(INET_ADDRSTRLEN)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
    memset(arginfo->gmond_ip, '\0', INET_ADDRSTRLEN);

    if (inet_ntop(AF_INET, 
		  (void *)hptr->h_addr, 
		  arginfo->gmond_ip, 
		  INET_ADDRSTRLEN) == NULL) {
      output_error("inet_ntop error", strerror(errno));
      return -1;
    }
  }
  else if (iopt == 1) {
    /* gmond ip address was provided, must also determine hostname */

    struct hostent *hptr;
    struct in_addr temp_in_addr;
    int ret;

    if ((arginfo->gmond_ip = strdup(ip)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }

    ret = inet_pton(AF_INET, arginfo->gmond_ip, &temp_in_addr);
    if (ret == 0) {
      output_error("inet_pton error", "invalid address");
      return -1;
    }
    else if (ret < 0) {
      output_error("inet_pton error", strerror(errno));
      return -1;
    }

    if ((hptr = gethostbyaddr(&temp_in_addr,
			      sizeof(struct in_addr), 
			      AF_INET)) == NULL) {
      output_error("gethostbyaddr error", (char *)hstrerror(h_errno));
      return -1;
    }

    if ((arginfo->gmond_hostname = strdup(hptr->h_name)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
  }
  else {
    /* use default hostname and ip address */

    if ((arginfo->gmond_hostname = strdup(hostname)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }

    if ((arginfo->gmond_ip = strdup(ip)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
  }

  if (popt > 1) {
    err_usage("you can only specify --gmond_port ('-p') once");
  }
  else  if (popt == 1) {
    arginfo->gmond_port = port;
  }

  if (bopt == 1  && (uopt == 1 || dopt == 1)) {
    err_usage("you cannot specify --upanddown ('b') and --up ('u') or --down ('d')");
  }
  else if (bopt == 1 || (uopt == 1 && dopt == 1)) {
    arginfo->output_type = WHATSUP_UP_AND_DOWN;
  }
  else if (uopt == 1) {
    arginfo->output_type = WHATSUP_UP;
  }
  else if (dopt == 1) {
    arginfo->output_type = WHATSUP_DOWN;
  }

  if ((lopt + copt + nopt + sopt) > 1) {
    err_usage("you can only specify one of the --hostlist ('-l'), --comma ('-c'), --newline ('-n'), or --space ('-s') options once");
  }
  else if (lopt == 1) {
    arginfo->list_type = WHATSUP_HOSTLIST;
  }
  else if (copt == 1) {
    arginfo->list_type = WHATSUP_COMMA;
  }
  else if (nopt == 1) {
    arginfo->list_type = WHATSUP_NEWLINE;
  }
  else if (sopt == 1) {
    arginfo->list_type = WHATSUP_SPACE;
  }

  if (aopt > 1) {
    err_usage("you can only specify --altnames ('-a') once");
  }
  else if (aopt == 1) {
    arginfo->list_altnames = WHATSUP_ON;
  }

  if ((arginfo->nodes = hostlist_create(NULL)) == NULL) {
      output_error("hostlist_create error", NULL);
      return -1;
  }

  /* store any nodes listed on the command line */
  index = optind;
  while (index < argc) {
    char *bracket1 = NULL;
    char *bracket2 = NULL;

    /* search for brackets.  Assume nodes are listed in hostlist format */

    bracket1 = strchr(argv[index], '[');
    bracket2 = strchr(argv[index], ']');
    
    if (bracket1 != NULL && bracket2 != NULL) {
      /* best situation, found both ends of a hostlist list of nodes */
      
      if ((ret = hostlist_push(arginfo->nodes, argv[index])) == 0) {
	output_error("hostlist_push error", 
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
	if ((bracket2 = strchr(argv[index], ']')) != NULL) {
	  break;
	}
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
      for (i = (index_begin+1); i <= index; i++) {
	strcat(buffer, argv[i]);
      }

      if (hostlist_push(arginfo->nodes, buffer) == 0) {
	free(buffer);
	output_error("hostlist_push error", 
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
      /* found no brackets, node input is comman or space separated */

      char *temp_char;

      if (strchr(argv[index],',') != NULL) {
	while ((temp_char = strchr(argv[index],',')) != NULL) {
	  *temp_char = ' ';
	}

	temp_char = strtok(argv[index]," ");
	while (temp_char != NULL) {
	  if (hostlist_push_host(arginfo->nodes, temp_char) == 0) {
	    output_error("hostlist_push_host error", 
			 "hosts may have been input incorrectly");
	    return -1;
	  }
	  temp_char = strtok(NULL," ");
	}
      }
      else {
	if (hostlist_push_host(arginfo->nodes, argv[index]) == 0) {
	  output_error("hostlist_push_host error", 
		       "hosts may have been input incorrectly");
	  return -1;
	}
      }
    }

    index++;
  }

  /* remove any duplicate nodes listed */
  hostlist_uniq(arginfo->nodes);

#ifdef WHATSUP_DEBUG
  fprintf(stderr,"arginfo\n");
  fprintf(stderr,"-------\n");
  fprintf(stderr,"genders_filename: %s\n", arginfo->genders_filename);
  fprintf(stderr,"gmond_hostname: %s\n", arginfo->gmond_hostname);
  fprintf(stderr,"gmond_ip: %s\n", arginfo->gmond_ip);
  fprintf(stderr,"gmond_port: %d\n", arginfo->gmond_port);
  fprintf(stderr,"output_type: %d\n", arginfo->output_type);
  fprintf(stderr,"list_type: %d\n", arginfo->list_type); 
  fprintf(stderr,"list_altnames: %d\n", arginfo->list_altnames);
  fprintf(stderr,"number of nodes: %d\n", hostlist_count(arginfo->nodes));

  debug_buffer_len = DEBUG_BUFFERLEN;
  if ((debug_buffer_ptr = (char *)malloc(debug_buffer_len)) == NULL) {
    output_error("out of memory", NULL);
    return -1;
  }
  memset(debug_buffer_ptr, '\0', debug_buffer_len);

  /* loop until the buffer is large enough */
  while (hostlist_ranged_string(arginfo->nodes, 
				debug_buffer_len, 
				debug_buffer_ptr) == -1) {
    free(debug_buffer_ptr);
    debug_buffer_len += DEBUG_BUFFERLEN;
    if ((debug_buffer_ptr = (char *)malloc(debug_buffer_len)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
    memset(debug_buffer_ptr, '\0', debug_buffer_len);
  }
  
  fprintf(stderr,"nodes inputted: %s\n", debug_buffer_ptr);
  free(debug_buffer_ptr);
  
  fprintf(stderr,"\n");
#endif

  return 0;
}

/* check_if_nodes_are_up_or_down
 * - determine if specific nodes passed in at the command line are up or down 
 */
int check_if_nodes_are_up_or_down(struct arginfo *arginfo, 
				  enum whatsup_output_type output_type,
				  nodeupdown_t handle, 
				  hostlist_t nodes) {
  hostlist_iterator_t iter;
  char *str;
  int ret;

  if ((iter = hostlist_iterator_create(arginfo->nodes)) == NULL) {
    output_error("hostlist_iterator_create", NULL);
    return -1;
  }

  while ((str = hostlist_next(iter)) != NULL) {
    if (output_type == WHATSUP_UP) {
      ret = nodeupdown_is_node_up(handle, str);
    }
    else {
      ret = nodeupdown_is_node_down(handle, str);
    }

    if (ret == 1) {
      if (hostlist_push_host(nodes, str) == 0) {
	hostlist_iterator_destroy(iter);
	output_error("hostlist_push_host", NULL);
	return -1;
      }
    }
    else if (ret == -1) {
      hostlist_iterator_destroy(iter);
      output_error("nodeupdown_is_node_up/down",
		   nodeupdown_strerror(nodeupdown_errnum(handle))); 
      return -1;
    }
			
  }
  hostlist_iterator_destroy(iter);

  return 0;
}

/* get_all_up_or_down_nodes
 * - get all up or down nodes
 */
int get_all_up_or_down_nodes(struct arginfo *arginfo, 
			     enum whatsup_output_type output_type,
			     nodeupdown_t handle, 
			     hostlist_t nodes) {

  if (output_type == WHATSUP_UP) {
    if (nodeupdown_get_up_nodes_hostlist(handle, nodes) == -1) {
      output_error("nodeupdown_get_up_nodes_hostlist",
		   nodeupdown_strerror(nodeupdown_errnum(handle)));
      return -1;
    }
  }
  else {
    if (nodeupdown_get_down_nodes_hostlist(handle, nodes) == -1) {
      output_error("nodeupdown_get_up_nodes_hostlist",
		   nodeupdown_strerror(nodeupdown_errnum(handle)));
      return -1;
    }
  }

  return 0;
}

/* output_nodes
 * - output the nodes indicated in nodes
 */
int output_nodes(struct arginfo *arginfo, hostlist_t nodes) {

  char *str = NULL;
  int i, str_len, num_nodes;
  char break_type;
  hostlist_iterator_t iter = NULL;

  /* sort nodes ahead of time */
  hostlist_sort(nodes);

  if (arginfo->list_type == WHATSUP_HOSTLIST) {
    /* output nodes in hostlist format */

    str_len = NODEUPDOWN_BUFFERLEN;
    if ((str = (char *)malloc(str_len)) == NULL) {
      output_error("out of memory", NULL);
      return -1;
    }
    memset(str, '\0', str_len);

    while (hostlist_ranged_string(nodes, str_len, str) == -1) {
      free(str);
      str_len += NODEUPDOWN_BUFFERLEN;
      if ((str = (char *)malloc(str_len)) == NULL) {
	output_error("out of memory", NULL);
	return -1;
      }
      memset(str, '\0', str_len);
    }
    fprintf(stdout, "%s\n", str);
    free(str);
    
  }
  else {
    /* output nodes separated by some break type */
    
    switch(arginfo->list_type) {
    case WHATSUP_COMMA:
      break_type = ',';
      break;
    case WHATSUP_NEWLINE:
      break_type = '\n';
      break;
    case WHATSUP_SPACE:
      break_type = ' ';
      break;
    default:
      output_error("output_nodes", "invalid list type");
      return -1;
      break;
    }

    /* handle odd situation to ensure output doesn't look odd */
    if (break_type == '\n' && arginfo->output_type == WHATSUP_UP_AND_DOWN) {
      fprintf(stdout, "\n");
    }

    if ((num_nodes = hostlist_count(nodes)) < 0) {
      output_error("hostlist_count", NULL);
      return -1;
    }

    if (num_nodes > 0) {
      if ((iter = hostlist_iterator_create(nodes)) == NULL) {
	output_error("hostlist_iterator_create", NULL);
	return -1;
      }
     
      for (i = 0; i < (num_nodes - 1); i++) {
	str = hostlist_next(iter);
	fprintf(stdout, "%s%c",str, break_type);
	free(str);
      }
      str = hostlist_next(iter);
      fprintf(stdout, "%s",str);
      free(str);
    }
    fprintf(stdout,"\n");

    hostlist_iterator_destroy(iter);
  }

  return 0;
}

int handle_up_or_down_nodes(struct arginfo *arginfo, 
			    enum whatsup_output_type output_type, 
			    nodeupdown_t handle, 
			    hostlist_t *nodes) {
  hostlist_t alternate_nodes = NULL;

  if (hostlist_count(arginfo->nodes) > 0) {
    if (check_if_nodes_are_up_or_down(arginfo, 
				      output_type, 
				      handle, 
				      *nodes) != 0) {
      goto cleanup;
    }
  }
  else {
    /* get all nodes */
    if (get_all_up_or_down_nodes(arginfo, 
			     output_type, 
			     handle, 
			     *nodes) != 0) {
      goto cleanup;
    }
  }

  if (arginfo->list_altnames == WHATSUP_ON) {
  
    if ((alternate_nodes = hostlist_create(NULL)) == NULL) {
      output_error("hostlist_create", NULL);
      goto cleanup;
    }
    
    if (nodeupdown_get_hostlist_alternate_names(handle, 
						*nodes, 
						alternate_nodes)) {
      output_error("nodeupdown_get_hostlist_alternate_names", 
		   nodeupdown_strerror(nodeupdown_errnum(handle)));
      goto cleanup;
    }
    
    hostlist_destroy(*nodes);
    *nodes = alternate_nodes;
    alternate_nodes = NULL;
  }    

  return 0;

 cleanup:

  if (alternate_nodes != NULL) {
    hostlist_destroy(alternate_nodes);
  }
  return -1;
}

int main(int argc, char **argv) {
  struct arginfo *arginfo = NULL;
  nodeupdown_t handle = NULL;
  hostlist_t up_nodes = NULL;
  hostlist_t down_nodes = NULL;

#ifdef WHATSUP_DEBUG
  mtrace();
#endif

  if ((arginfo = (struct arginfo *)malloc(sizeof(struct arginfo))) == NULL) {
    output_error("out of memory", NULL);
    goto cleanup;
  }

  if (initialize_struct_arginfo(arginfo) != 0) {
    output_error("initialize_struct_arginfo", NULL);
    goto cleanup;
  }

  if (cmdline_parse(arginfo, argc, argv) != 0) {
    goto cleanup;
  }
  
  if ((handle = nodeupdown_create()) == NULL) {
    output_error("nodeupdown_create", NULL);
    goto cleanup;
  }

  if (nodeupdown_load_data(handle, 
			   arginfo->genders_filename, 
			   arginfo->gmond_hostname, 
			   arginfo->gmond_ip, 
			   arginfo->gmond_port) == -1) {
    goto cleanup;
  }

  if (arginfo->output_type == WHATSUP_UP ||
      arginfo->output_type == WHATSUP_UP_AND_DOWN) {
    if ((up_nodes = hostlist_create(NULL)) == NULL) {
      goto cleanup;
    }
    
    if (handle_up_or_down_nodes(arginfo, 
				WHATSUP_UP, 
				handle, 
				&up_nodes) == -1) {
      goto cleanup;
    }
  }
  if (arginfo->output_type == WHATSUP_DOWN ||
      arginfo->output_type == WHATSUP_UP_AND_DOWN) {
    if ((down_nodes = hostlist_create(NULL)) == NULL) {
      goto cleanup;
    }
    
    if (handle_up_or_down_nodes(arginfo, 
				WHATSUP_DOWN, 
				handle, 
				&down_nodes) == -1) {
      goto cleanup;
    }
  }

  if (arginfo->output_type == WHATSUP_UP_AND_DOWN) {
    fprintf(stdout, "up:\t");
    if (output_nodes(arginfo, up_nodes) != 0) {
      goto cleanup;
    }

    /* odd situation with output formatting */
    if (arginfo->list_type == WHATSUP_NEWLINE) {
      fprintf(stdout, "\n");
    }

    fprintf(stdout, "down:\t");
    if (output_nodes(arginfo, down_nodes) != 0) {
      goto cleanup;
    }
  }
  else if (arginfo->output_type == WHATSUP_UP) {
    if (output_nodes(arginfo, up_nodes) != 0) {
      goto cleanup;
    }
  }
  else if (arginfo->output_type == WHATSUP_DOWN) {
    if (output_nodes(arginfo, down_nodes) != 0) {
      goto cleanup;
    }
  }

  cleanup_struct_arginfo(arginfo);
  (void)nodeupdown_destroy(handle);
  if (up_nodes != NULL) {
    hostlist_destroy(up_nodes);
  }
  if (down_nodes != NULL) {
    hostlist_destroy(down_nodes);
  }
  exit(0);

 cleanup:
  if (arginfo != NULL) {
    cleanup_struct_arginfo(arginfo);
  }
  
  if (handle != NULL) {
   (void)nodeupdown_destroy(handle);
  }

  if (up_nodes != NULL) {
    hostlist_destroy(up_nodes);
  }

  if (down_nodes != NULL) {
    hostlist_destroy(down_nodes);
  }

  exit(1);
}
