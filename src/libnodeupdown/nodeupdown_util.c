/*****************************************************************************\
 *  $Id: nodeupdown_util.c,v 1.3 2005-04-25 16:40:19 achu Exp $
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

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <dirent.h>
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_common.h"
#include "nodeupdown_util.h"
#include "ltdl.h"

int
nodeupdown_util_low_timeout_connect(nodeupdown_t handle,
                                    const char *hostname,
                                    int port,
                                    int connect_timeout)
{
  int ret, old_flags, fd = -1;
  struct sockaddr_in servaddr;
  char ipbuf[INET_ADDRSTRLEN+1];
  struct hostent *hptr;

  /* valgrind will report a mem-leak in gethostbyname() */
  if (!(hptr = gethostbyname(hostname))) 
    {
      handle->errnum = NODEUPDOWN_ERR_HOSTNAME;
      return -1;
    }
      
  if (!inet_ntop(AF_INET, (void *)hptr->h_addr, ipbuf, INET_ADDRSTRLEN)) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      return -1;
    }

  /* Alot of this code is from Unix Network Programming, by Stevens */

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if ((ret = inet_pton(AF_INET, ipbuf, (void *)&servaddr.sin_addr)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }

  if (!ret) 
    {
      handle->errnum = NODEUPDOWN_ERR_ADDRESS;
      goto cleanup;
    }

  if ((old_flags = fcntl(fd, F_GETFL, 0)) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
  
  if (fcntl(fd, F_SETFL, old_flags | O_NONBLOCK) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
  
  ret = connect(fd, 
                (struct sockaddr *)&servaddr, 
                sizeof(struct sockaddr_in));
  if (ret < 0 && errno != EINPROGRESS) 
    {
      handle->errnum = NODEUPDOWN_ERR_CONNECT;
      goto cleanup;
    }
  else if (ret < 0 && errno == EINPROGRESS) 
    {
      fd_set rset, wset;
      struct timeval tval;

      FD_ZERO(&rset);
      FD_SET(fd, &rset);
      FD_ZERO(&wset);
      FD_SET(fd, &wset);
      tval.tv_sec = connect_timeout;
      tval.tv_usec = 0;
      
      if ((ret = select(fd+1, &rset, &wset, NULL, &tval)) < 0) 
        {
          handle->errnum = NODEUPDOWN_ERR_INTERNAL;
          goto cleanup;
        }

      if (!ret) 
        {
          handle->errnum = NODEUPDOWN_ERR_TIMEOUT;
          goto cleanup;
        }
      else 
        {
          if (FD_ISSET(fd, &rset) || FD_ISSET(fd, &wset)) 
            {
              int len, error;

              len = sizeof(int);
              
              if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) 
                {
                  handle->errnum = NODEUPDOWN_ERR_INTERNAL;
                  goto cleanup;
                }
        
              if (error != 0) 
                {
                  errno = error;
                  handle->errnum = NODEUPDOWN_ERR_CONNECT;
                  goto cleanup;
                }
              /* else no error, connected within timeout length */
            }
          else 
            {
              handle->errnum = NODEUPDOWN_ERR_INTERNAL;
              goto cleanup;
            }
        }
    }
  
  /* reset flags */
  if (fcntl(fd, F_SETFL, old_flags) < 0) 
    {
      handle->errnum = NODEUPDOWN_ERR_INTERNAL;
      goto cleanup;
    }
  
  return fd;

 cleanup:
  close(fd);
  return -1;
}

int
nodeupdown_util_lookup_module(nodeupdown_t handle, 
			      char *search_dir,
			      char **modules_list,
			      int modules_list_len,
			      Nodeupdown_util_load_module load_module)
{
  DIR *dir;
  int i = 0, found = 0, rv = -1;
 
  /* Can't open the directory? we assume it doesn't exit, so its not
   * an error.
   */
  if (!(dir = opendir(search_dir)))
    return 0;

  for (i = 0; i < modules_list_len; i++)
    {
      struct dirent *dirent;

      while ((dirent = readdir(dir)))
        {
          if (!strcmp(dirent->d_name, modules_list[i]))
            {
              char filebuf[NODEUPDOWN_MAXPATHLEN+1];
              int ret;

              memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
              snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
                       search_dir, modules_list[i]);

              if ((ret = load_module(handle, filebuf)) < 0)
                  goto cleanup;

              if (ret)
                {
                  found++;
                  goto done;
                }
            }
        }
      rewinddir(dir);
    }

 done:
  rv = (found) ? 1 : 0;
 cleanup:
  closedir(dir);
  return rv;
}

int
nodeupdown_util_search_for_module(nodeupdown_t handle,
				  char *search_dir,
				  char *signature,
				  Nodeupdown_util_load_module load_module)
{
  DIR *dir;
  struct dirent *dirent;
  int rv = -1, found = 0;
 
  if (!(dir = opendir(search_dir)))
    return 0;
 
  while ((dirent = readdir(dir)))
    {
      char *ptr = strstr(dirent->d_name, signature);

      if (ptr && ptr == &dirent->d_name[0])
        {
          char filebuf[NODEUPDOWN_MAXPATHLEN+1];
          int ret;
 
          /*
           * Don't bother trying to load this file unless its a shared
           * object file or libtool file.
           */
          ptr = strchr(dirent->d_name, '.');
          if (!ptr || !(!strcmp(ptr, ".la") || !strcmp(ptr, ".so")))
            continue;
 
          memset(filebuf, '\0', NODEUPDOWN_MAXPATHLEN+1);
          snprintf(filebuf, NODEUPDOWN_MAXPATHLEN, "%s/%s",
                   search_dir, dirent->d_name);
           
          if ((ret = load_module(handle, filebuf)) < 0)
	    goto cleanup;
 
          if (ret)
            {
              found++;
              goto done;
            }
        }
    }
   
 done:
  rv = (found) ? 1 : 0;
 cleanup:
  closedir(dir);
  return rv;
}
