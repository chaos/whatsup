##*****************************************************************************
## $Id: Makefile.am,v 1.1 2006-07-07 21:00:48 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = whatsup-pingd
 
install-data-hook:
	$(top_srcdir)/config/install-sh -m 755 -d $(DESTDIR)/$(sysconfdir)/init.d
	cp $(srcdir)/whatsup-pingd $(DESTDIR)/$(sysconfdir)/init.d/whatsup-pingd; \
	chmod 755 $(DESTDIR)/$(sysconfdir)/init.d/whatsup-pingd
