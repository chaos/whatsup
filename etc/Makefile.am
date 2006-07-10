##*****************************************************************************
## $Id: Makefile.am,v 1.2 2006-07-10 23:51:21 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = whatsup-pingd
 
install-data-hook:
if WITH_PINGD
	$(top_srcdir)/config/install-sh -m 755 -d $(DESTDIR)/$(sysconfdir)/init.d
	cp $(srcdir)/whatsup-pingd $(DESTDIR)/$(sysconfdir)/init.d/whatsup-pingd; \
	chmod 755 $(DESTDIR)/$(sysconfdir)/init.d/whatsup-pingd
endif
