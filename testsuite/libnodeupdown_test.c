/*
 * $Id: libnodeupdown_test.c,v 1.16 2003-05-21 21:17:31 achu Exp $
 * $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/testsuite/libnodeupdown_test.c,v $
 *    
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
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

#include "hostlist.h"
#include "nodeupdown.h"
#include "libnodeupdown_testcases.h"

/*****************************************************************
 * DEFINITIONS                                                   * 
 *****************************************************************/

/*
 * Test Suite definitions
 */
#define LIBNODEUPDOWN_TEST_CONF             "libnodeupdown_test.conf"
#define LIBNODEUPDOWN_TEST_MAXNODES                                 3
#define LIBNODEUPDOWN_TEST_SLEEPTIME                               30
#define MAXBUFFERLEN                                             4096

/* struct test_env
 * - stores information about the test environment
 */
struct test_env {
  
  /* information from the configuration file */
  char *pdsh;
  char *pkill;
  char *gmond;
  char *conffile;
  char **nodes; 

  /* static information.  See initialize_test_env() for details */
  char *conf;
  char *nodes_ab;
  char *nodes_ac;
  char *nodes_bc;
  char *nodes_abc;
  
  /* test environment for individual tests */
  nodeupdown_t handle_not_loaded;
  nodeupdown_t handle_loaded;
  nodeupdown_t handle_destroyed;
  char *string;
  char **list;
};

/*****************************************************************
 * FUNCTION DECLARATIONS                                         *
 *****************************************************************/

void usage();
int read_and_store_line_from_file(FILE *, char **);
int concatenate_two_strings(char **, char *, char *);
int concatenate_three_strings(char **, char *, char *, char *);
int initialize_test_env(struct test_env *);
void cleanup_test_env(struct test_env *);
int compare_nodes_to_string(struct test_env *, char *, int);
int compare_nodes_to_list_helper(struct test_env *, char **, int, int, int, char *);
int compare_nodes_to_list(struct test_env *, char **, int, int);
int compare_nodes_to_hostlist_helper(hostlist_t, int, int, char *); 
int compare_nodes_to_hostlist(struct test_env *, hostlist_t, int);
int map_nodes_to_char_ptr(struct test_env *, int, char **);
int start_gmonds(struct test_env *, int);
int close_gmonds(struct test_env *, int);
int initialize_test_env_parameter_tests(struct test_env *);
int cleanup_test_env_parameter_tests(struct test_env *);
int run_a_test(struct test_env *, int, int, int, int, int, int, int, int, int, int);  
int run_param_tests(struct test_env *);
void func_test(struct test_env *, int, int, int, int, int, int);
int initialize_test_env_func_tests(struct test_env *, int, int, int);
int cleanup_test_env_func_tests(struct test_env *, int, int);
int run_func_tests(struct test_env *);

/*****************************************************************
 * FUNCTION                                                     *
 *****************************************************************/

void usage() {
  printf("libnodeupdown_test <int>\n");
  printf("   int - 0 to run parameter tests\n");
  printf("       - 1 to run functionality tests\n");
  exit(1);
}

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

/* concatenate_two_strings
 * - concatenate two strings with a comma in the middle
 */
