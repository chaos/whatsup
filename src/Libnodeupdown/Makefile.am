##*****************************************************************************
## $Id: Makefile.am,v 1.11 2004-05-20 00:28:29 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# Use Perl's ExtUtils::MakeMaker to get autoconf/automake to install
# into the correct location

PERL_INSTALL_PREFIX = @PERL_INSTALL_PREFIX@

all: Makefile.xs
	$(MAKE) -f Makefile.xs

Makefile.xs: Makefile.PL Libnodeupdown.xs Libnodeupdown.pm
	cp -f ../libcommon/*.c .
	cp -f ../libexpat/xmlparse.c .
	cp -f ../libexpat/xmlrole.c .
	cp -f ../libexpat/xmltok.c .
	cp -f ../libnodeupdown/*.c .
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
	if test -a Makefile.xs; then $(MAKE) -f Makefile.xs clean; fi
	rm -f *.c
	rm -f *.h
	rm -f *.o
	rm -f Makefile.xs
	rm -f Makefile.xs.old

EXTRA_DIST = Libnodeupdown.pm Libnodeupdown.xs Makefile.PL typemap
