/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/device.h                                         *
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


#ifndef PCE_DEVICE_H
#define PCE_DEVICE_H 1


typedef struct device_s {
	struct device_s *next;
	unsigned        refcnt;
	const char      *type;

	void            (*free) (struct device_s *dev);
	void            (*del) (struct device_s *dev);

	void            (*clock) (void *ext, unsigned n);

	void            *ext;
} device_t;


typedef struct {
	unsigned cnt;
	device_t *head;
	device_t *tail;
} dev_list_t;


void dev_init (device_t *dev, void *ext, const char *type);
void dev_free (device_t *dev);

device_t *dev_new (void *ext, const char *type);
void dev_del (device_t *dev);

void dev_ref (device_t *dev);
void dev_unref (device_t *dev);


void dev_lst_init (dev_list_t *lst);
void dev_lst_free (dev_list_t *lst);

void dev_lst_add (dev_list_t *lst, device_t *dev);
void dev_lst_insert (dev_list_t *lst, device_t *dev);
device_t *dev_lst_get (dev_list_t *lst, const char *type, unsigned idx);
void *dev_lst_get_ext (dev_list_t *lst, const char *type, unsigned idx);

void dev_lst_clock (dev_list_t *lst, unsigned n);


#endif
