/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkraw.h                                   *
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


#ifndef PCE_DEVICES_BLOCK_BLKIMG_H
#define PCE_DEVICES_BLOCK_BLKIMG_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <stdio.h>
#include <stdint.h>


/*!***************************************************************************
 * @short The image file disk structure
 *****************************************************************************/
typedef struct {
	disk_t   dsk;

	FILE     *fp;

	uint64_t start;
} disk_img_t;


disk_t *dsk_img_open_fp (FILE *fp, uint64_t ofs, int ro);
disk_t *dsk_img_open (const char *fname, uint64_t ofs, int ro);

void dsk_img_set_offset (disk_t *dsk, uint64_t ofs);

int dsk_img_create_fp (FILE *fp, uint32_t n, uint64_t ofs);
int dsk_img_create (const char *fname, uint32_t n, uint64_t ofs);


#endif
