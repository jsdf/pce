/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-kryo.c                                   *
 * Created:     2012-01-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2017 Hampa Hug <hampa@hampa.ch>                     *
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

#include "pfi.h"
#include "pfi-io.h"
#include "pfi-kryo.h"


#define KRYOFLUX_MCLK (((18432000 * 73) / 14) / 2)
#define KRYOFLUX_SCLK (KRYOFLUX_MCLK / 2)
#define KRYOFLUX_ICLK (KRYOFLUX_MCLK / 16)


static
int kryo_load_uint8 (FILE *fp, unsigned char *val)
{
	return (pfi_read (fp, val, 1));
}

static
int kryo_load_uint16_le (FILE *fp, unsigned *val)
{
	unsigned char buf[2];

	if (pfi_read (fp, buf, 2)) {
		return (1);
	}

	*val = (buf[1] << 8) | buf[0];

	return (0);
}

static
int kryo_load_uint32_le (FILE *fp, unsigned long *val)
{
	unsigned char buf[4];

	if (pfi_read (fp, buf, 4)) {
		return (1);
	}

	*val = buf[3];
	*val = (*val << 8) | buf[2];
	*val = (*val << 8) | buf[1];
	*val = (*val << 8) | buf[0];

	return (0);
}

static
int kryo_load_oob_start (FILE *fp, unsigned cnt, unsigned long *sofs)
{
	unsigned long pos, time;

	if (cnt != 8) {
		return (1);
	}

	if (kryo_load_uint32_le (fp, &pos)) {
		return (1);
	}

	if (kryo_load_uint32_le (fp, &time)) {
		return (1);
	}

	if (*sofs != pos) {
		fprintf (stderr, "kryo: stream offset: %lu / %lu\n", *sofs, pos);
		*sofs = pos;
	}

	return (0);
}

static
int kryo_load_oob_index (FILE *fp, unsigned cnt, unsigned long spos, unsigned long *pos, unsigned long *ofs)
{
	unsigned long icnt;

	if (cnt != 12) {
		return (1);
	}

	if (kryo_load_uint32_le (fp, pos)) {
		return (1);
	}

	if (kryo_load_uint32_le (fp, ofs)) {
		return (1);
	}

	if (kryo_load_uint32_le (fp, &icnt)) {
		return (1);
	}

	return (0);
}

static
int kryo_load_oob_comment (FILE *fp, pfi_img_t *img, unsigned cnt)
{
	unsigned char c;

	while (cnt > 0) {
		if (kryo_load_uint8 (fp, &c)) {
			return (1);
		}

		if ((c == 0) || (c == 0x0d)) {
			c = 0x0a;
		}

		if (pfi_img_add_comment (img, &c, 1)) {
			return (1);
		}

		cnt -= 1;
	}

	return (0);
}

static
int kryo_load_oob (FILE *fp, pfi_img_t *img, unsigned long *spos, unsigned long *ipos, unsigned long *iofs, int *eos)
{
	unsigned      cnt;
	unsigned char type;

	if (kryo_load_uint8 (fp, &type)) {
		return (1);
	}

	if (kryo_load_uint16_le (fp, &cnt)) {
		return (1);
	}

	if (type == 1) {
		return (kryo_load_oob_start (fp, cnt, spos));
	}
	else if (type == 2) {
		return (kryo_load_oob_index (fp, cnt, *spos, ipos, iofs));
	}
	else if (type == 3) {
		*eos = 1;
	}
	else if (type == 4) {
		return (kryo_load_oob_comment (fp, img, cnt));
	}
	else if (type == 0x0d) {
		*eos = 1;
	}
	else {
		fprintf (stderr, "%08lX: oob type=%u cnt=%u\n", *spos, type, cnt);
	}

	if (pfi_skip (fp, cnt)) {
		return (1);
	}

	return (0);
}

