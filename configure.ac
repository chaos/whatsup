##*****************************************************************************
## $Id: configure.ac,v 1.13 2003-11-06 01:10:18 achu Exp $
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

##
# Check for libraries
##
AC_CHECK_LIB(ganglia, XML_ParserCreate, [haveganglia=yes], [])
if test "$haveganglia" != "yes"; then
   AC_MSG_ERROR([libganglia not found!]) 
else
   GANGLIA_LIBS="-lganglia"
   AC_SUBST(GANGLIA_LIBS) 
fi
AC_CHECK_LIB(genders, genders_handle_create, [havegenders=yes], [])
AC_CHECK_LIB(gendersllnl, genders_get_cluster, [havegendersllnl=yes], [])

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

# Build with genders support
# - For the most part only used at LLNL
AC_MSG_CHECKING(for --with-genders )
AC_ARG_WITH(genders,
    AC_HELP_STRING([--with-genders=path], 
                   [build with genders support, optionally 
                    supplying genders database path]),
    [ case "$withval" in
    yes)
        withgenders=yes
        ;;
    no)
        ;;
    *)
        withgenders=yes
        ;;
    esac ]
)
AC_MSG_RESULT(${withgenders=no})
if test "$withgenders" = "yes"; then
   if test "$havegenders" != "yes" || test "$havegendersllnl" != "yes"; then
     AC_MSG_ERROR([genders library files not found])
   else
     GENDERS_LIBS="-lgenders -lgendersllnl"
     AC_DEFINE(HAVE_GENDERS, [1], [Define use of genders])
     NODEUPDOWN_MASTER_LIST_PTR="const char *genders_filename"
     WITH_GENDERS=1
   fi
else
   WITH_GENDERS=0
fi
AC_SUBST(WITH_GENDERS)
AC_SUBST(HAVE_GENDERS)
AC_SUBST(GENDERS_LIBS)

# Perl Install Prefix
# - Used almost exclusively so perl install works with rpm builds
AC_MSG_CHECKING(for --with-perl-install-prefix )
AC_ARG_WITH(perl-install-prefix,
    AC_HELP_STRING([--with-perl-install-prefix=path], [define perl modules install prefix]),
    [ case "$withval" in
    yes)
        withperllinstallprefix=yes
        ;;
    no)
        ;;
    *)
        withperllinstallprefix=yes
        PERL_INSTALL_PREFIX="$withval"
        ;;
    esac ]
)
AC_MSG_RESULT(${withperllinstallprefix=no})
if test -z "$PERL_INSTALL_PREFIX"; then
   PERL_INSTALL_PREFIX='${prefix}'
fi
AC_SUBST(PERL_INSTALL_PREFIX)

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

if test -z "$NODEUPDOWN_MASTER_LIST_PTR"; then
   NODEUPDOWN_MASTER_LIST_PTR="void *ptr"
   AC_DEFINE(HAVE_NOMASTERLIST, [1], [Define no master list])
   WITH_NOMASTERLIST=1
else
   WITH_NOMASTERLIST=0
fi
AC_SUBST(WITH_NOMASTERLIST)
AC_SUBST(NODEUPDOWN_MASTER_LIST_PTR)
AC_SUBST(NODEUPDOWN_NOMASTERLIST)

##
# Epilogue.
##
AC_CONFIG_FILES( \
  Makefile \
  whatsup.spec \
  src/Makefile \
  src/libcommon/Makefile \
  src/libnodeupdown/Makefile \
  src/libnodeupdown/nodeupdown.h
  src/whatsup/Makefile \
  src/Libnodeupdown/Makefile \
  src/Nodeupdown/Makefile \
  man/Makefile \
  man/libnodeupdown.3 \
  man/nodeupdown_load_data.3 \
  man/whatsup.1 \
)
AC_OUTPUT

