/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/pfdc-img-tc.c                              *
 * Created:     2011-09-18 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pfdc.h"
#include "pfdc-img-tc.h"


/*
 * Transcopy file format:
 *
 * 0x305  2*160  Track offsets (big endian integers, in 256 byte steps)
 * 0x505  2*160  Track lengths in bytes (little endian integers)
 */


typedef struct {
	FILE          *fp;

	unsigned char buf[65536];

	unsigned long bit_cnt;
	unsigned long bit_idx;

	unsigned      index;

	int           sync;
	int           clock;
} mfm_t;


static
void mfm_init (mfm_t *mfm, FILE *fp)
{
	mfm->fp = fp;

	mfm->bit_idx = 0;
	mfm->bit_cnt = 0;

	mfm->index = 0;

	mfm->sync = 0;
	mfm->clock = 0;
}

static
int mfm_read_bit (mfm_t *mfm)
{
	int           ret;
	unsigned long i;
	unsigned      m;

	i = mfm->bit_idx >> 3;
	m = 0x80 >> (mfm->bit_idx & 7);

	ret = (mfm->buf[i] & m) != 0;

	mfm->bit_idx += 1;

	if (mfm->bit_idx >= mfm->bit_cnt) {
		mfm->bit_idx = 0;
		mfm->index += 1;
	}

	mfm->clock = !mfm->clock;

	return (ret);
}

static
int mfm_read_byte (mfm_t *mfm, unsigned char *val)
{
	unsigned i;

	*val = 0;

	if (mfm->clock) {
		mfm_read_bit (mfm);
	}

	for (i = 0; i < 8; i++) {
		*val = *val << 1;

		if (mfm_read_bit (mfm)) {
			*val |= 0x01;
		}

		mfm_read_bit (mfm);
	}

	return (0);
}

static
unsigned mfm_read (mfm_t *mfm, void *buf, unsigned cnt)
{
	unsigned      i;
	unsigned char *dst;

	dst = buf;

	i = 0;

	while (i < cnt) {
		if (mfm_read_byte (mfm, dst + i)) {
			return (i);
		}

		i += 1;
	}

	return (i);
}

/*
 * Sync with MFM bit stream
 *
 * A1 = 10100001
 * A1 = [0] 1 [0] 0 [0] 1 [0] 0 [1] 0 [1/0] 0 [1] 0 [0] 1 [0]
 * A1 = 44A9 / 4489
 *
 * If mfm_sync() returns successfully the stream position is the clock
 * bit between the sync byte (0xa1) and the next byte.
 */
static
int mfm_sync (mfm_t *mfm)
{
	unsigned val, imax;

	mfm->sync = 0;

	imax = mfm->index + 2;

	val = 0;

	while (mfm->index < imax) {
		val = (val << 1) | (mfm_read_bit (mfm) != 0);

		if ((val & 0xffff) == 0x4489) {
			mfm->sync = 1;
			mfm->clock = 1;
			return (0);
		}
	}

	return (1);
}

static
int mfm_sync_am (mfm_t *mfm, unsigned char *am)
{
	unsigned      i;
	unsigned char val;

	if (mfm_sync (mfm)) {
		return (1);
	}

	/* skip two more sync bytes */
	for (i = 0; i < 2; i++) {
		if (mfm_read_byte (mfm, &val)) {
			return (1);
		}

		if (val != 0xa1) {
			return (1);
		}
	}

	if (mfm_read_byte (mfm, am)) {
		return (1);
	}

	return (0);
}

static
unsigned mfm_crc (unsigned crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *src;

	src = buf;

	while (cnt > 0) {
		crc ^= (unsigned) *src << 8;

		for (i = 0; i < 8; i++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ 0x1021;
			}
			else {
				crc = crc << 1;
			}
		}

		src += 1;
		cnt -= 1;
	}

	return (crc & 0xffff);
}

static
pfdc_sct_t *mfm_decode_idam (mfm_t *mfm)
{
	unsigned      c, h, s, n;
	unsigned      crc1, crc2;
	unsigned char buf[8];
	pfdc_sct_t    *sct;

	if (mfm_read (mfm, buf, 6) != 6) {
		return (NULL);
	}

	crc1 = pfdc_get_uint16_be (buf, 4);

	crc2 = mfm_crc (0xffff, "\xa1\xa1\xa1\xfe", 4);
	crc2 = mfm_crc (crc2, buf, 4);

	c = buf[0];
	h = buf[1];
	s = buf[2];
	n = buf[3];

	n = 128 << ((n < 6) ? n : 6);

	sct = pfdc_sct_new (c, h, s, n);

	if (sct == NULL) {
		return (NULL);
	}

	pfdc_sct_set_flags (sct, PFDC_FLAG_NO_DAM, 1);

	if (crc1 != crc2) {
		pfdc_sct_set_flags (sct, PFDC_FLAG_CRC_ID, 1);
	}

	pfdc_sct_fill (sct, 0);

	return (sct);
}

