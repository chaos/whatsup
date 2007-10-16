/*****************************************************************************\
 *  $Id: pingd_loop.c,v 1.7 2007-10-16 23:55:23 chu11 Exp $
 *****************************************************************************
 *  Copyright (C) 2007 Lawrence Livermore National Security, LLC.
 *  Copyright (C) 2003-2007 The Regents of the University of California.
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
 *  with Whatsup.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else /* !HAVE_SYS_TIME_H */
#include <time.h>
#endif  /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */
#include <signal.h>
#include <assert.h>
#include <errno.h>

#include "pingd.h"
#include "pingd_config.h"
#include "pingd_loop.h"
#include "debug.h"
#include "error.h"
#include "fd.h"
#include "hash.h"
#include "list.h"
#include "timeval.h"

#define PINGD_BUFLEN           1024
#define PINGD_NODES_PER_SOCKET 8
#define PINGD_SERVER_BACKLOG   5

extern struct pingd_config conf;

static struct timeval pingd_next_send;

struct pingd_info
{
  char *hostname;
  int fd;
  struct sockaddr_in destaddr;
  uint16_t sequence_number;
  struct timeval last_received;
};

int *fds = NULL;
unsigned int fds_count = 0;
List nodes = NULL;
unsigned int nodes_count = 0;
hash_t nodes_index = NULL;
int server_fd = 0;

extern int h_errno;

