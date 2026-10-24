/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-img-woz.c                                *
 * Created:     2019-06-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2019 Hampa Hug <hampa@hampa.ch>                          *
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


#include <config.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pri.h"
#include "pri-img.h"
#include "pri-img-woz.h"


#ifndef DEBUG_WOZ
#define DEBUG_WOZ 0
#endif

#define WOZ_MAGIC_WOZ2 0x325a4f57
#define WOZ_MAGIC_2    0x0a0d0aff
#define WOZ_MAGIC_INFO 0x4f464e49
#define WOZ_MAGIC_TMAP 0x50414d54
#define WOZ_MAGIC_TRKS 0x534b5254
#define WOZ_MAGIC_WRIT 0x54495257
#define WOZ_MAGIC_META 0x4154454d


typedef struct {
	FILE          *fp;
	unsigned long ofs;

	pri_img_t     *img;

	char          have_info;
	char          have_tmap;

	unsigned char info[64];
	unsigned char tmap[160];
	unsigned char trkh[8 * 160];
	char          creator[64];

	unsigned long clock;

	unsigned long crc;
} woz_load_t;


typedef struct {
	FILE            *fp;
	const pri_img_t *img;

	unsigned long   ofs;

	unsigned char   disk_type;
	unsigned char   sides;
	unsigned char   bit_timing;
	unsigned long   largest_track;

	pri_trk_t       *track[160];
	unsigned long   track_size[160];
} woz_save_t;


static
unsigned long woz_crc (unsigned long crc, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *p;

	p = buf;

	while (cnt > 0) {
		crc ^= *(p++);

		for (i = 0; i < 8; i++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ 0xedb88320;
			}
			else {
				crc = crc >> 1;
			}
		}

		cnt -= 1;
	}

	return (crc);
}

static
void woz_get_string (char *dst, const unsigned char *src, unsigned max)
{
	unsigned i;

	for (i = 0; i < max; i++) {
		if ((dst[i] = src[i]) == 0) {
			break;
		}
	}

	while ((i > 0) && (dst[i - 1] == 0x20)) {
		i -= 1;
	}

	dst[i] = 0;
}

static
int woz_set_pos (woz_load_t *woz, unsigned long ofs)
{
	if (pri_set_ofs (woz->fp, ofs)) {
		return (1);
	}

	woz->ofs = ofs;

	return (0);
}

static
int woz_read (woz_load_t *woz, void *buf, unsigned long cnt)
{
	if (pri_read (woz->fp, buf, cnt)) {
		return (1);
	}

	woz->ofs += cnt;

	return (0);
}

static
int woz_skip (woz_load_t *woz, unsigned long cnt)
{
	if (pri_skip (woz->fp, cnt)) {
		return (1);
	}

	woz->ofs += cnt;

	return (0);
}

static
int woz_load_crc (woz_load_t *woz, unsigned long *val)
{
	unsigned      n;
	unsigned long crc;
	unsigned char buf[256];

	crc = ~0UL;

	while ((n = fread (buf, 1, 256, woz->fp)) > 0) {
		crc = woz_crc (crc, buf, n);
	}

	*val = ~crc & 0xffffffff;

	return (0);
}

static
int woz_load_header (woz_load_t *woz)
{
	unsigned long crc;
	unsigned char buf[12];

	if (woz_read (woz, buf, 12)) {
		return (1);
	}

	if (pri_get_uint32_le (buf, 0) != WOZ_MAGIC_WOZ2) {
		fprintf (stderr, "woz: bad magic 1\n");
		return (1);
	}

	if (pri_get_uint32_le (buf, 4) != WOZ_MAGIC_2) {
		fprintf (stderr, "woz: bad magic 2\n");
		return (1);
	}

	woz->crc = pri_get_uint32_le (buf, 8);

	if (woz_load_crc (woz, &crc)) {
		return (1);
	}

	if (woz->crc != crc) {
		fprintf (stderr, "woz: crc error\n");
		return (1);
	}

	if (woz_set_pos (woz, 12)) {
		return (1);
	}

	return (0);
}

static
int woz_load_meta (woz_load_t *woz, unsigned long size)
{
	unsigned      n;
	unsigned char buf[256];

	while (size > 0) {
		n = (size < 256) ? size : 256;

		if (woz_read (woz, buf, n)) {
			return (1);
		}

		if (pri_img_add_comment (woz->img, buf, n)) {
			return (1);
		}

		size -= n;
	}

	return (0);
}

static
int woz_load_writ (woz_load_t *woz, unsigned long size)
{
	if (woz_skip (woz, size)) {
		return (1);
	}

	return (0);
}

