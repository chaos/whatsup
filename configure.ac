##*****************************************************************************
## $Id: configure.ac,v 1.32 2004-10-28 22:08:32 achu Exp $
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
        NODEUPDOWN_CONF_FILE=$withval
        ;;
    esac ]
)
AC_MSG_RESULT(${withnodeupdownconf=no})
if test -z "$NODEUPDOWN_CONF_FILE"; then
   NODEUPDOWN_CONF_FILE=/etc/nodeupdown.conf
fi
AC_DEFINE_UNQUOTED(NODEUPDOWN_CONF_FILE, "$NODEUPDOWN_CONF_FILE", [Define nodeupdown conf file])
AC_SUBST(NODEUPDOWN_CONF_FILE)

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

# Build with no master list mechanism
AC_MSG_CHECKING(for --with-nomasterlist )
AC_ARG_WITH(nomasterlist,
    AC_HELP_STRING([--with-nomasterlist=path], 
                   [build with no master list mechanism at all]),
    [ case "$withval" in
    yes)
        withnomasterlist=yes
        ;;
    no)
        ;;
    *)
        withnomasterlist=yes
        ;;
    esac ]
)
AC_MSG_RESULT(${withnomasterlist=no})

# Build with master list host file mechanism
AC_MSG_CHECKING(for --with-hostsfile )
AC_ARG_WITH(hostsfile,
    AC_HELP_STRING([--with-hostsfile=path], 
                   [build with master list host file support, optionally 
                    supplying master list host file default path]),
    [ case "$withval" in
    yes)
        withhostsfile=yes
        ;;
    no)
        ;;
    *)
        withhostsfile=yes
        ;;
    esac ]
)
AC_MSG_RESULT(${withhostsfile=no})

# Build with genders master list mechanism
AC_MSG_CHECKING(for --with-genders )
AC_ARG_WITH(genders,
    AC_HELP_STRING([--with-genders=path], 
                   [build with genders support, optionally 
                    supplying genders database default path]),
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

# Build with gendersllnl master list mechanism
AC_MSG_CHECKING(for --with-gendersllnl )
AC_ARG_WITH(gendersllnl,
    AC_HELP_STRING([--with-gendersllnl=path], 
                   [build with gendersllnl support, optionally 
                    supplying genders database default path]),
    [ case "$withval" in
    yes)
        withgendersllnl=yes
        ;;
    no)
        ;;
    *)
        withgendersllnl=yes
        ;;
    esac ]
)
AC_MSG_RESULT(${withgendersllnl=no})

##
# Check for libraries
##
AC_CHECK_LIB(genders, genders_handle_create, [havegenders=yes], [])
AC_CHECK_LIB(gendersllnl, genders_get_cluster, [havegendersllnl=yes], [])

##
# Error Checks
##

# I know this is ugly, I'll fix it later
if (test "$withnomasterlist" = "yes" && test "$withhostsfile" = "yes") ||
   (test "$withnomasterlist" = "yes" && test "$withgenders" = "yes") ||
   (test "$withnomasterlist" = "yes" && test "$withgendersllnl" = "yes") || 
   (test "$withhostsfile" = "yes" && test "$withgenders" = "yes") ||
   (test "$withhostsfile" = "yes" && test "$withgendersllnl" = "yes") ||
   (test "$withgenders" = "yes" && test "$withgendersllnl" = "yes"); then
   AC_MSG_ERROR([You can only specify one master list mechanism])
fi

if test "$withnomasterlist" != "yes" &&
   test "$withhostsfile" != "yes" && 
   test "$withgenders" != "yes" &&
   test "$withgendersllnl" != "yes"; then
   if test "$havegendersllnl" = "yes"; then
      withgendersllnl=yes
      AC_MSG_NOTICE([gendersllnl chosen as masterlist mechanism])
   elif test "$havegenders" = "yes"; then
      withgenders=yes
      AC_MSG_NOTICE([genders chosen as masterlist mechanism])
   else
      withnomasterlist=yes
      AC_MSG_NOTICE([no masterlist mechanism chosen by default])
   fi
