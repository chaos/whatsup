/*
 * $Id: libnodeupdown_test.c,v 1.3 2003-03-07 01:10:20 achu Exp $
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

#include "libnodeupdown_testcases.h"

/*****************************************************************
 * DEFINITIONS                                                   * 
 *****************************************************************/

/*
 * Test Suite definitions
 */
#define LIBNODEUPDOWN_TEST_CONF             "libnodeupdown_test.conf"
#define LIBNODEUPDOWN_TEST_MAXNODES                                 3
#define LIBNODEUPDOWN_TEST_GMOND_PORT                            8659
#define LIBNODEUPDOWN_TEST_SLEEPTIME                               20
#define MAXBUFFERLEN                                             4096

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

  nodeupdown_t handle_not_loaded;
  nodeupdown_t handle_loaded;
  hostlist_t empty_hostlist_1;
  hostlist_t empty_hostlist_2;
  char ** empty_list_1;
  char ** empty_list_2;

  char *conf;
  char *nodes_ab;
  char *nodes_ac;
  char *nodes_bc;
  char *nodes_abc;
};

/*****************************************************************
 * FUNCTION DECLARATIONS                                         *
 *****************************************************************/

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
  case LOCALHOST:
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
void output_parameter_test_result(int index, nodeupdown_t handle, int result, int return_value, int return_errnum) {
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

  /* can't use test_env->handle_not_loaded b/c nodeupdown_load_data() may clear 
   * or set data
   */
  if (nodeupdown_handle == IS_NOT_NULL) {
    if ((handle = nodeupdown_create()) == NULL) {
      printf("Test %d: nodeupdown_create() error\n", index);
      return;
    }
  }

  result = nodeupdown_load_data(handle, genders_filename, gmond_hostname, gmond_ip, gmond_port); 
  
  output_parameter_test_result(index, handle, result, return_value, return_errnum);

  if (nodeupdown_handle == IS_NOT_NULL) {
    if (nodeupdown_destroy(handle) == -1) {
      printf("Test %d: nodeupdown_destroy() error\n", index);
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
				       int nodeupdown_handle, 
				       int hostlist, 
				       int nodeupdown_load_data, 
				       int return_value, 
				       int return_errnum) {
  nodeupdown_t handle = NULL;
  hostlist_t hl = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    handle = test_env->handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = test_env->handle_loaded;
  }

  if (hostlist == IS_NOT_NULL) {
    hl = test_env->empty_hostlist_1;
  }

  if (function == GET_UP_NODES_HOSTLIST) {
    result = nodeupdown_get_up_nodes_hostlist(handle, hl);
  }
  else {
    result = nodeupdown_get_down_nodes_hostlist(handle, hl);
  }

  output_parameter_test_result(index, handle, result, return_value, return_errnum);
  
}

/* run_get_nodes_list_param_test
 * - run parameters tests for nodeupdown_get_up_nodes_list() and
 *   nodeupdown_get_down_nodes_list()
 */
void run_get_up_nodes_list_param_test(struct test_env *test_env, 
				      int function, 
				      int index, 
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
    handle = test_env->handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = test_env->handle_loaded;
  }

  if (list_flag == IS_NOT_NULL) {
    list = test_env->empty_list_1;
  }

  if (function == GET_UP_NODES_LIST) {
    result = nodeupdown_get_up_nodes_list(handle, list, len);
  }
  else {
    result = nodeupdown_get_down_nodes_list(handle, list, len);
  }

  output_parameter_test_result(index, handle, result, return_value, return_errnum);
	    
}

/* run_is_node_param_test
 * - run parameter tests for nodeupdown_is_node_up() and
 *   nodeupdown_is_node_down()
 */
