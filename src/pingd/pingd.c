/*****************************************************************************\
 *  $Id: pingd.c,v 1.2 2006-07-08 00:09:24 chu11 Exp $
\*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <syslog.h>
#include <sys/types.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <sys/stat.h>
#include <signal.h>

#include <assert.h>
#include <errno.h>

#include "pingd.h"
#include "pingd_config.h"
#include "pingd_loop.h"
#include "debug.h"
#include "error.h"

extern struct pingd_config conf;

static void
_daemon_init(void)
{
  /* Based on code in Unix network programming by R. Stevens */
  pid_t pid;
  int i;

  if ((pid = fork()) < 0)
    ERR_EXIT(("fork: %s", strerror(errno)));
  
  if (pid != 0)                 /* Terminate Parent */
    exit(0);
  
  setsid();
  
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
    ERR_EXIT(("signal: %s", strerror(errno)));
  
  if ((pid = fork()) < 0)
    ERR_EXIT(("fork: %s", strerror(errno)));

  if (pid != 0)                 /* Terminate 1st Child */
    exit(0);
  
  chdir("/");
  
  umask(0);
  
  for (i = 0; i < 64; i++)
    close(i);
}

int
main(int argc, char **argv)
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);

  pingd_config_setup(argc, argv);

#ifndef NDEBUG
  if (!conf.debug)
    {
      _daemon_init();
      err_set_flags(ERROR_SYSLOG);
    }
  else
    err_set_flags(ERROR_STDERR);
#else  /* NDEBUG */
  _daemon_init();
  err_set_flags(ERROR_SYSLOG);
#endif /* NDEBUG */

  /* Call after daemonization, since daemonization closes currently
   * open fds
   */
  openlog(argv[0], LOG_ODELAY | LOG_PID, LOG_DAEMON);

  pingd_loop();

  return 0;
}
