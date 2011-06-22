##*****************************************************************************
## $Id: configure.ac,v 1.70 2009-08-11 22:07:07 chu11 Exp $
##*****************************************************************************
## Process this file with autoconf to produce a configure script.
##*****************************************************************************

##
# Prologue.
##
AC_INIT([whatsup])
AC_META
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([README])
AC_CANONICAL_SYSTEM

##
# Automake support.
##
AM_INIT_AUTOMAKE($PROJECT, $VERSION)
AM_CONFIG_HEADER([config/config.h])
AM_MAINTAINER_MODE

# Update this information before a public release only!
  
# libnodeupdown versioning
#
# If library source code has changed at all increment rev
# (current:rev:age) = (current:rev+1:age)
#
# If any interfaces have been added,removed, or changed increment current
#  and set rev = 0;
#
# If any interfaces have been added increment age
#
# If any interfaces have been removed, set age = 0
#
LIBNODEUPDOWN_CURRENT=1
LIBNODEUPDOWN_REVISION=0
LIBNODEUPDOWN_AGE=1
LIBNODEUPDOWN_VERSION_INFO=$LIBNODEUPDOWN_CURRENT:$LIBNODEUPDOWN_REVISION:$LIBNODEUPDOWN_AGE
AC_SUBST([LIBNODEUPDOWN_VERSION_INFO])
  
# Libnodeupdown and Nodeupdown versioning
PERLNODEUPDOWN_MAJOR=0
PERLNODEUPDOWN_MINOR=01
PERLNODEUPDOWN_VERSION=$PERLNODEUPDOWN_MAJOR.$PERLNODEUPDOWN_MINOR
AC_SUBST([PERLNODEUPDOWN_VERSION])

##
# Checks for programs.
##
AC_PROG_CC
AC_PROG_MAKE_SET
LT_INIT([dlopen])
AC_LIB_LTDL
AM_CONDITIONAL(WITH_GNU_LD, test "$with_gnu_ld" = "yes")
AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(POD2MAN, pod2man)
AC_DEBUG

##
# Checks for header files.
##
AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS( \
  fcntl.h \
  unistd.h \
  getopt.h \
)

#
# Check for libs
#
AC_CHECK_LIB([expat], [XML_SetElementHandler], [have_expat=yes], [have_expat=no])
if test "x${have_expat}" = "xyes" ; then
   AC_DEFINE([HAVE_LIBEXPAT],[1],[Define if you have libexpat])
   EXPAT_LIBS="-lexpat"
fi
AC_SUBST(EXPAT_LIBS)
AM_CONDITIONAL(WITH_EXPAT, [test "$have_expat" = "yes"])

#
# Check for default paths
#
AC_NODEUPDOWN_CONF
AC_NODEUPDOWN_MODULE_DIR
AC_WHATSUP_MODULE_DIR
AC_PINGD_CONF
AC_PINGD_MODULE_DIR

#
# Check for perl stuff
#
AC_PERL_DESTDIR
AC_PERL_EXTENSIONS
AM_CONDITIONAL(WITH_PERL_EXTENSIONS, [test "$ac_with_perl_extensions" = "yes"])

#
# Determine Perl Install Location
#
AC_PERL_SITE_ARCH
AC_PERL_VENDOR_ARCH
if test "$ac_with_perl_site_arch" = yes && \
   test "$ac_with_perl_vendor_arch" = yes ; then
   AC_MSG_ERROR([cannot specify both perl site arch and vendor arch])
elif test "$ac_with_perl_site_arch" = yes; then
   PERL_ARCH_INSTALL=site
elif test "$ac_with_perl_vendor_arch" = yes; then
   PERL_ARCH_INSTALL=vendor
else
   PERL_ARCH_INSTALL=site
fi
AC_SUBST(PERL_ARCH_INSTALL)

#
# Check for cerebro
#
AC_CEREBRO
AM_CONDITIONAL(WITH_CEREBRO, [test "$ac_with_cerebro" = "yes"])
AM_CONDITIONAL(WITH_CEREBRO_EVENT, [test "$ac_with_cerebro_event" = "yes"])

#
# Check for ganglia build
#
AC_GANGLIA
AM_CONDITIONAL(WITH_GANGLIA, [test "$ac_with_ganglia" = "yes"])

#
# Check for pingd build
#
AC_PINGD
AM_CONDITIONAL(WITH_PINGD, [test "$ac_with_pingd" = "yes"])

#
# Check for genders
#
AC_GENDERS
AM_CONDITIONAL(WITH_GENDERS, [test "$ac_with_genders" = "yes"])

#
# Check for openib build
#
AC_OPENIB
AM_CONDITIONAL(WITH_OPENIB, [test "$ac_with_openib" = "yes"])

#
# Check for hostsfile 
#
AC_HOSTSFILE
AC_HOSTSFILE_PATH
AM_CONDITIONAL(WITH_HOSTSFILE, [test "$ac_with_hostsfile" = "yes"])

#
# Determine Perl section-3 man page extension
#
PERLMAN3EXT=`$PERL -MConfig -e 'print $Config{man3ext}'`
AC_SUBST([PERLMAN3EXT])

##
# Checks for typedefs, structures, and compiler characteristics.
##
AC_C_BIGENDIAN
AC_C_CONST
AC_TYPE_UID_T

##
# Checks for library functions.
##
AC_FUNC_MALLOC
AC_CHECK_FUNCS( \
  strcpy \
  strdup \
  strchr \ 
  strlen \ 
  strcat \ 
  strtok \ 
)
AC_CHECK_FUNCS([getopt_long])

##
# Epilogue.
##
AC_CONFIG_FILES( \
  Makefile \
  whatsup.spec \
  src/Makefile \
  src/libcommon/Makefile \
  src/libexpat/Makefile \
  src/libnodeupdown/Makefile \
  src/whatsup/Makefile \
  src/pingd/Makefile \
  src/scripts/Makefile \
  src/Libnodeupdown/Makefile \
  src/Nodeupdown/Makefile \
  man/Makefile \
  etc/Makefile
  man/libnodeupdown.3 \
  man/nodeupdown_module.3 \
  man/nodeupdown.conf.5 \
  man/whatsup.1 \
  man/pingd.8 \
  man/pingd.conf.5 \
  src/Libnodeupdown/Libnodeupdown.pm \
  src/Nodeupdown/Nodeupdown.pm \
)
AC_OUTPUT
