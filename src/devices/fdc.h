/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/fdc.h                                            *
 * Created:     2007-09-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_FDC_H
#define PCE_DEVICES_FDC_H 1


#include <chipset/82xx/e8272.h>

#include <devices/device.h>
#include <devices/memory.h>

#include <drivers/block/block.h>


typedef struct {
	device_t  dev;
	e8272_t   e8272;

	mem_blk_t blk;

	disks_t   *dsks;
	unsigned  drive[4];
} dev_fdc_t;


dev_fdc_t *dev_fdc_new (unsigned long addr);
void dev_fdc_del (dev_fdc_t *fdc);

void dev_fdc_mem_add_io (dev_fdc_t *fdc, memory_t *io);
void dev_fdc_mem_rmv_io (dev_fdc_t *fdc, memory_t *io);

void dev_fdc_reset (dev_fdc_t *fdc);

void dev_fdc_set_disks (dev_fdc_t *fdc, disks_t *dsks);

void dev_fdc_set_drive (dev_fdc_t *fdc, unsigned fdcdrv, unsigned drive);

unsigned dev_fdc_get_drive (dev_fdc_t *fdc, unsigned fdcdrv);


#endif
