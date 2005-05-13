##*****************************************************************************
## $Id: Makefile.am,v 1.12 2005-05-13 23:38:35 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

INSTALLSITEARCH=`perl -e 'use Config; $$T=$$Config{installsitearch}; $$T=~/\/usr\/(lib|lib64)\/(.*)/; print $$2;'`
 
NODEUPDOWNPMFILES = Nodeupdown.pm
NODEUPDOWNPMFILESdir = $(libdir)/$(INSTALLSITEARCH)
NODEUPDOWNPMFILES_DATA = $(NODEUPDOWNPMFILES)
 
Nodeupdown.3pm:
	$(POD2MAN) Nodeupdown.pm Nodeupdown.3pm
 
man3_MANS = Nodeupdown.3pm
 
install-data-hook:
	chmod 444 $(DESTDIR)/$(libdir)/$(INSTALLSITEARCH)/Nodeupdown.pm
 
EXTRA_DIST = Nodeupdown.pm
