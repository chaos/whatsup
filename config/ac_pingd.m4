##*****************************************************************************
## $Id: ac_pingd.m4,v 1.1 2006-07-10 17:44:01 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_PINGD],
[
  AC_MSG_CHECKING([for whether to build pingd modules])
  AC_ARG_WITH([pingd],
    AC_HELP_STRING([--with-pingd], [Build pingd modules]),
    [ case "$withval" in
        no)  ac_pingd_test=no ;;
        yes) ac_pingd_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-pingd]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_pingd_test=yes}])
  
  if test "$ac_pingd_test" = "yes"; then
     MANPAGE_PINGD=1
     ac_with_pingd=yes
  else
     MANPAGE_PINGD=0
     ac_with_pingd=no
  fi

  AC_SUBST(MANPAGE_PINGD)
])
