##*****************************************************************************
## $Id: Makefile.am,v 1.9 2003-11-24 16:13:19 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

CPPFLAGS = -I ../../config -I ../libcommon -I ../libdotconf -I ../libexpat

include_HEADERS = nodeupdown.h 

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_SOURCES = nodeupdown.c nodeupdown_masterlist.c

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la ../libdotconf/libdotconf.la ../libexpat/libexpat.la $(GENDERS_LIBS) $(GENDERSLLNL_LIBS)

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libdotconf/libdotconf.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
