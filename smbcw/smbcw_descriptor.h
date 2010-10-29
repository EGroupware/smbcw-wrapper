#ifndef _DESC_H
#define _DESC_H

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
 * Generates an id which can be passed to an external application
 */
int smbcw_gen_id(void *ptr);

/**
 * Frees the memory used for managing the given id
 */
void smbcw_free_id(int id);

/**
 * Returns the pointer the given id is connected with. Returns NULL if the ID
 * isn't registered.
 */
void* smbcw_get_ptr(int id);

#endif /*_DESC_H*/

