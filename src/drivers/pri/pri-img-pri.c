/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-img-pri.c                                *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2018 Hampa Hug <hampa@hampa.ch>                     *
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
#include "pri-img-pri.h"


#define PRI_CHUNK_PRI  0x50524920
#define PRI_CHUNK_TEXT 0x54455854
#define PRI_CHUNK_TRAK 0x5452414b
#define PRI_CHUNK_DATA 0x44415441
#define PRI_CHUNK_FUZZ 0x46555a5a
#define PRI_CHUNK_BCLK 0x42434c4b
#define PRI_CHUNK_WEAK 0x5745414b
#define PRI_CHUNK_END  0x454e4420

#define PRI_CRC_POLY   0x1edc6f41


static
unsigned long pri_crc (unsigned long crc, const void *buf, unsigned cnt)
{
	unsigned             i, j;
	unsigned             val;
	unsigned long        reg;
	const unsigned char  *src;
	static int           tab_ok = 0;
	static unsigned long tab[256];

	if (tab_ok == 0) {
		for (i = 0; i < 256; i++) {
			reg = (unsigned long) i << 24;

			for (j = 0; j < 8; j++) {
				if (reg & 0x80000000) {
					reg = (reg << 1) ^ PRI_CRC_POLY;
				}
				else {
					reg = reg << 1;
				}
			}

			tab[i] = reg;
		}

		tab_ok = 1;
	}

	src = buf;

	while (cnt > 0) {
		val = (crc >> 24) ^ *(src++);
		crc = (crc << 8) ^ tab[val & 0xff];
		cnt -= 1;
	}

	return (crc & 0xffffffff);
}

static
int pri_read_crc (FILE *fp, void *buf, unsigned cnt, unsigned long *crc)
{
	if (pri_read (fp, buf, cnt)) {
		return (1);
	}

	if (crc != NULL) {
		*crc = pri_crc (*crc, buf, cnt);
	}

	return (0);
}

static
int pri_write_crc (FILE *fp, const void *buf, unsigned cnt, unsigned long *crc)
{
	if (crc != NULL) {
		*crc = pri_crc (*crc, buf, cnt);
	}

	if (pri_write (fp, buf, cnt)) {
		return (1);
	}

	return (0);
}

static
int pri_skip_chunk (FILE *fp, unsigned long size, unsigned long crc)
{
	unsigned      cnt;
	unsigned char buf[256];

	while (size > 0) {
		cnt = (size < 256) ? size : 256;

		if (pri_read_crc (fp, buf, cnt, &crc)) {
			return (1);
		}

		size -= cnt;
	}

	if (pri_read (fp, buf, 4)) {
		return (1);
	}

	if (pri_get_uint32_be (buf, 0) != crc) {
		fprintf (stderr, "pri: crc error\n");
		return (1);
	}

	return (0);
}

static
int pri_load_header (FILE *fp, pri_img_t *img, unsigned long size, unsigned long crc)
{
	unsigned char buf[4];
	unsigned      vers;

	if (size < 4) {
		return (1);
	}

	if (pri_read_crc (fp, buf, 4, &crc)) {
		return (1);
	}

	vers = pri_get_uint16_be (buf, 0);

	if (vers != 0) {
		fprintf (stderr, "pri: unknown version number (%u)\n", vers);
		return (1);
	}

	if (pri_skip_chunk (fp, size - 4, crc)) {
		return (1);
	}

	return (0);
}

static
int pri_load_text (FILE *fp, pri_img_t *img, unsigned long size, unsigned long crc)
{
	int           r;
	unsigned long i, n;
	unsigned char *buf;

	if (size == 0) {
		return (pri_skip_chunk (fp, size, crc));
	}

	if ((buf = malloc (size)) == NULL) {
		return (1);
	}

	if (pri_read_crc (fp, buf, size, &crc)) {
		free (buf);
		return (1);
	}

	i = 0;
	n = size;

	if (buf[0] == 0x0a) {
		i = 1;
		n -= 1;
	}

	if ((n > 0) && (buf[i + n - 1] == 0x0a)) {
		n -= 1;
	}

	r = pri_img_add_comment (img, buf + i, n);

	free (buf);

	r |= pri_skip_chunk (fp, 0, crc);

	return (r);
}