fi   

# Build with no materlist mechanism
if test "$withnomasterlist" = "yes"; then
  AC_DEFINE(HAVE_NOMASTERLIST, [1], [Define no master list])
  NODEUPDOWN_MASTERLIST_PARAMETER="void *masterlist" 
  NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$masterlist' 
  NODEUPDOWN_MASTERLIST_COMMENT="masterlist is reserved for future use"
  WITH_NOMASTERLIST=1
else
  WITH_NOMASTERLIST=0
fi
AC_SUBST(WITH_NOMASTERLIST)
AC_SUBST(HAVE_NOMASTERLIST)

# Build with master list host file mechanism
if test "$withhostsfile" = "yes"; then
  AC_DEFINE(HAVE_HOSTSFILE, [1], [Define use of master list])
  NODEUPDOWN_MASTERLIST_DEFAULT="\"/etc/nodeupdown_hostsfile.conf\""
  NODEUPDOWN_MASTERLIST_PARAMETER="char *hostsfile" 
  NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$hostsfile'
  NODEUPDOWN_MASTERLIST_COMMENT="if hostsfile is NULL, default hosts is used"
  WITH_HOSTSFILE=1
else
  WITH_HOSTSFILE=0
fi
AC_SUBST(WITH_HOSTSFILE)
AC_SUBST(HAVE_HOSTSFILE)

# Build with genders support
if test "$withgenders" = "yes"; then
   if test "$havegenders" != "yes" ; then
     AC_MSG_ERROR([genders library not found!!])
   else
     GENDERS_LIBS="-lgenders"
     AC_DEFINE(HAVE_GENDERS, [1], [Define use of genders])
     NODEUPDOWN_MASTERLIST_DEFAULT=GENDERS_DEFAULT_FILE
     NODEUPDOWN_MASTERLIST_PARAMETER="char *gendersfile" 
     NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$gendersfile' 
     NODEUPDOWN_MASTERLIST_COMMENT="if gendersfile is NULL, default genders file is used"
     WITH_GENDERS=1
   fi
else
   WITH_GENDERS=0
fi
AC_SUBST(GENDERS_LIBS)
AC_SUBST(WITH_GENDERS)
AC_SUBST(HAVE_GENDERS)

# Build with gendersllnl support
# - For the most part only used at LLNL
if test "$withgendersllnl" = "yes"; then
   if test "$havegenders" != "yes" || test "$havegendersllnl" != "yes"; then
     AC_MSG_ERROR([genders and gendersllnl library files not found!!])
   else
     GENDERSLLNL_LIBS="-lgenders -lgendersllnl"
     AC_DEFINE(HAVE_GENDERSLLNL, [1], [Define use of genders])
     NODEUPDOWN_MASTERLIST_DEFAULT=GENDERS_DEFAULT_FILE
     NODEUPDOWN_MASTERLIST_PARAMETER="char *gendersfile" 
     NODEUPDOWN_MASTERLIST_PERL_PARAMETER='$gendersfile' 
     NODEUPDOWN_MASTERLIST_COMMENT="if gendersfile is NULL, default genders file is used"
     WITH_GENDERSLLNL=1
   fi
else
   WITH_GENDERSLLNL=0
fi
AC_SUBST(GENDERSLLNL_LIBS)
AC_SUBST(WITH_GENDERSLLNL)
AC_SUBST(HAVE_GENDERSLLNL)

if (test "$withgenders" = "yes") ||
   (test "$withgendersllnl" = "yes"); then
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
  src/Libnodeupdown/Libnodeupdown.pm \
  src/Nodeupdown/Makefile \
  man/Makefile \
  man/libnodeupdown.3 \
  man/nodeupdown_load_data.3 \
  man/nodeupdown.conf.5 \
  man/nodeupdown_hostsfile.conf.5 \
  man/whatsup.1 \
)
AC_OUTPUT

