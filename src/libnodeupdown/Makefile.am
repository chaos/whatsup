##*****************************************************************************
## $Id: Makefile.am,v 1.19 2005-01-11 00:14:21 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
VERSION_SCRIPT = $(srcdir)/nodeupdown.map
OTHER_FLAGS = -Wl,--version-script=$(VERSION_SCRIPT)
endif

include_HEADERS = nodeupdown.h 
noinst_HEADERS  = nodeupdown_masterlist.h nodeupdown_common.h
lib_LTLIBRARIES = libnodeupdown.la
libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config \
	                  -I $(srcdir)/../libcommon -I $(srcdir)/../libexpat
libnodeupdown_la_SOURCES = nodeupdown.c nodeupdown_masterlist.c
libnodeupdown_la_LIBADD = ../libcommon/libcommon.la ../libexpat/libexpat.la $(GENDERS_LIBS) $(GENDERSLLNL_LIBS)
libnodeupdown_la_LDFLAGS = -version-info @LIBNODEUPDOWN_VERSION_INFO@ $(OTHER_FLAGS)
EXTRA_DIST = nodeupdown.map

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