static
int pri_load_trak (FILE *fp, pri_img_t *img, pri_trk_t **trk, unsigned long size, unsigned long crc)
{
	unsigned long c, h, n, clock;
	unsigned char buf[16];

	if (size < 16) {
		return (1);
	}

	if (pri_read_crc (fp, buf, 16, &crc)) {
		return (1);
	}

	c = pri_get_uint32_be (buf, 0);
	h = pri_get_uint32_be (buf, 4);
	n = pri_get_uint32_be (buf, 8);
	clock = pri_get_uint32_be (buf, 12);

	if ((*trk = pri_img_get_track (img, c, h, 1)) == NULL) {
		return (1);
	}

	if (pri_trk_set_size (*trk, n)) {
		return (1);
	}

	pri_trk_set_clock (*trk, clock);

	if (pri_skip_chunk (fp, size - 16, crc)) {
		return (1);
	}

	return (0);
}

static
int pri_load_data (FILE *fp, pri_img_t *img, pri_trk_t *trk, unsigned long size, unsigned long crc)
{
	unsigned long cnt;

	if (trk == NULL) {
		return (1);
	}

	cnt = (trk->size + 7) / 8;

	if (cnt > size) {
		cnt = size;
	}

	if (pri_read_crc (fp, trk->data, cnt, &crc)) {
		return (1);
	}

	if (pri_skip_chunk (fp, size - cnt, crc)) {
		return (1);
	}

	return (0);
}

