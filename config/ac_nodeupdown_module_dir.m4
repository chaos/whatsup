##*****************************************************************************
## $Id: ac_nodeupdown_module_dir.m4,v 1.1 2005-03-31 18:29:29 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_NODEUPDOWN_MODULE_DIR],
[
  if test "$prefix" = "NONE"; then
     NODEUPDOWN_MODULE_DIR=${ac_default_prefix}/lib/nodeupdown
  else
     NODEUPDOWN_MODULE_DIR=${prefix}/lib/nodeupdown
  fi

  AC_DEFINE_UNQUOTED([NODEUPDOWN_MODULE_DIR], 
                     ["$NODEUPDOWN_MODULE_DIR"], 
                     [Define default nodeupdown module dir])
  AC_SUBST(NODEUPDOWN_MODULE_DIR)
])