static
int woz_load_info (woz_load_t *woz, unsigned long size)
{
	unsigned char *p;

	if (size < 60) {
		return (1);
	}

	if (woz_read (woz, woz->info, 60)) {
		return (1);
	}

	p = woz->info;

	if (p[0] != 2) {
		fprintf (stderr, "woz: bad info version (%u)\n", p[0]);
		return (1);
	}

	if (p[39] > 0) {
		woz->clock = 8000000 / p[39];
	}
	else {
		woz->clock = 500000;
	}

	woz->img->readonly = (p[2] != 0);
	woz->img->woz_track_sync = (p[3] != 0);
	woz->img->woz_cleaned = (p[4] != 0);

	woz_get_string (woz->creator, p + 5, 32);

#if DEBUG_WOZ >= 1
	fprintf (stderr, "woz: disk type       = %u\n", p[1]);
	fprintf (stderr, "woz: write protected = %u\n", p[2]);
	fprintf (stderr, "woz: synchronized    = %u\n", p[3]);
	fprintf (stderr, "woz: cleaned         = %u\n", p[4]);
	fprintf (stderr, "woz: creator         = '%s'\n", woz->creator);

	fprintf (stderr, "woz: disk sides      = %u\n", p[37]);
	fprintf (stderr, "woz: boot format     = %u\n", p[38]);
	fprintf (stderr, "woz: bit timing      = %u\n", p[39]);
	fprintf (stderr, "woz: bit clock       = %lu\n", woz->clock);
#endif

	if (woz_skip (woz, size - 60)) {
		return (1);
	}

	return (0);
}

static
int woz_load_tmap (woz_load_t *woz, unsigned long size)
{
	if (size < 160) {
		return (1);
	}

	if (woz_read (woz, woz->tmap, 160)) {
		return (1);
	}

	if (woz_skip (woz, size - 160)) {
		return (1);
	}

	return (0);
}

static
int woz_load_trks (woz_load_t *woz, unsigned long size)
{
	unsigned      i;
	unsigned      c, h;
	unsigned      idx;
	unsigned long ofs, bit, cnt;
	unsigned long next;
	pri_trk_t     *trk;

	next = woz->ofs + size;

	if (size < (8 * 160)) {
		return (1);
	}

	if (woz_read (woz, woz->trkh, 8 * 160)) {
		return (1);
	}

	for (i = 0; i < 160; i++) {
		idx = woz->tmap[i];

		if (idx == 0xff) {
			continue;
		}
		else if (idx >= 160) {
			return (1);
		}

		c = i >> 1;
		h = i & 1;

		ofs = pri_get_uint16_le (woz->trkh, 8 * idx);
		bit = pri_get_uint32_le (woz->trkh, 8 * idx + 4);

		ofs *= 512;
		cnt = (bit + 7) / 8;

#if DEBUG_WOZ >= 2
		fprintf (stderr, "woz: %u/%u %06lX+%04lX\n", c, h, ofs, cnt);
#endif

		if ((trk = pri_img_get_track (woz->img, c, h, 1)) == NULL) {
			return (1);
		}

		pri_trk_set_clock (trk, woz->clock);

		if (pri_trk_set_size (trk, bit)) {
			return (1);
		}

		if (pri_read_ofs (woz->fp, ofs, trk->data, cnt)) {
			return (1);
		}
	}

	if (woz_set_pos (woz, next)) {
		return (1);
	}

	return (0);
}

static
int woz_load_img (woz_load_t *woz)
{
	unsigned long type, size;
	unsigned char buf[8];

	if (woz_load_header (woz)) {
		return (1);
	}

	while (woz_read (woz, buf, 8) == 0) {
		type = pri_get_uint32_le (buf, 0);
		size = pri_get_uint32_le (buf, 4);

#if DEBUG_WOZ >= 2
		fprintf (stderr, "woz: %08lX: type=%08lX size=%08lX\n",
			woz->ofs - 8, type, size
		);
#endif

		if (type == WOZ_MAGIC_INFO) {
			if (woz_load_info (woz, size)) {
				return (1);
			}

			woz->have_info = 1;
		}
		else if (type == WOZ_MAGIC_TMAP) {
			if (woz_load_tmap (woz, size)) {
				return (1);
			}

			woz->have_tmap = 1;
		}
		else if (type == WOZ_MAGIC_TRKS) {
			if ((woz->have_info == 0) || (woz->have_tmap == 0)) {
				return (1);
			}

			if (woz_load_trks (woz, size)) {
				return (1);
			}
		}
		else if (type == WOZ_MAGIC_WRIT) {
			if (woz_load_writ (woz, size)) {
				return (1);
			}
		}
		else if (type == WOZ_MAGIC_META) {
			if (woz_load_meta (woz, size)) {
				return (1);
			}
		}
		else {
			fprintf (stderr,
				"woz: warning: unknown chunk type (%08lX)\n",
				type
			);

			if (woz_skip (woz, size)) {
				return (1);
			}
		}
	}

	return (0);
}

