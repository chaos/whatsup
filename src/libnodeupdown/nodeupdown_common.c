/*
 *  $Id: nodeupdown_common.c,v 1.1 2003-11-07 18:28:58 achu Exp $
 *  $Source: /g/g0/achu/temp/whatsup-cvsbackup/whatsup/src/libnodeupdown/nodeupdown_common.c,v $
 *    
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "fd.h"

int _readline(nodeupdown_t handle, int fd, char *buf, int buflen) {
  int ret;

  if ((ret = fd_read_line(fd, buf, buflen)) < 0) {
    handle->errnum = NODEUPDOWN_ERR_READ;
    return -1;
  }
  
  /* overflow counts as a parse error */
  /* XXX assumes buflen >= 2 */
  if (ret == buflen && buf[buflen-2] != '\n') {
    handle->errnum = NODEUPDOWN_ERR_INTERNAL;
    return -1;
  }

  return ret;
}

