##*****************************************************************************
## $Id: Makefile.am,v 1.14 2003-11-14 00:48:58 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

CPPFLAGS = -I../libnodeupdown -I ../libcommon

bin_PROGRAMS = whatsup

whatsup_SOURCES = whatsup.c

whatsup_LDADD = ../libcommon/libcommon.la ../libnodeupdown/libnodeupdown.la $(GENDERS_LIBS)

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
