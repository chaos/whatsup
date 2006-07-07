/*****************************************************************************\
 *  $Id: pingd_config.h,v 1.2 2006-07-07 21:00:48 chu11 Exp $
\*****************************************************************************/

#ifndef _PINGD_CONFIG_H
#define _PINGD_CONFIG_H

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "hostlist.h"

#define PINGD_DEBUG_DEFAULT           0
#define PINGD_PING_PERIOD_DEFAULT     15000
#define PINGD_SERVER_PORT_DEFAULT     9125

struct pingd_config
{
#ifndef NDEBUG
  int debug;
#endif /* NDEBUG */
  char *config_file;

  int ping_period;
  int pingd_server_port;
  hostlist_t hosts;
};

void pingd_config_setup(int argc, char **argv);

#endif /* _PINGD_CONFIG_H */
