##*****************************************************************************
## $Id: Makefile.am,v 1.25.2.1 2006-11-09 05:40:55 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

whatsup_module_builddir=`cd $(top_builddir)/src/whatsup && pwd`

#
# whatsup
#

noinst_HEADERS = whatsup_options.h

bin_PROGRAMS     = whatsup

whatsup_SOURCES  = whatsup.c

whatsup_CFLAGS   = -DWHATSUP_MODULE_BUILDDIR=\"$(whatsup_module_builddir)\" \
		   -I $(srcdir)/../libnodeupdown \
		   -I $(srcdir)/../libcommon

whatsup_LDADD    = ../libcommon/libcommon.la \
		   ../libnodeupdown/libnodeupdown.la

#
# whatsup modules
#

MODULE_FLAGS = -module -avoid-version

if WITH_CEREBRO_EVENT
CEREBRO_MONITOR_OPTIONS_MODULE = whatsup_options_cerebro_monitor.la
endif


whatsupmodulelibdir = $(WHATSUP_MODULE_DIR)
whatsupmodulelib_LTLIBRARIES = $(CEREBRO_MONITOR_OPTIONS_MODULE)

whatsup_options_cerebro_monitor_la_SOURCES = whatsup_options_cerebro_monitor.c
whatsup_options_cerebro_monitor_la_LDFLAGS = $(MODULE_FLAGS) $(CEREBRO_LIBS)
whatsup_options_cerebro_monitor_la_CFLAGS  = -I$(srcdir)/../libcommon
whatsup_options_cerebro_monitor_la_LIBADD  = ../libcommon/libcommon.la

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
