##*****************************************************************************
## $Id: Makefile.am,v 1.1 2003-08-18 16:28:01 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

# We will cheat and use ExtUtils::MakeMaker to install this
# Perl Module instead of doing some whacked out autoconf

NODEUPDOWN_PM_PREFIX = @NODEUPDOWN_PM_PREFIX@

all: Makefile.pm
	$(MAKE) -f Makefile.pm

Makefile.pm: Makefile.PL
	$(PERL) Makefile.PL PREFIX=$(NODEUPDOWN_PM_PREFIX) MAKEFILE=Makefile.pm FIRST_MAKEFILE=Makefile.pm

install: Makefile.pm
	$(MAKE) -f Makefile.pm pure_install

pure_install: Makefile.pm
	$(MAKE) -f Makefile.pm pure_install

install-data-local:
	$(MAKE) -f Makefile.pm pure_install

clean:
	$(MAKE) -f Makefile.pm clean
	rm -f Makefile.pm
	rm -f Makefile.pm.old
