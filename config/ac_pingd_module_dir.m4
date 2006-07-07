##*****************************************************************************
## $Id: ac_pingd_module_dir.m4,v 1.1 2006-07-07 18:14:15 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_PINGD_MODULE_DIR],
[
  # Workaround lack of nested unquoting (from Conman, Chris Dunlap, dunlap 6 at llnl dot gov)
  PINGD_MODULE_DIR_TMP1="`eval echo ${libdir}/pingd`"
  PINGD_MODULE_DIR_TMP2="`echo $PINGD_MODULE_DIR_TMP1 | sed 's/^NONE/$ac_default_prefix/'`"
  PINGD_MODULE_DIR="`eval echo $PINGD_MODULE_DIR_TMP2`"
  AC_DEFINE_UNQUOTED([PINGD_MODULE_DIR], 
                     ["$PINGD_MODULE_DIR"], 
                     [Define default pingd module dir])
  AC_SUBST(PINGD_MODULE_DIR)
])
