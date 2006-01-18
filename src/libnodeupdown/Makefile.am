##*****************************************************************************
## $Id: Makefile.am,v 1.57 2006-01-18 21:15:43 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
NODEUPDOWN_VERSION_SCRIPT = $(srcdir)/nodeupdown.map
NODEUPDOWN_OTHER_FLAGS = -Wl,--version-script=$(NODEUPDOWN_VERSION_SCRIPT)

NODEUPDOWN_BACKEND_VERSION_SCRIPT = $(srcdir)/nodeupdown_backend.map
NODEUPDOWN_BACKEND_OTHER_FLAGS = -Wl,--version-script=$(NODEUPDOWN_BACKEND_VERSION_SCRIPT)

NODEUPDOWN_CLUSTERLIST_VERSION_SCRIPT = $(srcdir)/nodeupdown_clusterlist.map
NODEUPDOWN_CLUSTERLIST_OTHER_FLAGS = -Wl,--version-script=$(NODEUPDOWN_CLUSTERLIST_VERSION_SCRIPT)

NODEUPDOWN_CONFIG_VERSION_SCRIPT = $(srcdir)/nodeupdown_config.map
NODEUPDOWN_CONFIG_OTHER_FLAGS = -Wl,--version-script=$(NODEUPDOWN_CONFIG_VERSION_SCRIPT)
endif

#
# libnodeupdown
#

MODULE_EXPORT_FLAGS       = -export-dynamic

nodeupdown_module_builddir=`cd $(top_builddir)/src/libnodeupdown && pwd`

include_HEADERS = nodeupdown.h 

nobase_include_HEADERS = nodeupdown/nodeupdown_backend_module.h \
			 nodeupdown/nodeupdown_clusterlist_module.h \
			 nodeupdown/nodeupdown_config.h \
			 nodeupdown/nodeupdown_config_module.h \
			 nodeupdown/nodeupdown_constants.h \
			 nodeupdown/nodeupdown_devel.h

noinst_HEADERS  = nodeupdown_api.h \
		  nodeupdown_clusterlist_util.h \
		  nodeupdown_clusterlist_genders_util.h \
		  nodeupdown_genders_util.h \
		  nodeupdown_module.h \
		  nodeupdown_util.h

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config \
			  -DNODEUPDOWN_MODULE_BUILDDIR=\"$(nodeupdown_module_builddir)\" \
	                  -I $(srcdir)/../libcommon

libnodeupdown_la_SOURCES = nodeupdown.c \
	 		   nodeupdown_clusterlist_default.c \
	 		   nodeupdown_config_default.c \
			   nodeupdown_devel.c \
			   nodeupdown_module.c \
			   nodeupdown_util.c

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la

libnodeupdown_la_LDFLAGS = -version-info @LIBNODEUPDOWN_VERSION_INFO@ \
			   $(NODEUPDOWN_OTHER_FLAGS) \
			   $(MODULE_EXPORT_FLAGS)

#
# libnodeupdown modules
#

MODULE_FLAGS = -module -avoid-version

if WITH_CEREBRO
CEREBRO_BACKEND_MODULE         = nodeupdown_backend_cerebro.la
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE     = nodeupdown_clusterlist_genders.la
endif

if WITH_HOSTSFILE
HOSTSFILE_CLUSTERLIST_MODULE   = nodeupdown_clusterlist_hostsfile.la
endif

nodeupdownmodulelibdir = $(NODEUPDOWN_MODULE_DIR)
nodeupdownmodulelib_LTLIBRARIES = \
	nodeupdown_backend_ganglia.la \
	$(CEREBRO_BACKEND_MODULE) \
	$(GENDERS_CLUSTERLIST_MODULE) \
	$(HOSTSFILE_CLUSTERLIST_MODULE)

nodeupdown_backend_ganglia_la_SOURCES = \
	nodeupdown_backend_ganglia.c
nodeupdown_backend_ganglia_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(NODEUPDOWN_BACKEND_OTHER_FLAGS) 
nodeupdown_backend_ganglia_la_CFLAGS  = \
	-I$(srcdir)/../libexpat
nodeupdown_backend_ganglia_la_LIBADD = \
	../libexpat/libexpat.la

nodeupdown_backend_cerebro_la_SOURCES = \
	nodeupdown_backend_cerebro.c
nodeupdown_backend_cerebro_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(CEREBRO_LIBS) \
	$(NODEUPDOWN_BACKEND_OTHER_FLAGS) 

nodeupdown_clusterlist_genders_la_SOURCES = \
	nodeupdown_clusterlist_genders.c \
	nodeupdown_clusterlist_genders_util.c \
	nodeupdown_genders_util.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_genders_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERS_LIBS) \
	$(NODEUPDOWN_CLUSTERLIST_OTHER_FLAGS) 
 
nodeupdown_clusterlist_hostsfile_la_SOURCES = \
	nodeupdown_clusterlist_hostsfile.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_hostsfile_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(NODEUPDOWN_CLUSTERLIST_OTHER_FLAGS) 
nodeupdown_clusterlist_hostsfile_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_clusterlist_hostsfile_la_LIBADD = \
	../libcommon/libcommon.la
 
../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = nodeupdown.map \
	     nodeupdown_backend.map \
	     nodeupdown_clusterlist.map \
	     nodeupdown_config.map
