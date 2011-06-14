/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkcow.h                                   *
 * Created:     2004-09-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_BLOCK_BLKCOW_H
#define PCE_DEVICES_BLOCK_BLKCOW_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <stdio.h>
#include <stdint.h>


/*!***************************************************************************
 * @short The copy on write disk structure
 *****************************************************************************/
typedef struct {
	disk_t        dsk;

	disk_t        *orig;

	FILE          *fp;

	uint64_t      bitmap_offset;
	uint64_t      data_offset;

	unsigned char *bitmap;
	uint32_t      bitmap_size;
} disk_cow_t;


disk_t *dsk_cow_new (disk_t *dsk, const char *fname);


#endif
