##*****************************************************************************
## $Id: Makefile.am,v 1.14 2004-04-27 18:05:16 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# Update this information before a public release only!
# 
# If library source code has changed at all increment rev
# (current:rev:age) = (current:rev+1:age)
#
# If any interfaces have been added,removed, or changed increment current
#  and set rev = 0;
#
# If any interfaces have been added increment age
#
# If any interfaces have been removed, set age = 0
#
current = 0
rev     = 0
age     = 0

AM_CPPFLAGS = -D_REENTRANT -I ../../config -I ../libcommon -I ../libexpat

if WITH_GNU_LD
VERSION_SCRIPT = nodeupdown.map
OTHER_FLAGS = -Wl,--version-script=$(VERSION_SCRIPT)
endif

include_HEADERS = nodeupdown.h 
lib_LTLIBRARIES = libnodeupdown.la
libnodeupdown_la_SOURCES = nodeupdown.c nodeupdown_masterlist.c
libnodeupdown_la_LIBADD = ../libcommon/libcommon.la ../libexpat/libexpat.la $(GENDERS_LIBS) $(GENDERSLLNL_LIBS)

libnodeupdown_la_LDFLAGS = -version-info $(current):$(rev):$(age) $(OTHER_FLAGS)

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
