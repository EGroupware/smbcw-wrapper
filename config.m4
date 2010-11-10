PHP_ARG_ENABLE(smbcw_wrapper, whether to enable Libsmpclient_wrapper support,
[ --enable-smbcw_wrapper   Enable smbcw support for SMB-Share access])

if test "$PHP_SMBCW_WRAPPER" = "yes"; then
  AC_MSG_CHECKING(for SMBCW in default path)
  for i in /usr/lib64 /usr/local/lib /usr/lib; do
    if test -r $i/libsmbcw.so; then
      SMBCW_DIR=$i
      AC_MSG_RESULT(found in $i)
      break
    fi
  done

  if test -z "$SMBCW_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Please reinstall the SMBCW distribution)
  fi

  PHP_ADD_LIBRARY_WITH_PATH(smbcw, $SMBCW_DIR, SMBCW_WRAPPER_SHARED_LIBADD)

  AC_DEFINE(HAVE_SMBCW_WRAPPER, 1, [Whether you have smbcw_wrapper])

  PHP_SUBST(SMBCW_WRAPPER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(smbcw_wrapper, smbcw_wrapper.c, $ext_shared)
fi
