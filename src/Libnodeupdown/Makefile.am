##*****************************************************************************
## $Id: Makefile.am,v 1.2 2003-11-06 00:53:29 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# Use Perl's ExtUtils::MakeMaker to get autoconf/automake to install
# into the correct location

PERL_INSTALL_PREFIX = @PERL_INSTALL_PREFIX@

all: Makefile.xs
	$(MAKE) -f Makefile.xs

Makefile.xs: Makefile.PL Libnodeupdown.xs Libnodeupdown.pm
	cp ../libnodeupdown/nodeupdown.c .
	cp ../libnodeupdown/nodeupdown.h .
	cp ../libnodeupdown/hostlist.c .
	cp ../libnodeupdown/hostlist.h .
	cp ../libnodeupdown/fd.c .
	cp ../libnodeupdown/fd.h .
	$(PERL) Makefile.PL PREFIX=$(PERL_INSTALL_PREFIX) MAKEFILE=Makefile.xs FIRST_MAKEFILE=Makefile.xs

test: Makefile.xs 
	$(MAKE) -f Makefile.xs test

install: Makefile.xs
	$(MAKE) -f Makefile.xs pure_install

pure_install: Makefile.xs
	$(MAKE) -f Makefile.xs pure_install

install-data-local:
	$(MAKE) -f Makefile.xs pure_install

clean: 
	$(MAKE) -f Makefile.xs clean
	rm -f Makefile.xs
	rm -f Makefile.xs.old
	rm -f nodeupdown.c
	rm -f nodeupdown.h
	rm -f hostlist.c
	rm -f hostlist.h

