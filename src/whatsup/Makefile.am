##*****************************************************************************
## $Id: Makefile.am,v 1.11 2003-11-06 00:53:29 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

include $(top_srcdir)/config/Make-inc.mk

CPPFLAGS = -I$(top_srcdir)/src/libnodeupdown

bin_PROGRAMS = whatsup

whatsup_SOURCES = whatsup.c hostlist.c hostlist.h

whatsup_LDADD = $(top_builddir)/src/libnodeupdown/libnodeupdown.la $(GENDERS_LIBS)
