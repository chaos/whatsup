##*****************************************************************************
## $Id: ac_gendersllnl.m4,v 1.1 2005-03-31 18:29:29 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GENDERSLLNL],
[
  AC_MSG_CHECKING([for whether to build gendersllnl modules])
  AC_ARG_WITH([gendersllnl],
    AC_HELP_STRING([--with-gendersllnl], [Build gendersllnl modules]),
    [ case "$withval" in
        no)  ac_gendersllnl_test=no ;;
        yes) ac_gendersllnl_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-gendersllnl]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_gendersllnl_test=yes}])
  
  if test "$ac_gendersllnl_test" = "yes"; then
     AC_MSG_CHECKING([for libgendersllnl library])
     AC_CHECK_LIB([gendersllnl], [genders_isnode_or_altnode], [ac_have_gendersllnl=yes], [])
     AC_MSG_RESULT([${ac_have_gendersllnl=no}])
  fi

  if test "$ac_have_gendersllnl" = "yes"; then
     AC_DEFINE([WITH_GENDERSLLNL], [1], [Define if you have gendersllnl.])
     GENDERSLLNL_LIBS="-lgendersllnl"
     MANPAGE_GENDERSLLNL=1
     ac_with_gendersllnl=yes
  else
     MANPAGE_GENDERSLLNL=0
     ac_with_gendersllnl=no
  fi

  AC_SUBST(GENDERSLLNL_LIBS)
  AC_SUBST(MANPAGE_GENDERSLLNL)
])