static
int mfm_decode_dam (mfm_t *mfm, pfdc_sct_t *sct, unsigned am)
{
	unsigned      crc1, crc2;
	unsigned char buf[4];

	if (mfm_read (mfm, sct->data, sct->n) != sct->n) {
		return (1);
	}

	if (mfm_read (mfm, buf, 2) != 2) {
		return (1);
	}

	pfdc_sct_set_flags (sct, PFDC_FLAG_NO_DAM, 0);
	pfdc_sct_set_flags (sct, PFDC_FLAG_DEL_DAM, am == 0xf8);

	crc1 = pfdc_get_uint16_be (buf, 0);

	buf[0] = 0xa1;
	buf[1] = 0xa1;
	buf[2] = 0xa1;
	buf[3] = am;

	crc2 = mfm_crc (0xffff, buf, 4);
	crc2 = mfm_crc (crc2, sct->data, sct->n);

	if (crc1 != crc2) {
		pfdc_sct_set_flags (sct, PFDC_FLAG_CRC_DATA, 1);
	}

	return (0);
}

static
int mfm_decode_am (mfm_t *mfm, pfdc_img_t *img, unsigned am, unsigned c, unsigned h)
{
	unsigned      index;
	unsigned long idx;
	unsigned char am2;
	pfdc_sct_t    *sct;

	index = mfm->index;
	idx = mfm->bit_idx;

	switch (am) {
	case 0xfe:
		sct = mfm_decode_idam (mfm);

		if (sct == NULL) {
			return (1);
		}

		pfdc_img_add_sector (img, sct, c, h);

		if (mfm_sync_am (mfm, &am2) == 0) {
			if ((am2 == 0xf8) || (am2 == 0xfb)) {
				if (mfm_decode_dam (mfm, sct, am2)) {
					return (1);
				}
			}
		}
		break;

	case 0xf8:
	case 0xfb:
		break;

	default:
		fprintf (stderr,
			"tc: unknown address mark (c=%u, h=%u, am=0x%02x)\n",
			c, h, am
		);
	}

	mfm->index = index;
	mfm->bit_idx = idx;

	return (0);
}

static
int mfm_decode_track (mfm_t *mfm, pfdc_img_t *img, unsigned c, unsigned h)
{
	unsigned char am;

	mfm->index = 0;

	while (mfm->index < 1) {
		if (mfm_sync_am (mfm, &am)) {
			continue;
		}

		if (mfm->index >= 1) {
			break;
		}

		if (mfm_decode_am (mfm, img, am, c, h)) {
			return (1);
		}
	}

	return (0);
}

static
int mfm_load_track (mfm_t *mfm, unsigned c, unsigned h)
{
	unsigned      i;
	unsigned      cnt;
	unsigned long ofs;
	unsigned char buf[2];

	mfm->bit_idx = 0;
	mfm->bit_cnt = 0;

	mfm->sync = 0;

	i = 2 * c + h;

	if (fseek (mfm->fp, 0x305 + 2 * i, SEEK_SET)) {
		return (1);
	}

	if (fread (buf, 1, 2, mfm->fp) != 2) {
		return (1);
	}

	ofs = pfdc_get_uint16_be (buf, 0);
	ofs = ofs << 8;

	if (ofs == 0) {
		return (0);
	}

	if (fseek (mfm->fp, 0x505 + 2 * i, SEEK_SET)) {
		return (1);
	}

	if (fread (buf, 1, 2, mfm->fp) != 2) {
		return (1);
	}

	cnt = pfdc_get_uint16_le (buf, 0);

	if (fseek (mfm->fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fread (mfm->buf, 1, cnt, mfm->fp) != cnt) {
		return (1);
	}

	mfm->bit_idx = 0;
	mfm->bit_cnt = 8UL * cnt;

	return (0);
}

static
int tc_load_fp (FILE *fp, pfdc_img_t *img)
{
	unsigned c, h;
	mfm_t    mfm;

	mfm_init (&mfm, fp);

	for (c = 0; c < 80; c++) {
		for (h = 0; h < 2; h++) {
			if (mfm_load_track (&mfm, c, h)) {
				return (1);
			}

			if (mfm.bit_idx >= mfm.bit_cnt) {
				continue;
			}

			if (mfm_decode_track (&mfm, img, c, h)) {
				return (1);
			}
		}
	}

	return (0);
}

pfdc_img_t *pfdc_load_tc (FILE *fp)
{
	pfdc_img_t *img;

	img = pfdc_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (tc_load_fp (fp, img)) {
		pfdc_img_del (img);
		return (NULL);
	}

	return (img);
}