int kryo_load_track (FILE *fp, pfi_img_t *img, pfi_trk_t *trk)
{
	int           eos;
	unsigned char v1, v2, v3;
	unsigned long spos, ipos, iofs;
	unsigned long pulse, oflow, clk;

	pfi_trk_set_clock (trk, KRYOFLUX_SCLK);

	eos = 0;

	spos = 0;
	ipos = -1;
	iofs = 0;

	pulse = 0;
	oflow = 0;
	clk = 0;

	while (eos == 0) {
		if (spos == ipos) {
			if (pfi_trk_add_index (trk, clk + iofs)) {
				return (1);
			}
		}

		if (kryo_load_uint8 (fp, &v1)) {
			return (0);
		}

		if (v1 < 8) {
			if (kryo_load_uint8 (fp, &v2)) {
				return (1);
			}

			pulse = (v1 << 8) | v2;
			spos += 2;
		}
		else if (v1 == 8) {
			spos += 1;
		}
		else if (v1 == 9) {
			if (pfi_skip (fp, 1)) {
				return (1);
			}

			spos += 2;
		}
		else if (v1 == 10) {
			if (pfi_skip (fp, 2)) {
				return (1);
			}

			spos += 3;
		}
		else if (v1 == 11) {
			oflow += 0x10000;
			spos += 1;
		}
		else if (v1 == 12) {
			if (kryo_load_uint8 (fp, &v2)) {
				return (1);
			}

			if (kryo_load_uint8 (fp, &v3)) {
				return (1);
			}

			pulse = (v2 << 8) | v3;
			spos += 3;
		}
		else if (v1 == 13) {
			if (kryo_load_oob (fp, img, &spos, &ipos, &iofs, &eos)) {
				return (1);
			}
		}
		else {
			pulse = v1;
			spos += 1;
		}

		if (pulse > 0) {
			if (pfi_trk_add_pulse (trk, pulse + oflow)) {
				return (1);
			}

			clk += pulse + oflow;

			pulse = 0;
			oflow = 0;
		}
	}

	return (0);
}

static
int kryo_load_track_ch (FILE *fp, pfi_img_t *img, unsigned long c, unsigned long h)
{
	pfi_trk_t *trk;

	trk = pfi_img_get_track (img, c, h, 1);

	if (trk == NULL) {
		return (1);
	}

	if (kryo_load_track (fp, img, trk)) {
		return (1);
	}

	return (0);
}

pfi_img_t *pfi_load_kryo (FILE *fp)
{
	pfi_img_t *img;

	img = pfi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (kryo_load_track_ch (fp, img, 0, 0)) {
		pfi_img_del (img);
		return (NULL);
	}

	return (img);
}

static
int kryo_set_name (char *str, unsigned long c, unsigned long h)
{
	int           dig, done;
	unsigned      i;
	unsigned long val;

	i = 0;
	while (str[i] != 0) {
		i += 1;
	}

	dig = 0;
	done = 0;
	val = h;

	while (i > 0) {
		i -= 1;

		if ((str[i] >= '0') && (str[i] <= '9')) {
			dig = 1;

			str[i] = '0' + (val % 10);
			val = val / 10;
		}
		else {
			if (dig) {
				if (done) {
					return (0);
				}

				val = c;
				done = 1;
			}

			dig = 0;
		}
	}

	return (0);

}

static
int kryo_load_set (pfi_img_t *img, char *name)
{
	unsigned long c, h;
	FILE          *fp;

	for (c = 0; c < 100; c++) {
		for (h = 0; h < 8; h++) {
			if (kryo_set_name (name, c, h)) {
				return (1);
			}

			if ((fp = fopen (name, "rb")) == NULL) {
				continue;
			}

			if (kryo_load_track_ch (fp, img, c, h)) {
				fclose (fp);
				return (1);
			}

			fclose (fp);
		}
	}

	return (0);
}

pfi_img_t *pfi_load_kryo_set (const char *fname)
{
	unsigned    n;
	char        *name;
	pfi_img_t *img;

	img = pfi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	n = strlen (fname);

	name = malloc (n + 1);

	if (name == NULL) {
		pfi_img_del (img);
		return (NULL);
	}

	memcpy (name, fname, n + 1);

	if (kryo_load_set (img, name)) {
		pfi_img_del (img);
		img = NULL;
	}

	free (name);

	return (img);
}


static
int kryo_save_start (FILE *fp, unsigned long pos, unsigned long time)
{
	unsigned char buf[16];

	buf[0] = 0x0d;
	buf[1] = 0x01;
	buf[2] = 8;
	buf[3] = 0;

	pfi_set_uint32_le (buf, 4, pos);
	pfi_set_uint32_le (buf, 8, time);

	if (fwrite (buf, 1, 12, fp) != 12) {
		return (1);
	}

	return (0);
}

