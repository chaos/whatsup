##*****************************************************************************
## $Id: Makefile.am,v 1.23 2005-04-01 17:59:01 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
VERSION_SCRIPT = $(srcdir)/nodeupdown.map
OTHER_FLAGS = -Wl,--version-script=$(VERSION_SCRIPT)
endif

include_HEADERS = nodeupdown.h 
noinst_HEADERS  = nodeupdown_common.h nodeupdown_ganglia_clusterlist.h nodeupdown_ganglia_clusterlist_util.h nodeupdown_ganglia_clusterlist_genders_util.h
lib_LTLIBRARIES = libnodeupdown.la
libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config \
	                  -I $(srcdir)/../libcommon -I $(srcdir)/../libexpat
libnodeupdown_la_SOURCES = nodeupdown.c nodeupdown_ganglia_clusterlist.c
libnodeupdown_la_LIBADD = ../libcommon/libcommon.la ../libexpat/libexpat.la $(GENDERS_LIBS) $(GENDERSLLNL_LIBS)
libnodeupdown_la_LDFLAGS = -version-info @LIBNODEUPDOWN_VERSION_INFO@ -export-dynamic $(OTHER_FLAGS)
EXTRA_DIST = nodeupdown.map

#
# Modules
#

MODULE_FLAGS = -module -avoid-version

nodeupdownmodulelibdir = $(NODEUPDOWN_MODULE_DIR)
nodeupdownmodulelib_LTLIBRARIES = \
	nodeupdown_ganglia_clusterlist_gendersllnl.la \
	nodeupdown_ganglia_clusterlist_genders.la \
	nodeupdown_ganglia_clusterlist_hostsfile.la \
	nodeupdown_ganglia_clusterlist_none.la

nodeupdown_ganglia_clusterlist_gendersllnl_la_SOURCES = \
	nodeupdown_ganglia_clusterlist_gendersllnl.c \
	nodeupdown_ganglia_clusterlist_genders_util.c \
	nodeupdown_ganglia_clusterlist_util.c
nodeupdown_ganglia_clusterlist_gendersllnl_la_LDFLAGS = \
	$(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
nodeupdown_ganglia_clusterlist_gendersllnl_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
 
nodeupdown_ganglia_clusterlist_genders_la_SOURCES = \
	nodeupdown_ganglia_clusterlist_genders.c \
	nodeupdown_ganglia_clusterlist_genders_util.c \
	nodeupdown_ganglia_clusterlist_util.c
nodeupdown_ganglia_clusterlist_genders_la_LDFLAGS = \
	$(MODULE_FLAGS) $(GENDERS_LIBS)
nodeupdown_ganglia_clusterlist_genders_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
 
nodeupdown_ganglia_clusterlist_hostsfile_la_SOURCES = \
	nodeupdown_ganglia_clusterlist_hostsfile.c \
	nodeupdown_ganglia_clusterlist_util.c
nodeupdown_ganglia_clusterlist_hostsfile_la_LDFLAGS = \
	$(MODULE_FLAGS)
nodeupdown_ganglia_clusterlist_hostsfile_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
 
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
