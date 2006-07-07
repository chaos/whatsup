##*****************************************************************************
## $Id: ac_nodeupdown_module_dir.m4,v 1.3 2006-07-07 18:14:15 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_NODEUPDOWN_MODULE_DIR],
[
  # Workaround lack of nested unquoting (from Conman, Chris Dunlap, dunlap 6 at llnl dot gov)
  NODEUPDOWN_MODULE_DIR_TMP1="`eval echo ${libdir}/nodeupdown`"
  NODEUPDOWN_MODULE_DIR_TMP2="`echo $NODEUPDOWN_MODULE_DIR_TMP1 | sed 's/^NONE/$ac_default_prefix/'`"
  NODEUPDOWN_MODULE_DIR="`eval echo $NODEUPDOWN_MODULE_DIR_TMP2`"
  AC_DEFINE_UNQUOTED([NODEUPDOWN_MODULE_DIR], 
                     ["$NODEUPDOWN_MODULE_DIR"], 
                     [Define default nodeupdown module dir])
  AC_SUBST(NODEUPDOWN_MODULE_DIR)
])
