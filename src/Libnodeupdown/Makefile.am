##*****************************************************************************
## $Id: Makefile.am,v 1.5 2003-11-07 03:20:06 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# Use Perl's ExtUtils::MakeMaker to get autoconf/automake to install
# into the correct location

PERL_INSTALL_PREFIX = @PERL_INSTALL_PREFIX@

all: Makefile.xs
	$(MAKE) -f Makefile.xs

Makefile.xs: Makefile.PL Libnodeupdown.xs Libnodeupdown.pm
	cp -f ../libnodeupdown/*.c .
	cp -f ../libnodeupdown/*.h .
	cp -f ../libcommon/*.c .
	cp -f ../libcommon/*.h .
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
	rm -f *.c
	rm -f *.h
