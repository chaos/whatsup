/*
 * $Id: nodeupdown.c,v 1.41 2003-05-14 19:30:13 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown.c,v $
 *    
 */

#include <genders.h>
#include <ganglia.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <ganglia/xmlparse.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/types.h>

#include "hostlist.h"
#include "nodeupdown.h"

/*********************************
 * Definitions                   *
 *********************************/

#define NODEUPDOWN_ERR_MIN               NODEUPDOWN_ERR_SUCCESS
#define NODEUPDOWN_ERR_MAX               NODEUPDOWN_ERR_ERRNUMRANGE

#define NODEUPDOWN_DATA_LOADED           1
#define NODEUPDOWN_DATA_NOT_LOADED       0

#define NODEUPDOWN_UP_NODES              1
#define NODEUPDOWN_DOWN_NODES            0

#define NODEUPDOWN_MAGIC_NUM             0xfeedbeef

#define NODEUPDOWN_BUFFERLEN             65536

#ifndef GENDERS_ALTNAME_ATTRIBUTE
#define GENDERS_ALTNAME_ATTRIBUTE        "altname"
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#ifndef CONNECT_TIMEOUT_LEN
#define CONNECT_TIMEOUT_LEN              5 
#endif

/* struct nodeupdown
 * - handle for all nodeupdown API functions
 * magic - magic number
 * errnum - error number
 * loaded_flag - indicates if data has been loaded
 * up_nodes - up nodes
 * down_nodes - down nodes
 * genders_handle - handle for genders API
 * max_nodes - maximum nodes in the genders_file
 */
struct nodeupdown {
  int magic;
  int errnum;
  int loaded_flag;
  hostlist_t up_nodes;
  hostlist_t down_nodes;
  genders_t genders_handle;
  int max_nodes;
};

/* parse_vars
 * - variables used when parsing ganglia XML data
 * - used so multiple variables can be passed as one variable
 *   to the xml parsing function
 */
struct parse_vars {
  nodeupdown_t handle;
  int timeout_len;
  unsigned long localtime;
};

/* error messages */
static char * errmsg[] = {
  "success",
  "nodeupdown handle is null",
  "open file error",
  "read file error",
  "connection to server error",
  "connection to server timeout",
  "improper address error",
  "network error",
  "data already loaded",
  "data not loaded",
  "array or string not large enough to store result",
  "incorrect parameters passed in",
  "null pointer reached in list",
  "out of memory",
  "gmond lists a node that genders has no knowledge of",
  "node not known by genders",
  "internal genders error",
  "internal ganglia error",
  "internal hostlist error",
  "nodeupdown handle magic number incorrect, improper handle passed in",
  "internal system error",
  "error number out of range",
};

/**********************************
 * Function Definitions           *
 **********************************/

/* get hostlist ranged string from specified hostlist 
 * Returns -1 on error, 0 on success
 */
static char * get_hostlist_ranged_string(nodeupdown_t handle, 
                                         hostlist_t hostlist);

/* common error checks for genders handle 
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_handle_err_check(nodeupdown_t handle);

/* common error checks for non-loaded nodeupdown data 
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_unloaded_handle_err_check(nodeupdown_t handle);

/* common error checks for loaded nodeupdown data 
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_loaded_handle_err_check(nodeupdown_t handle);

/* initialize nodeupdown handle 
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_initialization(nodeupdown_t handle);

/* free memory from nodeupdown handle 
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_cleanup(nodeupdown_t handle);

/* retrieve data from the genders file 
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_retrieve_genders_data(nodeupdown_t handle,
                                            const char *filename);

/* retrieve host data from gmond 
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_retrieve_gmond_data(nodeupdown_t handle,
                                          const char *ip,
                                          int port,
                                          int timeout_len);

/* connect to ganglia server with lower connect timeout 
 * Returns -1 on error, 0 on success
 */
static int low_timeout_connect(nodeupdown_t handle, const char *ip, int port);

/* xml start function for use with ganglia XML library
 * Returns -1 on error, 0 on success
 */
static void xml_parse_start(void *data, const char *e1, const char **attr);

/* xml end function for use with ganglia XML library
 * Returns -1 on error, 0 on success
 */
static void xml_parse_end(void *data, const char *e1);

/* Retrieve the primary node name listed in genders
 * Returns -1 on error, 0 on success
 */
