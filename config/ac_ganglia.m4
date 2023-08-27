##*****************************************************************************
## $Id: ac_ganglia.m4,v 1.1 2006-07-10 17:43:49 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_GANGLIA],
[
  AC_MSG_CHECKING([for whether to build ganglia modules])
  AC_ARG_WITH([ganglia],
    AS_HELP_STRING([--with-ganglia], [Build ganglia modules]),
    [ case "$withval" in
        no)  ac_ganglia_test=no ;;
        yes) ac_ganglia_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-ganglia]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_ganglia_test=yes}])

  if test "$ac_ganglia_test" = "yes"; then
     MANPAGE_GANGLIA=1
     ac_with_ganglia=yes
  else
     MANPAGE_GANGLIA=0
     ac_with_ganglia=no
  fi

  AC_SUBST(MANPAGE_GANGLIA)
])
