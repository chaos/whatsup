##*****************************************************************************
## $Id: Makefile.am,v 1.16 2004-06-08 15:52:36 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

bin_PROGRAMS = whatsup
whatsup_SOURCES = whatsup.c
whatsup_CFLAGS = -I ../libnodeupdown -I $(srcdir)/../libcommon
whatsup_LDADD = ../libcommon/libcommon.la ../libnodeupdown/libnodeupdown.la $(GENDERS_LIBS)

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libnodeupdown/libnodeupdown.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
