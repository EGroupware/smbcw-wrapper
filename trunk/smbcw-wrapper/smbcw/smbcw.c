/* Small wrapper library for the basic functions of libsmbclient. This library
 * provides a common interface to libsmbclient with fixed size types.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsmbclient.h>
#include <sys/stat.h>
#include <errno.h>

#include "smbcw.h"
#include "smbcw_common.h"
#include "smbcw_url.h"
#include "smbcw_connections.h"
#include "smbcw_descriptor.h"


/**
 * Alias for an pointer on SMBCCTX
 */
typedef SMBCCTX *lp_smbcctx;
typedef SMBCSRV *lp_smbcsrv;
typedef SMBCFILE *lp_smbcfile;

/* SMBCW Helper functions and macros */

/* The fd_struct_val is just some magic number to verify that an fd really points to
   and smbcw_fd (see below)*/
#define FD_STRUCT_VAL 0xF3C55ED1

typedef struct {
	/* Field is used internally to verify that the pointer retrieved from the descriptor
	   class is really an file descriptor and not something else, which would probably
	   lead to security problems or would result in a program crash. */
	int fd_check;
	lp_smbcctx ctx;
	lp_smbcfile file;  
} smbcw_file;

typedef smbcw_file *lp_smbcw_file;


int smbcw_create_file_desc(lp_smbcctx ctx, lp_smbcfile file, lp_smbcw_file *file_desc)
{
	lp_smbcw_file desc = malloc(sizeof(*desc));

	//Fill the descriptor with the stuff passed as function parameters
	desc->fd_check = FD_STRUCT_VAL;
	desc->ctx = ctx;
	desc->file = file;	

	//Register the descriptor
	int id = smbcw_gen_id(desc);

	if (id > 0)
	{

		//Set the output parameter
		if (file_desc)
			*file_desc = desc;

		return id;
	} else {
		return 0;
	}
}

int smbcw_errno = 0;

#define _RETURN(cmd)\
{\
int _retval1 = cmd;\
if (_retval1 < 0)\
	smbcw_errno = errno;\
else\
	smbcw_errno = 0;\
return _retval1;\
}

#define _RETURN_ERR(err)\
{\
	smbcw_errno = err;\
	return -1;\
}

/**
 * Checks whether the given url is valid (has "smb" as protocol). If yes, an
 * url descriptor is returned. It is your duty to free the descriptor when it is
 * no longer needed.
 */
lp_smbcw_url smbcw_check_url(char *url)
{
	lp_smbcw_url result = smbcw_url_create(url);

	if (strcmp(result->protocol, "smb") != 0)
	{
		smbcw_url_free(result);
		result = NULL;
	}

	return result;
}

/**
 * Checks whether the given url is valid, and if yes, obtains a SMBCW context
 * for the given URL an returns it. Returns NULL if any error occurs. In this case
 * url_out will also be set to NULL and any reserved memory will be freed.
 */
lp_smbcctx smbcw_get_url_context(char *url, lp_smbcw_url *url_out)
{
	lp_smbcw_url checked_url;

	//Set the output url structure to NULL
	if (url_out)
		*url_out = NULL;

	//Check the following URL for compliance with the SMBCW naming scheme
	checked_url = smbcw_check_url(url);

	//If the URL passed the check, return the context associated with the URL
	if (checked_url) {
		//Obtain the context
		lp_smbcctx ctx = connections_get_ctx(checked_url);

		if (ctx)
		{
			//Set the output url structure, if it does not point to NULL
			if (url_out)
			{
				*url_out = checked_url;
			} else {
				smbcw_url_free(checked_url);
			}

			//Return the context
			return ctx;
		} else {
			//Free the checked url
			smbcw_url_free(checked_url);
		}
	}

	return NULL;
}



/* SMBCW Initialization/Finalization functions */

