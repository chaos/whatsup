##*****************************************************************************
## $Id: Makefile.am,v 1.1 2006-07-07 18:14:16 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
PINGD_CLUSTERLIST_VERSION_SCRIPT = $(srcdir)/pingd_clusterlist.map
PINGD_CLUSTERLIST_OTHER_FLAGS = -Wl,--version-script=$(PINGD_CLUSTERLIST_VERSION_SCRIPT)
endif

module_builddir=`cd $(top_builddir)/src/pingd/ && pwd`

noinst_HEADERS = \
	pingd.h \
	pingd_config.h \
	pingd_loop.h

sbin_PROGRAMS = pingd

pingd_SOURCES = \
	pingd.c \
	pingd_config.c \
	pingd_loop.c

pingd_CFLAGS = \
	-DMODULE_BUILDDIR=\"$(module_builddir)\" \
	-I$(srcdir)/../include \
	-I$(srcdir)/../libcommon 

pingd_LDADD = ../libcommon/libcommon.la

#
# modules
#

MODULE_FLAGS = -module -avoid-version

if WITH_GENDERSLLNL
GENDERSLLNL_CLUSTERLIST_MODULE = pingd_clusterlist_gendersllnl.la
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE     = pingd_clusterlist_genders.la
endif

if WITH_HOSTSFILE
HOSTSFILE_CLUSTERLIST_MODULE   = pingd_clusterlist_hostsfile.la
endif

pingdmodulelibdir = $(PINGD_MODULE_DIR)
pingdmodulelib_LTLIBRARIES = \
	$(GENDERSLLNL_CLUSTERLIST_MODULE) \
	$(GENDERS_CLUSTERLIST_MODULE) \
	$(HOSTSFILE_CLUSTERLIST_MODULE)

pingd_clusterlist_gendersllnl_la_SOURCES = \
	pingd_clusterlist_gendersllnl.c
pingd_clusterlist_gendersllnl_la_CFLAGS = \
	-I$(srcdir)/../libcommon
pingd_clusterlist_gendersllnl_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERSLLNL_LIBS) \
	$(PINGD_CLUSTERLIST_OTHER_FLAGS)
pingd_clusterlist_gendersllnl_la_LIBADD = \
	../libcommon/libcommon.la

pingd_clusterlist_genders_la_SOURCES = \
	pingd_clusterlist_genders.c
pingd_clusterlist_genders_la_CFLAGS = \
	-I$(srcdir)/../libcommon
pingd_clusterlist_genders_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(GENDERS_LIBS) \
	$(PINGD_CLUSTERLIST_OTHER_FLAGS)
pingd_clusterlist_genders_la_LIBADD = \
	../libcommon/libcommon.la

pingd_clusterlist_hostsfile_la_SOURCES = \
	pingd_clusterlist_hostsfile.c
pingd_clusterlist_hostsfile_la_CFLAGS  = \
	-I$(srcdir)/../libcommon
pingd_clusterlist_hostsfile_la_LDFLAGS = \
	$(MODULE_FLAGS) \
	$(PINGD_CLUSTERLIST_OTHER_FLAGS)
pingd_clusterlist_hostsfile_la_LIBADD = \
	../libcommon/libcommon.la

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check: