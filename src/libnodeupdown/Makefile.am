##*****************************************************************************
## $Id: Makefile.am,v 1.4 2003-11-06 01:10:18 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

CPPFLAGS = -I ../../config -I ../libcommon

include_HEADERS = nodeupdown.h 

lib_LTLIBRARIES = libnodeupdown.la

libnodeupdown_la_SOURCES = nodeupdown.c

libnodeupdown_la_LIBADD = ../libcommon/libcommon.la $(GANGLIA_LIBS) $(GENDERS_LIBS)
