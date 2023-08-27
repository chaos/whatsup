##*****************************************************************************
## $Id: ac_pingd_conf_file.m4,v 1.1 2006-07-07 18:14:15 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_PINGD_CONF],
[
  PINGD_CONF_FILE=/etc/pingd.conf

  AC_MSG_CHECKING([for pingd config file default path])
  AC_ARG_WITH([pingd-config-file],
    AS_HELP_STRING([--with-pingd-config-file=PATH], 
                   [Specify default pingd config file path]),
    [ case "$withval" in
        no)  ;;
        yes) ;;
        *)   PINGD_CONF_FILE=$withval 
      esac
    ]
  )
  AC_MSG_RESULT($PINGD_CONF_FILE)

  AC_DEFINE_UNQUOTED([PINGD_CONF_FILE], 
                     ["$PINGD_CONF_FILE"], 
                     [Define default pingd config_file.])
  AC_SUBST(PINGD_CONF_FILE)
])