void run_is_node_param_test(struct test_env *test_env, 
			    int function, 
			    int index,
			    int nodeupdown_handle, 
			    char *node, 
			    int nodeupdown_load_data, 
			    int return_value, 
			    int return_errnum) {
  nodeupdown_t handle = NULL;
  int result;

  if (nodeupdown_handle == IS_NOT_NULL) {
    handle = test_env->handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = test_env->handle_loaded;
  }

  if (function == IS_NODE_UP) {
    result = nodeupdown_is_node_up(handle, node);
  }
  else {
    result = nodeupdown_is_node_down(handle, node);
  }

  output_parameter_test_result(index, handle, result, return_value, return_errnum);
}

/* run_alternate_param_test
 * - run parameter tests for nodeupdown_get_alternate_names_hostlist()
 *   and nodeupdown_get_alternate_names_list()
 */
void run_alternate_param_test(struct test_env *test_env, 
			      int function,
			      int index,
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
    handle = test_env->handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = test_env->handle_loaded;
  }

  if (function == GET_HOSTLIST_ALTERNATE_NAMES) {
    if (src_flag == IS_NOT_NULL) {
      src_h = test_env->empty_hostlist_1;
    }
    if (dest_flag == IS_NOT_NULL) {
      dest_h = test_env->empty_hostlist_2;
    }
  }
  else {
    if (src_flag == IS_NOT_NULL) {
      src_c = test_env->empty_list_1;
    }
    if (dest_flag == IS_NOT_NULL) {
      dest_c = test_env->empty_list_2;
    }
  }

  if (function == GET_HOSTLIST_ALTERNATE_NAMES) {
    result = nodeupdown_get_hostlist_alternate_names(handle, src_h, dest_h);
  }
  else {
    result = nodeupdown_get_list_alternate_names(handle, src_c, dest_c);
  }

  output_parameter_test_result(index, handle, result, return_value, return_errnum);

}

/* run_nodelist_param_test
 * - run parameter tests for nodeupdown_nodelist_create(), 
 *   nodeupdown_nodelist_clear(), and nodeupdown_nodelist_destroy()
 */
