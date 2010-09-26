/* Adds smbcw support to php and registers smb:// streamwrappers.
 *
 * (c) by Andreas Stoeckel 2010
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_smbcw_wrapper.h"
#include <stdio.h>
#include <errno.h>
#include <smbcw.h>
#include <sys/stat.h>

static function_entry smbcw_wrapper_functions[] = {
    PHP_FE(smb_chmod, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry smbcw_wrapper_module_entry = {
        STANDARD_MODULE_HEADER,
        PHP_SMBCW_WRAPPER_EXTNAME,
        smbcw_wrapper_functions,
        PHP_MINIT(smbcw),
        PHP_MSHUTDOWN(smbcw),
        NULL,
        NULL,
        PHP_MINFO(smbcw),
        PHP_SMBCW_WRAPPER_VERSION,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SMBCW_WRAPPER
ZEND_GET_MODULE(smbcw_wrapper)
#endif

void print_last_smb_err()
{
	//Get the smbcw error and print it
	php_error(E_ERROR, "[SMBCW_WRAPPER ERROR] %s ", strerror(smbcw_geterr()));
}

void copy_to_php_stat(smbcw_stat *src, struct stat *tar)
{
	tar->st_dev = src->s_dev;
	tar->st_ino = src->s_ino;
	tar->st_mode = src->s_mode;
	tar->st_nlink = src->s_nlink;
	tar->st_uid = src->s_uid;
	tar->st_gid = src->s_gid;
	tar->st_rdev = src->s_rdev;
	tar->st_size = src->s_size;
	tar->st_blksize = src->s_blksize;
	tar->st_atime = src->s_atime;
	tar->st_mtime = src->s_mtime;
	tar->st_ctime = src->s_ctime;
}

#define SMB_CHECK_ERR(err)\
if (err < 0) {\
print_last_smb_err();\
return -1;} else return 0;\

struct _php_smb_data {
	int fd;
};

typedef struct _php_smb_data php_smb_data;
typedef php_smb_data *lp_php_smb_data;

void free_smb_data(lp_php_smb_data data)
{
	if (data->fd > 0)
		data->fd = 0;
	efree(data);
}

static size_t php_smb_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
	size_t ret = 0;
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;

	if (self->fd > 0)
	{
		ret = smbcw_fread(self->fd, buf, count);

		if (ret == 0)
			stream->eof = 1;

		if (ret < 0)
			print_last_smb_err();
	}
	
	return ret;
}

static size_t php_smb_write(php_stream *stream, const char *buf, size_t count TSRMLS_DC)
{
	size_t ret = 0;
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;

	if (self->fd > 0)
	{
		ret = smbcw_fwrite(self->fd, (char*)buf, count);
		if (ret < 0)
			print_last_smb_err();
	}
	
	return ret;
}

static int php_smb_close(php_stream *stream, int close_handle TSRMLS_DC)
{
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;
	if (self->fd > 0)
		smbcw_fclose(self->fd);

	free_smb_data(self);

	return 0;
}

static int php_smb_flush(php_stream *stream TSRMLS_DC)
{
	//!
	return 0;
}

static int php_smb_seek(php_stream *stream, off_t offset, int whence, off_t *newoffset TSRMLS_DC)
{
	off_t ret = 0;
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;

	if (self->fd > 0)
	{
		ret = smbcw_fseek(self->fd, offset, whence);
		if (ret >= 0)
			*newoffset = ret;

		SMB_CHECK_ERR(ret);
	}
	
	return -1;
}

static int php_smb_stat(php_stream *stream, php_stream_statbuf *ssb TSRMLS_DC)
{
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;

	if (self->fd > 0)
	{
		smbcw_stat st;
		memset(&st, 0, sizeof(st));

		if (smbcw_fstat(self->fd, &st) == 0)
		{
			copy_to_php_stat(&st, &ssb->sb);
			return 0;
		}

		print_last_smb_err();
	}
	
	return -1;
}

php_stream_ops php_stream_smb_ops = {
        php_smb_write,
		php_smb_read,
        php_smb_close,
		php_smb_flush,
        "smb",
        php_smb_seek, /* seek */
        NULL, /* cast */
        php_smb_stat, /* stat */
        NULL  /* set_option */
};

#define FREE_AND_RETURN {\
free_smb_data(self);\
return NULL;\
}

php_stream *_php_stream_smbopen(php_stream_wrapper *wrapper, char *path, char *mode, int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	int fd = smbcw_fopen(path, mode);
	if (fd > 0)	
	{
		lp_php_smb_data self = emalloc(sizeof(*self));
		self->fd = fd;

		return php_stream_alloc_rel(&php_stream_smb_ops, self, 0, mode);
	}
	else
	{
		print_last_smb_err();
	}

	return NULL;
}

//----- DIR STREAM FUNCTIONS ------

