##*****************************************************************************
## $Id: Makefile.am,v 1.15 2009-08-11 21:51:41 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

INSTALLSITEARCH=`perl -e 'use Config; $$T=$$Config{installsitearch}; print $$T;'`
 
if WITH_PERL_EXTENSIONS
NODEUPDOWNPMFILES = Nodeupdown.pm
NODEUPDOWNPMFILESdir = $(INSTALLSITEARCH)
NODEUPDOWNPMFILES_DATA = $(NODEUPDOWNPMFILES)
 
Nodeupdown.3pm:
# Hacks to make distcheck work
	if ! test -a Nodeupdown.pm; then \
		cp $(srcdir)/Nodeupdown.pm .; \
	fi
	$(POD2MAN) Nodeupdown.pm Nodeupdown.3pm
 
man3_MANS = Nodeupdown.3pm
 
install-data-hook:
	chmod 444 $(DESTDIR)/$(INSTALLSITEARCH)/Nodeupdown.pm
endif
 
EXTRA_DIST = Nodeupdown.pm
