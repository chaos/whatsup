##*****************************************************************************
## $Id: Makefile.am,v 1.12 2003-11-06 01:10:18 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

include $(top_srcdir)/config/Make-inc.mk

CPPFLAGS = -I../libnodeupdown -I ../libcommon

bin_PROGRAMS = whatsup

whatsup_SOURCES = whatsup.c

whatsup_LDADD = ../libcommon/libcommon.la ../libnodeupdown/libnodeupdown.la $(GENDERS_LIBS)
