/*
 *  $Id: nodeupdown_common.h,v 1.1 2003-11-07 18:28:58 achu Exp $
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

#if HAVE_MASTERLIST
#include "list.h"
#elif HAVE_GENDERS
#include <genders.h>
#include <gendersllnl.h>
#endif 

#include "hostlist.h"
#include "nodeupdown.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#define NODEUPDOWN_BUFFERLEN  65536

struct nodeupdown {
  int magic;                  /* magic number */
  int errnum;                 /* error code */
  int is_loaded;              /* nodeupdown data loaded? */
  hostlist_t up_nodes;        /* up nodes */
  hostlist_t down_nodes;      /* down nodes */
  int max_nodes;              /* max nodes in genders file */
#if HAVE_MASTERLIST
  List masterlist;            /* list of all nodes */
#elif HAVE_GENDERS
  genders_t genders_handle;   /* genders handle */
#endif 
};

int _readline(nodeupdown_t, int, char *, int);

#endif /* _NODEUPDOWN_COMMON_H */