static void
_fds_setup(void)
{
  struct sockaddr_in addr;
  int i;

  assert(!fds);
  assert(!fds_count);
  assert(!nodes_count);
  assert(!server_fd);

  nodes_count = hostlist_count(conf.hosts);
  fds_count = nodes_count/PINGD_NODES_PER_SOCKET;
  if (nodes_count % PINGD_NODES_PER_SOCKET)
    fds_count++;

  if (!(fds = (int *)malloc(fds_count * sizeof(int))))
    ERR_EXIT(("malloc: %s", strerror(errno)));

  for (i = 0; i < fds_count; i++)
    {
      if ((fds[i] = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        ERR_EXIT(("socket: %s", strerror(errno)));

      memset(&addr, '\0', sizeof(struct sockaddr_in));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(0);
      addr.sin_addr.s_addr = htonl(INADDR_ANY);

      if (bind(fds[i], (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
        ERR_EXIT(("bind: %s", strerror(errno)));
    }

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    ERR_EXIT(("socket: %s", strerror(errno)));
  
  memset(&addr, '\0', sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(conf.pingd_server_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
    ERR_EXIT(("bind: %s", strerror(errno)));

  if (listen(server_fd, PINGD_SERVER_BACKLOG) < 0)
    ERR_EXIT(("listen: %s", strerror(errno)));
}

static void
_nodes_setup(void)
{
  hostlist_iterator_t itr = NULL;
  char *host = NULL;
  int i = 0;

  assert(fds);
  assert(fds_count);
  assert(!nodes);
  assert(nodes_count);
  assert(!nodes_index);
  
  if (!(nodes = list_create((ListDelF)free)))
    ERR_EXIT(("list_create: %s", strerror(errno)));
  
  if (!(nodes_index = hash_create(nodes_count,
                                  (hash_key_f)hash_key_string,
                                  (hash_cmp_f)strcmp,
                                  NULL)))
    ERR_EXIT(("hash_create: %s", strerror(errno)));

  if (!(itr = hostlist_iterator_create(conf.hosts)))
    ERR_EXIT(("hostlist_iterator_create: %s", strerror(errno)));

  while ((host = hostlist_next(itr)))
    {
      struct pingd_info *info = NULL;
      struct hostent *h;
      char *tmpstr;
      char *ip;

      if (!(info = (struct pingd_info *)malloc(sizeof(struct pingd_info))))
        ERR_EXIT(("malloc: %s", strerror(errno)));
      memset(info, '\0', sizeof(struct pingd_info));

      if (!(info->hostname = strdup(host)))
        ERR_EXIT(("strdup: %s", strerror(errno)));

      info->fd = fds[i/PINGD_NODES_PER_SOCKET];
      
      if (!(h = gethostbyname(host)))
        ERR_EXIT(("gethostbyname: %s", hstrerror(h_errno)));
        
      info->destaddr.sin_family = AF_INET;
      info->destaddr.sin_addr = *((struct in_addr *)h->h_addr);
      free(host);
      
      if (!list_append(nodes, info))
        ERR_EXIT(("list_append: %s", strerror(errno)));

      if (!(tmpstr = inet_ntoa(info->destaddr.sin_addr)))
        ERR_EXIT(("inet_ntoa: %s", strerror(errno))); /* strerror? */

      if (!(ip = strdup(tmpstr)))
        ERR_EXIT(("strdup: %s", strerror(errno)));
        
      if (hash_find(nodes_index, ip))
        ERR_EXIT(("Duplicate host ip: %s", ip));

      if (!hash_insert(nodes_index, ip, info))
        ERR_EXIT(("hash_insert: %s", strerror(errno)));

      i++;
    }

  hostlist_iterator_destroy(itr);
}

static void
_pingd_setup(void)
{
  /* Initialize pingd_next_send to 0 so there is a sweep of pings in the beginning */
  memset(&pingd_next_send, '\0', sizeof(struct timeval));

  _fds_setup();
  _nodes_setup();

  /* Avoid sigpipe exiting during server writes */
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    ERR_EXIT(("signal: %s", strerror(errno)));

}

/* From Unix Network Programming, R. Stevens Chapter 25 */
static uint16_t
_in_cksum(uint16_t *addr, unsigned int len)
{
  unsigned int nleft = len;
  uint16_t *w = addr;
  uint32_t sum = 0;
  uint16_t answer = 0;

  assert(addr);
  assert(len);

  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }

  /* mop up an odd byte, if necessary */
  if (nleft == 1) {
    *(uint8_t *)(&answer) = *(uint8_t *)w ;
    sum += answer;
  }

  /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
  sum += (sum >> 16);                     /* add carry */
  answer = ~sum;                          /* truncate to 16 bits */
  return (answer);
}

static int
_icmp_ping_build(struct pingd_info *info, char *buf, unsigned int buflen)
{
  assert(info);
  assert(buf);
  assert(buflen);

  /* 
   * ICMP Ping Packet
   * 
   * ICMP Type - 1 byte - 8 for ICMP Ping Request, 0 for ICMP Ping Reply
   * ICMP Code - 1 byte - 0 for ICMP ping 
   * ICMP Checksum - 2 bytes - initialize to 0 for checksum calculation
   * ICMP Identifier - 2 bytes - unused in pingd
   * ICMP Sequence Number - 2 bytes - used appropriately
   * ICMP Optional Data - unused in pingd
   */

  buf[0] = 8;
  buf[1] = 0;
  buf[2] = 0;
  buf[3] = 0;
  buf[4] = 0;
  buf[5] = 0;
#if WORDS_BIGENDIAN
  buf[6] = (info->sequence_number & 0xFF00) >> 8;
  buf[7] = info->sequence_number & 0xFF;
#else  /* !WORDS_BIGENDIAN */
  buf[6] = info->sequence_number & 0xFF;
  buf[7] = (info->sequence_number & 0xFF00) >> 8;
#endif /* !WORDS_BIGENDIAN */

  /* endian is not an issue for the internet checksum */
  *((uint16_t *)&buf[2]) = _in_cksum((uint16_t *)buf, 8);

  info->sequence_number++;
  return 8;
}

static void
_pingd_send_pings(void)
{
  char buf[PINGD_BUFLEN];
  int len;
  struct pingd_info *info;
  ListIterator itr;

  assert(nodes);
  assert(nodes_count);
  
  if (!(itr = list_iterator_create(nodes)))
    ERR_EXIT(("list_iterator_create: %s", strerror(errno)));

  while ((info = list_next(itr)))
    {
      memset(buf, '\0', PINGD_BUFLEN);
      if ((len = _icmp_ping_build(info, buf, PINGD_BUFLEN)) < 0)
        ERR_EXIT(("_icmp_ping_build: %s", strerror(errno)));

      if (sendto(info->fd, buf, len, 0, (struct sockaddr *)&(info->destaddr), sizeof(struct sockaddr_in)) < 0)
        ERR_EXIT(("sendto: %s", strerror(errno)));

#ifndef NDEBUG
      if (conf.debug)
        fprintf(stderr, "Ping Request to %s\n", info->hostname);
#endif /* NDEBUG */
    }

  list_iterator_destroy(itr);
}

static void
_setup_pfds(struct pollfd *pfds)
{
  int i;

  assert(pfds);

  for (i = 0; i < fds_count; i++)
    {
      pfds[i].fd = fds[i];
      pfds[i].events = POLLIN;
      pfds[i].revents = 0;
    }

  pfds[fds_count].fd = server_fd;
  pfds[fds_count].events = POLLIN;
  pfds[fds_count].revents = 0;
}

static void
_receive_ping(int fd)
{
  struct sockaddr_in from;
  struct pingd_info *info;
  char buf[PINGD_BUFLEN];
  int len;
  socklen_t fromlen = sizeof(struct sockaddr_in);
  char *tmpstr;

  /* We're happy as long as we receive something.  We don't bother
   * checking sequence numbers or anything like that.
  */

  if ((len = recvfrom(fd, buf, PINGD_BUFLEN, 0, (struct sockaddr *)&from, &fromlen)) < 0)
    ERR_EXIT(("recvfrom: %s", strerror(errno)));

  printf("%s:%d\n", __FUNCTION__, __LINE__);

  if (!(tmpstr = inet_ntoa(from.sin_addr)))
    ERR_EXIT(("inet_ntoa: %s", strerror(errno))); /* strerror? */
  
  if ((info = hash_find(nodes_index, tmpstr)))
    {
      if (gettimeofday(&(info->last_received), NULL) < 0)
        ERR_EXIT(("gettimeofday: %s", strerror(errno)));

#ifndef NDEBUG
      if (conf.debug)
        fprintf(stderr, "Ping Reply from %s\n", info->hostname);
#endif /* NDEBUG */
    }
}

static void
_send_ping_data(void)
{
  ListIterator itr;
  struct sockaddr_in rhost;
  struct pingd_info *info;
  socklen_t rhost_len = sizeof(struct sockaddr_in);
  int rhost_fd;
  
  assert(nodes);
  assert(nodes_count);

  if ((rhost_fd = accept(server_fd, (struct sockaddr *)&rhost, &rhost_len)) < 0)
    ERR_EXIT(("accept: %s", strerror(errno)));
  
#ifndef NDEBUG
  if (conf.debug)
    fprintf(stderr, "Received pingd server request\n");
#endif /* NDEBUG */
  
  if (!(itr = list_iterator_create(nodes)))
    ERR_EXIT(("list_iterator_create: %s", strerror(errno)));

  while ((info = list_next(itr)))
    {
      char buf[PINGD_BUFLEN];
      int len, n;

      len = snprintf(buf, PINGD_BUFLEN, "%s %lu\n", info->hostname, info->last_received.tv_sec);
      if (len >= PINGD_BUFLEN)
        ERR_EXIT(("len=%d", len));

      if ((n = fd_write_n(rhost_fd, buf, len)) < 0)
        {
          if (errno == EPIPE)
            break;
          else
            ERR_EXIT(("fd_write_n: %s", strerror(errno)));
        }

      if (n != len)
        ERR_EXIT(("fd_write_n: n=%d len=%d", n, len));
    }

  list_iterator_destroy(itr);
  close(rhost_fd);
}

void 
pingd_loop(void)
{
  struct pollfd *pfds = NULL;
  int i;

  _pingd_setup();

  assert(nodes_count);

  /* +1 fd for the server fd */
  if (!(pfds = (struct pollfd *)malloc((fds_count + 1)*sizeof(struct pollfd))))
    ERR_EXIT(("malloc: %s", strerror(errno)));

  while (1)
    {
      struct timeval now, timeout;
      unsigned int timeout_ms;
      int num;

      if (gettimeofday(&now, NULL) < 0)
        ERR_EXIT(("gettimeofday: %s", strerror(errno)));

      if (timeval_gt(&now, &pingd_next_send))
        {
          _pingd_send_pings();

          if (gettimeofday(&now, NULL) < 0)
            ERR_EXIT(("gettimeofday: %s", strerror(errno)));

          timeval_add_ms(&now, conf.ping_period, &pingd_next_send);
        }

      _setup_pfds(pfds);

      timeval_sub(&pingd_next_send, &now, &timeout);
      timeval_millisecond_calc(&timeout, &timeout_ms);

      if ((num = poll(pfds, fds_count + 1, timeout_ms)) < 0)
        ERR_EXIT(("poll: %s", strerror(errno)));
      
      if (num)
        {
          for (i = 0; i < fds_count; i++)
            {
              if (pfds[i].revents & POLLIN)
                _receive_ping(fds[i]);
            }

          if (pfds[fds_count].revents & POLLIN)
            _send_ping_data();
        }
    }
}
