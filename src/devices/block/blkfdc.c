/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/block/blkfdc.c                                   *
 * Created:     2010-08-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
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


#include "blkfdc.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "pfdc.h"
#include "pfdc-img-ana.h"
#include "pfdc-img-imd.h"
#include "pfdc-img-pfdc.h"
#include "pfdc-img-raw.h"
#include "pfdc-img-td0.h"


#define BLKFDC_FORMAT_NONE    0
#define BLKFDC_FORMAT_PFDC    1
#define BLKFDC_FORMAT_ANADISK 2
#define BLKFDC_FORMAT_IMD     3
#define BLKFDC_FORMAT_TD0     4


unsigned dsk_fdc_read_chs (disk_fdc_t *fdc, void *buf, unsigned *cnt,
	unsigned c, unsigned h, unsigned s, int phy)
{
	unsigned   ret;
	pfdc_sct_t *sct, *alt;

	if (fdc->img == NULL) {
		*cnt = 0;
		return (PCE_BLK_FDC_NO_ID);
	}

	ret = 0;

	sct = pfdc_img_get_sector (fdc->img, c, h, s, phy);

	if (sct == NULL) {
		*cnt = 0;
		return (PCE_BLK_FDC_NO_ID);
	}

	alt = pfdc_sct_get_alternate (sct, sct->cur_alt);

	if (alt == NULL) {
		sct->cur_alt = 0;
		alt = sct;
	}

	if (*cnt > alt->n) {
		*cnt = alt->n;
		ret |= PCE_BLK_FDC_DATALEN;
	}

	if (*cnt > 0) {
		memcpy (buf, alt->data, *cnt);
	}

	if (alt->flags & PFDC_FLAG_CRC_ID) {
		ret |= PCE_BLK_FDC_CRC_ID;
	}

	if (alt->flags & PFDC_FLAG_CRC_DATA) {
		ret |= PCE_BLK_FDC_CRC_DATA;
	}

	if (sct->next != NULL) {
		sct->cur_alt += 1;
	}

	return (ret);
}

unsigned dsk_fdc_write_chs (disk_fdc_t *fdc, const void *buf, unsigned *cnt,
	unsigned c, unsigned h, unsigned s, int phy)
{
	unsigned   ret;
	pfdc_sct_t *sct;

	if (fdc->img == NULL) {
		*cnt = 0;
		return (PCE_BLK_FDC_NO_ID);
	}

	if (fdc->dsk.readonly) {
		return (PCE_BLK_FDC_WPROT);
	}

	ret = 0;

	sct = pfdc_img_get_sector (fdc->img, c, h, s, phy);

	if (sct == NULL) {
		*cnt = 0;
		return (PCE_BLK_FDC_NO_ID);
	}

	fdc->dirty = 1;

	if (*cnt > sct->n) {
		*cnt = sct->n;
		ret |= PCE_BLK_FDC_DATALEN;
	}

	if (*cnt > 0) {
		memcpy (sct->data, buf, *cnt);
	}

	sct->flags &= ~PFDC_FLAG_CRC_DATA;

	if (sct->next != NULL) {
		pfdc_sct_del (sct->next);

		sct->next = NULL;
		sct->cur_alt = 0;
	}

	return (ret);
}

int dsk_fdc_erase_track (disk_fdc_t *fdc, unsigned c, unsigned h)
{
	pfdc_trk_t *trk;

	if (fdc->img == NULL) {
		return (1);
	}

	if (fdc->dsk.readonly) {
		return (1);
	}

	trk = pfdc_img_get_track (fdc->img, c, h, 0);

	if (trk == NULL) {
		return (0);
	}

	fdc->dirty = 1;

	pfdc_trk_free (trk);

	return (0);
}

int dsk_fdc_format_sector (disk_fdc_t *fdc,
	unsigned pc, unsigned ph, unsigned c, unsigned h, unsigned s,
	unsigned cnt, unsigned fill)
{
	pfdc_trk_t *trk;
	pfdc_sct_t *sct;

	if (fdc->img == NULL) {
		return (1);
	}

	if (fdc->dsk.readonly) {
		return (1);
	}

	trk = pfdc_img_get_track (fdc->img, pc, ph, 1);

	if (trk == NULL) {
		return (1);
	}

	fdc->dirty = 1;

	sct = pfdc_sct_new (c, h, s, cnt);

	if (sct == NULL) {
		return (1);
	}

	pfdc_sct_fill (sct, fill);

	if (pfdc_trk_add_sector (trk, sct)) {
		pfdc_sct_del (sct);
		return (1);
	}

	return (0);
}

