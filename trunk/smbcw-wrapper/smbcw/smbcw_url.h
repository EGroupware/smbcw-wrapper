#ifndef _URL_H
#define _URL_H

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

/**
 * The t_smbcw_url structure is used to access the individual parts of an url without
 * having to reparse it all the time. The url 
 *	 http://user:foo@127.0.0.1:1080/path/to/some/file.txt
 * would parse to
 *	 protocol -> "http"
 *	 user -> "user"
 *	 password -> "foo"
 *	 host -> "127.0.0.1"
 *	 port -> "1080"
 *	 path -> "path/to/some/file.txt"
 * Note that any of the strings might also be NULL! So always test this before accessing.
 * The port parameter is currently not used in SMBCW - I don't know if it is even possible
 * to pass a port to SMBC.
 */
typedef struct {
	char *protocol;
	char *user;
	char *password;
	char *host;
	char *port;
	char *path;
} t_smbcw_url;


/**
 * Pointer on the t_smbcw_url structure
 */
typedef t_smbcw_url *lp_smbcw_url;


/**
 * Creates a new url descriptor and divides the given url into multiple parts
 * which might then be accessed. Please note that some parts of the url descriptor
 * can be NULL. So always test the url descriptor parts for not being NULL before
 * accessing them. You have to free each created url descriptor by passing it to
 * the smbcw_url_free function.
 *
 * @param url is the url which should be divided.
 */
lp_smbcw_url smbcw_url_create(const char *url);

/**
 * Every url descriptor created by the smbcw_url_create function has to be freed
 * by using this function
 *
 * @param url is the pointer to the url descriptor which should be freed.
 */
void smbcw_url_free(lp_smbcw_url url);

/**
 * Creates an duplicate of the given url in memory.
 *
 * @param url is the url structure which will be duplicated including the strings inside
 *	the url structure.
 */
lp_smbcw_url smbcw_url_dup(lp_smbcw_url url);

/**
 * Assembles an filename which may e.g. passed to the SMBC open or urlstat functions.
 * This filename only contains host and path - all other information has already
 * been stored in the SMBC context.
 */
char* smbcw_url_gen_filename(lp_smbcw_url url);

/**
 * Returns a string containing the share the url points to. You're responsible
 * for freeing the returned string.
 */
char* smbcw_url_get_share(lp_smbcw_url url);

#endif /*_URL_H*/

