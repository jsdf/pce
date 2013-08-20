/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/fdc.h                                       *
 * Created:     2013-06-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_ATARIST_FDC_H
#define PCE_ATARIST_FDC_H 1


#include <chipset/wd179x.h>

#include <drivers/block/block.h>

#include <drivers/pri/pri.h>


typedef struct {
	wd179x_t       wd179x;
	disks_t        *dsks;

	char           use_fname[2];
	char           *fname[2];

	unsigned short diskid[2];

	unsigned char  media_change[2];
	unsigned long  media_change_clk;

	pri_img_t      *img[2];

	char           modified[2];
} st_fdc_t;


void st_fdc_set_head (st_fdc_t *fdc, unsigned h);

void st_fdc_init (st_fdc_t *fdc);
void st_fdc_free (st_fdc_t *fdc);

void st_fdc_reset (st_fdc_t *fdc);

void st_fdc_set_disks (st_fdc_t *fdc, disks_t *dsks);
void st_fdc_set_disk_id (st_fdc_t *fdc, unsigned drive, unsigned diskid);

void st_fdc_set_fname (st_fdc_t *fdc, unsigned drive, const char *fname);

int st_fdc_insert (st_fdc_t *fdc, const char *str);

unsigned char st_fdc_get_reserve (const st_fdc_t *fdc);
void st_fdc_set_reserve (st_fdc_t *fdc, unsigned char val);

unsigned char st_fdc_get_fcr (const st_fdc_t *fdc);
void st_fdc_set_fcr (st_fdc_t *fdc, unsigned char val);

int st_fdc_save (st_fdc_t *fdc, unsigned drive);
int st_fdc_load (st_fdc_t *fdc, unsigned drive);

void st_fdc_clock_media_change (st_fdc_t *fdc, unsigned cnt);

#define st_fdc_clock(fdc, cnt) wd179x_clock (&(fdc)->wd179x, cnt)


#endif
