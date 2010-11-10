PHP_ARG_ENABLE(smbcw_wrapper, whether to enable Libsmpclient_wrapper support,
[ --enable-smbcw_wrapper   Enable smbcw support for SMB-Share access])

if test "$PHP_SMBCW_WRAPPER" != "no"; then
  PHP_ADD_INCLUDE(smbcw)
  
  dnl PHP_ADD_LIBRARY_WITH_PATH(smbcw, $SMBCW_DIR, SMBCW_WRAPPER_SHARED_LIBADD)

  dnl AC_DEFINE(HAVE_SMBCW_WRAPPER, 1, [Whether you have smbcw_wrapper])

  dnl PHP_SUBST(SMBCW_WRAPPER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(smbcw_wrapper, smbcw_wrapper.c smbcw/smbcw.c smbcw/smbcw_url.c smbcw/smbcw_descriptor.c smbcw/smbcw_connections.c, $ext_shared)
fi
