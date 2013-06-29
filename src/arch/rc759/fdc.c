/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/fdc.c                                         *
 * Created:     2012-07-05 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "fdc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chipset/wd179x.h>

#include <drivers/block/block.h>
#include <drivers/block/blkpsi.h>

#include <drivers/psi/psi.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>
#include <drivers/pri/mfm-ibm.h>

#include <lib/log.h>
#include <lib/string.h>


#ifndef DEBUG_FDC
#define DEBUG_FDC 1
#endif


static
int rc759_read_track (void *ext, wd179x_drive_t *drv)
{
	unsigned long cnt;
	rc759_fdc_t   *fdc;
	pri_img_t     *img;
	pri_trk_t     *trk;

	fdc = ext;

	img = fdc->img[drv->d & 1];

	if (img == NULL) {
		return (1);
	}

	trk = pri_img_get_track (img, drv->c, drv->h, 0);

	if (trk == NULL) {
		return (1);
	}

	cnt = (trk->size + 7) / 8;

	if (cnt > WD179X_TRKBUF_SIZE) {
		return (1);
	}

	memcpy (drv->trkbuf, trk->data, cnt);

	drv->trkbuf_cnt = trk->size;

	return (0);
}

static
int rc759_write_track (void *ext, wd179x_drive_t *drv)
{
	unsigned long cnt;
	rc759_fdc_t   *fdc;
	pri_img_t     *img;
	pri_trk_t     *trk;

	fdc = ext;

	img = fdc->img[drv->d & 1];

	if (img == NULL) {
		return (1);
	}

	fdc->modified[drv->d & 1] = 1;

	trk = pri_img_get_track (img, drv->c, drv->h, 1);

	if (trk == NULL) {
		return (1);
	}

	if (pri_trk_set_size (trk, drv->trkbuf_cnt)) {
		return (1);
	}

	pri_trk_set_clock (trk, 1000000);

	cnt = (trk->size + 7) / 8;

	memcpy (trk->data, drv->trkbuf, cnt);

	return (0);
}

void rc759_fdc_init (rc759_fdc_t *fdc)
{
	unsigned i;

	wd179x_init (&fdc->wd179x);

	wd179x_set_read_track_fct (&fdc->wd179x, fdc, rc759_read_track);
	wd179x_set_write_track_fct (&fdc->wd179x, fdc, rc759_write_track);

	wd179x_set_ready (&fdc->wd179x, 0, 0);
	wd179x_set_ready (&fdc->wd179x, 1, 0);

	fdc->fcr = 0;

	for (i = 0; i < 2; i++) {
		fdc->use_fname[i] = 0;
		fdc->fname[i] = NULL;
		fdc->diskid[i] = 0xffff;
		fdc->img[i] = NULL;
		fdc->modified[i] = 0;
	}
}

void rc759_fdc_free (rc759_fdc_t *fdc)
{
	unsigned i;

	wd179x_free (&fdc->wd179x);

	for (i = 0; i < 2; i++) {
		rc759_fdc_save (fdc, i);

		pri_img_del (fdc->img[i]);

		free (fdc->fname[i]);
	}
}

void rc759_fdc_reset (rc759_fdc_t *fdc)
{
	wd179x_reset (&fdc->wd179x);

	fdc->reserve = 0;
	fdc->fcr = 0;
}

void rc759_fdc_set_disks (rc759_fdc_t *fdc, disks_t *dsks)
{
	fdc->dsks = dsks;
}

void rc759_fdc_set_disk_id (rc759_fdc_t *fdc, unsigned drive, unsigned diskid)
{
	fdc->diskid[drive] = diskid;
}

void rc759_fdc_set_fname (rc759_fdc_t *fdc, unsigned drive, const char *fname)
{
	unsigned n;
	char     *str;

	if (drive >= 2) {
		return;
	}

	free (fdc->fname[drive]);
	fdc->fname[drive] = NULL;
	fdc->use_fname[drive] = 0;

	if (fname == NULL) {
		return;
	}

	n = strlen (fname);

	str = malloc (n + 1);

	if (str == NULL) {
		return;
	}

	memcpy (str, fname, n + 1);

	fdc->fname[drive] = str;
}

