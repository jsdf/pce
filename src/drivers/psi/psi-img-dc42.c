/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-dc42.c                               *
 * Created:     2011-07-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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

/* Apple DiskCopy 4.2 image files */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "psi.h"
#include "psi-io.h"
#include "psi-img-dc42.h"


#define DC42_MAGIC 0x0100


unsigned long dc42_calc_checksum (const void *buf, unsigned long cnt, unsigned long chk)
{
	unsigned            val;
	const unsigned char *tmp = buf;

	cnt >>= 1;

	while (cnt > 0) {
		val = tmp[0] & 0xff;
		val = (val << 8) | (tmp[1] & 0xff);
		chk = (chk + val) & 0xffffffff;
		chk = ((chk >> 1) | (chk << 31)) & 0xffffffff;
		cnt -= 1;
		tmp += 2;
	}

	return (chk);
}

/*
 * Load the sector data for 400K and 800K disks.
 */
static
int dc42_load_gcr (FILE *fp, psi_img_t *img, unsigned hcnt, unsigned long *check, unsigned fmt)
{
	unsigned  c, h, s;
	unsigned  sct_cnt;
	psi_trk_t *trk;
	psi_sct_t *sct;

	*check = 0;

	for (c = 0; c < 80; c++) {
		sct_cnt = 12 - (c / 16);

		for (h = 0; h < hcnt; h++) {
			for (s = 0; s < sct_cnt; s++) {
				sct = psi_sct_new (c, h, s, 512);

				if (sct == NULL) {
					return (1);
				}

				psi_sct_set_gcr_format (sct, fmt);
				psi_sct_set_encoding (sct, PSI_ENC_GCR);

				if (psi_img_add_sector (img, sct, c, h)) {
					psi_sct_del (sct);
					return (1);
				}

				if (psi_read (fp, sct->data, 512)) {
					return (1);
				}

				*check = dc42_calc_checksum (sct->data, 512, *check);
			}

			trk = psi_img_get_track (img, c, h, 0);

			if (trk != NULL) {
				psi_trk_interleave (trk, 2);
			}
		}
	}

	return (0);
}

/*
 * Load the sector data for 720K and 1440K disks.
 */
static
int dc42_load_mfm (FILE *fp, psi_img_t *img, unsigned scnt, unsigned long *check)
{
	unsigned  c, h, s, enc;
	psi_sct_t *sct;

	*check = 0;

	enc = (scnt < 14) ? PSI_ENC_MFM_DD : PSI_ENC_MFM_HD;

	for (c = 0; c < 80; c++) {
		for (h = 0; h < 2; h++) {
			for (s = 0; s < scnt; s++) {
				sct = psi_sct_new (c, h, s + 1, 512);

				if (sct == NULL) {
					return (1);
				}

				psi_sct_set_encoding (sct, enc);

				if (psi_img_add_sector (img, sct, c, h)) {
					psi_sct_del (sct);
					return (1);
				}

				if (psi_read (fp, sct->data, 512)) {
					return (1);
				}

				*check = dc42_calc_checksum (sct->data, 512, *check);
			}
		}
	}

	return (0);
}

static
int dc42_load_tags (FILE *fp, psi_img_t *img, unsigned long size, unsigned long *check)
{
	unsigned      c, h, s;
	psi_cyl_t     *cyl;
	psi_trk_t     *trk;
	psi_sct_t     *sct;
	unsigned char buf[12];

	*check = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = psi_img_get_sector (img, c, h, s, 0);

				if (sct == NULL) {
					return (1);
				}

				if (size < 12) {
					return (1);
				}

				if (psi_read (fp, buf, 12)) {
					return (1);
				}

				psi_sct_set_tags (sct, buf, 12);

				if ((c > 0) || (h > 0) || (s > 0)) {
					*check = dc42_calc_checksum (buf, 12, *check);
				}

				size -= 12;
			}
		}
	}

	if (size != 0) {
		return (1);
	}

	return (0);
}

