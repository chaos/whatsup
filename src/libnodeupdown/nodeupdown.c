/*****************************************************************************\
 *  $Id: nodeupdown.c,v 1.103 2004-01-16 00:13:42 achu Exp $
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
#endif

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <sys/stat.h>

#include "list.h"
#include "hostlist.h"
#include "xmlparse.h"
#include "conffile.h"
#include "nodeupdown.h"
#include "nodeupdown_masterlist.h"
#include "nodeupdown_common.h"

#ifndef GANGLIA_DEFAULT_XML_PORT
#define GANGLIA_DEFAULT_XML_PORT  8649
#endif

/* to store config file data */
struct nodeupdown_confdata {
  List gmond_hostnames;
  int gmond_hostnames_found;
  int gmond_port;
  int gmond_port_found;
  int timeout_len;
  int timeout_len_found;
#if HAVE_HOSTSFILE
  char hostsfile[MAXPATHLEN+1];
  int hostsfile_found; 
#elif (HAVE_GENDERS || HAVE_GENDERSLLNL)
  char gendersfile[MAXPATHLEN+1];
  int gendersfile_found; 
#endif
};

/* to pass multiple variables as one during XML parsing */
struct parse_vars {
  nodeupdown_t handle;
  int timeout_len;
  unsigned long localtime;
};

/* error messages */
static char * errmsg[] = {
  "success",
  "nodeupdown handle is null",
  "connection to gmond server error",
  "connection to gmond server timeout",
  "improper gmond hostname error",
  "improper gmond address error",
  "network error",
  "data already loaded",
  "data not loaded",
  "array or string not large enough to store result",
  "incorrect parameters passed in",
  "null pointer reached in list",
  "out of memory",
  "node not found",
  "internal master list error",
  "open masterlist file error",
  "read masterlist file error",
  "parse masterlist error",
  "internal configuration file error",
  "open conf file error",
  "read conf file error",
  "parse conf file error",
  "internal XML parsing error",
  "internal hostlist error",
  "nodeupdown handle magic number incorrect, improper handle passed in",
  "internal system error",
  "error number out of range",
};

static int 
_handle_error_check(nodeupdown_t handle) 
{
  if (handle == NULL || handle->magic != NODEUPDOWN_MAGIC_NUM)
    return -1;

  return 0;
}

static int 
_unloaded_handle_error_check(nodeupdown_t handle) 
{
  if (_handle_error_check(handle) == -1)
    return -1;

  if (handle->is_loaded) {
    handle->errnum = NODEUPDOWN_ERR_ISLOADED;
    return -1;
  }

  return 0;
}

static int 
_loaded_handle_error_check(nodeupdown_t handle) 
{
  if (_handle_error_check(handle) == -1)
    return -1;

  if (!handle->is_loaded) {
    handle->errnum = NODEUPDOWN_ERR_NOTLOADED;
    return -1;
  }

  return 0;
}

static void 
_initialize_handle(nodeupdown_t handle) 
{
  handle->magic = NODEUPDOWN_MAGIC_NUM;
  handle->is_loaded = 0;
  handle->up_nodes = NULL;
  handle->down_nodes = NULL;
  handle->max_nodes = 0;
  nodeupdown_masterlist_initialize_handle(handle);
}

nodeupdown_t 
nodeupdown_handle_create() 
{
  nodeupdown_t handle;

  if ((handle = (nodeupdown_t)malloc(sizeof(struct nodeupdown))) == NULL)
    return NULL;
  
  _initialize_handle(handle);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle;
}

static void 
_free_handle_data(nodeupdown_t handle) 
{
  hostlist_destroy(handle->up_nodes);
  hostlist_destroy(handle->down_nodes);
  nodeupdown_masterlist_free_handle_data(handle);
  _initialize_handle(handle);
}

int 
nodeupdown_handle_destroy(nodeupdown_t handle) 
{
  if (_handle_error_check(handle) == -1)
    return -1;

  _free_handle_data(handle);

  /* "clean" magic number */ 
  handle->magic = ~NODEUPDOWN_MAGIC_NUM;

  free(handle);
  return 0;
}  

static int 
_cb_gmond_hostnames(conffile_t cf, struct conffile_data *data, char *optionname,
                    int option_type, void *option_ptr, int option_data,
                    void *app_ptr, int app_data) 
{
  List l = (List)option_ptr;
  char *str;
  int i;
  
  if (data->stringlist_len > NODEUPDOWN_CONF_GMOND_HOSTNAME_MAX)
    return -1;

  for (i = 0; i < data->stringlist_len; i++) {
    if (strlen(data->stringlist[i]) > MAXHOSTNAMELEN)
      return -1;
    if ((str = strdup(data->stringlist[i])) == NULL)
      return -1;
    if (list_append(l, str) == NULL)
      return -1;
  }
  return 0;
}

