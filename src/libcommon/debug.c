/*****************************************************************************\
 *  $Id: debug.c,v 1.1 2006-07-07 18:14:15 chu11 Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef NDEBUG

#if STDC_HEADERS
#include <stdarg.h>
#endif /* STDC_HEADERS */

#include "debug.h"

char *
_debug_msg_create(const char *fmt, ...)
{
  char *buffer;
  va_list ap;

  if (!fmt)
    return NULL;

  if (!(buffer = malloc(DEBUG_BUFFER_LEN)))
    return NULL;

  va_start(ap, fmt);
  vsnprintf(buffer, DEBUG_BUFFER_LEN, fmt, ap);
  va_end(ap);

  return buffer;
}

#endif /* NDEBUG */
