##*****************************************************************************
## $Id: ac_cerebro.m4,v 1.1 2005-05-10 22:29:34 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO],
[
  AC_MSG_CHECKING([for whether to build cerebro modules])
  AC_ARG_WITH([cerebro],
    AC_HELP_STRING([--with-cerebro], [Build cerebro modules]),
    [ case "$withval" in
        no)  ac_cerebro_test=no ;;
        yes) ac_cerebro_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-cerebro]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_cerebro_test=yes}])
  
  if test "$ac_cerebro_test" = "yes"; then
     AC_MSG_CHECKING([for libcerebro library])
     AC_CHECK_LIB([cerebro], [cerebro_handle_create], [ac_have_cerebro=yes], [])
     AC_MSG_RESULT([${ac_have_cerebro=no}])
  fi

  if test "$ac_have_cerebro" = "yes"; then
     AC_DEFINE([WITH_CEREBRO], [1], [Define if you have cerebro.])
     CEREBRO_LIBS="-lcerebro"
     MANPAGE_CEREBRO=1
     ac_with_cerebro=yes
  else 
     MANPAGE_CEREBRO=0
     ac_with_cerebro=no
  fi

  AC_SUBST(CEREBRO_LIBS)
  AC_SUBST(MANPAGE_CEREBRO)
])
