##*****************************************************************************
## $Id: Makefile.am,v 1.10 2006-07-07 18:14:15 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h \
		 debug.h \
		 error.h \
		 fd.h \
		 hash.h \
		 hostlist.h \
		 list.h \
		 thread.h \
		 timeval.h 

noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = conffile.c \
		       debug.c \
		       error.c \
		       fd.c \
		       hash.c \
		       hostlist.c \
		       list.c \
		       thread.c \
		       timeval.c
libcommon_la_CFLAGS = -I../../config

libcommon_la_LIBADD = $(LIBADD_DL)