/* Used internally as fallback authentification function for SMBC */
static void smb_auth_fn(const char *server, const char *share,
	char *workgroup, int wgmaxlen, char *username, int unmaxlen,
	char *password, int pwmaxlen)
{
	if ((workgroup != NULL) && (wgmaxlen >= strlen(DEFAULT_WORKGROUP)))
		strcpy(workgroup, DEFAULT_WORKGROUP);
	if ((username != NULL) && (unmaxlen >= strlen(DEFAULT_USERNAME)))
		strcpy(username, DEFAULT_USERNAME);
	if ((password != NULL) && (pwmaxlen >= strlen(DEFAULT_PASSWORD)))
		strcpy(password, DEFAULT_PASSWORD);
}

int smbcw_init()
{
	//Initialize the smbcw connection manager
	connections_init();


	/*smbc_init is only needed if we would be using the deprecated compatibility
	  layer provided in libsmb_compat.h

	//Intialize libsmbclient and return the initialization result
	_RETURN(smbc_init(smb_auth_fn, MAX_DEBUG_LEVEL));*/

	_RETURN(0);
}

void smbcw_finalize()
{
	//Finalize all connections
	connections_finalize();
}


/** SMBCW File Operation functions **/

void smbcw_write_stat(struct stat *src, smbcw_stat *tar)
{
	tar->s_dev = src->st_dev;
	tar->s_ino = src->st_ino;
	tar->s_mode = src->st_mode;
	tar->s_nlink = src->st_nlink;
	tar->s_uid = 0; //src->st_uid; uid and gid contain the webserver URL
	tar->s_gid = 0; //src->st_gid;
	tar->s_rdev = src->st_rdev;
	tar->s_size = src->st_size;
	tar->s_blksize = src->st_blksize;
	tar->s_blocks = src->st_blocks;
	tar->s_atime = src->st_atime;
	tar->s_mtime = src->st_mtime;
	tar->s_ctime = src->st_ctime;
}

int smbcw_assembleflags(char *mode)
{
	int flags = -1;

	//Validate the mode parameter
	if ((mode[0] == 'r' || mode[0] == 'w' || mode[0] == 'a' || mode[0] == 'x')
		&& ((mode[1] == '+' && mode[2] == '\0') || (mode[1] == 'b' && mode[2] == '\0' ) || mode[1] == '\0'))
	{
		//Assemble the open mode parameter
		if (mode[1] == '+')
			flags = O_RDWR;
		else
			if (mode[0] == 'r')
				flags = O_RDONLY;
			else
				flags = O_WRONLY;

		switch (mode[0])
		{
			case 'w':
				flags |= O_CREAT | O_TRUNC;
				break;
			case 'a':
				flags |= O_CREAT | O_APPEND;
				break;
			case 'x':
				flags |= O_CREAT | O_EXCL;
				break;
		}
	}

	return flags;
}

int smbcw_fopen(char *url, char *mode)
{
	errno = EINVAL;
	int ret = -1;

	lp_smbcw_url url_desc;
	lp_smbcctx ctx = smbcw_get_url_context(url, &url_desc);
	if (ctx)
	{
		smbc_open_fn open_fn = smbc_getFunctionOpen(ctx);

		if (open_fn)
		{
			//Assemble the flag parameter from the mode string
			int flags = smbcw_assembleflags(mode);

			if (flags >= 0)
			{
				//Assemble the filename smbc should open
				char *fn = smbcw_url_gen_filename(url_desc);

				//Obtain a pointer on the smbc file construct
				lp_smbcfile file = open_fn(ctx, fn, flags, 0);

				//Free the filename string again
				free(fn);

				if (file)
				{
					//Create the file descriptor which will be returned to the user of the
					//library
					int id = smbcw_create_file_desc(ctx, file, NULL);

					if (id > 0)
					{
						ret = id;
					} else {
						//The context couldn't be created for whatever reason. Close the file
						//descriptor again.
						smbc_close_fn close_fn = smbc_getFunctionClose(ctx);
						close_fn(ctx, file);
					}
				}
			}

			//Free the url_descriptor
			smbcw_url_free(url_desc);
		}
	}

	_RETURN(ret);
}

