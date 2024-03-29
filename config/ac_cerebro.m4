##*****************************************************************************
## $Id: ac_cerebro.m4,v 1.3 2006-11-10 00:09:14 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO],
[
  AC_MSG_CHECKING([for whether to build cerebro modules])
  AC_ARG_WITH([cerebro],
    AS_HELP_STRING([--with-cerebro], [Build cerebro modules]),
    [ case "$withval" in
        no)  ac_cerebro_test=no ;;
        yes) ac_cerebro_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-cerebro]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_cerebro_test=yes}])

  if test "$ac_cerebro_test" = "yes"; then
     AC_CHECK_LIB([cerebro], [cerebro_handle_create], [ac_have_cerebro=yes], [])
     AC_CHECK_LIB([cerebro], [cerebro_event_register], [ac_have_cerebro_event=yes], [])
  fi

  if test "$ac_have_cerebro" = "yes"; then
     AC_DEFINE([WITH_CEREBRO], [1], [Define if you have cerebro.])
     CEREBRO_LIBS="-lcerebro"
     MANPAGE_CEREBRO=1
     ac_with_cerebro=yes
     if test "$ac_have_cerebro_event" = "yes"; then
        AC_DEFINE([WITH_CEREBRO_EVENT], [1], [Define if you have cerebro events.])
        ac_with_cerebro_event=yes
        MANPAGE_CEREBRO_EVENT=1
     fi
  else
     MANPAGE_CEREBRO=0
     ac_with_cerebro=no
  fi

  AC_SUBST(CEREBRO_LIBS)
  AC_SUBST(MANPAGE_CEREBRO)
  AC_SUBST(MANPAGE_CEREBRO_EVENT)
])