int dsk_fdc_read_id (disk_fdc_t *fdc,
	unsigned pc, unsigned ph, unsigned ps,
	unsigned *c, unsigned *h, unsigned *s, unsigned *cnt)
{
	pfdc_sct_t *sct;

	if (fdc->img == NULL) {
		return (1);
	}

	sct = pfdc_img_get_sector (fdc->img, pc, ph, ps, 1);

	if (sct == NULL) {
		return (1);
	}

	*c = sct->c;
	*h = sct->h;
	*s = sct->s;
	*cnt = sct->n;

	return (0);
}


static
int dsk_fdc_read (disk_t *dsk, void *buf, uint32_t i, uint32_t n)
{
	disk_fdc_t    *fdc;
	unsigned      c, h, s;
	unsigned      cnt;
	unsigned char *tmp;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	fdc = dsk->ext;

	s = (i % dsk->s) + 1;
	h = (i / dsk->s) % dsk->h;
	c = i / (dsk->h * dsk->s);

	tmp = buf;

	while (n > 0) {
		cnt = 512;

		if (dsk_fdc_read_chs (fdc, tmp, &cnt, c, h, s, 0)) {
			return (1);
		}

		if (cnt != 512) {
			return (1);
		}

		s += 1;

		if (s > dsk->s) {
			s = 1;
			h += 1;

			if (h >= dsk->h) {
				h = 0;
				c += 1;
			}
		}

		n -= 1;
		tmp += 512;
	}

	return (0);
}

static
int dsk_fdc_write (disk_t *dsk, const void *buf, uint32_t i, uint32_t n)
{
	disk_fdc_t          *fdc;
	unsigned            c, h, s;
	unsigned            cnt;
	const unsigned char *tmp;

	if ((i + n) > dsk->blocks) {
		return (1);
	}

	if (dsk->readonly) {
		return (1);
	}

	fdc = dsk->ext;

	s = (i % dsk->s) + 1;
	h = (i / dsk->s) % dsk->h;
	c = i / (dsk->h * dsk->s);

	tmp = buf;

	while (n > 0) {
		cnt = 512;

		if (dsk_fdc_write_chs (fdc, tmp, &cnt, c, h, s, 0)) {
			return (1);
		}

		if (cnt != 512) {
			return (1);
		}

		s += 1;

		if (s > dsk->s) {
			s = 1;
			h += 1;

			if (h >= dsk->h) {
				h = 0;
				c += 1;
			}
		}

		n -= 1;
		tmp += 512;
	}

	return (0);
}


static
int fdc_load (disk_fdc_t *fdc, unsigned type)
{
	fdc->type = type;

	if (type == BLKFDC_FORMAT_PFDC) {
		fdc->img = pfdc_load_pfdc (fdc->fp);
	}
	else if (type == BLKFDC_FORMAT_ANADISK) {
		fdc->img = pfdc_load_anadisk (fdc->fp);
	}
	else if (type == BLKFDC_FORMAT_IMD) {
		fdc->img = pfdc_load_imd (fdc->fp);
	}
	else if (type == BLKFDC_FORMAT_TD0) {
		fdc->img = pfdc_load_td0 (fdc->fp);
	}
	else {
		fdc->img = NULL;
	}

	return (fdc->img == NULL);
}

static
int fdc_save (disk_fdc_t *fdc)
{
	if (dsk_get_readonly (&fdc->dsk)) {
		return (1);
	}

	if (fdc->img == NULL) {
		return (1);
	}

	if (fdc->fname != NULL) {
		fclose (fdc->fp);
		fdc->fp = fopen (fdc->fname, "wb");

		if (fdc->fp == NULL) {
			return (1);
		}
	}

	if (fdc->type == BLKFDC_FORMAT_PFDC) {
		return (pfdc_save_pfdc (fdc->fp, fdc->img, 2));
	}
	else if (fdc->type == BLKFDC_FORMAT_ANADISK) {
		return (pfdc_save_anadisk (fdc->fp, fdc->img));
	}
	else if (fdc->type == BLKFDC_FORMAT_IMD) {
		return (pfdc_save_imd (fdc->fp, fdc->img));
	}
	else if (fdc->type == BLKFDC_FORMAT_TD0) {
		return (pfdc_save_td0 (fdc->fp, fdc->img));
	}

	return (1);
}

static
int fdc_guess_geometry (disk_fdc_t *fdc)
{
	unsigned   c, h, s, t;
	unsigned   cyl_cnt, trk_cnt, sct_cnt;
	pfdc_img_t *img;
	pfdc_cyl_t *cyl;
	pfdc_trk_t *trk;

	img = fdc->img;

	cyl_cnt = img->cyl_cnt;
	trk_cnt = 0;
	sct_cnt = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		trk_cnt += cyl->trk_cnt;

		for (t = 0; t < cyl->trk_cnt; t++) {
			trk = cyl->trk[t];

			sct_cnt += trk->sct_cnt;
		}
	}

	if ((cyl_cnt == 0) || (trk_cnt == 0) || (sct_cnt == 0)) {
		return (1);
	}

	c = cyl_cnt;
	h = (trk_cnt + (trk_cnt / cyl_cnt / 2)) / cyl_cnt;
	s = (sct_cnt + (sct_cnt / trk_cnt / 2)) / trk_cnt;

	if (dsk_set_geometry (&fdc->dsk, 0, c, h, s)) {
		return (1);
	}

	return (0);
}

