#ifndef PHP_SMBCW_WRAPPER_H
#define PHP_SMBCW_WRAPPER_H 1

#define PHP_SMBCW_WRAPPER_VERSION "1.0"
#define PHP_SMBCW_WRAPPER_EXTNAME "smbcw_wrapper"

PHP_MINIT_FUNCTION(smbcw);
PHP_MSHUTDOWN_FUNCTION(smbcw);
PHP_MINFO_FUNCTION(smbcw);
PHP_FUNCTION(smb_chmod);

extern zend_module_entry smbcw_wrapper_module_entry;
#define phpext_smbcw_wrapper_ptr &smbcw_wrapper_module_entry

#endif
