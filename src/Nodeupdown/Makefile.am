##*****************************************************************************
## $Id: Makefile.am,v 1.17 2009-08-12 21:59:47 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

INSTALLXARCH="install${PERL_ARCH_INSTALL}arch"
XPREFIX="${PERL_ARCH_INSTALL}prefix"
INSTALLARCH=`$(PERL) -e 'use Config; $$T=$$Config{$(INSTALLXARCH)}; $$T=~/^\Q$$Config{$(XPREFIX)}\E\/(.+)/; print $$1;'`

if WITH_PERL_EXTENSIONS
NODEUPDOWNPMFILES = Nodeupdown.pm
NODEUPDOWNPMFILESdir = $(prefix)/$(INSTALLARCH)
NODEUPDOWNPMFILES_DATA = $(NODEUPDOWNPMFILES)
 
Nodeupdown.$(PERLMAN3EXT):
# Hacks to make distcheck work
	if ! test -a Nodeupdown.pm; then \
		cp $(srcdir)/Nodeupdown.pm .; \
	fi
	$(POD2MAN) Nodeupdown.pm Nodeupdown.$(PERLMAN3EXT)
 
man3_MANS = Nodeupdown.$(PERLMAN3EXT)
 
install-data-hook:
	chmod 444 $(DESTDIR)/$(prefix)/$(INSTALLARCH)/Nodeupdown.pm
endif
 
EXTRA_DIST = Nodeupdown.pm
