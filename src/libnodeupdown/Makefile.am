##*****************************************************************************
## $Id: Makefile.am,v 1.3 2003-11-06 00:53:29 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

CPPFLAGS = -I ../../config

include_HEADERS = \
	nodeupdown.h 

lib_LTLIBRARIES = \
	libnodeupdown.la

libnodeupdown_la_SOURCES = \
	nodeupdown.c \
	hostlist.c \
	fd.c

libnodeupdown_la_LIBADD = $(GANGLIA_LIBS) $(GENDERS_LIBS)
