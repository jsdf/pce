/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfdc/pfdc-img-io.c                                 *
 * Created:     2010-08-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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

#include <drivers/block/pfdc-img-ana.h>
#include <drivers/block/pfdc-img-dc42.h>
#include <drivers/block/pfdc-img-imd.h>
#include <drivers/block/pfdc-img-pfdc.h>
#include <drivers/block/pfdc-img-raw.h>
#include <drivers/block/pfdc-img-td0.h>

#include "pfdc-img-io.h"


static
int pfdc_get_file_size (const char *fname, unsigned long *size)
{
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (1);
	}

	if (fseek (fp, 0, SEEK_END) != 0) {
		fclose (fp);
		return (1);
	}

	*size = ftell (fp);

	fclose (fp);

	return (0);
}

static
int pfdc_is_raw (const char *fname)
{
	unsigned long size;
	unsigned      c, h, s;

	if (pfdc_get_file_size (fname, &size)) {
		return (0);
	}

	if (pfdc_get_geometry_from_size (size, &c, &h, &s)) {
		return (0);
	}

	return (1);
}

static
unsigned pfdc_get_type (unsigned type, const char *fname, int save)
{
	unsigned   i;
	const char *ext;

	if (type != PFDC_FORMAT_NONE) {
		return (type);
	}

	ext = "";

	i = 0;
	while (fname[i] != 0) {
		if (fname[i] == '.') {
			ext = fname + i;
		}

		i += 1;
	}

	if (strcasecmp (ext, ".ana") == 0) {
		return (PFDC_FORMAT_ANA);
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
	else if (strcasecmp (ext, ".raw") == 0) {
		return (PFDC_FORMAT_RAW);
	}
	else if (strcasecmp (ext, ".td0") == 0) {
		return (PFDC_FORMAT_TD0);
	}

	if (save == 0) {
		if (strcasecmp (ext, ".img") == 0) {
			if (pfdc_is_raw (fname)) {
				return (PFDC_FORMAT_RAW);
			}
		}
	}

	return (PFDC_FORMAT_PFDC);
}

pfdc_img_t *pfdc_img_load_fp (FILE *fp, unsigned type)
{
	pfdc_img_t *img;

	img = NULL;

	switch (type) {
	case PFDC_FORMAT_PFDC0:
	case PFDC_FORMAT_PFDC1:
	case PFDC_FORMAT_PFDC:
		img = pfdc_load_pfdc (fp);
		break;

	case PFDC_FORMAT_ANA:
		img = pfdc_load_anadisk (fp);
		break;

	case PFDC_FORMAT_DC42:
		img = pfdc_load_dc42 (fp);
		break;

	case PFDC_FORMAT_IMD:
		img = pfdc_load_imd (fp);
		break;

	case PFDC_FORMAT_RAW:
		img = pfdc_load_raw (fp);
		break;

	case PFDC_FORMAT_TD0:
		img = pfdc_load_td0 (fp);
		break;
	}

	return (img);
}

pfdc_img_t *pfdc_img_load (const char *fname, unsigned type)
{
	FILE       *fp;
	pfdc_img_t *img;

	type = pfdc_get_type (type, fname, 0);

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (NULL);
	}

	img = pfdc_img_load_fp (fp, type);

	fclose (fp);

	return (img);
}

int pfdc_img_save_fp (FILE *fp, const pfdc_img_t *img, unsigned type)
{
	switch (type) {
	case PFDC_FORMAT_PFDC0:
		return (pfdc_save_pfdc (fp, img, 0));

	case PFDC_FORMAT_PFDC1:
		return (pfdc_save_pfdc (fp, img, 1));

	case PFDC_FORMAT_PFDC:
		return (pfdc_save_pfdc (fp, img, 2));

	case PFDC_FORMAT_ANA:
		return (pfdc_save_anadisk (fp, img));

	case PFDC_FORMAT_DC42:
		return (pfdc_save_dc42 (fp, img));

	case PFDC_FORMAT_IMD:
		return (pfdc_save_imd (fp, img));

	case PFDC_FORMAT_RAW:
		return (pfdc_save_raw (fp, img));

	case PFDC_FORMAT_TD0:
		return (pfdc_save_td0 (fp, img));
	}

	return (1);
}

int pfdc_img_save (const char *fname, const pfdc_img_t *img, unsigned type)
{
	int  r;
	FILE *fp;

	type = pfdc_get_type (type, fname, 1);

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = pfdc_img_save_fp (fp, img, type);

	fclose (fp);

	return (r);
}