/* parse configuration file and store data into confdata */
static int 
_read_conffile(nodeupdown_t handle, struct nodeupdown_confdata *cd) 
{
  struct conffile_option options[] = {
    {NODEUPDOWN_CONF_GMOND_HOSTNAME, CONFFILE_OPTION_LIST_STRING, -1, 
     _cb_gmond_hostnames, 1, 0, &(cd->gmond_hostnames_found),
     cd->gmond_hostnames, 0},
    {NODEUPDOWN_CONF_GMOND_PORT, CONFFILE_OPTION_INT, 0, 
     conffile_int, 1, 0, &(cd->gmond_port_found), &(cd->gmond_port), 0},
    {NODEUPDOWN_CONF_TIMEOUT_LEN, CONFFILE_OPTION_INT, 0, 
     conffile_int, 1, 0, &(cd->timeout_len_found), &(cd->timeout_len), 0},
#if HAVE_HOSTSFILE
    {NODEUPDOWN_CONF_HOSTSFILE, CONFFILE_OPTION_STRING, 0, 
     conffile_string, 1, 0, &(cd->hostsfile_found), cd->hostsfile, MAXPATHLEN},
#elif (HAVE_GENDERS || HAVE_GENDERSLLNL)
    {NODEUPDOWN_CONF_GENDERSFILE, CONFFILE_OPTION_STRING, 0, 
     conffile_string, 1, 0, &(cd->gendersfile_found), cd->gendersfile, MAXPATHLEN}
#endif
  };
  conffile_t cf = NULL;
  int num, ret = -1;

  if ((cf = conffile_handle_create()) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
  }

  /* NODEUPDOWN_CONF_FILE defined in config.h */
  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, NODEUPDOWN_CONF_FILE, options, num, NULL, 0, 0) < 0) {
      /* Not an error if the file does not exist */
      if (conffile_errnum(cf) != CONFFILE_ERR_EXIST) {
        int errnum = conffile_errnum(cf);
        if (errnum == CONFFILE_ERR_OPEN)
          handle->errnum = NODEUPDOWN_ERR_CONF_OPEN;
        else if (errnum == CONFFILE_ERR_READ)
          handle->errnum = NODEUPDOWN_ERR_CONF_READ;
        else if (CONFFILE_IS_PARSE_ERR(errnum))
          handle->errnum = NODEUPDOWN_ERR_CONF_PARSE;
        else
          handle->errnum = NODEUPDOWN_ERR_CONF;
        goto cleanup;
      }
  }

  ret = 0;
 cleanup:
  (void)conffile_handle_destroy(cf);
  return ret;
}

