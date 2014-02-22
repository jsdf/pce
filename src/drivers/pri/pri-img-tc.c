/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-img-tc.c                                 *
 * Created:     2012-02-01 by Hampa Hug <hampa@hampa.ch>                     *
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

#include "pri.h"
#include "pri-img.h"
#include "pri-img-tc.h"


static
int tc_add_comment (pri_img_t *img, const unsigned char *buf, unsigned max)
{
	unsigned i;

	if (*buf == 0) {
		return (0);
	}

	i = 0;

	while ((i < max) && (buf[i] != 0)) {
		i += 1;
	}

	if (img->comment_size > 0) {
		if (pri_img_add_comment (img, (unsigned char *) "\n", 1)) {
			return (1);
		}
	}

	if (pri_img_add_comment (img, buf, i)) {
		return (1);
	}

	return (0);
}

static
int tc_load_header (FILE *fp, pri_img_t *img, unsigned long *ofs, unsigned short *len, unsigned *c, unsigned *h)
{
	unsigned      i;
	unsigned long val;
	unsigned char buf[512];

	if (pri_read_ofs (fp, 0, buf, 512)) {
		return (1);
	}

	if (pri_get_uint16_be (buf, 0) != 0x5aa5) {
		fprintf (stderr, "pri/tc: bad magic\n");
		return (1);
	}

	*c = buf[0x0102] + 1;
	*h = buf[0x0103];

	if (tc_add_comment (img, buf + 2, 32)) {
		return (1);
	}

	if (tc_add_comment (img, buf + 34, 32)) {
		return (1);
	}

	if (pri_read_ofs (fp, 0x0305, buf, 512)) {
		return (1);
	}

	for (i = 0; i < 256; i++) {
		val = pri_get_uint16_be (buf, 2 * i);

		ofs[i] = val << 8;
	}

	if (pri_read_ofs (fp, 0x0505, buf, 512)) {
		return (1);
	}

	for (i = 0; i < 256; i++) {
		val = pri_get_uint16_le (buf, 2 * i);

		len[i] = val;
	}

	return (0);
}

static
int tc_load_track (FILE *fp, pri_img_t *img, unsigned long c, unsigned long h, unsigned long ofs, unsigned len)
{
	unsigned long cnt;
	pri_trk_t    *trk;

	trk = pri_img_get_track (img, c, h, 1);

	if (trk == NULL) {
		return (1);
	}

	if (pri_trk_set_size (trk, 8UL * len)) {
		return (1);
	}

	pri_trk_set_clock (trk, (len < 8000) ? 250000 : 500000);

	cnt = (trk->size + 7) / 8;

	if (pri_read_ofs (fp, ofs, trk->data, cnt)) {
		return (1);
	}

	return (0);
}

pri_img_t *pri_load_tc (FILE *fp)
{
	unsigned       c, h, nc, nh, t;
	unsigned long  ofs[256];
	unsigned short len[256];
	pri_img_t     *img;

	img = pri_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (tc_load_header (fp, img, ofs, len, &nc, &nh)) {
		pri_img_del (img);
		return (NULL);
	}

	if ((nc > 99) || (nh > 2)) {
		pri_img_del (img);
		return (NULL);
	}

	for (c = 0; c < nc; c++) {
		t = 2 * c;

		for (h = 0; h < nh; h++) {
			if (t > 255) {
				pri_img_del (img);
				return (NULL);
			}

			if ((ofs[t] == 0) || (len[t] == 0)) {
				t += 1;
				continue;
			}

			if (tc_load_track (fp, img, c, h, ofs[t], len[t])) {
				pri_img_del (img);
				return (NULL);
			}

			t += 1;
		}
	}

	return (img);
}


int pri_save_tc (FILE *fp, const pri_img_t *img)
{
	unsigned      i, c, h, nc, nh;
	unsigned long ofs, cnt;
	unsigned char buf[16384];
	pri_trk_t     *trk;

	nc = img->cyl_cnt;

	if (nc == 0) {
		return (1);
	}
	else if (nc > 128) {
		nc = 128;
	}

	nh = 0;

	for (c = 0; c < nc; c++) {
		if (img->cyl[c] != NULL) {
			if (img->cyl[c]->trk_cnt > nh) {
				nh = img->cyl[c]->trk_cnt;
			}
		}
	}

	if (nh == 0) {
		return (1);
	}
	else if (nh > 2) {
		nh = 2;
	}

	memset (buf, 0, sizeof (buf));

	buf[0] = 0x5a;
	buf[1] = 0xa5;

	for (i = 0; i < 95; i++) {
		buf[0x42 + 2 * i + 0] = 0x20;
		buf[0x42 + 2 * i + 1] = 0x07;
	}

	buf[0x100] = 0xff;
	buf[0x101] = 0;
	buf[0x102] = nc - 1;
	buf[0x103] = nh;
	buf[0x104] = 1;

	memset (buf + 0x105, 0x11, 512);
	memset (buf + 0x305, 0x00, 512);
	memset (buf + 0x505, 0x33, 512);
	memset (buf + 0x705, 0x44, 512);

	ofs = 16384;

	for (c = 0; c < nc; c++) {
		for (h = 0; h < 2; h++) {
			i = 2 * c + h;

			trk = pri_img_get_track ((pri_img_t *) img, c, h, 0);

			if (trk == NULL) {
				continue;
			}

			cnt = (pri_trk_get_size (trk) + 7) / 8;

			if (cnt > 65535) {
				return (1);
			}

			if ((ofs ^ (ofs + cnt)) & ~65535UL) {
				ofs = (ofs + 65535) & ~65535UL;
			}

			pri_set_uint16_le (buf, 0x105 + 2 * i, 0x0005);
			pri_set_uint16_be (buf, 0x305 + 2 * i, ofs / 256);
			pri_set_uint16_le (buf, 0x505 + 2 * i, cnt);

			buf[0x705 + 2 * i + 0] = 0x08;
			buf[0x705 + 2 * i + 1] = 0x07;

			if (trk->clock < ((500000 + 250000) / 2)) {
				buf[0x705 + 2 * i + 1] = 0x05;
			}

			if (pri_write_ofs (fp, ofs, trk->data, cnt)) {
				return (1);
			}

			ofs = (ofs + cnt + 255) & ~255UL;
		}
	}

	if (pri_write_ofs (fp, 0, buf, 16384)) {
		return (1);
	}

	return (0);
}


int pri_probe_tc_fp (FILE *fp)
{
	return (0);
}

int pri_probe_tc (const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (0);
	}

	r = pri_probe_tc_fp (fp);

	fclose (fp);

	return (r);
}