static
int dc42_load_fp (FILE *fp, psi_img_t *img)
{
	int           r;
	unsigned char fmt;
	unsigned long data_size, tags_size;
	unsigned long data_check, tags_check;
	unsigned long check;
	unsigned char buf[128];

	if (psi_read (fp, buf, 84)) {
		return (1);
	}

	if (psi_get_uint16_be (buf, 82) != DC42_MAGIC) {
		return (1);
	}

	if (buf[0] > 63) {
		return (1);
	}

	data_size = psi_get_uint32_be (buf, 64);
	tags_size = psi_get_uint32_be (buf, 68);

	data_check = psi_get_uint32_be (buf, 72);
	tags_check = psi_get_uint32_be (buf, 76);

	fmt = buf[81];

	switch (data_size) {
	case 400UL * 1024UL:
		r = dc42_load_gcr (fp, img, 1, &check, fmt);
		break;

	case 800UL * 1024UL:
		r = dc42_load_gcr (fp, img, 2, &check, fmt);
		break;

	case 720UL * 1024UL:
		r = dc42_load_mfm (fp, img, 9, &check);
		break;

	case 1440UL * 1024UL:
		r = dc42_load_mfm (fp, img, 18, &check);
		break;

	default:
		return (1);
	}

	if (r) {
		return (1);
	}

	if (data_check != check) {
		fprintf (stderr, "dc42: data checksum error\n");
		return (1);
	}

	if (tags_size > 0) {
		if (dc42_load_tags (fp, img, tags_size, &check)) {
			return (1);
		}

		if (tags_check != check) {
			fprintf (stderr, "dc42: tag checksum error\n");
			return (1);
		}
	}

	return (0);
}

psi_img_t *psi_load_dc42 (FILE *fp)
{
	psi_img_t *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (dc42_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


/*
 * Check if an image is a 400K or 800K GCR disk
 */
static
int dc42_check_gcr (const psi_img_t *img, unsigned trk_cnt)
{
	unsigned        c, h, s, sct_cnt;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	if (img->cyl_cnt != 80) {
		return (0);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		if (cyl->trk_cnt != trk_cnt) {
			return (0);
		}

		sct_cnt = 12 - c / 16;

		for (h = 0; h < trk_cnt; h++) {
			trk = cyl->trk[h];

			if (trk->sct_cnt != sct_cnt) {
				return (0);
			}

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (sct->n != 512) {
					return (0);
				}

				if (sct->s >= sct_cnt) {
					return (0);
				}
			}
		}
	}

	return (1);
}

/*
 * Check if an image is a 720K or 1440K MFM disk
 */
static
int dc42_check_mfm (const psi_img_t *img, unsigned trk_cnt, unsigned sct_cnt)
{
	unsigned        c, h, s;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct;

	if (img->cyl_cnt != 80) {
		return (0);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		if (cyl->trk_cnt != trk_cnt) {
			return (0);
		}

		for (h = 0; h < trk_cnt; h++) {
			trk = cyl->trk[h];

			if (trk->sct_cnt != sct_cnt) {
				return (0);
			}

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (sct->n != 512) {
					return (0);
				}

				if ((sct->s < 1) || (sct->s > sct_cnt)) {
					return (0);
				}
			}
		}
	}

	return (1);
}

/*
 * Check if an image contains any tags
 */
static
int dc42_check_tags (const psi_img_t *img)
{
	unsigned        c, h, s;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				if (trk->sct[s]->tag_cnt > 0) {
					return (1);
				}
			}
		}
	}

	return (0);
}

static
int dc42_calc_checksums (const psi_img_t *img, unsigned cn, unsigned hn, unsigned sn,
	unsigned long *data, unsigned long *tags)
{
	unsigned        c, h, s, s1, s2;
	unsigned char   buf[12];
	psi_img_t       *tmp;
	const psi_sct_t *sct;

	*data = 0;
	*tags = 0;

	tmp = (psi_img_t *) img;

	s1 = (sn == 0) ? 0 : 1;

	for (c = 0; c < cn; c++) {
		s2 = (sn == 0) ? (12 - c / 16) : (sn + 1);

		for (h = 0; h < hn; h++) {
			for (s = s1; s < s2; s++) {
				sct = psi_img_get_sector (tmp, c, h, s, 0);

				if (sct == NULL) {
					return (1);
				}

				*data = dc42_calc_checksum (sct->data, sct->n, *data);

				if ((c > 0) || (h > 0) || (s > 0)) {
					psi_sct_get_tags (sct, buf, 12);
					*tags = dc42_calc_checksum (buf, 12, *tags);
				}
			}
		}
	}

	return (0);
}

static
int dc42_save_data (FILE *fp, const psi_img_t *img, unsigned cn, unsigned hn, unsigned sn)
{
	unsigned        c, h, s, s1, s2;
	psi_img_t       *tmp;
	const psi_sct_t *sct;

	tmp = (psi_img_t *) img;

	s1 = (sn == 0) ? 0 : 1;

	for (c = 0; c < cn; c++) {
		s2 = (sn == 0) ? (12 - c / 16) : (sn + 1);

		for (h = 0; h < hn; h++) {
			for (s = s1; s < s2; s++) {
				sct = psi_img_get_sector (tmp, c, h, s, 0);

				if (sct == NULL) {
					return (1);
				}

				if (psi_write (fp, sct->data, sct->n)) {
					return (1);
				}
			}
		}
	}

	return (0);
}

