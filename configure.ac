##*****************************************************************************
## $Id: configure.ac,v 1.2 2003-05-16 21:46:17 achu Exp $
##*****************************************************************************
## Process this file with autoconf to produce a configure script.
##*****************************************************************************

##
# Prologue.
##
AC_INIT([whatsup], [1.0], [chu11@llnl.gov])
AC_CONFIG_AUX_DIR([config])
AC_CANONICAL_SYSTEM

##
# Automake support.
##
AM_INIT_AUTOMAKE([check-news dist-bzip2 gnu])
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
  stdlib.h \
  stdio.h \
  string.h \
  strings.h \
  unistd.h \
  ganglia.h \
  genders.h \
  gendersllnl.h \
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
AC_CHECK_LIB(genders, genders_handle_create)
AC_CHECK_LIB(gendersllnl, genders_string_to_altnames_preserve)

##
# Epilogue.
##
AC_CONFIG_FILES( \
  Makefile \
  src/Makefile \
  src/libnodeupdown/Makefile \
  src/whatsup/Makefile \
)
AC_OUTPUT