int concatenate_two_strings(char **ptr, char *str1, char *str2) {
  if ((*ptr = (char *)malloc((MAXHOSTNAMELEN + 1)*2)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(*ptr, '\0', (MAXHOSTNAMELEN + 1)*2);
  strcpy(*ptr, str1);
  strcat(*ptr, ",");
  strcat(*ptr, str2);

  return 0;
}

/* concatenate_three_strings
 * - concatenate three strings with comma separation
 */
int concatenate_three_strings(char **ptr, char *str1, char *str2, char *str3) {
  if ((*ptr = (char *)malloc((MAXHOSTNAMELEN + 1)*3)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(*ptr, '\0', (MAXHOSTNAMELEN + 1)*3);
  strcpy(*ptr, str1);
  strcat(*ptr, ",");
  strcat(*ptr, str2);
  strcat(*ptr, ",");
  strcat(*ptr, str3);

  return 0; 
}

/* initialize_test_env
 * - initialize the test environment for the entire program
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
   * just make them one time and save them in the test environment
   */
  
  if ((test_env->conf = 
       (char *)malloc(strlen("--conf=") + strlen(test_env->conffile) + 1)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }
  memset(test_env->conf, '\0', strlen("--conf=") + strlen(test_env->conffile) + 1);
  strcpy(test_env->conf, "--conf=");
  strcat(test_env->conf, test_env->conffile);

  if (concatenate_two_strings(&test_env->nodes_ab, 
			      test_env->nodes[0], test_env->nodes[1]) == -1) {
    printf("concatenate_two_strings() error\n");
    return -1;
  }
  if (concatenate_two_strings(&test_env->nodes_ac, 
			      test_env->nodes[0], test_env->nodes[2]) == -1) {
    printf("concatenate_two_strings() error\n");
    return -1;
  }
  if (concatenate_two_strings(&test_env->nodes_bc, 
			      test_env->nodes[1], test_env->nodes[2]) == -1) {
    printf("concatenate_two_strings() error\n");
    return -1;
  }
  if (concatenate_three_strings(&test_env->nodes_abc, 
				test_env->nodes[0], 
				test_env->nodes[1], 
				test_env->nodes[2]) == -1) {
    printf("concatenate_three_strings() error\n");
    return -1;
  } 
  
  fclose(fp);
  return 0;

 cleanup:

  fclose(fp);
  return -1;
}

/* cleanup_test_env
 * - cleanup the struct test_env structure 
 */
void cleanup_test_env(struct test_env *test_env) {
  int i;

  free(test_env->pdsh);
  free(test_env->pkill);
  free(test_env->gmond);
  free(test_env->conffile);
  
  if (test_env->nodes != NULL) {
    for (i = 0; i < LIBNODEUPDOWN_TEST_MAXNODES; i++)
	free(test_env->nodes[i]);
    free(test_env->nodes);
  }

  free(test_env->conf);
  free(test_env->nodes_ab);
  free(test_env->nodes_ac);
  free(test_env->nodes_bc);
  free(test_env->nodes_abc);

  return;
}

/* compare_nodes_to_string
 * - compare the set of nodes to the nodes indicated in the string.  
 *   Whatever nodes are indicated should be in the string, and whatever 
 *   nodes are not indicated must not be in the string.
 * - returns 1 if nodes and list match, 0 if not, -1 on error.
 */ 
int compare_nodes_to_string(struct test_env *test_env, 
			    char *str,
			    int nodes) {
  int retval;
  hostlist_t hl;

  if ((hl = hostlist_create(str)) == NULL) {
    printf("hostlist_create() error\n");
    return -1;
  }

  retval = compare_nodes_to_hostlist(test_env, hl, nodes);

  hostlist_destroy(hl);

  return retval;
}

/* compare_nodes_to_list_helper
 * - helper for compare_nodes_to_list, handles the common
 *   operation of the function.  Checks if 
 *   'checknode' is in the list or not.
 */
int compare_nodes_to_list_helper(struct test_env *test_env,
				 char **list,
				 int len,
				 int nodes,
				 int node_to_check,
				 char *checknode) {

  int i, retval = 1;

  if (nodes & node_to_check) {
    int flag = 0;
    for (i = 0; i < len; i++) {
      if (strcmp(list[i], checknode) == 0) {
	flag = 1;
	break;
      }
    }
    if (flag == 0)
      retval = 0;
  }
  else {
    for (i = 0; i < len; i++) {
      if (strcmp(list[i],checknode) == 0) {
	retval = 0;
	break;
      }
    }
  }

  return retval;
}

/* compare_nodes_to_list
 * - compare the set of nodes to the list.  Whatever nodes
 *   are indicated should be in the list, and whatever nodes
 *   are not indicated must not be in the list.
 * - returns 1 if nodes and list match, 0 if not.
 */ 
int compare_nodes_to_list(struct test_env *test_env, 
			  char **list,
			  int len,
			  int nodes) {
  int retval = 1;

  retval = compare_nodes_to_list_helper(test_env, list, len, nodes, 
					NODE_A, test_env->nodes[0]);
  retval = compare_nodes_to_list_helper(test_env, list, len, nodes,
					NODE_B, test_env->nodes[1]);
  retval = compare_nodes_to_list_helper(test_env, list, len, nodes,
					NODE_C, test_env->nodes[2]);
  return retval;
}

/* compare_nodes_to_hostlist_helper
 * - handles common operation of compare_nodes_to_hostlist
 * - checks is "node_to_check" is in the hostlist
 */
int compare_nodes_to_hostlist_helper(hostlist_t hl, 
				     int nodes, 
				     int node_to_check, 
				     char *nodename) {
  int retval = 1;

  if (nodes & node_to_check) {
    if (hostlist_find(hl, nodename) == -1) 
      retval = 0;
  }
  else {
    if (hostlist_find(hl, nodename) >= 0)
      retval = 0;
  }

  return retval;
}

/* compare_nodes_to_hostlist
 * - compare the set of nodes to the hostlist.  Whatever nodes
 *   are indicated should be in the hostlist, and whatever nodes
 *   are not indicated must not be in the hostlist.
 * - returns 1 if nodes and hostlist match, 0 if not.
 */
int compare_nodes_to_hostlist(struct test_env *test_env, 
			      hostlist_t hl, 
			      int nodes) {
  int retval = 1;

  retval = compare_nodes_to_hostlist_helper(hl, nodes, NODE_A, test_env->nodes[0]);
  retval = compare_nodes_to_hostlist_helper(hl, nodes, NODE_B, test_env->nodes[1]);
  retval = compare_nodes_to_hostlist_helper(hl, nodes, NODE_C, test_env->nodes[2]);

  return retval;
}

/* map_nodes_to_char_ptr
 * - maps one of the node sets to a comma separated list of the nodes
 *   stored in the test_env;
 */
int map_nodes_to_char_ptr(struct test_env *test_env, int nodes, char **ptr) {

  switch(nodes) {
  case NODE_A:          *ptr = test_env->nodes[0];        break;
  case NODE_B:          *ptr = test_env->nodes[1];        break;
  case NODE_C:          *ptr = test_env->nodes[2];        break;
  case NODE_AB:         *ptr = test_env->nodes_ab;        break;
  case NODE_AC:         *ptr = test_env->nodes_ac;        break;
  case NODE_BC:         *ptr = test_env->nodes_bc;        break;
  case NODE_ABC:        *ptr = test_env->nodes_abc;       break;
  case LOCALHOST:       *ptr = NULL;                      break;
  default:
    printf("INTERNAL ERROR: incorrect nodes value in map_nodes_to_char_ptr()\n");
    return -1;
    break;
  }

  return 0;
}

/* start_gmonds
 * - run gmond on the specified nodes using pdsh 
 */
int start_gmonds(struct test_env *test_env, int nodes) {
  int pid, ret;
  char *ptr;

  pid = fork();
  if (pid == 0) { /* child */
    if (map_nodes_to_char_ptr(test_env, nodes, &ptr) == -1) {
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
  else if (pid > 0) { /* parent */
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
 * - close gmond on the specified nodes using pdsh and pkill
 */
int close_gmonds(struct test_env *test_env, int nodes) {
  int pid, ret;
  char *ptr;

  pid = fork();
  if (pid == 0) { /* child */
    if (map_nodes_to_char_ptr(test_env, nodes, &ptr) == -1) {
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
  else if (pid > 0) { /* parent */
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

/* initialize_test_env_parameter_tests
 * - initialize environment for parameter tests
 */
int initialize_test_env_parameter_tests(struct test_env *test_env) {
  test_env->handle_not_loaded = NULL;
  test_env->handle_loaded = NULL;
  test_env->string = NULL;
  test_env->list = NULL;

  /* kill existing gmonds first */
  if (close_gmonds(test_env, NODE_ALL) == -1) {
    printf("close_gmonds() error\n");
    return -1;
  }

  if (start_gmonds(test_env, NODE_ALL) == -1) {
    printf("start_gmonds() error\n");
    return -1;
  }

  /* sleep a bit, wait for gmonds to get going */
  sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);

  if ((test_env->handle_not_loaded = nodeupdown_handle_create()) == NULL) {
    printf("nodeupdown_handle_create() error\n");
    return -1;
  }

  if ((test_env->handle_loaded = nodeupdown_handle_create()) == NULL) {
    printf("nodeupdown_handle_create() error\n");
    return -1;
  }

  if (nodeupdown_load_data(test_env->handle_loaded, 
			   NULL, NULL, NULL, 0, 0) == -1) {
    printf("nodeupdown_load_data() error: %s\n",
	   nodeupdown_strerror(nodeupdown_errnum(test_env->handle_loaded))); 
    return -1;
  }
  
  if ((test_env->handle_destroyed = nodeupdown_handle_create()) == NULL) {
    printf("nodeupdown_handle_create() error\n");
    return -1;
  }

  if (nodeupdown_handle_destroy(test_env->handle_destroyed) == -1) {
    printf("nodeupdown_handle_destroy() error\n");
    return -1;
  }

  if ((test_env->string = (char *)malloc(MAXHOSTNAMELEN+1)) == NULL) {
    printf("malloc() error\n");
    return -1;
  }

  if (nodeupdown_nodelist_create(test_env->handle_loaded, 
				 &test_env->list) == -1) {
    printf("nodeupdown_nodelist_create() error\n");
    return -1;
  }

  return 0;
}

/* cleanup_test_env_parameter_tests
 * - cleanup test_env for the parameter tests
 */
int cleanup_test_env_parameter_tests(struct test_env *test_env) {
  int retval = 0;

  if (test_env->string != NULL) {
    free(test_env->string);
  }

  if (test_env->list != NULL) {
    if (nodeupdown_nodelist_destroy(test_env->handle_loaded, 
				    test_env->list) == -1) {
      printf("nodeupdown_nodelist_destroy() error\n");
      retval = -1;
    }
  }
  
  if (test_env->handle_not_loaded != NULL) {
    if (nodeupdown_handle_destroy(test_env->handle_not_loaded) == -1) {
      printf("nodeupdown_handle_destroy() error\n");
      retval = -1;
    }
  }

  if (test_env->handle_loaded != NULL) {
    if (nodeupdown_handle_destroy(test_env->handle_loaded) == -1) {
      printf("nodeupdown_handle_destroy() error\n");
      retval = -1;
    }
  }

  if (close_gmonds(test_env, NODE_ALL) == -1) {
    printf("close_gmonds() error\n");
    retval = -1;
  }

  return retval;
}

/* run_a_test
 * - run a single parameter test
 */
void run_a_test(struct test_env *test_env, 
                int index, 
                int function, 
                int param1, 
                int param2, 
                int param3, 
                int param4, 
                int param5, 
                int param6, 
                int return_value, 
                int return_errnum) {
  int result;
  nodeupdown_t handle = NULL;
  char *genders_filename = NULL;
  char *gmond_hostname = NULL;
  char *gmond_ip = NULL;
  char **ptr = NULL;
  char ***pptr = NULL;
  char **list = NULL;
  char *node = NULL;
  char *buf = NULL;
  
  if (param1 == IS_NOT_NULL_LOADED)
    handle = test_env->handle_loaded;
  else if (param1 == IS_NOT_NULL_NOT_LOADED) {
    if (function != LOAD_DATA)
      handle = test_env->handle_not_loaded;
    else {
      /* can't use test_env->handle_not_loaded b/c
       * nodeupdown_load_data() may clear or set data
       */
      if ((handle = nodeupdown_handle_create()) == NULL) {
        printf("Test %d, %s: nodeupdown_handle_create() error\n", 
               index,
               function_names[parameter_tests[index].function & FUNCTION_NAME]);
        return;
      }
    }
  }
  else if (param1 == IS_NOT_NULL_DESTROYED)
    handle = test_env->handle_destroyed;      
  
  if (function == LOAD_DATA) {
    if (param2 == IS_NOT_NULL_GOOD)
      genders_filename = GOOD_FILENAME;
    else if (param2 == IS_NOT_NULL_BAD)
      genders_filename = BAD_FILENAME;

    if (param3 == IS_NOT_NULL_GOOD)
      gmond_hostname = GOOD_HOSTNAME;
    else if (param3 == IS_NOT_NULL_BAD)
      gmond_hostname = BAD_HOSTNAME;

    if (param4 == IS_NOT_NULL_GOOD)
      gmond_ip = GOOD_IP;
    else if (param4 == IS_NOT_NULL_BAD)
      gmond_ip = BAD_IP;
  }

  if (function & IS_NODELIST_FUNCTION && param2 == IS_NOT_NULL) {
    if (function == NODELIST_CREATE)
      pptr = &ptr;
    else
      ptr = test_env->list;
  }

  if (function & IS_LIST_FUNCTION && param2 == IS_NOT_NULL)
    list = test_env->list;

  if (function & IS_IS_FUNCTION && param2 == IS_NOT_NULL_BAD)
    node = BAD_NODE;

  if (function & IS_STRING_FUNCTION && param2 == IS_NOT_NULL)
    buf = test_env->string;

  if (function == HANDLE_DESTROY)
    result = nodeupdown_handle_destroy(handle);
  else if (function == LOAD_DATA)
    result = nodeupdown_load_data(handle, 
                                  genders_filename, 
                                  gmond_hostname, 
                                  gmond_ip, 
                                  param5,
                                  param6);
  else if (function == DUMP)
    result = nodeupdown_dump(handle, NULL);
  else if (function == GET_UP_NODES_STRING)
    result = nodeupdown_get_up_nodes_string(handle, buf, param3);
  else if (function == GET_DOWN_NODES_STRING)
    result = nodeupdown_get_down_nodes_string(handle, buf, param3);
  else if (function == GET_UP_NODES_LIST)
    result = nodeupdown_get_up_nodes_list(handle, list, param3);
  else if (function == GET_DOWN_NODES_LIST)
    result = nodeupdown_get_down_nodes_list(handle, list, param3);
  else if (function == IS_NODE_UP)
    result = nodeupdown_is_node_up(handle, node);
  else if (function == IS_NODE_DOWN)
    result = nodeupdown_is_node_down(handle, node);
  else if (function == NODELIST_CREATE)
    result = nodeupdown_nodelist_create(handle, pptr);
  else if (function == NODELIST_CLEAR)
    result = nodeupdown_nodelist_clear(handle, ptr);
  else if (function == NODELIST_DESTROY)
    result = nodeupdown_nodelist_destroy(handle, ptr);

  if (result == return_value && nodeupdown_errnum(handle) == return_errnum)
    printf("Test %d, %s: PASS\n", 
           index,
           function_names[parameter_tests[index].function & FUNCTION_NAME]);
  else
    printf("Test %d, %s: ***FAIL*** return value = %d, return errnum = %d\n", 
           index, 
           function_names[parameter_tests[index].function & FUNCTION_NAME], 
           result, 
           nodeupdown_errnum(handle));

  if (function == LOAD_DATA && param1 == IS_NOT_NULL_NOT_LOADED)
    nodeupdown_handle_destroy(handle);
}

/* run_param_tests
 * - run all parameter tests
 */
int run_param_tests(struct test_env *test_env) {
  int i;
  
  if (initialize_test_env_parameter_tests(test_env) == -1) {
    printf("initialize_test_env_parameter_tests() error\n");
    goto cleanup;
  }

  i = 0;
  while (parameter_tests[i].function != -1) {

    run_a_test(test_env, i, 
               parameter_tests[i].function,
               parameter_tests[i].param1,
               parameter_tests[i].param2,
               parameter_tests[i].param3,
               parameter_tests[i].param4,
               parameter_tests[i].param5,
               parameter_tests[i].param6,
               parameter_tests[i].return_value,
               parameter_tests[i].return_errnum);
    i++;
  }

  if (cleanup_test_env_parameter_tests(test_env) == -1) {
    printf("cleanup_test_env_parameter_tests() error\n");
    goto cleanup;
  }

  return 0;
  
 cleanup:
  
  (void)cleanup_test_env_parameter_tests(test_env);
  
  return -1;
}

/* func_test
 * - the main tester for functionality tests
 */
void func_test(struct test_env *test_env,
               int function,
               int index,
               int nodes_to_check,
               int host_to_query,
               int return_value,
               int return_errnum) {
  int result, i;
  char **list;
  char *node;
  int len;

  if (function & IS_STRING_FUNCTION) {
    if ((node = (char *)malloc(MAXBUFFERLEN)) == NULL) {
      printf("malloc() error\n");
      return;
    }
  }  
  else if (function & IS_LIST_FUNCTION) {
    if ((len = nodeupdown_nodelist_create(test_env->handle_loaded, &list)) == -1) {
      printf("Test %d: nodeupdown_nodelist_create() error\n", index);
      return;
    }
  }
  else {
    if (map_nodes_to_char_ptr(test_env, nodes_to_check, &node) == -1) {
      printf("Test %d: map_nodes_to_char_ptr() error\n", index);
      return;
    }
  }

  if (function == GET_UP_NODES_STRING)
    result = nodeupdown_get_up_nodes_string(test_env->handle_loaded, node, MAXBUFFERLEN);
  else if (function == GET_DOWN_NODES_STRING)
    result = nodeupdown_get_down_nodes_string(test_env->handle_loaded, node, MAXBUFFERLEN);
  else if (function == GET_UP_NODES_LIST)
    result = nodeupdown_get_up_nodes_list(test_env->handle_loaded, list, len);
  else if (function == GET_DOWN_NODES_LIST) {
    result = nodeupdown_get_down_nodes_list(test_env->handle_loaded, list, len); 

    /* FIX LATER - b/c test suite doesn't account for extra nodes in the cluster */
    if (result != -1) 
      result = return_value;
  }
  else if (function == IS_NODE_UP)
    result = nodeupdown_is_node_up(test_env->handle_loaded, node);
  else if (function == IS_NODE_DOWN)
    result = nodeupdown_is_node_down(test_env->handle_loaded, node);
  
  if (result == return_value && 
      nodeupdown_errnum(test_env->handle_loaded) == return_errnum) {
    if (function & IS_STRING_FUNCTION) {
      if (compare_nodes_to_string(test_env, node, nodes_to_check))
	printf("Test %d: PASS\n", index);
      else
	printf("Test %d: ***FAIL*** returned nodes %s\n", index, node);
    }
    else if (function & IS_LIST_FUNCTION) {
      if (compare_nodes_to_list(test_env, list, len, nodes_to_check))
	printf("Test %d: PASS\n", index);
      else {
	printf("Test %d: ***FAIL*** returned nodes ", index);
	i = 0;
	while (strlen(list[i]) > 0 && i < len) {
	  if (list[i] != NULL) 
	    printf("%s ", list[i++]);
	}
	printf("\n");
      }
    }
    else
      printf("Test %d: PASS\n", index);
  }
  else
    printf("Test %d: ***FAIL*** return value = %d, return errnum = %d\n", 
	   index, result, nodeupdown_errnum(test_env->handle_loaded));

  if (function & IS_STRING_FUNCTION)
    free(node);
  else if (function & IS_LIST_FUNCTION)
    (void)nodeupdown_nodelist_destroy(test_env->handle_loaded, list);
}

/* initialize_test_env_func_tests
 * - setup test environment for functionality tests
 */
int initialize_test_env_func_tests(struct test_env *test_env, 
			     int nodes, 
			     int host_to_query, 
			     int restart_gmonds_flag) {
  char * ptr;

  test_env->handle_loaded = NULL;

  /* don't restart gmonds if we don't have to */
  if (restart_gmonds_flag == 1) {
    if (start_gmonds(test_env, nodes) == -1) {
      printf("start_gmonds() error\n");
      return -1;
    }

    /* sleep a bit, wait for gmonds to get going */
    sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);
  }

  if ((test_env->handle_loaded = nodeupdown_handle_create()) == NULL) {
    printf("nodeupdown_handle_create() error\n");
    return -1;
  }

  if (map_nodes_to_char_ptr(test_env, host_to_query, &ptr) == -1) {
    printf("map_nodes_to_char_ptr() error\n");
    return -1;
  }
  
  if (nodeupdown_load_data(test_env->handle_loaded, 
			   NULL, ptr, NULL, 0, 0) == -1) {
    printf("nodeupdown_load_data() error: %s\n", 
	   nodeupdown_strerror(nodeupdown_errnum(test_env->handle_loaded)));
    return -1;
  }

  return 0;
}

/* cleanup_test_env_func_tests
 * - cleanup functionality test environment
 */
int cleanup_test_env_func_tests(struct test_env *test_env, 
				int nodes, 
				int restart_gmonds_flag) {
  int retval = 0;

  if (test_env->handle_loaded != NULL) {
    if (nodeupdown_handle_destroy(test_env->handle_loaded) == -1) {
      printf("nodeupdown_handle_destroy() error\n");
      retval = -1;
    }
  }

  /* don't restart gmonds if we don't have to */
  if (restart_gmonds_flag == 1) {
    if (close_gmonds(test_env, nodes) == -1) {
      printf("close_gmonds() error\n");
      retval = -1;
    }
  }

  return retval;
}

/* run_func_tests
 * - run all of the functionality tests
 */
int run_func_tests(struct test_env *test_env) {
  int i, restart_gmonds_flag, old_nodes_up, old_host_to_query;

  /* kill existing gmonds first */
  if (close_gmonds(test_env, NODE_ALL) == -1) {
    printf("close_gmonds() error\n");
    return -1;
  }

  /* we use the "restart_gmonds_flag" variable to minimize the number
   * of times gmonds must be closed and started up again.  if
   * restart_gmonds_flag = 1, restart, else don't restart.
   */

  i = 0;
  restart_gmonds_flag = 1;
  while(func_tests[i].function != -1) {
    if (initialize_test_env_func_tests(test_env, 
				       func_tests[i].nodes_up, 
				       func_tests[i].host_to_query,
				       restart_gmonds_flag) == -1) {
      printf("initialize_test_env_func_tests() error\n");
      return -1;
    }
    
    old_nodes_up = func_tests[i].nodes_up;
    old_host_to_query = func_tests[i].host_to_query;

    /* if the "host_to_query" or "nodes_up" in the test cases changes,
     * we have re-initialize the environment.  A change in nodes_up,
     * requires us to close and restart a new set of gmonds.  A change
     * in host_to_query requires us to re-call nodeupdown_load_data().
     */
    while (func_tests[i].function != -1 &&
	   func_tests[i].nodes_up == old_nodes_up &&
	   func_tests[i].host_to_query == old_host_to_query) {
      func_test(test_env, 
		func_tests[i].function,
		i, 
		func_tests[i].nodes_to_check,
		func_tests[i].host_to_query,
		func_tests[i].return_value, 
		func_tests[i].return_errnum);  
      i++;
      sleep(1);
    }

    /* check if we have to restart gmonds?? */
    if (func_tests[i].nodes_up != old_nodes_up)
      restart_gmonds_flag = 1;
    else
      restart_gmonds_flag = 0;

    if (cleanup_test_env_func_tests(test_env, 
				    old_nodes_up, 
				    restart_gmonds_flag) == -1) {
      printf("cleanup_test_env_func_tests() error\n");
      return -1;
    }

    if (restart_gmonds_flag == 1) {
      /* sleep a bit, wait for gmonds to close */
      sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  struct test_env *test_env = NULL;
  int which_test_type;

  if (argc != 2)
    usage();
  else {
    which_test_type = atoi(argv[1]);
    if (which_test_type != 0 && which_test_type != 1)
      usage();
  }

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
  
  if (which_test_type == 0) {
    if (run_param_tests(test_env) == -1) {
      printf("run_param_tests() error\n");
      goto cleanup;
    }
  }
  else {
    if (run_func_tests(test_env) == -1) {
      printf("func_tests() error\n");
      goto cleanup;
    }
  }

  cleanup_test_env(test_env);
  free(test_env);

  return 0;

 cleanup:

  free(test_env);
  return -1;
}
