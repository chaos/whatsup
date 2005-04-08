##*****************************************************************************
## $Id: Makefile.am,v 1.8 2005-04-08 01:01:11 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if !WITH_STATIC_MODULES
LTDL_HEADERFILES = ltdl.h
LTDL_SOURCES     = ltdl.c
LTDL_LIBADD      = $(LIBADD_DL)
endif

noinst_HEADERS = conffile.h \
		 error.h \
		 fd.h \
		 hostlist.h \
		 list.h \
		 $(LTDL_HEADERFILES)

noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = conffile.c \
		       error.c fd.c \
		       hostlist.c \
		       list.c \
		       $(LTDL_SOURCES)

libcommon_la_CFLAGS = -I../../config

libcommon_la_LIBADD = $(LTDL_LIBADD)
