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

#include <stdlib.h>
#include <string.h>

#include "smbcw_url.h"

/**
 * Splits a string to a left and a right part at the first occurence of delem.
 * Only used internally by smbcw_url_parse.
 * @param in is the string which should be splitted. The memory in points to will
 *  be freed. This means, that you should always create a duplicate of the the in string
 *  if you get it as an function parameter.
 * @param left is a pointer to the string the left half should be written to.
 * @param right is the pointer to the string the right half should be written to.
 * @param delem is the delemiter. The delimiter itself will not be included inside
 *  the result
 * @param dir marks wheter the result should be written to "left" (dir <= 0) or
 *  to "right" (dir > 0)
 */
void _str_split(char *in, char **left, char **right, const char *delem, int dir)
{
	//Get the position of the delimiter string
	int in_len = strlen(in);
	int delem_len = strlen(delem);
	int pos;

	char *dup = strdup(in);

	//Free the old string which was passed to this function
	free(in);

	//Get the position of the delimiter in the string
	char *str_pos = strstr(dup, delem);
	if (str_pos) {
		pos = str_pos - dup;
	} else {
		//If the delimiter has not been found, copy the whole string to the...
		if (dir > 0) {
			//...right side
			pos = 0;
			delem_len = 0;
		} else {
			//...left side
			pos = in_len;
		}
	}

	//Create a new string for the left side
	int left_len = pos;
	*left = malloc(pos + 1);
	memset(*left, 0, pos + 1);
	strncpy(*left, dup, pos);

	//Create a new string for the right side
	int right_len = in_len - delem_len - pos;
	if (right_len < 0)
		right_len = 0;
	*right = malloc(right_len + 1); 
	memset(*right, 0, right_len + 1);
	strncpy(*right, dup + pos + delem_len, right_len);

	//Free the duplicate of the in string
	free(dup);
}

/**
 * Return integer value of a hex character
 *
 * @param char c 0-9, A-F or a-f
 * @return int 0-15 on success or -1 on error (non-hex char)
 */
int _hex2int(char c)
{
	if ('0' <= c && c <= '9')
	{
		return c - '0';
	}
	if ('A' <= c && c <= 'F')
	{
		return 10+(c - 'A');
	}
	if ('a' <= c && c <= 'f')
	{
		return 10+(c - 'a');
	}
	return -1;	// error
}

/**
 * URL decode a string
 *
 * As a string get shorter by url decoding, we dont have to reallocate memory!
 *
 * @param char *str
 */
void _str_url_decode(char *str)
{
	char *dst = str;
	int upper, lower;

	while(*str)
	{
		if (*str == '%')
		{
			// %% --> %
			if (*(str+1) == '%')
			{
				str += 2;
				dst++;
				continue;
			}
			// %HH H=[0-9A-Fa-f], everything else is left unchanged (so we can not read over '\0'!)
			if ((upper = _hex2int(*(str+1))) != -1 && (lower = _hex2int(*(str+2))) != -1)
			{
				*dst++ = upper << 4 | lower;
				str += 3;
				continue;
			}
		}
		// + encodes a space
		if (*str == '+') *str = ' ';
		// regular char --> nothing to do, just increment both pointers
		str++;
		dst++;
	}
	*dst = '\0';
}

/**
 * Parses the given url string and writes the parts into the url descriptor.
 * Internally used.
 * @param url is the pointer to the current smbcw url descriptor
 * @param url_str is the url which should be divided.
 */
int _smbcw_url_parse(lp_smbcw_url url, char *url_str)
{
	char *tmp = NULL;
	char *tmp2 = NULL;
	char *cpy = NULL;

	cpy = strdup(url_str);

	_str_split(cpy, &(url->protocol), &tmp, "://", 1);
	if (strlen(url->protocol) > 0) {
		_str_split(tmp, &tmp, &(url->path), "/", -1);
		_str_split(tmp, &tmp, &tmp2, "@", 1);
		_str_split(tmp, &(url->user), &(url->password), ":", -1);
		_str_url_decode(url->password);
		_str_split(tmp2, &(url->host), &(url->port), ":", -1);	
	} else {
		url->path = tmp;
	}
	
	return 0;
}

/* See url.h */
lp_smbcw_url smbcw_url_create(const char *url)
{
	lp_smbcw_url result;

	/* Reserve some memory for the url descriptor we're returning and initialize it
		 with zeros */
	result = malloc(sizeof(*result));
	memset(result, 0, sizeof(*result));

	_smbcw_url_parse(result, (char*)url);

	return result;
}

/* See url.h */
lp_smbcw_url smbcw_url_dup(lp_smbcw_url url)
{
	lp_smbcw_url result;

	result = malloc(sizeof(*result));
	memset(result, 0, sizeof(*result));

	if (url->protocol)
		result->protocol = strdup(url->protocol);

	if (url->user)
		result->user = strdup(url->user);

	if (url->password)
		result->password = strdup(url->password);

	if (url->host)
		result->host = strdup(url->host);

	if (url->port)
		result->port = strdup(url->port);

	if (url->path)
		result->path = strdup(url->path);

	return result;
}

/* See url.h */
char* smbcw_url_gen_filename(lp_smbcw_url url)
{
	char *result = NULL;
	int str_len = 0;

	//Calculate how many bytes of memory have to be allocated for the resulting
	//string.
	if (url->host && url->path && url->protocol)
		str_len += strlen(url->host) + strlen(url->path) + strlen(url->protocol) + 5;

	if (str_len > 0)
	{
		//Reserve enough memory for the result string
		result = malloc(str_len);
		memset(result, 0, str_len);

		//Append the protocol, host name and the path to the url. Everything else has
		//already stored in the SMBC context and therefore we don't have to pass it
		//to SMBC again. The protocol is needed for smbc to accept the filename.
		strcat(result, url->protocol);
		strcat(result, "://");
		strcat(result, url->host);
		strcat(result, "/");
		strcat(result, url->path);
	}

	return result;
}

/* See url.h */
char* smbcw_url_get_share(lp_smbcw_url url)
{
	char *result = "";

	if (url && url->path)
	{
		//Get the the first occurence of "/" and copy the first part of the string
		char *path = url->path;
		int pos = 0;
		while (*path && (*path != '/'))
		{
			pos++;
			path++;
		}

		result = malloc(pos + 1);
		memset(result, 0, pos + 1);
		strncpy(result, url->path, pos);
	}

	return result;
}


/* See url.h */
void smbcw_url_free(lp_smbcw_url url)
{
	//Free all strings inside the url structure
	if (url->protocol)
		free(url->protocol);

	if (url->user)
		free(url->user);

	if (url->password)
		free(url->password);

	if (url->host)
		free(url->host);

	if (url->port)
		free(url->port);

	if (url->path)
		free(url->path);

	//Free the url structure itself
	free(url);
}

