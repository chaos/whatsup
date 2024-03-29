/*****************************************************************************\
 *  $Id: nodeupdown_backend_ganglia.c,v 1.33 2010-02-02 00:01:58 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007-2015 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Albert Chu <chu11@llnl.gov>
 *  UCRL-CODE-155699
 *
 *  This file is part of Whatsup, tools and libraries for determining up and
 *  down nodes in a cluster. For details, see https://github.com/chaos/whatsup.
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
 *  with Whatsup.  If not, see <http://www.gnu.org/licenses/>.
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
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <errno.h>

#include "nodeupdown.h"
#include "nodeupdown_backend_connect_util.h"
#include "nodeupdown_module.h"
#include "nodeupdown/nodeupdown_backend_module.h"
#include "nodeupdown/nodeupdown_constants.h"
#include "nodeupdown/nodeupdown_devel.h"

#if HAVE_LIBEXPAT
#include <expat.h>
#else  /* !HAVE_LIBEXPAT */
#include "xmlparse.h"
#endif  /* !HAVE_LIBEXPAT */

/* Used to pass multiple variables as one during XML parsing */
struct parse_vars
{
  nodeupdown_t handle;
  int timeout_len;
  unsigned long localtime;
};

#define GANGLIA_BACKEND_DEFAULT_PORT        8649
#define GANGLIA_BACKEND_DEFAULT_TIMEOUT_LEN 60
#define GANGLIA_BACKEND_CONNECT_LEN         5

char ganglia_default_hostname[NODEUPDOWN_MAXHOSTNAMELEN+1];

/*
 * ganglia_backend_default_hostname
 *
 * ganglia backend module default_hostname function
 */
static char *
ganglia_backend_default_hostname(nodeupdown_t handle)
{
  memset(ganglia_default_hostname, '\0', NODEUPDOWN_MAXHOSTNAMELEN+1);
  if (gethostname(ganglia_default_hostname, NODEUPDOWN_MAXHOSTNAMELEN) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "gethostname: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      return NULL;
    }
  return &ganglia_default_hostname[0];
}

/*
 * ganglia_backend_default_port
 *
 * ganglia backend module default_port function
 */
static int
ganglia_backend_default_port(nodeupdown_t handle)
{
  return GANGLIA_BACKEND_DEFAULT_PORT;
}

/*
 * ganglia_backend_default_timeout_len
 *
 * ganglia backend module default_timeout_len function
 */
static int
ganglia_backend_default_timeout_len(nodeupdown_t handle)
{
  return GANGLIA_BACKEND_DEFAULT_TIMEOUT_LEN;
}

/*
 * ganglia_backend_preferred_clusterlist_module
 *
 * ganglia backend preferred_clusterlist_module function
 */
static char *
ganglia_backend_preferred_clusterlist_module(nodeupdown_t handle)
{
  return NULL;
}

/*
 * ganglia_backend_setup
 *
 * ganglia backend module setup function
 */
static int
ganglia_backend_setup(nodeupdown_t handle)
{
  /* nothing to do */
  return 0;
}

/*
 * ganglia_backend_cleanup
 *
 * ganglia backend module cleanup function
 */
static int
ganglia_backend_cleanup(nodeupdown_t handle)
{
  /* nothing to do */
  return 0;
}

/*
 * _xml_start
 *
 * Callback for xml parser.  Parses beginning tags like <FOO attr1=X
 * attr2=Y>
 */
static void
_xml_parse_start(void *data, const char *e1, const char **attr)
{
  nodeupdown_t handle = ((struct parse_vars *)data)->handle;
  int timeout_len = ((struct parse_vars *)data)->timeout_len;
  unsigned long localtime = ((struct parse_vars *)data)->localtime;
  unsigned long reported;

  if (strcmp("HOST", e1) == 0)
    {
      /* attributes of XML HOST tag
       * attr[0] - "NAME"
       * attr[1] - hostname
       * attr[2] - "IP"
       * attr[3] - ip address of host
       * attr[4] - "REPORTED"
       * attr[5] - time gmond received a multicast message from the host
       * - remaining attributes aren't needed
       */

      /* store as up or down */
      reported = atol(attr[5]);
      /* With ganglia 3.2.0, attr[4] is actually TAGS, and we want the
         next pair.  */
      if (!reported) reported = atol(attr[7]);
      if (abs(localtime - reported) < timeout_len)
        nodeupdown_add_up_node(handle, attr[1]);
      else
        nodeupdown_add_down_node(handle, attr[1]);
    }
}

/*
 * _xml_start
 *
 * Callback for xml parser.  Parses end tags like </FOO>
 */
static void
_xml_parse_end(void *data, const char *e1)
{
  /* nothing to parse at this time */
}

/*
 * ganglia_backend_get_updown_data
 *
 * ganglia backend module get_updown_data function
 */
static int
ganglia_backend_get_updown_data(nodeupdown_t handle,
                                const char *hostname,
                                unsigned int port,
                                unsigned int timeout_len,
                                char *reserved)
{
  XML_Parser xml_parser = NULL;
  struct parse_vars pv;
  struct timeval tv;
  int fd, rv = -1;

  if ((fd = _low_timeout_connect(handle,
                                 hostname,
                                 port,
                                 GANGLIA_BACKEND_CONNECT_LEN)) < 0)
    goto cleanup;

  /* Setup parse vars to pass to _xml_parse_start */
  pv.handle = handle;
  pv.timeout_len = timeout_len;

  /* Call gettimeofday at the latest point right before XML stuff. */
  if (gettimeofday(&tv, NULL) < 0)
    {
#ifndef NDEBUG
      fprintf(stderr, "gettimeofday: %s\n", strerror(errno));
#endif /* NDEBUG */
      nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
      goto cleanup;
    }
  pv.localtime = tv.tv_sec;

  /* Following XML parsing loop more or less ripped from libganglia by
   * Matt Massie <massie at CS.Berkeley.EDU>
   */

  xml_parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xml_parser, _xml_parse_start, _xml_parse_end);
  XML_SetUserData(xml_parser, (void *)&pv);

  while (1)
    {
      int bytes_read;
      void *buff;

      if (!(buff = XML_GetBuffer(xml_parser, BUFSIZ)))
        {
#ifndef NDEBUG
          fprintf(stderr, "XML_GetBuffer: %s\n", strerror(errno));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
          goto cleanup;
        }

      if ((bytes_read = read(fd, buff, BUFSIZ)) < 0)
        {
#ifndef NDEBUG
          fprintf(stderr, "read: %s\n", strerror(errno));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
          goto cleanup;
        }

      if (!XML_ParseBuffer(xml_parser, bytes_read, bytes_read == 0))
        {
#ifndef NDEBUG
          fprintf(stderr, "XML_ParseBuffer: %s\n", strerror(errno));
#endif /* NDEBUG */
          nodeupdown_set_errnum(handle, NODEUPDOWN_ERR_INTERNAL);
          goto cleanup;
        }

      if (bytes_read == 0)
        break;
    }

  rv = 0;

 cleanup:
  /* ignore potential error, just return error or result */
  close(fd);
  if (xml_parser != NULL)
    XML_ParserFree(xml_parser);
  return rv;
}

struct nodeupdown_backend_module_info backend_module_info =
  {
    "ganglia",
    &ganglia_backend_default_hostname,
    &ganglia_backend_default_port,
    &ganglia_backend_default_timeout_len,
    &ganglia_backend_preferred_clusterlist_module,
    &ganglia_backend_setup,
    &ganglia_backend_cleanup,
    &ganglia_backend_get_updown_data
  };
