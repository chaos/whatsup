##*****************************************************************************
## $Id: Makefile.am,v 1.15 2004-12-20 16:39:06 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# Use Perl's ExtUtils::MakeMaker to get autoconf/automake to install
# into the correct location

# DESTDIR is usually set during make install time, not configure or
# make time, so this is our work around
PERL_INSTALL_PREFIX = @PERL_INSTALL_PREFIX@

PERL_INC = "-I../../config -I$(srcdir)/../libcommon -I$(srcdir)/../libexpat -I$(srcdir)/../libnodeupdown"

all: Makefile.xs
	$(MAKE) -f Makefile.xs

Makefile.xs: Makefile.PL Libnodeupdown.xs Libnodeupdown.pm
	cp -f $(srcdir)/../libcommon/*.c .
	cp -f $(srcdir)/../libexpat/xmlparse.c .
	cp -f $(srcdir)/../libexpat/xmlrole.c .
	cp -f $(srcdir)/../libexpat/xmltok.c .
	cp -f $(srcdir)/../libnodeupdown/*.c .
# Hacks to make distcheck work
	if ! test -a Makefile.PL; then \
		cp $(srcdir)/Makefile.PL .; \
	fi
	if ! test -a typemap; then \
		cp $(srcdir)/typemap .; \
	fi
	if ! test -a Libnodeupdown.pm; then \
		cp $(srcdir)/Libnodeupdown.pm .; \
	fi
	if ! test -a Libnodeupdown.xs; then \
		cp $(srcdir)/Libnodeupdown.xs .; \
	fi
	$(PERL) Makefile.PL PREFIX=$(PERL_INSTALL_PREFIX) MAKEFILE=Makefile.xs FIRST_MAKEFILE=Makefile.xs INC=$(PERL_INC) 

test: Makefile.xs 
	$(MAKE) -f Makefile.xs test

install: Makefile.xs
	$(MAKE) -f Makefile.xs pure_install

pure_install: Makefile.xs
	$(MAKE) -f Makefile.xs pure_install

install-data-local:
	$(MAKE) -f Makefile.xs pure_install

clean: 
	if test -a Makefile.xs; then \
		$(MAKE) -f Makefile.xs clean; \
	fi
	rm -f *.c
	rm -f *.h
	rm -f *.o
	rm -f Makefile.xs
	rm -f Makefile.xs.old

EXTRA_DIST = Libnodeupdown.pm Libnodeupdown.xs Makefile.PL typemap
