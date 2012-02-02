/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfdc/pfdc-img-pfdc1.c                            *
 * Created:     2012-01-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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
#include "pfdc-io.h"


#define PFDC1_MAGIC           0x50464443

#define PFDC1_FLAG_ALTERNATE  0x01
#define PFDC1_FLAG_CRC_ID     0x04
#define PFDC1_FLAG_CRC_DATA   0x08
#define PFDC1_FLAG_DEL_DAM    0x10
#define PFDC1_FLAG_COMPRESSED 0x80
#define PFDC1_FLAG_MASK       0x1d

#define PFDC1_ENC_UNKNOWN     0
#define PFDC1_ENC_FM          1
#define PFDC1_ENC_MFM         2


static
void pfdc1_sct_set_flags (pfdc_sct_t *sct, unsigned long flg)
{
	if (flg & PFDC1_FLAG_CRC_ID) {
		sct->flags |= PFDC_FLAG_CRC_ID;
	}

	if (flg & PFDC1_FLAG_CRC_DATA) {
		sct->flags |= PFDC_FLAG_CRC_DATA;
	}

	if (flg & PFDC1_FLAG_DEL_DAM) {
		sct->flags |= PFDC_FLAG_DEL_DAM;
	}
}

static
void pfdc1_sct_set_encoding (pfdc_sct_t *sct, unsigned enc, unsigned long dr)
{
	switch (enc) {
	case PFDC1_ENC_FM:
		enc = (dr < 375000) ? PFDC_ENC_FM_DD : PFDC_ENC_FM_HD;
		break;

	case PFDC1_ENC_MFM:
		enc = (dr < 375000) ? PFDC_ENC_MFM_DD : PFDC_ENC_MFM_HD;
		break;

	default:
		enc = 0;
		break;
	}

	pfdc_sct_set_encoding (sct, enc);
}

static
int pfdc1_load_sector (FILE *fp, pfdc_img_t *img, pfdc_sct_t **last)
{
	unsigned      c, h, n;
	unsigned long f, dr;
	unsigned char buf[32];
	pfdc_sct_t    *sct;

	if (pfdc_read (fp, buf, 16)) {
		return (1);
	}

	if (buf[0] != 'S') {
		return (1);
	}

	c = buf[1];
	h = buf[2];
	n = pfdc_get_uint16_be (buf, 6);

	sct = pfdc_sct_new (buf[3], buf[4], buf[5], n);

	if (sct == NULL) {
		return (1);
	}

	f = pfdc_get_uint32_be (buf, 12);
	dr = pfdc_get_uint32_be (buf, 8) & 0x00ffffff;

	pfdc1_sct_set_flags (sct, f);
	pfdc1_sct_set_encoding (sct, buf[8], dr);

	if (f & PFDC1_FLAG_ALTERNATE) {
		if (*last == NULL) {
			pfdc_sct_del (sct);
			return (1);
		}

		pfdc_sct_add_alternate (*last, sct);
	}
	else {
		if (pfdc_img_add_sector (img, sct, c, h)) {
			pfdc_sct_del (sct);
			return (1);
		}
	}

	if (f & PFDC1_FLAG_COMPRESSED) {
		if (pfdc_read (fp, buf + 16, 1)) {
			return (1);
		}

		pfdc_sct_fill (sct, buf[16]);
	}
	else {
		if (pfdc_read (fp, sct->data, n)) {
			return (1);
		}
	}

	*last = sct;

	return (0);
}

int pfdc1_load_fp (FILE *fp, pfdc_img_t *img, unsigned long id, unsigned long sz)
{
	unsigned long i;
	unsigned long cnt, ofs;
	unsigned char buf[8];
	pfdc_sct_t    *last;

	fprintf (stderr, "pfdc: warning: loading deprecated version 1 file\n");

	if (pfdc_read (fp, buf, 8)) {
		return (1);
	}

	cnt = pfdc_get_uint32_be (buf, 0);
	ofs = pfdc_get_uint32_be (buf, 4);

	if (ofs < 16) {
		return (1);
	}

	if (pfdc_skip (fp, ofs - 16)) {
		return (1);
	}

	last = NULL;

	for (i = 0; i < cnt; i++) {
		if (pfdc1_load_sector (fp, img, &last)) {
			return (1);
		}
	}

	return (0);
}


