##*****************************************************************************
## $Id: ac_whatsup_module_dir.m4,v 1.2 2005-05-17 15:59:11 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_WHATSUP_MODULE_DIR],
[
  if echo ${libdir} | grep 'lib64'; then
     LIBDIRTYPE=lib64
  else
     LIBDIRTYPE=lib
  fi

  if test "$prefix" = "NONE"; then
     WHATSUP_MODULE_DIR=${ac_default_prefix}/$LIBDIRTYPE/whatsup
  else
     WHATSUP_MODULE_DIR=${prefix}/$LIBDIRTYPE/whatsup
  fi

  AC_DEFINE_UNQUOTED([WHATSUP_MODULE_DIR], 
                     ["$WHATSUP_MODULE_DIR"], 
                     [Define default whatsup module dir])
  AC_SUBST(WHATSUP_MODULE_DIR)
])
