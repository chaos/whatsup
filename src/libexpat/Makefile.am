##*****************************************************************************
## $Id: Makefile.am,v 1.1.2.1 2004-11-16 21:30:10 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# xmltok_impl.c and xmltok_ns.c are #included by xmltok.c (why are
# they .c files??) they should not be compiled.
 
noinst_HEADERS = ascii.h  asciitab.h  iasciitab.h  latin1tab.h  nametab.h  \
                 utf8tab.h xmldef.h  xmlparse.h  xmlrole.h  xmltok.h  xmltok_impl.h \
                 xmltok_impl.c xmltok_ns.c
 
noinst_LTLIBRARIES = libexpat.la
libexpat_la_SOURCES = xmlparse.c xmlrole.c xmltok.c