/*
 * $Id: libnodeupdown_testcases.h,v 1.4 2003-03-08 00:13:57 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/testsuite/libnodeupdown_testcases.h,v $
 *    
 */

/*****************************************************************
 * DEFINITIONS                                                   * 
 *****************************************************************/

/* 
 * Function definitions
 */

#define GET_UP_NODES_HOSTLIST          0
#define GET_DOWN_NODES_HOSTLIST        1
#define GET_UP_NODES_LIST              2
#define GET_DOWN_NODES_LIST            3
#define IS_NODE_UP                     4
#define IS_NODE_DOWN                   5 
#define GET_HOSTLIST_ALTERNATE_NAMES   6 
#define GET_LIST_ALTERNATE_NAMES       7
#define NODELIST_CREATE                8 
#define NODELIST_CLEAR                 9 
#define NODELIST_DESTROY              10

/*
 * Flags for test cases
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

#define GOOD_PORT                                 8659
#define BAD_PORT                                  9999

#define BAD_NODE                       "foobar-fuhbar"
#define NULL_NODE                                 NULL

/*
 * Node Sets
 */
#define NODE_NONE              0
#define NODE_A                 4
#define NODE_B                 2
#define NODE_C                 1
#define NODE_AB                6
#define NODE_AC                5
#define NODE_BC                3
#define NODE_ABC               7
#define NODE_ALL               7
#define LOCALHOST              8

/*****************************************************************
 * PARAMETER TESTS                                               *
 * - for the time being, we will not execute any tests that      *
 *   "succeed", they are commented out.                          * 
 *****************************************************************/

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
  /* {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_OVERFLOW  }, */
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

/*****************************************************************
 * FUNCTIONALITY TESTS                                           *
 * - to limit the number of times gmonds are started, closed,    *
 *   and restarted, we organize the functionality tests in the   *
 *   mannger below.                                              *   
 *****************************************************************/

struct {
  int function;
  int nodes_up;
  int nodes_to_check;
  int host_to_query;
  int return_value;
  int return_errnum;
} func_tests[] = {
#if 0
  {GET_UP_NODES_HOSTLIST,   NODE_ALL, NODE_ALL,  LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_ALL, NODE_NONE, LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_ALL, NODE_ALL,  LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_ALL, NODE_NONE, LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_A,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_B,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_C,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_A,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_B,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_C,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_ALL, NODE_ALL,  NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_ALL, NODE_NONE, NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_ALL, NODE_ALL,  NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_ALL, NODE_NONE, NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_A,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_B,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_C,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_A,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_B,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_C,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_ALL, NODE_ALL,  NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_ALL, NODE_NONE, NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_ALL, NODE_ALL,  NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_ALL, NODE_NONE, NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_A,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_B,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_ALL, NODE_C,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_A,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_B,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_ALL, NODE_C,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_HOSTLIST,   NODE_A,   NODE_A,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_A,   NODE_BC,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_A,   NODE_A,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_A,   NODE_BC,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_A,   NODE_A,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_A,   NODE_B,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_A,   NODE_C,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_A,   NODE_A,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_A,   NODE_B,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_A,   NODE_C,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_A,   NODE_A,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_A,   NODE_BC,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_A,   NODE_A,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_A,   NODE_BC,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_A,   NODE_A,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_A,   NODE_B,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_A,   NODE_C,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_A,   NODE_A,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_A,   NODE_B,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_A,   NODE_C,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_B,   NODE_B,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_B,   NODE_AC,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_B,   NODE_B,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_B,   NODE_AC,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_B,   NODE_A,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_B,   NODE_B,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_B,   NODE_C,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_B,   NODE_A,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_B,   NODE_B,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_B,   NODE_C,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_AB,  NODE_AB,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_AB,  NODE_C,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_AB,  NODE_AB,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_AB,  NODE_C,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_A,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_B,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_C,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_A,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_B,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_C,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_AB,  NODE_AB,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_AB,  NODE_C,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_AB,  NODE_AB,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_AB,  NODE_C,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_A,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_B,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_C,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_A,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_B,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_C,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_AB,  NODE_AB,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_AB,  NODE_C,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_AB,  NODE_AB,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_AB,  NODE_C,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_A,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_B,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_AB,  NODE_C,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_A,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_B,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_AB,  NODE_C,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_BC,  NODE_BC,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_BC,  NODE_A,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_BC,  NODE_BC,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_BC,  NODE_A,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_BC,  NODE_A,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_BC,  NODE_B,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_BC,  NODE_C,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_BC,  NODE_A,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_BC,  NODE_B,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_BC,  NODE_C,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_HOSTLIST,   NODE_BC,  NODE_BC,   NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST, NODE_BC,  NODE_A,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,       NODE_BC,  NODE_BC,   NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,     NODE_BC,  NODE_A,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_BC,  NODE_A,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_BC,  NODE_B,    NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,              NODE_BC,  NODE_C,    NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_BC,  NODE_A,    NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_BC,  NODE_B,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,            NODE_BC,  NODE_C,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {-1, -1, -1, -1, -1, -1},
}; 

