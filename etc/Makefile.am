##*****************************************************************************
## $Id: Makefile.am,v 1.5 2010-06-29 21:16:07 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = whatsup-pingd nodeupdown.conf pingd.conf
 
install-data-local:
	$(INSTALL) -m 755 -d $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 755 $(srcdir)/nodeupdown.conf $(DESTDIR)$(sysconfdir)
if WITH_PINGD
	$(INSTALL) -m 755 -d $(DESTDIR)$(sysconfdir)/rc.d/init.d
	$(INSTALL) -m 755 $(srcdir)/pingd.conf $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 755 $(srcdir)/whatsup-pingd $(DESTDIR)$(sysconfdir)/rc.d/init.d/whatsup-pingd
endif

uninstall-local:
	$(RM) -f $(DESTDIR)/$(sysconfdir)/nodeupdown.conf
if WITH_PINGD
	$(RM) -f $(DESTDIR)/$(sysconfdir)/rc.d/init.d/whatsup-pingd
	$(RM) -f $(DESTDIR)/$(sysconfdir)/pingd.conf
endif
