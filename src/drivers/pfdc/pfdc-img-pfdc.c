/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-img-pfdc.c                             *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pfdc.h"
#include "pfdc-io.h"
#include "pfdc-img-pfdc.h"


int pfdc0_load_fp (FILE *fp, pfdc_img_t *img, unsigned long id, unsigned long sz);
int pfdc0_save_fp (FILE *fp, const pfdc_img_t *img);

int pfdc1_load_fp (FILE *fp, pfdc_img_t *img, unsigned long id, unsigned long sz);
int pfdc1_save_fp (FILE *fp, const pfdc_img_t *img);

int pfdc2_load_fp (FILE *fp, pfdc_img_t *img, unsigned long id, unsigned long sz);
int pfdc2_save_fp (FILE *fp, const pfdc_img_t *img);

int pfdc4_load_fp (FILE *fp, pfdc_img_t *img, unsigned long id, unsigned long sz);
int pfdc4_save_fp (FILE *fp, const pfdc_img_t *img);


static
int pfdc_load_fp (FILE *fp, pfdc_img_t *img)
{
	unsigned long id, sz;
	unsigned char buf[8];

	if (pfdc_read (fp, buf, 8)) {
		return (1);
	}

	id = pfdc_get_uint32_be (buf, 0);
	sz = pfdc_get_uint32_be (buf, 4);

	if (id != PFDC_MAGIC_PFDC) {
		return (1);
	}

	if (sz == 0x00000000) {
		return (pfdc0_load_fp (fp, img, id, sz));
	}
	else if (sz == 0x00010000) {
		return (pfdc1_load_fp (fp, img, id, sz));
	}
	else if (sz == 0x00020000) {
		return (pfdc2_load_fp (fp, img, id, sz));
	}
	else {
		return (pfdc4_load_fp (fp, img, id, sz));
	}

	return (1);
}

pfdc_img_t *pfdc_load_pfdc (FILE *fp)
{
	pfdc_img_t *img;

	img = pfdc_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (pfdc_load_fp (fp, img)) {
		pfdc_img_del (img);
		return (NULL);
	}

	return (img);
}


int pfdc_save_pfdc (FILE *fp, const pfdc_img_t *img, unsigned vers)
{
	if (vers == 0) {
		return (pfdc0_save_fp (fp, img));
	}
	else if (vers == 1) {
		return (pfdc1_save_fp (fp, img));
	}
	else if (vers == 2) {
		return (pfdc2_save_fp (fp, img));
	}
	else if (vers == 4) {
		return (pfdc4_save_fp (fp, img));
	}
	else {
		return (pfdc4_save_fp (fp, img));
	}

	return (0);
}

int pfdc_probe_pfdc_fp (FILE *fp)
{
	unsigned char buf[4];

	if (pfdc_read_ofs (fp, 0, buf, 4)) {
		return (0);
	}

	if (pfdc_get_uint32_be (buf, 0) != PFDC_MAGIC_PFDC) {
		return (0);
	}

	return (1);
}

int pfdc_probe_pfdc (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = pfdc_probe_pfdc_fp (fp);

	fclose (fp);

	return (r);
}