static
int pri_load_fuzz (FILE *fp, pri_img_t *img, pri_trk_t *trk, unsigned long size, unsigned long crc)
{
	unsigned long i, n;
	unsigned long pos, val;
	unsigned char buf[8];

	if (trk == NULL) {
		return (1);
	}

	n = size / 8;

	for (i = 0; i < n; i++) {
		if (pri_read_crc (fp, buf, 8, &crc)) {
			return (1);
		}

		size -= 8;

		pos = pri_get_uint32_be (buf, 0);
		val = pri_get_uint32_be (buf, 4);

		if (pri_trk_evt_add (trk, PRI_EVENT_FUZZY, pos, val) == NULL) {
			return (1);
		}
	}

	if (pri_skip_chunk (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int pri_load_bclk (FILE *fp, pri_img_t *img, pri_trk_t *trk, unsigned long size, unsigned long crc)
{
	unsigned long i, n;
	unsigned long pos, val;
	unsigned char buf[8];

	if (trk == NULL) {
		return (1);
	}

	n = size / 8;

	for (i = 0; i < n; i++) {
		if (pri_read_crc (fp, buf, 8, &crc)) {
			return (1);
		}

		size -= 8;

		pos = pri_get_uint32_be (buf, 0);
		val = pri_get_uint32_be (buf, 4);

		if (pri_trk_evt_add (trk, PRI_EVENT_CLOCK, pos, val) == NULL) {
			return (1);
		}
	}

	if (pri_skip_chunk (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int pri_load_weak (FILE *fp, pri_img_t *img, pri_trk_t *trk, unsigned long size, unsigned long crc)
{
	unsigned long i, n;
	unsigned long pos, val;
	unsigned char buf[8];

	if (trk == NULL) {
		return (1);
	}

	n = size / 8;

	for (i = 0; i < n; i++) {
		if (pri_read_crc (fp, buf, 8, &crc)) {
			return (1);
		}

		size -= 8;

		pos = pri_get_uint32_be (buf, 0);
		val = pri_get_uint32_be (buf, 4);

		if (pri_trk_evt_add (trk, PRI_EVENT_WEAK, pos, val) == NULL) {
			return (1);
		}
	}

	if (pri_skip_chunk (fp, size, crc)) {
		return (1);
	}

	return (0);
}

static
int pri_load_image (FILE *fp, pri_img_t *img)
{
	unsigned long type, size;
	unsigned long crc;
	pri_trk_t     *trk;
	unsigned char buf[8];

	crc = 0;

	if (pri_read_crc (fp, buf, 4, &crc)) {
		return (0);
	}

	type = pri_get_uint32_be (buf, 0);

	if (type != PRI_CHUNK_PRI) {
		return (1);
	}

	if (pri_read_crc (fp, buf + 4, 4, &crc)) {
		return (1);
	}

	size = pri_get_uint32_be (buf, 4);

	if (pri_load_header (fp, img, size, crc)) {
		return (1);
	}

	trk = NULL;

	while (1) {
		crc = 0;

		if (pri_read_crc (fp, buf, 8, &crc)) {
			return (1);
		}

		type = pri_get_uint32_be (buf, 0);
		size = pri_get_uint32_be (buf, 4);

		switch (type) {
		case PRI_CHUNK_END:
			if (pri_skip_chunk (fp, size, crc)) {
				return (1);
			}
			return (0);

		case PRI_CHUNK_TEXT:
			if (pri_load_text (fp, img, size, crc)) {
				return (1);
			}
			break;

		case PRI_CHUNK_TRAK:
			if (pri_load_trak (fp, img, &trk, size, crc)) {
				return (1);
			}
			break;

		case PRI_CHUNK_DATA:
			if (pri_load_data (fp, img, trk, size, crc)) {
				return (1);
			}
			break;

		case PRI_CHUNK_FUZZ:
			if (pri_load_fuzz (fp, img, trk, size, crc)) {
				return (1);
			}
			break;

		case PRI_CHUNK_BCLK:
			if (pri_load_bclk (fp, img, trk, size, crc)) {
				return (1);
			}
			break;

		case PRI_CHUNK_WEAK:
			if (pri_load_weak (fp, img, trk, size, crc)) {
				return (1);
			}
			break;

		default:
			if (pri_skip_chunk (fp, size, crc)) {
				return (1);
			}
			break;
		}
	}

	return (1);
}

pri_img_t *pri_load_pri (FILE *fp)
{
	pri_img_t *img;

	if ((img = pri_img_new()) == NULL) {
		return (NULL);
	}

	if (pri_load_image (fp, img)) {
		pri_img_del (img);
		return (NULL);
	}

	return (img);
}


static
int pri_save_chunk (FILE *fp, unsigned ckid, unsigned size, const void *data)
{
	unsigned long crc;
	unsigned char buf[8];

	pri_set_uint32_be (buf, 0, ckid);
	pri_set_uint32_be (buf, 4, size);

	crc = 0;

	if (pri_write_crc (fp, buf, 8, &crc)) {
		return (1);
	}

	if (size > 0) {
		if (pri_write_crc (fp, data, size, &crc)) {
			return (1);
		}
	}

	pri_set_uint32_be (buf, 0, crc);

	if (pri_write_crc (fp, buf, 4, NULL)) {
		return (1);
	}

	return (0);
}

static
int pri_save_header (FILE *fp, const pri_img_t *img)
{
	unsigned char buf[4];

	pri_set_uint32_be (buf, 0, 0);

	if (pri_save_chunk (fp, PRI_CHUNK_PRI, 4, buf)) {
		return (1);
	}

	return (0);
}

static
int pri_save_text (FILE *fp, const pri_img_t *img)
{
	int           r;
	unsigned long crc;
	unsigned char buf[16];

	if (img->comment_size == 0) {
		return (0);
	}

	pri_set_uint32_be (buf, 0, PRI_CHUNK_TEXT);
	pri_set_uint32_be (buf, 4, img->comment_size + 2);

	buf[8] = 0x0a;

	crc = 0;

	r = pri_write_crc (fp, buf, 9, &crc);
	r |= pri_write_crc (fp, img->comment, img->comment_size, &crc);
	r |= pri_write_crc (fp, buf + 8, 1, &crc);
	pri_set_uint32_be (buf, 0, crc);
	r |= pri_write (fp, buf, 4);

	return (r);
}

static
int pri_save_trak (FILE *fp, const pri_trk_t *trk, unsigned long c, unsigned long h)
{
	unsigned char buf[16];

	pri_set_uint32_be (buf, 0, c);
	pri_set_uint32_be (buf, 4, h);
	pri_set_uint32_be (buf, 8, trk->size);
	pri_set_uint32_be (buf, 12, trk->clock);

	if (pri_save_chunk (fp, PRI_CHUNK_TRAK, 16, buf)) {
		return (1);
	}

	return (0);
}

static
int pri_save_data (FILE *fp, const pri_trk_t *trk)
{
	unsigned long cnt, crc;
	unsigned char buf[8];

	if (trk->size == 0) {
		return (0);
	}

	crc = 0;

	cnt = (trk->size + 7) / 8;

	pri_set_uint32_be (buf, 0, PRI_CHUNK_DATA);
	pri_set_uint32_be (buf, 4, cnt);

	if (pri_write_crc (fp, buf, 8, &crc)) {
		return (1);
	}

	if (pri_write_crc (fp, trk->data, cnt, &crc)) {
		return (1);
	}

	pri_set_uint32_be (buf, 0, crc);

	if (pri_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int pri_save_weak (FILE *fp, const pri_trk_t *trk)
{
	unsigned long cnt, crc;
	pri_evt_t     *evt;
	unsigned char buf[8];

	cnt = pri_trk_evt_count (trk, PRI_EVENT_WEAK);

	if (cnt == 0) {
		return (0);
	}

	crc = 0;

	pri_set_uint32_be (buf, 0, PRI_CHUNK_WEAK);
	pri_set_uint32_be (buf, 4, 8 * cnt);

	if (pri_write_crc (fp, buf, 8, &crc)) {
		return (1);
	}

	evt = trk->evt;

	while (evt != NULL) {
		if (evt->type == PRI_EVENT_WEAK) {
			pri_set_uint32_be (buf, 0, evt->pos);
			pri_set_uint32_be (buf, 4, evt->val);

			if (pri_write_crc (fp, buf, 8, &crc)) {
				return (1);
			}
		}

		evt = evt->next;
	}

	pri_set_uint32_be (buf, 0, crc);

	if (pri_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int pri_save_bclk (FILE *fp, const pri_trk_t *trk)
{
	unsigned long cnt, crc;
	pri_evt_t     *evt;
	unsigned char buf[8];

	cnt = pri_trk_evt_count (trk, PRI_EVENT_CLOCK);

	if (cnt == 0) {
		return (0);
	}

	crc = 0;

	pri_set_uint32_be (buf, 0, PRI_CHUNK_BCLK);
	pri_set_uint32_be (buf, 4, 8 * cnt);

	if (pri_write_crc (fp, buf, 8, &crc)) {
		return (1);
	}

	evt = trk->evt;

	while (evt != NULL) {
		if (evt->type == PRI_EVENT_CLOCK) {
			pri_set_uint32_be (buf, 0, evt->pos);
			pri_set_uint32_be (buf, 4, evt->val);

			if (pri_write_crc (fp, buf, 8, &crc)) {
				return (1);
			}
		}

		evt = evt->next;
	}

	pri_set_uint32_be (buf, 0, crc);

	if (pri_write (fp, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int pri_save_track (FILE *fp, const pri_trk_t *trk, unsigned long c, unsigned long h)
{
	if (pri_save_trak (fp, trk, c, h)) {
		return (1);
	}

	if (pri_save_data (fp, trk)) {
		return (1);
	}

	if (pri_save_weak (fp, trk)) {
		return (1);
	}

	if (pri_save_bclk (fp, trk)) {
		return (1);
	}

	return (0);
}

int pri_save_pri (FILE *fp, const pri_img_t *img)
{
	unsigned long c, h;
	pri_cyl_t     *cyl;
	pri_trk_t     *trk;

	if (pri_save_header (fp, img)) {
		return (1);
	}

	if (pri_save_text (fp, img)) {
		return (1);
	}

	for (c = 0; c < img->cyl_cnt; c++) {
		if ((cyl = img->cyl[c]) == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			if ((trk = cyl->trk[h]) == NULL) {
				continue;
			}

			if (pri_save_track (fp, trk, c, h)) {
				return (1);
			}
		}
	}

	if (pri_save_chunk (fp, PRI_CHUNK_END, 0, NULL)) {
		return (1);
	}

	return (0);
}


int pri_probe_pri_fp (FILE *fp)
{
	unsigned char buf[4];

	if (pri_read_ofs (fp, 0, buf, 4)) {
		return (0);
	}

	if (pri_get_uint32_be (buf, 0) != PRI_CHUNK_PRI) {
		return (0);
	}

	return (1);
}

int pri_probe_pri (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pri_probe_pri_fp (fp);

	fclose (fp);

	return (r);
}
