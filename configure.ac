##*****************************************************************************
## $Id: configure.ac,v 1.11 2003-09-11 16:05:58 achu Exp $
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

# default location for nodeupdown conf file
AC_MSG_CHECKING(for --with-nodeupdown-conf )
AC_ARG_WITH(nodeupdown-conf,
    AC_HELP_STRING([--with-nodeupdown-conf=path], 
                   [define location of nodeupdown configuration file]),
    [ case "$withval" in
    yes)
        withnodeupdownconf=yes
        ;;
    no)
        ;;
    *)
        withnodeupdownconf=yes
        NODEUPDOWN_CONF_FILE="\"$withval\""
        ;;
    esac ]
)
AC_MSG_RESULT(${withnodeupdownconf=no})
if test -z "$NODEUPDOWN_CONF_FILE"; then
   NODEUPDOWN_CONF_FILE="\"/etc/nodeupdown.conf\""
fi
AC_SUBST(NODEUPDOWN_CONF_FILE)

# Libnodeupdown install location
AC_MSG_CHECKING(for --with-xs-prefix )
AC_ARG_WITH(xs-prefix,
    AC_HELP_STRING([--with-xs-prefix=path], [define perl xs prefix dir]),
    [ case "$withval" in
    yes)
        withxsprefix=yes
        ;;
    no)
        ;;
    *)
        withxsprefix=yes
        XS_PREFIX="$withval"
        ;;
    esac ]
)
AC_MSG_RESULT(${withxsprefix=no})
if test -z "$XS_PREFIX"; then
   XS_PREFIX='${prefix}'
fi
AC_SUBST(XS_PREFIX)

# Nodeupdown.pm install location
AC_MSG_CHECKING(for --with-nodeupdown-pm-prefix )
AC_ARG_WITH(nodeupdown-pm-prefix,
    AC_HELP_STRING([--with-nodeupdown-pm-prefix=path], [define nodeupdown.pm prefix dir]),
    [ case "$withval" in
    yes)
        withnodeupdownpm=yes
        ;;
    no)
        ;;
    *)
        withnodeupdownpm=yes
        NODEUPDOWN_PM_PREFIX="$withval"
        ;;
    esac ]
)
AC_MSG_RESULT(${withnodeupdownpm=no})
if test -z "$NODEUPDOWN_PM_PREFIX"; then
   NODEUPDOWN_PM_PREFIX='${prefix}'
fi
AC_SUBST(NODEUPDOWN_PM_PREFIX)

##
# Checks for programs.
##
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_PATH_PROG(PERL, perl)

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
  whatsup.spec \
  src/Makefile \
  src/libnodeupdown/Makefile \
  src/whatsup/Makefile \
  man/Makefile \
  src/libnodeupdown/nodeupdown.h
  src/Libnodeupdown/Makefile \
  src/Nodeupdown/Makefile \
)
AC_OUTPUT

