/*
 * $Id: libnodeupdown_test.c,v 1.2 2003-03-05 23:47:16 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/testsuite/libnodeupdown_test.c,v $
 *    
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <nodeupdown.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>

/**************************
 * DEFINITIONS            *
 **************************/

/*
 * Test Suite definitions
 */
#define LIBNODEUPDOWN_TEST_CONF             "libnodeupdown_test.conf"
#define LIBNODEUPDOWN_TEST_MAXNODES                                 3
#define LIBNODEUPDOWN_TEST_GMOND_PORT                            8659
#define LIBNODEUPDOWN_TEST_SLEEPTIME                                5
#define MAXBUFFERLEN                                             4096

/* 
 * Functions
 */

#define GET_UP_NODES_HOSTLIST         0
#define GET_DOWN_NODES_HOSTLIST       1

#define GET_UP_NODES_LIST             0
#define GET_DOWN_NODES_LIST           1

#define IS_NODE_UP                    0
#define IS_NODE_DOWN                  1

#define GET_HOSTLIST_ALTERNATE_NAMES  0
#define GET_LIST_ALTERNATE_NAMES      1

#define NODELIST_CREATE               0
#define NODELIST_CLEAR                1
#define NODELIST_DESTROY              2

/*
 * Flags for test situations
 * funny note: Amazingly, there is a machine here at LLNL named
 *             "foobar."  This caused me some headaches when I was
 *             trying to debug the testsuite and the libnodeupdown
 *             library.  We now use "foobar-fuhbar" as the
 *             the bad machine name. :-)
 */

#define EXECUTE                                      1
#define DO_NOT_EXECUTE                               0

#define IS_NULL                                      1
#define IS_NOT_NULL                                  0       

#define IS_NEGATIVE                                 -1
#define IS_ZERO                                      0
#define IS_POSITIVE_SMALL                            1
#define IS_POSITIVE_LONG                         99999

#define NO_ERRNUM                                    0

#define BAD_FILENAME                   "foobar-fuhbar"
#define GOOD_FILENAME                             NULL

#define BAD_HOSTNAME                   "foobar-fuhbar"
#define GOOD_HOSTNAME                             NULL

#define BAD_IP                       "333.333.333.333"
#define GOOD_IP                                   NULL

#define GOOD_PORT        LIBNODEUPDOWN_TEST_GMOND_PORT
#define BAD_PORT                                  9999

#define BAD_NODE                        "foobar-fubar"
#define NULL_NODE                                 NULL

/*
 * Node Sets
 */
#define NODE_NONE              0
#define NODE_A                 1
#define NODE_B                 2
#define NODE_C                 3
#define NODE_AB                4
#define NODE_AC                5
#define NODE_BC                6
#define NODE_ABC               7
#define NODE_ALL               7

/* struct test_env
 * - stores information about the test environment, in particular
 *   things from the configuration file.
 */
struct test_env {
  char *pdsh;
  char *pkill;
  char *gmond;
  char *conffile;
  char **nodes; 

  char *conf;
  char *nodes_ab;
  char *nodes_ac;
  char *nodes_bc;
  char *nodes_abc;
};

/* nodeupdown_load_data() parameter tests */
struct {
  int nodeupdown_handle;
  char *genders_filename;
  char *gmond_hostname;
  char *gmond_ip;
  int gmond_port;
  int return_value;
  int return_errnum;
} load_data_param_tests[] = {
  {IS_NULL,     BAD_FILENAME,  BAD_HOSTNAME,  BAD_IP,  BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, BAD_HOSTNAME,  BAD_IP,  BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     BAD_FILENAME,  GOOD_HOSTNAME, BAD_IP,  BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, GOOD_HOSTNAME, BAD_IP,  BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     BAD_FILENAME,  BAD_HOSTNAME,  GOOD_IP, BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, BAD_HOSTNAME,  GOOD_IP, BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     BAD_FILENAME,  GOOD_HOSTNAME, GOOD_IP, BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, GOOD_HOSTNAME, GOOD_IP, BAD_PORT,  -1, NO_ERRNUM              },
  {IS_NULL,     BAD_FILENAME,  BAD_HOSTNAME,  BAD_IP,  GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, BAD_HOSTNAME,  BAD_IP,  GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NULL,     BAD_FILENAME,  GOOD_HOSTNAME, BAD_IP,  GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, GOOD_HOSTNAME, BAD_IP,  GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NULL,     BAD_FILENAME,  BAD_HOSTNAME,  GOOD_IP, GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, BAD_HOSTNAME,  GOOD_IP, GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NULL,     BAD_FILENAME,  GOOD_HOSTNAME, GOOD_IP, GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NULL,     GOOD_FILENAME, GOOD_HOSTNAME, GOOD_IP, GOOD_PORT, -1, NO_ERRNUM              },
  {IS_NOT_NULL, BAD_FILENAME,  BAD_HOSTNAME,  BAD_IP,  BAD_PORT,  -1, NODEUPDOWN_ERR_OPEN    },
  {IS_NOT_NULL, GOOD_FILENAME, BAD_HOSTNAME,  BAD_IP,  BAD_PORT,  -1, NODEUPDOWN_ERR_ADDRESS },
  {IS_NOT_NULL, BAD_FILENAME,  GOOD_HOSTNAME, BAD_IP,  BAD_PORT,  -1, NODEUPDOWN_ERR_OPEN    },
  {IS_NOT_NULL, GOOD_FILENAME, GOOD_HOSTNAME, BAD_IP,  BAD_PORT,  -1, NODEUPDOWN_ERR_ADDRESS },
  {IS_NOT_NULL, BAD_FILENAME,  BAD_HOSTNAME,  GOOD_IP, BAD_PORT,  -1, NODEUPDOWN_ERR_INTERNAL},
  {IS_NOT_NULL, GOOD_FILENAME, BAD_HOSTNAME,  GOOD_IP, BAD_PORT,  -1, NODEUPDOWN_ERR_INTERNAL},
  {IS_NOT_NULL, BAD_FILENAME,  GOOD_HOSTNAME, GOOD_IP, BAD_PORT,  -1, NODEUPDOWN_ERR_OPEN    },
  {IS_NOT_NULL, GOOD_FILENAME, GOOD_HOSTNAME, GOOD_IP, BAD_PORT,  -1, NODEUPDOWN_ERR_CONNECT },
  {IS_NOT_NULL, BAD_FILENAME,  BAD_HOSTNAME,  BAD_IP,  GOOD_PORT, -1, NODEUPDOWN_ERR_OPEN    },
  {IS_NOT_NULL, GOOD_FILENAME, BAD_HOSTNAME,  BAD_IP,  GOOD_PORT, -1, NODEUPDOWN_ERR_ADDRESS },
  {IS_NOT_NULL, BAD_FILENAME,  GOOD_HOSTNAME, BAD_IP,  GOOD_PORT, -1, NODEUPDOWN_ERR_OPEN    },
  {IS_NOT_NULL, GOOD_FILENAME, GOOD_HOSTNAME, BAD_IP,  GOOD_PORT, -1, NODEUPDOWN_ERR_ADDRESS },
  {IS_NOT_NULL, BAD_FILENAME,  BAD_HOSTNAME,  GOOD_IP, GOOD_PORT, -1, NODEUPDOWN_ERR_INTERNAL},
  {IS_NOT_NULL, GOOD_FILENAME, BAD_HOSTNAME,  GOOD_IP, GOOD_PORT, -1, NODEUPDOWN_ERR_INTERNAL},
  {IS_NOT_NULL, BAD_FILENAME,  GOOD_HOSTNAME, GOOD_IP, GOOD_PORT, -1, NODEUPDOWN_ERR_OPEN    },
  /* {IS_NOT_NULL, GOOD_FILENAME, GOOD_HOSTNAME, GOOD_IP, GOOD_PORT,  0, NODEUPDOWN_ERR_SUCCESS }, */
  {-1, NULL, NULL, NULL, -1, -1, -1},
};

