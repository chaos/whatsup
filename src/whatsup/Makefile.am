##*****************************************************************************
## $Id: Makefile.am,v 1.17 2005-04-02 05:51:16 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

whatsup_module_builddir=`cd $(top_builddir)/src/libnodeupdown && pwd`

bin_PROGRAMS = whatsup

whatsup_SOURCES = whatsup.c

whatsup_CFLAGS = -export-dynamic \
		 -DWHATSUP_MODULE_BUILDDIR=\"$(whatsup_module_builddir)\" \
		 -I ../libnodeupdown \
		 -I $(srcdir)/../libcommon

whatsup_LDADD = ../libcommon/libcommon.la \
		../libnodeupdown/libnodeupdown.la

MODULE_FLAGS = -module -avoid-version

if WITH_GENDERSLLNL
GENDERSLLNL_OPTIONS_MODULE = whatsup_options_gendersllnl.la
endif

whatsupmodulelibdir = $(WHATSUP_MODULE_DIR)
whatsupmodulelib_LTLIBRARIES = $(GENDERSLLNL_OPTIONS_MODULE)

whatsup_options_gendersllnl_la_SOURCES = whatsup_options_gendersllnl.c
whatsup_options_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
whatsup_options_gendersllnl_la_CFLAGS  = -I$(srcdir)/../libcommon

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
