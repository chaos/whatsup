/*
 * $Id: nodeupdown.c,v 1.79 2003-11-08 18:59:39 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown.c,v $
 *    
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <ganglia.h>
#include <ganglia/xmlparse.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <sys/stat.h>

#include "hostlist.h"
#include "nodeupdown.h"
#include "nodeupdown_masterlist.h"
#include "nodeupdown_common.h"

/* for gethostbyname */
extern int h_errno;

/* used so multiple variables can be passed as one variable */
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
  "improper hostname error",
  "improper address error",
  "network error",
  "data already loaded",
  "data not loaded",
  "array or string not large enough to store result",
  "incorrect parameters passed in",
  "null pointer reached in list",
  "out of memory",
  "node not found",
  "internal master list error",
  "internal ganglia error",
  "internal hostlist error",
  "nodeupdown handle magic number incorrect, improper handle passed in",
  "internal system error",
  "error number out of range",
};

static int _handle_error_check(nodeupdown_t handle) {
  if (handle == NULL || handle->magic != NODEUPDOWN_MAGIC_NUM)
    return -1;

  return 0;
}

static int _unloaded_handle_error_check(nodeupdown_t handle) {
  if (_handle_error_check(handle) == -1)
    return -1;

  if (handle->is_loaded) {
    handle->errnum = NODEUPDOWN_ERR_ISLOADED;
    return -1;
  }

  return 0;
}

static int _loaded_handle_error_check(nodeupdown_t handle) {
  if (_handle_error_check(handle) == -1)
    return -1;

  if (!handle->is_loaded) {
    handle->errnum = NODEUPDOWN_ERR_NOTLOADED;
    return -1;
  }

  return 0;
}

static void _initialize_handle(nodeupdown_t handle) {
  handle->magic = NODEUPDOWN_MAGIC_NUM;
  handle->is_loaded = 0;
  handle->up_nodes = NULL;
  handle->down_nodes = NULL;
  handle->max_nodes = 0;
  nodeupdown_masterlist_initialize_handle(handle);
}

nodeupdown_t nodeupdown_handle_create() {
  nodeupdown_t handle;

  if ((handle = (nodeupdown_t)malloc(sizeof(struct nodeupdown))) == NULL)
    return NULL;
  
  _initialize_handle(handle);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle;
}

static void _free_handle_data(nodeupdown_t handle) {
  hostlist_destroy(handle->up_nodes);
  hostlist_destroy(handle->down_nodes);
  nodeupdown_masterlist_free_handle_data(handle);
  _initialize_handle(handle);
}

int nodeupdown_handle_destroy(nodeupdown_t handle) {
  if (_handle_error_check(handle) == -1)
    return -1;

  _free_handle_data(handle);

  /* "clean" magic number */ 
  handle->magic = ~NODEUPDOWN_MAGIC_NUM; /* ~0xfeedbeef = 0xeatbeef :-) */

  free(handle);
  return 0;
}  