int smbcw_fclose(int fd)
{
	errno = EINVAL;
	int ret = -1;
	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL)
	{
		//Obtain the close function pointer
		smbc_close_fn close_fn = smbc_getFunctionClose(pfd->ctx);
		if (close_fn)
		{
			//Close the file
			ret = close_fn(pfd->ctx, pfd->file);

			//Free the memory reserved for the file descriptor
			free(pfd);

			//Remove the fd from the descriptor list
			smbcw_free_id(fd);
		}
	}

	 _RETURN(ret);
}

int64_t smbcw_fread(int fd, char *buf, uint64_t size)
{
	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL)
	{
		//Obtain the read function pointer
		smbc_read_fn read_fn = smbc_getFunctionRead(pfd->ctx);

		if (read_fn)
		{
			//Read from the file
			_RETURN(read_fn(pfd->ctx, pfd->file, buf, size));
		}
	}

	_RETURN_ERR(EINVAL);
}

int64_t smbcw_fwrite(int fd, char *buf, uint64_t size)
{
	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL) {

		//Obtain the write function pointer
		smbc_write_fn write_fn = smbc_getFunctionWrite(pfd->ctx);

		//Write to the file
		_RETURN(write_fn(pfd->ctx, pfd->file, buf, size));
	}

	_RETURN_ERR(EINVAL);
}

int64_t smbcw_fseek(int fd, int64_t offset, int whence)
{
	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL) {
		//Obtain the seek function pointer
		smbc_lseek_fn lseek_fn = smbc_getFunctionLseek(pfd->ctx);

		//Write to the file
		_RETURN(lseek_fn(pfd->ctx, pfd->file, offset, whence));
	}

	_RETURN_ERR(EINVAL);
}

int smbcw_fstat(int fd, smbcw_stat *stat)
{
	errno = EINVAL;
	int ret = -1;
	//Clear the stat value passed to this function
	memset(stat, 0, sizeof(*stat));

	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL) {
		//Obtain the fstat function pointer
		smbc_fstat_fn fstat_fn = smbc_getFunctionFstat(pfd->ctx);

		if (fstat_fn)
		{
			//Obtain the system internal file stat from smbc
			struct stat fstat;

			ret = fstat_fn(pfd->ctx, pfd->file, &fstat);

			//Translate the system internal stat format into our own smbcw internal stat
			//format.
			if (ret >= 0)
				smbcw_write_stat(&fstat, stat);

			return ret;
		}
	}

	_RETURN(ret);
}

int smbcw_urlstat(char *url, smbcw_stat *stat)
{
	errno = EINVAL;
	int ret = -1;

	//Clear the stat value passed to this function
	memset(stat, 0, sizeof(*stat));

	//Obtain the url context associated to this url
	lp_smbcw_url checked_url;
	lp_smbcctx ctx = smbcw_get_url_context(url, &checked_url);

	if (ctx)
	{
		smbc_stat_fn stat_fn = smbc_getFunctionStat(ctx);

		if (stat_fn)
		{
			struct stat fstat;

			char *fn = smbcw_url_gen_filename(checked_url);
			ret = stat_fn(ctx, fn, &fstat);

			free(fn);

			//Translate the system internal stat format into our own smbcw internal stat
			//format.
			if (ret >= 0)
				smbcw_write_stat(&fstat, stat);

			//Check whether the file is really readable - this is the only information
			//which might be wrong as windows only has a READONLY flag - so files
			//are always marked as readable although this might not be true when
			//connecting to a Samba/UNIX server
			int fd = smbcw_fopen(url, "r");
			if (fd > 0) {
				smbcw_fclose(fd);
			} else {
				if (smbcw_geterr() == EACCES)
				{
					// Check whether the file is a directory - if yes, the EACCES
					// error might have been triggered although we have read access
					// on the directory. If this is the case perform an "opendir"
					// to validate. Unfortunately "opendir" takes much time - thats
					// why it is placed inside the outer check.
					if (stat->s_mode & S_IFDIR)
					{
						int fd = smbcw_opendir(url);
						if (fd > 0) {
							smbcw_closedir(fd);
						} else {
							if (smbcw_geterr() == EACCES)
								stat->s_mode &= ~(0777);
						}
					}
					else
					{
						stat->s_mode &= ~(0555);
					}
				}
			}
		}

		smbcw_url_free(checked_url);
	}

	_RETURN(ret);
}