void run_nodelist_param_test(struct test_env *test_env, 
			     int function, 
			     int index, 
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
    handle = test_env->handle_not_loaded;
  }

  if (nodeupdown_load_data == EXECUTE) {
    handle = test_env->handle_loaded;
  }

  if (list == IS_NOT_NULL) {
    if (function == NODELIST_CREATE) {
      pptr = &ptr;
    }
    else {
      ptr = test_env->empty_list_1;
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

  output_parameter_test_result(index, handle, result, return_value, return_errnum);
  
}

/* initialize_test_env_parameter_tests
 * - initialize environment for parameter tests
 */
int initialize_test_env_parameter_tests(struct test_env *test_env) {
  test_env->handle_not_loaded = NULL;
  test_env->handle_loaded = NULL;
  test_env->empty_hostlist_1 = NULL;
  test_env->empty_hostlist_2 = NULL;
  test_env->empty_list_1 = NULL;
  test_env->empty_list_2 = NULL;

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

  if ((test_env->handle_not_loaded = nodeupdown_create()) == NULL) {
    printf("nodeupdown_create() error\n");
    return -1;
  }

  if ((test_env->handle_loaded = nodeupdown_create()) == NULL) {
    printf("nodeupdown_create() error\n");
    return -1;
  }
  
  if (nodeupdown_load_data(test_env->handle_loaded, 
			   NULL, 
			   NULL, 
			   NULL, 
			   LIBNODEUPDOWN_TEST_GMOND_PORT) == -1) {
    printf("nodeupdown_load_data() error: %s\n",nodeupdown_strerror(nodeupdown_errnum(test_env->handle_loaded))); 
    return -1;
  }
  
  if (nodeupdown_nodelist_create(test_env->handle_loaded, 
				 &test_env->empty_list_1) == -1) {
    printf("nodeupdown_nodelist_create() error\n");
    return -1;
  }

  if (nodeupdown_nodelist_create(test_env->handle_loaded, 
				 &test_env->empty_list_2) == -1) {
    printf("nodeupdown_nodelist_create() error\n");
    return -1;
  }

  if ((test_env->empty_hostlist_1 = hostlist_create(NULL)) == NULL) {
    printf("hostlist_create() error\n");
    return -1;
  }

  if ((test_env->empty_hostlist_2 = hostlist_create(NULL)) == NULL) {
    printf("hostlist_create() error\n");
    return -1;
  }

  return 0;
}

/* cleanup_test_env_parameter_tests
 * - cleanup test_env for the parameter tests
 */
int cleanup_test_env_parameter_tests(struct test_env *test_env) {
  int retval = 0;

  if (test_env->empty_hostlist_1 != NULL) {
    hostlist_destroy(test_env->empty_hostlist_1);
  }

  if (test_env->empty_hostlist_2 != NULL) {
    hostlist_destroy(test_env->empty_hostlist_2);
  }

  if (test_env->empty_list_1 != NULL) {
    if (nodeupdown_nodelist_destroy(test_env->handle_loaded, test_env->empty_list_1) == -1) {
      printf("nodeupdown_nodelist_destroy() error\n");
      retval = -1;
    }
  }
  
  if (test_env->empty_list_2 != NULL) {
    if (nodeupdown_nodelist_destroy(test_env->handle_loaded, test_env->empty_list_2) == -1) {
      printf("nodeupdown_nodelist_destroy() error\n");
      retval = -1;
    }
  }

  if (test_env->handle_not_loaded != NULL) {
    if (nodeupdown_destroy(test_env->handle_not_loaded) == -1) {
      printf("nodeupdown_destroy() error\n");
      retval = -1;
    }
  }

  if (test_env->handle_loaded != NULL) {
    if (nodeupdown_destroy(test_env->handle_loaded) == -1) {
      printf("nodeupdown_destroy() error\n");
      retval = -1;
    }
  }

  if (close_gmonds(test_env, NODE_ALL) == -1) {
    printf("close_gmonds() error\n");
    retval = -1;
  }

  return retval;
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
			    nodelist_param_tests[i].nodeupdown_handle, 
			    nodelist_param_tests[i].list,
			    nodelist_param_tests[i].nodeupdown_load_data, 
			    nodelist_param_tests[i].return_value, 
			    nodelist_param_tests[i].return_errnum);  
    i++;
  }
  printf("\n\n");

  if (cleanup_test_env_parameter_tests(test_env) == -1) {
    printf("cleanup_test_env_parameter_tests() error\n");
    goto cleanup;
  }

  return 0;

 cleanup:

  (void)cleanup_test_env_parameter_tests(test_env);
  
  return -1;
}

int compare_nodes_to_hostlist(struct test_env *test_env, 
			      hostlist_t hl, 
			      int nodes) {

  int retval = 0;

  switch(nodes) {
  case NODE_NONE:
    if (!((hostlist_find(hl, test_env->nodes[0])) >= 0 ||
	  (hostlist_find(hl, test_env->nodes[1])) >= 0 ||
	  (hostlist_find(hl, test_env->nodes[2])) >= 0)) {
      retval = 1;
    }
    break;
  case NODE_A:
    if (((hostlist_find(hl, test_env->nodes[0])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[1])) == -1) &&
	((hostlist_find(hl, test_env->nodes[2])) == -1)) {
      return 1;
    }
    break;
  case NODE_B:
    if (((hostlist_find(hl, test_env->nodes[0])) == -1) &&
	((hostlist_find(hl, test_env->nodes[1])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[2])) == -1)) {
      return 1;
    }
    break;
  case NODE_C:
    if (((hostlist_find(hl, test_env->nodes[0])) == -1) &&
	((hostlist_find(hl, test_env->nodes[1])) == -1) &&
	((hostlist_find(hl, test_env->nodes[2])) >= 0)) {
      return 1;
    }
    break;
  case NODE_AB:
    if (((hostlist_find(hl, test_env->nodes[0])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[1])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[2])) == -1)) {
      return 1;
    }
    break;
  case NODE_AC:
    if (((hostlist_find(hl, test_env->nodes[0])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[1])) == -1) &&
	((hostlist_find(hl, test_env->nodes[2])) >= 0)) {
      return 1;
    }
    break;
  case NODE_BC:
    if (((hostlist_find(hl, test_env->nodes[0])) == -1) &&
	((hostlist_find(hl, test_env->nodes[1])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[2])) >= 0)) {
      return 1;
    }
    break;
  case NODE_ABC:
    if (((hostlist_find(hl, test_env->nodes[0])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[1])) >= 0) &&
	((hostlist_find(hl, test_env->nodes[2])) >= 0)) {
      return 1;
    }
    break;
  default:
    printf("incorrect nodes value in compare_nodes_to_hostlist()\n");
    return -1;
    break;
  }

  return retval;
}

