##*****************************************************************************
## $Id: Makefile.am,v 1.3 2003-05-16 21:50:54 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

include $(top_srcdir)/config/Make-rpm.mk

ACLOCAL_AMFLAGS =       -I config
EXTRA_DIST =            AUTHORS COPYING DISCLAIMER INSTALL NEWS README
EXTRA_DIST +=           ChangeLog whatsup.spec mans
SUBDIRS =               src man

distclean-local:
	-rm -fr autom4te*.cache autoscan.*
	-find $(PACKAGE)-* -exec chmod u+w {} \;
	-rm -fr $(PACKAGE)-*
