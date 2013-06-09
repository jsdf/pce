/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-tc.c                                 *
 * Created:     2011-09-18 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "psi.h"
#include "psi-io.h"
#include "psi-img-tc.h"


/*
 * Transcopy file format:
 *
 * 0x305  2*160  Track offsets (big endian integers, in 256 byte steps)
 * 0x505  2*160  Track lengths in bytes (little endian integers)
 */


typedef struct {
	FILE           *fp;

	unsigned char  buf[65536];

	unsigned long  bit_cnt;
	unsigned long  bit_idx;

	unsigned       index;

	unsigned short crc;

	char           sync;
	char           clock;
} mfm_t;


static
void mfm_init (mfm_t *mfm, FILE *fp)
{
	mfm->fp = fp;

	mfm->bit_idx = 0;
	mfm->bit_cnt = 0;

	mfm->index = 0;

	mfm->crc = 0;

	mfm->sync = 0;
	mfm->clock = 0;
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
 * Sync with MFM/FM bit stream.
 *
 * MFM mark:
 *   A1 = 10100001 / 0A = 00001010
 *   4489 = [0] 1 [0] 0 [0] 1 [0] 0 [1] 0 [0] 0 [1] 0 [0] 1
 *
 * FM index mark:
 *   FC = 11111100 / D7 = 11010111
 *   F76A = [1] 1 [1] 1 [0] 1 [1] 1 [0] 1 [1] 1 [1] 0 [1] 0
 *
 * FM ID address mark:
 *   FE = 11111110 / C7 = 11000111
 *   F57E = [1] 1 [1] 1 [0] 1 [0] 1 [0] 1 [1] 1 [1] 1 [1] 0
 *
 * FM data address mark: (FB / C7)
 *   FB = 11111011 / C7 = 11000111
 *   F56F = [1] 1 [1] 1 [0] 1 [0] 1 [0] 1 [1] 0 [1] 1 [1] 1
 *
 * FM deleted data address mark: (F8 / C7)
 *   F8 = 11111000 / C7 = 11000111
 *   F56A = [1] 1 [1] 1 [0] 1 [0] 1 [0] 1 [1] 0 [1] 0 [1] 0
 *
 * If mfm_sync() returns successfully the stream position is the clock
 * bit between the mark byte and the next byte.
 */
static
int mfm_sync (mfm_t *mfm, unsigned char *mark)
{
	unsigned val, imax;

	*mark = 0;

	mfm->sync = 0;

	imax = mfm->index + 2;

	val = 0;

	while (1) {
		val = (val << 1) | (mfm_read_bit (mfm) != 0);

		switch (val & 0xffff) {
		case 0x4489:
			*mark = 0xa1;
			break;

		case 0xf76a:
			*mark = 0xfc;
			break;

		case 0xf57e:
			*mark = 0xfe;
			break;

		case 0xf56f:
			*mark = 0xfb;
			break;

		case 0xf56a:
			*mark = 0xf8;
			break;
		}

		if (*mark != 0) {
			break;
		}

		if (mfm->index >= imax) {
			return (1);
		}
	}

	mfm->sync = 1;
	mfm->clock = 1;

	return (0);
}

/*
 * Sync with the next mark and initialize the crc.
 */
static
int mfm_sync_mark (mfm_t *mfm, unsigned char *mark, int *fm)
{
	unsigned i;

	*fm = 0;

	if (mfm_sync (mfm, mark)) {
		return (1);
	}

	mfm->crc = mfm_crc (0xffff, mark, 1);

	if (*mark != 0xa1) {
		*fm = 1;
		return (0);
	}

	/* skip two more 0xa1 mark bytes */
	for (i = 0; i < 2; i++) {
		if (mfm_read_byte (mfm, mark)) {
			return (1);
		}

		mfm->crc = mfm_crc (mfm->crc, mark, 1);

		if (*mark != 0xa1) {
			return (0);
		}
	}

	if (mfm_read_byte (mfm, mark)) {
		return (1);
	}

	mfm->crc = mfm_crc (mfm->crc, mark, 1);

	return (0);
}

static
psi_sct_t *mfm_decode_idam (mfm_t *mfm)
{
	unsigned      c, h, s, n;
	unsigned      crc;
	unsigned char buf[8];
	psi_sct_t     *sct;

	if (mfm_read (mfm, buf, 6) != 6) {
		return (NULL);
	}

	crc = psi_get_uint16_be (buf, 4);

	mfm->crc = mfm_crc (mfm->crc, buf, 4);

	c = buf[0];
	h = buf[1];
	s = buf[2];
	n = buf[3];

	n = 128 << ((n < 6) ? n : 6);

	sct = psi_sct_new (c, h, s, n);

	if (sct == NULL) {
		return (NULL);
	}

	psi_sct_set_mfm_size (sct, buf[3]);

	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 1);

	if (mfm->crc != crc) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_ID, 1);
	}

	psi_sct_fill (sct, 0);

	return (sct);
}