int smbcw_rename(char *url_from, char* url_to)
{
	errno = EINVAL;
	int ret = -1;

	//Obtain the two url contexts associated to the given urls
	lp_smbcw_url checked_url_from;
	lp_smbcw_url checked_url_to;

	lp_smbcctx ctx_from = smbcw_get_url_context(url_from, &checked_url_from);
	lp_smbcctx ctx_to = smbcw_get_url_context(url_to, &checked_url_to);

	//Currently smbc can only move files which are on the same share, so we check
	//whether the two contexts are the same
	if (ctx_from && (ctx_from == ctx_to))
	{
		smbc_rename_fn rename_fn = smbc_getFunctionRename(ctx_from);

		if (rename_fn)
		{
			struct stat fstat;

			//Get a clean version of the filename
			char *fn_from = smbcw_url_gen_filename(checked_url_from);
			char *fn_to = smbcw_url_gen_filename(checked_url_to);

			//Call the rename function of smbcw
			ret = rename_fn(ctx_from, fn_from, ctx_to, fn_to);
			
			//Free the filename strings and the url contexts
			free(fn_from);
			free(fn_to);
		}
	}

	//Free some structures which might still be initialized
	if (checked_url_from)
		smbcw_url_free(checked_url_from);

	if (checked_url_to)
		smbcw_url_free(checked_url_to);

	_RETURN(ret);
}

int smbcw_unlink(char *url)
{
	errno = EINVAL;
	int ret = -1;

	//Obtain the url context associated to this url
	lp_smbcw_url checked_url;
	lp_smbcctx ctx = smbcw_get_url_context(url, &checked_url);

	if (ctx)
	{
		smbc_unlink_fn unlink_fn = smbc_getFunctionUnlink(ctx);

		if (unlink_fn)
		{
			char *fn = smbcw_url_gen_filename(checked_url);
			ret = unlink_fn(ctx, fn);

			free(fn);
		}
	
		smbcw_url_free(checked_url);
	}

	_RETURN(ret);
}

int smbcw_mkdir(char *url)
{
	errno = EINVAL;
	int ret = -1;

	//Obtain the url context associated to this url
	lp_smbcw_url checked_url;
	lp_smbcctx ctx = smbcw_get_url_context(url, &checked_url);

	if (ctx)
	{
		smbc_mkdir_fn mkdir_fn = smbc_getFunctionMkdir(ctx);

		if (mkdir_fn)
		{
			char *fn = smbcw_url_gen_filename(checked_url);
			ret = mkdir_fn(ctx, fn, 0);

			free(fn);
		}
	
		smbcw_url_free(checked_url);
	}

	_RETURN(ret);
}

int smbcw_rmdir(char *url)
{
	errno = EINVAL;
	int ret = -1;

	//Obtain the url context associated to this url
	lp_smbcw_url checked_url;
	lp_smbcctx ctx = smbcw_get_url_context(url, &checked_url);

	if (ctx)
	{
		smbc_rmdir_fn rmdir_fn = smbc_getFunctionRmdir(ctx);

		if (rmdir_fn)
		{
			char *fn = smbcw_url_gen_filename(checked_url);
			ret = rmdir_fn(ctx, fn);

			free(fn);
		}
	
		smbcw_url_free(checked_url);
	}

	_RETURN(ret);
}

