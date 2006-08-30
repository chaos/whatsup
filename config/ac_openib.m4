##*****************************************************************************
## $Id: ac_openib.m4,v 1.1 2006-08-30 17:10:00 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_OPENIB],
[
  AC_MSG_CHECKING([for whether to build openib modules])
  AC_ARG_WITH([openib],
    AC_HELP_STRING([--with-openib], [Build openib modules]),
    [ case "$withval" in
        no)  ac_openib_test=no ;;
        yes) ac_openib_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-openib]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_openib_test=yes}])

  if test "$ac_openib_test" = "yes"; then
     AC_CHECK_LIB([opensm], [osm_mad_pool_init], [ac_openib_opensm=yes], [], [-losmcomp -losmvendor])
     AC_CHECK_LIB([osmcomp], [complib_init], [ac_openib_osmcomp=yes], [], [-lopensm -losmvendor])
     AC_CHECK_LIB([osmvendor], [osm_vendor_new], [ac_openib_osmvendor=yes], [], [-lopensm -losmcomp])
  fi
  
  if (test "$ac_openib_opensm" = "yes") && 
	(test "$ac_openib_osmcomp" = "yes") &&
	(test "$ac_openib_osmvendor" = "yes"); then
     OPENIB_LIBS="-lopensm -losmcomp -losmvendor"
     OPENIB_CFLAGS="-DOSM_VENDOR_INTF_OPENIB -I/usr/include/infiniband"
     MANPAGE_OPENIB=1
     ac_with_openib=yes
  else
     MANPAGE_OPENIB=0
     ac_with_openib=no
  fi

  AC_SUBST(MANPAGE_OPENIB)
  AC_SUBST(OPENIB_LIBS)
  AC_SUBST(OPENIB_CFLAGS)
])
