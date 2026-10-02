/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-img-mfm.c                                *
 * Created:     2023-12-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2023-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <config.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/endian.h>

#include "pri.h"
#include "pri-img.h"
#include "pri-img-mfm.h"


typedef struct {
	FILE          *fp;
	pri_img_t     *img;

	unsigned long hofs;
	unsigned long clock;

	unsigned      cn;
	unsigned      hn;
} mfm_load_t;


typedef struct {
	FILE            *fp;
	const pri_img_t *img;

	unsigned long   hofs;
	unsigned long   dofs;

	unsigned        cn;
	unsigned        hn;
} mfm_save_t;


static const char mfm_magic[] = "HXCMFM\x00";


static
int mfm_load_header (mfm_load_t *mfm)
{
	unsigned char buf[20];

	if (pri_read (mfm->fp, buf, 19)) {
		return (1);
	}

	if (memcmp (buf, mfm_magic, 7) != 0) {
		fprintf (stderr, "mfm: bad magic\n");
		return (1);
	}

	mfm->cn = get_uint16_le (buf, 7);
	mfm->hn = get_uint8 (buf, 9);
	mfm->hofs = get_uint32_le (buf, 15);
	mfm->clock = get_uint16_le (buf, 12);
	mfm->clock = (mfm->clock > 0) ? (2000UL * mfm->clock) : 500000UL;

	return (0);
}

static
int mfm_load_track (mfm_load_t *mfm, unsigned c, unsigned h)
{
	unsigned      c2, h2;
	unsigned long dofs, size;
	unsigned char buf[16];
	pri_trk_t     *trk;

	if (pri_read_ofs (mfm->fp, mfm->hofs, buf, 11)) {
		return (1);
	}

	c2 = get_uint16_le (buf, 0);
	h2 = get_uint8 (buf, 2);
	size = get_uint32_le (buf, 3);
	dofs = get_uint32_le (buf, 7);

	if ((c2 != c) || (h2 != h)) {
		fprintf (stderr, "mfm: track address mismatch (%u/%u %u/%u)\n",
			c, h, c2, h2
		);
	}

	if ((trk = pri_img_get_track (mfm->img, c, h, 1)) == NULL) {
		return (1);
	}

	pri_trk_set_clock (trk, mfm->clock);

	if (pri_trk_set_size (trk, 8 * size)) {
		return (1);
	}

	if (size > 0) {
		if (pri_read_ofs (mfm->fp, dofs, trk->data, size)) {
			return (1);
		}
	}

	mfm->hofs += 1;

	return (0);
}

static
int mfm_load_img (mfm_load_t *mfm)
{
	unsigned c, h;

	if (mfm_load_header (mfm)) {
		return (1);
	}

	for (c = 0; c < mfm->cn; c++) {
		for (h = 0; h < mfm->hn; h++) {
			if (mfm_load_track (mfm, c, h)) {
				return (1);
			}
		}
	}

	return (0);
}

pri_img_t *pri_load_mfm (FILE *fp)
{
	mfm_load_t mfm;

	mfm.fp = fp;
	mfm.img = NULL;

	if ((mfm.img = pri_img_new()) == NULL) {
		return (NULL);
	}

	if (mfm_load_img (&mfm)) {
		pri_img_del (mfm.img);
		return (NULL);
	}

	return (mfm.img);
}


static
int mfm_save_header (mfm_save_t *mfm)
{
	unsigned        c, h;
	unsigned long   clock;
	unsigned char   buf[20];
	const pri_cyl_t *cyl;
	const pri_trk_t *trk;

	mfm->cn = 0;
	mfm->hn = 0;

	clock = 500000;

	for (c = 0; c < mfm->img->cyl_cnt; c++) {
		if ((cyl = mfm->img->cyl[c]) == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			if ((trk = cyl->trk[h]) == NULL) {
				continue;
			}

			mfm->cn = c + 1;

			if (h >= mfm->hn) {
				mfm->hn = h + 1;
			}

			if (trk->clock > clock) {
				clock = trk->clock;
			}
		}
	}

	memcpy (buf, mfm_magic, 7);
	set_uint16_le (buf, 7, mfm->cn);
	set_uint8 (buf, 9, mfm->hn);
	set_uint16_le (buf, 10, 0);
	set_uint16_le (buf, 12, clock / 2000);
	set_uint8 (buf, 14, 7);
	set_uint32_le (buf, 15, 19);

	if (pri_write (mfm->fp, buf, 19)) {
		return (1);
	}

	mfm->hofs = 19;
	mfm->dofs = mfm->hofs + (11UL * mfm->cn * mfm->hn);

	return (0);
}

int mfm_save_track (mfm_save_t *mfm, const pri_trk_t *trk, unsigned c, unsigned h)
{
	unsigned long size;
	unsigned char buf[16];

	size = (trk != NULL) ? ((trk->size + 7) / 8) : 0;

	set_uint16_le (buf, 0, c);
	set_uint8 (buf, 2, h);
	set_uint32_le (buf, 3, size);
	set_uint32_le (buf, 7, mfm->dofs);

	if (pri_write_ofs (mfm->fp, mfm->hofs, buf, 11)) {
		return (1);
	}

	mfm->hofs += 11;

	if (size > 0) {
		if (pri_write_ofs (mfm->fp, mfm->dofs, trk->data, size)) {
			return (1);
		}

		mfm->dofs += size;
	}

	return (0);
}

int mfm_save_img (mfm_save_t *mfm)
{
	unsigned        c, h;
	const pri_trk_t *trk;

	if (mfm_save_header (mfm)) {
		return (1);
	}

	for (c = 0; c < mfm->cn; c++) {
		for (h = 0; h < mfm->hn; h++) {
			trk = pri_img_get_track_const (mfm->img, c, h);

			if (mfm_save_track (mfm, trk, c, h)) {
				return (1);
			}
		}
	}

	return (0);
}

int pri_save_mfm (FILE *fp, const pri_img_t *img)
{
	mfm_save_t mfm;

	mfm.fp = fp;
	mfm.img = img;

	if (mfm_save_img (&mfm)) {
		return (1);
	}

	return (0);
}


int pri_probe_mfm_fp (FILE *fp)
{
	unsigned char buf[8];

	if (pri_read_ofs (fp, 0, buf, 8)) {
		return (0);
	}

	if (memcmp (buf, mfm_magic, 7) != 0) {
		return (0);
	}

	return (1);
}

int pri_probe_mfm (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pri_probe_mfm_fp (fp);

	fclose (fp);

	return (r);
}
