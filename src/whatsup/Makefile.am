##*****************************************************************************
## $Id: Makefile.am,v 1.8 2003-03-12 00:16:38 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

include $(top_srcdir)/config/Make-inc.mk

CPPFLAGS = -I$(top_srcdir)/src/libnodeupdown

LDADD = $(top_builddir)/src/libnodeupdown/libnodeupdown.la

bin_PROGRAMS = whatsup

whatsup_SOURCES = whatsup.c hostlist.c 

