##*****************************************************************************
## $Id: ac_pingd_hostsfile.m4,v 1.1 2006-07-07 16:40:46 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_HOSTSFILE],
[
  AC_MSG_CHECKING([for whether to build hostsfile module])
  AC_ARG_WITH([hostsfile],
    AC_HELP_STRING([--with-hostsfile], [Build hostsfile modules]),
    [ case "$withval" in
        no)  ac_hostsfile_test=no ;;
        yes) ac_hostsfile_test=yes ;;
        *)   ac_hostsfile_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_hostsfile_test=yes}])
 
  if test "$ac_genders_test" = "yes"; then
     MANPAGE_HOSTSFILE=1
     ac_with_hostsfile=yes
  else
     MANPAGE_HOSTSFILE=0
     ac_with_hostsfile=no
  fi
  
  AC_SUBST(MANPAGE_HOSTSFILE)
])

AC_DEFUN([AC_HOSTSFILE_PATH],
[
  NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/hostsfile

  AC_MSG_CHECKING([for default hostsfile path])
  AC_ARG_WITH([hostsfile-path],
    AC_HELP_STRING([--with-hostsfile-path=PATH], [Specify default hostsfile clusterlist path]),
    [ case "$withval" in
        no)  ;;
        yes) ;;
        *)   NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT=$withval ;;
      esac
    ]
  )
  AC_MSG_RESULT([$NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT])
 
  AC_DEFINE_UNQUOTED([NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT],
                     ["$NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT"],
                     [Define default hostsfile clusterlist.])
  AC_SUBST(NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT)
])