pri_img_t *pri_load_woz (FILE *fp)
{
	woz_load_t woz;

	woz.fp = fp;
	woz.ofs = 0;
	woz.img = NULL;

	woz.have_info = 0;
	woz.have_tmap = 0;
	woz.creator[0] = 0;
	woz.clock = 0;
	woz.crc = 0;

	if ((woz.img = pri_img_new()) == NULL) {
		return (NULL);
	}

	if (woz_load_img (&woz)) {
		pri_img_del (woz.img);
		return (NULL);
	}

	return (woz.img);
}


static
int woz_set_string (unsigned char *dst, const char *src, unsigned max)
{
	unsigned i;

	for (i = 0; i < max; i++) {
		if (*src == 0) {
			dst[i] = 0x20;
		}
		else {
			dst[i] = *(src++);
		}
	}

	return (*src != 0);
}

static
unsigned woz_get_disk_sides (const pri_img_t *img)
{
	unsigned c;

	for (c = 0; c < 80; c++) {
		if (pri_img_get_track_const (img, c, 1) != NULL) {
			return (2);
		}
	}

	return (1);
}

static
int woz_save_init (woz_save_t *woz, FILE *fp, const pri_img_t *img)
{
	unsigned  i, c, h;
	unsigned  timing;
	pri_trk_t *trk;

	woz->fp = fp;
	woz->img = img;
	woz->ofs = 0;

	woz->sides = woz_get_disk_sides (img);

	if ((woz->sides > 1) || (img->cyl_cnt < 90)) {
		woz->disk_type = 2;
	}
	else {
		woz->disk_type = 1;
	}

	woz->bit_timing = 0;
	woz->largest_track = 0;

	i = 0;

	for (i = 0; i < 160; i++) {
		if (woz->disk_type == 2) {
			c = i >> 1;
			h = i & 1;
		}
		else {
			c = i;
			h = 0;
		}

		trk = pri_img_get_track_const (woz->img, c, h);

		woz->track[i] = trk;

		if (trk == NULL) {
			woz->track_size[i] = 0;
			timing = 0;
		}
		else {
			woz->track_size[i] = trk->size;
			timing = 8000000 / trk->clock;
		}

		if (woz->track_size[i] > woz->largest_track) {
			woz->largest_track= woz->track_size[i];
		}

		if (timing > woz->bit_timing) {
			woz->bit_timing = timing;
		}
	}

	return (1);
}

static
int woz_save_header (woz_save_t *woz)
{
	unsigned char buf[12];

	pri_set_uint32_le (buf, 0, WOZ_MAGIC_WOZ2);
	pri_set_uint32_le (buf, 4, WOZ_MAGIC_2);
	pri_set_uint32_le (buf, 8, 0);

	if (pri_write_ofs (woz->fp, 0, buf, 12)) {
		return (1);
	}

	woz->ofs = 12;

	return (0);
}

