##*****************************************************************************
## $Id: Makefile.am,v 1.38 2005-04-25 17:49:18 achu Exp $
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

if WITH_GENDERSLLNL
GENDERSLLNL_STATIC_SOURCES  = nodeupdown_clusterlist_gendersllnl.c \
			      nodeupdown_config_gendersllnl.c
GENDERS_UTIL_STATIC_SOURCES = nodeupdown_clusterlist_genders_util.c \
			      nodeupdown_genders_util.c
GENDERSLLNL_STATIC_LIBS     = $(GENDERSLLNL_LIBS)
endif
 
if WITH_GENDERS
GENDERS_STATIC_SOURCES      = nodeupdown_clusterlist_genders.c
GENDERS_UTIL_STATIC_SOURCES = nodeupdown_clusterlist_genders_util.c \
			      nodeupdown_genders_util.c
GENDERS_STATIC_LIBS         = $(GENDERS_LIBS)
endif

if WITH_STATIC_MODULES
STATIC_MODULE_SOURCES     = nodeupdown_backend_ganglia.c \
			    $(GENDERSLLNL_STATIC_SOURCES) \
			    $(GENDERS_STATIC_SOURCES) \
			    $(GENDERS_UTIL_STATIC_SOURCES) \
			    nodeupdown_clusterlist_hostsfile.c \
			    nodeupdown_clusterlist_none.c \
			    nodeupdown_clusterlist_util.c
STATIC_MODULE_LIBS        = $(GENDERSLLNL_STATIC_LIBS) $(GENDERS_STATIC_LIBS)
STATIC_MODULE_LIBEXPAT    = ../libexpat/libexpat.la
STATIC_MODULE_LIBEXPATDIR = -I$(srcdir)/../libexpat
else
MODULE_EXPORT_FLAGS    = -export-dynamic
endif

nodeupdown_module_builddir=`cd $(top_builddir)/src/libnodeupdown && pwd`

include_HEADERS = nodeupdown.h 

moduleheadersdir = $(includedir)/nodeupdown

moduleheaders = nodeupdown_backend.h \
		nodeupdown_clusterlist.h \
		nodeupdown_config.h

noinst_HEADERS  = nodeupdown_common.h \
		  nodeupdown_clusterlist_util.h \
		  nodeupdown_clusterlist_genders_util.h \
		  nodeupdown_genders_util.h \
		  nodeupdown_util.h

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config \
			  -DNODEUPDOWN_MODULE_BUILDDIR=\"$(nodeupdown_module_builddir)\" \
	                  -I $(srcdir)/../libcommon \
			  $(STATIC_MODULE_LIBEXPATDIR)

libnodeupdown_la_SOURCES = nodeupdown.c \
			   nodeupdown_backend.c \
			   nodeupdown_clusterlist.c \
			   nodeupdown_config.c \
			   nodeupdown_util.c \
			   $(STATIC_MODULE_SOURCES)

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la \
			  $(STATIC_MODULE_LIBEXPAT)

libnodeupdown_la_LDFLAGS = -version-info @LIBNODEUPDOWN_VERSION_INFO@ \
			   $(NODEUPDOWN_OTHER_FLAGS) \
			   $(MODULE_EXPORT_FLAGS) \
			   $(STATIC_MODULE_LIBEXPATDIR)

#
# libnodeupdown modules
#

MODULE_FLAGS = -module -avoid-version

if WITH_GENDERSLLNL
GENDERSLLNL_CLUSTERLIST_MODULE = nodeupdown_clusterlist_gendersllnl.la
GENDERSLLNL_CONFIG_MODULE      = nodeupdown_config_gendersllnl.la
endif
 
if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE     = nodeupdown_clusterlist_genders.la
endif

if !WITH_STATIC_MODULES
nodeupdownmodulelibdir = $(NODEUPDOWN_MODULE_DIR)
nodeupdownmodulelib_LTLIBRARIES = \
	nodeupdown_backend_ganglia.la \
	$(GENDERSLLNL_CLUSTERLIST_MODULE) \
	$(GENDERS_CLUSTERLIST_MODULE) \
	nodeupdown_clusterlist_hostsfile.la \
	nodeupdown_clusterlist_none.la \
	$(GENDERSLLNL_CONFIG_MODULE)
endif

nodeupdown_backend_ganglia_la_SOURCES = \
	nodeupdown_backend_ganglia.c
nodeupdown_backend_ganglia_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(NODEUPDOWN_BACKEND_OTHER_FLAGS) 
nodeupdown_backend_ganglia_la_CFLAGS  = \
	-I$(srcdir)/../libcommon \
	-I$(srcdir)/../libexpat
nodeupdown_backend_ganglia_la_LIBADD = \
	../libcommon/libcommon.la \
	../libexpat/libexpat.la

nodeupdown_clusterlist_gendersllnl_la_SOURCES = \
	nodeupdown_clusterlist_gendersllnl.c \
	nodeupdown_clusterlist_genders_util.c \
	nodeupdown_genders_util.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_gendersllnl_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERSLLNL_LIBS) \
	$(NODEUPDOWN_CLUSTERLIST_OTHER_FLAGS) 
nodeupdown_clusterlist_gendersllnl_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_clusterlist_gendersllnl_la_LIBADD = \
	../libcommon/libcommon.la
 
nodeupdown_clusterlist_genders_la_SOURCES = \
	nodeupdown_clusterlist_genders.c \
	nodeupdown_clusterlist_genders_util.c \
	nodeupdown_genders_util.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_genders_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERS_LIBS) \
	$(NODEUPDOWN_CLUSTERLIST_OTHER_FLAGS) 
nodeupdown_clusterlist_genders_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_clusterlist_genders_la_LIBADD = \
	../libcommon/libcommon.la
 
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
 
nodeupdown_clusterlist_none_la_SOURCES = \
	nodeupdown_clusterlist_none.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_none_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(NODEUPDOWN_CLUSTERLIST_OTHER_FLAGS) 
nodeupdown_clusterlist_none_la_CFLAGS  = \
        -I$(srcdir)/../libcommon

nodeupdown_config_gendersllnl_la_SOURCES = \
	nodeupdown_config_gendersllnl.c \
	nodeupdown_genders_util.c
nodeupdown_config_gendersllnl_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERSLLNL_LIBS) \
	$(NODEUPDOWN_CONFIG_OTHER_FLAGS)
nodeupdown_config_gendersllnl_la_CFLAGS  = \
	-I$(srcdir)/../libcommon

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = nodeupdown.map \
	     nodeupdown_backend.map \
	     nodeupdown_clusterlist.map \
	     nodeupdown_config.map
