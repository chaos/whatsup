##*****************************************************************************
## $Id: configure.ac,v 1.65 2006-08-30 17:10:00 chu11 Exp $
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
LIBNODEUPDOWN_CURRENT=0
LIBNODEUPDOWN_REVISION=1
LIBNODEUPDOWN_AGE=0
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
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
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
# Check for default paths
#
AC_NODEUPDOWN_CONF
AC_NODEUPDOWN_MODULE_DIR
AC_WHATSUP_MODULE_DIR
AC_PINGD_CONF
AC_PINGD_MODULE_DIR

#
# Check for perl destdir
#
AC_PERL_DESTDIR

#
# Check for cerebro
#
AC_CEREBRO
AM_CONDITIONAL(WITH_CEREBRO, [test "$ac_with_cerebro" = "yes"])

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
