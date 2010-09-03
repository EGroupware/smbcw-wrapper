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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libsmbclient.h>
#include <sys/stat.h>
#include <errno.h>

#include "smbcw.h"

#define CHECK_SMB_URL(url) (strncmp("smb://", url, 6) == 0)

int smbcw_errno = 0;

#define _RETURN(cmd)\
{\
int retval1 = cmd;\
if (retval1 < 0)\
	smbcw_errno = errno;\
else\
	smbcw_errno = 0;\
return retval1;\
}

#define _RETURN_ERR(err)\
{\
	smbcw_errno = err;\
	return -1;\
}

void smbcw_write_stat(struct stat *src, smbcw_stat *tar)
{
	tar->s_dev = src->st_dev;
	tar->s_ino = src->st_ino;
	tar->s_mode = src->st_mode;
	tar->s_nlink = src->st_nlink;
	tar->s_uid = src->st_uid;
	tar->s_gid = src->st_gid;
	tar->s_rdev = src->st_rdev;
	tar->s_size = src->st_size;
	tar->s_blksize = src->st_blksize;
	tar->s_blocks = src->st_blocks;
	tar->s_atime = src->st_atime;
	tar->s_mtime = src->st_mtime;
	tar->s_ctime = src->st_ctime;
}

static void smb_auth_fn(const char *server, const char *share,
             char *workgroup, int wgmaxlen, char *username, int unmaxlen,
             char *password, int pwmaxlen)
{
	if ((workgroup != NULL) && (wgmaxlen >= strlen("WORKGROUP")))
		strcpy(workgroup, "WORKGROUP");
	if ((username != NULL) && (unmaxlen >= strlen("guest")))
		strcpy(username, "guest");
	if ((password != NULL) && (pwmaxlen >= strlen("")))
		strcpy(password, "");
}

int smbcw_init()
{
	//Intialize libsmbclient and return the initialization result
	_RETURN(smbc_init(smb_auth_fn, 0));
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
	if (CHECK_SMB_URL(url))
	{
		//Assemble the flag parameter from the mode string
		int flags = smbcw_assembleflags(mode);
		if (flags >= 0)
		{
			//Actually perform the open and return the result
			_RETURN(smbc_open(url, flags, 0));
		}
	}
	_RETURN_ERR(EINVAL);
}

int smbcw_fclose(int fd)
{
	_RETURN(smbc_close(fd));
}

int64_t smbcw_fread(int fd, char *buf, uint64_t size)
{
	_RETURN(smbc_read(fd, buf, size));
}

int64_t smbcw_fwrite(int fd, char *buf, uint64_t size)
{
	_RETURN(smbc_write(fd, buf, size));
}

int64_t smbcw_fseek(int fd, int64_t offset, int whence)
{
	_RETURN(smbc_lseek(fd, offset, whence));
}

int smbcw_fstat(int fd, smbcw_stat *stat)
{
	struct stat fstat;
	int ret = smbc_fstat(fd, &fstat);
	if (ret >= 0)
		smbcw_write_stat(&fstat, stat);
	_RETURN(ret);
}

int smbcw_urlstat(char *url, smbcw_stat *stat)
{
	if (CHECK_SMB_URL(url))
	{
		struct stat fstat;
		int ret = smbc_stat(url, &fstat);
		if (ret >= 0)
			smbcw_write_stat(&fstat, stat);
		_RETURN(ret);
	}
	_RETURN_ERR(EINVAL);
}

int smbcw_rename(char *url_from, char* url_to)
{
	if (CHECK_SMB_URL(url_from) && CHECK_SMB_URL(url_to))
	{
		_RETURN(smbc_rename(url_from, url_to));
	}
	_RETURN_ERR(EINVAL);
}

int smbcw_unlink(char *url)
{
	if (CHECK_SMB_URL(url))
	{
		_RETURN(smbc_unlink(url));
	}
	_RETURN_ERR(EINVAL);
}

int smbcw_mkdir(char *url)
{
	if (CHECK_SMB_URL(url))
	{
		_RETURN(smbc_mkdir(url, 0));
	}
	_RETURN_ERR(EINVAL);
}

int smbcw_rmdir(char *url)
{
	if (CHECK_SMB_URL(url))
	{
		_RETURN(smbc_rmdir(url));
	}
	_RETURN_ERR(EINVAL);
}

int smbcw_opendir(char *url)
{
	if (CHECK_SMB_URL(url))
	{
		_RETURN(smbc_opendir(url));
	}
	_RETURN_ERR(EINVAL);
}

int smbcw_closedir(int fd)
{
	_RETURN(smbc_closedir(fd));
}

int smbcw_readdir(int fd, char **name)
{
	//Set the result pointer to NULL
	*name = NULL;

	struct smbc_dirent *ent = smbc_readdir(fd);
	if (ent != NULL)
	{
		*name = ent->name;
		_RETURN(0)
	}
	_RETURN(-1)
}

int smbcw_rewinddir(int fd)
{
	_RETURN(smbc_lseekdir(fd, 0));
}

int smbcw_chmod(char *url, int mode)
{
	_RETURN(smbc_chmod(url, mode));
}

int smbcw_geterr()
{
	return smbcw_errno;
}

