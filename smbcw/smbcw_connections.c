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

#include <libsmbclient.h>

#include "smbcw_common.h"
#include "smbcw_connections.h"
#include "smbcw_url.h"

/**
 * Alias for an pointer on SMBCCTX
 */
typedef SMBCCTX *lp_smbcctx;

/**
 * List element internally used to keep the url descriptor and the corresponding
 * context together
 */
typedef struct {
	lp_smbcw_url url;
	lp_smbcctx ctx;
	void *next;
} t_smbcw_connection;

/**
 * Pointer on t_smbcw_connection
 */
typedef t_smbcw_connection *lp_smbcw_connection;

/**
 * Global variable which holds the first element of the connections list.
 */
lp_smbcw_connection first_connection = NULL;

/**
 * Creates a new connection element and adds it to the connections list.
 */
lp_smbcw_connection connection_create()
{
	lp_smbcw_connection result;

	//Create a new connection structure and initialize it with zeros.
	result = malloc(sizeof(*result));
	memset(result, 0, sizeof(*result));

	//Append this new connection to the connections list
	if (first_connection) {
		//Go to the end of the connection list and set the last item to the new one
		lp_smbcw_connection tmp = first_connection;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = result;
	} else {
		first_connection = result;
	}

	return result;
}

/**
 * Frees the given connection element and removes it from the connections list.
 *
 * @param connection is a pointer to the connection element which should be freed.
 */
void connection_free(lp_smbcw_connection connection)
{
	//Iterate over the connection list and remove this item
	lp_smbcw_connection tmp = first_connection;
	lp_smbcw_connection last = NULL;
	while (tmp != connection && tmp != NULL) {
		last = tmp;
		tmp = tmp->next;
	}

	//If tmp is NULL, the item is not in the list
	if (tmp != NULL) {
		//If last is NULL, this is the first item
		if (last != NULL) {
			//Remove this item from the chain
			last->next = tmp->next;
		} else {
			//Set the first item to the next item
			first_connection = first_connection->next;
		}
	}

	//Free the url if this field is set
	if (connection->url)
		smbcw_url_free(connection->url);

	//Free the SMBCW
	if (connection->ctx)
		smbc_free_context(connection->ctx, 1);

	//Free this item
	free(connection);
}

/**
 * Checks whether the url data (host, password, user) of the given connection matches
 * the given url data. If this is the case, the function returns 1, else 0.
 */
int connection_match(lp_smbcw_connection connection, lp_smbcw_url url)
{
	//Check whether the url part of the connection is actually set. Then compare
	//the two pointers of each part as they both could be zero.
	return ((connection->url != NULL) && 
			((connection->url->user == url->user) || (strcmp(connection->url->user, url->user) == 0)) &&
			((connection->url->host == url->host) || (strcasecmp(connection->url->host, url->host) == 0)) &&
			((connection->url->password == url->password) || (strcmp(connection->url->password, url->password) == 0)) &&
			((connection->url->protocol == url->protocol) || (strcmp(connection->url->protocol, url->protocol) == 0))
	       ) ? 1 : 0;
}

/**
 * Does the same as the connection_match function, but iterates over the complete
 * connections list. Returns NULL if no item matching the url data is found else
 * the matching connection item is returned.
 */
lp_smbcw_connection connections_match(lp_smbcw_url url)
{
	lp_smbcw_connection tmp = first_connection;

	while (tmp != NULL) {
		if (connection_match(tmp, url))
			return tmp;
		tmp = tmp->next;
	}

	return NULL;
}

/* See connections.h */
void connections_init()
{
	smbc_set_context(NULL);
	connections_finalize();
}

/* See connections.h */
void connections_finalize()
{
	//Iterate over the connections list and free each one
	while (first_connection != NULL)
		connection_free(first_connection);
}

/**
 * Callback function called by libsmbclient whenever authentification for the given
 * context is needed. We're then setting the authentification data which is inside
 * the url connected with the context.
 */
void smbc_auth_callback(SMBCCTX *c, const char *srv, const char *shr, char *wg,
	int wglen, char *un, int unlen, char *pw, int pwlen)
{
	//Get the pointer to the connection from the context user data
	lp_smbcw_connection con = (lp_smbcw_connection)smbc_getOptionUserData(c);

	//Set the workgroup to the default dummy workgroup
	if (wg) {
		char *env_wg = getenv("WORKGROUP");
		if (env_wg) {
			if (env_wg && strlen(env_wg) < wglen)
				strcpy(wg, env_wg);
		} else {
			if (wg && strlen(DEFAULT_WORKGROUP) < wglen)
				strcpy(wg, DEFAULT_WORKGROUP);
		}
	}

	//Set the username the the username specified in the url - if it is set
	if (un) {
		if (con->url->user && strlen(con->url->user) < unlen) {
			strcpy(un, con->url->user);
		} else {
			if (strlen(DEFAULT_USERNAME) < unlen)
				strcpy(un, DEFAULT_USERNAME);
		}
	}

	//Set the password the the password specified in the url - if it is set
	if (pw) {
		if (con->url->password && strlen(con->url->password) < pwlen) {
			strcpy(pw, con->url->password);
		} else {
			if (strlen(DEFAULT_PASSWORD) < pwlen)
				strcpy(pw, DEFAULT_PASSWORD);
		}
	}
}

/* See connections.h */
SMBCCTX * connections_get_ctx(lp_smbcw_url url)
{
	lp_smbcw_connection con;

	//Iterate over the connections and search one which matches the given url
	con = connections_match(url);

	//If a connection has been found, simply set it as the current smbc_context
	if (con && con->ctx) {
		return con->ctx;
	} else {
		//Create a new connection
		con = connection_create();
		con->url = smbcw_url_dup(url);

		//Create a new context
		lp_smbcctx ctx = smbc_new_context();
		smbc_setDebug(ctx, 0);
		
		//Initialize the newly created context
		if (!smbc_init_context(ctx)) {
			//For some reason the initialization of the context failed. Do some cleanup

			//Free the connection
			connection_free(con);

			//Free the context variable
			smbc_free_context(ctx, 1);
			return NULL;
		}

		//Attach the context to the url
		con->ctx = ctx;

		//Set the user data parameter in order to be able to access the connection
		//structure inside of the context callback functions
		smbc_setOptionUserData(ctx, con);

		//Set the authentification callack
		smbc_setFunctionAuthDataWithContext(ctx, &smbc_auth_callback);

		return ctx;
	}
}