static
int mfm_decode_dam (mfm_t *mfm, psi_sct_t *sct, unsigned mark)
{
	unsigned      crc;
	unsigned char buf[4];

	if (mfm_read (mfm, sct->data, sct->n) != sct->n) {
		return (1);
	}

	if (mfm_read (mfm, buf, 2) != 2) {
		return (1);
	}

	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 0);
	psi_sct_set_flags (sct, PSI_FLAG_DEL_DAM, mark == 0xf8);

	crc = psi_get_uint16_be (buf, 0);

	mfm->crc = mfm_crc (mfm->crc, sct->data, sct->n);

	if (mfm->crc != crc) {
		psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, 1);
	}

	return (0);
}

static
int mfm_decode_mark (mfm_t *mfm, psi_img_t *img, unsigned mark, int fm, unsigned c, unsigned h)
{
	unsigned      index;
	unsigned long idx;
	int           fm2;
	unsigned char mark2;
	psi_sct_t     *sct;

	index = mfm->index;
	idx = mfm->bit_idx;

	switch (mark) {
	case 0xfe:
		sct = mfm_decode_idam (mfm);

		if (sct == NULL) {
			return (1);
		}

		if (fm) {
			psi_sct_set_encoding (sct, PSI_ENC_FM_DD);
		}
		else {
			psi_sct_set_encoding (sct, PSI_ENC_MFM_DD);
		}

		psi_img_add_sector (img, sct, c, h);

		if (mfm_sync_mark (mfm, &mark2, &fm2) == 0) {
			if ((mark2 == 0xf8) || (mark2 == 0xfb)) {
				if (mfm_decode_dam (mfm, sct, mark2)) {
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
			"tc: unknown mark 0x%02x"
			" (%s, c=%u, h=%u, bit=%lu/%lu)\n",
			mark, fm ? "fm" : "mfm", c, h,
			mfm->bit_idx, mfm->bit_cnt
		);
	}

	mfm->index = index;
	mfm->bit_idx = idx;

	return (0);
}

static
int mfm_decode_track (mfm_t *mfm, psi_img_t *img, unsigned c, unsigned h)
{
	unsigned      i;
	int           fm;
	unsigned char mark;

	for (i = 0; i < 2; i++) {
		mfm->index = 0;
		mfm->bit_idx = 0;

		while (mfm->index < 1) {
			if (mfm_sync_mark (mfm, &mark, &fm)) {
				continue;
			}

			if (((i == 0) && fm) || ((i == 1) && !fm)) {
				continue;
			}

			if (mfm->index >= 1) {
				break;
			}

			if (mfm_decode_mark (mfm, img, mark, fm, c, h)) {
				return (1);
			}
		}

		if (psi_img_get_sector (img, c, h, 0, 1) != NULL) {
			/* There were MFM sectors, skip FM. */
			break;
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

	ofs = psi_get_uint16_be (buf, 0);
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

	cnt = psi_get_uint16_le (buf, 0);

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
int tc_load_fp (FILE *fp, psi_img_t *img)
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

psi_img_t *psi_load_tc (FILE *fp)
{
	psi_img_t *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (tc_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}
