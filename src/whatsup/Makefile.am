##*****************************************************************************
## $Id: Makefile.am,v 1.23 2005-04-07 06:12:28 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

whatsup_module_builddir=`cd $(top_builddir)/src/whatsup && pwd`

#
# whatsup
#

noinst_HEADERS = whatsup_options.h

if WITH_GENDERSLLNL
GENDERSLLNL_STATIC_SOURCES  = whatsup_options_gendersllnl.c
GENDERSLLNL_STATIC_LIBS     = $(GENDERSLLNL_LIBS)
endif

if WITH_STATIC_MODULES
STATIC_MODULES_SOURCES  = $(GENDERSLLNL_STATIC_SOURCES)
STATIC_MODULES_LIBS     = $(GENDERSLLNL_STATIC_LIBS)
endif

bin_PROGRAMS     = whatsup

whatsup_SOURCES  = whatsup.c \
		   $(STATIC_MODULES_SOURCES)

whatsup_CFLAGS   = -DWHATSUP_MODULE_BUILDDIR=\"$(whatsup_module_builddir)\" \
		   -I $(srcdir)/../libnodeupdown \
		   -I $(srcdir)/../libcommon

whatsup_LDFLAGS  = $(STATIC_MODULES_LIBS)

whatsup_LDADD    = ../libcommon/libcommon.la \
		   ../libnodeupdown/libnodeupdown.la

#
# whatsup modules
#

MODULE_FLAGS = -module -avoid-version

if WITH_GENDERSLLNL
GENDERSLLNL_OPTIONS_MODULE = whatsup_options_gendersllnl.la
endif

if !WITH_STATIC_MODULES
whatsupmodulelibdir = $(WHATSUP_MODULE_DIR)
whatsupmodulelib_LTLIBRARIES = $(GENDERSLLNL_OPTIONS_MODULE)
endif

whatsup_options_gendersllnl_la_SOURCES = whatsup_options_gendersllnl.c
whatsup_options_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
whatsup_options_gendersllnl_la_CFLAGS  = -I$(srcdir)/../libcommon
whatsup_options_gendersllnl_la_LIBADD  = ../libcommon/libcommon.la

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