int compare_nodes_to_list_helper(struct test_env *test_env, 
				 char **list,
				 int len,
				 int a, 
				 int b, 
				 int c) {
  int i;
  int retval = 1;

  if (a == 1) {
    retval = 0;
    for (i = 0; i < len; i++) {
      if (strcmp(list[i], test_env->nodes[0]) == 0) {
	retval = 1;
	break;
      }
    }
  }
  if (b == 1) {
    retval = 0;
    for (i = 0; i < len; i++) {
      if (strcmp(list[i], test_env->nodes[1]) == 0) {
	retval = 1;
	break;
      }
    }
  }
  if (c == 1) {
    retval = 0;
    for (i = 0; i < len; i++) {
      if (strcmp(list[i], test_env->nodes[2]) == 0) {
	retval = 1;
	break;
      }
    }
  }

  if (a == 0 || b == 0 || c == 0) {
    for (i = 0; i < len; i++) {
      if (a == 0 && strcmp(list[i],test_env->nodes[0]) == 0) {
	retval = 0;
	break;
      }
      if (b == 0 && strcmp(list[i],test_env->nodes[1]) == 0) {
	retval = 0;
	break;
      }
      if (c == 0 && strcmp(list[i],test_env->nodes[2]) == 0) {
	retval = 0;
	break;
      }
    }
  }

  return retval;
}

int compare_nodes_to_list(struct test_env *test_env, 
			  char **list, 
			  int len, 
			  int nodes) {

  switch(nodes) {
  case NODE_NONE:
    return compare_nodes_to_list_helper(test_env, list, len, 0, 0, 0);
    break;
  case NODE_A:
    return compare_nodes_to_list_helper(test_env, list, len, 1, 0, 0);
    break;
  case NODE_B:
    return compare_nodes_to_list_helper(test_env, list, len, 0, 1, 0);
    break;
  case NODE_C:
    return compare_nodes_to_list_helper(test_env, list, len, 0, 0, 1);
    break;
  case NODE_AB:
    return compare_nodes_to_list_helper(test_env, list, len, 1, 1, 0);
    break;
  case NODE_AC:
    return compare_nodes_to_list_helper(test_env, list, len, 1, 0, 1);
    break;
  case NODE_BC:
    return compare_nodes_to_list_helper(test_env, list, len, 0, 1, 1);
    break;
  case NODE_ABC:
    return compare_nodes_to_list_helper(test_env, list, len, 1, 1, 1);
    break;
  default:
    printf("incorrect nodes value in compare_nodes_to_list()\n");
    return -1;
    break;
  }

  return -1;
}

