/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/iwm-io.c                                    *
 * Created:     2012-01-16 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "iwm.h"
#include "iwm-io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/block/block.h>
#include <drivers/block/blkpsi.h>

#include <drivers/psi/psi.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>
#include <drivers/pri/gcr-mac.h>


static
int iwm_drv_get_block_geo (disk_t *dsk, unsigned *cn, unsigned *hn)
{
	unsigned long cnt;

	cnt = dsk_get_block_cnt (dsk);

	if (cnt == 800) {
		*hn = 1;
	}
	else if (cnt == 1600) {
		*hn = 2;
	}
	else {
		return (1);
	}

	*cn = 80;

	return (0);
}

static
psi_img_t *iwm_drv_load_block (mac_iwm_drive_t *drv, disk_t *dsk)
{
	unsigned      c, h, s;
	unsigned      cn, hn, sn;
	unsigned long lba;
	psi_sct_t     *sct;
	psi_img_t     *img;

	if (iwm_drv_get_block_geo (dsk, &cn, &hn)) {
		return (NULL);
	}

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	lba = 0;

	for (c = 0; c < cn; c++) {
		sn = 12 - c / 16;

		for (h = 0; h < hn; h++) {
			for (s = 0; s < sn; s++) {
				sct = psi_sct_new (c, h, s, 512);

				if (sct == NULL) {
					psi_img_del (img);
					return (NULL);
				}

				psi_img_add_sector (img, sct, c, h);

				if (dsk_read_lba (dsk, sct->data, lba, 1)) {
					psi_img_del (img);
					return (NULL);
				}

				lba += 1;
			}
		}
	}

	return (img);
}

static
pri_img_t *iwm_drv_load_disk (mac_iwm_drive_t *drv)
{
	disk_t     *dsk;
	disk_psi_t *psi;
	psi_img_t  *img, *del;
	pri_img_t  *ret;

	dsk = dsks_get_disk (drv->dsks, drv->diskid);

	if (dsk == NULL) {
		return (NULL);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		psi = dsk->ext;
		img = psi->img;
		del = NULL;
	}
	else {
		img = iwm_drv_load_block (drv, dsk);
		del = img;
	}

	if (img == NULL) {
		return (NULL);
	}

	ret = pri_encode_gcr (img);

	psi_img_del (del);

	return (ret);
}

static
pri_img_t *iwm_drv_load_pri (mac_iwm_drive_t *drv)
{
	pri_img_t *ret;

	if (drv->fname == NULL) {
		return (NULL);
	}

	ret = pri_img_load (drv->fname, PRI_FORMAT_NONE);

	return (ret);
}

int iwm_drv_load (mac_iwm_drive_t *drv)
{
	pri_img_t *img;

	img = NULL;

	drv->use_fname = 0;

	if (drv->fname != NULL) {
		img = iwm_drv_load_pri (drv);

		if (img != NULL) {
			drv->use_fname = 1;
			mac_log_deb ("iwm: loading drive %u (pri)\n", drv->drive + 1);
		}
	}

	if (img == NULL) {
		img = iwm_drv_load_disk (drv);

		if (img != NULL) {
			mac_log_deb ("iwm: loading drive %u (disk)\n", drv->drive + 1);
		}
	}

	if (img == NULL) {
		mac_log_deb ("iwm: loading drive %u failed\n", drv->drive + 1);
		return (1);
	}

	pri_img_del (drv->img);

	drv->img = img;

	drv->dirty = 0;
	drv->cur_track = NULL;

	return (0);
}


static
int iwm_drv_save_block (mac_iwm_drive_t *drv, disk_t *dsk, psi_img_t *img)
{
	unsigned      c, h, s;
	unsigned      cn, hn, sn;
	unsigned      cnt;
	unsigned long lba;
	unsigned char buf[512];
	psi_sct_t     *sct;

	if (iwm_drv_get_block_geo (dsk, &cn, &hn)) {
		return (1);
	}

	lba = 0;

	for (c = 0; c < cn; c++) {
		sn = 12 - c / 16;

		for (h = 0; h < hn; h++) {
			for (s = 0; s < sn; s++) {
				sct = psi_img_get_sector (img, c, h, s, 0);

				if (sct == NULL) {
					memset (buf, 0, 512);
				}
				else {
					cnt = 512;

					if (sct->n < 512) {
						cnt = sct->n;
						memset (buf + cnt, 0, 512 - cnt);
					}

					memcpy (buf, sct->data, cnt);
				}

				if (dsk_write_lba (dsk, buf, lba + s, 1)) {
					return (1);
				}
			}

			lba += sn;
		}
	}

	return (0);
}

static
int iwm_drv_save_disk (mac_iwm_drive_t *drv)
{
	int        r;
	disk_t     *dsk;
	disk_psi_t *psi;
	psi_img_t  *img;

	dsk = dsks_get_disk (drv->dsks, drv->diskid);

	if (dsk == NULL) {
		return (1);
	}

	img = pri_decode_gcr (drv->img);

	if (img == NULL) {
		return (1);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		psi = dsk->ext;
		psi_img_del (psi->img);
		psi->img = img;
		psi->dirty = 1;
	}
	else {
		r = iwm_drv_save_block (drv, dsk, img);

		psi_img_del (img);

		if (r) {
			return (1);
		}
	}

	return (0);
}

static
int iwm_drv_save_pri (mac_iwm_drive_t *drv)
{
	if (drv->fname == NULL) {
		return (1);
	}

	if (pri_img_save (drv->fname, drv->img, PRI_FORMAT_NONE)) {
		return (1);
	}

	return (0);
}

int iwm_drv_save (mac_iwm_drive_t *drv)
{
	if (drv->img == NULL) {
		return (1);
	}

	mac_log_deb ("iwm: saving drive %u\n", drv->drive + 1);

	if (drv->use_fname) {
		if (iwm_drv_save_pri (drv)) {
			mac_log_deb ("iwm: saving drive %u failed (pri)\n",
				drv->drive + 1
			);
			return (1);
		}
	}
	else {
		if (iwm_drv_save_disk (drv)) {
			mac_log_deb ("iwm: saving drive %u failed (disk)\n",
				drv->drive + 1
			);
			return (1);
		}
	}

	drv->dirty = 0;

	return (0);
}