static
int kryo_save_index (FILE *fp, unsigned long pos, unsigned long ofs, unsigned long systime)
{
	unsigned char buf[16];

	buf[0] = 0x0d;
	buf[1] = 0x02;
	buf[2] = 12;
	buf[3] = 0;

	pfi_set_uint32_le (buf, 4, pos);
	pfi_set_uint32_le (buf, 8, ofs);
	pfi_set_uint32_le (buf, 12, systime);

	if (fwrite (buf, 1, 16, fp) != 16) {
		return (1);
	}

	return (0);
}

static
int kryo_save_end (FILE *fp, unsigned long pos, unsigned long result)
{
	unsigned char buf[16];

	buf[0] = 0x0d;
	buf[1] = 0x03;
	buf[2] = 8;
	buf[3] = 0;

	pfi_set_uint32_le (buf, 4, pos);
	pfi_set_uint32_le (buf, 8, result);

	if (fwrite (buf, 1, 12, fp) != 12) {
		return (1);
	}

	return (0);
}

static
int kryo_save_track_ch (FILE *fp, pfi_trk_t *trk)
{
	unsigned           i;
	unsigned long      val, ofs;
	unsigned long      pos, clk;
	unsigned long long mul, div, tmp, rem;

	pfi_trk_rewind (trk);

	if (kryo_save_start (fp, 0, 0)) {
		return (1);
	}

	pos = 0;
	clk = 0;

	mul = KRYOFLUX_SCLK;
	div = pfi_trk_get_clock (trk);
	rem = 0;

	if (mul == div) {
		mul = 1;
		div = 1;
	}

	while (pfi_trk_get_pulse (trk, &val, &ofs) == 0) {
		if ((val == 0) || (ofs < val)) {
			ofs = (mul * ofs) / div;

			if (kryo_save_index (fp, pos, ofs, clk + ofs)) {
				return (1);
			}
		}

		if (val == 0) {
			continue;
		}

		tmp = mul * val + rem;
		val = tmp / div;
		rem = tmp % div;

		clk += val;

		while (val > 65535) {
			fputc (0x0b, fp);
			pos += 1;
			val -= 65536;
		}

		if (val <= 0x0d) {
			fputc (0, fp);
			fputc (val, fp);
			pos += 2;
		}
		else if (val <= 0xff) {
			fputc (val, fp);
			pos += 1;
		}
		else if (val <= 0x7ff) {
			fputc ((val >> 8), fp);
			fputc (val & 0xff, fp);
			pos += 2;
		}
		else {
			fputc (0x0c, fp);
			fputc (val >> 8, fp);
			fputc (val & 0xff, fp);
			pos += 3;
		}

	}

	if (kryo_save_end (fp, pos, 0)) {
		return (1);
	}

	for (i = 0; i < 8; i++) {
		fputc (0x0d, fp);
	}

	return (0);
}

static
int kryo_save_set (pfi_img_t *img, char *name)
{
	unsigned long c, h, cn, hn;
	pfi_trk_t     *trk;
	FILE          *fp;

	cn = pfi_img_get_cyl_cnt (img);

	for (c = 0; c < cn; c++) {
		hn = pfi_img_get_trk_cnt (img, c);

		for (h = 0; h < hn; h++) {
			trk = pfi_img_get_track (img, c, h, 0);

			if (trk == NULL) {
				continue;
			}

			if (kryo_set_name (name, c, h)) {
				return (1);
			}

			if ((fp = fopen (name, "wb")) == NULL) {
				continue;
			}

			if (kryo_save_track_ch (fp, trk)) {
				fclose (fp);
				return (1);
			}

			fclose (fp);
		}
	}

	return (0);
}

int pfi_save_kryo_set (const char *fname, pfi_img_t *img)
{
	unsigned n;
	char     *name;

	n = strlen (fname);

	name = malloc (n + 1);

	if (name == NULL) {
		return (1);
	}

	memcpy (name, fname, n + 1);

	if (kryo_save_set ((pfi_img_t *) img, name)) {
		free (name);
		return (1);
	}

	free (name);

	return (0);
}


int pfi_probe_kryo_fp (FILE *fp)
{
	return (0);
}

int pfi_probe_kryo (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pfi_probe_kryo_fp (fp);

	fclose (fp);

	return (r);
}
