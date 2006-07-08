/*****************************************************************************\
 *  $Id: pingd_config.c,v 1.4 2006-07-08 00:20:08 chu11 Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#if HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#include <dirent.h>
#include <assert.h>
#include <errno.h>

#include "pingd.h"
#include "pingd_config.h"

#include "conffile.h"
#include "debug.h"
#include "error.h"
#include "ltdl.h"

struct pingd_config conf;

#define CLUSTERLIST_MODULE_SIGNATURE "pingd_clusterlist"
#define CLUSTERLIST_MODULE_SYMBOL    "module_info"

static char *clusterlist_modules[] = 
  {
    "pingd_clusterlist_genders.so",
    "pingd_clusterlist_hostsfile.so",
    NULL,
  };

static void
_config_default(void)
{
  memset(&conf, '\0', sizeof(struct pingd_config));

#ifndef NDEBUG
  conf.debug = PINGD_DEBUG_DEFAULT;
#endif /* NDEBUG */
  conf.config_file = PINGD_CONF_FILE;
  conf.ping_period = PINGD_PING_PERIOD_DEFAULT;
  conf.pingd_server_port = PINGD_SERVER_PORT_DEFAULT;

  if (!(conf.hosts = hostlist_create(NULL)))
    ERR_EXIT(("hostlist_create: %s", strerror(errno)));
}

static void
_usage(void)
{
  fprintf(stderr, "Usage: cerebrod [OPTIONS]\n"
          "-h    --help          Output Help\n"
          "-v    --version       Output Version\n");
#ifndef NDEBUG
  fprintf(stderr, 
	  "-c    --config_file   Specify alternate config file\n"
          "-d    --debug         Turn on debugging and run daemon in foreground\n"); 
#endif /* NDEBUG */
  exit(0);
}

static void
_version(void)
{
  fprintf(stderr, "%s %s-%s\n", PROJECT, VERSION, RELEASE);
  exit(0);
}

static void
_cmdline_parse(int argc, char **argv)
{ 
  char options[100];
  int c;

#if HAVE_GETOPT_LONG
  struct option long_options[] =
    {
      {"help",                0, NULL, 'h'},
      {"version",             0, NULL, 'v'},
#ifndef NDEBUG
      {"config-file",         1, NULL, 'c'},
      {"debug",               0, NULL, 'd'},
#endif /* NDEBUG */
    };
#endif /* HAVE_GETOPT_LONG */

  assert(argv);

  memset(options, '\0', sizeof(options));
  strcat(options, "hv");
#ifndef NDEBUG
  strcat(options, "c:d");
#endif /* NDEBUG */

  /* turn off output messages */
  opterr = 0;

#if HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1)
#else
  while ((c = getopt(argc, argv, options)) != -1)
#endif
    {
      switch (c)
        {
        case 'h':       /* --help */
          _usage();
          break;
        case 'v':       /* --version */
          _version();
          break;
#ifndef NDEBUG
	case 'c':       /* --config-file */
          if (!(conf.config_file = strdup(optarg)))
	    ERR_EXIT(("strdup: %s", strerror(errno)));
          break;
        case 'd':       /* --debug */
          conf.debug++;
          break;
#endif /* NDEBUG */
        case '?':
        default:
          ERR_EXIT(("unknown command line option '%c'", c));
        }          
    }
}

static int
_cb_host(conffile_t cf,
	 struct conffile_data *data,
	 char *optionname,
	 int option_type,
	 void *option_ptr,
	 int option_data,
	 void *app_ptr,
	 int app_data)
{
  char *ptr;

  assert(data);

  /* Shorten hostname if necessary */
  if ((ptr = strchr(data->string, '.')))
    *ptr = '\0';
  
  if (!hostlist_push(conf.hosts, data->string))
    ERR_EXIT(("hostlist_push: %s", strerror(errno)));
  return 0;
}

static void
_config_file_parse(void)
{
  int ping_period_flag, 
    pingd_server_port_flag,
    host_flag;
  
  struct conffile_option options[] =
    {
      {
        "ping_period",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &(ping_period_flag),
        &(conf.ping_period),
        0
      },
      {
        "pingd_server_port",
        CONFFILE_OPTION_INT,
        -1,
        conffile_int,
        1,
        0,
        &(pingd_server_port_flag),
        &(conf.pingd_server_port),
        0,
      },
      {
	"host",
	CONFFILE_OPTION_STRING,
	-1,
	_cb_host,
	INT_MAX,
	0,
	&host_flag,
	NULL,
	0
      },
    };
  conffile_t cf = NULL;
  int num;

  if (!(cf = conffile_handle_create()))
    {
      ERR_DEBUG(("conffile_handle_create"));
      goto cleanup;
    }

  num = sizeof(options)/sizeof(struct conffile_option);
  if (conffile_parse(cf, conf.config_file, options, num, NULL, 0, 0) < 0)
    {
      char buf[CONFFILE_MAX_ERRMSGLEN];

      /* Its not an error if the default configuration file doesn't exist */
      if (!strcmp(conf.config_file, PINGD_CONF_FILE)
          && conffile_errnum(cf) == CONFFILE_ERR_EXIST)
	goto cleanup;

      if (conffile_errmsg(cf, buf, CONFFILE_MAX_ERRMSGLEN) < 0)
        ERR_EXIT(("conffile_parse: %d", conffile_errnum(cf)));
      else
        ERR_EXIT(("conffile_parse: %s", buf));
    }

 cleanup:
  conffile_handle_destroy(cf);
}

