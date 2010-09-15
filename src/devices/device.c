/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/device.c                                         *
 * Created:     2005-12-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2009 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <stdlib.h>
#include <string.h>

#include "device.h"


void dev_init (device_t *dev, void *ext, const char *type)
{
	dev->next = NULL;
	dev->refcnt = 1;
	dev->type = type;

	dev->free = NULL;
	dev->del = NULL;

	dev->clock = NULL;

	dev->ext = ext;
}

void dev_free (device_t *dev)
{
	if ((dev != NULL) && (dev->free != NULL)) {
		dev->free (dev);
	}
}

device_t *dev_new (void *ext, const char *type)
{
	device_t *dev;

	dev = malloc (sizeof (device_t));
	if (dev == NULL) {
		return (NULL);
	}

	dev_init (dev, ext, type);

	return (dev);
}

void dev_del (device_t *dev)
{
	if (dev == NULL) {
		return;
	}

	if (dev->del != NULL) {
		dev->del (dev);
	}
	else {
		dev_free (dev);
		free (dev->ext);
		free (dev);
	}
}

void dev_ref (device_t *dev)
{
	dev->refcnt += 1;
}

void dev_unref (device_t *dev)
{
	dev->refcnt -= 1;

	if (dev->refcnt == 0) {
		dev_del (dev);
	}
}


void dev_lst_init (dev_list_t *lst)
{
	lst->cnt = 0;
	lst->head = 0;
	lst->tail = 0;
}

void dev_lst_free (dev_list_t *lst)
{
	device_t *dev;

	while (lst->head != NULL) {
		dev = lst->head;
		lst->head = lst->head->next;

		dev_unref (dev);
	}
}

void dev_lst_add (dev_list_t *lst, device_t *dev)
{
	if (lst->head == NULL) {
		lst->head = dev;
	}
	else {
		lst->tail->next = dev;
	}

	lst->tail = dev;

	lst->cnt += 1;
}

void dev_lst_insert (dev_list_t *lst, device_t *dev)
{
	dev->next = lst->head;
	lst->head = dev;

	if (lst->tail == NULL) {
		lst->tail = dev;
	}
}

device_t *dev_lst_get (dev_list_t *lst, const char *type, unsigned idx)
{
	device_t *dev;

	dev = lst->head;

	while (dev != NULL) {
		if (strcmp (dev->type, type) == 0) {
			if (idx == 0) {
				return (dev);
			}
			else {
				idx -= 1;
			}
		}

		dev = dev->next;
	}

	return (NULL);
}

void *dev_lst_get_ext (dev_list_t *lst, const char *type, unsigned idx)
{
	device_t *dev;

	dev = dev_lst_get (lst, type, idx);

	if (dev == NULL) {
		return (NULL);
	}

	return (dev->ext);
}

void dev_lst_clock (dev_list_t *lst, unsigned n)
{
	device_t *dev;

	dev = lst->head;

	while (dev != NULL) {
		if (dev->clock != NULL) {
			dev->clock (dev->ext, n);
		}

		dev = dev->next;
	}
}