static int 
_low_timeout_connect(nodeupdown_t handle, const char *hostname, int port) 
{
  int ret, old_flags, error, len, sockfd = -1;
  int sa_in_size = sizeof(struct sockaddr_in);
  struct sockaddr_in servaddr;
  fd_set rset, wset;
  struct timeval tval;
  char ip_buf[INET_ADDRSTRLEN+1];

  /* use default hostname? */
  if (hostname == NULL)
    strncpy(ip_buf, "127.0.0.1", INET_ADDRSTRLEN);
  else {
    struct hostent *hptr;

    /* valgrind will report a mem-leak in gethostbyname() */
    if ((hptr = gethostbyname(hostname)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_HOSTNAME;
      return -1;
    }

    if (!inet_ntop(AF_INET, (void *)hptr->h_addr, ip_buf, INET_ADDRSTRLEN)) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
  }

  /* use default port? */
  if (port <= 0)
    port = GANGLIA_DEFAULT_XML_PORT;

  /* Alot of this code is from Unix Network Programming, by Stevens */

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if ((ret = inet_pton(AF_INET, ip_buf, (void *)&servaddr.sin_addr)) < 0) {
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

/* xml start function for use with expat XML parsing library
 * - parse beginning tags like <FOO attr1=X attr2=Y> 
 */
static void 
_xml_parse_start(void *data, const char *e1, const char **attr) 
{
  nodeupdown_t handle = ((struct parse_vars *)data)->handle;
  int timeout_len = ((struct parse_vars *)data)->timeout_len;
  unsigned long localtime = ((struct parse_vars *)data)->localtime;
  char shorthostname[MAXHOSTNAMELEN+1];
  char buffer[MAXHOSTNAMELEN+1];
  unsigned long reported;
  char *ptr;
  int ret;

  if (strcmp("HOST", e1) == 0) {

    /* attributes of XML HOST tag
     * attr[0] - "NAME"
     * attr[1] - hostname
     * attr[2] - "IP"
     * attr[3] - ip address of host
     * attr[4] - "REPORTED"
     * attr[5] - time gmond received a multicast message from the host
     * - remaining attributes aren't needed 
     */

    /* shorten hostname if necessary */
    memset(shorthostname, '\0', MAXHOSTNAMELEN+1);
    strncpy(shorthostname, attr[1], MAXHOSTNAMELEN);
    if ((ptr = strchr(shorthostname, '.')) != NULL)
      *ptr = '\0';

    if (nodeupdown_masterlist_is_node_legit(handle, shorthostname) <= 0)
      return;

    if (nodeupdown_masterlist_get_nodename(handle, shorthostname, 
                                           buffer, MAXHOSTNAMELEN+1) == -1)
      return;
      
    /* store as up or down */
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

/* xml end function for use with expat XML parsing library
 * - parse end tags like </FOO>
 */
static void 
_xml_parse_end(void *data, const char *e1) 
{
  /* nothing to do at this time */
}

static int 
_get_gmond_data(nodeupdown_t handle, int fd, int timeout_len) 
{
  XML_Parser xml_parser = NULL;
  struct parse_vars pv;
  struct timeval tv;
  int retval = -1;

  /* Setup parse vars to pass to _xml_parse_start */

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

  /* Following XML parsing loop more or less ripped from libganglia by
   * Matt Massie <massie@CS.Berkeley.EDU>
   */

  xml_parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xml_parser, _xml_parse_start, _xml_parse_end);
  XML_SetUserData(xml_parser, (void *)&pv);

  while (1) {
    int bytes_read;
    void *buff;

    if ((buff = XML_GetBuffer(xml_parser, BUFSIZ)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_EXPAT;
      goto cleanup;
    }

    if ((bytes_read = read(fd, buff, BUFSIZ)) == -1) {
      handle->errnum = NODEUPDOWN_ERR_NETWORK;
      goto cleanup;
    }

    if (XML_ParseBuffer(xml_parser, bytes_read, bytes_read == 0) == 0) {
      handle->errnum = NODEUPDOWN_ERR_EXPAT;
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

int 
nodeupdown_load_data(nodeupdown_t handle, const char *gmond_hostname, 
                     int gmond_port, int timeout_len, 
#if HAVE_NOMASTERLIST
                         void *ptr
#elif HAVE_HOSTSFILE
                         char *hostsfile
#elif (HAVE_GENDERS || HAVE_GENDERSLLNL)
                         char *gendersfile
#endif
                         ) 
{
  struct nodeupdown_confdata cd;
  int port, fd = -1;

  if (_unloaded_handle_error_check(handle) == -1)
    return -1;

  /* Read conffile */
  memset(&cd, '\0', sizeof(struct nodeupdown_confdata));
  if ((cd.gmond_hostnames = list_create((ListDelF)free)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  if (_read_conffile(handle, &cd) < 0)
    goto cleanup;

  /* Must call masterlist_init before _connect_to_gmond */
#if HAVE_NOMASTERLIST
  if (nodeupdown_masterlist_init(handle, ptr) == -1)
    goto cleanup;
#elif HAVE_HOSTSFILE
  hostsfile = (hostsfile == NULL && cd.hostsfile_found > 0) ? 
    &(cd.hostsfile[0]) : hostsfile;
  if (nodeupdown_masterlist_init(handle, hostsfile) == -1)
    goto cleanup;
#elif (HAVE_GENDERS || HAVE_GENDERSLLNL)
  gendersfile = (gendersfile == NULL && cd.gendersfile_found > 0) ? 
    &(cd.gendersfile[0]) : gendersfile;
  if (nodeupdown_masterlist_init(handle, gendersfile) == -1)
    goto cleanup;
#endif

  if (gmond_hostname == NULL && cd.gmond_hostnames_found > 0) {
    ListIterator itr = NULL;
    char *str;

    /* Use conffile hostnames as default */
    if ((itr = list_iterator_create(cd.gmond_hostnames)) == NULL)
      goto cleanup;

    while ((str = list_next(itr)) != NULL) {
      port = (gmond_port <= 0 && cd.gmond_port_found > 0) ? 
        cd.gmond_port : gmond_port;
      if ((fd = _low_timeout_connect(handle, str, port)) >= 0) 
        break;
    }

    if (itr)
      list_iterator_destroy(itr);
  }
  else {
    port = (gmond_port <= 0 && cd.gmond_port_found > 0) ? 
      cd.gmond_port : gmond_port;
    fd = _low_timeout_connect(handle, gmond_hostname, port);
  }

  if (fd < 0)
    goto cleanup;

  if ((handle->up_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if ((handle->down_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  timeout_len = (timeout_len <= 0 && cd.timeout_len_found > 0) ? 
    cd.timeout_len : timeout_len;
  if (_get_gmond_data(handle, fd, timeout_len) == -1)
    goto cleanup;

  if (nodeupdown_masterlist_compare_gmond_to_masterlist(handle) == -1)
    goto cleanup;

  hostlist_sort(handle->up_nodes);
  hostlist_sort(handle->down_nodes);

  if (nodeupdown_masterlist_finish(handle) == -1)
    goto cleanup;

  /* loading complete */
  handle->is_loaded++;

  close(fd);
  list_destroy(cd.gmond_hostnames);
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

 cleanup:
  close(fd);
  if (cd.gmond_hostnames)
    list_destroy(cd.gmond_hostnames);
  _free_handle_data(handle);
  return -1;
}

int 
nodeupdown_errnum(nodeupdown_t handle) 
{
  if (handle == NULL)
    return NODEUPDOWN_ERR_NULLHANDLE;
  else if (handle->magic != NODEUPDOWN_MAGIC_NUM)
    return NODEUPDOWN_ERR_MAGIC;

  return handle->errnum;
}

char *
nodeupdown_strerror(int errnum) 
{
  if (errnum < NODEUPDOWN_ERR_SUCCESS || errnum > NODEUPDOWN_ERR_ERRNUMRANGE)
    return errmsg[NODEUPDOWN_ERR_ERRNUMRANGE];

  return errmsg[errnum];
}

char *
nodeupdown_errormsg(nodeupdown_t handle) 
{
  return nodeupdown_strerror(nodeupdown_errnum(handle));
}

void 
nodeupdown_perror(nodeupdown_t handle, const char *msg) 
{
  char *errormsg = nodeupdown_strerror(nodeupdown_errnum(handle));

  if (msg == NULL)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}

static int 
_get_nodes_string(nodeupdown_t handle, char *buf, int buflen, int up_or_down) 
{
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

int 
nodeupdown_get_up_nodes_string(nodeupdown_t handle, char *buf, int buflen) 
{
  return _get_nodes_string(handle, buf, buflen, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_get_down_nodes_string(nodeupdown_t handle, char *buf, int buflen) 
{
  return _get_nodes_string(handle, buf, buflen, NODEUPDOWN_DOWN_NODES);
}

static int 
_get_nodes_list(nodeupdown_t handle, char **list, int len, int up_or_down) 
{
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

int 
nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list, int len) 
{
  return _get_nodes_list(handle, list, len, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list, int len) 
{
  return _get_nodes_list(handle, list, len, NODEUPDOWN_DOWN_NODES);
}

static int 
_is_node(nodeupdown_t handle, const char *node, int up_or_down) 
{ 
  char buffer[MAXHOSTNAMELEN+1];
  int ret, retval = -1;

  if (_loaded_handle_error_check(handle) == -1)
    return -1;

  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if ((ret = nodeupdown_masterlist_is_node_in_cluster(handle, node)) == -1)
    return -1;

  if (ret == 0) {
    handle->errnum = NODEUPDOWN_ERR_NOTFOUND;
    return -1;
  }

  if (nodeupdown_masterlist_get_nodename(handle, node, 
                                         buffer, MAXHOSTNAMELEN+1) == -1)
    return -1;

  if (up_or_down == NODEUPDOWN_UP_NODES)
    ret = hostlist_find(handle->up_nodes, buffer);
  else
    ret = hostlist_find(handle->down_nodes, buffer);

  if (ret != -1)
    retval = 1;
  else
    retval = 0;
  
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return retval;
}

int 
nodeupdown_is_node_up(nodeupdown_t handle, const char *node) 
{
  return _is_node(handle, node, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_is_node_down(nodeupdown_t handle, const char *node) 
{
  return _is_node(handle, node, NODEUPDOWN_DOWN_NODES);
}

static int 
_node_count(nodeupdown_t handle, int up_or_down) 
{
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

int 
nodeupdown_up_count(nodeupdown_t handle) 
{
  return _node_count(handle, NODEUPDOWN_UP_NODES);
}

int 
nodeupdown_down_count(nodeupdown_t handle) 
{
  return _node_count(handle, NODEUPDOWN_DOWN_NODES);
}

int 
nodeupdown_nodelist_create(nodeupdown_t handle, char ***list) 
{
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

int 
nodeupdown_nodelist_clear(nodeupdown_t handle, char **list) 
{
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

int 
nodeupdown_nodelist_destroy(nodeupdown_t handle, char **list) 
{
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

void 
nodeupdown_set_errnum(nodeupdown_t handle, int errnum) 
{
  if (_handle_error_check(handle) == -1)
    return;

  if (errnum >= NODEUPDOWN_ERR_SUCCESS && errnum <= NODEUPDOWN_ERR_ERRNUMRANGE) 
    handle->errnum = errnum;
  else
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
}
