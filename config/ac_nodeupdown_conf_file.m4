##*****************************************************************************
## $Id: ac_nodeupdown_conf_file.m4,v 1.1 2005-03-31 00:27:38 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_NODEUPDOWN_CONF],
[
  AC_MSG_CHECKING([for nodeupdown config file default path])
  AC_ARG_WITH([nodeupdown-config-file],
    AC_HELP_STRING([--with-nodeupdown-config-file=PATH], 
                   [Specify default nodeupdown config file path]),
    [ case "$withval" in
        no)  NODEUPDOWN_CONF_FILE=/etc/nodeupdown.conf ;;
        yes) NODEUPDOWN_CONF_FILE=/etc/nodeupdown.conf ;;
        *)   NODEUPDOWN_CONF_FILE=$withval 
      esac
    ]
  )
  AC_MSG_RESULT([${NODEUPDOWN_CONF_FILE=/etc/nodeupdown.conf}])

  AC_DEFINE_UNQUOTED([NODEUPDOWN_CONF_FILE], 
                     ["$NODEUPDOWN_CONF_FILE"], 
                     [Define default nodeupdown config_file.])
  AC_SUBST(NODEUPDOWN_CONF_FILE)
])
