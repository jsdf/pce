/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-img-raw.c                              *
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
#include "pfdc-img-raw.h"


static pfdc_geometry_t disk_sizes[] = {
	{ 160 * 1024UL, 40, 1, 8, 512, PFDC_ENC_MFM_DD },
	{ 180 * 1024UL, 40, 1, 9, 512, PFDC_ENC_MFM_DD },
	{ 320 * 1024UL, 40, 2, 8, 512, PFDC_ENC_MFM_DD },
	{ 360 * 1024UL, 40, 2, 9, 512, PFDC_ENC_MFM_DD },
	{ 640 * 1024UL, 80, 2, 8, 512, PFDC_ENC_MFM_DD },
	{ 720 * 1024UL, 80, 2, 9, 512, PFDC_ENC_MFM_DD },
	{ 1200 * 1024UL, 80, 2, 15, 512, PFDC_ENC_MFM_HD },
	{ 1440 * 1024UL, 80, 2, 18, 512, PFDC_ENC_MFM_HD },
	{ 2880 * 1024UL, 80, 2, 36, 512, PFDC_ENC_MFM_HD },
	{ 1232 * 1024UL, 77, 2, 8, 1024, PFDC_ENC_MFM_HD },
	{ 2002 * 128UL, 77, 1, 26, 128, PFDC_ENC_FM_DD },
	{ 4004 * 128UL, 77, 2, 26, 128, PFDC_ENC_FM_DD },
	{ 0, 0, 0, 0, 0, 0 }
};


const pfdc_geometry_t *pfdc_get_geometry_from_size (unsigned long size, unsigned long mask)
{
	unsigned i;

	mask = ~mask;

	i = 0;
	while (disk_sizes[i].size != 0) {
		if ((disk_sizes[i].size & mask) == (size & mask)) {
			return (&disk_sizes[i]);
		}

		i += 1;
	}

	return (NULL);
}

static
int raw_get_file_size (FILE *fp, unsigned long *size)
{
	unsigned long pos;

	pos = ftell (fp);

	if (fseek (fp, 0, SEEK_END) != 0) {
		return (1);
	}

	*size = ftell (fp);

	if (fseek (fp, pos, SEEK_SET) != 0) {
		return (1);
	}

	return (0);
}

static
int raw_load_fp (FILE *fp, pfdc_img_t *img)
{
	unsigned              c, h, s;
	unsigned long         size;
	pfdc_trk_t            *trk;
	pfdc_sct_t            *sct;
	const pfdc_geometry_t *geo;

	if (raw_get_file_size (fp, &size)) {
		return (1);
	}

	geo = pfdc_get_geometry_from_size (size, 0);

	if (geo == NULL) {
		return (1);
	}

	for (c = 0; c < geo->c; c++) {
		for (h = 0; h < geo->h; h++) {
			trk = pfdc_img_get_track (img, c, h, 1);

			if (trk == NULL) {
				return (1);
			}

			for (s = 0; s < geo->s; s++) {
				sct = pfdc_sct_new (c, h, s + 1, geo->ssize);

				if (sct == NULL) {
					return (1);
				}

				pfdc_sct_set_encoding (sct, geo->encoding);

				if (pfdc_trk_add_sector (trk, sct)) {
					pfdc_sct_del (sct);
					return (1);
				}

				if (fread (sct->data, 1, geo->ssize, fp) != geo->ssize) {
					return (1);
				}
			}
		}
	}

	return (0);
}

pfdc_img_t *pfdc_load_raw (FILE *fp)
{
	pfdc_img_t *img;

	img = pfdc_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (raw_load_fp (fp, img)) {
		pfdc_img_del (img);
		return (NULL);
	}

	return (img);
}


static
const pfdc_sct_t *raw_get_next_sector (const pfdc_trk_t *trk, unsigned *idx)
{
	unsigned   i, j;
	pfdc_sct_t *sct;

	sct = NULL;

	j = 0xffff;

	for (i = 0; i < trk->sct_cnt; i++) {
		if (trk->sct[i]->s < *idx) {
			continue;
		}

		if (trk->sct[i]->s < j) {
			sct = trk->sct[i];
			j = sct->s;
		}
	}

	if (sct == NULL) {
		return (NULL);
	}

	*idx = sct->s + 1;

	return (sct);
}

int pfdc_save_raw (FILE *fp, const pfdc_img_t *img)
{
	unsigned         c, h, s;
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;
	const pfdc_sct_t *sct;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			s = 0;

			sct = raw_get_next_sector (trk, &s);

			while (sct != NULL) {
				if (fwrite (sct->data, 1, sct->n, fp) != sct->n) {
					return (1);
				}

				sct = raw_get_next_sector (trk, &s);
			}
		}
	}

	fflush (fp);

	return (0);
}
