/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkpart.h                                  *
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


#ifndef PCE_DEVICES_BLOCK_BLKPART_H
#define PCE_DEVICES_BLOCK_BLKPART_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <stdio.h>
#include <stdint.h>


#define DSK_PART_MAX 16


/*!***************************************************************************
 * @short The partitioned image file disk structure
 *****************************************************************************/
typedef struct {
	disk_t        dsk;

	unsigned      part_cnt;

	struct {
		uint32_t block_i;
		uint32_t block_n;
		uint64_t start;
		FILE     *fp;
		int      close;
		int      ro;
	} part[DSK_PART_MAX];
} disk_part_t;


/*!***************************************************************************
 * @short Add a partition
 *****************************************************************************/
int dsk_part_add_partition_fp (disk_t *dsk, FILE *fp, int close,
	uint64_t start, uint32_t blk_i, uint32_t blk_n, int ro);

int dsk_part_add_partition (disk_t *dsk, const char *fname,
	uint64_t start, uint32_t blk_i, uint32_t blk_n, int ro);

/*!***************************************************************************
 * @short Create a new partition image disk
 *****************************************************************************/
disk_t *dsk_part_open (uint32_t c, uint32_t h, uint32_t s, int ro);


#endif
