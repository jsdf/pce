/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc-img-dc42.c                            *
 * Created:     2011-07-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011 Hampa Hug <hampa@hampa.ch>                          *
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

#include "pfdc.h"
#include "pfdc-img-dc42.h"


#define DC42_MAGIC 0x0100


static
int dc42_read (FILE *fp, void *buf, unsigned cnt)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

static
int dc42_write (FILE *fp, const void *buf, unsigned cnt)
{
	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

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
int dc42_load_gcr (FILE *fp, pfdc_img_t *img, unsigned hcnt, unsigned long *check)
{
	unsigned   c, h, s;
	unsigned   sct_cnt;
	pfdc_sct_t *sct;

	*check = 0;

	sct_cnt = 13;

	for (c = 0; c < 80; c++) {
		if ((c & 15) == 0) {
			sct_cnt -= 1;
		}

		for (h = 0; h < hcnt; h++) {
			for (s = 0; s < sct_cnt; s++) {
				sct = pfdc_sct_new (c, h, s, 512);

				if (sct == NULL) {
					return (1);
				}

				pfdc_sct_set_encoding (sct, PFDC_ENC_GCR, 250000);

				if (pfdc_img_add_sector (img, sct, c, h)) {
					pfdc_sct_del (sct);
					return (1);
				}

				if (dc42_read (fp, sct->data, 512)) {
					return (1);
				}

				*check = dc42_calc_checksum (sct->data, 512, *check);
			}
		}
	}

	return (0);
}

/*
 * Load the sector data for 720K and 1440K disks.
 */
static
int dc42_load_mfm (FILE *fp, pfdc_img_t *img, unsigned scnt, unsigned long *check)
{
	unsigned   c, h, s;
	pfdc_sct_t *sct;

	*check = 0;

	for (c = 0; c < 80; c++) {
		for (h = 0; h < 2; h++) {
			for (s = 0; s < scnt; s++) {
				sct = pfdc_sct_new (c, h, s + 1, 512);

				if (sct == NULL) {
					return (1);
				}

				pfdc_sct_set_encoding (sct, PFDC_ENC_MFM, 250000);

				if (pfdc_img_add_sector (img, sct, c, h)) {
					pfdc_sct_del (sct);
					return (1);
				}

				if (dc42_read (fp, sct->data, 512)) {
					return (1);
				}

				*check = dc42_calc_checksum (sct->data, 512, *check);
			}
		}
	}

	return (0);
}

static
int dc42_load_tags (FILE *fp, pfdc_img_t *img, unsigned long size, unsigned long *check)
{
	unsigned      c, h, s;
	pfdc_cyl_t    *cyl;
	pfdc_trk_t    *trk;
	pfdc_sct_t    *sct;
	unsigned char buf[12];

	*check = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (size < 12) {
					return (1);
				}

				if (dc42_read (fp, buf, 12)) {
					return (1);
				}

				pfdc_sct_set_tags (sct, buf, 12);

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
int dc42_load_fp (FILE *fp, pfdc_img_t *img)
{
	int           r;
	unsigned long data_size, tags_size;
	unsigned long data_check, tags_check;
	unsigned long check;
	unsigned char buf[128];

	if (dc42_read (fp, buf, 84)) {
		return (1);
	}

	if (pfdc_get_uint16_be (buf, 82) != DC42_MAGIC) {
		return (1);
	}

	if (buf[0] > 63) {
		return (1);
	}

	data_size = pfdc_get_uint32_be (buf, 64);
	tags_size = pfdc_get_uint32_be (buf, 68);

	data_check = pfdc_get_uint32_be (buf, 72);
	tags_check = pfdc_get_uint32_be (buf, 76);

	switch (data_size) {
	case 400UL * 1024UL:
		r = dc42_load_gcr (fp, img, 1, &check);
		break;

	case 800UL * 1024UL:
		r = dc42_load_gcr (fp, img, 2, &check);
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

pfdc_img_t *pfdc_load_dc42 (FILE *fp)
{
	pfdc_img_t *img;

	img = pfdc_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (dc42_load_fp (fp, img)) {
		pfdc_img_del (img);
		return (NULL);
	}

	return (img);
}

static
int dc42_save_data (FILE *fp, const pfdc_img_t *img, int gcr)
{
	unsigned         c, h, s, si;
	pfdc_img_t       *tmp;
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;
	const pfdc_sct_t *sct;

	tmp = (pfdc_img_t *) img;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				si = gcr ? s : (s + 1);

				sct = pfdc_img_get_sector (tmp, c, h, si, 0);

				if (sct == NULL) {
					return (1);
				}

				if (dc42_write (fp, sct->data, 512)) {
					return (1);
				}
			}
		}
	}

	return (0);
}

static
int dc42_save_tags (FILE *fp, const pfdc_img_t *img, int gcr)
{
	unsigned         c, h, s, si;
	unsigned char    buf[12];
	pfdc_img_t       *tmp;
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;
	const pfdc_sct_t *sct;

	tmp = (pfdc_img_t *) img;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				si = gcr ? s : (s + 1);

				sct = pfdc_img_get_sector (tmp, c, h, si, 0);

				if (sct == NULL) {
					return (1);
				}

				pfdc_sct_get_tags (sct, buf, 12);

				if (dc42_write (fp, buf, 12)) {
					return (1);
				}
			}
		}
	}

	return (0);
}