void run_a_functionality_test(struct test_env *test_env,
			      int function,
			      int index,
			      int nodes_to_check,
			      int host_to_query,
			      int return_value,
			      int return_errnum) {
  int result, i;
  hostlist_t hostlist;
  char **list;
  char *node;
  int len;
  char buffer[MAXBUFFERLEN];

  if (function == GET_UP_NODES_HOSTLIST || function == GET_DOWN_NODES_HOSTLIST) {
    if ((hostlist = hostlist_create(NULL)) == NULL) {
      printf("Test %d: hostlist_create() error\n", index);
      return;
    }
  }
  else if (function == GET_UP_NODES_LIST || function == GET_DOWN_NODES_LIST) {
    if ((len = nodeupdown_nodelist_create(test_env->handle_loaded, &list)) == -1) {
      printf("Test %d: nodeupdown_nodelist_create() error\n", index);
      return;
    }
  }
  else {
    if ((node = get_nodes_ptr(test_env, nodes_to_check)) == NULL) {
      printf("Test %d: get_nodes_ptr() error\n", index);
      return;
    }
  }

  if (function == GET_UP_NODES_HOSTLIST) {
    result = nodeupdown_get_up_nodes_hostlist(test_env->handle_loaded, hostlist);
  }
  else if (function == GET_DOWN_NODES_HOSTLIST){
    result = nodeupdown_get_down_nodes_hostlist(test_env->handle_loaded, hostlist);
  }
  else if (function == GET_UP_NODES_LIST) {
    result = nodeupdown_get_up_nodes_list(test_env->handle_loaded, list, len);
  }
  else if (function == GET_DOWN_NODES_LIST) {
    result = nodeupdown_get_down_nodes_list(test_env->handle_loaded, list, len);
  }
  else if (function == IS_NODE_UP) {
    result = nodeupdown_is_node_up(test_env->handle_loaded, node);
  }
  else if (function == IS_NODE_DOWN) {
    result = nodeupdown_is_node_down(test_env->handle_loaded, node);
  }

  if (result == return_value && nodeupdown_errnum(test_env->handle_loaded) == return_errnum) {
    if (function == GET_UP_NODES_HOSTLIST || function == GET_DOWN_NODES_HOSTLIST) {
      if (compare_nodes_to_hostlist(test_env, hostlist, nodes_to_check)) {
	printf("Test %d: PASS\n", index);
      }
      else {
	memset(buffer, '\0', MAXBUFFERLEN);
	hostlist_ranged_string(hostlist, MAXBUFFERLEN, buffer);
	printf("Test %d: ***FAIL*** returned hostlists = %s\n", index, buffer);
      }
    }
    else if (function == GET_UP_NODES_LIST || function == GET_DOWN_NODES_LIST) {
      if (compare_nodes_to_list(test_env, list, len, nodes_to_check)) {
	printf("Test %d: PASS\n", index);
      }
      else {
	printf("Test %d: ***FAIL*** returned nodes ", index);
	i = 0;
	while (strlen(list[i]) > 0 && i < len) {
	  printf("%s ", list[i++]);
	}
	printf("\n");
      }
    }
    else {
      printf("Test %d: PASS\n", index);
    }
  }
  else {
    printf("Test %d: ***FAIL*** return value = %d, return errnum = %d\n", 
	   index, result, nodeupdown_errnum(test_env->handle_loaded));
  }

  if (function == GET_UP_NODES_HOSTLIST || function == GET_DOWN_NODES_HOSTLIST) {
    hostlist_destroy(hostlist);
  }
  else if (function == GET_UP_NODES_LIST || function == GET_DOWN_NODES_LIST) {
    (void)nodeupdown_nodelist_destroy(test_env->handle_loaded, list);
  }
}