int rc759_fdc_insert (rc759_fdc_t *fdc, const char *str)
{
	unsigned i;
	unsigned drv;
	char     buf[16];

	i = 0;
	while ((i < 16) && (str[i] != 0)) {
		if (str[i] == ':') {
			buf[i] = 0;
			break;
		}

		buf[i] = str[i];

		i += 1;
	}

	if ((i >= 16) || (i == 0) || (str[i] == 0)) {
		return (1);
	}

	drv = strtoul (buf, NULL, 0);
	str = str + i + 1;

	if (rc759_fdc_save (fdc, drv)) {
		return (1);
	}

	rc759_fdc_set_fname (fdc, drv, str);

	if (rc759_fdc_load (fdc, drv)) {
		return (1);
	}

	return (0);
}

int rc759_fdc_eject (rc759_fdc_t *fdc, const char *str)
{
	unsigned drv;

	drv = strtoul (str, NULL, 0);

	if (rc759_fdc_save (fdc, drv)) {
		return (1);
	}

	rc759_fdc_set_fname (fdc, drv, NULL);

	if (rc759_fdc_load (fdc, drv)) {
		return (1);
	}

	return (0);
}

unsigned char rc759_fdc_get_reserve (const rc759_fdc_t *fdc)
{
	return (fdc->reserve);
}

void rc759_fdc_set_reserve (rc759_fdc_t *fdc, unsigned char val)
{
	if (val) {
		fdc->reserve = 0;
	}
	else {
		fdc->reserve = 0x80;
	}
}

unsigned char rc759_fdc_get_fcr (const rc759_fdc_t *fdc)
{
	return (fdc->fcr);
}

void rc759_fdc_set_fcr (rc759_fdc_t *fdc, unsigned char val)
{
	if (fdc->fcr == val) {
		return;
	}

#if DEBUG_FDC >= 2
	sim_log_deb ("FDC: FCR = %02X\n", val);
#endif

	fdc->fcr = val;

	wd179x_select_drive (&fdc->wd179x, val & 1);
	wd179x_set_motor (&fdc->wd179x, 0, (val >> 1) & 1);
	wd179x_set_motor (&fdc->wd179x, 1, (val >> 2) & 1);
}

static
pri_img_t *rc759_fdc_load_pri (rc759_fdc_t *fdc, unsigned drive)
{
	pri_img_t *img;

	if (fdc->fname[drive] == NULL) {
		return (NULL);
	}

	img = pri_img_load (fdc->fname[drive], PRI_FORMAT_PBIT);

	return (img);
}

static
psi_img_t *rc759_fdc_load_block (rc759_fdc_t *fdc, unsigned drive, disk_t *dsk)
{
	unsigned      c, h, s;
	unsigned      cn, hn, sn;
	unsigned long lba;
	psi_sct_t     *sct;
	psi_img_t     *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	lba = 0;

	sn = 8;
	hn = 2;
	cn = dsk_get_block_cnt (dsk) / (hn * sn * 2);

	for (c = 0; c < cn; c++) {
		for (h = 0; h < hn; h++) {
			for (s = 0; s < sn; s++) {
				sct = psi_sct_new (c, h, s + 1, 1024);

				if (sct == NULL) {
					psi_img_del (img);
					return (NULL);
				}

				psi_sct_set_encoding (sct, PSI_ENC_MFM_HD);

				psi_img_add_sector (img, sct, c, h);

				if (dsk_read_lba (dsk, sct->data, lba, 2)) {
					psi_img_del (img);
					return (NULL);
				}

				lba += 2;
			}
		}
	}

	return (img);
}

static
pri_img_t *rc759_fdc_load_disk (rc759_fdc_t *fdc, unsigned drive)
{
	disk_t     *dsk;
	disk_psi_t *dskpsi;
	psi_img_t  *img, *del;
	pri_img_t  *ret;

	dsk = dsks_get_disk (fdc->dsks, fdc->diskid[drive]);

	if (dsk == NULL) {
		return (NULL);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		dskpsi = dsk->ext;
		img = dskpsi->img;
		del = NULL;
	}
	else {
		img = rc759_fdc_load_block (fdc, drive, dsk);
		del = img;
	}

	if (img == NULL) {
		return (NULL);
	}

	ret = pri_encode_mfm_hd_360 (img);

	psi_img_del (del);

	return (ret);
}