static int get_genders_nodename(nodeupdown_t handle, 
                                const char *node, 
                                char **buffer); 

/* compare genders nodes to gmond nodes
 * - There is a chance gmond does not know of a node listed in genders, 
 *   because gmond never received a multicast message from that noe.
 * - Comparing gmond nodes to genders nodes can identify additional 
 *   down nodes
 * Return -1 on error, 0 on success
 */
static int nodeupdown_compare_genders_to_gmond_nodes(nodeupdown_t handle);

/* output nodes stored in a hostlist, helper to nodeupdown_dump() 
 * Returns -1 on error, 0 on success
 */
static int output_hostlist_nodes(nodeupdown_t handle, 
                                 hostlist_t hostlist, 
                                 FILE *stream, 
                                 char *msg);

/* wrapper function for common code between 
 * get_up_nodes_string() & get_down_node_strings()
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_get_nodes_string(nodeupdown_t handle,
                                       char *buf,
                                       int buflen,
                                       int up_or_down);

/* wrapper function for common code between
 * get_up_nodes_list() & get_down_nodes_list()
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_get_nodes_list(nodeupdown_t handle,
                                     char **list,
                                     int len,
                                     int up_or_down);

/* copy nodes from a hostlist to a list
 * Returns -1 on error, 0 on success
 */
static int nodeupdown_copy_nodes_into_list(nodeupdown_t handle,
                                           hostlist_t src,
                                           char **dest,
                                           int len);

/* wrapper function for common code between
 * is_node_up() and is_node_down()
 * Returns 1 if true, 0 if false, -1 on error
 */
static int nodeupdown_is_node(nodeupdown_t handle, 
                              const char *node, 
                              int up_or_down);

/* check if a node is listed in a hostlist
 * Returns 1 if true, 0 if false, -1 on error
 */
static int nodeupdown_check_if_node_in_hostlist(nodeupdown_t handle,
                                                hostlist_t nodes,
                                                const char *node);

