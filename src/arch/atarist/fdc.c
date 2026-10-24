/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/fdc.c                                       *
 * Created:     2013-06-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "fdc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chipset/wd179x.h>

#include <drivers/block/block.h>
#include <drivers/block/blkpri.h>
#include <drivers/block/blkpsi.h>

#include <drivers/psi/psi.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>
#include <drivers/pri/pri-enc-mfm.h>

#include <lib/log.h>
#include <lib/string.h>


#ifndef DEBUG_FDC
#define DEBUG_FDC 0
#endif


static
int st_read_track (void *ext, unsigned d, unsigned c, unsigned h, pri_trk_t **trk)
{
	st_fdc_t  *fdc;
	pri_img_t *img;

	fdc = ext;

	if ((img = fdc->img[d & 1]) == NULL) {
		return (1);
	}

	if ((*trk = pri_img_get_track (img, c, h, 1)) == NULL) {
		return (1);
	}

	return (0);
}

static
int st_write_track (void *ext, unsigned d, unsigned c, unsigned h, pri_trk_t *trk)
{
	st_fdc_t *fdc;

	fdc = ext;

	if (fdc->img[d & 1] == NULL) {
		return (1);
	}

	fdc->modified[d & 1] = 1;

	return (0);
}

void st_fdc_init (st_fdc_t *fdc)
{
	unsigned i;

	wd179x_init (&fdc->wd179x);

	wd179x_set_read_track_fct (&fdc->wd179x, fdc, st_read_track);
	wd179x_set_write_track_fct (&fdc->wd179x, fdc, st_write_track);

	wd179x_set_ready (&fdc->wd179x, 0, 0);
	wd179x_set_ready (&fdc->wd179x, 1, 0);

	for (i = 0; i < 2; i++) {
		fdc->diskid[i] = 0xffff;
		fdc->wprot[i] = 0;
		fdc->media_change[i] = 0;
		fdc->img[i] = NULL;
		fdc->img_del[i] = 0;
		fdc->modified[i] = 0;
	}
}

void st_fdc_free (st_fdc_t *fdc)
{
	unsigned i;

	for (i = 0; i < 2; i++) {
		st_fdc_save (fdc, i);

		if (fdc->img_del[i]) {
			pri_img_del (fdc->img[i]);
		}
	}

	wd179x_free (&fdc->wd179x);
}

void st_fdc_reset (st_fdc_t *fdc)
{
	unsigned i;

	wd179x_reset (&fdc->wd179x);

	for (i = 0; i < 2; i++) {
		if (fdc->img[i] == NULL) {
			st_fdc_load (fdc, i);
		}
		else {
			st_fdc_save (fdc, i);
		}
	}
}

void st_fdc_set_disks (st_fdc_t *fdc, disks_t *dsks)
{
	fdc->dsks = dsks;
}

void st_fdc_set_disk_id (st_fdc_t *fdc, unsigned drive, unsigned diskid)
{
	fdc->diskid[drive] = diskid;

	wd179x_set_drive_mask (&fdc->wd179x, drive, diskid < 0xffff);
}

void st_fdc_set_wprot (st_fdc_t *fdc, unsigned drive, int wprot)
{
	if (drive < 2) {
		fdc->wprot[drive] = (wprot != 0);

		if (fdc->media_change[drive] == 0) {
			wd179x_set_wprot (&fdc->wd179x, drive, fdc->wprot[drive]);
		}
	}
}

static
int st_fdc_eject_drive (st_fdc_t *fdc, unsigned drive)
{
	wd179x_flush (&fdc->wd179x, drive);

	if (st_fdc_save (fdc, drive)) {
		return (1);
	}

	wd179x_set_ready (&fdc->wd179x, drive, 0);
	wd179x_set_wprot (&fdc->wd179x, drive, 1);

	fdc->media_change[drive] = 1;
	fdc->media_change_clk = 8000000 / 10;

	if (fdc->img_del[drive]) {
		pri_img_del (fdc->img[drive]);
	}

	fdc->img[drive] = NULL;
	fdc->img_del[drive] = 0;

	fdc->modified[drive] = 0;

	return (0);
}

static
int st_fdc_insert_drive (st_fdc_t *fdc, unsigned drive)
{
	wd179x_flush (&fdc->wd179x, drive);

	if (st_fdc_load (fdc, drive)) {
		return (1);
	}

	return (0);
}

int st_fdc_eject_disk (st_fdc_t *fdc, unsigned id)
{
	int      r;
	unsigned i;

	r = 0;

	for (i = 0; i < 2; i++) {
		if (fdc->diskid[i] == id) {
			r |= st_fdc_eject_drive (fdc, i);
		}
	}

	return (r);
}

int st_fdc_insert_disk (st_fdc_t *fdc, unsigned id)
{
	int      r;
	unsigned i;

	r = 0;

	for (i = 0; i < 2; i++) {
		if (fdc->diskid[i] == id) {
			r |= st_fdc_insert_drive (fdc, i);
		}
	}

	return (r);
}

