##*****************************************************************************
## $Id: Makefile.am,v 1.5 2005-03-31 18:29:29 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h fd.h hostlist.h list.h ltdl.h 
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = conffile.c fd.c hostlist.c list.c ltdl.c 
libcommon_la_CFLAGS = -I../../config
libcommon_la_LIBADD = $(LIBADD_DL)