static int
_load_nodes(struct pingd_clusterlist_module_info *module_info)
{
  int numnodes, i, rv = -1;
  char **nodes = NULL;

  assert(module_info);

  if (!module_info->clusterlist_module_name
      || !module_info->setup
      || !module_info->cleanup
      || !module_info->get_nodes)
    {
      ERR_DEBUG(("invalid module symbols: %s", strerror(errno)));
      return -1;
    }

  if ((*(module_info->setup))() < 0)
    goto cleanup;

  if ((numnodes = ((*(module_info->get_nodes))(&nodes))) < 0)
    goto cleanup;
  
  for (i = 0; i < numnodes; i++)
    {
      if (!hostlist_push(conf.hosts, nodes[i]))
        {
          ERR_DEBUG(("hostlist_push: %s", strerror(errno)));
          goto cleanup;
        }
    }

  rv = 0;
 cleanup:
  (*(module_info->cleanup))();
  if (nodes)
    {
      for (i = 0; i < numnodes; i++)
        free(nodes[i]);
      free(nodes);
    }
  return rv;
}

static int
_load_module(char *filename)
{
  struct pingd_clusterlist_module_info *module_info;
  lt_dlhandle dl_handle = NULL;
  struct stat buf;
  int rv = -1;

  assert(filename);

  if (stat(filename, &buf) < 0)
    {
      ERR_DEBUG(("stat: filename=%s; %s", filename, strerror(errno)));
      goto cleanup;
    }

  if (!(dl_handle = lt_dlopen(filename)))
    {
      ERR_DEBUG(("lt_dlopen: filename=%s; %s", filename, strerror(errno)));
      goto cleanup;
    }
  
  /* clear lt_dlerror */
  lt_dlerror();

  if (!(module_info = lt_dlsym(dl_handle, CLUSTERLIST_MODULE_SYMBOL)))
    {
      ERR_DEBUG(("lt_dlsym: filename=%s; %s", filename, strerror(errno)));
      goto cleanup;
    }
      
  if (_load_nodes(module_info) < 0)
    goto cleanup;

  rv = 0;
 cleanup:
  if (dl_handle)
    lt_dlclose(dl_handle);
  return rv;
}

#ifndef NDEBUG
static int
_load_builddir_modules(void)
{
  int i, rv = -1;

  i = 0;
  while (clusterlist_modules[i])
    {
      char filebuf[MAXPATHLEN+1];

      memset(filebuf, '\0', MAXPATHLEN+1);
      snprintf(filebuf, MAXPATHLEN, "%s/%s/%s", MODULE_BUILDDIR, ".libs", clusterlist_modules[i]);

      if ((rv = _load_module(filebuf)) < 0)
        goto continue_loop;
      
      if (!rv)
        break;

    continue_loop:
      i++;
    }

  return rv;
}
#endif /* NDEBUG */

static int
_load_known_modules(void)
{
  int i, rv = -1;

  i = 0;
  while (clusterlist_modules[i])
    {
      char filebuf[MAXPATHLEN+1];

      memset(filebuf, '\0', MAXPATHLEN+1);
      snprintf(filebuf, MAXPATHLEN, "%s/%s", PINGD_MODULE_DIR, clusterlist_modules[i]);

      if ((rv = _load_module(filebuf)) < 0)
        goto continue_loop;

      if (!rv)
        break;

    continue_loop:
      i++;
    }

  return rv;
}

static int
_load_unknown_modules(void)
{
  struct dirent *dirent;
  DIR *dir;
  int rv = -1;

  if (!(dir = opendir(PINGD_MODULE_DIR)))
    {
      if (errno != ENOENT)
        {
          ERR_EXIT(("opendir: %s", strerror(errno)));
          goto cleanup;
        }
    }

  while ((dirent = readdir(dir)))
    {
      char *ptr;

      /* Don't bother unless its a shared object */
      ptr = strchr(dirent->d_name, '.');
      if (!ptr || strcmp(ptr, ".so"))
        continue;

      ptr = strstr(dirent->d_name, CLUSTERLIST_MODULE_SIGNATURE);
      if (ptr && ptr == &dirent->d_name[0])
        {
          char filebuf[MAXPATHLEN+1];
          memset(filebuf, '\0', MAXPATHLEN+1);
          snprintf(filebuf, MAXPATHLEN, "%s/%s", PINGD_MODULE_DIR, dirent->d_name);

          if (!(rv = _load_module(filebuf)))
            break;
        }
    }

 cleanup:
  closedir(dir);
  return rv; 
}

static void
_module_load(void)
{
  if (hostlist_count(conf.hosts))
    return;

  if (lt_dlinit() != 0)
    {
      ERR_OUTPUT(("lt_dlinit: %s", strerror(errno)));
      goto cleanup;
    }

#ifndef NDEBUG
  if (!_load_builddir_modules())
    goto cleanup;
#endif /* NDEBUG */

  if (!_load_known_modules())
    goto cleanup;

  if (!_load_unknown_modules())
    goto cleanup;

 cleanup: 
  lt_dlexit();
}

void
pingd_config_setup(int argc, char **argv)
{
  assert(argv);

  _config_default();
  _cmdline_parse(argc, argv);
  _config_file_parse();
  _module_load();

  if (!hostlist_count(conf.hosts))
    ERR_EXIT(("No nodes configured"));

  hostlist_uniq(conf.hosts);

#ifndef NDEBUG
  if (conf.debug)
    {
      char hbuf[1024];
      fprintf(stderr, "conf.debug = %d\n", conf.debug);
      fprintf(stderr, "conf.ping_period = %d\n", conf.ping_period);
      fprintf(stderr, "conf.pingd_server_port = %d\n", conf.pingd_server_port);
      hostlist_ranged_string(conf.hosts, 1024, hbuf);
      fprintf(stderr, "conf.hosts = %s\n", hbuf);
    }
#endif /* NDEBUG */
}
