##*****************************************************************************
## $Id: Makefile.am,v 1.15 2004-05-20 00:28:30 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

bin_PROGRAMS = whatsup
whatsup_SOURCES = whatsup.c
whatsup_CFLAGS = -I../libnodeupdown -I ../libcommon
whatsup_LDADD = ../libcommon/libcommon.la ../libnodeupdown/libnodeupdown.la $(GENDERS_LIBS)

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
