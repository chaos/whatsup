##*****************************************************************************
## $Id: Makefile.am,v 1.13 2003-11-07 19:10:04 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

include $(top_srcdir)/config/Make-inc.mk

CPPFLAGS = -I../libnodeupdown -I ../libcommon

bin_PROGRAMS = whatsup

whatsup_SOURCES = whatsup.c

whatsup_LDADD = ../libcommon/libcommon.la ../libnodeupdown/libnodeupdown.la $(GENDERS_LIBS)

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