static
int woz_save_crc (woz_save_t *woz)
{
	unsigned      n;
	unsigned long crc;
	unsigned long size;
	unsigned char buf[256];

	if (pri_read_ofs (woz->fp, 12, buf, 244)) {
		return (1);
	}

	crc = woz_crc (~0UL, buf, 244);

	size = woz->ofs - 256;

	while (size > 0) {
		n = (size < 256) ? size : 256;

		if (pri_read (woz->fp, buf, n)) {
			return (1);
		}

		crc = woz_crc (crc, buf, n);

		size -= n;
	}

	pri_set_uint32_le (buf, 0, ~crc);

	if (pri_write_ofs (woz->fp, 8, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int woz_save_info (woz_save_t *woz)
{
	unsigned char buf[68];
	char          str[256];

	memset (buf, 0, 68);

	pri_set_uint32_le (buf, 0, WOZ_MAGIC_INFO);
	pri_set_uint32_le (buf, 4, 60);

	buf[8] = 2;	/* version */
	buf[9] = woz->disk_type;
	buf[10] = (woz->img->readonly != 0);
	buf[11] = (woz->img->woz_track_sync != 0);
	buf[12] = (woz->img->woz_cleaned != 0);

	strcpy (str, "pce-");
	strcat (str, PCE_VERSION_STR);

	woz_set_string (buf + 13, str, 32);

	buf[45] = woz->sides;
	buf[46] = 0;	/* boot sector format */
	buf[47] = woz->bit_timing;

	pri_set_uint16_le (buf, 48, 0);	/* compatible hardware */
	pri_set_uint16_le (buf, 50, 0);	/* required ram */
	pri_set_uint16_le (buf, 52, (woz->largest_track + 4095) / 4096);

	if (pri_write_ofs (woz->fp, woz->ofs, buf, 68)) {
		return (1);
	}

	woz->ofs += 68;

	return (0);
}

static
int woz_save_tmap (woz_save_t *woz)
{
	unsigned      i, n;
	unsigned char buf[168];

	pri_set_uint32_le (buf, 0, WOZ_MAGIC_TMAP);
	pri_set_uint32_le (buf, 4, 160);

	n = 0;

	for (i = 0; i < 160; i++) {
		if (woz->track_size[i] == 0) {
			buf[i + 8] = 0xff;
		}
		else {
			buf[i + 8] = n++;
		}
	}

	if (pri_write_ofs (woz->fp, woz->ofs, buf, 168)) {
		return (1);
	}

	woz->ofs += 168;

	return (0);
}

static
int woz_save_trks (woz_save_t *woz)
{
	unsigned      i, n;
	unsigned long bytes;
	unsigned long ofs;
	pri_trk_t     *trk;
	unsigned char buf[1288];
	unsigned char zero;

	memset (buf, 0, sizeof (buf));

	pri_set_uint32_le (buf, 0, WOZ_MAGIC_TRKS);

	ofs = (woz->ofs + 1288 + 511) & ~511UL;

	n = 8;

	for (i = 0; i < 160; i++) {
		if (woz->track_size[i] == 0) {
			continue;
		}

		trk = woz->track[i];
		bytes = (trk->size + 7) / 8;

		pri_set_uint16_le (buf, n + 0, (ofs + 511) / 512);
		pri_set_uint16_le (buf, n + 2, (bytes + 511) / 512);
		pri_set_uint32_le (buf, n + 4, trk->size);

		n += 8;

		if (pri_write_ofs (woz->fp, ofs, trk->data, bytes)) {
			return (1);
		}

		ofs = (ofs + bytes + 511) & ~511UL;

		if (bytes & 511) {
			zero = 0;

			if (pri_write_ofs (woz->fp, ofs - 1, &zero, 1)) {
				return (1);
			}
		}
	}

	pri_set_uint32_le (buf, 4, ofs - woz->ofs - 8);

	if (pri_write_ofs (woz->fp, woz->ofs, buf, 1288)) {
		return (1);
	}

	woz->ofs = ofs;

	return (0);
}

static
int woz_save_meta (woz_save_t *woz)
{
	unsigned long size;
	unsigned char buf[8];

	size = woz->img->comment_size;

	if (size == 0) {
		return (0);
	}

	pri_set_uint32_le (buf, 0, WOZ_MAGIC_META);
	pri_set_uint32_le (buf, 4, size);

	if (pri_write_ofs (woz->fp, woz->ofs, buf, 8)) {
		return (1);
	}

	if (pri_write (woz->fp, woz->img->comment, size)) {
		return (1);
	}

	woz->ofs += 8 + size;

	return (0);
}

static
int woz_save_img (woz_save_t *woz)
{
	if (woz_save_header (woz)) {
		return (1);
	}

	if (woz_save_info (woz)) {
		return (1);
	}

	if (woz_save_tmap (woz)) {
		return (1);
	}

	if (woz_save_trks (woz)) {
		return (1);
	}

	if (woz_save_meta (woz)) {
		return (1);
	}

	if (woz_save_crc (woz)) {
		return (1);
	}

	return (0);
}

int pri_save_woz (FILE *fp, const pri_img_t *img)
{
	woz_save_t woz;

	woz_save_init (&woz, fp, img);

	if (woz_save_img (&woz)) {
		return (1);
	}

	return (0);
}


int pri_probe_woz_fp (FILE *fp)
{
	unsigned char buf[8];

	if (pri_read_ofs (fp, 0, buf, 8)) {
		return (0);
	}

	if (pri_get_uint32_le (buf, 0) != WOZ_MAGIC_WOZ2) {
		return (0);
	}

	if (pri_get_uint32_le (buf, 4) != WOZ_MAGIC_2) {
		return (0);
	}

	return (1);
}

int pri_probe_woz (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pri_probe_woz_fp (fp);

	fclose (fp);

	return (r);
}
