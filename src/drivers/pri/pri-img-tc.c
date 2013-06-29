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
int tc_load_header (FILE *fp, pri_img_t *img, unsigned long *ofs, unsigned short *len, unsigned *c, unsigned *h)
{
	unsigned      i;
	unsigned long val;
	unsigned char buf[512];

	if (pri_read_ofs (fp, 0, buf, 512)) {
		return (1);
	}

	if (pri_get_uint16_be (buf, 0) != 0x5aa5) {
		return (1);
	}

	*c = buf[0x0102] + 1;
	*h = buf[0x0103];

	i = 2;
	while (i < 66) {
		if (buf[i] == 0) {
			break;
		}

		i += 1;
	}

	if (i > 2) {
		pri_img_set_comment (img, buf + 2, i - 2);
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
	return (1);
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