static
int dsk_fdc_set_msg (disk_t *dsk, const char *msg, const char *val)
{
	disk_fdc_t *fdc;

	fdc = dsk->ext;

	if (strcmp (msg, "commit") == 0) {
		if (fdc_save (fdc)) {
			return (1);
		}

		fdc->dirty = 0;

		return (0);
	}

	return (1);
}

static
void dsk_fdc_del (disk_t *dsk)
{
	disk_fdc_t *fdc;

	fdc = dsk->ext;

	if (fdc->dirty) {
		fprintf (stderr, "disk %u: writing back fdc image\n",
			dsk->drive
		);

		if (fdc_save (fdc)) {
			fprintf (stderr, "writing back failed\n");
		}
	}

	if (fdc->img != NULL) {
		pfdc_img_del (fdc->img);
	}

	free (fdc->fname);

	if (fdc->fp != NULL) {
		fclose (fdc->fp);
	}

	free (fdc);
}

static
disk_t *dsk_fdc_open_fp (FILE *fp, unsigned type,
	unsigned c, unsigned h, unsigned s, int ro)
{
	disk_fdc_t *fdc;

	fdc = malloc (sizeof (disk_fdc_t));

	if (fdc == NULL) {
		return (NULL);
	}

	dsk_init (&fdc->dsk, fdc, 0, c, h, s);

	dsk_set_type (&fdc->dsk, PCE_DISK_FDC);

	dsk_set_readonly (&fdc->dsk, ro);

	fdc->dsk.del = dsk_fdc_del;
	fdc->dsk.read = dsk_fdc_read;
	fdc->dsk.write = dsk_fdc_write;
	fdc->dsk.set_msg = dsk_fdc_set_msg;

	fdc->img = NULL;

	fdc->dirty = 0;

	fdc->type = BLKFDC_FORMAT_NONE;
	fdc->fname = NULL;
	fdc->fp = fp;

	if (fdc_load (fdc, type)) {
		fdc->fp = NULL;

		dsk_fdc_del (&fdc->dsk);

		return (NULL);
	}

	if ((c == 0) || (h == 0) || (s == 0)) {
		fdc_guess_geometry (fdc);
	}

	return (&fdc->dsk);
}

static
disk_t *dsk_fdc_open (const char *fname, unsigned type,
	unsigned c, unsigned h, unsigned s, unsigned ro)
{
	unsigned   n;
	disk_t     *dsk;
	disk_fdc_t *fdc;
	FILE       *fp;

	if (ro) {
		fp = fopen (fname, "rb");
	}
	else {
		fp = fopen (fname, "r+b");
	}

	if (fp == NULL) {
		return (NULL);
	}

	dsk = dsk_fdc_open_fp (fp, type, c, h, s, ro);

	if (dsk == NULL) {
		fclose (fp);
		return (NULL);
	}

	fdc = dsk->ext;

	n = strlen (fname);

	fdc->fname = malloc (n + 1);

	if (fdc->fname != NULL) {
		strcpy (fdc->fname, fname);
	}

	return (dsk);
}

disk_t *dsk_fdc_open_pfdc_fp (FILE *fp,
	unsigned c, unsigned h, unsigned s, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_PFDC, c, h, s, ro));
}

disk_t *dsk_fdc_open_pfdc (const char *fname,
	unsigned c, unsigned h, unsigned s, unsigned ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_PFDC, c, h, s, ro));
}

disk_t *dsk_fdc_open_anadisk_fp (FILE *fp,
	unsigned c, unsigned h, unsigned s, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_ANADISK, c, h, s, ro));
}

disk_t *dsk_fdc_open_anadisk (const char *fname,
	unsigned c, unsigned h, unsigned s, unsigned ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_ANADISK, c, h, s, ro));
}

disk_t *dsk_fdc_open_imd_fp (FILE *fp,
	unsigned c, unsigned h, unsigned s, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_IMD, c, h, s, ro));
}

disk_t *dsk_fdc_open_imd (const char *fname,
	unsigned c, unsigned h, unsigned s, unsigned ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_IMD, c, h, s, ro));
}

disk_t *dsk_fdc_open_td0_fp (FILE *fp,
	unsigned c, unsigned h, unsigned s, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_TD0, c, h, s, ro));
}

disk_t *dsk_fdc_open_td0 (const char *fname,
	unsigned c, unsigned h, unsigned s, unsigned ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_TD0, c, h, s, ro));
}
