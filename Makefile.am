##*****************************************************************************
## $Id: Makefile.am,v 1.1 2003-02-24 19:30:42 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

include $(top_srcdir)/config/Make-rpm.mk

ACLOCAL_AMFLAGS =       -I config
EXTRA_DIST =            AUTHORS COPYING DISCLAIMER INSTALL NEWS README
EXTRA_DIST +=           ChangeLog whatsup.spec
SUBDIRS =               src

distclean-local:
	-rm -fr autom4te*.cache autoscan.*
	-find $(PACKAGE)-* -exec chmod u+w {} \;
	-rm -fr $(PACKAGE)-*