static int _low_timeout_connect(nodeupdown_t handle, const char *ip, int port) {
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
    tval.tv_sec = NODEUPDOWN_CONNECT_LEN;
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

/* get proper IP address and port given gmond_hostname, ip, and port.
 * - caller responsible for large enough ip_buf
 */ 
static int _get_ip_and_port(nodeupdown_t handle, const char *gmond_hostname,
                            const char *gmond_ip, int gmond_port,
                            char *ip_buf, int iplen, int *port_buf) {

  if (gmond_hostname == NULL && gmond_ip == NULL)
    strcpy(ip_buf, "127.0.0.1");
  else if (gmond_hostname != NULL && gmond_ip == NULL) {
    /* if only hostname is given, determine ip address */
    struct hostent *hptr;

    /* valgrind will report a mem-leak in gethostbyname() */
    if ((hptr = gethostbyname(gmond_hostname)) == NULL) {
      if (h_errno == HOST_NOT_FOUND)
        handle->errnum = NODEUPDOWN_ERR_HOSTNAME;
      else
        handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

    if (inet_ntop(AF_INET, (void *)hptr->h_addr, ip_buf, iplen) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  }
  else if (gmond_ip != NULL) {
    /* we don't care about hostname, just use IP address */
    if (strlen(gmond_ip)+1 > iplen) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
    strcpy(ip_buf, gmond_ip);
  }

  if (gmond_port <= 0) 
    *port_buf = GANGLIA_DEFAULT_XML_PORT;
  else
    *port_buf = gmond_port;

  return 0;
}

/* parse configuration file 
 * - return ip and port of what's in the conf file
 */
static int _read_conffile(nodeupdown_t handle, char *ip_buf, int iplen, 
                          int *port_buf) {
  char buf[NODEUPDOWN_BUFFERLEN];
  char hostbuf[MAXHOSTNAMELEN+1];
  char ipbuf[INET_ADDRSTRLEN+1];
  int hostname_flag = 0, ip_flag = 0, port_flag = 0;
  int len, retval = -1;
  int fd = -1;
  int port_val = -1;
  char *conf_name, *conf_val;

  if ((fd = open(NODEUPDOWN_CONF_FILE, O_RDONLY)) < 0)
    goto cleanup;

  while ((len = _readline(handle, fd, buf, NODEUPDOWN_BUFFERLEN)) > 0) {

    if (buf[0] == '#')
      continue;        /* comment */
    
    if ((conf_name = strtok(buf, " \t\n")) == NULL)
      continue;        /* empty line */ 

    if ((conf_val = strtok(NULL, " \t\n")) == NULL)
      goto cleanup;    /* parse error, no value */

    if (strcmp(conf_name, NODEUPDOWN_CONF_HOSTNAME) == 0) {
      strncpy(hostbuf, conf_val, MAXHOSTNAMELEN+1);
      hostbuf[MAXHOSTNAMELEN] = '\0';
      hostname_flag++;
    }
    else if (strcmp(conf_name, NODEUPDOWN_CONF_IP) == 0) {
      strncpy(ipbuf, conf_val, MAXHOSTNAMELEN+1);
      ipbuf[INET_ADDRSTRLEN] = '\0';
      ip_flag++;
    }
    else if (strcmp(conf_name, NODEUPDOWN_CONF_PORT) == 0) {
      port_val = atoi(conf_val);
      port_flag++;
    }
    else
      goto cleanup;  /* parse error, invalid key */ 
  }

  if (len == -1)
    goto cleanup;

  /* did we find anything? */
  if (hostname_flag == 0 && ip_flag == 0 && port_flag == 0)
    goto cleanup;

  /* get ip and port based on the conffile info */
  if (_get_ip_and_port(handle, 
                       (hostname_flag > 0) ? hostbuf : NULL,
                       (ip_flag > 0) ? ipbuf : NULL, 
                       (port_flag > 0) ? port_val : -1, 
                       ip_buf, iplen, port_buf) == -1)
    goto cleanup; 

  retval = 0;
 cleanup:
  close(fd);
  return retval;
}

static int _connect_to_gmond(nodeupdown_t handle, const char *gmond_hostname,
                             const char *gmond_ip, int gmond_port) {
  int sockfd = -1;
  char ip_buf[INET_ADDRSTRLEN+1];
  int iplen = INET_ADDRSTRLEN+1;
  int port;

  if (_get_ip_and_port(handle, gmond_hostname, gmond_ip, gmond_port, 
                       ip_buf, iplen, &port) < 0)
    return -1;

  /* read conf file, see if we should first try some different defaults */
  if ((gmond_hostname == NULL && gmond_ip == NULL) || gmond_port <= 0) {
    char conf_ip_buf[INET_ADDRSTRLEN+1];
    int conf_port;

    /* if anything fails, just try again */
    if (_read_conffile(handle, conf_ip_buf, iplen, &conf_port) < 0) 
      goto try_again;

    /* connect using values specified in conf file */
    sockfd = _low_timeout_connect(handle, 
                                  (!gmond_hostname && !gmond_ip) ? 
                                          conf_ip_buf : ip_buf,
                                  (gmond_port <= 0) ? conf_port : port);
    if (sockfd > -1)
      return sockfd;
    /* else fallthrough and try with defaults */
  }

 try_again:
  return _low_timeout_connect(handle, ip_buf, port);
}

/* xml start function for use with ganglia XML library
 * - handle parsing of beginning tags
 */
static void _xml_parse_start(void *data, const char *e1, const char **attr) {
  nodeupdown_t handle = ((struct parse_vars *)data)->handle;
  int timeout_len = ((struct parse_vars *)data)->timeout_len;
  unsigned long localtime = ((struct parse_vars *)data)->localtime;
  char *ptr;
  char shorthostname[MAXHOSTNAMELEN+1];
  char buffer[MAXHOSTNAMELEN+1];
  unsigned long reported;
  int ret;

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
     */

    /* shorten hostname if necessary */
    memset(shorthostname, '\0', MAXHOSTNAMELEN+1);
    strcpy(shorthostname, attr[1]);
    shorthostname[MAXHOSTNAMELEN] = '\0';
    if ((ptr = strchr(shorthostname, '.')) != NULL)
      *ptr = '\0';

    if (nodeupdown_masterlist_get_nodename(handle, shorthostname, 
                                           buffer, MAXHOSTNAMELEN+1) == -1)
      return;
      
    if (nodeupdown_masterlist_is_node_legit(handle, buffer) <= 0)
      return;

    /* store up or down */
    reported = atol(attr[5]);
    if (abs(localtime - reported) < timeout_len)
      ret = hostlist_push(handle->up_nodes, buffer);
    else
      ret = hostlist_push(handle->down_nodes, buffer);

    if (ret == 0)
      return;

    if (nodeupdown_masterlist_increase_max_nodes(handle) == -1)
      return;
  }
}

/* xml end function for use with ganglia XML library
 * - handle parsing of end tags
 */
static void _xml_parse_end(void *data, const char *e1) {
  /* nothing to do */
}

static int _get_gmond_data(nodeupdown_t handle, int sockfd, int timeout_len) {
  XML_Parser xml_parser = NULL;
  struct parse_vars pv;
  struct timeval tv;
  int retval = -1;

  pv.handle = handle;

  if (timeout_len <= 0)
    pv.timeout_len = NODEUPDOWN_TIMEOUT_LEN;
  else
    pv.timeout_len = timeout_len;

  /* Call gettimeofday at the latest point right before XML stuff. */
  if (gettimeofday(&tv, NULL) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  } 
  pv.localtime = tv.tv_sec;

  xml_parser = XML_ParserCreate(NULL);

  XML_SetElementHandler(xml_parser, _xml_parse_start, _xml_parse_end);
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

  retval = 0;
 cleanup:
  if (xml_parser != NULL)
    XML_ParserFree(xml_parser);
  return retval;
}

int nodeupdown_load_data(nodeupdown_t handle, 
#if (HAVE_MASTERLIST || HAVE_GENDERS)
			 const char *filename,
#else
                         void *ptr,
#endif
                         const char *gmond_hostname, const char *gmond_ip, 
                         int gmond_port, int timeout_len) {

  int sockfd = -1;

  if (_unloaded_handle_error_check(handle) == -1)
    return -1;

  /* Must call before _connect_to_gmond */
  /* XXX ACK, I know this is ugly! What's a cleaner way?? */
  if (nodeupdown_masterlist_init(handle,
#if (HAVE_MASTERLIST || HAVE_GENDERS)
                                 (void *)filename
#else
                                 (void *)ptr
#endif
                                 ) == -1)
    goto cleanup;

  sockfd = _connect_to_gmond(handle, gmond_hostname, gmond_ip, gmond_port);
  if (sockfd == -1)
    goto cleanup;

  if ((handle->up_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if ((handle->down_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if (_get_gmond_data(handle, sockfd, timeout_len) == -1)
    goto cleanup;

  if (nodeupdown_masterlist_compare_gmond_to_masterlist(handle) == -1)
    goto cleanup;

  hostlist_sort(handle->up_nodes);
  hostlist_sort(handle->down_nodes);

  if (nodeupdown_masterlist_finish(handle) == -1)
    goto cleanup;

  /* loading complete */
  handle->is_loaded++;

  close(sockfd);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

 cleanup:
  close(sockfd);
  _free_handle_data(handle);
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
  if (errnum < NODEUPDOWN_ERR_SUCCESS || errnum > NODEUPDOWN_ERR_ERRNUMRANGE)
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

static int _get_nodes_string(nodeupdown_t handle, char *buf, int buflen,
                             int up_or_down) {
  hostlist_t hl;
 
  if (_loaded_handle_error_check(handle) == -1)
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

int nodeupdown_get_up_nodes_string(nodeupdown_t handle, 
                                   char *buf, int buflen) {
  return _get_nodes_string(handle, buf, buflen, NODEUPDOWN_UP_NODES);
}

int nodeupdown_get_down_nodes_string(nodeupdown_t handle, 
                                     char *buf, int buflen) {
  return _get_nodes_string(handle, buf, buflen, NODEUPDOWN_DOWN_NODES);
}

static int _get_nodes_list(nodeupdown_t handle, char **list, int len, 
                           int up_or_down) {
  int count = 0;
  hostlist_t hl;
  hostlist_iterator_t iter;
  char *nodename = NULL;

  if (_loaded_handle_error_check(handle) == -1)
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

int nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list, int len) {
  return _get_nodes_list(handle, list, len, NODEUPDOWN_UP_NODES);
}

int nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list, int len) {
  return _get_nodes_list(handle, list, len, NODEUPDOWN_DOWN_NODES);
}

static int _is_node(nodeupdown_t handle, const char *node, int up_or_down) { 
  char buffer[MAXHOSTNAMELEN+1];
  int ret, return_value = -1;

  if (_loaded_handle_error_check(handle) == -1)
    return -1;

  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (nodeupdown_masterlist_get_nodename(handle, node, 
                                         buffer, MAXHOSTNAMELEN+1) == -1)
    return -1;

  if ((ret = nodeupdown_masterlist_is_node_in_cluster(handle, buffer)) == -1)
    return -1;

  if (ret == 0) {
    handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
    return -1;
  }

  if (up_or_down == NODEUPDOWN_UP_NODES)
    ret = hostlist_find(handle->up_nodes, buffer);
  else
    ret = hostlist_find(handle->down_nodes, buffer);

  if (ret != -1)
    return_value = 1;
  else
    return_value = 0;
  
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return return_value;
}

int nodeupdown_is_node_up(nodeupdown_t handle, const char *node) {
  return _is_node(handle, node, NODEUPDOWN_UP_NODES);
}

int nodeupdown_is_node_down(nodeupdown_t handle, const char *node) {
  return _is_node(handle, node, NODEUPDOWN_DOWN_NODES);
}

static int _node_count(nodeupdown_t handle, int up_or_down) {
  int ret;

  if (_loaded_handle_error_check(handle) == -1)
    return -1;

  if (up_or_down == NODEUPDOWN_UP_NODES)
    ret = hostlist_count(handle->up_nodes);
  else
    ret = hostlist_count(handle->down_nodes);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return ret;
}

int nodeupdown_up_count(nodeupdown_t handle) {
  return _node_count(handle, NODEUPDOWN_UP_NODES);
}

int nodeupdown_down_count(nodeupdown_t handle) {
  return _node_count(handle, NODEUPDOWN_DOWN_NODES);
}

int nodeupdown_nodelist_create(nodeupdown_t handle, char ***list) {
  int i, j;
  char **nodes;
  
  if (_loaded_handle_error_check(handle) == -1)
    return -1;

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if ((nodes = (char **)malloc(sizeof(char *) * handle->max_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    if ((nodes[i] = (char *)malloc(MAXHOSTNAMELEN+1)) == NULL) {
      for (j = 0; j < i; j++) 
        free(nodes[j]);
      free(nodes);

      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
    memset(nodes[i], '\0', MAXHOSTNAMELEN+1);
  }

  *list = nodes;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle->max_nodes;
}

int nodeupdown_nodelist_clear(nodeupdown_t handle, char **list) {
  int i;
  
  if (_loaded_handle_error_check(handle) == -1)
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

  if (_loaded_handle_error_check(handle) == -1)
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

void nodeupdown_set_errnum(nodeupdown_t handle, int errnum) {
  if (_handle_error_check(handle) == -1)
    return;

  if (errnum >= NODEUPDOWN_ERR_SUCCESS && errnum <= NODEUPDOWN_ERR_ERRNUMRANGE) 
    handle->errnum = errnum;
  else
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
}
