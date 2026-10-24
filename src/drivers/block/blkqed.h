/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkqed.h                                   *
 * Created:     2011-05-10 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_DEVICES_BLOCK_BLKQED_H
#define PCE_DEVICES_BLOCK_BLKQED_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <stdio.h>
#include <stdint.h>


/*!***************************************************************************
 * @short The QED image file disk structure
 *****************************************************************************/
typedef struct {
	disk_t        dsk;

	disk_t        *next;

	unsigned long cluster_size;
	unsigned long table_size;
	unsigned long header_size;

	uint64_t      features;
	uint64_t      compat_features;
	uint64_t      autoclear_features;

	uint64_t      l1_table_offset;
	uint64_t      image_size;

	unsigned long table_bytes;
	uint64_t      cluster_mask;

	uint64_t      offset;

	unsigned char header[4096];
	char          header_modified;

	uint64_t      l2_table_offset;

	unsigned char *t1;
	unsigned char *t2;
	unsigned char *cl;

	FILE          *fp;
} disk_qed_t;


disk_t *dsk_qed_open_fp (FILE *fp, int ro);
disk_t *dsk_qed_open (const char *fname, int ro);

disk_t *dsk_qed_cow_open (disk_t *dsk, const char *fname);
disk_t *dsk_qed_cow_create (disk_t *dsk, const char *fname, uint32_t n, uint32_t minblk);

int dsk_qed_create_fp (FILE *fp, uint32_t n, uint32_t minclst);
int dsk_qed_create (const char *fname, uint32_t n, uint32_t minclst);

int dsk_qed_probe_fp (FILE *fp);
int dsk_qed_probe (const char *fname);


#endif
