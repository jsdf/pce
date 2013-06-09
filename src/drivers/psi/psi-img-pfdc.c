/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-pfdc.c                               *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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

#include "psi.h"
#include "psi-io.h"
#include "psi-img-pfdc.h"


int pfdc0_load_fp (FILE *fp, psi_img_t *img, unsigned long id, unsigned long sz);
int pfdc0_save_fp (FILE *fp, const psi_img_t *img);

int pfdc1_load_fp (FILE *fp, psi_img_t *img, unsigned long id, unsigned long sz);
int pfdc1_save_fp (FILE *fp, const psi_img_t *img);

int pfdc2_load_fp (FILE *fp, psi_img_t *img, unsigned long id, unsigned long sz);
int pfdc2_save_fp (FILE *fp, const psi_img_t *img);

int pfdc4_load_fp (FILE *fp, psi_img_t *img, unsigned long id, unsigned long sz);
int pfdc4_save_fp (FILE *fp, const psi_img_t *img);


static
int psi_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned long id, sz;
	unsigned char buf[8];

	if (psi_read (fp, buf, 8)) {
		return (1);
	}

	id = psi_get_uint32_be (buf, 0);
	sz = psi_get_uint32_be (buf, 4);

	if (id != PFDC_MAGIC) {
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

psi_img_t *psi_load_pfdc (FILE *fp)
{
	psi_img_t *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (psi_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


int psi_save_pfdc (FILE *fp, const psi_img_t *img, unsigned vers)
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

int psi_probe_pfdc_fp (FILE *fp)
{
	unsigned char buf[4];

	if (psi_read_ofs (fp, 0, buf, 4)) {
		return (0);
	}

	if (psi_get_uint32_be (buf, 0) != PFDC_MAGIC) {
		return (0);
	}

	return (1);
}

int psi_probe_pfdc (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = psi_probe_pfdc_fp (fp);

	fclose (fp);

	return (r);
}
