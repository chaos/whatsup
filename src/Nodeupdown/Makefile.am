##*****************************************************************************
## $Id: Makefile.am,v 1.16 2009-08-11 22:07:07 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

INSTALLSITEARCH=`perl -e 'use Config; $$T=$$Config{installsitearch}; print $$T;'`
 
if WITH_PERL_EXTENSIONS
NODEUPDOWNPMFILES = Nodeupdown.pm
NODEUPDOWNPMFILESdir = $(INSTALLSITEARCH)
NODEUPDOWNPMFILES_DATA = $(NODEUPDOWNPMFILES)
 
Nodeupdown.$(PERLMAN3EXT):
# Hacks to make distcheck work
	if ! test -a Nodeupdown.pm; then \
		cp $(srcdir)/Nodeupdown.pm .; \
	fi
	$(POD2MAN) Nodeupdown.pm Nodeupdown.$(PERLMAN3EXT)
 
man3_MANS = Nodeupdown.$(PERLMAN3EXT)
 
install-data-hook:
	chmod 444 $(DESTDIR)/$(INSTALLSITEARCH)/Nodeupdown.pm
endif
 
EXTRA_DIST = Nodeupdown.pm
