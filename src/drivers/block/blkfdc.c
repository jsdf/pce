/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/blkfdc.c                                   *
 * Created:     2010-08-11 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "blkfdc.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "pfdc.h"
#include "pfdc-img-ana.h"
#include "pfdc-img-dc42.h"
#include "pfdc-img-imd.h"
#include "pfdc-img-pfdc.h"
#include "pfdc-img-raw.h"
#include "pfdc-img-td0.h"


#define BLKFDC_FORMAT_NONE    0
#define BLKFDC_FORMAT_PFDC    1
#define BLKFDC_FORMAT_ANADISK 2
#define BLKFDC_FORMAT_DC42    3
#define BLKFDC_FORMAT_IMD     4
#define BLKFDC_FORMAT_TD0     5


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

unsigned dsk_fdc_read_tags (disk_fdc_t *fdc, void *buf, unsigned cnt,
	unsigned c, unsigned h, unsigned s, int phy)
{
	pfdc_sct_t *sct, *alt;

	memset (buf, 0, cnt);

	if (fdc->img == NULL) {
		return (0);
	}

	sct = pfdc_img_get_sector (fdc->img, c, h, s, phy);

	if (sct == NULL) {
		return (0);
	}

	alt = pfdc_sct_get_alternate (sct, sct->cur_alt);

	if (alt == NULL) {
		sct->cur_alt = 0;
		alt = sct;
	}

	cnt = pfdc_sct_get_tags (alt, buf, cnt);

	return (cnt);
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

unsigned dsk_fdc_write_tags (disk_fdc_t *fdc, const void *buf, unsigned cnt,
	unsigned c, unsigned h, unsigned s, int phy)
{
	pfdc_sct_t *sct;

	if (fdc->img == NULL) {
		return (0);
	}

	if (fdc->dsk.readonly) {
		return (0);
	}

	sct = pfdc_img_get_sector (fdc->img, c, h, s, phy);

	if (sct == NULL) {
		return (0);
	}

	fdc->dirty = 1;

	cnt = pfdc_sct_set_tags (sct, buf, cnt);

	return (cnt);
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

	fdc->dsk.blocks -= trk->sct_cnt;

	pfdc_trk_free (trk);

	return (0);
}

int dsk_fdc_erase_disk (disk_fdc_t *fdc)
{
	if (fdc->img == NULL) {
		return (1);
	}

	if (fdc->dsk.readonly) {
		return (1);
	}

	pfdc_img_erase (fdc->img);

	fdc->dsk.blocks = 0;

	fdc->dirty = 1;

	return (0);
}

void dsk_fdc_set_encoding (disk_fdc_t *fdc, unsigned enc, unsigned long rate)
{
	fdc->encoding = enc;
	fdc->rate = rate;
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

	pfdc_sct_set_encoding (sct, fdc->encoding, fdc->rate);

	fdc->dsk.blocks += 1;

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

	fdc = dsk->ext;

	tmp = buf;

	while (n > 0) {
		if (pfdc_img_map_sector (fdc->img, i, &c, &h, &s)) {
			return (1);
		}

		cnt = 512;

		if (dsk_fdc_read_chs (fdc, tmp, &cnt, c, h, s, 1)) {
			return (1);
		}

		if (cnt != 512) {
			return (1);
		}

		tmp += 512;

		i += 1;
		n -= 1;
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

	if (dsk->readonly) {
		return (1);
	}

	fdc = dsk->ext;

	tmp = buf;

	while (n > 0) {
		if (pfdc_img_map_sector (fdc->img, i, &c, &h, &s)) {
			return (1);
		}

		cnt = 512;

		if (dsk_fdc_write_chs (fdc, tmp, &cnt, c, h, s, 1)) {
			return (1);
		}

		if (cnt != 512) {
			return (1);
		}

		tmp += 512;

		i += 1;
		n -= 1;
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
	else if (type == BLKFDC_FORMAT_DC42) {
		fdc->img = pfdc_load_dc42 (fdc->fp);
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
	else if (fdc->type == BLKFDC_FORMAT_DC42) {
		return (pfdc_save_dc42 (fdc->fp, fdc->img));
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
int fdc_set_geometry (disk_fdc_t *fdc)
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

	if (dsk_set_geometry (&fdc->dsk, sct_cnt, c, h, s)) {
		return (1);
	}

	dsk_set_visible_chs (&fdc->dsk, c, h, s);

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
disk_t *dsk_fdc_open_fp (FILE *fp, unsigned type, int ro)
{
	disk_fdc_t *fdc;

	fdc = malloc (sizeof (disk_fdc_t));

	if (fdc == NULL) {
		return (NULL);
	}

	dsk_init (&fdc->dsk, fdc, 0, 0, 0, 0);

	dsk_set_type (&fdc->dsk, PCE_DISK_FDC);

	dsk_set_readonly (&fdc->dsk, ro);

	fdc->dsk.del = dsk_fdc_del;
	fdc->dsk.read = dsk_fdc_read;
	fdc->dsk.write = dsk_fdc_write;
	fdc->dsk.set_msg = dsk_fdc_set_msg;

	fdc->img = NULL;

	fdc->dirty = 0;

	fdc->encoding = PFDC_ENC_MFM;
	fdc->rate = 250000;

	fdc->type = BLKFDC_FORMAT_NONE;
	fdc->fname = NULL;
	fdc->fp = fp;

	if (fdc_load (fdc, type)) {
		fdc->fp = NULL;

		dsk_fdc_del (&fdc->dsk);

		return (NULL);
	}

	fdc_set_geometry (fdc);

	return (&fdc->dsk);
}

static
disk_t *dsk_fdc_open (const char *fname, unsigned type, int ro)
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

	dsk = dsk_fdc_open_fp (fp, type, ro);

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

	dsk_set_fname (dsk, fname);

	return (dsk);
}

disk_t *dsk_fdc_open_pfdc_fp (FILE *fp, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_PFDC, ro));
}

disk_t *dsk_fdc_open_pfdc (const char *fname, int ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_PFDC, ro));
}

disk_t *dsk_fdc_open_anadisk_fp (FILE *fp, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_ANADISK, ro));
}

