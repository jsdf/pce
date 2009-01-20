/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/clock/ds1743.h                                   *
 * Created:     2006-12-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_CLOCK_DS1743_H
#define PCE_DEVICES_CLOCK_DS1743_H 1


#include <devices/device.h>
#include <devices/memory.h>

#include <chipset/clock/ds1743.h>


typedef struct {
	device_t  dev;
	ds1743_t  ds1743;
	mem_blk_t blk;

	FILE      *fp;
	int       fpclose;
} dev_ds1743_t;


dev_ds1743_t *dev_ds1743_new (unsigned long addr, unsigned long size);
void dev_ds1743_del (dev_ds1743_t *rtc);

void dev_ds1743_set_date (dev_ds1743_t *rtc);

int dev_ds1743_set_fp (dev_ds1743_t *rtc, FILE *fp, int close);
int dev_ds1743_set_fname (dev_ds1743_t *rtc, const char *fname);
int dev_ds1743_save (dev_ds1743_t *rtc);


#endif
