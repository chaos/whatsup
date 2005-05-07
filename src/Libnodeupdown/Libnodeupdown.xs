/*****************************************************************************\
 *  $Id: Libnodeupdown.xs,v 1.17 2005-05-07 18:29:53 achu Exp $
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

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "nodeupdown.h"
#include "nodeupdown/nodeupdown_devel.h"

#define LIBNODEUPDOWN_BUFLEN 4096
 
MODULE = Libnodeupdown             PACKAGE = Libnodeupdown

PROTOTYPES: ENABLE

int
NODEUPDOWN_ERR_SUCCESS (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_SUCCESS;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_NULLHANDLE (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_NULLHANDLE;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_CONNECT (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_CONNECT;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_TIMEOUT (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_TIMEOUT;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_HOSTNAME (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_HOSTNAME;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_ADDRESS (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_ADDRESS;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_ISLOADED (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_ISLOADED;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_NOTLOADED (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_NOTLOADED;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_OVERFLOW (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_OVERFLOW;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_PARAMETERS (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_PARAMETERS;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_NULLPTR (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_NULLPTR;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_OUTMEM (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_OUTMEM;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_NOTFOUND (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_NOTFOUND;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_CLUSTERLIST_MODULE (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_CLUSTERLIST_MODULE;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_BACKEND_MODULE (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_BACKEND_MODULE;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_CONFIG_MODULE (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_CONFIG_MODULE;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_CONF_PARSE (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_CONF_PARSE;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_CONF_INPUT (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_CONF_INPUT;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_CONF_INTERNAL (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_CONF_INTERNAL;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_MAGIC (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_MAGIC;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_INTERNAL (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_INTERNAL;
    OUTPUT:
        RETVAL    

int
NODEUPDOWN_ERR_ERRNUMRANGE (sv=&PL_sv_undef)
    SV *sv    
    CODE:
        RETVAL = NODEUPDOWN_ERR_ERRNUMRANGE;
    OUTPUT:
        RETVAL    

void
DESTROY(handle)
    nodeupdown_t handle    
    CODE:
       (void)nodeupdown_handle_destroy(handle);

nodeupdown_t
nodeupdown_handle_create(CLASS)
    char *CLASS;
    CODE:
        RETVAL = nodeupdown_handle_create();
    OUTPUT:
        RETVAL

int
nodeupdown_load_data(handle, hostname=NULL, port=0, timeout_len=0, reserved=NULL)
    nodeupdown_t handle
    char *reserved
    char *hostname
    int port
    int timeout_len
    CODE:
        RETVAL = nodeupdown_load_data(handle,
                                      hostname, 
                                      port, 
                                      timeout_len,
                                      reserved);
    OUTPUT:
        RETVAL

int
nodeupdown_errnum(handle)
    nodeupdown_t handle
    CODE:
        RETVAL = nodeupdown_errnum(handle);
    OUTPUT:
        RETVAL

char *
nodeupdown_strerror(handle, errnum)
    nodeupdown_t handle    
    int errnum
    CODE:
        RETVAL = nodeupdown_strerror(errnum);
    OUTPUT:
        RETVAL

char *
nodeupdown_errormsg(handle)
    nodeupdown_t handle
    CODE:
        RETVAL = nodeupdown_errormsg(handle);
    OUTPUT:
        RETVAL

void
nodeupdown_perror(handle, msg=NULL)
    nodeupdown_t handle
    char *msg
    CODE:
        nodeupdown_perror(handle, msg);

SV *
nodeupdown_get_up_nodes_string(handle)
    nodeupdown_t handle
    PREINIT:
        int rv, len;
        char *buf = NULL;
    CODE:

        len = 0;
        while (1) {
            len += LIBNODEUPDOWN_BUFLEN;
            if ((buf = (char *)malloc(len+1)) == NULL) 
                goto handle_error;

            memset(buf, '\0', len+1);
            if ((rv = nodeupdown_get_up_nodes_string(handle, buf, len+1)) == -1) {
                if (nodeupdown_errnum(handle) == NODEUPDOWN_ERR_OVERFLOW) {
                    free(buf);
                    continue;
                }
                else
                    goto handle_error;
            }
            else
                break;
        }

        RETVAL = newSVpv(buf, 0);
        free(buf);
        goto the_end;

        handle_error:

            free(buf);
            XSRETURN_UNDEF;
        
        the_end:
    OUTPUT:
        RETVAL

SV *
nodeupdown_get_down_nodes_string(handle)
    nodeupdown_t handle
    PREINIT:
        int rv, len;
        char *buf = NULL;
    CODE:

        len = 0;
        while (1) {
            len += LIBNODEUPDOWN_BUFLEN;
            if ((buf = (char *)malloc(len+1)) == NULL) 
                goto handle_error;

            memset(buf, '\0', len+1);
            if ((rv = nodeupdown_get_down_nodes_string(handle, buf, len+1)) == -1) {
                if (nodeupdown_errnum(handle) == NODEUPDOWN_ERR_OVERFLOW) {
                    free(buf);
                    continue;
                }
                else
                    goto handle_error;
            }
            else
                break;
        }

        RETVAL = newSVpv(buf, 0);
        free(buf);
        goto the_end;

        handle_error:

            free(buf);
            XSRETURN_UNDEF;
        
        the_end:
    OUTPUT:
        RETVAL

AV *
nodeupdown_get_up_nodes_list(handle) 
    nodeupdown_t handle
    PREINIT:
        int len, num, errnum, i;
        char **nlist = NULL; 
    CODE:
        if ((len = nodeupdown_nodelist_create(handle, &nlist)) == -1) 
            goto handle_error;

        if ((num = nodeupdown_get_up_nodes_list(handle, nlist, num)) == -1)
            goto handle_error;

        RETVAL = newAV();
        for (i = 0; i < num; i++)
            av_push(RETVAL, newSVpv(nlist[i], 0));
        
        if (nodeupdown_nodelist_destroy(handle, nlist) == -1)
            goto handle_error;

        goto the_end;

        handle_error:

            errnum = nodeupdown_errnum(handle);

            (void)nodeupdown_nodelist_destroy(handle, nlist);

            nodeupdown_set_errnum(handle, errnum);

            XSRETURN_UNDEF;

        the_end:
    OUTPUT:
        RETVAL    

AV *
nodeupdown_get_down_nodes_list(handle) 
    nodeupdown_t handle
    PREINIT:
        int len, num, errnum, i;
        char **nlist = NULL; 
    CODE:
        if ((len = nodeupdown_nodelist_create(handle, &nlist)) == -1) 
            goto handle_error;

        if ((num = nodeupdown_get_down_nodes_list(handle, nlist, len)) == -1)
            goto handle_error;

        RETVAL = newAV();
        for (i = 0; i < num; i++)
            av_push(RETVAL, newSVpv(nlist[i], 0));
        
        if (nodeupdown_nodelist_destroy(handle, nlist) == -1)
            goto handle_error;

        goto the_end;

        handle_error:

            errnum = nodeupdown_errnum(handle);

            (void)nodeupdown_nodelist_destroy(handle, nlist);

            nodeupdown_set_errnum(handle, errnum);

            XSRETURN_UNDEF;

        the_end:
    OUTPUT:
        RETVAL    

int
nodeupdown_is_node_up(handle, node)
    nodeupdown_t handle
    char *node
    CODE:
        RETVAL = nodeupdown_is_node_up(handle, node);
    OUTPUT:
        RETVAL      

int
nodeupdown_is_node_down(handle, node)
    nodeupdown_t handle
    char *node
    CODE:
        RETVAL = nodeupdown_is_node_down(handle, node);
    OUTPUT:
        RETVAL

int
nodeupdown_up_count(handle)
    nodeupdown_t handle
    CODE:
        RETVAL = nodeupdown_up_count(handle);
    OUTPUT:
        RETVAL

int
nodeupdown_down_count(handle)
    nodeupdown_t handle
    CODE:
        RETVAL = nodeupdown_down_count(handle);
    OUTPUT:
        RETVAL

void
nodeupdown_set_errnum(handle, errnum)
    nodeupdown_t handle
    int errnum
    CODE:
        nodeupdown_set_errnum(handle, errnum);

