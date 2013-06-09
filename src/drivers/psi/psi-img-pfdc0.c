/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-pfdc0.c                              *
 * Created:     2012-01-30 by Hampa Hug <hampa@hampa.ch>                     *
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


#define PFDC0_MAGIC 0x50464443

#define PFDC0_FLAG_FM         0x01
#define PFDC0_FLAG_MFM        0x02
#define PFDC0_FLAG_CRC_ID     0x04
#define PFDC0_FLAG_CRC_DATA   0x08
#define PFDC0_FLAG_DEL_DAM    0x10
#define PFDC0_FLAG_COMPRESSED 0x80
#define PFDC0_FLAG_MASK       0x1f


static
void pfdc0_sct_set_flags (psi_sct_t *sct, unsigned flg, unsigned long dr)
{
	unsigned enc;

	if (flg & PFDC0_FLAG_FM) {
		enc = (dr < 375000) ? PSI_ENC_FM_DD : PSI_ENC_FM_DD;
	}
	else if (flg & PFDC0_FLAG_MFM) {
		enc = (dr < 375000) ? PSI_ENC_MFM_DD : PSI_ENC_MFM_DD;
	}
	else {
		enc = PSI_ENC_UNKNOWN;
	}

	psi_sct_set_encoding (sct, enc);

	if (flg & PFDC0_FLAG_CRC_ID) {
		sct->flags |= PSI_FLAG_CRC_ID;
	}

	if (flg & PFDC0_FLAG_CRC_DATA) {
		sct->flags |= PSI_FLAG_CRC_DATA;
	}

	if (flg & PFDC0_FLAG_DEL_DAM) {
		sct->flags |= PSI_FLAG_DEL_DAM;
	}
}

static
int pfdc0_load_sector (FILE *fp, psi_img_t *img)
{
	unsigned      c, h, n, f;
	unsigned char buf[16];
	psi_sct_t     *sct;

	if (psi_read (fp, buf, 12)) {
		return (1);
	}

	c = buf[0];
	h = buf[1];
	f = buf[5];
	n = psi_get_uint16_be (buf, 6);

	sct = psi_sct_new (buf[2], buf[3], buf[4], n);

	if (sct == NULL) {
		return (1);
	}

	if (psi_img_add_sector (img, sct, c, h)) {
		psi_sct_del (sct);
		return (1);
	}

	pfdc0_sct_set_flags (sct, f, psi_get_uint32_be (buf, 8));

	if (f & PFDC0_FLAG_COMPRESSED) {
		if (psi_read (fp, buf + 12, 1)) {
			return (1);
		}

		psi_sct_fill (sct, buf[12]);
	}
	else {
		if (psi_read (fp, sct->data, n)) {
			return (1);
		}
	}

	return (0);
}

int pfdc0_load_fp (FILE *fp, psi_img_t *img, unsigned long id, unsigned long sz)
{
	unsigned long i;
	unsigned long cnt, ofs;
	unsigned char buf[8];

	fprintf (stderr, "pfdc: warning: loading deprecated version 0 file\n");

	if (psi_read (fp, buf, 8)) {
		return (1);
	}

	cnt = psi_get_uint32_be (buf, 0);
	ofs = psi_get_uint32_be (buf, 4);

	if (ofs < 16) {
		return (1);
	}

	if (psi_skip (fp, ofs - 16)) {
		return (1);
	}

	for (i = 0; i < cnt; i++) {
		if (pfdc0_load_sector (fp, img)) {
			return (1);
		}
	}

	return (0);
}


static
void pfdc0_sct_get_flags (const psi_sct_t *sct, unsigned *flg, unsigned long *dr)
{
	*flg = 0;

	switch (sct->encoding) {
	case PSI_ENC_FM_DD:
		*flg |= PFDC0_FLAG_FM;
		*dr = 125000;
		break;

	case PSI_ENC_FM_HD:
		*flg |= PFDC0_FLAG_FM;
		*dr = 250000;
		break;

	case PSI_ENC_MFM_DD:
		*flg |= PFDC0_FLAG_MFM;
		*dr = 250000;
		break;

	case PSI_ENC_MFM_HD:
		*flg |= PFDC0_FLAG_MFM;
		*dr = 500000;
		break;

	case PSI_ENC_MFM_ED:
		*flg |= PFDC0_FLAG_MFM;
		*dr = 1000000;
		break;

	default:
		*dr = 0;
		break;
	}

	if (sct->flags & PSI_FLAG_CRC_ID) {
		*flg |= PFDC0_FLAG_CRC_ID;
	}

	if (sct->flags & PSI_FLAG_CRC_DATA) {
		*flg |= PFDC0_FLAG_CRC_DATA;
	}

	if (sct->flags & PSI_FLAG_DEL_DAM) {
		*flg |= PFDC0_FLAG_DEL_DAM;
	}
}

static
int pfdc0_save_sector (FILE *fp, const psi_sct_t *sct, unsigned c, unsigned h)
{
	int           compr;
	unsigned      flg;
	unsigned long dr;
	unsigned char buf[16];

	pfdc0_sct_get_flags (sct, &flg, &dr);

	buf[0] = c;
	buf[1] = h;
	buf[2] = sct->c;
	buf[3] = sct->h;
	buf[4] = sct->s;
	buf[5] = flg;
	buf[6] = (sct->n >> 8) & 0xff;
	buf[7] = sct->n & 0xff;

	psi_set_uint32_be (buf, 8, dr);

	compr = psi_sct_uniform (sct) && (sct->n > 0);

	if (compr) {
		buf[5] |= PFDC0_FLAG_COMPRESSED;
		buf[12] = sct->data[0];

		if (psi_write (fp, buf, 13)) {
			return (1);
		}
	}
	else {
		if (psi_write (fp, buf, 12)) {
			return (1);
		}

		if (psi_write (fp, sct->data, sct->n)) {
			return (1);
		}
	}

	return (0);
}

int pfdc0_save_fp (FILE *fp, const psi_img_t *img)
{
	unsigned        c, h, s;
	unsigned long   scnt;
	unsigned char   buf[16];
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	scnt = psi_img_get_sector_count (img);

	psi_set_uint32_be (buf, 0, PFDC0_MAGIC);
	psi_set_uint32_be (buf, 4, 0);
	psi_set_uint32_be (buf, 8, scnt);
	psi_set_uint32_be (buf, 12, 16);

	if (psi_write (fp, buf, 16)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (pfdc0_save_sector (fp, sct, c, h)) {
					return (1);
				}
			}
		}
	}

	fflush (fp);

	return (0);
}