int rc759_fdc_load (rc759_fdc_t *fdc, unsigned drive)
{
	pri_img_t *img;

	wd179x_set_ready (&fdc->wd179x, drive, 0);

	img = NULL;

	fdc->use_fname[drive] = 0;

	if (fdc->fname[drive] != NULL) {
		img = rc759_fdc_load_pri (fdc, drive);

		if (img != NULL) {
			fdc->use_fname[drive] = 1;
			sim_log_deb ("fdc: loading drive %u (pri)\n", drive);
		}
	}

	if (img == NULL) {
		img = rc759_fdc_load_disk (fdc, drive);

		if (img != NULL) {
			sim_log_deb ("fdc: loading drive %u (disk)\n", drive);
		}
	}

	if (img == NULL) {
		sim_log_deb ("fdc: loading drive %u failed\n", drive);
		return (1);
	}

	pri_img_del (fdc->img[drive]);

	fdc->img[drive] = img;

	fdc->modified[drive] = 0;

	wd179x_set_ready (&fdc->wd179x, drive, 1);

	return (0);
}

static
int rc759_fdc_save_block (rc759_fdc_t *fdc, unsigned drive, disk_t *dsk, psi_img_t *img)
{
	unsigned      c, h, s;
	unsigned      cn, hn, sn;
	unsigned      cnt;
	unsigned long lba;
	unsigned char buf[1024];
	psi_sct_t     *sct;

	lba = 0;

	cn = 77;
	hn = 2;
	sn = 8;

	for (c = 0; c < cn; c++) {
		for (h = 0; h < hn; h++) {
			for (s = 0; s < sn; s++) {
				sct = psi_img_get_sector (img, c, h, s + 1, 0);

				if (sct == NULL) {
					memset (buf, 0, 1024);
				}
				else {
					cnt = 1024;

					if (sct->n < 1024) {
						cnt = sct->n;
						memset (buf + cnt, 0, 1024 - cnt);
					}

					memcpy (buf, sct->data, cnt);
				}

				if (dsk_write_lba (dsk, buf, lba, 2)) {
					return (1);
				}

				lba += 2;
			}
		}
	}

	return (0);
}

static
int rc759_fdc_save_disk (rc759_fdc_t *fdc, unsigned drive)
{
	int        r;
	disk_t     *dsk;
	disk_psi_t *dskpsi;
	psi_img_t  *img;

	dsk = dsks_get_disk (fdc->dsks, fdc->diskid[drive]);

	if (dsk == NULL) {
		return (1);
	}

	img = pri_decode_mfm (fdc->img[drive]);

	if (img == NULL) {
		return (1);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		dskpsi = dsk->ext;
		psi_img_del (dskpsi->img);
		dskpsi->img = img;
		dskpsi->dirty = 1;
	}
	else {
		r = rc759_fdc_save_block (fdc, drive, dsk, img);

		psi_img_del (img);

		if (r) {
			return (1);
		}
	}

	return (0);
}

static
int rc759_fdc_save_pri (rc759_fdc_t *fdc, unsigned drive)
{
	if (fdc->fname[drive] == NULL) {
		return (1);
	}

	if (pri_img_save (fdc->fname[drive], fdc->img[drive], PRI_FORMAT_NONE)) {
		return (1);
	}

	return (0);
}

int rc759_fdc_save (rc759_fdc_t *fdc, unsigned drive)
{
	if (fdc->img[drive] == NULL) {
		return (1);
	}

	if (fdc->modified[drive] == 0) {
		return (0);
	}

	sim_log_deb ("fdc: saving drive %u\n", drive);

	if (fdc->use_fname[drive]) {
		if (rc759_fdc_save_pri (fdc, drive)) {
			sim_log_deb ("fdc: saving drive %u failed (pri)\n",
				drive
			);
			return (1);
		}
	}
	else {
		if (rc759_fdc_save_disk (fdc, drive)) {
			sim_log_deb ("fdc: saving drive %u failed (disk)\n",
				drive
			);
			return (1);
		}
	}

	fdc->modified[drive] = 0;

	return (0);
}
