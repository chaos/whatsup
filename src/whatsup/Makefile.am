##*****************************************************************************
## $Id: Makefile.am,v 1.25 2006-01-18 21:15:43 chu11 Exp $
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

whatsupmodulelibdir = $(WHATSUP_MODULE_DIR)
whatsupmodulelib_LTLIBRARIES = 

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
