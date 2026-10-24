/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkpbi.h                                   *
 * Created:     2018-02-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_DEVICES_BLOCK_BLKPBI_H
#define PCE_DEVICES_BLOCK_BLKPBI_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <stdio.h>
#include <stdint.h>


/*!***************************************************************************
 * @short The PBI image file disk structure
 *****************************************************************************/
typedef struct {
	disk_t        dsk;

	disk_t        *next;

	unsigned long header_size;

	unsigned char l1bits;
	unsigned char l2bits;
	unsigned char blbits;

	char          header_modified;

	uint64_t      image_size;
	uint64_t      l1_table_offset;
	uint64_t      file_size;

	uint32_t      c;
	uint16_t      h;
	uint16_t      s;

	unsigned long block_size;
	uint64_t      block_mask;
	uint64_t      l2_mask;

	unsigned long l1_table_entries;
	unsigned long l2_table_entries;

	unsigned long l1_table_size;
	unsigned long l2_table_size;

	unsigned char header[512];

	uint64_t      l2_table_offset;

	unsigned char *t1;
	unsigned char *t2;
	unsigned char *bl;

	FILE          *fp;
} disk_pbi_t;


int dsk_pbi_set_uniform (disk_pbi_t *pbi, uint64_t ofs, unsigned long val);

disk_t *dsk_pbi_open_fp (FILE *fp, int ro);
disk_t *dsk_pbi_open (const char *fname, int ro);

disk_t *dsk_pbi_cow_open (disk_t *dsk, const char *fname);
disk_t *dsk_pbi_cow_create (disk_t *dsk, const char *fname, uint32_t n, uint32_t c, uint32_t h, uint32_t s, uint32_t minblk);

int dsk_pbi_create_flat_fp (FILE *fp, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk);
int dsk_pbi_create_flat (const char *fname, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk);

int dsk_pbi_create_fp (FILE *fp, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk);
int dsk_pbi_create (const char *fname, uint32_t n, uint32_t c, uint16_t h, uint16_t s, uint32_t minblk);

int dsk_pbi_probe_fp (FILE *fp);
int dsk_pbi_probe (const char *fname);


#endif
