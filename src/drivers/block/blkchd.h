/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkchd.h                                   *
 * Created:     2017-12-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_BLOCK_BLKCHD_H
#define PCE_DEVICES_BLOCK_BLKCHD_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <stdio.h>
#include <stdint.h>


#define CHD_CACHE_SIZE 16


typedef struct {
	disk_t        dsk;

	FILE          *fp;

	unsigned char header[128];

	uint64_t      map_offset;
	uint64_t      image_size;
	uint32_t      hunk_size;

	uint32_t      next_hunk;

	unsigned char cache_valid[CHD_CACHE_SIZE];
	uint32_t      cache_src[CHD_CACHE_SIZE];
	uint32_t      cache_dst[CHD_CACHE_SIZE];

	uint32_t      hunks;
	uint32_t      *map;
} disk_chd_t;


disk_t *dsk_chd_open_fp (FILE *fp, int ro);
disk_t *dsk_chd_open (const char *fname, int ro);

int dsk_chd_create_fp (FILE *fp,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s
);

int dsk_chd_create (const char *fname,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s
);

int dsk_chd_probe_fp (FILE *fp);
int dsk_chd_probe (const char *fname);


#endif
