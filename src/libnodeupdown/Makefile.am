##*****************************************************************************
## $Id: Makefile.am,v 1.29 2005-04-05 23:54:50 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
NODEUPDOWN_VERSION_SCRIPT = $(srcdir)/nodeupdown.map
NODEUPDOWN_OTHER_FLAGS = -Wl,--version-script=$(NODEUPDOWN_VERSION_SCRIPT)

NODEUPDOWN_CLUSTERLIST_VERSION_SCRIPT = $(srcdir)/nodeupdown_clusterlist.map
NODEUPDOWN_CLUSTERLIST_OTHER_FLAGS = -Wl,--version-script=$(NODEUPDOWN_CLUSTERLIST_VERSION_SCRIPT)
endif

#
# libnodeupdown
#
# no export dynamic, b/c of nodeupdown.map
#

nodeupdown_module_builddir=`cd $(top_builddir)/src/libnodeupdown && pwd`

include_HEADERS = nodeupdown.h 

noinst_HEADERS  = nodeupdown_common.h \
		  nodeupdown_ganglia.h \
		  nodeupdown_clusterlist.h \
		  nodeupdown_clusterlist_util.h \
		  nodeupdown_clusterlist_genders_util.h \
		  nodeupdown_util.h

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config \
			  -DNODEUPDOWN_MODULE_BUILDDIR=\"$(nodeupdown_module_builddir)\" \
	                  -I $(srcdir)/../libcommon \
			  -I $(srcdir)/../libexpat

libnodeupdown_la_SOURCES = nodeupdown.c \
			   nodeupdown_ganglia.c \
			   nodeupdown_clusterlist.c \
			   nodeupdown_util.c

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la \
			  ../libexpat/libexpat.la

libnodeupdown_la_LDFLAGS = -version-info @LIBNODEUPDOWN_VERSION_INFO@ \
			   $(NODEUPDOWN_OTHER_FLAGS) 

EXTRA_DIST = nodeupdown.map

#
# libnodeupdown ganglia clusterlist modules
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
	$(GENDERSLLNL_CLUSTERLIST_MODULE) \
	$(GENDERS_CLUSTERLIST_MODULE) \
	nodeupdown_clusterlist_hostsfile.la \
	nodeupdown_clusterlist_none.la

nodeupdown_clusterlist_gendersllnl_la_SOURCES = \
	nodeupdown_clusterlist_gendersllnl.c \
	nodeupdown_clusterlist_genders_util.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_gendersllnl_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERSLLNL_LIBS) \
	$(NODEUPDOWN_GANGLIA_OTHER_FLAGS) 
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
	$(NODEUPDOWN_GANGLIA_OTHER_FLAGS) 
nodeupdown_clusterlist_genders_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_clusterlist_genders_la_LIBADD = \
	../libcommon/libcommon.la
 
nodeupdown_clusterlist_hostsfile_la_SOURCES = \
	nodeupdown_clusterlist_hostsfile.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_hostsfile_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(NODEUPDOWN_GANGLIA_OTHER_FLAGS) 
nodeupdown_clusterlist_hostsfile_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_clusterlist_hostsfile_la_LIBADD = \
	../libcommon/libcommon.la
 
nodeupdown_clusterlist_none_la_SOURCES = \
	nodeupdown_clusterlist_none.c \
	nodeupdown_clusterlist_util.c
nodeupdown_clusterlist_none_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(NODEUPDOWN_GANGLIA_OTHER_FLAGS) 
nodeupdown_clusterlist_none_la_CFLAGS  = \
        -I$(srcdir)/../libcommon

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
