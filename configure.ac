##*****************************************************************************
## $Id: configure.ac,v 1.39 2005-03-31 21:04:14 achu Exp $
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
LIBNODEUPDOWN_REVISION=0
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

#
# Check for defaults paths
#
AC_NODEUPDOWN_CONF
AC_NODEUPDOWN_MODULE_DIR
AC_WHATSUP_MODULE_DIR

#
# Check for perl install destdir
#
AC_PERL_INSTALL_DESTDIR

#
# Check for genders
#
AC_GENDERS
AM_CONDITIONAL(WITH_GENDERS, [test "$ac_with_genders" = "yes"])
 
#
# Check for gendersllnl
#
AC_GENDERSLLNL
AM_CONDITIONAL(WITH_GENDERSLLNL, [test "$ac_with_gendersllnl" = "yes"])

if test "$ac_with_genders" = "yes" ||
   test "$ac_with_gendersllnl" = "yes"; then
   AC_CHECK_LIB(genders,
                genders_index_attrvals,
                AC_DEFINE(HAVE_GENDERS_INDEX_ATTRVALS, [1],
                          [define genders_index_attrvals exists]),
                [])
fi

##
# Checks for header files.
##
AC_HEADER_STDC
AC_CHECK_HEADERS( \
  errno.h \
  fcntl.h \
  socket.h \
  time.h \
  types.h \
  stdlib.h \
  stdio.h \
  string.h \
  unistd.h \
)

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

# Do this last, you could add compile options which can make
# function checks fail
AC_DEBUG

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
  src/libnodeupdown/nodeupdown.h \
  src/whatsup/Makefile \
  src/Libnodeupdown/Makefile \
  src/Nodeupdown/Makefile \
  man/Makefile \
  man/nodeupdown.conf.5 \
  man/nodeupdown_hostsfile.conf.5 \
  man/whatsup.1 \
  src/Libnodeupdown/Libnodeupdown.pm \
  src/Nodeupdown/Nodeupdown.pm \
)
AC_OUTPUT

