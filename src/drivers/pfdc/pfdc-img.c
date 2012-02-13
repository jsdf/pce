/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-img.c                                  *
 * Created:     2012-02-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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
#include "pfdc-img.h"
#include "pfdc-img-ana.h"
#include "pfdc-img-cp2.h"
#include "pfdc-img-dc42.h"
#include "pfdc-img-imd.h"
#include "pfdc-img-pfdc.h"
#include "pfdc-img-raw.h"
#include "pfdc-img-tc.h"
#include "pfdc-img-td0.h"


unsigned pfdc_guess_type (const char *fname)
{
	unsigned   i;
	const char *ext;

	ext = "";

	i = 0;
	while (fname[i] != 0) {
		if (fname[i] == '.') {
			ext = fname + i;
		}

		i += 1;
	}

	if (strcasecmp (ext, ".ana") == 0) {
		return (PFDC_FORMAT_ANADISK);
	}
	else if (strcasecmp (ext, ".cp2") == 0) {
		return (PFDC_FORMAT_CP2);
	}
	else if (strcasecmp (ext, ".image") == 0) {
		return (PFDC_FORMAT_DC42);
	}
	else if (strcasecmp (ext, ".ima") == 0) {
		return (PFDC_FORMAT_RAW);
	}
	else if (strcasecmp (ext, ".imd") == 0) {
		return (PFDC_FORMAT_IMD);
	}
	else if (strcasecmp (ext, ".img") == 0) {
		return (PFDC_FORMAT_RAW);
	}
	else if (strcasecmp (ext, ".raw") == 0) {
		return (PFDC_FORMAT_RAW);
	}
	else if (strcasecmp (ext, ".tc") == 0) {
		return (PFDC_FORMAT_TC);
	}
	else if (strcasecmp (ext, ".td0") == 0) {
		return (PFDC_FORMAT_TD0);
	}

	return (PFDC_FORMAT_PFDC);
}

pfdc_img_t *pfdc_load_fp (FILE *fp, unsigned type)
{
	switch (type) {
	case PFDC_FORMAT_ANADISK:
		return (pfdc_load_anadisk (fp));

	case PFDC_FORMAT_CP2:
		return (pfdc_load_cp2 (fp));

	case PFDC_FORMAT_DC42:
		return (pfdc_load_dc42 (fp));

	case PFDC_FORMAT_IMD:
		return (pfdc_load_imd (fp));

	case PFDC_FORMAT_PFDC:
	case PFDC_FORMAT_PFDC0:
	case PFDC_FORMAT_PFDC1:
	case PFDC_FORMAT_PFDC2:
	case PFDC_FORMAT_PFDC4:
		return (pfdc_load_pfdc (fp));

	case PFDC_FORMAT_RAW:
		return (pfdc_load_raw (fp));

	case PFDC_FORMAT_TC:
		return (pfdc_load_tc (fp));

	case PFDC_FORMAT_TD0:
		return (pfdc_load_td0 (fp));
	}

	return (NULL);
}

pfdc_img_t *pfdc_load (const char *fname, unsigned type)
{
	FILE       *fp;
	pfdc_img_t *img;

	if (type == PFDC_FORMAT_NONE) {
		type = pfdc_probe (fname);
	}

	if (type == PFDC_FORMAT_NONE) {
		type = pfdc_guess_type (fname);
	}

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (NULL);
	}

	img = pfdc_load_fp (fp, type);

	fclose (fp);

	return (img);
}


int pfdc_save_fp (FILE *fp, const pfdc_img_t *img, unsigned type)
{
	switch (type) {
	case PFDC_FORMAT_ANADISK:
		return (pfdc_save_anadisk (fp, img));

	case PFDC_FORMAT_CP2:
		return (pfdc_save_cp2 (fp, img));

	case PFDC_FORMAT_DC42:
		return (pfdc_save_dc42 (fp, img));

	case PFDC_FORMAT_IMD:
		return (pfdc_save_imd (fp, img));

	case PFDC_FORMAT_PFDC:
		return (pfdc_save_pfdc (fp, img, -1));

	case PFDC_FORMAT_PFDC0:
		return (pfdc_save_pfdc (fp, img, 0));

	case PFDC_FORMAT_PFDC1:
		return (pfdc_save_pfdc (fp, img, 1));

	case PFDC_FORMAT_PFDC2:
		return (pfdc_save_pfdc (fp, img, 2));

	case PFDC_FORMAT_PFDC4:
		return (pfdc_save_pfdc (fp, img, 4));

	case PFDC_FORMAT_RAW:
		return (pfdc_save_raw (fp, img));

	case PFDC_FORMAT_TD0:
		return (pfdc_save_td0 (fp, img));
	}

	return (1);
}

int pfdc_save (const char *fname, const pfdc_img_t *img, unsigned type)
{
	int  r;
	FILE *fp;

	if (type == PFDC_FORMAT_NONE) {
		type = pfdc_guess_type (fname);
	}

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = pfdc_save_fp (fp, img, type);

	fclose (fp);

	return (r);
}


unsigned pfdc_probe_fp (FILE *fp)
{
	if (pfdc_probe_pfdc_fp (fp)) {
		return (PFDC_FORMAT_PFDC);
	}

	if (pfdc_probe_td0_fp (fp)) {
		return (PFDC_FORMAT_TD0);
	}

	if (pfdc_probe_imd_fp (fp)) {
		return (PFDC_FORMAT_IMD);
	}

	if (pfdc_probe_dc42_fp (fp)) {
		return (PFDC_FORMAT_DC42);
	}

	return (PFDC_FORMAT_NONE);
}

unsigned pfdc_probe (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (PFDC_FORMAT_NONE);
	}

	r = pfdc_probe_fp (fp);

	fclose (fp);

	return (r);
}
