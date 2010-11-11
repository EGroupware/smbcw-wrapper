PHP_ARG_ENABLE(smbcw_wrapper, whether to enable libsmbclient_wrapper support,
[ --enable-smbcw_wrapper   Enable smbcw support for SMB-Share access])

if test "$PHP_SMBCW_WRAPPER" != "no"; then
  PHP_ADD_INCLUDE(smbcw)
  AC_MSG_CHECKING([for libsmbclient in default path])
  for i in /usr/lib64 /usr/local/lib /usr/lib; do
    if test -r $i/libsmbclient.so; then
      SMB_DIR=$i
      AC_MSG_RESULT(found in $i)
      break
    fi
  done

  if test -z "$SMB_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall libsmbclient])
  fi
  dnl PHP_ADD_INCLUDE(/usr/include)
  PHP_CHECK_LIBRARY(smbclient, smbc_getFunctionOpen,
  [
    PHP_ADD_LIBRARY_WITH_PATH(smbclient, $SMB_DIR, SMBCW_WRAPPER_SHARED_LIBADD)
    AC_DEFINE(HAVE_SMBLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong smbclient version or lib not found])
  ],[
    -L$SMB_DIR 
  ])

  dnl PHP_ADD_LIBRARY_WITH_PATH(smbclient, $SMB_DIR, SMB_SHARED_LIBADD)

  dnl AC_DEFINE(HAVE_SMB, 1, [Whether you have smbclient installed])

  PHP_SUBST(SMBCW_WRAPPER_SHARED_LIBADD)

  dnl PHP_NEW_EXTENSION(smbcw_wrapper, smbcw_wrapper.c smbcw/smbcw.c smbcw/smbcw_url.c smbcw/smbcw_descriptor.c smbcw/smbcw_connections.c, $ext_shared)
  PHP_NEW_EXTENSION(smbcw_wrapper, smbcw_wrapper.c smbcw/smbcw.c smbcw/smbcw_url.c smbcw/smbcw_descriptor.c smbcw/smbcw_connections.c, $ext_shared)
  dnl PHP_NEW_EXTENSION(smbcw_wrapper, smbcw_wrapper.c, $ext_shared)
fi
