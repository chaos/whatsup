/*****************************************************************************\
 *  $Id: whatsup_options_cerebro_monitor.c,v 1.1.2.1 2006-11-09 05:40:55 chu11 Exp $
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
#include <sys/poll.h>
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
#include <cerebro.h>

#include "whatsup_options.h"
#include "error.h"

struct whatsup_option gendersllnl_options[] = {
  {
    'z',
    NULL,
    "monitor",
    "Monitor up-down changes via Cerebro Up-Down event monitoring",
    WHATSUP_OPTION_TYPE_MONITOR
  },
  {
    0,
    NULL,
    NULL,
    NULL,
    0
  }
};

static int gendersllnl_option_z_registered = 0;

#define CEREBRO_EVENT_UPDOWN    "updown"
#define CEREBRO_MONITOR_BUFLEN  4096
#define CEREBRO_STATE_UP        "UP"
#define CEREBRO_STATE_DOWN      "DOWN"

/* 
 * gendersllnl_options_setup
 * 
 * gendersllnl setup func
 */
int 
gendersllnl_options_setup(void)
{
  return 0;
}

/* 
 * gendersllnl_options_cleanup
 * 
 * gendersllnl cleanup func
 */
int 
gendersllnl_options_cleanup(void)
{
  return 0;
}

/* 
 * gendersllnl_options_process_option
 * 
 * gendersllnl process_option func
 */
int
gendersllnl_options_process_option(char c, char *optarg)
{
  if (c != 'z')
    return -1;

  gendersllnl_list_z_registered++;
  return 0;
}


/* 
 * gendersllnl_options_monitor
 * 
 * gendersllnl monitor func
 */
int
gendersllnl_options_monitor(void)
{
  cerebro_t handle;
  int fd;

  if (!(handle = cerebro_handle_create()))
    err_exit("cerebro_handle_create()");


  if ((fd = cerebro_event_register(handle, CEREBRO_EVENT_UPDOWN)) < 0)
    err_exit("cerebro_event_register: %s", cerebro_strerror(cerebro_errnum(handle)));

  while (1)
    {
      struct pollfd pfd;
      int n;

      pfd.fd = fd;
      pfd.events = POLLIN;
      pfd.revents = 0;

      if ((n = poll(&pfd, 1, -1)) < 0)
        err_exit("poll: %s\n", strerror(errno));

      if (n && pfd.revents & POLLIN)
        {
          char *nodename;
          unsigned int event_value_type;
          unsigned int event_value_len;
          void *event_value;
          time_t t;
          struct tm *tm;

          if (cerebro_event_parse(handle,
                                  fd,
                                  &nodename,
                                  &event_value_type,
                                  &event_value_len,
                                  &event_value) < 0)
            {
              char *msg = ;

              _clean_err_exit(cerebro_errnum(handle));
              err_exit("cerebro_event_parse: %s", cerebro_strerror(cerebro_errnum(handle)));
            }

          memset(vbuf, '\0', CEREBRO_STAT_BUFLEN);

          if (event_value_type == CEREBRO_DATA_VALUE_TYPE_INT32
              && event_value_len == sizeof(int32_t))
            {
              int32_t *stateptr = (int32_t *)event_value;
              char tbuf[CEREBRO_MONITOR_BUFLEN];
              char *statestr;

              memset(tbuf, '\0', CEREBRO_STAT_BUFLEN);
              t = time(NULL);

              /* achu: I'm not sure if this sets errno, oh well */
              if (!(tm = localtime(&t)))
                err_exit("localtime: %s", strerror(errno));
              
              strftime(tbuf, CEREBRO_STAT_BUFLEN, "%F %I:%M:%S%P", tm);
              
              if (*stateptr)
                statestr = CEREBRO_STATE_UP;
              else
                statestr = CEREBRO_STATE_DOWN;

              fprintf(stdout, 
                      "%s(%s): %s\n", 
                      nodename,
                      tbuf, 
                      statestr);
              
            }

          if (nodename)
            free(nodename);
          if (event_value)
            free(event_value);
        }
    }

  (void)cerebro_event_unregister(handle, fd);
}

struct whatsup_options_module_info whatsup_module_info = 
  {
    "gendersllnl",
    &gendersllnl_options[0],
    &gendersllnl_options_setup,
    &gendersllnl_options_cleanup,
    &gendersllnl_options_process_option,
    NULL,
    NULL,
    &gendersllnl_options_monitor,
  };
