##*****************************************************************************
## $Id: ac_hostsfile.m4,v 1.1 2005-03-31 22:44:22 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_HOSTSFILE],
[
  AC_MSG_CHECKING([for hostsfile default path])
  AC_ARG_WITH([hostsfile],
    AC_HELP_STRING([--with-hostsfile=PATH], [Specify default hostsfile clusterlist path]),
    [ case "$withval" in
        no)  NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/nodeupdown_hostsfile ;;
        yes) NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/nodeupdown_hostsfile ;;
        *)   NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT=$withval 
      esac
    ]
  )
  AC_MSG_RESULT([${NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/nodeupdown_hostsfile}])

  AC_DEFINE_UNQUOTED([NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT], 
                     ["$NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT"], 
                     [Define default hostsfile clusterlist.])
  AC_SUBST(NODEUPDOWN_CLUSTERLIST_HOSTSFILE_DEFAULT)
])
