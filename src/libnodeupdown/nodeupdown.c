/*
 * $Id: nodeupdown.c,v 1.58 2003-05-30 19:20:28 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown.c,v $
 *    
 */

#include <gendersllnl.h>
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

#define NODEUPDOWN_UP_NODES              1
#define NODEUPDOWN_DOWN_NODES            0

#define NODEUPDOWN_DATA_LOADED           1
#define NODEUPDOWN_DATA_NOT_LOADED       0

#define NODEUPDOWN_MAGIC_NUM             0xfeedbeef

#define NODEUPDOWN_BUFFERLEN             65536

#ifndef CONNECT_TIMEOUT_LEN
#define CONNECT_TIMEOUT_LEN              5 
#endif

/* for gethostbyname */
extern int h_errno;

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
 */
struct parse_vars {
  nodeupdown_t handle;
  int timeout_len;
  unsigned long localtime;
  char *buf;
  int buflen;
};

/* error messages */
static char * errmsg[] = {
  "success",
  "nodeupdown handle is null",
  "open file error",
  "read file error",
  "connection to server error",
  "connection to server timeout",
  "improper hostname error",
  "improper address error",
  "network error",
  "data already loaded",
  "data not loaded",
  "array or string not large enough to store result",
  "incorrect parameters passed in",
  "null pointer reached in list",
  "out of memory",
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

/* initialize nodeupdown handle 
 */
static void initialize_handle(nodeupdown_t handle);

/* free memory from nodeupdown handle 
 */
static void cleanup_handle(nodeupdown_t handle);

/* retrieve data from the genders file 
 * Returns -1 on error, 0 on success
 */
static int get_genders_data(nodeupdown_t handle, const char *filename);

/* retrieve host data from gmond 
 * Returns -1 on error, 0 on success
 */
static int get_gmond_data(nodeupdown_t handle, 
                          const char *gmond_ip, 
                          int gmond_port, 
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

/* compare genders nodes to gmond nodes to identify addition down nodes
 * Return -1 on error, 0 on success
 */
static int compare_genders_to_gmond_nodes(nodeupdown_t handle);

/* wrapper function for common code between 
 * get_up_nodes_string() & get_down_node_strings()
 * Returns -1 on error, 0 on success
 */
static int get_nodes_string(nodeupdown_t handle, 
                            char *buf, 
                            int buflen, 
                            int which);

/* wrapper function for common code between
 * get_up_nodes_list() & get_down_nodes_list()
 * Returns -1 on error, 0 on success
 */
static int get_nodes_list(nodeupdown_t handle,
                          char **list, 
                          int len, 
                          int up_or_down);

/* wrapper function for common code between
 * is_node_up() and is_node_down()
 * Returns 1 if true, 0 if false, -1 on error
 */
static int is_node(nodeupdown_t handle, const char *node, int up_or_down);

/* common error checks for genders handle 
 * Returns -1 on error, 0 on success
 */
static int handle_err_check(nodeupdown_t handle);

/* common error checks for non-loaded nodeupdown data 
 * Returns -1 on error, 0 on success
 */
static int unloaded_handle_err_check(nodeupdown_t handle);

/* common error checks for loaded nodeupdown data 
 * Returns -1 on error, 0 on success
 */
static int loaded_handle_err_check(nodeupdown_t handle);

nodeupdown_t nodeupdown_handle_create() {
  nodeupdown_t handle;

  if ((handle = (nodeupdown_t)malloc(sizeof(struct nodeupdown))) == NULL)
    return NULL;
  
  initialize_handle(handle);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle;
}

void initialize_handle(nodeupdown_t handle) {
  handle->magic = NODEUPDOWN_MAGIC_NUM;
  handle->loaded_flag = NODEUPDOWN_DATA_NOT_LOADED;
  handle->up_nodes = NULL;
  handle->down_nodes = NULL;
  handle->genders_handle = NULL;
  handle->max_nodes = -1;
}

int nodeupdown_handle_destroy(nodeupdown_t handle) {

  if (handle_err_check(handle) == -1)
    return -1;

  cleanup_handle(handle);

  /* "clean" magic number 
   * ~0xfeedbeef = 0xeatbeef?
   */ 
  handle->magic = ~NODEUPDOWN_MAGIC_NUM;

  free(handle);
  return 0;
}  

void cleanup_handle(nodeupdown_t handle) {

  hostlist_destroy(handle->up_nodes);
  hostlist_destroy(handle->down_nodes);
  (void)genders_handle_destroy(handle->genders_handle);

  initialize_handle(handle);
}

int nodeupdown_load_data(nodeupdown_t handle, 
                         const char *genders_filename, 
                         const char *gmond_hostname, 
                         const char *gmond_ip, 
                         int gmond_port,
                         int timeout_len) {

  char ip_buf[INET_ADDRSTRLEN];

  if (unloaded_handle_err_check(handle) == -1)
    return -1;

  if (gmond_port < -1 || timeout_len < -1) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  /* determine filename */
  if (genders_filename == NULL)
    genders_filename = GENDERS_DEFAULT_FILE;

  /* handle hostname and IP paramters */
  memset(ip_buf, '\0', INET_ADDRSTRLEN);
  if (gmond_hostname == NULL && gmond_ip == NULL) {
    /* use default */
    strcpy(ip_buf, "127.0.0.1");
  }
  else if (gmond_hostname != NULL && gmond_ip == NULL) {
    /* if only hostname is given, determine ip address */
    struct hostent *hptr;
    int inetlen = INET_ADDRSTRLEN;

    /* valgrind will report a mem-leak in gethostbyname() */
    if ((hptr = gethostbyname(gmond_hostname)) == NULL) {
      if (h_errno == HOST_NOT_FOUND)
        handle->errnum = NODEUPDOWN_ERR_HOSTNAME;
      else
        handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

    if (inet_ntop(AF_INET, (void *)hptr->h_addr, ip_buf, inetlen) == NULL) {
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
    strcpy(ip_buf, gmond_ip);
  }

  if (gmond_port == 0 || gmond_port == -1)
    gmond_port = GANGLIA_DEFAULT_XML_PORT;

  if (timeout_len == 0 || timeout_len == -1)
    timeout_len = NODEUPDOWN_TIMEOUT_LEN;

  if (get_genders_data(handle, genders_filename) == -1)
    goto cleanup;

  if ((handle->up_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if ((handle->down_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if (get_gmond_data(handle, ip_buf, gmond_port, timeout_len) == -1)
    goto cleanup;

  if (compare_genders_to_gmond_nodes(handle))
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

  cleanup_handle(handle);
  return -1;
}

int get_genders_data(nodeupdown_t handle, const char *filename) {
  
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

int get_gmond_data(nodeupdown_t handle, 
                   const char *gmond_ip, 
                   int gmond_port, 
                   int timeout_len) {
  XML_Parser xml_parser = NULL;
  int sockfd = -1;
  struct parse_vars pv;
  struct timeval tv;
  pv.buf = NULL;

  if ((sockfd = low_timeout_connect(handle, gmond_ip, gmond_port)) == -1)
    goto cleanup;

  pv.handle = handle;
  pv.timeout_len = timeout_len;

  if (gettimeofday(&tv, NULL) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  } 

  pv.localtime = tv.tv_sec;

  /* create buffer here instead of in xml_parse_start, so we don't
   * have to continually re-malloc buffer space
   */
  if ((pv.buflen = genders_getmaxnodelen(handle->genders_handle)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }
  pv.buflen++;
                 
  if ((pv.buf = (char *)malloc(pv.buflen)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  xml_parser = XML_ParserCreate(NULL);

  XML_SetElementHandler(xml_parser, xml_parse_start, xml_parse_end);
  XML_SetUserData(xml_parser, (void *)&pv);

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
  free(pv.buf);
  XML_ParserFree(xml_parser);

  return 0;

 cleanup:

  close(sockfd);

  if (xml_parser != NULL)
    XML_ParserFree(xml_parser);

  free(pv.buf);

  return -1;
}

int low_timeout_connect(nodeupdown_t handle, const char *ip, int port) {
  int ret, old_flags, error, len, sockfd = -1;
  int sa_in_size = sizeof(struct sockaddr_in);
  struct sockaddr_in servaddr;
  fd_set rset, wset;
  struct timeval tval;

  /* Alot of this code is from Unix Network Programming, by Stevens */

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if ((ret = inet_pton(AF_INET, ip, (void *)&servaddr.sin_addr)) < 0) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  if (ret == 0) {
    handle->errnum = NODEUPDOWN_ERR_ADDRESS;
    goto cleanup;
  }

  if ((old_flags = fcntl(sockfd, F_GETFL, 0)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  if (fcntl(sockfd, F_SETFL, old_flags | O_NONBLOCK) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  ret = connect(sockfd, (struct sockaddr *)&servaddr, sa_in_size);
  if (ret == -1 && errno != EINPROGRESS) {
    handle->errnum = NODEUPDOWN_ERR_CONNECT;
    goto cleanup;
  }
  else if (ret == -1 && errno == EINPROGRESS) {
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
        len = sizeof(int);

        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
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
  char *buf = ((struct parse_vars *)data)->buf;
  int buflen = ((struct parse_vars *)data)->buflen;
  genders_t genders_handle = handle->genders_handle;
  char *ptr;
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

    memset(buf, '\0', buflen);
    if (genders_to_gendname(genders_handle, attr[1], buf, buflen) == -1)
      return;

    /* shorten hostname if necessary */
    if ((ptr = strchr(buf, '.')) != NULL)
      *ptr = '\0';

    /* store up or down */
    reported = atol(attr[5]);
    if (abs(localtime - reported) < timeout_len)
      (void)hostlist_push(handle->up_nodes, buf);
    else
      (void)hostlist_push(handle->down_nodes, buf);
  }
}

void xml_parse_end(void *data, const char *e1) {
  /* do nothing for the time being */
}

int compare_genders_to_gmond_nodes(nodeupdown_t handle) {
  int i, ret, num;
  char **nlist = NULL;
  genders_t genders_handle = handle->genders_handle;

  /* get all genders nodes */
  if ((num = genders_nodelist_create(genders_handle, &nlist)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }
  
  if ((ret = genders_getnodes(genders_handle, nlist, num, NULL, NULL)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }
  
  for (i = 0; i < num; i++) {
    /* check if gmond knows of this genders node */
    if ((hostlist_find(handle->up_nodes, nlist[i]) == -1) &&
        (hostlist_find(handle->down_nodes, nlist[i]) == -1)) {

      /* gmond doesn't know this genders node, it must also be down */
      if (hostlist_push_host(handle->down_nodes, nlist[i]) == 0) {
        handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
        goto cleanup;
      }
    }
  }

  if (genders_nodelist_destroy(handle->genders_handle, nlist) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }

  hostlist_sort(handle->down_nodes);
  return 0;

 cleanup: 
  
  (void)genders_nodelist_destroy(handle->genders_handle, nlist);
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
  char *errormsg = nodeupdown_strerror(nodeupdown_errnum(handle));

  if (msg == NULL)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}

int nodeupdown_get_up_nodes_string(nodeupdown_t handle, 
                                   char *buf, 
                                   int buflen) {
  return get_nodes_string(handle, buf, buflen, NODEUPDOWN_UP_NODES);
}

int nodeupdown_get_down_nodes_string(nodeupdown_t handle, 
                                     char *buf, 
                                     int buflen) {
  return get_nodes_string(handle, buf, buflen, NODEUPDOWN_DOWN_NODES);
}

int get_nodes_string(nodeupdown_t handle, 
                     char *buf, 
                     int buflen,
                     int up_or_down) {
  hostlist_t hl;
 
  if (loaded_handle_err_check(handle) == -1)
    return -1;

  if (buf == NULL || buflen <= 0) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    hl = handle->up_nodes;
  else
    hl = handle->down_nodes;

  if (hostlist_ranged_string(hl, buflen, buf) == -1) {
    handle->errnum = NODEUPDOWN_ERR_OVERFLOW;
    return -1;
  }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list, int len) {
  return get_nodes_list(handle, list, len, NODEUPDOWN_UP_NODES);
}

int nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list, int len) {
  return get_nodes_list(handle, list, len, NODEUPDOWN_DOWN_NODES);
}

int get_nodes_list(nodeupdown_t handle, char **list, int len, int up_or_down) {
  int count = 0;
  hostlist_t hl;
  hostlist_iterator_t iter;
  char *nodename = NULL;

  if (loaded_handle_err_check(handle) == -1)
    return -1;

  if (list == NULL || len <= 0) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    hl = handle->up_nodes;
  else
    hl = handle->down_nodes;

  if ((iter = hostlist_iterator_create(hl)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_HOSTLIST;
    return -1;
  }

  while ((nodename = hostlist_next(iter)) != NULL) {
    if (count >= len) {
      handle->errnum = NODEUPDOWN_ERR_OVERFLOW;
      goto cleanup;
    }

    if (list[count] == NULL) {
      handle->errnum = NODEUPDOWN_ERR_NULLPTR;
      goto cleanup;
    }

    strcpy(list[count], nodename);
    free(nodename);
    count++;
  }
  
  hostlist_iterator_destroy(iter);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return count;

 cleanup:

  free(nodename);
  hostlist_iterator_destroy(iter);
  return -1;
}

int nodeupdown_is_node_up(nodeupdown_t handle, const char *node) {
  return is_node(handle, node, NODEUPDOWN_UP_NODES);
}

int nodeupdown_is_node_down(nodeupdown_t handle, const char *node) {
  return is_node(handle, node, NODEUPDOWN_DOWN_NODES);
}

int is_node(nodeupdown_t handle, const char *node, int up_or_down) {
  char *buf = NULL;
  int buflen;
  int ret, return_value;

  if (loaded_handle_err_check(handle) == -1)
    return -1;

  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  /* make sure node passed in is legitimate */
  if ((ret = genders_isnode_or_altnode(handle->genders_handle, node)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    return -1;
  }
  else if (ret == 0) {
    handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
    return -1;
  }

  if ((buflen = genders_getmaxnodelen(handle->genders_handle)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }

  if ((buf = (char *)malloc(buflen + 1)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  /* get genders nodename */
  memset(buf, '\0', buflen + 1);
  if (genders_to_gendname(handle->genders_handle, node, buf, buflen+1) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    goto cleanup;
  }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    ret = hostlist_find(handle->up_nodes, buf);
  else
    ret = hostlist_find(handle->down_nodes, buf);

  if (ret != -1)
    return_value = 1;
  else
    return_value = 0;
  
  free(buf);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return return_value;

 cleanup:

  free(buf);
  return -1;
}

int nodeupdown_nodelist_create(nodeupdown_t handle, char ***list) {
  int i, j, maxnodelen;
  char **node;
  
  if (loaded_handle_err_check(handle) == -1)
    return -1;

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  /* we use maxnodelen b/c all nodes in nodeupdown_t are stored
   * as genders nodes
   */
  if ((maxnodelen = genders_getmaxnodelen(handle->genders_handle)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    return -1;
  }

  if ((node = (char **)malloc(sizeof(char *) * handle->max_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    if ((node[i] = (char *)malloc(maxnodelen+1)) == NULL) {
      for (j = 0; j < i; j++) 
        free(node[j]);
      free(node);

      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
    memset(node[i], '\0', maxnodelen+1);
  }

  *list = node;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle->max_nodes;
}

int nodeupdown_nodelist_clear(nodeupdown_t handle, char **list) {
  int i, maxnodelen;
  
  if (loaded_handle_err_check(handle) == -1)
    return -1;

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if ((maxnodelen = genders_getmaxnodelen(handle->genders_handle)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_GENDERS;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    if (list[i] == NULL) {
      handle->errnum = NODEUPDOWN_ERR_NULLPTR;
      return -1;
    }
    memset(list[i], '\0', maxnodelen+1);
  }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int nodeupdown_nodelist_destroy(nodeupdown_t handle, char **list) {
  int i;

  if (loaded_handle_err_check(handle) == -1)
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

int handle_err_check(nodeupdown_t handle) {
  if (handle == NULL || handle->magic != NODEUPDOWN_MAGIC_NUM)
    return -1;

  return 0;
}

int unloaded_handle_err_check(nodeupdown_t handle) {

  if (handle_err_check(handle) == -1)
    return -1;

  if (handle->loaded_flag == NODEUPDOWN_DATA_LOADED) {
    handle->errnum = NODEUPDOWN_ERR_ISLOADED;
    return -1;
  }

  return 0;
}

int loaded_handle_err_check(nodeupdown_t handle) {

  if (handle_err_check(handle) == -1)
    return -1;

  if (handle->loaded_flag == NODEUPDOWN_DATA_NOT_LOADED) {
    handle->errnum = NODEUPDOWN_ERR_NOTLOADED;
    return -1;
  }

  return 0;
}
