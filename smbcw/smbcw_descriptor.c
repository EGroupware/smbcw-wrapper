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

#include "smbcw_descriptor.h"

typedef struct{
	int id;
	void *ptr;
	void *next;
} smbcw_id_entry;

typedef smbcw_id_entry *lp_smbcw_id_entry;

lp_smbcw_id_entry first_entry = NULL;

/**
 * Search in the linked list whether the given id does already exist
 */
int smbcw_id_exists(int id)
{
	lp_smbcw_id_entry tmp = first_entry;

	while (tmp) {
		if (tmp->id == id)
			return 1;
		tmp = tmp->next;
	}

	return 0;
}

int smbcw_gen_id(void *ptr)
{
	//Allocate memory for a new id entry
	lp_smbcw_id_entry new = malloc(sizeof(*new));

	//Initialize the newly allocated memory with zeros
	memset(new, 0, sizeof(*new));

	//Associate the new list element with the given pointer
	new->ptr = ptr;

	//Generate a rather random id by calculating the address of the pointer mod the
	//size of the resulting id
	int mod_mask = (1 << ((sizeof(new->id) * 8) - 2));
	new->id = *((int*)(&ptr)) % mod_mask;
	if (new->id < 1)
		new->id = 1;

	//Check whether this id already exists
	int cnt = 0;
	while (smbcw_id_exists(new->id) && (cnt < mod_mask))
	{
		new->id = ((new-> id + 1) % mod_mask);
		if (new->id < 1)
			new->id = 1;
		++cnt;
	}

	//Return -1 if all ids are exhausted
	if (cnt >= mod_mask) {
		free(new);

		return -1;
	}

	//Append the new item to the list
	if (!first_entry)
	{
		first_entry = new;
	}
	else
	{
		lp_smbcw_id_entry tmp = first_entry;
		while (tmp->next) {
			tmp = tmp->next;
		}

		tmp->next = new;
	}

	return new->id;
}

void smbcw_free_id(int id)
{
	//Remove the element from the linked list and delete it
	lp_smbcw_id_entry tmp = first_entry;
	lp_smbcw_id_entry prev = NULL;
	while (tmp)
	{
		if (tmp->id == id)
		{
			if (prev)
				prev->next = tmp->next;
			else
				first_entry = first_entry->next;

			free(tmp);

			return;
		}

		prev = tmp;
		tmp = tmp->next;
	}
}

void* smbcw_get_ptr(int id)
{
	//Search for an entry with the given id and return it
	lp_smbcw_id_entry tmp = first_entry;
	while (tmp)
	{
		if (tmp->id == id)
			 return tmp->ptr;

		tmp = tmp->next;
	}

	return NULL;
}


