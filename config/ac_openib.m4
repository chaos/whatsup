##*****************************************************************************
## $Id: ac_openib.m4,v 1.2 2009-02-26 00:14:38 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_OPENIB],
[
  AC_MSG_CHECKING([for whether to build openib modules])
  AC_ARG_WITH([openib],
    AS_HELP_STRING([--with-openib], [Build openib modules]),
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

  if test "$ac_with_openib" = "yes"; then
    SAVE_LIBS=$LIBS
    LIBS="$LIBS -lopensm -losmcomp -losmvendor"
    SAVE_CFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -DOSM_VENDOR_INTF_OPENIB -I/usr/include/infiniband"
    AC_TRY_LINK(
    [
        #include <opensm/osm_mad_pool.h>
    ],
    [
        osm_mad_pool_t mad_pool;
        osm_log_t log;
        osm_mad_pool_init(&mad_pool, &log);
    ],
    [ac_osm_mad_pool_init=two], [
        AC_TRY_LINK(
        [
            #include <opensm/osm_mad_pool.h>
        ],
        [
            osm_mad_pool_t mad_pool;
            osm_mad_pool_init(&mad_pool);
        ],
        [ac_osm_mad_pool_init=one],
        [ac_osm_mad_pool_init=no])
    ])

    if test "x${ac_osm_mad_pool_init}" = "xtwo"; then
       AC_DEFINE(HAVE_FUNC_OSM_MAD_POOL_INIT_2, [], [Define osm_mad_pool_init with 2 args])
    elif test "x${ac_osm_mad_pool_init}" = "xone"; then
       AC_DEFINE(HAVE_FUNC_OSM_MAD_POOL_INIT_1, [], [Define osm_mad_pool_init with 1 args])
    fi
    LIBS=$SAVE_LIBS
    CFLAGS=$SAVE_CFLAGS
  fi

  AC_SUBST(MANPAGE_OPENIB)
  AC_SUBST(OPENIB_LIBS)
  AC_SUBST(OPENIB_CFLAGS)
])
