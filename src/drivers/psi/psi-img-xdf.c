/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-xdf.c                                *
 * Created:     2012-06-24 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "psi.h"
#include "psi-io.h"
#include "psi-img-xdf.h"


typedef struct {
	unsigned char h;
	unsigned char s;
	unsigned char n;
	unsigned long ofs;
} xdf_map_t;

typedef struct {
	unsigned      cyl_cnt;

	unsigned long track_size;

	unsigned      map_cnt0;
	xdf_map_t     *map0;

	unsigned      map_cnt1;
	xdf_map_t     *map1;
} xdf_format_t;


static xdf_map_t xdf_1840_map0[] = {
	{ 0,   1, 2, 12 * 512 },
	{ 0, 138, 2,  9 * 512 },
	{ 0, 129, 2,  0 * 512 },
	{ 0, 139, 2, 10 * 512 },
	{ 0, 130, 2,  1 * 512 },
	{ 0,   2, 2, 13 * 512 },
	{ 0, 131, 2,  2 * 512 },
	{ 0,   3, 2, 14 * 512 },
	{ 0, 132, 2,  3 * 512 },
	{ 0,   4, 2, 15 * 512 },
	{ 0, 133, 2,  4 * 512 },
	{ 0,   5, 2, 16 * 512 },
	{ 0, 134, 2,  5 * 512 },
	{ 0,   6, 2, 17 * 512 },
	{ 0, 135, 2,  6 * 512 },
	{ 0,   7, 2, 18 * 512 },
	{ 0, 136, 2,  7 * 512 },
	{ 0,   8, 2, 19 * 512 },
	{ 0, 137, 2,  8 * 512 },

	{ 1, 144, 2, 42 * 512 },
	{ 1, 135, 2, 28 * 512 },
	{ 1, 145, 2, 43 * 512 },
	{ 1, 136, 2, 29 * 512 },
	{ 1, 146, 2, 44 * 512 },
	{ 1, 137, 2, 30 * 512 },
	{ 1, 147, 2, 45 * 512 },
	{ 1, 138, 2, 31 * 512 },
	{ 1, 129, 2, 11 * 512 },
	{ 1, 139, 2, 32 * 512 },
	{ 1, 130, 2, 23 * 512 },
	{ 1, 140, 2, 33 * 512 },
	{ 1, 131, 2, 24 * 512 },
	{ 1, 141, 2, 34 * 512 },
	{ 1, 132, 2, 25 * 512 },
	{ 1, 142, 2, 35 * 512 },
	{ 1, 133, 2, 26 * 512 },
	{ 1, 143, 2, 36 * 512 },
	{ 1, 134, 2, 27 * 512 }
};

static xdf_map_t xdf_1840_map1[] = {
	{ 0, 131, 3,  0 * 512 },
	{ 0, 130, 2, 22 * 512 },
	{ 0, 132, 4,  2 * 512 },
	{ 0, 134, 6, 24 * 512 },

	{ 1, 132, 4, 40 * 512 },
	{ 1, 130, 2, 23 * 512 },
	{ 1, 131, 3, 44 * 512 },
	{ 1, 134, 6,  6 * 512 }
};

static xdf_format_t xdf_1840 = {
	80,
	46 * 512,
	38,
	xdf_1840_map0,
	8,
	xdf_1840_map1
};


static
int xdf_load_sector (FILE *fp, psi_img_t *img, const xdf_map_t *map, unsigned long ofs, unsigned c)
{
	unsigned  size;
	psi_sct_t *sct;

	size = 128U << map->n;

	sct = psi_sct_new (c, map->h, map->s, size);

	if (sct == NULL) {
		return (1);
	}

	psi_sct_set_encoding (sct, PSI_ENC_MFM_HD);

	if (psi_img_add_sector (img, sct, c, map->h)) {
		return (1);
	}

	if (psi_read_ofs (fp, map->ofs + ofs, sct->data, size)) {
		return (1);
	}

	return (0);
}

static
int xdf_load_fp (FILE *fp, psi_img_t *img, const xdf_format_t *fmt)
{
	unsigned      i, j;
	unsigned long ofs;

	for (i = 0; i < fmt->map_cnt0; i++) {
		if (xdf_load_sector (fp, img, &fmt->map0[i], 0, 0)) {
			return (1);
		}
	}

	ofs = fmt->track_size;

	for (j = 1; j < fmt->cyl_cnt; j++) {
		for (i = 0; i < fmt->map_cnt1; i++) {
			if (xdf_load_sector (fp, img, &fmt->map1[i], ofs, j)) {
				return (1);
			}
		}

		ofs += fmt->track_size;
	}


	return (0);
}

psi_img_t *psi_load_xdf (FILE *fp)
{
	psi_img_t *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (xdf_load_fp (fp, img, &xdf_1840)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


static
int xdf_save_sector (FILE *fp, const psi_img_t *img, const xdf_map_t *map, unsigned long ofs, unsigned c)
{
	unsigned        size;
	const psi_sct_t *sct;

	size = 128U << map->n;

	sct = psi_img_get_sector ((psi_img_t *) img, c, map->h, map->s, 0);

	if (sct == NULL) {
		return (1);
	}

	if (sct->n != size) {
		return (1);
	}

	if (psi_write_ofs (fp, map->ofs + ofs, sct->data, size)) {
		return (1);
	}

	return (0);
}

static
int xdf_save_fp (FILE *fp, const psi_img_t *img, const xdf_format_t *fmt)
{
	unsigned      i, j;
	unsigned long ofs;

	for (i = 0; i < fmt->map_cnt0; i++) {
		if (xdf_save_sector (fp, img, &fmt->map0[i], 0, 0)) {
			return (1);
		}
	}

	ofs = fmt->track_size;

	for (j = 1; j < fmt->cyl_cnt; j++) {
		for (i = 0; i < fmt->map_cnt1; i++) {
			if (xdf_save_sector (fp, img, &fmt->map1[i], ofs, j)) {
				return (1);
			}
		}

		ofs += fmt->track_size;
	}


	return (0);
}

int psi_save_xdf (FILE *fp, const psi_img_t *img)
{
	return (xdf_save_fp (fp, img, &xdf_1840));
}
