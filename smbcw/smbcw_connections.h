#ifndef _CONNECTIONS_H
#define _CONNECTIONS_H

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

#include <libsmbclient.h>
#include "smbcw_url.h"

/**
 * Initializes the SMBCW connection handling. Has to be called before any other 
 * connections_* function is called. SMBC has not to be initialized yet.
 */
void connections_init();

/**
 * Finalizes the SMBCW connection handling: Closes all open connections and frees
 * all memory used by the connections module.
 */
void connections_finalize();

/**
 * Returns an SMBCCTX structure for the given url - use this structure to get the
 * file access functions.
 *
 * @param url the url descriptor the context should be returned for. Which context
 *  is selected depends on the user, the password and the host the url refers to.
 *  if no such context exists, a new one will be created. The context authentification
 *  function gets automatically connected to an callback which then fills out the
 *  authentification data according to the values in the url.
 */
SMBCCTX * connections_get_ctx(lp_smbcw_url url);

#endif /*_CONNECTIONS_H*/

