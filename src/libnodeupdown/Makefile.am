##*****************************************************************************
## $Id: Makefile.am,v 1.32 2005-04-06 21:50:19 achu Exp $
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
endif

#
# libnodeupdown
#

nodeupdown_module_builddir=`cd $(top_builddir)/src/libnodeupdown && pwd`

include_HEADERS = nodeupdown.h 

noinst_HEADERS  = nodeupdown_backend.h \
		  nodeupdown_common.h \
		  nodeupdown_clusterlist.h \
		  nodeupdown_clusterlist_util.h \
		  nodeupdown_clusterlist_genders_util.h \
		  nodeupdown_util.h

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config \
			  -DNODEUPDOWN_MODULE_BUILDDIR=\"$(nodeupdown_module_builddir)\" \
	                  -I $(srcdir)/../libcommon

libnodeupdown_la_SOURCES = nodeupdown.c \
			   nodeupdown_backend.c \
			   nodeupdown_clusterlist.c \
			   nodeupdown_util.c

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la

libnodeupdown_la_LDFLAGS = -export-dynamic \
			   -version-info @LIBNODEUPDOWN_VERSION_INFO@ \
			   $(NODEUPDOWN_OTHER_FLAGS) 

EXTRA_DIST = nodeupdown.map

#
# libnodeupdown modules
#

MODULE_FLAGS = -module -avoid-version

if WITH_GENDERSLLNL
GENDERSLLNL_CLUSTERLIST_MODULE = nodeupdown_clusterlist_gendersllnl.la
endif
 
if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE     = nodeupdown_clusterlist_genders.la
endif

nodeupdownmodulelibdir = $(NODEUPDOWN_MODULE_DIR)
nodeupdownmodulelib_LTLIBRARIES = \
	nodeupdown_backend_ganglia.la \
	$(GENDERSLLNL_CLUSTERLIST_MODULE) \
	$(GENDERS_CLUSTERLIST_MODULE) \
	nodeupdown_clusterlist_hostsfile.la \
	nodeupdown_clusterlist_none.la

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

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