/* nodeupdown_get_up_nodes_hostlist() and nodeupdown_get_down_nodes_hostlist()
 * parameter tests 
 */
struct {
  int nodeupdown_handle;
  int hostlist;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} get_hostlist_param_tests[] = {
  {IS_NULL,     IS_NULL,     DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_LOAD      },
  {IS_NOT_NULL, IS_NULL,     EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  /* {IS_NOT_NULL, IS_NOT_NULL, EXECUTE,         0, NODEUPDOWN_ERR_SUCCESS   }, */
  {-1, -1, -1, -1, -1},
};

/* nodeupdown_get_up_nodes_list() and nodeupdown_get_down_nodes_list()
 * parameter tests
 */
struct {
  int nodeupdown_handle;
  int list;
  int len;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} get_list_param_tests[] = {
  {IS_NULL,     IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_POSITIVE_LONG,  DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_POSITIVE_LONG,  DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_LONG,  DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_LONG,  DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_OVERFLOW  },
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_LONG,  EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_LONG,  EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {-1, -1, -1, -1, -1},
};

/* nodeupdown_is_node_up() and nodeupdown_is_node_down() 
 * parameter tests
 */
struct {
  int nodeupdown_handle;
  char *node;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} is_node_param_tests[] = {
  {IS_NULL,      BAD_NODE,  DO_NOT_EXECUTE, -1, NO_ERRNUM                }, 
  {IS_NULL,      NULL_NODE, DO_NOT_EXECUTE, -1, NO_ERRNUM                }, 
  {IS_NOT_NULL,  BAD_NODE,  DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_LOAD      },
  {IS_NOT_NULL,  NULL_NODE, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {-1, NULL, -1, -1, -1},
};

/* nodeupdown_get_hostlist_alternate_names() and nodeupdown_get_list_alternate_names()
 * parameter tests
 */
struct {
  int nodeupdown_handle;
  int src;
  int dest;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} alternate_param_tests[] = {
  {IS_NULL,     IS_NULL,     IS_NULL,     DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_LOAD      },
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  /* {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, EXECUTE,         0, NODEUPDOWN_ERR_SUCCESS   }, */
  {-1, -1, -1, -1, -1, -1},
};

/* nodeupdown_nodelist_create(), nodeupdown_nodelist_clear() and
 * nodeupdown_nodelist_destroy() parameter tests
 */
struct {
  int nodeupdown_handle;
  int list;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} nodelist_param_tests[] = {
  {IS_NULL,     IS_NULL,     DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_LOAD      },
  {IS_NOT_NULL, IS_NULL,     EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  /* {IS_NOT_NULL, IS_NOT_NULL, EXECUTE,         0, NODEUPDOWN_ERR_SUCCESS   }, */
  {-1, -1, -1, -1, -1},
};

struct {
  int nodes_up;
  int nodes_down;
  int host_to_query;
  int return_value;
  int return_errnum;
} get_up_f[] = {
  {NODE_A,    NODE_BC,   0,      0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_AB,   NODE_C,    0,      0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_AC,   NODE_B,    0,      0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_ALL,  NODE_NONE, 0,      0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_A,    NODE_BC,   NODE_A, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_AB,   NODE_C,    NODE_A, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_AC,   NODE_B,    NODE_A, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_ALL,  NODE_NONE, NODE_A, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_B,    NODE_AC,   NODE_B, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_AB,   NODE_C,    NODE_B, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_BC,   NODE_A,    NODE_B, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_ALL,  NODE_NONE, NODE_B, 0, NODEUPDOWN_ERR_SUCCESS}, 
  {NODE_C,    NODE_AB,   NODE_C, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_AC,   NODE_B,    NODE_C, 0, NODEUPDOWN_ERR_SUCCESS},
  {NODE_BC,   NODE_A,    NODE_C, 0, NODEUPDOWN_ERR_SUCCESS}, 
  {NODE_ALL,  NODE_NONE, NODE_C, 0, NODEUPDOWN_ERR_SUCCESS}, 
  {-1, -1, -1, -1, -1},
}; 

/* read_and_store_line_from_file
 * - reads a line from a file and stores it
 */
int read_and_store_line_from_file(FILE *fp, char **ptr) {
  char buffer[MAXBUFFERLEN];
  memset(buffer, '\0', MAXBUFFERLEN);
  fscanf(fp, "%s\n", buffer);
  if ((*ptr = (char *)malloc(strlen(buffer)+1)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(*ptr, '\0', strlen(buffer) + 1);
  strcpy(*ptr, buffer);
  return 0;
}

/* initialize_test_env
 * - initialize the test environment
 */
int initialize_test_env(struct test_env * test_env) {
  FILE *fp = NULL;

  test_env->pdsh = NULL;
  test_env->pkill = NULL;
  test_env->gmond = NULL;
  test_env->conffile = NULL;
  test_env->nodes = NULL;
  test_env->conf = NULL;
  test_env->nodes_ab = NULL;
  test_env->nodes_ac = NULL;
  test_env->nodes_bc = NULL;
  test_env->nodes_abc = NULL;

  if ((fp = fopen(LIBNODEUPDOWN_TEST_CONF, "r")) == NULL) {
    printf("fopen() error\n"); 
    goto cleanup;
  }
  
  if (read_and_store_line_from_file(fp, &test_env->pdsh)  == -1) {
    printf("read_and_store_line_from_file() error\n");
    return -1;
  }
  if (read_and_store_line_from_file(fp, &test_env->pkill)  == -1) {
    printf("read_and_store_line_from_file() error\n");
    return -1;
  }
  if (read_and_store_line_from_file(fp, &test_env->gmond)  == -1) {
    printf("read_and_store_line_from_file() error\n");
    return -1;
  }
  if (read_and_store_line_from_file(fp, &test_env->conffile)  == -1) {
    printf("read_and_store_line_from_file() error\n");
    return -1;
  }
  if ((test_env->nodes = 
       (char **)malloc(sizeof(char *)*LIBNODEUPDOWN_TEST_MAXNODES)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  if (read_and_store_line_from_file(fp, &test_env->nodes[0])  == -1) {
    printf("read_and_store_line_from_file() error\n");
    return -1;
  }
  if (read_and_store_line_from_file(fp, &test_env->nodes[1])  == -1) {
    printf("read_and_store_line_from_file() error\n");
    return -1;
  }
  if (read_and_store_line_from_file(fp, &test_env->nodes[2])  == -1) {
    printf("read_and_store_line_from_file() error\n");
    return -1;
  }

  /* a variety of string concatenations are needed in this test suite.
   * Instead of wasting time concatenating strings throughout the code,
   * just make then one time and save them in the test environment
   */
  
  if ((test_env->conf = 
       (char *)malloc(strlen("--conf=") + strlen(test_env->conffile) + 1)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(test_env->conf, '\0', strlen("--conf=") + strlen(test_env->conffile) + 1);
  strcpy(test_env->conf, "--conf=");
  strcat(test_env->conf, test_env->conffile);

  if ((test_env->nodes_ab = (char *)malloc((MAXHOSTNAMELEN + 1)*2)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(test_env->nodes_ab, '\0', (MAXHOSTNAMELEN + 1)*2);
  strcpy(test_env->nodes_ab, test_env->nodes[0]);
  strcat(test_env->nodes_ab, ",");
  strcat(test_env->nodes_ab, test_env->nodes[1]);
      
  if ((test_env->nodes_ac = (char *)malloc((MAXHOSTNAMELEN + 1)*2)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(test_env->nodes_ac, '\0', (MAXHOSTNAMELEN + 1)*2);
  strcpy(test_env->nodes_ac, test_env->nodes[0]);
  strcat(test_env->nodes_ac, ",");
  strcat(test_env->nodes_ac, test_env->nodes[2]);

  if ((test_env->nodes_bc = (char *)malloc((MAXHOSTNAMELEN + 1)*2)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(test_env->nodes_bc, '\0', (MAXHOSTNAMELEN + 1)*2);
  strcpy(test_env->nodes_bc, test_env->nodes[1]);
  strcat(test_env->nodes_bc, ",");
  strcat(test_env->nodes_bc, test_env->nodes[2]);

  if ((test_env->nodes_abc = (char *)malloc((MAXHOSTNAMELEN + 1)*3)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(test_env->nodes_abc, '\0', (MAXHOSTNAMELEN + 1)*3);
  strcpy(test_env->nodes_abc, test_env->nodes[0]);
  strcat(test_env->nodes_abc, ",");
  strcat(test_env->nodes_abc, test_env->nodes[1]);
  strcat(test_env->nodes_abc, ",");
  strcat(test_env->nodes_abc, test_env->nodes[2]);

  fclose(fp);

  return 0;

 cleanup:

  if (fp != NULL) {
    fclose(fp);
  }

  return -1;

}

/* cleanup_test_env
 * - cleanup the struct test_env structure
 */
void cleanup_test_env(struct test_env *test_env) {
  int i;

  if (test_env->pdsh != NULL) {
    free(test_env->pdsh);
  }  
  if (test_env->pkill != NULL) {
    free(test_env->pkill);
  }  
  if (test_env->gmond != NULL) {
    free(test_env->gmond);
  }  
  if (test_env->conffile != NULL) {
    free(test_env->conffile);
  }  
  if (test_env->nodes != NULL) {
    for (i = 0; i < LIBNODEUPDOWN_TEST_MAXNODES; i++) {
      if (test_env->nodes[i] != NULL) {
	free(test_env->nodes[i]);
      }
    }
    free(test_env->nodes);
  }
  if (test_env->conf != NULL) {
    free(test_env->conf);
  }
  if (test_env->nodes_ab != NULL) {
    free(test_env->nodes_ab);
  }
  if (test_env->nodes_ac != NULL) {
    free(test_env->nodes_ac);
  }
  if (test_env->nodes_bc != NULL) {
    free(test_env->nodes_bc);
  }
  if (test_env->nodes_abc != NULL) {
    free(test_env->nodes_abc);
  }

  return;
}

/* get_nodes_ptr
 * - maps one of the node sets to a comma separated list of the nodes
 *   stored in the test_env;
 */
char * get_nodes_ptr(struct test_env *test_env, int nodes) {
  char * ptr;

  switch(nodes) {
  case NODE_A:
    ptr = test_env->nodes[0];
    break;
  case NODE_B:
    ptr = test_env->nodes[1];
    break;
  case NODE_C:
    ptr = test_env->nodes[2];
    break;
  case NODE_AB:
    ptr = test_env->nodes_ab;
    break;
  case NODE_AC:
    ptr = test_env->nodes_ac;
    break;
  case NODE_BC:
    ptr = test_env->nodes_bc;
    break;
  case NODE_ABC:
    ptr = test_env->nodes_abc; 
    break;
  case NODE_NONE:
    ptr = NULL;
    break;
  default:
    printf("INTERNAL ERROR: incorrect nodes value in get_nodes_ptr()\n");
    ptr = NULL;
    break;
  }

  return ptr;
}

/* start_gmonds
 * - run gmond on the specified nodes
 */
int start_gmonds(struct test_env *test_env, int nodes) {
  int pid, ret;
  char *ptr;
  pid = fork();
  if (pid == 0) {
    if ((ptr = get_nodes_ptr(test_env, nodes)) == NULL) {
      printf("INTERNAL ERROR: incorrect nodes value in start_gmonds()\n");
      exit(1);
    }
    if (ptr != NULL) {
      if (execl(test_env->pdsh, "pdsh", "-w", ptr, 
		test_env->gmond, test_env->conf, (char *)NULL) == -1) {
	printf("execl() error!  Did not execute \"pdsh -w %s %s %s\"", 
	       ptr, test_env->gmond, test_env->conf);
	exit(1);
      }
    }
    exit(0);
  }
  else if (pid > 0) {
    ret = waitpid(pid, NULL, 0);
    if (ret == -1) {
      printf("waitpid() error\n");
      return -1;
    }
    else if (ret != pid) {
      printf("waitpid() error, incorrect child process closed\n");
      return -1;
    }
    return 0;
  }
  else {
    printf("fork() error\n");
    return -1;
  }
}

/* close_gmonds
 * - close gmond on the specified nodes
 */
int close_gmonds(struct test_env *test_env, int nodes) {
  int pid, ret;
  char *ptr;

  pid = fork();
  if (pid == 0) {
    if ((ptr = get_nodes_ptr(test_env, nodes)) == NULL) {
      printf("INTERNAL ERROR: incorrect nodes value in close_gmonds()\n");
      exit(1);
    }
    if (ptr != NULL) {
      if (execl(test_env->pdsh, "pdsh", "-w", ptr, 
		test_env->pkill, "-P", "1", "gmond", (char *)NULL) == -1) {
	printf("execl() error!  Did not execute \"pdsh -w %s %s -P 1 gmond\"", 
	       ptr, test_env->pkill);
	exit(1);
      }
    }
    exit(0);
  }
  else if (pid > 0) {
    ret = waitpid(pid, NULL, 0);
    if (ret == -1) {
      printf("waitpid() error\n");
      return -1;
    }
    else if (ret != pid) {
      printf("waitpid() error, incorrect child process closed\n");
      return -1;
    }
    return 0;
  }
  else {
    printf("fork() error\n");
    return -1;
  }
}

/* output_result
 * - output pass or fail based on inputs
 */
void output_result(int index, nodeupdown_t handle, int result, int return_value, int return_errnum) {
  if (return_errnum != NO_ERRNUM) {
    if (result == return_value && nodeupdown_errnum(handle) == return_errnum) {
      printf("Test %d: PASS\n", index);
    }
    else {
      printf("Test %d: ***FAIL*** return value = %d, return errnum = %d\n", index, result, nodeupdown_errnum(handle));
    }
  }
  else {
    if (result == return_value) {
      printf("Test %d: PASS\n", index);
    }
    else {
      printf("Test %d: ***FAIL*** return value = %d\n", index, result);
    }
  }
}

/* run_load_data_param_test
 * - run parameters tests for nodeupdown_load_data()
 */
void run_load_data_param_test(struct test_env *test_env, 
			      int index, 
			      int nodeupdown_handle, 
			      char *genders_filename, 
			      char *gmond_hostname,
			      char *gmond_ip, 
			      int gmond_port, 
			      int return_value, 
			      int return_errnum) {
  nodeupdown_t handle = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    if ((handle = nodeupdown_create()) == NULL) {
      printf("INTERNAL ERROR: nodeupdown_create()\n");
      return;
    }
  }

  result = nodeupdown_load_data(handle, genders_filename, gmond_hostname, gmond_ip, gmond_port); 
  
  output_result(index, handle, result, return_value, return_errnum);

  if (nodeupdown_handle == IS_NOT_NULL) {
    if (nodeupdown_destroy(handle) == -1) {
      printf("INTERNAL ERROR: nodeupdown_destroy()\n");
      return;
    }
  }
}

/* run_get_nodes_hostlist_param_test
 * - run parameters tests for nodeupdown_get_up_nodes_hostlist() and
 *   nodeupdown_get_down_nodes_hostlist()
 */
void run_get_nodes_hostlist_param_test(struct test_env *test_env, 
				       int function, 
				       int index, 
				       nodeupdown_t handle_not_loaded,
				       nodeupdown_t handle_loaded,
				       int nodeupdown_handle, 
				       int hostlist, 
				       int nodeupdown_load_data, 
				       int return_value, 
				       int return_errnum) {
  nodeupdown_t handle = NULL;
  hostlist_t hl = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    handle = handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = handle_loaded;
  }

  if (hostlist == IS_NOT_NULL) {
    if ((hl = hostlist_create(NULL)) == NULL) {
      printf("INTERNAL ERROR: hostlist_create()\n");
      return;
    }
  }

  if (function == GET_UP_NODES_HOSTLIST) {
    result = nodeupdown_get_up_nodes_hostlist(handle, hl);
  }
  else {
    result = nodeupdown_get_down_nodes_hostlist(handle, hl);
  }

  output_result(index, handle, result, return_value, return_errnum);
	    
  if (hostlist == IS_NOT_NULL) {
    hostlist_destroy(hl);
  }
  
}

/* run_get_nodes_list_param_test
 * - run parameters tests for nodeupdown_get_up_nodes_list() and
 *   nodeupdown_get_down_nodes_list()
 */
void run_get_up_nodes_list_param_test(struct test_env *test_env, 
				      int function, 
				      int index, 
				      nodeupdown_t handle_not_loaded,
				      nodeupdown_t handle_loaded,
				      int nodeupdown_handle, 
				      int list_flag, 
				      int len, 
				      int nodeupdown_load_data, 
				      int return_value, 
				      int return_errnum) {
  nodeupdown_t handle = NULL;
  char **list = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    handle = handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = handle_loaded;
  }

  if (list_flag == IS_NOT_NULL) {
    if ((list = (char **)malloc(sizeof(char *))) == NULL) {
      printf("malloc() error\n");
      return;
    }
  }

  if (function == GET_UP_NODES_LIST) {
    result = nodeupdown_get_up_nodes_list(handle, list, len);
  }
  else {
    result = nodeupdown_get_down_nodes_list(handle, list, len);
  }

  output_result(index, handle, result, return_value, return_errnum);
	    
  if (list == IS_NOT_NULL) {
    free(list);
  }

}

/* run_is_node_param_test
 * - run parameter tests for nodeupdown_is_node_up() and
 *   nodeupdown_is_node_down()
 */
void run_is_node_param_test(struct test_env *test_env, 
			    int function, 
			    int index,
			    nodeupdown_t handle_not_loaded,
			    nodeupdown_t handle_loaded,
			    int nodeupdown_handle, 
			    char *node, 
			    int nodeupdown_load_data, 
			    int return_value, 
			    int return_errnum) {
  nodeupdown_t handle = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    handle = handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = handle_loaded;
  }

  if (function == IS_NODE_UP) {
    result = nodeupdown_is_node_up(handle, node);
  }
  else {
    result = nodeupdown_is_node_down(handle, node);
  }

  output_result(index, handle, result, return_value, return_errnum);
}

/* run_alternate_param_test
 * - run parameter tests for nodeupdown_get_alternate_names_hostlist()
 *   and nodeupdown_get_alternate_names_list()
 */
void run_alternate_param_test(struct test_env *test_env, 
			      int function,
			      int index,
			      nodeupdown_t handle_not_loaded,
			      nodeupdown_t handle_loaded,
			      int nodeupdown_handle, 
			      int src_flag,
			      int dest_flag,
			      int nodeupdown_load_data, 
			      int return_value, 
			      int return_errnum) {
  nodeupdown_t handle = NULL;
  hostlist_t src_h = NULL;
  hostlist_t dest_h = NULL;
  char **src_c = NULL;
  char **dest_c = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    handle = handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = handle_loaded;
  }

  if (function == GET_HOSTLIST_ALTERNATE_NAMES) {
    if (src_flag == IS_NOT_NULL) {
      if ((src_h = hostlist_create(NULL)) == NULL) {
	printf("INTERNAL ERROR: hostlist_create()\n");
	return;
      }
    }
    if (dest_flag == IS_NOT_NULL) {
      if ((dest_h = hostlist_create(NULL)) == NULL) {
	printf("INTERNAL ERROR: hostlist_create()\n");
	return;
      }
    }
  }
  else {
    if (src_flag == IS_NOT_NULL) {
      if ((src_c = (char **)malloc(sizeof(char *))) == NULL) {
	printf("malloc() error\n");
	return;
      }
    }
    if (dest_flag == IS_NOT_NULL) {
      if ((dest_c = (char **)malloc(sizeof(char *))) == NULL) {
	printf("malloc() error\n");
	return;
      }
    }
  }

  if (function == GET_HOSTLIST_ALTERNATE_NAMES) {
    result = nodeupdown_get_hostlist_alternate_names(handle, src_h, dest_h);
  }
  else {
    result = nodeupdown_get_list_alternate_names(handle, src_c, dest_c);
  }

  output_result(index, handle, result, return_value, return_errnum);

  if (function == GET_HOSTLIST_ALTERNATE_NAMES) {
    if (src_flag == IS_NOT_NULL)
      hostlist_destroy(src_h);
    if (dest_flag == IS_NOT_NULL)
      hostlist_destroy(dest_h);
  }
  else {
    if (src_flag == IS_NOT_NULL)
      free(src_c);
    if (dest_flag == IS_NOT_NULL)
      free(dest_c);
  }
}

/* run_nodelist_param_test
 * - run parameter tests for nodeupdown_nodelist_create(), 
 *   nodeupdown_nodelist_clear(), and nodeupdown_nodelist_destroy()
 */
void run_nodelist_param_test(struct test_env *test_env, 
			     int function, 
			     int index, 
			     nodeupdown_t handle_not_loaded,
			     nodeupdown_t handle_loaded,
			     int nodeupdown_handle, 
			     int list, 
			     int nodeupdown_load_data, 
			     int return_value, 
			     int return_errnum) {
  nodeupdown_t handle = NULL;
  char **ptr = NULL;
  char ***pptr = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    handle = handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = handle_loaded;
  }

  if (list == IS_NOT_NULL) {
    if (function == NODELIST_CREATE) {
      pptr = &ptr;
    }
    else {
      if ((ptr = (char **)malloc(sizeof(char *))) == NULL) {
	printf("malloc() error\n");
	return;
      }
    }
  }

  if (function == NODELIST_CREATE) {
    result = nodeupdown_nodelist_create(handle, pptr);
  }
  else if (function == NODELIST_CLEAR) {
    result = nodeupdown_nodelist_clear(handle, ptr);
  }
  else {
    result = nodeupdown_nodelist_destroy(handle, ptr);
  }

  output_result(index, handle, result, return_value, return_errnum);
	    
  if (list == IS_NOT_NULL) {
    free(ptr);
  }
  
}

/* run_param_tests
 * - run all parameter tests
 */
void run_param_tests(struct test_env *test_env) {
  int i;
  nodeupdown_t handle_not_loaded;
  nodeupdown_t handle_loaded;
  
  if (start_gmonds(test_env, NODE_ALL) == -1) {
    printf("INTERNAL ERROR: start_gmonds() error\n");
    exit(1);
  }

  /* sleep a bit, wait for gmonds to get going */
  sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);

  if ((handle_not_loaded = nodeupdown_create()) == NULL) {
    printf("INTERNAL ERROR: nodeupdown_create()\n");
    exit(1);
  }

  if ((handle_loaded = nodeupdown_create()) == NULL) {
    printf("INTERNAL ERROR: nodeupdown_create()\n");
    exit(1);
  }
  
  if (nodeupdown_load_data(handle_loaded, 
			   NULL, 
			   NULL, 
			   NULL, 
			   LIBNODEUPDOWN_TEST_GMOND_PORT) == -1) {
    printf("INTERNAL ERROR: nodeupdown_load_data()\n");
    exit(1);
  }

  i = 0;
  printf("nodeupdown_load_data(), parameter tests                       \n");
  printf("--------------------------------------------------------------\n");
  while (load_data_param_tests[i].nodeupdown_handle != -1) {
    run_load_data_param_test(test_env, 
			     i, 
			     load_data_param_tests[i].nodeupdown_handle, 
			     load_data_param_tests[i].genders_filename,
			     load_data_param_tests[i].gmond_hostname,
			     load_data_param_tests[i].gmond_ip,
			     load_data_param_tests[i].gmond_port,
			     load_data_param_tests[i].return_value, 
			     load_data_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_get_up_nodes_hostlist(), parameter tests           \n");
  printf("--------------------------------------------------------------\n");
  while (get_hostlist_param_tests[i].nodeupdown_handle != -1) {
    run_get_nodes_hostlist_param_test(test_env, 
				      GET_UP_NODES_HOSTLIST, 
				      i, 
				      handle_not_loaded, 
				      handle_loaded,
				      get_hostlist_param_tests[i].nodeupdown_handle, 
				      get_hostlist_param_tests[i].hostlist, 
				      get_hostlist_param_tests[i].nodeupdown_load_data, 
				      get_hostlist_param_tests[i].return_value, 
				      get_hostlist_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_get_down_nodes_hostlist(), parameter tests         \n");
  printf("--------------------------------------------------------------\n");
  while (get_hostlist_param_tests[i].nodeupdown_handle != -1) {
    run_get_nodes_hostlist_param_test(test_env, 
				      GET_DOWN_NODES_HOSTLIST, 
				      i, 
				      handle_not_loaded, 
				      handle_loaded,
				      get_hostlist_param_tests[i].nodeupdown_handle, 
				      get_hostlist_param_tests[i].hostlist, 
				      get_hostlist_param_tests[i].nodeupdown_load_data, 
				      get_hostlist_param_tests[i].return_value, 
				      get_hostlist_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_get_up_nodes_list(), parameter tests               \n");
  printf("--------------------------------------------------------------\n");
  while (get_list_param_tests[i].nodeupdown_handle != -1) {
    run_get_up_nodes_list_param_test(test_env, 
				     GET_UP_NODES_LIST, 
				     i, 
				     handle_not_loaded, 
				     handle_loaded,
				     get_list_param_tests[i].nodeupdown_handle, 
				     get_list_param_tests[i].list, 
				     get_list_param_tests[i].len, 
				     get_list_param_tests[i].nodeupdown_load_data, 
				     get_list_param_tests[i].return_value, 
				     get_list_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_get_down_nodes_list(), parameter tests             \n");
  printf("--------------------------------------------------------------\n");
  while (get_list_param_tests[i].nodeupdown_handle != -1) {
    run_get_up_nodes_list_param_test(test_env, 
				     GET_DOWN_NODES_LIST, 
				     i, 
				     handle_not_loaded, 
				     handle_loaded,
				     get_list_param_tests[i].nodeupdown_handle, 
				     get_list_param_tests[i].list,
				     get_list_param_tests[i].len,  
				     get_list_param_tests[i].nodeupdown_load_data, 
				     get_list_param_tests[i].return_value, 
				     get_list_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_is_node_up(), parameter tests                      \n");
  printf("--------------------------------------------------------------\n");
  while (is_node_param_tests[i].nodeupdown_handle != -1) {
    run_is_node_param_test(test_env, 
			   IS_NODE_UP, 
			   i, 
			   handle_not_loaded, 
			   handle_loaded,
			   is_node_param_tests[i].nodeupdown_handle, 
			   is_node_param_tests[i].node,
			   is_node_param_tests[i].nodeupdown_load_data, 
			   is_node_param_tests[i].return_value, 
			   is_node_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_is_node_down(), parameter tests                    \n");
  printf("--------------------------------------------------------------\n");
  while (is_node_param_tests[i].nodeupdown_handle != -1) {
    run_is_node_param_test(test_env, 
			   IS_NODE_DOWN, 
			   i, 
			   handle_not_loaded, 
			   handle_loaded,
			   is_node_param_tests[i].nodeupdown_handle, 
			   is_node_param_tests[i].node,
			   is_node_param_tests[i].nodeupdown_load_data, 
			   is_node_param_tests[i].return_value, 
			   is_node_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_get_hostlist_alternate_names(), parameter tests    \n");
  printf("--------------------------------------------------------------\n");
  while (alternate_param_tests[i].nodeupdown_handle != -1) {
    run_alternate_param_test(test_env, 
			     GET_HOSTLIST_ALTERNATE_NAMES, 
			     i, 
			     handle_not_loaded, 
			     handle_loaded,
			     alternate_param_tests[i].nodeupdown_handle, 
			     alternate_param_tests[i].src,
			     alternate_param_tests[i].dest,
			     alternate_param_tests[i].nodeupdown_load_data, 
			     alternate_param_tests[i].return_value, 
			     alternate_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_get_list_alternate_names(), parameter tests        \n");
  printf("--------------------------------------------------------------\n");
  while (alternate_param_tests[i].nodeupdown_handle != -1) {
    run_alternate_param_test(test_env, 
			     GET_LIST_ALTERNATE_NAMES, 
			     i, 
			     handle_not_loaded, 
			     handle_loaded,
			     alternate_param_tests[i].nodeupdown_handle, 
			     alternate_param_tests[i].src,
			     alternate_param_tests[i].dest,
			     alternate_param_tests[i].nodeupdown_load_data, 
			     alternate_param_tests[i].return_value, 
			     alternate_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");
  
  i = 0;
  printf("nodeupdown_nodelist_create(), parameter tests                 \n");
  printf("--------------------------------------------------------------\n");
  while (nodelist_param_tests[i].nodeupdown_handle != -1) {
    run_nodelist_param_test(test_env, 
			    NODELIST_CREATE, 
			    i, 
			    handle_not_loaded, 
			    handle_loaded,
			    nodelist_param_tests[i].nodeupdown_handle, 
			    nodelist_param_tests[i].list,
			    nodelist_param_tests[i].nodeupdown_load_data, 
			    nodelist_param_tests[i].return_value, 
			    nodelist_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_nodelist_clear(), parameter tests                  \n");
  printf("--------------------------------------------------------------\n");
  while (nodelist_param_tests[i].nodeupdown_handle != -1) {
    run_nodelist_param_test(test_env, 
			    NODELIST_CLEAR, 
			    i, 
			    handle_not_loaded, 
			    handle_loaded,
			    nodelist_param_tests[i].nodeupdown_handle, 
			    nodelist_param_tests[i].list,
			    nodelist_param_tests[i].nodeupdown_load_data, 
			    nodelist_param_tests[i].return_value, 
			    nodelist_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  i = 0;
  printf("nodeupdown_nodelist_destroy(), parameter tests                \n");
  printf("--------------------------------------------------------------\n");
  while (nodelist_param_tests[i].nodeupdown_handle != -1) {
    run_nodelist_param_test(test_env, 
			    NODELIST_DESTROY, 
			    i, 
			    handle_not_loaded, 
			    handle_loaded,
			    nodelist_param_tests[i].nodeupdown_handle, 
			    nodelist_param_tests[i].list,
			    nodelist_param_tests[i].nodeupdown_load_data, 
			    nodelist_param_tests[i].return_value, 
			    nodelist_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  if (nodeupdown_destroy(handle_not_loaded) == -1) {
    printf("INTERNAL ERROR: nodeupdown_destroy()\n");
    exit(1);
  }

  if (nodeupdown_destroy(handle_loaded) == -1) {
    printf("INTERNAL ERROR: nodeupdown_destroy()\n");
    exit(1);
  }

  if (close_gmonds(test_env, NODE_ALL) == -1) {
    printf("INTERNAL ERROR: close_gmonds() error\n");
    exit(1);
  }
}

int compare_nodes_to_hostlist_helper(struct test_env *test_env, 
				     hostlist_t hl, 
				     int a, 
				     int b, 
				     int c) {
  if (((hostlist_find(hl, test_env->nodes[0])) == a) &&
      ((hostlist_find(hl, test_env->nodes[1])) == b) &&
      ((hostlist_find(hl, test_env->nodes[2])) == c)) {
    return 1;
  }
  else {
    return 0;
  }
}

int compare_nodes_to_hostlist(struct test_env *test_env, 
			      hostlist_t hl, 
			      int nodes) {

  switch(nodes) {
  case NODE_NONE:
    if ((hostlist_find(hl, test_env->nodes[0])) >= 0 ||
	(hostlist_find(hl, test_env->nodes[1])) >= 0 ||
	(hostlist_find(hl, test_env->nodes[2])) >= 0) {
      return 0;
    }
    else {
      return 1;
    }
    break;
  case NODE_A:
    return compare_nodes_to_hostlist_helper(test_env, hl, 0, -1, -1);
    break;
  case NODE_B:
    return compare_nodes_to_hostlist_helper(test_env, hl, -1, 0, -1);
    break;
  case NODE_C:
    return compare_nodes_to_hostlist_helper(test_env, hl, -1, -1, 0);
    break;
  case NODE_AB:
    return compare_nodes_to_hostlist_helper(test_env, hl, 0, 1, -1);
    break;
  case NODE_AC:
    return compare_nodes_to_hostlist_helper(test_env, hl, 0, -1, 1);
    break;
  case NODE_BC:
    return compare_nodes_to_hostlist_helper(test_env, hl, -1, 0, 1);
    break;
  case NODE_ABC:
    return compare_nodes_to_hostlist_helper(test_env, hl, 0, 1, 2);
    break;
  default:
    printf("INTERNAL ERROR: incorrect nodes value in compare_nodes_to_hostlist()\n");
    return -1;
    break;
  }

  return -1;
}

void run_get_up_nodes_hostlist_functionality_tests(struct test_env *test_env) {
  int index = 0;
  nodeupdown_t handle = NULL;
  hostlist_t hl = NULL;
  char buffer[MAXBUFFERLEN];
  char *ptr = NULL;
  int result;

  printf("get_up_nodes_hostlist(), functionality tests             \n");
  printf("-----------------------------------------------------\n");

  while (get_up_f[index].nodes_up != -1) {
    handle = NULL;
    hl = NULL;
    ptr = NULL;

    memset(buffer, '\0', MAXBUFFERLEN);

    start_gmonds(test_env, get_up_f[index].nodes_up);

    sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);

    if ((handle = nodeupdown_create()) == NULL) {
      printf("INTERNAL ERROR: nodeupdown_create()\n");
      printf("EXITING get_up_nodes_hostlist() functionality tests\n");
      break;
    }

    if ((hl = hostlist_create(NULL)) == NULL) {
      printf("INTERNAL ERROR: hostlist_create()\n");
      printf("EXITING get_up_nodes_hostlist() functionality tests\n");
      break;
    }

    if (get_up_f[index].host_to_query == NODE_A)
      ptr = test_env->nodes[0];
    else if (get_up_f[index].host_to_query == NODE_B)
      ptr = test_env->nodes[1];
    else if (get_up_f[index].host_to_query == NODE_C)
      ptr = test_env->nodes[2];
    else
      ptr = NULL;

    if (nodeupdown_load_data(handle, NULL, ptr, NULL, LIBNODEUPDOWN_TEST_GMOND_PORT) == -1) {
      printf("INTERNAL ERROR: nodeupdown_load_data()\n");
      printf("EXITING get_up_nodes_hostlist() functionality tests\n");
      printf("%d\n", nodeupdown_errnum(handle));
      break;
    }

    /* run the test */

    result = nodeupdown_get_up_nodes_hostlist(handle, hl);
    
    if (result == get_up_f[index].return_value && nodeupdown_errnum(handle) == get_up_f[index].return_errnum) {
      
      if (compare_nodes_to_hostlist(test_env, hl, get_up_f[index].nodes_up)) {
	printf("Test %d: PASS\n", index);
      }
      else {
	hostlist_ranged_string(hl, MAXBUFFERLEN, buffer);
	printf("Test %d: ***FAIL*** returned hostlists = %s\n", index, buffer);
      }
    }
    else {
      printf("Test %d: ***FAIL*** return value = %d, return errnum = %d\n", index, result, nodeupdown_errnum(handle));
    }

    if (nodeupdown_destroy(handle) == -1) {
      printf("INTERNAL ERROR: nodeupdown_destroy()\n");
      printf("EXITING get_up_nodes_hostlist() functionality tests\n");
      break;
    }

    hostlist_destroy(hl);
  
    close_gmonds(test_env, get_up_f[index].nodes_up);

    sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);

    index++;
  }
  
  printf("\n\n");

}

int main(int argc, char **argv) {
  struct test_env *test_env = NULL;

  if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
    printf("signal() error\n");
    goto cleanup;
  }

  if ((test_env = (struct test_env *)malloc(sizeof(struct test_env))) == NULL) {
    printf("malloc() error\n");
    goto cleanup;
  }

  if (initialize_test_env(test_env) == -1) {
    printf("initialize_test_env() error\n"); 
    goto cleanup;
  }
  
  run_param_tests(test_env);

  /* run_get_up_nodes_hostlist_functionality_tests(test_env);  */

  cleanup_test_env(test_env);

  return 0;

 cleanup:

  if (test_env != NULL) {
    free(test_env);
  }

  return -1;
}