char * get_hostlist_ranged_string(nodeupdown_t handle, hostlist_t hostlist) {
  char *str;
  int str_len;

  str_len = NODEUPDOWN_BUFFERLEN;
  if ((str = (char *)malloc(str_len)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return NULL;
  }
  memset(str, '\0', str_len);

  /* loop until the buffer is large enough */
  while (hostlist_ranged_string(hostlist, str_len, str) == -1) {
    free(str);
    str_len += NODEUPDOWN_BUFFERLEN;
    if ((str = (char *)malloc(str_len)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return NULL;
    }
    memset(str, '\0', str_len);
  }
  
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return str;
}

int nodeupdown_handle_err_check(nodeupdown_t handle) {
  if (handle == NULL)
    return -1;

  if (handle->magic != NODEUPDOWN_MAGIC_NUM)
    return -1;

  return 0;
}

int nodeupdown_unloaded_handle_err_check(nodeupdown_t handle) {
  
  if (nodeupdown_handle_err_check(handle) == -1)
    return -1;

  if (handle->loaded_flag == NODEUPDOWN_DATA_LOADED) {
    handle->errnum = NODEUPDOWN_ERR_ISLOADED;
    return -1;
  }

  return 0;
}

int nodeupdown_loaded_handle_err_check(nodeupdown_t handle) {
  if (nodeupdown_handle_err_check(handle) == -1)
    return -1;

  if (handle->loaded_flag == NODEUPDOWN_DATA_NOT_LOADED) {
    handle->errnum = NODEUPDOWN_ERR_NOTLOADED;
    return -1;
  }

  return 0;
}

nodeupdown_t nodeupdown_handle_create() {
  nodeupdown_t handle;
  if ((handle = (nodeupdown_t)malloc(sizeof(struct nodeupdown))) == NULL)
    return NULL;
  
  nodeupdown_initialization(handle);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle;
}

int nodeupdown_initialization(nodeupdown_t handle) {
  if (handle == NULL)
    return -1;

  handle->magic = NODEUPDOWN_MAGIC_NUM;
  handle->loaded_flag = NODEUPDOWN_DATA_NOT_LOADED;
  handle->up_nodes = NULL;
  handle->down_nodes = NULL;
  handle->genders_handle = NULL;
  handle->max_nodes = -1;

  return 0;
}

int nodeupdown_handle_destroy(nodeupdown_t handle) {

  if (nodeupdown_handle_err_check(handle) == -1)
    return -1;

  nodeupdown_cleanup(handle);

  /* "clean" magic number 
   * ~0xfeedbeef = 0xeatbeef?
   */ 
  handle->magic = ~NODEUPDOWN_MAGIC_NUM;

  free(handle);
  return 0;
}  

int nodeupdown_cleanup(nodeupdown_t handle) {
  if (handle == NULL)
    return -1;

  if (handle->up_nodes != NULL)
    hostlist_destroy(handle->up_nodes);

  if (handle->down_nodes != NULL)
    hostlist_destroy(handle->down_nodes);

  if (handle->genders_handle != NULL)
    (void)genders_handle_destroy(handle->genders_handle);

  nodeupdown_initialization(handle);

  return 0;
}

int nodeupdown_load_data(nodeupdown_t handle, 
                         const char *genders_filename, 
                         const char *gmond_hostname, 
                         const char *gmond_ip, 
                         int gmond_port,
                         int timeout_len) {

  char ip_buffer[INET_ADDRSTRLEN];

  if (nodeupdown_unloaded_handle_err_check(handle) == -1)
    return -1;

  if (gmond_port < -1 || timeout_len < -1) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  /* determine filename */
  if (genders_filename == NULL)
    genders_filename = DEFAULT_GENDERS_FILE;

  /* handle hostname and IP paramters, there are four combinations
   *  of parameters 
   */

  memset(ip_buffer, '\0', INET_ADDRSTRLEN);
  if (gmond_hostname == NULL && gmond_ip == NULL) {
    /* use default */
    strcpy(ip_buffer, "127.0.0.1");
  }
  else if (gmond_hostname != NULL && gmond_ip == NULL) {
    /* if only hostname is given, determine ip address */

    struct hostent *hptr;

    if ((hptr = gethostbyname(gmond_hostname)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

    if (inet_ntop(AF_INET, 
                  (void *)hptr->h_addr, 
                  ip_buffer,
                  INET_ADDRSTRLEN) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
  }
  else if (gmond_ip != NULL) {
    /* we don't care about hostname, just store IP address */
    if (strlen(gmond_ip)+1 > INET_ADDRSTRLEN) {
      handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
      goto cleanup;
    }
    strcpy(ip_buffer, gmond_ip);
  }

  if (gmond_port == 0 || gmond_port == -1)
    gmond_port = GANGLIA_DEFAULT_XML_PORT;

  if (timeout_len == 0 || timeout_len == -1)
    timeout_len = NODEUPDOWN_TIMEOUT_LEN;

  if (nodeupdown_retrieve_genders_data(handle, genders_filename) == -1) {
    goto cleanup;
  }

  if ((handle->up_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if ((handle->down_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if (nodeupdown_retrieve_gmond_data(handle, 
                                     ip_buffer, 
                                     gmond_port, 
                                     timeout_len) == -1)
    goto cleanup;

  if (nodeupdown_compare_genders_to_gmond_nodes(handle))
    goto cleanup;

  if ((handle->max_nodes = genders_getnumnodes(handle->genders_handle)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }

  hostlist_sort(handle->up_nodes);
  hostlist_sort(handle->down_nodes);

  /* loading complete */
  handle->loaded_flag = NODEUPDOWN_DATA_LOADED;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

 cleanup:

  nodeupdown_cleanup(handle);
  return -1;
}

int nodeupdown_retrieve_genders_data(nodeupdown_t handle,
                                     const char *filename) {

  if ((handle->genders_handle = genders_handle_create()) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  if (genders_load_data(handle->genders_handle, filename) == -1) {
    handle->errnum = NODEUPDOWN_ERR_OPEN;
    return -1;
  }

  return 0;
}

int nodeupdown_retrieve_gmond_data(nodeupdown_t handle, 
                                   const char *ip, 
                                   int port,
                                   int timeout_len) {
  XML_Parser xml_parser = NULL;
  int sockfd = -1;
  struct parse_vars parse_vars;
  struct timeval tv;

  if ((sockfd = low_timeout_connect(handle, ip, port)) == -1)
    goto cleanup;

  parse_vars.handle = handle;
  parse_vars.timeout_len = timeout_len;

  if (gettimeofday(&tv, NULL) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  } 

  parse_vars.localtime = tv.tv_sec;

  xml_parser = XML_ParserCreate(NULL);

  XML_SetElementHandler(xml_parser, xml_parse_start, xml_parse_end);
  XML_SetUserData(xml_parser, (void *)&parse_vars);

  while (1) {
    int bytes_read;
    void *buff = XML_GetBuffer(xml_parser, BUFSIZ);

    if (buff == NULL) {
      handle->errnum = NODEUPDOWN_ERR_GANGLIA;
      goto cleanup;
    }

    if ((bytes_read = read(sockfd, buff, BUFSIZ)) == -1) {
      handle->errnum = NODEUPDOWN_ERR_NETWORK;
      goto cleanup;
    }

    if (XML_ParseBuffer(xml_parser, bytes_read, bytes_read == 0) == 0) {
      handle->errnum = NODEUPDOWN_ERR_GANGLIA;
      goto cleanup;
    }

    if (bytes_read == 0)
      break;
  }

  close(sockfd);
  XML_ParserFree(xml_parser);

  return 0;

 cleanup:

  close(sockfd);

  if (xml_parser != NULL)
    XML_ParserFree(xml_parser);

  return -1;
}

int low_timeout_connect(nodeupdown_t handle, const char *ip, int port) {
  int ret, old_flags, error, error_len;
  int sockfd = -1;
  struct sockaddr_in servaddr;
  fd_set rset, wset;
  struct timeval tval;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  ret = inet_pton(AF_INET, ip, (void *)&servaddr.sin_addr);
  if (ret < 0) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  if (ret == 0) {
    handle->errnum = NODEUPDOWN_ERR_ADDRESS;
    goto cleanup;
  }

  /* Set the socket non-blocking.  We will do this so we can set a
   * smaller timeout on the connect() than the long default timeout
   * of around 3 minutes.
   */
  if ((old_flags = fcntl(sockfd, F_GETFL, 0)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  if (fcntl(sockfd, F_SETFL, old_flags | O_NONBLOCK) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  ret = connect(sockfd, 
                (struct sockaddr *)&servaddr, 
                sizeof(struct sockaddr_in));

  if (ret == -1 && errno != EINPROGRESS) {
    handle->errnum = NODEUPDOWN_ERR_CONNECT;
    goto cleanup;
  }
  else if (ret == -1 && errno == EINPROGRESS) {
    /* We will use a timeout of 5 seconds, a "reasonable" timeout
     * length.
     */
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    FD_ZERO(&wset);
    FD_SET(sockfd, &wset);
    tval.tv_sec = CONNECT_TIMEOUT_LEN;
    tval.tv_usec = 0;
      
    if ((ret = select(sockfd+1, &rset, &wset, NULL, &tval)) == -1) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

    if (ret == 0) {
      handle->errnum = NODEUPDOWN_ERR_TIMEOUT;
      goto cleanup;
    }
    else {
      if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        error_len = sizeof(int);

        if (getsockopt(sockfd, 
                       SOL_SOCKET, 
                       SO_ERROR, 
                       &error, 
                       &error_len) == -1) {

          handle->errnum = NODEUPDOWN_ERR_INTERNAL;
          goto cleanup;
        }
        
        if (error != 0) {
          errno = error;
          handle->errnum = NODEUPDOWN_ERR_CONNECT;
          goto cleanup;
        }
        /* else no error, connected within timeout length */
      }
      else {
        handle->errnum = NODEUPDOWN_ERR_INTERNAL;
        goto cleanup;
      }
    }
  }

  /* reset flags */
  if (fcntl(sockfd, F_SETFL, old_flags) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }
  
  return sockfd;

 cleanup:

  close(sockfd);
  
  return -1;
}

void xml_parse_start(void *data, const char *e1, const char **attr) {
  nodeupdown_t handle = ((struct parse_vars *)data)->handle;
  int timeout_len = ((struct parse_vars *)data)->timeout_len;
  unsigned long localtime = ((struct parse_vars *)data)->localtime;
  char *ptr;
  char *buffer;
  unsigned long reported;

  /* ignore "CLUSTER" and "METRIC" tags.  Assume ganglia executed on
   * only 1 cluster 
   */
     
  if (strcmp("HOST", e1) == 0) {

    /* attributes of XML HOST tag
     * attr[0] - "NAME"
     * attr[1] - hostname
     * attr[2] - "IP"
     * attr[3] - ip address of host
     * attr[4] - "REPORTED"
     * attr[5] - time gmond received a multicast message from the host
     * attr[6] - "GMOND_STARTED"
     * attr[7] - when the host's gmond daemon started 
     */

    /* sanity check */
    if (strcmp(attr[0], "NAME") != 0 ||
        strcmp(attr[4], "REPORTED") != 0) {
      handle->errnum = NODEUPDOWN_ERR_GANGLIA;
      return;
    }

    if (get_genders_nodename(handle, attr[1], &buffer) == -1)
      return;

    /* shorten hostname if necessary */
    if ((ptr = strchr(buffer, '.')) != NULL)
      *ptr = '\0';

    /* store up or down */
    reported = atol(attr[5]);
    if (abs(localtime - reported) < timeout_len) {
      if (hostlist_push(handle->up_nodes, buffer) == 0) {
        free(buffer);
        handle->errnum = NODEUPDOWN_ERR_HOSTLIST; 
        return;
      }
    }
    else {
      if (hostlist_push(handle->down_nodes, buffer) == 0) {
        free(buffer);
        handle->errnum = NODEUPDOWN_ERR_HOSTLIST; 
        return;
      }
    }

    free(buffer);
  }
}

void xml_parse_end(void *data, const char *e1) {
  /* do nothing for the time being */
}

int get_genders_nodename(nodeupdown_t handle, 
                         const char *node, 
                         char **buffer) {
  int ret, len;
  char *buf = NULL;

  ret = genders_testnode(handle->genders_handle, node);
  if (ret == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }
  else {
    if (ret == 1) {
      /* it is a genders node! */

      if ((buf = (char *)malloc(strlen(node) + 1)) == NULL) {
        handle->errnum = NODEUPDOWN_ERR_OUTMEM;
        goto cleanup;
      }
      strcpy(buf, node);
    }
    else {
      /* node is the alternate name? */

      if ((len = genders_getmaxnodelen(handle->genders_handle)) == -1) {
        handle->errnum = NODEUPDOWN_ERR_GENDERS;
        goto cleanup;
      }

      if ((buf = (char *)malloc(len + 1)) == NULL) {
        handle->errnum = NODEUPDOWN_ERR_GENDERS;
        goto cleanup;
      }
      memset(buf, '\0', len + 1);

      ret = genders_getnodes(handle->genders_handle,
                             &buf,
                             1,
                             GENDERS_ALTNAME_ATTRIBUTE,
                             node);
      if (ret == 0) {
        handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
        goto cleanup;
      }
      else if (ret == -1 || ret > 1) {
        handle->errnum = NODEUPDOWN_ERR_GENDERS;
        goto cleanup;
      }
      /* else ret == 1, node name stored in buf */
    }
  }

  *buffer = buf;

  return 0;

 cleanup:

  free(buf);

  return -1;
}

int nodeupdown_compare_genders_to_gmond_nodes(nodeupdown_t handle) {
  int i, ret, num_nodes;
  char *altname = NULL;
  char **genders_nodes = NULL;

  /* get all genders nodes */

  if ((num_nodes = genders_nodelist_create(handle->genders_handle, 
                                           &genders_nodes)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }
  
  if ((ret = genders_getnodes(handle->genders_handle, 
                              genders_nodes, 
                              num_nodes, 
                              NULL,
                              NULL)) == -1) {
    (void)genders_nodelist_destroy(handle->genders_handle, genders_nodes);
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }
  
  if ((altname = (char *)malloc(MAXHOSTNAMELEN + 1)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }
  
  for (i = 0; i < num_nodes; i++) {

    /* check if gmond knows of this genders node */
    if ((hostlist_find(handle->up_nodes, genders_nodes[i]) == -1) &&
        (hostlist_find(handle->down_nodes, genders_nodes[i]) == -1)) {

      /* maybe it is stored as an alternate name?? */

      memset(altname, '\0', MAXHOSTNAMELEN + 1);
      if ((ret = genders_testattr(handle->genders_handle,
                                  genders_nodes[i],
                                  GENDERS_ALTNAME_ATTRIBUTE,
                                  altname,
                                  MAXHOSTNAMELEN + 1)) == -1) {
        handle->errnum = NODEUPDOWN_ERR_GENDERS;
        goto cleanup;
      }

      /* if there is no alternate name or if ganglia did not know
       * about this alternate name, this must also be a down node
       */
      if ((ret == 0) || 
          ((ret == 1) &&
           (hostlist_find(handle->up_nodes, altname) == -1) &&
           (hostlist_find(handle->down_nodes, altname) == -1))) {
        
        if (hostlist_push_host(handle->down_nodes, 
                               genders_nodes[i]) == 0) {
          handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
          goto cleanup;
        }
      }
    }
  }
  free(altname);
  altname = NULL;

  if (genders_nodelist_destroy(handle->genders_handle, genders_nodes) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }

  hostlist_sort(handle->down_nodes);

  return 0;

 cleanup: 
  
  (void)genders_nodelist_destroy(handle->genders_handle, genders_nodes);

  free(altname);

  return -1;
}

int nodeupdown_errnum(nodeupdown_t handle) {
  if (handle == NULL)
    return NODEUPDOWN_ERR_NULLHANDLE;
  else if (handle->magic != NODEUPDOWN_MAGIC_NUM)
    return NODEUPDOWN_ERR_MAGIC;

  return handle->errnum;
}

char *nodeupdown_strerror(int errnum) {
  if (errnum < NODEUPDOWN_ERR_MIN || errnum > NODEUPDOWN_ERR_MAX)
    return errmsg[NODEUPDOWN_ERR_ERRNUMRANGE];

  return errmsg[errnum];
}


char *nodeupdown_errormsg(nodeupdown_t handle) {
  return nodeupdown_strerror(nodeupdown_errnum(handle));
}

void nodeupdown_perror(nodeupdown_t handle, const char *msg) {
  char *errormsg;

  errormsg = nodeupdown_strerror(nodeupdown_errnum(handle));

  if (msg == NULL)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}

int nodeupdown_dump(nodeupdown_t handle, FILE *stream) {

  if (nodeupdown_loaded_handle_err_check(handle) == -1)
    return -1;

  if (stream == NULL)
    stream = stdout;

  if (handle->up_nodes != NULL) {
    if (output_hostlist_nodes(handle, 
                              handle->up_nodes, 
                              stream, 
                              "whatsup up nodes") == -1)
      return -1;
  }

  if (handle->down_nodes != NULL) {
    if (output_hostlist_nodes(handle, 
                              handle->down_nodes, 
                              stream, 
                              "whatsup down nodes") == -1)
      return -1;
  }
  fprintf(stream, "\n");

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int output_hostlist_nodes(nodeupdown_t handle, 
                          hostlist_t hostlist, 
                          FILE *stream, 
                          char *msg) {
  char *str;

  if ((str = get_hostlist_ranged_string(handle, hostlist)) == NULL)
    return -1;

  fprintf(stream, "%s: %s\n", msg, str);
  free(str);

  return 0;
}

int nodeupdown_get_up_nodes_string(nodeupdown_t handle, 
                                   char *buf, 
                                   int buflen) {

  return nodeupdown_get_nodes_string(handle, 
                                     buf, 
                                     buflen, 
                                     NODEUPDOWN_UP_NODES);
}

int nodeupdown_get_down_nodes_string(nodeupdown_t handle, 
                                     char *buf, 
                                     int buflen) {

  return nodeupdown_get_nodes_string(handle, 
                                     buf, 
                                     buflen, 
                                     NODEUPDOWN_DOWN_NODES);
}

int nodeupdown_get_nodes_string(nodeupdown_t handle, 
                                char *buf, 
                                int buflen,
                                int up_or_down) {
  char *str = NULL;
  hostlist_t hl;

  if (nodeupdown_loaded_handle_err_check(handle) == -1)
    return -1;

  if (buf == NULL || buflen <= 0) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    hl = handle->up_nodes;
  else
    hl = handle->down_nodes;

  if ((str = get_hostlist_ranged_string(handle, hl)) == NULL)
    return -1;

  if (strlen(str) >= buflen) {
    free(str);
    handle->errnum = NODEUPDOWN_ERR_OVERFLOW;
    return -1;
  }

  strcpy(buf, str);

  free(str);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int nodeupdown_get_up_nodes_list(nodeupdown_t handle, 
                                 char **list, 
                                 int len) {

  return nodeupdown_get_nodes_list(handle, 
                                   list, 
                                   len, 
                                   NODEUPDOWN_UP_NODES);
}

int nodeupdown_get_down_nodes_list(nodeupdown_t handle, 
                                   char **list, 
                                   int len) {

  return nodeupdown_get_nodes_list(handle, 
                                   list, 
                                   len, 
                                   NODEUPDOWN_DOWN_NODES);
}

int nodeupdown_get_nodes_list(nodeupdown_t handle, 
                              char **list, 
                              int len, 
                              int up_or_down) {
  int ret;
  hostlist_t hl;

  if (nodeupdown_loaded_handle_err_check(handle) == -1)
    return -1;

  if (list == NULL || len <= 0) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    hl = handle->up_nodes;
  else
    hl = handle->down_nodes;

  if ((ret = nodeupdown_copy_nodes_into_list(handle, 
                                             hl,
                                             list,
                                             len)) == -1)
    return -1;
 
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return ret;
}

int nodeupdown_copy_nodes_into_list(nodeupdown_t handle,
                                    hostlist_t src,
                                    char **dest,
                                    int len) {
  int count = 0;
  hostlist_iterator_t iter;
  char *nodename;
  
  if ((iter = hostlist_iterator_create(src)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
    return -1;
  }
  
  while ((nodename = hostlist_next(iter)) != NULL) {
    if (count >= len) {
      free(nodename);
      hostlist_iterator_destroy(iter);
      handle->errnum = NODEUPDOWN_ERR_OVERFLOW;
      return -1;
    }

    if (dest[count] == NULL) {
      handle->errnum = NODEUPDOWN_ERR_NULLPTR;
      return -1;
    }

    strcpy(dest[count], nodename);
    free(nodename);
    count++;
  }
  
  hostlist_iterator_destroy(iter);
  
  return count;
}

int nodeupdown_is_node_up(nodeupdown_t handle, const char *node) {
  return nodeupdown_is_node(handle, node, NODEUPDOWN_UP_NODES);
}

int nodeupdown_is_node_down(nodeupdown_t handle, const char *node) {
  return nodeupdown_is_node(handle, node, NODEUPDOWN_DOWN_NODES);
}

int nodeupdown_is_node(nodeupdown_t handle, const char *node, int up_or_down) {
  int ret;
  hostlist_t hl;

  if (nodeupdown_loaded_handle_err_check(handle) == -1)
    return -1;

  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    hl = handle->up_nodes;
  else
    hl = handle->down_nodes;

  if ((ret = nodeupdown_check_if_node_in_hostlist(handle,
                                                  hl,
                                                  node)) == -1)
    return -1;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return ret;
}

int nodeupdown_check_if_node_in_hostlist(nodeupdown_t handle,
                                         hostlist_t nodes,
                                         const char *node) {
  char *buffer = NULL;
  int return_value;

  if (hostlist_find(nodes, node) != -1) {
    handle->errnum = NODEUPDOWN_ERR_SUCCESS;
    return_value = 1;
  }
  else {
    /* user may have input alternate name instead of main node name, 
     * so lets find the main node name
     */

    if (get_genders_nodename(handle, node, &buffer) == -1)
      goto cleanup;

    if (hostlist_find(nodes, buffer) != -1)
      return_value = 1;
    else
      return_value = 0;
    
    free(buffer);
  }

  return return_value;

 cleanup:

  free(buffer);
  
  return -1;
}

int nodeupdown_nodelist_create(nodeupdown_t handle, char ***list) {
  int i, j;
  char **node;
  
  if (nodeupdown_loaded_handle_err_check(handle) == -1)
    return -1;

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  node = (char **)malloc(sizeof(char *) * handle->max_nodes);
  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    node[i] = (char *)malloc(MAXHOSTNAMELEN+1);
    if (node[i] == NULL) {

      for (j = 0; j < i; j++) 
        free(node[j]);
      free(node);

      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
    memset(node[i], '\0', MAXHOSTNAMELEN+1);
  }

  *list = node;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle->max_nodes;
}

int nodeupdown_nodelist_clear(nodeupdown_t handle, char **list) {
  int i;
  
  if (nodeupdown_loaded_handle_err_check(handle) == -1)
    return -1;

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    if (list[i] == NULL) {
      handle->errnum = NODEUPDOWN_ERR_NULLPTR;
      return -1;
    }
    memset(list[i], '\0', MAXHOSTNAMELEN+1);
  }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

}

int nodeupdown_nodelist_destroy(nodeupdown_t handle, char **list) {
  int i;

  if (nodeupdown_loaded_handle_err_check(handle) == -1)
    return -1;

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++)
    free(list[i]);
  free(list);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}
