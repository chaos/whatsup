##*****************************************************************************
## $Id: Makefile.am,v 1.15 2004-05-20 00:28:30 achu Exp $
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

if WITH_GNU_LD
VERSION_SCRIPT = nodeupdown.map
OTHER_FLAGS = -Wl,--version-script=$(VERSION_SCRIPT)
endif

include_HEADERS = nodeupdown.h 
noinst_HEADERS  = nodeupdown_masterlist.h nodeupdown_common.h
lib_LTLIBRARIES = libnodeupdown.la
libnodeupdown_la_CFLAGS = -D_REENTRANT -I ../../config -I ../libcommon -I ../libexpat
libnodeupdown_la_SOURCES = nodeupdown.c nodeupdown_masterlist.c
libnodeupdown_la_LIBADD = ../libcommon/libcommon.la ../libexpat/libexpat.la $(GENDERS_LIBS) $(GENDERSLLNL_LIBS)
libnodeupdown_la_LDFLAGS = -version-info $(current):$(rev):$(age) $(OTHER_FLAGS)
EXTRA_DIST = nodeupdown.map

../libcommon/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libexpat/libexpat.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
