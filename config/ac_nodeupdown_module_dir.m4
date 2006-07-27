##*****************************************************************************
## $Id: ac_nodeupdown_module_dir.m4,v 1.4 2006-07-27 02:29:42 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_NODEUPDOWN_MODULE_DIR],
[
  if echo ${libdir} | grep 'lib64'; then
     LIBDIRTYPE=lib64
  else
     LIBDIRTYPE=lib
  fi

  if test "$prefix" = "NONE"; then
     NODEUPDOWN_MODULE_DIR=${ac_default_prefix}/$LIBDIRTYPE/nodeupdown
  else
     NODEUPDOWN_MODULE_DIR=${prefix}/$LIBDIRTYPE/nodeupdown
  fi

  AC_DEFINE_UNQUOTED([NODEUPDOWN_MODULE_DIR], 
                     ["$NODEUPDOWN_MODULE_DIR"], 
                     [Define default nodeupdown module dir])
  AC_SUBST(NODEUPDOWN_MODULE_DIR)
])