static
void pfdc1_sct_get_encoding (const pfdc_sct_t *sct, unsigned *enc, unsigned long *dr)
{
	switch (sct->encoding) {
	case PFDC_ENC_FM_DD:
		*enc = PFDC1_ENC_FM;
		*dr = 125000;
		break;

	case PFDC_ENC_FM_HD:
		*enc = PFDC1_ENC_FM;
		*dr = 250000;
		break;

	case PFDC_ENC_MFM_DD:
		*enc = PFDC1_ENC_MFM;
		*dr = 250000;
		break;

	case PFDC_ENC_MFM_HD:
		*enc = PFDC1_ENC_MFM;
		*dr = 500000;
		break;

	case PFDC_ENC_MFM_ED:
		*enc = PFDC1_ENC_MFM;
		*dr = 1000000;
		break;

	default:
		*enc = 0;
		*dr = 0;
		break;
	}
}

static
unsigned long pfdc1_sct_get_flags (const pfdc_sct_t *sct)
{
	unsigned long f;

	f = 0;

	if (sct->flags & PFDC_FLAG_CRC_ID) {
		f |= PFDC1_FLAG_CRC_ID;
	}

	if (sct->flags & PFDC_FLAG_CRC_DATA) {
		f |= PFDC1_FLAG_CRC_DATA;
	}

	if (sct->flags & PFDC_FLAG_DEL_DAM) {
		f |= PFDC1_FLAG_DEL_DAM;
	}

	return (f);
}

static
int pfdc1_save_sector (FILE *fp, const pfdc_sct_t *sct, unsigned c, unsigned h, int alt)
{
	unsigned long flg;
	unsigned      enc;
	unsigned long dr;
	unsigned char buf[32];

	flg = pfdc1_sct_get_flags (sct);

	if (pfdc_sct_uniform (sct) && (sct->n > 0)) {
		flg |= PFDC1_FLAG_COMPRESSED;
	}

	if (alt) {
		flg |= PFDC1_FLAG_ALTERNATE;
	}

	pfdc1_sct_get_encoding (sct, &enc, &dr);

	pfdc_set_uint32_be (buf, 8, dr);

	buf[0] = 'S';
	buf[1] = c;
	buf[2] = h;
	buf[3] = sct->c;
	buf[4] = sct->h;
	buf[5] = sct->s;
	buf[6] = (sct->n >> 8) & 0xff;
	buf[7] = sct->n & 0xff;
	buf[8] = enc;

	pfdc_set_uint32_be (buf, 12, flg);

	if (flg & PFDC1_FLAG_COMPRESSED) {
		buf[16] = sct->data[0];

		if (pfdc_write (fp, buf, 17)) {
			return (1);
		}
	}
	else {
		if (pfdc_write (fp, buf, 16)) {
			return (1);
		}

		if (pfdc_write (fp, sct->data, sct->n)) {
			return (1);
		}
	}

	return (0);
}

static
int pfdc1_save_alternates (FILE *fp, const pfdc_sct_t *sct, unsigned c, unsigned h)
{
	if (pfdc1_save_sector (fp, sct, c, h, 0)) {
		return (1);
	}

	sct = sct->next;

	while (sct != NULL) {
		if (pfdc1_save_sector (fp, sct, c, h, 1)) {
			return (1);
		}

		sct = sct->next;
	}

	return (0);
}

int pfdc1_save_fp (FILE *fp, const pfdc_img_t *img)
{
	unsigned         c, h, s;
	unsigned long    scnt;
	unsigned char    buf[16];
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;
	const pfdc_sct_t *sct;

	scnt = pfdc_img_get_sector_count (img);

	pfdc_set_uint32_be (buf, 0, PFDC1_MAGIC);
	pfdc_set_uint16_be (buf, 4, 1);
	pfdc_set_uint16_be (buf, 6, 0);
	pfdc_set_uint32_be (buf, 8, scnt);
	pfdc_set_uint32_be (buf, 12, 16);

	if (pfdc_write (fp, buf, 16)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (pfdc1_save_alternates (fp, sct, c, h)) {
					return (1);
				}
			}
		}
	}

	fflush (fp);

	return (0);
}
