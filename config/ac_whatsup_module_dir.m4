##*****************************************************************************
## $Id: ac_whatsup_module_dir.m4,v 1.1 2005-03-31 18:29:29 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_WHATSUP_MODULE_DIR],
[
  if test "$prefix" = "NONE"; then
     WHATSUP_MODULE_DIR=${ac_default_prefix}/lib/whatsup
  else
     WHATSUP_MODULE_DIR=${prefix}/lib/whatsup
  fi

  AC_DEFINE_UNQUOTED([WHATSUP_MODULE_DIR], 
                     ["$WHATSUP_MODULE_DIR"], 
                     [Define default whatsup module dir])
  AC_SUBST(WHATSUP_MODULE_DIR)
])
