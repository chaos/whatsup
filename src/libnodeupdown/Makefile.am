##*****************************************************************************
## $Id: Makefile.am,v 1.7 2003-11-07 19:10:04 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

CPPFLAGS = -I ../../config -I ../libcommon

include_HEADERS = nodeupdown.h 

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_SOURCES = nodeupdown.c nodeupdown_common.c nodeupdown_masterlist.c

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la $(GANGLIA_LIBS) $(GENDERS_LIBS)

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
