##*****************************************************************************
## $Id: configure.ac,v 1.5 2003-05-27 18:16:28 achu Exp $
##*****************************************************************************
## Process this file with autoconf to produce a configure script.
##*****************************************************************************

##
# Prologue.
##
AC_INIT
AC_CONFIG_AUX_DIR([config])
AC_CANONICAL_SYSTEM

##
# Determine Project/Version/Release
##
PROJECT="`perl -ne 'print,exit if s/^\s*NAME:\s*(\S*).*/\1/i' $srcdir/META`"
AC_DEFINE_UNQUOTED(PROJECT, "$PROJECT", [Define the project's name.])
AC_SUBST(PROJECT)

# Automake desires "PACKAGE" variable instead of PROJECT
PACKAGE=$PROJECT

VERSION="`perl -ne 'print,exit if s/^\s*VERSION:\s*(\S*).*/\1/i' $srcdir/META`"
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Define the project's version.])
AC_SUBST(VERSION)

RELEASE="`perl -ne 'print,exit if s/^\s*RELEASE:\s*(\S*).*/\1/i' $srcdir/META`"
AC_DEFINE_UNQUOTED(RELEASE, "$RELEASE", [Define the project's release.])
AC_SUBST(RELEASE)

##
# Automake support.
##
AM_INIT_AUTOMAKE($PROJECT, $VERSION)
AM_CONFIG_HEADER([config/config.h])
AM_MAINTAINER_MODE

##
# Checks for programs.
##
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET

##
# Checks for header files.
##
AC_HEADER_STDC
AC_CHECK_HEADERS( \
  errno.h \
  fcntl.h \
  gendersllnl.h \
  ganglia.h \
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

##
# Check for genders and ganglia libraries that we need
##
AC_CHECK_LIB(ganglia, XML_ParserCreate)
AC_CHECK_LIB(gendersllnl, genders_string_to_altnames_preserve)

##
# Epilogue.
##
AC_CONFIG_FILES( \
  Makefile \
  src/Makefile \
  src/libnodeupdown/Makefile \
  src/whatsup/Makefile \
  man/Makefile 
)
AC_OUTPUT