int setup_functionality_test(struct test_env *test_env, int nodes, int host_to_query, int restart_gmonds_flag) {
  char * ptr;

  test_env->handle_loaded = NULL;

  if (restart_gmonds_flag == 1) {
    if (start_gmonds(test_env, nodes) == -1) {
      printf("start_gmonds() error\n");
      return -1;
    }

    /* sleep a bit, wait for gmonds to get going */
    sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);

  }

  /* sleep a bit, wait for gmonds to get going */
  sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);

  if ((test_env->handle_loaded = nodeupdown_create()) == NULL) {
    printf("nodeupdown_create() error\n");
    return -1;
  }

  if ((test_env->handle_loaded = nodeupdown_create()) == NULL) {
    printf("nodeupdown_create() error\n");
    return -1;
  }

  ptr = get_nodes_ptr(test_env, host_to_query);
  
  if (nodeupdown_load_data(test_env->handle_loaded, 
			   NULL, 
			   ptr,
			   NULL, 
			   LIBNODEUPDOWN_TEST_GMOND_PORT) == -1) {
    printf("nodeupdown_load_data() error: %s\n", nodeupdown_strerror(nodeupdown_errnum(test_env->handle_loaded)));
    return -1;
  }

  return 0;
}

int cleanup_functionality_test(struct test_env *test_env, int nodes, int restart_gmonds_flag) {
  int retval = 0;

  if (test_env->handle_loaded != NULL) {
    if (nodeupdown_destroy(test_env->handle_loaded) == -1) {
      printf("nodeupdown_destroy() error\n");
      retval = -1;
    }
  }

  if (restart_gmonds_flag == 1) {
    if (close_gmonds(test_env, nodes) == -1) {
      printf("close_gmonds() error\n");
      retval = -1;
    }
  }

  return retval;
}

int run_functionality_tests(struct test_env *test_env) {
  int i, restart_gmonds_flag, old_nodes_up, old_host_to_query;

  /* kill existing gmonds first */
  if (close_gmonds(test_env, NODE_ALL) == -1) {
    printf("close_gmonds() error\n");
    return -1;
  }

  /* we use the "restart_gmonds_flag" variable to minimize
   * the number of times gmonds must be closed and started
   * up again.
   */

  i = 0;
  restart_gmonds_flag = 1;
  while(functionality_tests[i].function != -1) {
    if (setup_functionality_test(test_env, 
				 functionality_tests[i].nodes_up, 
				 functionality_tests[i].host_to_query,
				 restart_gmonds_flag) == -1) {
      printf("setup_functionality_test() error\n");
      return -1;
    }
    
    old_nodes_up = functionality_tests[i].nodes_up;
    old_host_to_query = functionality_tests[i].host_to_query;

    while (functionality_tests[i].function != -1 &&
	   functionality_tests[i].nodes_up == old_nodes_up &&
	   functionality_tests[i].host_to_query == old_host_to_query) {
      run_a_functionality_test(test_env, 
			       functionality_tests[i].function,
			       i, 
			       functionality_tests[i].nodes_to_check,
			       functionality_tests[i].host_to_query,
			       functionality_tests[i].return_value, 
			       functionality_tests[i].return_errnum);  
      i++;
      sleep(1);
    }

    if (functionality_tests[i].nodes_up != old_nodes_up)
      restart_gmonds_flag = 1;
    else
      restart_gmonds_flag = 0;

    if (cleanup_functionality_test(test_env, NODE_ALL, restart_gmonds_flag) == -1) {
      printf("cleanup_functionality_test() error\n");
      return -1;
    }

    if (restart_gmonds_flag == 1) {
      /* sleep a bit, wait for gmonds to get going */
      sleep(LIBNODEUPDOWN_TEST_SLEEPTIME);
    }
  }
  return 0;
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
  
  if (run_param_tests(test_env) == -1) {
    printf("run_param_tests() error\n");
    goto cleanup;
  }

  sleep(LIBNODEUPDOWN_SLEEPTIME);

  if (run_functionality_tests(test_env) == -1) {
    printf("run_functionality_tests() error\n");
    goto cleanup;
  }

  cleanup_test_env(test_env);

  free(test_env);

  return 0;

 cleanup:

  if (test_env != NULL) {
    free(test_env);
  }

  return -1;
}
