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

#ifndef _SMBCW_H
#define _SMBCW_H

#include <stdint.h>

/* The fixed size stat construct internally used by smbcw. See bits/stath.h for
	 more detail */
typedef struct smbcw_stat{
	uint32_t s_dev;			/* device */
	uint32_t s_ino;			/* inode */
	uint32_t s_mode;		/* protection */
	uint32_t s_nlink;		/* number of hard links */
	uint32_t s_uid;			/* user ID of owner */
	uint32_t s_gid;			/* group ID of owner */
	uint32_t s_rdev;		/* device type (if inode device) */
	uint64_t s_size;		/* total size, in bytes */
	uint32_t s_blksize; /* blocksize for filesystem I/O */
	uint32_t s_blocks;	/* number of blocks allocated */
	uint32_t s_atime;		/* time of last access */
	uint32_t s_mtime;		/* time of last modification */
	uint32_t s_ctime;		/* time of last change */
} smbcw_stat;

/* Inits smbcw. Returns -1 if an error occurred, 0 if the operation was successful. */
extern int smbcw_init();

/* Finalizes smbcw. Allways call this function before you quit your application to 
 * avoid memory leaks.
 */
extern void smbcw_finalize();

/* Opens the file specified by url. Mode might be one of "r,w,a,x,r+,w+,a+,x+".
    r  : O_RDONLY
    r+ : O_RDWR
    w  : O_WRONLY | O_CREAT | O_TRUNC
    w+ : O_RDWR | O_CREAT | O_TRUNC
    a  : O_WRONLY | O_CREAT | O_APPEND
    a+ : O_RDWR | O_CREAT | O_APPEND
    x  : O_WRONLY | O_CREAT | O_EXCL
    x+ : O_RDWR | O_CREAT | O_EXCL 
*/
extern int smbcw_fopen(char *url, char *mode);
/* Closes the file specified by the file descriptor fd. */
extern int smbcw_fclose(int fd);
/* Reads size bytes to buf from fd */
extern int64_t smbcw_fread(int fd, char *buf, uint64_t size);
/* Writes size bytes from buf to fd */
extern int64_t smbcw_fwrite(int fd, char *buf, uint64_t size);
/* Seeks offset bytes from whence (SEEK_SET, SEEK_CUR, SEEK_END) in fd */
extern int64_t smbcw_fseek(int fd, int64_t offset, int whence);
/* Writes the file stat structure of the opened file fd to stat */
extern int smbcw_fstat(int fd, smbcw_stat *stat);

/* Retrieves the file stat structure of a non opened file and writes it to stat */
extern int smbcw_urlstat(char *url, smbcw_stat *stat);
/* Renames the file specified by url_from to url_to */
extern int smbcw_rename(char *url_from, char* url_to);
/* Deletes the file specified by url */
extern int smbcw_unlink(char *url);
/* Creates a new directory specified by url */
extern int smbcw_mkdir(char *url);
/* Removes the directory specified by url*/
extern int smbcw_rmdir(char *url);
extern int smbcw_chmod(char *url, int mode);

/* Openes the directory specified by url and returns a directory descriptor > 0
   on success, -1 on failure */
extern int smbcw_opendir(char *url);
/* Closes a directory descriptor previously opened by smbcw_opendir */
extern int smbcw_closedir(int fd);
/* Reads a directory entry from the directory descriptor fd and writes the file name
   to name. If name is NULL, readdir has finished. The pointer supplied by name remains
   valid until the next smbcw_readdir is called or smbcw_closedir or smbcw_rewinddir
   is called.*/
extern int smbcw_readdir(int fd, char **name);
/* Rewinds the directory pointer */
extern int smbcw_rewinddir(int fd);

/* Returns the error code which has been set whenever any of the SMBCW functions
   failed. */
extern int smbcw_geterr();

//extern void smbcw_getattrs(char *url);

#endif /* _SMBCW_H */

