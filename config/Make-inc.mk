##*****************************************************************************
## $Id: Make-inc.mk,v 1.1 2003-07-01 17:52:29 achu Exp $
##*****************************************************************************

# Dependencies to ensure libraries get rebuilt.
#
$(top_builddir)/src/libnodeupdown/libnodeupdown.la \
: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check :
