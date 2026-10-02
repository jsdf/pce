/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkpri.h                                   *
 * Created:     2017-10-28 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_BLOCK_BLKPRI_H
#define PCE_BLOCK_BLKPRI_H 1


#include <config.h>

#include <drivers/block/block.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>

#include <stdint.h>
#include <stdio.h>


typedef struct {
	disk_t     dsk;

	pri_img_t  *img;

	char       dirty;

	unsigned   type;
} disk_pri_t;


disk_t *dsk_pri_open_fp (FILE *fp, unsigned type, int ro);
disk_t *dsk_pri_open (const char *fname, unsigned type, int ro);

int dsk_pri_create_fp (FILE *fp, unsigned type, uint32_t c, uint32_t h, uint32_t s);
int dsk_pri_create (const char *name, unsigned type, uint32_t c, uint32_t h, uint32_t s);

unsigned dsk_pri_probe_fp (FILE *fp);
unsigned dsk_pri_probe (const char *fname);


#endif
