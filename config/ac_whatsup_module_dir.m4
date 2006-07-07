##*****************************************************************************
## $Id: ac_whatsup_module_dir.m4,v 1.3 2006-07-07 18:14:15 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_WHATSUP_MODULE_DIR],
[
  # Workaround lack of nested unquoting (from Conman, Chris Dunlap, dunlap 6 at llnl dot gov)
  WHATSUP_MODULE_DIR_TMP1="`eval echo ${libdir}/whatsup`"
  WHATSUP_MODULE_DIR_TMP2="`echo $WHATSUP_MODULE_DIR_TMP1 | sed 's/^NONE/$ac_default_prefix/'`"
  WHATSUP_MODULE_DIR="`eval echo $WHATSUP_MODULE_DIR_TMP2`"
  AC_DEFINE_UNQUOTED([WHATSUP_MODULE_DIR], 
                     ["$WHATSUP_MODULE_DIR"], 
                     [Define default whatsup module dir])
  AC_SUBST(WHATSUP_MODULE_DIR)
])