static
psi_img_t *st_fdc_load_block (st_fdc_t *fdc, disk_t *dsk, unsigned drive)
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
int st_fdc_load_psi (st_fdc_t *fdc, psi_img_t *img, unsigned drive)
{
	pri_enc_mfm_t par;

	pri_encode_mfm_init (&par, 500000, 300);

	par.enable_iam = 0;
	par.auto_gap3 = 1;
	par.gap4a = 96;
	par.gap1 = 0;
	par.gap3 = 80;

	fdc->img[drive] = pri_encode_mfm (img, &par);
	fdc->img_del[drive] = (fdc->img[drive] != NULL);

	return (0);
}

static
int st_fdc_load_disk_pri (st_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	disk_pri_t *pri;

	pri = dsk->ext;

	fdc->img[drive] = pri->img;
	fdc->img_del[drive] = 0;

	return (0);
}

static
int st_fdc_load_disk_psi (st_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	disk_psi_t *psi;

	psi = dsk->ext;

	if (st_fdc_load_psi (fdc, psi->img, drive)) {
		return (1);
	}

	return (0);
}

static
int st_fdc_load_disk (st_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	int       r;
	psi_img_t *psi;

	if ((psi = st_fdc_load_block (fdc, dsk, drive)) == NULL) {
		return (1);
	}

	r = st_fdc_load_psi (fdc, psi, drive);

	psi_img_del (psi);

	return (r);
}

int st_fdc_load (st_fdc_t *fdc, unsigned drive)
{
	unsigned type;
	disk_t   *dsk;

	if (drive >= 2) {
		return (1);
	}

	st_fdc_eject_drive (fdc, drive);

	if ((dsk = dsks_get_disk (fdc->dsks, fdc->diskid[drive])) == NULL) {
		return (1);
	}

	type = dsk_get_type (dsk);

	if (type == PCE_DISK_PRI) {
		if (st_fdc_load_disk_pri (fdc, dsk, drive)) {
			return (1);
		}
	}
	else if (type == PCE_DISK_PSI) {
		if (st_fdc_load_disk_psi (fdc, dsk, drive)) {
			return (1);
		}
	}
	else {
		if (st_fdc_load_disk (fdc, dsk, drive)) {
			return (1);
		}
	}

	st_fdc_set_wprot (fdc, drive, dsk_get_readonly (dsk));

	wd179x_set_ready (&fdc->wd179x, drive, 1);

	return (0);
}

static
int st_fdc_save_block (st_fdc_t *fdc, disk_t *dsk, unsigned drive, psi_img_t *img)
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
int st_fdc_save_disk_pri (st_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	disk_pri_t *pri;

	pri = dsk->ext;
	pri->dirty = 1;

	return (0);
}

static
int st_fdc_save_disk_psi (st_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	disk_psi_t *psi;
	psi_img_t  *img;

	if ((img = pri_decode_mfm (fdc->img[drive], NULL)) == NULL) {
		return (1);
	}

	psi = dsk->ext;
	psi_img_del (psi->img);
	psi->img = img;
	psi->dirty = 1;

	return (0);
}

static
int st_fdc_save_disk (st_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	int       r;
	psi_img_t *img;

	if ((img = pri_decode_mfm (fdc->img[drive], NULL)) == NULL) {
		return (1);
	}

	r = st_fdc_save_block (fdc, dsk, drive, img);

	psi_img_del (img);

	return (r);
}

int st_fdc_save (st_fdc_t *fdc, unsigned drive)
{
	unsigned type;
	disk_t   *dsk;

	if (drive >= 2) {
		return (1);
	}

	if (fdc->img[drive] == NULL) {
		return (1);
	}

	wd179x_flush (&fdc->wd179x, drive);

	if (fdc->modified[drive] == 0) {
		return (0);
	}

	if ((dsk = dsks_get_disk (fdc->dsks, fdc->diskid[drive])) == NULL) {
		return (1);
	}

	type = dsk_get_type (dsk);

	if (type == PCE_DISK_PRI) {
		if (st_fdc_save_disk_pri (fdc, dsk, drive)) {
			return (1);
		}
	}
	else if (type == PCE_DISK_PSI) {
		if (st_fdc_save_disk_psi (fdc, dsk, drive)) {
			return (1);
		}
	}
	else {
		if (st_fdc_save_disk (fdc, dsk, drive)) {
			return (1);
		}
	}

	fdc->modified[drive] = 0;

	return (0);
}

void st_fdc_clock_media_change (st_fdc_t *fdc, unsigned cnt)
{
	if (fdc->media_change_clk == 0) {
		return;
	}

	if (cnt < fdc->media_change_clk) {
		fdc->media_change_clk -= cnt;
		return;
	}

	fdc->media_change_clk = 0;

	if (fdc->media_change[0]) {
		wd179x_set_wprot (&fdc->wd179x, 0, fdc->wprot[0]);
		fdc->media_change[0] = 0;
	}

	if (fdc->media_change[1]) {
		wd179x_set_wprot (&fdc->wd179x, 1, fdc->wprot[1]);
		fdc->media_change[1] = 0;
	}
}
