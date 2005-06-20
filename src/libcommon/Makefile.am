##*****************************************************************************
## $Id: Makefile.am,v 1.9 2005-06-20 22:04:40 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h \
		 error.h \
		 fd.h \
		 hostlist.h \
		 list.h \
		 ltdl.h

noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = conffile.c \
		       error.c fd.c \
		       hostlist.c \
		       list.c \
		       ltdl.c 
libcommon_la_CFLAGS = -I../../config

libcommon_la_LIBADD = $(LIBADD_DL)
