/*
 * $Id: libnodeupdown_testcases.h,v 1.6 2003-03-11 23:04:54 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/testsuite/libnodeupdown_testcases.h,v $
 *    
 */

/*****************************************************************
 * DEFINITIONS                                                   * 
 *****************************************************************/

/* 
 * Function definitions
 */

#define GET_UP_NODES_STRING                 0 
#define GET_DOWN_NODES_STRING               1 
#define GET_UP_NODES_LIST                   2
#define GET_DOWN_NODES_LIST                 3
#define GET_UP_NODES_HOSTLIST               4
#define GET_DOWN_NODES_HOSTLIST             5
#define GET_UP_NODES_STRING_ALTNAMES        6 
#define GET_DOWN_NODES_STRING_ALTNAMES      7 
#define GET_UP_NODES_LIST_ALTNAMES          8 
#define GET_DOWN_NODES_LIST_ALTNAMES        9
#define GET_UP_NODES_HOSTLIST_ALTNAMES     10 
#define GET_DOWN_NODES_HOSTLIST_ALTNAMES   11 
#define IS_NODE_UP                         12 
#define IS_NODE_DOWN                       13 
#define CONVERT_STRING_TO_ALTNAMES         14 
#define CONVERT_LIST_TO_ALTNAMES           15 
#define CONVERT_HOSTLIST_TO_ALTNAMES       16
#define NODELIST_CREATE                    17
#define NODELIST_CLEAR                     18
#define NODELIST_DESTROY                   19

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
#define IS_POSITIVE_LARGE                        99999

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
 * Node Sets - using bit wise operators 
 */
#define NODE_NONE             0x00
#define NODE_A                0x04
#define NODE_B                0x02
#define NODE_C                0x01
#define NODE_AB               0x06
#define NODE_AC               0x05
#define NODE_BC               0x03
#define NODE_ABC              0x07
#define NODE_ALL              0x07
#define NODE_A_ALT            0x20
#define NODE_B_ALT            0x10
#define NODE_C_ALT            0x08
#define NODE_AB_ALT           0x30
#define NODE_AC_ALT           0x28
#define NODE_BC_ALT           0x18
#define NODE_ABC_ALT          0x38
#define NODE_ALL_ALT          0x38
#define LOCALHOST             0x40 

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

/* nodeupdown_get_up_nodes_string(), nodeupdown_get_down_nodes_string(),
 * nodeupdown_get_up_nodes_string_altnames(), and nodeupdown_get_down_nodes_string_altnames()
 * parameter tests
 */
struct {
  int nodeupdown_handle;
  int buf;
  int buflen;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} get_string_param_tests[] = {
  {IS_NULL,     IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_LOAD      },
  {IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_OVERFLOW  },
  {-1, -1, -1, -1, -1, -1},
};

/* nodeupdown_get_up_nodes_list(), nodeupdown_get_down_nodes_list(),
 * nodeupdown_get_up_nodes_list_altnames(), and nodeupdown_get_down_nodes_list_altnames()
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
  {IS_NULL,     IS_NULL,     IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_OVERFLOW  }, 
  {IS_NOT_NULL, IS_NULL,     IS_POSITIVE_LARGE, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_LARGE, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {-1, -1, -1, -1, -1, -1},
};

#ifdef NODEUPDOWN_HOSTLIST_API
/* nodeupdown_get_up_nodes_hostlist(), nodeupdown_get_down_nodes_hostlist(),
 * nodeupdown_get_up_nodes_hostlist_altnames(), and nodeupdown_get_down_nodes_hostlist_altnames()
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
#endif

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

/* nodeupdown_convert_string_to_altnames() parameter tests
 */
struct {
  int nodeupdown_handle;
  int src;
  int dest;
  int len;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} string_convert_param_tests[] = {
  {IS_NULL,     IS_NULL,     IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_LOAD      },
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  /* need to re-program libnoeupdown_test.c for this test case */
  /* {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_OVERFLOW  }, */
  {-1, -1, -1, -1, -1, -1, -1},
};

/* nodeupdown_convert_list_to_altnames() parameter tests
 */
struct {
  int nodeupdown_handle;
  int src;
  int dest;
  int len;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} list_convert_param_tests[] = {
  {IS_NULL,     IS_NULL,     IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NULL,     IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NULL,     IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NULL,     IS_NOT_NULL, IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NULL,     IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NO_ERRNUM                },
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_LARGE, DO_NOT_EXECUTE, -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NULL,     IS_POSITIVE_LARGE, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NULL,     IS_POSITIVE_LARGE, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NULL,     IS_NOT_NULL, IS_POSITIVE_LARGE, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_NEGATIVE,       EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_ZERO,           EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  /* need to re-program libnoeupdown_test.c for this test case */
  /* {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_SMALL, EXECUTE,        -1, NODEUPDOWN_ERR_OVERFLOW  }, */
  {IS_NOT_NULL, IS_NOT_NULL, IS_NOT_NULL, IS_POSITIVE_LARGE, EXECUTE,        -1, NODEUPDOWN_ERR_PARAMETERS},
  {-1, -1, -1, -1, -1, -1, -1},
};

