/*
 * $Id: nodeupdown.c,v 1.1.1.1 2003-02-19 19:27:33 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/nodeupdown.c,v $
 *    
 */

#include "nodeupdown.h"

/*********************************
 * Definitions                   *
 *********************************/

/* struct nodeupdown
 * - handle for all nodeupdown API functions
 * errnum - error number
 * genders_filename - genders filename
 * gmond_hostname - gmond server hostname
 * gmond_ip - gmond server IP address
 * gmond_port - gmond server port
 * genders_nodes - nodes stored in the genders file
 * genders_alt_nodes - alternate names of nodes stored in genders file
 * gmond_nodes - nodes stored in the gmond
 * up_nodes - nodes the gmond believes are up
 * down_nodes - nodes the gmond believes are down
 * genders_handle - handle for genders API
 * ganglia_context - handle for ganglia API
 */

struct nodeupdown {
  int errnum;
  char *genders_filename;
  char *gmond_hostname;
  char *gmond_ip;
  int gmond_port;
  hostlist_t gmond_nodes;
  hostlist_t gmond_up_nodes;
  hostlist_t gmond_down_nodes;
  hostlist_t whatsup_up_nodes;
  hostlist_t whatsup_down_nodes;
  genders_t genders_handle;
  cluster_t *ganglia_cluster;
  int max_nodes;
};

/* error messages */
static char * errmsg[] = {
  "success",
  "open file error",
  "read file error",
  "connection to server error",
  "improper address error",
  "network error",
  "data not loaded",
  "incorrect parameters passed in",
  "out of memory",
  "unknown internal error"
};

/* struct cluster_and_hostlist
 * - used so two pointers can be passed as one variable
 *   by hashforeach()
 */
struct cluster_and_hostlist {
  cluster_t *cluster;
  hostlist_t hostlist;
};

/**********************************
 * Function Definitions           *
 **********************************/

/* initialize nodeupdown handle */
static int nodeupdown_initialization(nodeupdown_t handle);

/* retrieve gmond data from the genders file */
static int nodeupdown_retrieve_genders_data(nodeupdown_t handle);

/* retrieve gmond data from a gmond server */
static int nodeupdown_retrieve_gmond_data(nodeupdown_t handle);

/* retrieve gmond up node data */
static int nodeupdown_retrieve_gmond_up_nodes_data(nodeupdown_t handle);

/* retrieve gmond down node data */
static int nodeupdown_retrieve_gmond_down_nodes_data(nodeupdown_t handle);

/* used by hashforeach - retrieve all nodes gmond knows of */
static int store_gmond_nodes(datum_t *key, datum_t *val, void *arg);

/* used by hashforeach - retrieve all up or down nodes gmond knows of */
static int store_gmond_up_or_down_nodes(datum_t *key, datum_t *val, void *arg);

/* convert node names stored in gmond to standard (genders) hostnams */
static int nodeupdown_convert_up_nodes_to_standard_hostnames(nodeupdown_t handle);

/* convert node names stored in gmond to standard (genders) hostnams */
static int nodeupdown_convert_down_nodes_to_standard_hostnames(nodeupdown_t handle);

/* determines if the specified node is stored in the genders file 
 * returns: 1 if node is found, 0 if not found, -1 on error
 */
static int nodeupdown_is_node_in_genders_file(nodeupdown_t handle, char *node);

/* compare genders nodes to gmond nodes */
static int nodeupdown_compare_genders_to_gmond_down_nodes(nodeupdown_t handle);

/* free memory from nodeupdown handle */
static int nodeupdown_cleanup(nodeupdown_t handle);

/* output nodes stored in a hostlist, helper to nodeupdown_dump() */
static int output_hostlist_nodes(nodeupdown_t handle, 
				 hostlist_t hostlist, 
				 FILE *stream, 
				 char *msg);

/* wrapper function to call all up node related functions */
static int nodeupdown_calculate_and_store_up_nodes(nodeupdown_t handle);

/* wrapper function to call all down node related functions */
static int nodeupdown_calculate_and_store_down_nodes(nodeupdown_t handle);


nodeupdown_t nodeupdown_create() {
  nodeupdown_t handle;
  if ((handle = (nodeupdown_t)malloc(sizeof(struct nodeupdown))) == NULL) {
    return NULL;
  }

  nodeupdown_initialization(handle);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle;
}

int nodeupdown_initialization(nodeupdown_t handle) {
  if (handle == NULL) {
    return -1;
  }
  handle->genders_filename = NULL;
  handle->gmond_hostname = NULL;
  handle->gmond_ip = NULL;
  handle->gmond_port = -1;
  handle->gmond_nodes = NULL;
  handle->gmond_up_nodes = NULL;
  handle->gmond_down_nodes = NULL;
  handle->whatsup_up_nodes = NULL;
  handle->whatsup_down_nodes = NULL;
  handle->genders_handle = NULL;
  handle->ganglia_cluster = NULL;
  handle->max_nodes = -1;

  return 0;
}