static
int dc42_save_tags (FILE *fp, const psi_img_t *img, unsigned cn, unsigned hn, unsigned sn)
{
	unsigned        c, h, s, s1, s2;
	unsigned char   buf[12];
	psi_img_t       *tmp;
	const psi_sct_t *sct;

	tmp = (psi_img_t *) img;

	s1 = (sn == 0) ? 0 : 1;

	for (c = 0; c < cn; c++) {
		s2 = (sn == 0) ? (12 - c / 16) : (sn + 1);

		for (h = 0; h < hn; h++) {
			for (s = s1; s < s2; s++) {
				sct = psi_img_get_sector (tmp, c, h, s, 0);

				if (sct == NULL) {
					return (1);
				}

				psi_sct_get_tags (sct, buf, 12);

				if (psi_write (fp, buf, 12)) {
					return (1);
				}
			}
		}
	}

	return (0);
}

static
int dc42_set_image_name (unsigned char *buf, const psi_img_t *img, unsigned mdb_sct)
{
	unsigned            i, n;
	const               psi_sct_t *sct;
	const unsigned char *src;

	src = (const unsigned char *) "\x06Noname";

	sct = psi_img_get_sector ((psi_img_t *) img, 0, 0, mdb_sct, 0);

	if (sct != NULL) {
		if ((sct->data[0] == 'B') && (sct->data[1] == 'D')) {
			src = sct->data + 36;
		}
		else if ((sct->data[0] == 0xd2) && (sct->data[1] == 0xd7)) {
			src = sct->data + 36;
		}
	}

	n = src[0];

	if (n > 63) {
		n = 63;
	}

	for (i = 0; i < n; i++) {
		buf[i + 1] = src[i + 1];
	}

	buf[0] = n;

	return (0);
}

int psi_save_dc42 (FILE *fp, const psi_img_t *img)
{
	unsigned      cn, hn, sn, enc;
	unsigned long data_check, tags_check;
	unsigned long data_size, tags_size;
	unsigned char buf[128];

	cn = 80;
	hn = 2;
	sn = 0;

	tags_size = 0;

	if (dc42_check_gcr (img, 1)) {
		hn = 1;
		enc = 0x0002;
		data_size = 400UL * 1024UL;

		if (dc42_check_tags (img)) {
			tags_size = 12 * (data_size / 512);
		}
	}
	else if (dc42_check_gcr (img, 2)) {
		enc = 0x0122;
		data_size = 800UL * 1024UL;

		if (dc42_check_tags (img)) {
			tags_size = 12 * (data_size / 512);
		}
	}
	else if (dc42_check_mfm (img, 2, 9)) {
		sn = 9;
		enc = 0x0222;
		data_size = 720UL * 1024UL;
	}
	else if (dc42_check_mfm (img, 2, 18)) {
		sn = 18;
		enc = 0x0322;
		data_size = 1440UL * 1024UL;
	}
	else {
		return (1);
	}

	if (dc42_calc_checksums (img, cn, hn, sn, &data_check, &tags_check)) {
		return (1);
	}

	memset (buf, 0, 128);

	dc42_set_image_name (buf, img, (sn == 0) ? 2 : 3);

	psi_set_uint32_be (buf, 64, data_size);
	psi_set_uint32_be (buf, 68, tags_size);

	psi_set_uint32_be (buf, 72, data_check);
	psi_set_uint32_be (buf, 76, tags_check);

	psi_set_uint16_be (buf, 80, enc);
	psi_set_uint16_be (buf, 82, 0x0100);

	if (psi_write (fp, buf, 84)) {
		return (1);
	}

	if (dc42_save_data (fp, img, cn, hn, sn)) {
		return (1);
	}

	if (tags_size > 0) {
		if (dc42_save_tags (fp, img, cn, hn, sn)) {
			return (1);
		}
	}

	fflush (fp);

	return (0);
}

int psi_probe_dc42_fp (FILE *fp)
{
	unsigned char buf[128];

	if (fseek (fp, 0, SEEK_SET)) {
		return (0);
	}

	if (psi_read (fp, buf, 84)) {
		return (0);
	}

	if (psi_get_uint16_be (buf, 82) != DC42_MAGIC) {
		return (0);
	}

	if (buf[0] > 63) {
		return (0);
	}

	return (1);
}

int psi_probe_dc42 (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = psi_probe_dc42_fp (fp);

	fclose (fp);

	return (r);
}