#ifdef NODEUPDOWN_HOSTLIST_API
/* nodeupdown_convert_hostlist_to_altnames() parameter tests
 */
struct {
  int nodeupdown_handle;
  int src;
  int dest;
  int nodeupdown_load_data;
  int return_value;
  int return_errnum;
} hostlist_convert_param_tests[] = {
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
#endif

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
  {GET_UP_NODES_STRING,              NODE_ALL, NODE_ALL,      LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_ALL, NODE_NONE,     LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_ALL, NODE_ALL,      LOCALHOST, 3, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_ALL, NODE_NONE,     LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_ALL, NODE_ALL,      LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_ALL, NODE_NONE,     LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_ALL, NODE_ALL_ALT,  LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_ALL, NODE_NONE,     LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_ALL, NODE_ALL_ALT,  LOCALHOST, 3, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_ALL, NODE_NONE,     LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_ALL, NODE_ALL_ALT,  LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_ALL, NODE_NONE,     LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_ALL, NODE_A,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_B,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_C,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_A_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_B_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_C_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_A,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_B,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_C,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_A_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_B_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_C_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_ALL, NODE_ALL,      NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_ALL, NODE_NONE,     NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_ALL, NODE_ALL,      NODE_A,    3, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_ALL, NODE_NONE,     NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_ALL, NODE_ALL,      NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_ALL, NODE_NONE,     NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_ALL, NODE_ALL_ALT,  NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_ALL, NODE_NONE,     NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_ALL, NODE_ALL_ALT,  NODE_A,    3, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_ALL, NODE_NONE,     NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_ALL, NODE_ALL_ALT,  NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_ALL, NODE_NONE,     NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_ALL, NODE_A,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_B,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_C,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_A_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_B_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_C_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_A,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_B,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_C,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_A_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_B_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_C_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_ALL, NODE_ALL,      NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_ALL, NODE_NONE,     NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_ALL, NODE_ALL,      NODE_B,    3, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_ALL, NODE_NONE,     NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_ALL, NODE_ALL,      NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_ALL, NODE_NONE,     NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_ALL, NODE_ALL_ALT,  NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_ALL, NODE_NONE,     NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_ALL, NODE_ALL_ALT,  NODE_B,    3, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_ALL, NODE_NONE,     NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_ALL, NODE_ALL_ALT,  NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_ALL, NODE_NONE,     NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_ALL, NODE_A,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_B,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_C,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_A_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_B_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_ALL, NODE_C_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_A,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_B,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_C,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_A_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_B_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_ALL, NODE_C_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#if 0
  {GET_UP_NODES_STRING,              NODE_A,   NODE_A,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_A,   NODE_BC,       LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_A,   NODE_A,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_A,   NODE_BC,       LOCALHOST, 2, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_A,   NODE_A,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_A,   NODE_BC,       LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_A,   NODE_A_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_A,   NODE_BC_ALT,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_A,   NODE_A_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_A,   NODE_BC_ALT,   LOCALHOST, 2, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_A,   NODE_A_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_A,   NODE_BC_ALT,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_A,   NODE_A,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_B,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_C,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_A_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_B_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_C_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_A,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_B,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_C,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_A_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_B_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_C_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_A,   NODE_A,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_A,   NODE_BC,       NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_A,   NODE_A,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_A,   NODE_BC,       NODE_A,    2, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_A,   NODE_A,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_A,   NODE_BC,       NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_A,   NODE_A_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_A,   NODE_BC_ALT,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_A,   NODE_A_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_A,   NODE_BC_ALT,   NODE_A,    2, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_A,   NODE_A_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_A,   NODE_BC_ALT,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_A,   NODE_A,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_B,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_C,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_A_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_B_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_A,   NODE_C_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_A,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_B,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_C,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_A_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_B_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_A,   NODE_C_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_B,   NODE_B,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_B,   NODE_AC,       NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_B,   NODE_B,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_B,   NODE_AC,       NODE_B,    2, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_B,   NODE_B,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_B,   NODE_AC,       NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_B,   NODE_B_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_B,   NODE_AC_ALT,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_B,   NODE_B_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_B,   NODE_AC_ALT,   NODE_B,    2, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_B,   NODE_B_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_B,   NODE_AC_ALT,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_B,   NODE_A,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_B,   NODE_B,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_B,   NODE_C,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_B,   NODE_A_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_B,   NODE_B_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_B,   NODE_C_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_B,   NODE_A,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_B,   NODE_B,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_B,   NODE_C,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_B,   NODE_A_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_B,   NODE_B_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_B,   NODE_C_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_AB,  NODE_AB,       LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_AB,  NODE_C,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_AB,  NODE_AB,       LOCALHOST, 2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_AB,  NODE_C,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_AB,  NODE_AB,       LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_AB,  NODE_C,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_AB,  NODE_AB_ALT,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_AB,  NODE_C_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_AB,  NODE_AB_ALT,   LOCALHOST, 2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_AB,  NODE_C_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_AB,  NODE_AB_ALT,   LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_AB,  NODE_C_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_AB,  NODE_A,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_B,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_C,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_A_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_B_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_C_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_A,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_B,        LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_C,        LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_A_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_B_ALT,    LOCALHOST, 0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_C_ALT,    LOCALHOST, 1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_AB,  NODE_AB,       NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_AB,  NODE_C,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_AB,  NODE_AB,       NODE_A,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_AB,  NODE_C,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_AB,  NODE_AB,       NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_AB,  NODE_C,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_AB,  NODE_AB_ALT,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_AB,  NODE_C_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_AB,  NODE_AB_ALT,   NODE_A,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_AB,  NODE_C_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_AB,  NODE_AB_ALT,   NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_AB,  NODE_C_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_AB,  NODE_A,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_B,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_C,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_A_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_B_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_C_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_A,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_B,        NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_C,        NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_A_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_B_ALT,    NODE_A,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_C_ALT,    NODE_A,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_AB,  NODE_AB,       NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_AB,  NODE_C,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_AB,  NODE_AB,       NODE_B,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_AB,  NODE_C,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_AB,  NODE_AB,       NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_AB,  NODE_C,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_AB,  NODE_AB_ALT,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_AB,  NODE_C_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_AB,  NODE_AB_ALT,   NODE_B,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_AB,  NODE_C_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_AB,  NODE_AB_ALT,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_AB,  NODE_C_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_AB,  NODE_A,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_B,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_C,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_A_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_B_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_AB,  NODE_C_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_A,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_B,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_C,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_A_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_B_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_AB,  NODE_C_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_BC,  NODE_BC,       NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_BC,  NODE_A,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_BC,  NODE_BC,       NODE_B,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_BC,  NODE_A,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_BC,  NODE_BC,       NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_BC,  NODE_A,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_BC,  NODE_BC_ALT,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_BC,  NODE_A_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_BC,  NODE_BC_ALT,   NODE_B,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_BC,  NODE_A_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_BC,  NODE_BC_ALT,   NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_BC,  NODE_A_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_BC,  NODE_A,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_B,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_C,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_A_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_B_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_C_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_A,        NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_B,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_C,        NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_A_ALT,    NODE_B,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_B_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_C_ALT,    NODE_B,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_STRING,              NODE_BC,  NODE_BC,       NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING,            NODE_BC,  NODE_A,        NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST,                NODE_BC,  NODE_BC,       NODE_C,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST,              NODE_BC,  NODE_A,        NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST,            NODE_BC,  NODE_BC,       NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST,          NODE_BC,  NODE_A,        NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {GET_UP_NODES_STRING_ALTNAMES,     NODE_BC,  NODE_BC_ALT,   NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_STRING_ALTNAMES,   NODE_BC,  NODE_A_ALT,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_UP_NODES_LIST_ALTNAMES,       NODE_BC,  NODE_BC_ALT,   NODE_C,    2, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_LIST_ALTNAMES,     NODE_BC,  NODE_A_ALT,    NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
#ifdef NODEUPDOWN_HOSTLIST_API
  {GET_UP_NODES_HOSTLIST_ALTNAMES,   NODE_BC,  NODE_BC_ALT,   NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {GET_DOWN_NODES_HOSTLIST_ALTNAMES, NODE_BC,  NODE_A_ALT,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {IS_NODE_UP,                       NODE_BC,  NODE_A,        NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_B,        NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_C,        NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_A_ALT,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_B_ALT,    NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_UP,                       NODE_BC,  NODE_C_ALT,    NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_A,        NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_B,        NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_C,        NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_A_ALT,    NODE_C,    1, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_B_ALT,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
  {IS_NODE_DOWN,                     NODE_BC,  NODE_C_ALT,    NODE_C,    0, NODEUPDOWN_ERR_SUCCESS},
#endif
  {-1, -1, -1, -1, -1, -1},
}; 