int smbcw_opendir(char *url)
{
	errno = EINVAL;
	int ret = -1;

	lp_smbcw_url url_desc;
	lp_smbcctx ctx = smbcw_get_url_context(url, &url_desc);
	if (ctx)
	{
		smbc_opendir_fn opendir_fn = smbc_getFunctionOpendir(ctx);

		if (opendir_fn)
		{
			//Assemble the dirname smbc should open
			char *fn = smbcw_url_gen_filename(url_desc);

			//Obtain a pointer on the smbc file construct (which is also used for dirs)
			lp_smbcfile file = opendir_fn(ctx, fn);

			//Free the filename string again
			free(fn);

			if (file)
			{
				//Create the file descriptor which will be returned to the user of the
				//library
				int id = smbcw_create_file_desc(ctx, file, NULL);

				if (id > 0)
				{
					ret = id;
				} else {
					//The context couldn't be created for whatever reason. Close the file
					//descriptor again.
					smbc_closedir_fn closedir_fn = smbc_getFunctionClosedir(ctx);
					closedir_fn(ctx, file);
				}
			}

			//Free the url_descriptor
			smbcw_url_free(url_desc);
		}
	}

	_RETURN(ret);
}

int smbcw_closedir(int fd)
{
	errno = EINVAL;
	int ret = -1;

	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL)
	{
		//Obtain the close function pointer
		smbc_closedir_fn closedir_fn = smbc_getFunctionClosedir(pfd->ctx);
		if (closedir_fn)
		{
			//Close the file
			ret = closedir_fn(pfd->ctx, pfd->file);

			//Free the memory reserved for the file descriptor
			free(pfd);

			//Remove the fd from the descriptor list
			smbcw_free_id(fd);
		}
	}

	_RETURN(ret);
}

int smbcw_readdir(int fd, char **name)
{
	errno = EINVAL;
	int ret = -1;

	//Set the output to NULL
	*name = NULL;

	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL)
	{
		//Obtain the readdir function pointer
		smbc_readdir_fn readdir_fn = smbc_getFunctionReaddir(pfd->ctx);
		if (readdir_fn)
		{
			//Read an dir entry from the directory
			struct smbc_dirent *ent = readdir_fn(pfd->ctx, pfd->file);

			if (ent != NULL)
			{
				*name = ent->name;
				_RETURN(0);
			}
		}
	}

	_RETURN(ret);
}

int smbcw_rewinddir(int fd)
{
	errno = EINVAL;
	int ret = -1;

	//Obtain the file descriptor
	lp_smbcw_file pfd = smbcw_get_ptr(fd);
	if (pfd && pfd->fd_check == FD_STRUCT_VAL)
	{
		//Obtain the seekdir function pointer
		smbc_lseekdir_fn lseekdir_fn = smbc_getFunctionLseekdir(pfd->ctx);

		if (lseekdir_fn)
			//Seek to the beginning of the file
			ret = lseekdir_fn(pfd->ctx, pfd->file, 0);
	}

	_RETURN(ret);
}

int smbcw_chmod(char *url, int mode)
{
	errno = EINVAL;
	int ret = -1;

	lp_smbcw_url url_ctx;
	lp_smbcctx ctx = smbcw_get_url_context(url, &url_ctx);

	if (ctx)
	{
		smbc_chmod_fn chmod_fn = smbc_getFunctionChmod(ctx);

		char *fn = smbcw_url_gen_filename(url_ctx);

		if (chmod_fn)
			ret = chmod_fn(ctx, fn, mode);

		free(fn);
	}

	_RETURN(ret);
}

/*void smbcw_getattrs(char *url)
{
	lp_smbcw_url url_ctx;

	lp_smbcctx ctx = smbcw_get_url_context(url, &url_ctx);
	if (ctx)
	{
		char *fn = smbcw_url_gen_filename(url_ctx);

		smbc_listxattr_fn listxattr_fn = smbc_getFunctionListxattr(ctx);
		int size = listxattr_fn(ctx, fn, NULL, 0);
		if (size >= 0)
		{
			char *buf = malloc(size);

			listxattr_fn(ctx, fn, buf, size);
			char *part = buf;
			while (size > 0)
			{
				printf("%s\n", part);

				size = size - (strlen(part) + 1);
				part = part + strlen(part) + 1;
			}

			free(buf);
		}

		free(fn);
	}
}*/

int smbcw_geterr()
{
	return smbcw_errno;
}