static size_t php_smb_dirstream_read(php_stream *stream, char *buf, size_t count TSRMLS_DC)
{
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;

	if (self->fd > 0)
	{
		/* avoid problems if someone mis-uses the stream */
		if (count != sizeof(php_stream_dirent))
			return 0;

		/* Read the dir entry name */
		char *name;
		smbcw_readdir(self->fd, &name);
		if (name != NULL)
		{
			php_stream_dirent *php_ent = (php_stream_dirent*)buf;

			PHP_STRLCPY(php_ent->d_name, name, sizeof(php_ent->d_name),
				strlen(name));

			return sizeof(php_stream_dirent);
		}
	}
	
	return 0;	
}

static int php_smb_dirstream_close(php_stream *stream, int close_handle TSRMLS_DC)
{
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;

	if (self->fd == 0)
		smbcw_closedir(self->fd);

	free_smb_data(self);
	
	return 0;
}

static int php_smb_dirstream_rewind(php_stream *stream, off_t offset, int whence, off_t *newoffs TSRMLS_DC)
{
	lp_php_smb_data self = (lp_php_smb_data)stream->abstract;

	if (self->fd > 0)
	{
		int err = smbc_lseekdir(self->fd, 0);
		SMB_CHECK_ERR(err);
	}
	
	return -1;
}


static php_stream_ops php_smb_dirstream_ops = {
	NULL,
	php_smb_dirstream_read,
	php_smb_dirstream_close,
	NULL,
	"dir",
	php_smb_dirstream_rewind,
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};


php_stream *_php_smb_dir_opener(php_stream_wrapper *wrapper, char *path, char *mode,
                int options, char **opened_path, php_stream_context *context STREAMS_DC TSRMLS_DC)
{
	int fd = smbcw_opendir(path);
	if (fd > 0)
	{
		lp_php_smb_data self = emalloc(sizeof(*self));
		self->fd = fd;
		
		return php_stream_alloc_rel(&php_smb_dirstream_ops, self, 0, mode);
	} else
		print_last_smb_err();

	return NULL;
}

//---- MISC FILE OPERATIONS ----

int _php_smb_url_stat(php_stream_wrapper *wrapper, char *url, int flags, php_stream_statbuf *ssb, php_stream_context *context TSRMLS_DC)
{
	smbcw_stat stat;
	if (smbcw_urlstat(url, &stat) >= 0)
	{
		copy_to_php_stat(&stat, &ssb->sb);
		return 0;
	}
	else
		print_last_smb_err();

	return -1;
}

int _php_smb_unlink(php_stream_wrapper *wrapper, char *url, int options, php_stream_context *context TSRMLS_DC)
{
	int err = smbcw_unlink(url);
	SMB_CHECK_ERR(err);
}

int _php_smb_rename(php_stream_wrapper *wrapper, char *url_from, char *url_to, int options, php_stream_context *context TSRMLS_DC)
{
	int err = smbcw_rename(url_from, url_to);
	SMB_CHECK_ERR(err);	
}

int _php_smb_mkdir(php_stream_wrapper *wrapper, char *url, int mode, int options, php_stream_context *context TSRMLS_DC)
{
	int err = smbcw_mkdir(url);
	SMB_CHECK_ERR(err);
}

int _php_smb_rmdir(php_stream_wrapper *wrapper, char *url, int options, php_stream_context *context TSRMLS_DC)
{
	int err = smbcw_rmdir(url);
	SMB_CHECK_ERR(err);
}

static php_stream_wrapper_ops smb_stream_wops = {
        _php_stream_smbopen,
        NULL,
        NULL,
        _php_smb_url_stat,
        _php_smb_dir_opener,
        "smb",
		_php_smb_unlink,
		_php_smb_rename,
		_php_smb_mkdir,
		_php_smb_rmdir
};

php_stream_wrapper php_stream_smb_wrapper = {
        &smb_stream_wops,
        NULL,
        0
};

PHP_FUNCTION(smb_chmod)
{
	char *url;
	int url_len = 0;	
	int mode = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &url, &url_len, 
		&mode) == FAILURE) {
        RETURN_NULL();
    }

	int ret = smbcw_chmod(url, mode);
	RETURN_LONG(ret >= 0 ? 1 : 0);
}

PHP_MINIT_FUNCTION(smbcw)
{
	//Initialize SMBCW
	if (smbcw_init() >= 0) 
	{
		//Register the smbcw wrapper library
		php_register_url_stream_wrapper("smb", &php_stream_smb_wrapper TSRMLS_CC);
	}

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(smbcw)
{
	//Unregister the SMBCW wrapper library
	php_unregister_url_stream_wrapper("smb" TSRMLS_CC);	

	return SUCCESS;
}

PHP_MINFO_FUNCTION(smbcw)
{
        php_info_print_table_start();
        php_info_print_table_row(2, "SMBCW_WRAPPER Support", "Enabled");
        php_info_print_table_end();
}

