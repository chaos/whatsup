##*****************************************************************************
## $Id: configure.ac,v 1.36 2005-03-31 00:27:38 achu Exp $
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

AC_NODEUPDOWN_CONF

AC_PERL_INSTALL_DESTDIR

##
# Check for libraries
##
AC_CHECK_LIB(genders, genders_handle_create, [havegenders=yes], [])
AC_CHECK_LIB(gendersllnl, genders_get_cluster, [havegendersllnl=yes], [])

##
# Check for masterlist mechanism
##
AC_MSG_CHECKING(for --with-masterlist-mechanism )
AC_ARG_WITH(masterlist-mechanism,
    AC_HELP_STRING([--with-masterlist-mechanism=type], 
                   [build with a specific master list mechanism]),
    [ case "$withval" in
    yes)
        withmasterlist=yes
        ;;
    no)
        withmasterlist=no
        ;;
    *)
        withmasterlist=yes
        MASTERLIST_MECHANISM="$withval"
        ;;
    esac ]
)
AC_MSG_RESULT(${withmasterlist=no})

if test -n "$MASTERLIST_MECHANISM" &&
   test "$MASTERLIST_MECHANISM" != "gendersllnl" &&
   test "$MASTERLIST_MECHANISM" != "genders" && 
   test "$MASTERLIST_MECHANISM" != "hostsfile" &&
   test "$MASTERLIST_MECHANISM" != "none"; then
   AC_MSG_ERROR([Invalid masterlist mechanism specified])
fi

if test "$MASTERLIST_MECHANISM" = "gendersllnl"; then
   if test "$havegenders" != "yes" || test "$havegendersllnl" != "yes"; then
     AC_MSG_ERROR([genders and gendersllnl libraries not found])
   fi
fi

if test "$MASTERLIST_MECHANISM" = "genders"; then
   if test "$havegenders" != "yes" ; then
     AC_MSG_ERROR([genders library not found])
   fi
fi

if test -z "$MASTERLIST_MECHANISM"; then
   if test "$havegendersllnl" = "yes"; then
      MASTERLIST_MECHANISM=gendersllnl
   elif test "$havegenders" = "yes"; then
      MASTERLIST_MECHANISM=genders
   else
      MASTERLIST_MECHANISM=hostsfile
   fi
fi

# Build with gendersllnl support
# - For the most part only used at LLNL
if test "$MASTERLIST_MECHANISM" = "gendersllnl"; then
   AC_DEFINE(HAVE_GENDERSLLNL, [1], [Define use of gendersllnl])
   GENDERSLLNL_LIBS="-lgenders -lgendersllnl"
   NODEUPDOWN_MASTERLIST_DEFAULT=GENDERS_DEFAULT_FILE
   NODEUPDOWN_MASTERLIST_PARAMETER="char *gendersfile" 
   NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$gendersfile' 
   NODEUPDOWN_MASTERLIST_COMMENT="if gendersfile is NULL, default genders file is used"
   MANPAGE_GENDERSLLNL=1
else
   MANPAGE_GENDERSLLNL=0
fi
AC_SUBST(GENDERSLLNL_LIBS)
AC_SUBST(HAVE_GENDERSLLNL)
AC_SUBST(MANPAGE_GENDERSLLNL)

# Build with genders support
if test "$MASTERLIST_MECHANISM" = "genders"; then
   AC_DEFINE(HAVE_GENDERS, [1], [Define use of genders])
   GENDERS_LIBS="-lgenders"
   NODEUPDOWN_MASTERLIST_DEFAULT=GENDERS_DEFAULT_FILE
   NODEUPDOWN_MASTERLIST_PARAMETER="char *gendersfile" 
   NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$gendersfile' 
   NODEUPDOWN_MASTERLIST_COMMENT="if gendersfile is NULL, default genders file is used"
   MANPAGE_GENDERS=1
else
   MANPAGE_GENDERS=0
fi
AC_SUBST(GENDERS_LIBS)
AC_SUBST(HAVE_GENDERS)
AC_SUBST(MANPAGE_GENDERS)

# Build with master list host file mechanism
if test "$MASTERLIST_MECHANISM" = "hostsfile"; then
  AC_DEFINE(HAVE_HOSTSFILE, [1], [Define use of master list])
  NODEUPDOWN_MASTERLIST_DEFAULT="\"/etc/nodeupdown_hostsfile.conf\""
  NODEUPDOWN_MASTERLIST_PARAMETER="char *hostsfile" 
  NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$hostsfile'
  NODEUPDOWN_MASTERLIST_COMMENT="if hostsfile is NULL, default hosts is used"
  MANPAGE_HOSTSFILE=1
else
  MANPAGE_HOSTSFILE=0
fi
AC_SUBST(HAVE_HOSTSFILE)
AC_SUBST(MANPAGE_HOSTSFILE)

# Build with no materlist mechanism
if test "$MASTERLIST_MECHANISM" = "nomasterlist"; then
  AC_DEFINE(HAVE_NOMASTERLIST, [1], [Define no master list])
  NODEUPDOWN_MASTERLIST_PARAMETER="void *masterlist"
  NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$masterlist'
  NODEUPDOWN_MASTERLIST_COMMENT="masterlist is reserved for future use"
  MANPAGE_NOMASTERLIST=1
else
  MANPAGE_NOMASTERLIST=0
fi
AC_SUBST(HAVE_NOMASTERLIST)
AC_SUBST(MANPAGE_NOMASTERLIST)

if test "$MASTERLIST_MECHANISM" = "gendersllnl" ||
   test "$MASTERLIST_MECHANISM" = "genders"; then
   AC_CHECK_LIB(genders,
                genders_index_attrvals, 
                AC_DEFINE(HAVE_GENDERS_INDEX_ATTRVALS, [1], 
                          [define genders_index_attrvals exists]),
                [])
fi

AC_DEFINE_UNQUOTED(NODEUPDOWN_MASTERLIST_DEFAULT, $NODEUPDOWN_MASTERLIST_DEFAULT, [Define default masterlist])
AC_SUBST(NODEUPDOWN_MASTERLIST_DEFAULT)
AC_SUBST(NODEUPDOWN_MASTERLIST_PARAMETER)
AC_SUBST(NODEUPDOWN_MASTERLIST_PERL_PARAMETER)
AC_SUBST(NODEUPDOWN_MASTERLIST_COMMENT)

##
# Checks for programs.
##
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AM_CONDITIONAL(WITH_GNU_LD, test "$with_gnu_ld" = "yes")
AC_PATH_PROG(PERL, perl)

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
  man/libnodeupdown.3 \
  man/nodeupdown_load_data.3 \
  man/nodeupdown.conf.5 \
  man/nodeupdown_hostsfile.conf.5 \
  man/whatsup.1 \
  src/Libnodeupdown/Libnodeupdown.pm \
  src/Nodeupdown/Nodeupdown.pm \
)
AC_OUTPUT

