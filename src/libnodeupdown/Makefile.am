##*****************************************************************************
## $Id: Makefile.am,v 1.27 2005-04-05 21:51:54 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
VERSION_SCRIPT = $(srcdir)/nodeupdown.map
OTHER_FLAGS = -Wl,--version-script=$(VERSION_SCRIPT)
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
		  nodeupdown_ganglia_clusterlist.h \
		  nodeupdown_ganglia_clusterlist_util.h \
		  nodeupdown_ganglia_clusterlist_genders_util.h \
		  nodeupdown_util.h

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config \
			  -DNODEUPDOWN_MODULE_BUILDDIR=\"$(nodeupdown_module_builddir)\" \
	                  -I $(srcdir)/../libcommon \
			  -I $(srcdir)/../libexpat

libnodeupdown_la_SOURCES = nodeupdown.c \
			   nodeupdown_ganglia.c \
			   nodeupdown_ganglia_clusterlist.c \
			   nodeupdown_util.c

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la \
			  ../libexpat/libexpat.la

libnodeupdown_la_LDFLAGS = -version-info @LIBNODEUPDOWN_VERSION_INFO@ \
			   $(OTHER_FLAGS) 

EXTRA_DIST = nodeupdown.map

#
# libnodeupdown ganglia clusterlist modules
#

MODULE_FLAGS = -module -avoid-version

if WITH_GENDERSLLNL
GENDERSLLNL_GANGLIA_CLUSTERLIST_MODULE = nodeupdown_ganglia_clusterlist_gendersllnl.la
endif
 
if WITH_GENDERS
GENDERS_GANGLIA_CLUSTERLIST_MODULE     = nodeupdown_ganglia_clusterlist_genders.la
endif

nodeupdownmodulelibdir = $(NODEUPDOWN_MODULE_DIR)
nodeupdownmodulelib_LTLIBRARIES = \
	$(GENDERSLLNL_GANGLIA_CLUSTERLIST_MODULE) \
	$(GENDERS_GANGLIA_CLUSTERLIST_MODULE) \
	nodeupdown_ganglia_clusterlist_hostsfile.la \
	nodeupdown_ganglia_clusterlist_none.la

nodeupdown_ganglia_clusterlist_gendersllnl_la_SOURCES = \
	nodeupdown_ganglia_clusterlist_gendersllnl.c \
	nodeupdown_ganglia_clusterlist_genders_util.c \
	nodeupdown_ganglia_clusterlist_util.c
nodeupdown_ganglia_clusterlist_gendersllnl_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERSLLNL_LIBS)
nodeupdown_ganglia_clusterlist_gendersllnl_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_ganglia_clusterlist_gendersllnl_la_LIBADD = \
	../libcommon/libcommon.la
 
nodeupdown_ganglia_clusterlist_genders_la_SOURCES = \
	nodeupdown_ganglia_clusterlist_genders.c \
	nodeupdown_ganglia_clusterlist_genders_util.c \
	nodeupdown_ganglia_clusterlist_util.c
nodeupdown_ganglia_clusterlist_genders_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERS_LIBS)
nodeupdown_ganglia_clusterlist_genders_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_ganglia_clusterlist_genders_la_LIBADD = \
	../libcommon/libcommon.la
 
nodeupdown_ganglia_clusterlist_hostsfile_la_SOURCES = \
	nodeupdown_ganglia_clusterlist_hostsfile.c \
	nodeupdown_ganglia_clusterlist_util.c
nodeupdown_ganglia_clusterlist_hostsfile_la_LDFLAGS = \
	$(MODULE_FLAGS)
nodeupdown_ganglia_clusterlist_hostsfile_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
nodeupdown_ganglia_clusterlist_hostsfile_la_LIBADD = \
	../libcommon/libcommon.la
 
nodeupdown_ganglia_clusterlist_none_la_SOURCES = \
	nodeupdown_ganglia_clusterlist_none.c \
	nodeupdown_ganglia_clusterlist_util.c
nodeupdown_ganglia_clusterlist_none_la_LDFLAGS = \
	$(MODULE_FLAGS)
nodeupdown_ganglia_clusterlist_none_la_CFLAGS  = \
        -I$(srcdir)/../libcommon

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
