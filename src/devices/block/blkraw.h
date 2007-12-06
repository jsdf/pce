/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/block/blkraw.h                                 *
 * Created:       2004-09-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2007 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#ifndef PCE_DEVICES_BLOCK_BLKIMG_H
#define PCE_DEVICES_BLOCK_BLKIMG_H 1


#include <config.h>

#include <devices/block/block.h>

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


disk_t *dsk_img_open_fp (FILE *fp,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s,
	uint64_t ofs, int ro
);

disk_t *dsk_img_open (const char *fname,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s,
	uint64_t ofs, int ro
);

disk_t *dsk_dosimg_open_fp (FILE *fp, uint64_t ofs, int ro);
disk_t *dsk_dosimg_open (const char *fname, uint64_t ofs, int ro);

disk_t *dsk_mbrimg_open_fp (FILE *fp, uint64_t ofs, int ro);
disk_t *dsk_mbrimg_open (const char *fname, uint64_t ofs, int ro);

disk_t *dsk_hfsimg_open_fp (FILE *fp, uint64_t ofs, int ro);
disk_t *dsk_hfsimg_open (const char *fname, uint64_t ofs, int ro);

disk_t *dsk_macimg_open_fp (FILE *fp, uint64_t ofs, int ro);
disk_t *dsk_macimg_open (const char *fname, uint64_t ofs, int ro);

disk_t *dsk_fdimg_open_fp (FILE *fp, uint64_t ofs, int ro);
disk_t *dsk_fdimg_open (const char *fname, uint64_t ofs, int ro);

void dsk_img_set_offset (disk_t *dsk, uint64_t ofs);

int dsk_img_create_fp (FILE *fp,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s,
	uint64_t ofs
);

int dsk_img_create (const char *fname,
	uint32_t n, uint32_t c, uint32_t h, uint32_t s,
	uint64_t ofs
);


#endif
