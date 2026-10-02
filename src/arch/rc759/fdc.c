/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/fdc.c                                         *
 * Created:     2012-07-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2021 Hampa Hug <hampa@hampa.ch>                     *
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
#include <drivers/block/blkpri.h>
#include <drivers/block/blkpsi.h>

#include <drivers/psi/psi.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>
#include <drivers/pri/pri-enc-mfm.h>

#include <lib/log.h>
#include <lib/string.h>


#ifndef DEBUG_FDC
#define DEBUG_FDC 1
#endif


#define RC759_TRACK_SIZE 166666


static
int rc759_read_track (void *ext, unsigned d, unsigned c, unsigned h, pri_trk_t **trk)
{
	rc759_fdc_t *fdc;
	pri_img_t   *img;

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
int rc759_write_track (void *ext, unsigned d, unsigned c, unsigned h, pri_trk_t *trk)
{
	rc759_fdc_t *fdc;

	fdc = ext;

	if (fdc->img[d & 1] == NULL) {
		return (1);
	}

	fdc->modified[d & 1] = 1;

	return (0);
}

void rc759_fdc_init (rc759_fdc_t *fdc)
{
	unsigned i;

	wd179x_init (&fdc->wd179x);

	wd179x_set_read_track_fct (&fdc->wd179x, fdc, rc759_read_track);
	wd179x_set_write_track_fct (&fdc->wd179x, fdc, rc759_write_track);

	wd179x_set_default_track_size (&fdc->wd179x, 0, RC759_TRACK_SIZE);
	wd179x_set_default_track_size (&fdc->wd179x, 1, RC759_TRACK_SIZE);

	wd179x_set_ready (&fdc->wd179x, 0, 0);
	wd179x_set_ready (&fdc->wd179x, 1, 0);

	fdc->fcr = 0;

	for (i = 0; i < 2; i++) {
		fdc->diskid[i] = 0xffff;
		fdc->img[i] = NULL;
		fdc->img_del[i] = 0;
		fdc->modified[i] = 0;
	}
}

void rc759_fdc_free (rc759_fdc_t *fdc)
{
	unsigned i;

	for (i = 0; i < 2; i++) {
		rc759_fdc_save (fdc, i);

		if (fdc->img_del[i]) {
			pri_img_del (fdc->img[i]);
		}
	}

	wd179x_free (&fdc->wd179x);
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

static
int rc759_fdc_eject_drive (rc759_fdc_t *fdc, unsigned drive)
{
	wd179x_flush (&fdc->wd179x, drive);

	if (rc759_fdc_save (fdc, drive)) {
		return (1);
	}

	wd179x_set_ready (&fdc->wd179x, drive, 0);

	if (fdc->img_del[drive]) {
		pri_img_del (fdc->img[drive]);
	}

	fdc->img[drive] = NULL;
	fdc->img_del[drive] = 0;

	fdc->modified[drive] = 0;

	return (0);
}

static
int rc759_fdc_insert_drive (rc759_fdc_t *fdc, unsigned drive)
{
	wd179x_flush (&fdc->wd179x, drive);

	if (rc759_fdc_load (fdc, drive)) {
		return (1);
	}

	return (0);
}

int rc759_fdc_eject_disk (rc759_fdc_t *fdc, unsigned id)
{
	int      r;
	unsigned i;

	r = 0;

	for (i = 0; i < 2; i++) {
		if (fdc->diskid[i] == id) {
			r |= rc759_fdc_eject_drive (fdc, i);
		}
	}

	return (r);
}

int rc759_fdc_insert_disk (rc759_fdc_t *fdc, unsigned id)
{
	int      r;
	unsigned i;

	r = 0;

	for (i = 0; i < 2; i++) {
		if (fdc->diskid[i] == id) {
			r |= rc759_fdc_insert_drive (fdc, i);
		}
	}

	return (r);
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
psi_img_t *rc759_fdc_load_block (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive)
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
int rc759_fdc_load_psi (rc759_fdc_t *fdc, psi_img_t *img, unsigned drive)
{
	pri_enc_mfm_t par;

	pri_encode_mfm_init (&par, 1000000, 360);

	par.enable_iam = 1;
	par.auto_gap3 = 1;
	par.gap4a = 80;
	par.gap1 = 50;
	par.gap3 = 54;

	fdc->img[drive] = pri_encode_mfm (img, &par);
	fdc->img_del[drive] = (fdc->img[drive] != NULL);

	return (0);
}

static
int rc759_fdc_load_disk_pri (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	disk_pri_t *pri;

	pri = dsk->ext;

	fdc->img[drive] = pri->img;
	fdc->img_del[drive] = 0;

	return (0);
}

static
int rc759_fdc_load_disk_psi (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	disk_psi_t *psi;

	psi = dsk->ext;

	if (rc759_fdc_load_psi (fdc, psi->img, drive)) {
		return (1);
	}

	return (0);
}

static
int rc759_fdc_load_disk (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	int       r;
	psi_img_t *psi;

	if ((psi = rc759_fdc_load_block (fdc, dsk, drive)) == NULL) {
		return (1);
	}

	r = rc759_fdc_load_psi (fdc, psi, drive);

	psi_img_del (psi);

	return (r);
}

int rc759_fdc_load (rc759_fdc_t *fdc, unsigned drive)
{
	unsigned type;
	disk_t   *dsk;

	if (drive >= 2) {
		return (1);
	}

	rc759_fdc_eject_drive (fdc, drive);

	if ((dsk = dsks_get_disk (fdc->dsks, fdc->diskid[drive])) == NULL) {
		return (1);
	}

	type = dsk_get_type (dsk);

	if (type == PCE_DISK_PRI) {
		if (rc759_fdc_load_disk_pri (fdc, dsk, drive)) {
			return (1);
		}
	}
	else if (type == PCE_DISK_PSI) {
		if (rc759_fdc_load_disk_psi (fdc, dsk, drive)) {
			return (1);
		}
	}
	else {
		if (rc759_fdc_load_disk (fdc, dsk, drive)) {
			return (1);
		}
	}

	wd179x_set_ready (&fdc->wd179x, drive, 1);

	return (0);
}

static
int rc759_fdc_save_block (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive, psi_img_t *img)
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
int rc759_fdc_save_disk_pri (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	disk_pri_t *pri;

	pri = dsk->ext;
	pri->dirty = 1;

	return (0);
}

static
int rc759_fdc_save_disk_psi (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive)
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
int rc759_fdc_save_disk (rc759_fdc_t *fdc, disk_t *dsk, unsigned drive)
{
	int       r;
	psi_img_t *img;

	if ((img = pri_decode_mfm (fdc->img[drive], NULL)) == NULL) {
		return (1);
	}

	r = rc759_fdc_save_block (fdc, dsk, drive, img);

	psi_img_del (img);

	return (r);
}

int rc759_fdc_save (rc759_fdc_t *fdc, unsigned drive)
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
		if (rc759_fdc_save_disk_pri (fdc, dsk, drive)) {
			return (1);
		}
	}
	else if (type == PCE_DISK_PSI) {
		if (rc759_fdc_save_disk_psi (fdc, dsk, drive)) {
			return (1);
		}
	}
	else {
		if (rc759_fdc_save_disk (fdc, dsk, drive)) {
			return (1);
		}
	}

	fdc->modified[drive] = 0;

	return (0);
}
