/*
 *  $Id: nodeupdown_common.h,v 1.5 2003-11-24 16:13:19 achu Exp $
 *  $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown_common.h,v $
 *    
 */

#ifndef _NODEUPDOWN_COMMON_H
#define _NODEUPDOWN_COMMON_H

/* Common code, definitions, and functions in nodeupdown.c and
 * nodeupdown_masterlist.c 
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_HOSTSFILE
#include "list.h"
#elif HAVE_GENDERS
#include <genders.h>
#elif HAVE_GENDERSLLNL
#include <gendersllnl.h>
#endif 

#include "hostlist.h"
#include "nodeupdown.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#define NODEUPDOWN_UP_NODES              1
#define NODEUPDOWN_DOWN_NODES            0

#define NODEUPDOWN_MAGIC_NUM             0xfeedbeef

#define NODEUPDOWN_BUFFERLEN             65536

#define NODEUPDOWN_CONNECT_LEN           5 

/* Configuration file keys */
#define NODEUPDOWN_CONF_GMOND_HOSTNAME     "gmond_hostname"
#define NODEUPDOWN_CONF_GMOND_PORT         "gmond_port"
#define NODEUPDOWN_CONF_TIMEOUT_LEN        "timeout_len"
#define NODEUPDOWN_CONF_MASTERLIST         "masterlist"
#define NODEUPDOWN_CONF_GMOND_HOSTNAME_MAX 8
#define NODEUPDOWN_CONF_MASTERLIST_BUFLEN  64

struct nodeupdown {
  int magic;                  /* magic number */
  int errnum;                 /* error code */
  int is_loaded;              /* nodeupdown data loaded? */
  hostlist_t up_nodes;        /* up nodes */
  hostlist_t down_nodes;      /* down nodes */
  int max_nodes;              /* max nodes in genders file */
#if HAVE_HOSTSFILE
  List hosts;                 /* list of all nodes */
#elif (HAVE_GENDERS || HAVE_GENDERSLLNL)
  genders_t genders_handle;   /* genders handle */
#endif 
};

#endif /* _NODEUPDOWN_COMMON_H */