static
int dc42_set_image_name (unsigned char *buf, const pfdc_img_t *img)
{
	unsigned            i, n;
	const               pfdc_sct_t *sct;
	const unsigned char *src;

	src = (const unsigned char *) "\x06Noname";

	sct = pfdc_img_get_sector ((pfdc_img_t *) img, 0, 0, 3, 0);

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

int pfdc_save_dc42 (FILE *fp, const pfdc_img_t *img)
{
	int              tags, gcr;
	unsigned         c, h, s;
	unsigned         enc;
	unsigned long    data_check, tags_check;
	unsigned long    data_size, tags_size;
	unsigned char    buf[128];
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;
	const pfdc_sct_t *sct;

	data_check = 0;
	tags_check = 0;

	data_size = 0;

	tags = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (sct->n != 512) {
					return (1);
				}

				data_check = dc42_calc_checksum (sct->data, sct->n, data_check);
				data_size += 512;

				if (sct->tag_cnt > 0) {
					tags = 1;
				}

				pfdc_sct_get_tags (sct, buf, 12);

				if ((c > 0) || (h > 0) || (s > 0)) {
					tags_check = dc42_calc_checksum (buf, 12, tags_check);
				}
			}
		}
	}

	if (tags) {
		tags_size = 12 * (data_size / 512);
	}
	else {
		tags_size = 0;
	}

	memset (buf, 0, 128);

	dc42_set_image_name (buf, img);

	pfdc_set_uint32_be (buf, 64, data_size);
	pfdc_set_uint32_be (buf, 68, tags_size);

	pfdc_set_uint32_be (buf, 72, data_check);
	pfdc_set_uint32_be (buf, 76, tags_check);

	switch (data_size) {
	case 400UL * 1024UL:
		enc = 0x00;
		gcr = 1;
		break;

	case 800UL * 1024UL:
		enc = 0x01;
		gcr = 1;
		break;

	case 720UL * 1024UL:
		enc = 0x02;
		gcr = 0;
		break;

	case 1440UL * 1024UL:
		enc = 0x03;
		gcr = 0;
		break;

	default:
		return (1);
	}

	pfdc_set_uint16_be (buf, 80, (enc << 8) | 0x22);
	pfdc_set_uint16_be (buf, 82, 0x0100);

	if (dc42_write (fp, buf, 84)) {
		return (1);
	}

	if (dc42_save_data (fp, img, gcr)) {
		return (1);
	}

	if (tags) {
		if (dc42_save_tags (fp, img, gcr)) {
			return (1);
		}
	}

	return (0);
}

int pfdc_probe_dc42_fp (FILE *fp)
{
	unsigned char buf[128];

	if (fseek (fp, 0, SEEK_SET)) {
		return (0);
	}

	if (dc42_read (fp, buf, 84)) {
		return (0);
	}

	if (pfdc_get_uint16_be (buf, 82) != DC42_MAGIC) {
		return (0);
	}

	if (buf[0] > 63) {
		return (0);
	}

	return (1);
}

int pfdc_probe_dc42 (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = pfdc_probe_dc42_fp (fp);

	fclose (fp);

	return (r);
}
