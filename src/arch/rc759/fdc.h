/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/fdc.h                                         *
 * Created:     2012-07-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_RC759_FDC_H
#define PCE_RC759_FDC_H 1


#include <chipset/wd179x.h>

#include <drivers/block/block.h>

#include <drivers/pri/pri.h>


typedef struct {
	wd179x_t       wd179x;

	unsigned char  reserve;
	unsigned char  fcr;

	disks_t        *dsks;

	char           use_fname[2];
	char           *fname[2];

	unsigned short diskid[2];

	pri_img_t      *img[2];

	char           modified[2];
} rc759_fdc_t;


void rc759_fdc_init (rc759_fdc_t *fdc);
void rc759_fdc_free (rc759_fdc_t *fdc);

void rc759_fdc_reset (rc759_fdc_t *fdc);

void rc759_fdc_set_disks (rc759_fdc_t *fdc, disks_t *dsks);
void rc759_fdc_set_disk_id (rc759_fdc_t *fdc, unsigned drive, unsigned diskid);

void rc759_fdc_set_fname (rc759_fdc_t *fdc, unsigned drive, const char *fname);

int rc759_fdc_insert (rc759_fdc_t *fdc, const char *str);

unsigned char rc759_fdc_get_reserve (const rc759_fdc_t *fdc);
void rc759_fdc_set_reserve (rc759_fdc_t *fdc, unsigned char val);

unsigned char rc759_fdc_get_fcr (const rc759_fdc_t *fdc);
void rc759_fdc_set_fcr (rc759_fdc_t *fdc, unsigned char val);

int rc759_fdc_save (rc759_fdc_t *fdc, unsigned drive);
int rc759_fdc_load (rc759_fdc_t *fdc, unsigned drive);

#define rc759_fdc_clock(fdc, cnt) wd179x_clock (fdc, cnt)


#endif