int nodeupdown_load_data(nodeupdown_t handle, 
			 char *genders_filename, 
			 char *gmond_hostname, 
			 char *gmond_ip, 
			 int gmond_port) {
  if (handle == NULL) {
    return -1;
  }

  /* determine filename */
  if (genders_filename != NULL) {
    if ((handle->genders_filename = strdup(genders_filename)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
  }
  else {
    if ((handle->genders_filename = strdup(DEFAULT_GENDERS_FILE)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
  }

  /* handle hostname and IP paramters, there are four combinations
   *  of parameters 
   */
  if (gmond_hostname == NULL && gmond_ip == NULL) {
    /* use defaults */

    if ((handle->gmond_hostname = strdup("localhost")) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
    if ((handle->gmond_ip = strdup("127.0.0.1")) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
  }
  else if (gmond_hostname != NULL && gmond_ip == NULL) {
    /* if only hostname is given, determine ip address */

    struct hostent *hptr;

    if ((handle->gmond_hostname = strdup(gmond_hostname)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

    if ((hptr = gethostbyname(handle->gmond_hostname)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

    if ((handle->gmond_ip = (char *)malloc(INET_ADDRSTRLEN)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
    memset(handle->gmond_ip, '\0', INET_ADDRSTRLEN);

    if (inet_ntop(AF_INET, 
		  (void *)hptr->h_addr, 
		  handle->gmond_ip, 
		  INET_ADDRSTRLEN) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
  }
  else if (gmond_hostname == NULL && gmond_ip != NULL) {
    /* if only ip address is given, determine hostname */
    struct hostent *hptr;
    struct in_addr temp_in_addr;

    if ((handle->gmond_ip = strdup(gmond_ip)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

    if (inet_pton(AF_INET, handle->gmond_ip, &temp_in_addr) <= 0) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

    if ((hptr = gethostbyaddr(&temp_in_addr,
			      sizeof(struct in_addr), 
			      AF_INET)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

    if ((handle->gmond_hostname = strdup(hptr->h_name)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
  }
  else {
    /* if both are given, it is the responsibility of the user 
     * that the ip address actually is the IP address of the 
     * hostname 
     */
    if ((handle->gmond_hostname = strdup(gmond_hostname)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }

    if ((handle->gmond_ip = strdup(gmond_ip)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      goto cleanup;
    }
  }

  if (gmond_port != 0 && gmond_port != -1) {
    handle->gmond_port = gmond_port;
  }
  else {
    handle->gmond_port = GANGLIA_DEFAULT_XML_PORT;
  }

  if (nodeupdown_retrieve_genders_data(handle) == -1) {
    goto cleanup;
  }

  if (nodeupdown_retrieve_gmond_data(handle) == -1) {
    goto cleanup;
  }

  if ((handle->max_nodes = genders_getnumnodes(handle->genders_handle)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

 cleanup:

  nodeupdown_cleanup(handle);
  return -1;
}

int nodeupdown_retrieve_genders_data(nodeupdown_t handle) {

  if ((handle->genders_handle = genders_handle_create()) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  if (genders_open(handle->genders_handle, handle->genders_filename) == -1) {
    handle->errnum = NODEUPDOWN_ERR_OPEN;
    return -1;
  }

  return 0;
}

int nodeupdown_retrieve_gmond_data(nodeupdown_t handle) {
  struct cluster_and_hostlist ch;

  if ((handle->gmond_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }
 
  if ((handle->ganglia_cluster = (cluster_t *)malloc(sizeof(cluster_t))) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }
  handle->ganglia_cluster->source_list = NULL;
  handle->ganglia_cluster->host_cache = NULL;
  handle->ganglia_cluster->nodes = NULL;
  handle->ganglia_cluster->dead_nodes = NULL;
  handle->ganglia_cluster->llist = NULL;
  
  if (ganglia_cluster_init(handle->ganglia_cluster, 
			   NODEUPDOWN_GANGLIA_NAME, 
			   NODEUPDOWN_MAX_NODES_GUESS) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  /********************************************************
   * FIX LATER
   *
   * For some odd reason, the ganglia_cluster_init() 
   * function allocates several list node entries, but
   * never assigns values to them.  We set their values
   * to NULL to make sure we clean up correctly in the
   * nodeupdown_cleanup() function later on.  This block
   * should be removed once (if?) the ganglia library 
   * implements a "destroy cluster structure" function.
   ********************************************************/
  {
    if (handle->ganglia_cluster->source_list != NULL) {
      handle->ganglia_cluster->source_list->val = NULL;
      handle->ganglia_cluster->source_list->prev = NULL;
      handle->ganglia_cluster->source_list->next = NULL;
    }

    if (handle->ganglia_cluster->llist != NULL) {
      handle->ganglia_cluster->llist->val = NULL;
      handle->ganglia_cluster->llist->prev = NULL;
      handle->ganglia_cluster->llist->next = NULL;
    }
  }
  
  if (ganglia_add_datasource(handle->ganglia_cluster, 
			     NODEUPDOWN_GANGLIA_CLUSTER_NAME, 
			     handle->gmond_ip, 
			     handle->gmond_port, 
			     0) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  /********************************************************
   * FIX LATER
   *
   * As of the time of this code writing, the libganglia
   * library does not provide an API function to 
   * "get XML data from a gmond."
   *
   * There is a function called ganglia_sync_hash_with_xml()
   * that will retrieve the data, however, it cannot be 
   * called explicitly.  It can be called by the  
   * ganglia_cluster_print() function that outputs
   * debugging information to standard output.
   *
   * For the time being, we will re-route standard output to
   * /dev/null so this information is not printed, but we 
   * effectively call ganglia_sync_hash_with_xml() and retrieve
   * the information we desire.
   *
   * If the libganglia library is updated to include a 
   * a "get XML data from a gmond" API function in the future, 
   * the following block of code can be replaced with the
   * appropriate API call.
   ********************************************************/

  {
    FILE *temp;
    FILE *dv;
    int sockfd;
    struct sockaddr_in servaddr;
    int ret;
    char buffer[1000];

    /* FIX LATER
     *
     * Unfortunately, there is no way to capture a hostname,
     * IP address, or port error because there is no "get XML
     * data from gmond" function available.  We will try to
     * connect to the server ourselves, and "eat" all of the data 
     * it returns.  This way, we can output appropriate error
     * messages if there is a failure.
     *
     * Yeah, this is inefficient, but I think its necessary ...
     */
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(handle->gmond_port);
    
    ret = inet_pton(AF_INET, handle->gmond_ip, (void *)&servaddr.sin_addr);
    if (ret < 0) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
    if (ret == 0) {
      handle->errnum = NODEUPDOWN_ERR_ADDRESS;
      return -1;
    }

    if (connect(sockfd, 
		(struct sockaddr *)&servaddr, 
		sizeof(struct sockaddr_in)) == -1) {
      handle->errnum = NODEUPDOWN_ERR_CONNECT;
      return -1;
    }

    /* eat the data that is returned */
    /* could we technically close the connection?  Perhaps.  But
     * better not to test if gmond can handle SIG_PIPE signals.
     */
    do {
      ret = read(sockfd, buffer, 1000);
      if (ret == -1) {
	close(sockfd);
	handle->errnum = NODEUPDOWN_ERR_NETWORK;
	return -1;
      }
    } while (ret != 0);

    close(sockfd);

    temp = stdout;

    if ((dv = fopen("/dev/null", "w")) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

    stdout = dv;

    if (ganglia_cluster_print(handle->ganglia_cluster) == -1) {
      fclose(dv);
      stdout = temp;
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
    
    stdout = temp;
    fclose(dv);
  }

  /* 
   * store node information in hostlists 
   */

  ch.cluster = handle->ganglia_cluster;
  ch.hostlist = handle->gmond_nodes;
  if (hash_foreach(handle->ganglia_cluster->host_cache, 
		   store_gmond_nodes, 
		   (void *)&ch) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  return 0;
}

int nodeupdown_retrieve_gmond_up_nodes_data(nodeupdown_t handle) {
  struct cluster_and_hostlist ch;

  if ((handle->gmond_up_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  ch.cluster = handle->ganglia_cluster;
  ch.hostlist = handle->gmond_up_nodes;
  if (hash_foreach(handle->ganglia_cluster->nodes, 
		   store_gmond_up_or_down_nodes, 
		   (void *)&ch) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  return 0;
}

int nodeupdown_retrieve_gmond_down_nodes_data(nodeupdown_t handle) {
  struct cluster_and_hostlist ch;

  if ((handle->gmond_down_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  ch.cluster = handle->ganglia_cluster;
  ch.hostlist = handle->gmond_down_nodes;
  if (hash_foreach(handle->ganglia_cluster->dead_nodes, 
		   store_gmond_up_or_down_nodes, 
		   (void *)&ch) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  return 0;
}

int store_gmond_nodes(datum_t *key, datum_t *val, void *arg)
{
  hostlist_t nodes = ((struct cluster_and_hostlist *)arg)->hostlist;

  if (hostlist_push_host(nodes, (char *)val->data) == 0) {
    return -1;
  }
 
  return 0;
}

int store_gmond_up_or_down_nodes(datum_t *key, datum_t *val, void *arg)
{
  cluster_t *cluster = ((struct cluster_and_hostlist *)arg)->cluster;
  hostlist_t nodes = ((struct cluster_and_hostlist *)arg)->hostlist;
  datum_t *name;

  name = hash_lookup(key, cluster->host_cache );
  if (hostlist_push_host(nodes, (char *)name->data) == 0) {
    return -1;
  }
 
  if(name)
    datum_free(name);
  return 0;
}

int nodeupdown_convert_up_nodes_to_standard_hostnames(nodeupdown_t handle) {
  char **genders_nodes = NULL;
  int genders_nodes_count;
  char *nodename = NULL;
  hostlist_iterator_t gmond_up_nodes_iter = NULL;

  if ((handle->whatsup_up_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if ((genders_nodes = (char **)malloc(sizeof(char *))) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }
  if ((genders_nodes[0] = (char *)malloc(MAXHOSTNAMELEN+1)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }
  genders_nodes_count = 1;

  if ((gmond_up_nodes_iter = hostlist_iterator_create(handle->gmond_up_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  while ((nodename = hostlist_next(gmond_up_nodes_iter)) != NULL) {
    if (nodeupdown_is_node_in_genders_file(handle, nodename) == 1) {
      if (hostlist_push_host(handle->whatsup_up_nodes, nodename) == 0) {
	handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	goto cleanup;
      }
    }
    else {
      if (genders_getnodes(handle->genders_handle,
			   genders_nodes,
			   genders_nodes_count,
			   GENDERS_ALTNAME_ATTRIBUTE,
			   nodename) == 1) {
	if (hostlist_push_host(handle->whatsup_up_nodes, genders_nodes[0]) == 0) {
	  handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	  goto cleanup;
	}
      }
      else {
	printf("%d\n", genders_errnum(handle->genders_handle));
	handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	goto cleanup;
      }

    }
    free(nodename);
  }

  hostlist_iterator_destroy(gmond_up_nodes_iter);
  gmond_up_nodes_iter = NULL;

  free(genders_nodes[0]);
  free(genders_nodes);
  genders_nodes = NULL;

  return 0;

 cleanup: 

  if (genders_nodes != NULL) {
    (void) genders_nodelist_destroy(handle->genders_handle, genders_nodes);
  }
  
  if (gmond_up_nodes_iter != NULL) {
    hostlist_iterator_destroy(gmond_up_nodes_iter);
  }

  if (nodename != NULL) {
    free(nodename);
  }

  return -1;

}

int nodeupdown_convert_down_nodes_to_standard_hostnames(nodeupdown_t handle) {
  char **genders_nodes = NULL;
  int genders_nodes_count;
  char *nodename = NULL;
  hostlist_iterator_t gmond_down_nodes_iter = NULL;

  if ((handle->whatsup_down_nodes = hostlist_create(NULL)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  if ((genders_nodes = (char **)malloc(sizeof(char *))) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }
  if ((genders_nodes[0] = (char *)malloc(MAXHOSTNAMELEN+1)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }
  genders_nodes_count = 1;

  if ((gmond_down_nodes_iter = hostlist_iterator_create(handle->gmond_down_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  while ((nodename = hostlist_next(gmond_down_nodes_iter)) != NULL) {
    if (nodeupdown_is_node_in_genders_file(handle, nodename) == 1) {
      if (hostlist_push_host(handle->whatsup_down_nodes, nodename) == 0) {
	handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	goto cleanup;
      }
    }
    else {
      if (genders_getnodes(handle->genders_handle,
			   genders_nodes,
			   genders_nodes_count,
			   GENDERS_ALTNAME_ATTRIBUTE,
			   nodename) == 1) {
	if (hostlist_push_host(handle->whatsup_up_nodes, genders_nodes[0]) == 0) {
	  handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	  goto cleanup;
	}
      }
      else {
	handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	goto cleanup;
      }
    }
    free(nodename);
  }
  hostlist_iterator_destroy(gmond_down_nodes_iter);
  gmond_down_nodes_iter = NULL;

  free(genders_nodes[0]);
  free(genders_nodes);
  genders_nodes = NULL;

  return 0;

 cleanup: 

  if (genders_nodes != NULL) {
    (void) genders_nodelist_destroy(handle->genders_handle, genders_nodes);
  }

  if (gmond_down_nodes_iter != NULL) {
    hostlist_iterator_destroy(gmond_down_nodes_iter);
  }

  if (nodename != NULL) {
    free(nodename);
  }

  return -1;
}

int nodeupdown_is_node_in_genders_file(nodeupdown_t handle, char *node) {
  int ret;

  /* test by using genders_testattr() and searching for a "dummy" attribute */
  ret = genders_testattr(handle->genders_handle, node, "dummy_attribute", NULL);

  if (ret == 0 || ret == 1) {
    /* node found */
    return 1;
  }
  else if (ret == -1 && 
	   (genders_errnum(handle->genders_handle) == GENDERS_ERR_NOTFOUND)) {
    /* node not found */
    return 0;
  }
  else {
    /* error */
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

}

nodeupdown_compare_genders_to_gmond_down_nodes(nodeupdown_t handle) {
  int i, ret, genders_nodes_count;
  char *altname = NULL;
  char **genders_nodes = NULL;

  if ((genders_nodes_count = genders_nodelist_create(handle->genders_handle, 
						     &genders_nodes)) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }
  
  if ((ret = genders_getnodes(handle->genders_handle, 
			      genders_nodes, 
			      genders_nodes_count, 
			      NULL,
			      NULL)) == -1) {
    (void)genders_nodelist_destroy(handle->genders_handle, genders_nodes);
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }
  
  if (ret != genders_nodes_count) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }

  if ((altname = (char *)malloc(MAXHOSTNAMELEN + 1)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }
  
  for (i = 0; i < genders_nodes_count; i++) {
    if (hostlist_find(handle->gmond_nodes, genders_nodes[i]) == -1) {
      memset(altname, '\0', MAXHOSTNAMELEN + 1);
      if (genders_testattr(handle->genders_handle,
			   genders_nodes[i],
			   GENDERS_ALTNAME_ATTRIBUTE,
			   altname) == 1) {
	if (hostlist_find(handle->gmond_nodes, altname) == -1) {
	  if (hostlist_push_host(handle->whatsup_down_nodes, genders_nodes[i]) == 0) {
	    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	    goto cleanup;
	  }
	}
      }
      else {
	if (hostlist_push_host(handle->whatsup_down_nodes, genders_nodes[i]) == 0) {
	  handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	  goto cleanup;
	}
      }
    }
  }
  free(altname);
  altname = NULL;

  if (genders_nodelist_destroy(handle->genders_handle, genders_nodes) == -1) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    goto cleanup;
  }
  genders_nodes = NULL;

  return 0;

 cleanup: 
  if (genders_nodes != NULL) {
    (void)genders_nodelist_destroy(handle->genders_handle, genders_nodes);
  }

  if (altname != NULL) {
    free(altname);
  }

  return -1;
}

int nodeupdown_destroy(nodeupdown_t handle) {
  nodeupdown_cleanup(handle);
  return 0;
}  

int nodeupdown_cleanup(nodeupdown_t handle) {
  if (handle == NULL) {
    return -1;
  }
  if (handle->genders_filename != NULL) {
    free(handle->genders_filename);
  }
  if (handle->gmond_hostname != NULL) {
    free(handle->gmond_hostname);
  }
  if (handle->gmond_ip != NULL) {
    free(handle->gmond_ip);
  }
  if (handle->gmond_nodes != NULL) {
    hostlist_destroy(handle->gmond_nodes);
  }
  if (handle->gmond_up_nodes != NULL) {
    hostlist_destroy(handle->gmond_up_nodes);
  }
  if (handle->gmond_down_nodes != NULL) {
    hostlist_destroy(handle->gmond_down_nodes);
  }
  if (handle->whatsup_up_nodes != NULL) {
    hostlist_destroy(handle->whatsup_up_nodes);
  }
  if (handle->whatsup_down_nodes != NULL) {
    hostlist_destroy(handle->whatsup_down_nodes);
  }
  if (handle->genders_handle != NULL) {
    (void)genders_close(handle->genders_handle);
    (void)genders_handle_destroy(handle->genders_handle);
  }
  if (handle->ganglia_cluster != NULL) {

    /********************************************************
     * FIX LATER
     *
     * As of the time of this code writing, the libganglia
     * library is in beta, and the majority of what should
     * be in a legitimate library is not yet written.
     *
     * There is currently no API function that handles 
     * "destroy cluster data" or "free cluster data". 
     * Therefore, the following block of code must
     * manually free the data.  
     *
     * If the libganglia library is updated to include a 
     * "destroy" or "free" API function in the future, the
     * following block of code can be replaced with the
     * appropriate API call.
     ********************************************************/

    {

    llist_entry *temp;

    /* free hashes */
    if (handle->ganglia_cluster->host_cache != NULL) {
      hash_destroy(handle->ganglia_cluster->host_cache);
    }
    if (handle->ganglia_cluster->nodes != NULL) {
      hash_destroy(handle->ganglia_cluster->nodes);
    }
    if (handle->ganglia_cluster->dead_nodes != NULL) {
      hash_destroy(handle->ganglia_cluster->dead_nodes);
    }

    /* free lists */
    while (handle->ganglia_cluster->source_list != NULL) {
      temp = handle->ganglia_cluster->source_list->next;
      if (handle->ganglia_cluster->source_list->val != NULL) {
	free(handle->ganglia_cluster->source_list->val);
      }
      free(handle->ganglia_cluster->source_list);
      handle->ganglia_cluster->source_list = temp;
    }

    while (handle->ganglia_cluster->llist != NULL) {
      temp = handle->ganglia_cluster->llist->next;
      if (handle->ganglia_cluster->llist->val != NULL) {
	free(handle->ganglia_cluster->llist->val);
      }
      free(handle->ganglia_cluster->llist);
      handle->ganglia_cluster->llist = temp;
    }

    free(handle->ganglia_cluster);

    }

  }
  nodeupdown_initialization(handle);

  return 0;
}

int nodeupdown_errnum(nodeupdown_t handle) {
  if (handle == NULL) {
    return -1;
  }

  return handle->errnum;
}

char *nodeupdown_strerror(int errnum) {
  if (errnum < NODEUPDOWN_ERR_MIN || errnum > NODEUPDOWN_ERR_MAX) {
    return NULL;
  }

  return errmsg[errnum];
}

void nodeupdown_perror(nodeupdown_t handle, char *msg) {
  char *errormsg;

  if (handle == NULL) {
    return;
  }

  errormsg = nodeupdown_strerror(nodeupdown_errnum(handle));

  if (msg == NULL) {
    fprintf(stderr, "%s\n", errormsg);
  }
  else {
    fprintf(stderr, "%s: %s\n", msg, errormsg);
  }
}

int nodeupdown_dump(nodeupdown_t handle, FILE *stream) {
  if (handle == NULL) {
    return -1;
  }

  if (stream == NULL) {
    stream = stdout;
  }

  if (handle->genders_filename != NULL) {
    fprintf(stream, "genders_filename: %s\n", handle->genders_filename);
  }
  if (handle->gmond_hostname != NULL) {
    fprintf(stream, "gmond_hostname: %s\n", handle->gmond_hostname);
  }
  if (handle->gmond_ip != NULL) {
    fprintf(stream, "gmond_ip: %s\n", handle->gmond_ip);
  }
  if (handle->gmond_nodes != NULL) {
    output_hostlist_nodes(handle, handle->gmond_nodes, stream, "gmond nodes");
  }
  if (handle->gmond_up_nodes != NULL) {
    output_hostlist_nodes(handle, handle->gmond_up_nodes, stream, "gmond up nodes");
  }
  if (handle->gmond_down_nodes != NULL) {
    output_hostlist_nodes(handle, handle->gmond_down_nodes, stream, "gmond down nodes");
  }
  if (handle->whatsup_up_nodes != NULL) {
    output_hostlist_nodes(handle, handle->whatsup_up_nodes, stream, "whatsup up nodes");
  }
  if (handle->whatsup_down_nodes != NULL) {
    output_hostlist_nodes(handle, handle->whatsup_down_nodes, stream, "whatsup down nodes");
  }
  fprintf(stream, "\n");

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int output_hostlist_nodes(nodeupdown_t handle, hostlist_t hostlist, FILE *stream, char *msg) {
  char *str;
  int str_len;

  str_len = NODEUPDOWN_BUFFERLEN;
  if ((str = (char *)malloc(str_len)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }
  memset(str, '\0', str_len);

  while (hostlist_ranged_string(hostlist, str_len, str) == -1) {
    free(str);
    str_len += NODEUPDOWN_BUFFERLEN;
    if ((str = (char *)malloc(str_len)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
    memset(str, '\0', str_len);
  }
  fprintf(stream, "%s: %s\n", msg, str);
  free(str);

  return 0;
}

int nodeupdown_calculate_and_store_up_nodes(nodeupdown_t handle) {
  if (nodeupdown_retrieve_gmond_up_nodes_data(handle) == -1) {
    return -1;
  }
  if (nodeupdown_convert_up_nodes_to_standard_hostnames(handle) == -1) {
    return -1;
  }
  return 0;
}

int nodeupdown_calculate_and_store_down_nodes(nodeupdown_t handle) {
  if (nodeupdown_retrieve_gmond_down_nodes_data(handle) == -1) {
    return -1;
  }
  if (nodeupdown_convert_down_nodes_to_standard_hostnames(handle) == -1) {
    return -1;
  }
  if (nodeupdown_compare_genders_to_gmond_down_nodes(handle) == -1) {
    return -1;
  }
  return 0;
}

int nodeupdown_get_up_nodes_hostlist(nodeupdown_t handle, hostlist_t hl) {

  hostlist_iterator_t iter;
  char *str;


  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL || handle->gmond_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (handle->whatsup_up_nodes == NULL) {
    if (nodeupdown_calculate_and_store_up_nodes(handle) == -1) {
      return -1;
    }
  }

  if((iter = hostlist_iterator_create(handle->whatsup_up_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  while ((str = hostlist_next(iter)) != NULL) {
    
    if (hostlist_push_host(hl, str) == 0) {
      free(str);
      hostlist_iterator_destroy(iter);
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
    free(str);
  }

  hostlist_iterator_destroy(iter);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int nodeupdown_get_down_nodes_hostlist(nodeupdown_t handle, hostlist_t hl) {
  hostlist_iterator_t iter;
  char *str;

  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL || handle->gmond_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (handle->whatsup_down_nodes == NULL) {
    if (nodeupdown_calculate_and_store_down_nodes(handle) == -1) {
      return -1;
    }
  }

  if((iter = hostlist_iterator_create(handle->whatsup_down_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  while ((str = hostlist_next(iter)) != NULL) {
    
    if (hostlist_push_host(hl, str) == 0) {
      free(str);
      hostlist_iterator_destroy(iter);
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
    free(str);
  }

  hostlist_iterator_destroy(iter);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int nodeupdown_get_up_nodes_list(nodeupdown_t handle, char **list) {
  int i, count = 0;
  hostlist_iterator_t iter;
  char *nodename;

  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL || handle->gmond_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (handle->whatsup_up_nodes == NULL) {
    if (nodeupdown_calculate_and_store_up_nodes(handle) == -1) {
      return -1;
    }
  }

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }
  
  if ((iter = hostlist_iterator_create(handle->whatsup_up_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  while ((nodename = hostlist_next(iter)) != NULL) {
    strcpy(list[i], nodename);
    free(nodename);
    count++;
  }

  hostlist_iterator_destroy(iter);
  
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int nodeupdown_get_down_nodes_list(nodeupdown_t handle, char **list) {
  int i, count = 0;
  hostlist_iterator_t iter;
  char *nodename;

  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL || handle->gmond_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (handle->whatsup_down_nodes == NULL) {
    if (nodeupdown_calculate_and_store_down_nodes(handle) == -1) {
      return -1;
    }
  }

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }
  
  if ((iter = hostlist_iterator_create(handle->whatsup_down_nodes)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  while ((nodename = hostlist_next(iter)) != NULL) {
    strcpy(list[i], nodename);
    free(nodename);
    count++;
  }

  hostlist_iterator_destroy(iter);
  
  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;
}

int nodeupdown_is_node_up(nodeupdown_t handle, char *node) {
  char *altname;
  int ret, return_value;

  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL || handle->gmond_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (handle->whatsup_up_nodes == NULL) {
    if (nodeupdown_calculate_and_store_up_nodes(handle) == -1) {
      return -1;
    }
  }

  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (hostlist_find(handle->whatsup_up_nodes, node) != -1) {
    handle->errnum = NODEUPDOWN_ERR_SUCCESS;
    return_value = 1;
  }
  else {
    if ((altname = malloc(MAXHOSTNAMELEN+1)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
    memset(altname, '\0', MAXHOSTNAMELEN+1);
    
    ret = genders_testattr(handle->genders_handle, 
			   node,
			   GENDERS_ALTNAME_ATTRIBUTE,
			   altname);
    if (ret == 1) {
      if (hostlist_find(handle->whatsup_up_nodes, altname) != -1) {
	return_value = 1;
      }
      else {
	return_value = 0;
      }
    }
    else if (ret == 0) {
      return_value = 0;
    }
    else {
      free(altname);
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
    free(altname);
  }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return return_value;
}

int nodeupdown_is_node_down(nodeupdown_t handle, char *node) {
  char *altname;
  int ret, return_value;

  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL || handle->gmond_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (handle->whatsup_down_nodes == NULL) {
    if (nodeupdown_calculate_and_store_down_nodes(handle) == -1) {
      return -1;
    }
  }
  
  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }
  
  if (hostlist_find(handle->whatsup_down_nodes, node) != -1) {
    handle->errnum = NODEUPDOWN_ERR_SUCCESS;
    return_value = 1;
  }
  else {
    if ((altname = malloc(MAXHOSTNAMELEN+1)) == NULL) {
      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
    memset(altname, '\0', MAXHOSTNAMELEN+1);

    ret = genders_testattr(handle->genders_handle, 
			   node,
			   GENDERS_ALTNAME_ATTRIBUTE,
			   altname);
    if (ret == 1) {
      if (hostlist_find(handle->whatsup_down_nodes, altname) != -1) {
	return_value = 1;
      }
      else {
	return_value = 0;
      }
    }
    else if (ret == 0) {
      return_value = 0;
    }
    else {
      free(altname);
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }
    free(altname);
  }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return return_value;
}

int nodeupdown_get_hostlist_alternate_names(nodeupdown_t handle, hostlist_t src, hostlist_t dest) {
  hostlist_iterator_t iter = NULL;
  char *nodename = NULL;
  char *altname = NULL;
  int ret;

  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL || handle->gmond_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (src == NULL || dest == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if ((iter = hostlist_iterator_create(src)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  if ((altname = malloc(MAXHOSTNAMELEN + 1)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  while ((nodename = hostlist_next(iter)) != NULL) {
 
    memset(altname, '\0', MAXHOSTNAMELEN + 1);
    ret = genders_testattr(handle->genders_handle, 
			   nodename,
			   GENDERS_ALTNAME_ATTRIBUTE,
			   altname);
    if (ret == 1) {
      if (hostlist_push_host(dest, altname) == 0) {
	handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	goto cleanup;
      }
    }
    else if (ret == 0) {
      if (hostlist_push_host(dest, nodename) == 0) {
	handle->errnum = NODEUPDOWN_ERR_INTERNAL;
	goto cleanup;
      }
    }
    else {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
    free(nodename);
  }
  
  hostlist_iterator_destroy(iter);
  free(altname);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

 cleanup:

  if (iter != NULL) {
    hostlist_iterator_destroy(iter);
  }

  if (nodename != NULL) {
    free(nodename);
  }

  if (altname != NULL){
    free(altname);
  }

  return -1;
}

int nodeupdown_get_list_alternate_names(nodeupdown_t handle, char **src, char **dest) {
  char *altname = NULL;
  int i, ret;
  
  if (handle == NULL) {
    return -1;
  }

  if (handle->genders_handle == NULL) {
    handle->errnum = NODEUPDOWN_ERR_LOAD;
    return -1;
  }

  if (src == NULL || dest == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if ((altname = malloc(MAXHOSTNAMELEN + 1)) == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    goto cleanup;
  }

  for (i = 0; i < handle->max_nodes; i++) {
 
    memset(altname, '\0', MAXHOSTNAMELEN + 1);
    ret = genders_testattr(handle->genders_handle, 
			   src[i],
			   GENDERS_ALTNAME_ATTRIBUTE,
			   altname);
    if (ret == 1) {
      strcpy(dest[i], altname);
    }
    else if (ret == 0) {
      strcpy(dest[i], src[i]);
    }
    else {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
    
  }
  
  free(altname);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

 cleanup:

  if (altname != NULL){
    free(altname);
  }

  return -1;

}

int nodeupdown_nodelist_create(nodeupdown_t handle, char ***list) {
  int i, j;
  char **node;
  
  if (handle == NULL) {
    return -1;
  }

  if (handle->whatsup_up_nodes == NULL || handle->whatsup_down_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  node = (char **)malloc(sizeof(char *) * handle->max_nodes);
  if (node == NULL) {
    handle->errnum = NODEUPDOWN_ERR_OUTMEM;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    node[i] = (char *)malloc(MAXHOSTNAMELEN);
    if (node[i] == NULL) {

      for (j = 0; j < i; j++) 
	free(node[j]);
      free(node);

      handle->errnum = NODEUPDOWN_ERR_OUTMEM;
      return -1;
    }
    memset(node[i], '\0', MAXHOSTNAMELEN);
  }

  *list = node;

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return handle->max_nodes;
}

int nodeupdown_nodelist_clear(nodeupdown_t handle, char **list) {
  int i;
  
  if (handle == NULL) {
    return -1;
  }

  if (handle->whatsup_up_nodes == NULL || handle->whatsup_down_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    memset(list[i], '\0', MAXHOSTNAMELEN);
  }

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

}

int nodeupdown_nodelist_destroy(nodeupdown_t handle, char **list) {
  int i;

  if (handle == NULL) {
    return -1;
  }

  if (handle->whatsup_up_nodes == NULL || handle->whatsup_down_nodes == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  if (list == NULL) {
    handle->errnum = NODEUPDOWN_ERR_PARAMETERS;
    return -1;
  }

  for (i = 0; i < handle->max_nodes; i++) {
    free(list[i]);
  }
  free(list);

  handle->errnum = NODEUPDOWN_ERR_SUCCESS;
  return 0;

}
