##*****************************************************************************
## $Id: Makefile.am,v 1.7 2004-12-29 22:41:46 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

include $(top_srcdir)/config/Make-rpm.mk

DISTCHECK_CONFIGURE_FLAGS = --with-perl-install-destdir=$$dc_install_base

SUBDIRS = src man

EXTRA_DIST = META DISCLAIMER README.expat README.genders_install  \
             README.hostsfile_install  README.troubleshooting