disk_t *dsk_fdc_open_anadisk (const char *fname, int ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_ANADISK, ro));
}

disk_t *dsk_fdc_open_dc42_fp (FILE *fp, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_DC42, ro));
}

disk_t *dsk_fdc_open_dc42 (const char *fname, int ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_DC42, ro));
}

disk_t *dsk_fdc_open_imd_fp (FILE *fp, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_IMD, ro));
}

disk_t *dsk_fdc_open_imd (const char *fname, int ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_IMD, ro));
}

disk_t *dsk_fdc_open_td0_fp (FILE *fp, int ro)
{
	return (dsk_fdc_open_fp (fp, BLKFDC_FORMAT_TD0, ro));
}

disk_t *dsk_fdc_open_td0 (const char *fname, int ro)
{
	return (dsk_fdc_open (fname, BLKFDC_FORMAT_TD0, ro));
}

int dsk_fdc_probe_pfdc_fp (FILE *fp)
{
	return (pfdc_probe_pfdc_fp (fp));
}

int dsk_fdc_probe_pfdc (const char *fname)
{
	return (pfdc_probe_pfdc (fname));
}

int dsk_fdc_probe_dc42_fp (FILE *fp)
{
	return (pfdc_probe_dc42_fp (fp));
}

int dsk_fdc_probe_dc42 (const char *fname)
{
	return (pfdc_probe_dc42 (fname));
}

int dsk_fdc_probe_imd_fp (FILE *fp)
{
	return (pfdc_probe_imd_fp (fp));
}

int dsk_fdc_probe_imd (const char *fname)
{
	return (pfdc_probe_imd (fname));
}

int dsk_fdc_probe_td0_fp (FILE *fp)
{
	return (pfdc_probe_td0_fp (fp));
}

int dsk_fdc_probe_td0 (const char *fname)
{
	return (pfdc_probe_td0 (fname));
}
