##*****************************************************************************
## $Id: Makefile.am,v 1.12 2004-06-08 16:18:13 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# Use Perl's ExtUtils::MakeMaker to get autoconf/automake to install
# into the correct location

PERL_INSTALL_PREFIX = @PERL_INSTALL_PREFIX@

all: Makefile.xs
	$(MAKE) -f Makefile.xs

Makefile.xs: Makefile.PL Libnodeupdown.xs Libnodeupdown.pm
	cp -f $(srcdir)/../libcommon/*.c .
	cp -f $(srcdir)/../libexpat/xmlparse.c .
	cp -f $(srcdir)/../libexpat/xmlrole.c .
	cp -f $(srcdir)/../libexpat/xmltok.c .
	cp -f $(srcdir)/../libnodeupdown/*.c .
	$(PERL) $(srcdir)/Makefile.PL PREFIX=$(PERL_INSTALL_PREFIX) MAKEFILE=Makefile.xs FIRST_MAKEFILE=Makefile.xs

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
