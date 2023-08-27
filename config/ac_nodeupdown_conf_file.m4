##*****************************************************************************
## $Id: ac_nodeupdown_conf_file.m4,v 1.2 2005-07-18 21:31:02 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_NODEUPDOWN_CONF],
[
  NODEUPDOWN_CONF_FILE=/etc/nodeupdown.conf

  AC_MSG_CHECKING([for nodeupdown config file default path])
  AC_ARG_WITH([nodeupdown-config-file],
    AS_HELP_STRING([--with-nodeupdown-config-file=PATH],
                   [Specify default nodeupdown config file path]),
    [ case "$withval" in
        no)  ;;
        yes) ;;
        *)   NODEUPDOWN_CONF_FILE=$withval
      esac
    ]
  )
  AC_MSG_RESULT($NODEUPDOWN_CONF_FILE)

  AC_DEFINE_UNQUOTED([NODEUPDOWN_CONF_FILE],
                     ["$NODEUPDOWN_CONF_FILE"],
                     [Define default nodeupdown config_file.])
  AC_SUBST(NODEUPDOWN_CONF_FILE)
])
