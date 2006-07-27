##*****************************************************************************
## $Id: ac_pingd_module_dir.m4,v 1.2 2006-07-27 02:29:42 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_PINGD_MODULE_DIR],
[
  if echo ${libdir} | grep 'lib64'; then
     LIBDIRTYPE=lib64
  else
     LIBDIRTYPE=lib
  fi

  if test "$prefix" = "NONE"; then
     PINGD_MODULE_DIR=${ac_default_prefix}/$LIBDIRTYPE/pingd
  else
     PINGD_MODULE_DIR=${prefix}/$LIBDIRTYPE/pingd
  fi

  AC_DEFINE_UNQUOTED([PINGD_MODULE_DIR], 
                     ["$PINGD_MODULE_DIR"], 
                     [Define default pingd module dir])
  AC_SUBST(PINGD_MODULE_DIR)
])
