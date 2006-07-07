/*****************************************************************************\
 *  $Id: debug.h,v 1.1 2006-07-07 18:14:15 chu11 Exp $
\*****************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "error.h"

#define DEBUG_BUFFER_LEN 8192

#define DEBUG_MSG_CREATE(__msg) \
    char __err[DEBUG_BUFFER_LEN]; \
    int __len; \
    \
    memset(__err, '\0', DEBUG_BUFFER_LEN); \
    \
    __len = snprintf(__err, \
                     DEBUG_BUFFER_LEN, \
                     "(%s, %s, %d): ", \
                     __FILE__, \
                     __FUNCTION__, \
                     __LINE__); \
    \
    if (__len < DEBUG_BUFFER_LEN) \
      { \
        char *__str; \
        if ((__str = _debug_msg_create __msg)) \
          { \
            strncat(__err, __str, DEBUG_BUFFER_LEN - __len - 1); \
            __len += strlen(__str); \
            free(__str); \
          } \
      }

/*
 * _debug_msg_create
 *
 * create a buffer and put the a mesage inside it
 *
 * Returns message buffer or NULL on error
 */
char *_debug_msg_create(const char *fmt, ...);

#ifndef NDEBUG

#define ERR_DEBUG(__msg) \
    do { \
      DEBUG_MSG_CREATE(__msg) \
      err_debug(__err); \
    } while(0)

#define ERR_OUTPUT(__msg) \
    do { \
      DEBUG_MSG_CREATE(__msg) \
      err_output(__err); \
    } while(0)

#define ERR_EXIT(__msg) \
    do { \
      DEBUG_MSG_CREATE(__msg) \
      err_exit(__err); \
    } while(0)
   
#else /* NDEBUG */

#define ERR_DEBUG(__msg)

#define ERR_OUTPUT(__msg) \
    do { \
      err_output __msg; \
    } while(0)

#define ERR_EXIT(__msg) \
    do { \
      err_exit __msg; \
    } while(0)

#endif /* NDEBUG */

#endif /* _DEBUG_H */
