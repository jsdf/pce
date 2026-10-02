/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-img-moof.c                               *
 * Created:     2022-10-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2023 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <config.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pri.h"
#include "pri-img.h"
#include "pri-img-moof.h"


#ifndef DEBUG_MOOF
#define DEBUG_MOOF 0
#endif

#define MOOF_MAGIC_MOOF 0x464f4f4d
#define MOOF_MAGIC_2    0x0a0d0aff
#define MOOF_MAGIC_INFO 0x4f464e49
#define MOOF_MAGIC_TMAP 0x50414d54
#define MOOF_MAGIC_TRKS 0x534b5254
#define MOOF_MAGIC_META 0x4154454d
#define MOOF_MAGIC_FLUX 0x58554c46


typedef struct {
	FILE           *fp;
	unsigned long  ofs;

	pri_img_t      *img;

	char           have_info;
	char           have_tmap;

	unsigned char  info[64];
	unsigned char  tmap[160];
	unsigned char  trkh[8 * 160];
	char           creator[64];

	unsigned short flux_block;
	unsigned short flux_size;

	unsigned long  clock;

	unsigned long  crc;
} moof_load_t;


typedef struct {
	FILE            *fp;
	const pri_img_t *img;

	unsigned long   ofs;

	unsigned char   disk_type;
	unsigned char   sides;
	unsigned long   clock;
	unsigned long   largest_track;

	pri_trk_t       *track[160];
	unsigned long   track_size[160];
} moof_save_t;


static
unsigned long moof_crc (unsigned long crc, const void *buf, unsigned cnt)
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
void moof_get_string (char *dst, const unsigned char *src, unsigned max)
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
int moof_set_pos (moof_load_t *moof, unsigned long ofs)
{
	if (pri_set_ofs (moof->fp, ofs)) {
		return (1);
	}

	moof->ofs = ofs;

	return (0);
}

static
int moof_read (moof_load_t *moof, void *buf, unsigned long cnt)
{
	if (pri_read (moof->fp, buf, cnt)) {
		return (1);
	}

	moof->ofs += cnt;

	return (0);
}

static
int moof_skip (moof_load_t *moof, unsigned long cnt)
{
	if (pri_skip (moof->fp, cnt)) {
		return (1);
	}

	moof->ofs += cnt;

	return (0);
}

static
int moof_load_crc (moof_load_t *moof, unsigned long *val)
{
	unsigned      n;
	unsigned long crc;
	unsigned char buf[256];

	crc = ~0UL & 0xffffffff;

	while ((n = fread (buf, 1, 256, moof->fp)) > 0) {
		crc = moof_crc (crc, buf, n);
	}

	*val = ~crc & 0xffffffff;

	return (0);
}

static
int moof_load_header (moof_load_t *moof)
{
	unsigned long crc;
	unsigned char buf[12];

	if (moof_read (moof, buf, 12)) {
		return (1);
	}

	if (pri_get_uint32_le (buf, 0) != MOOF_MAGIC_MOOF) {
		fprintf (stderr, "moof: bad magic 1\n");
		return (1);
	}

	if (pri_get_uint32_le (buf, 4) != MOOF_MAGIC_2) {
		fprintf (stderr, "moof: bad magic 2\n");
		return (1);
	}

	moof->crc = pri_get_uint32_le (buf, 8);

	if (moof->crc != 0) {
		if (moof_load_crc (moof, &crc)) {
			return (1);
		}

		if (moof->crc != crc) {
			fprintf (stderr, "moof: crc error (%08lX / %08lX)\n", moof->crc, crc);
			return (1);
		}
	}

	if (moof_set_pos (moof, 12)) {
		return (1);
	}

	return (0);
}

static
int moof_load_meta (moof_load_t *moof, unsigned long size)
{
	unsigned      n;
	unsigned char buf[256];

	while (size > 0) {
		n = (size < 256) ? size : 256;

		if (moof_read (moof, buf, n)) {
			return (1);
		}

		if (pri_img_add_comment (moof->img, buf, n)) {
			return (1);
		}

		size -= n;
	}

	return (0);
}

static
int moof_load_info (moof_load_t *moof, unsigned long size)
{
	unsigned char *p;

	if (size < 60) {
		return (1);
	}

	if (moof_read (moof, moof->info, 60)) {
		return (1);
	}

	p = moof->info;

	if (p[0] != 1) {
		fprintf (stderr, "moof: unsupported info version (%u)\n", p[0]);
		return (1);
	}

	if (p[4] == 0) {
		fprintf (stderr, "moof: bad bit clock (%u)\n", p[4]);
		return (1);
	}

	moof->clock = 8000000 / p[4];

	moof->img->readonly = (p[2] != 0);
	moof->img->woz_track_sync = (p[3] != 0);

	moof_get_string (moof->creator, p + 5, 32);

	moof->flux_block = pri_get_uint16_le (p, 40);
	moof->flux_size = pri_get_uint16_le (p, 44);

	if (moof->flux_block != 0) {
		fprintf (stderr, "moof: ignoring flux data\n");
	}

#if DEBUG_MOOF >= 1
	fprintf (stderr, "moof: disk type       = %u\n", p[1]);
	fprintf (stderr, "moof: write protected = %u\n", p[2]);
	fprintf (stderr, "moof: synchronized    = %u\n", p[3]);
	fprintf (stderr, "moof: creator         = '%s'\n", moof->creator);
	fprintf (stderr, "moof: bit clock       = %lu\n", moof->clock);
#endif

	if (moof_skip (moof, size - 60)) {
		return (1);
	}

	return (0);
}

static
int moof_load_tmap (moof_load_t *moof, unsigned long size)
{
	if (size < 160) {
		return (1);
	}

	if (moof_read (moof, moof->tmap, 160)) {
		return (1);
	}

	if (moof_skip (moof, size - 160)) {
		return (1);
	}

	return (0);
}

static
int moof_load_trks (moof_load_t *moof, unsigned long size)
{
	unsigned      i;
	unsigned      c, h;
	unsigned      idx;
	unsigned long ofs, bit, cnt;
	unsigned long next;
	pri_trk_t     *trk;

	next = moof->ofs + size;

	if (size < (8 * 160)) {
		return (1);
	}

	if (moof_read (moof, moof->trkh, 8 * 160)) {
		return (1);
	}

	for (i = 0; i < 160; i++) {
		idx = moof->tmap[i];

		if (idx == 0xff) {
			continue;
		}
		else if (idx >= 160) {
			return (1);
		}

		c = i >> 1;
		h = i & 1;

		ofs = pri_get_uint16_le (moof->trkh, 8 * idx);
		bit = pri_get_uint32_le (moof->trkh, 8 * idx + 4);

		ofs *= 512;
		cnt = (bit + 7) / 8;

#if DEBUG_MOOF >= 2
		fprintf (stderr, "moof: %u/%u %06lX+%04lX\n", c, h, ofs, cnt);
#endif

		if ((trk = pri_img_get_track (moof->img, c, h, 1)) == NULL) {
			return (1);
		}

		pri_trk_set_clock (trk, moof->clock);

		if (pri_trk_set_size (trk, bit)) {
			return (1);
		}

		if (pri_read_ofs (moof->fp, ofs, trk->data, cnt)) {
			return (1);
		}
	}

	if (moof_set_pos (moof, next)) {
		return (1);
	}

	return (0);
}

static
int moof_load_img (moof_load_t *moof)
{
	unsigned long type, size;
	unsigned char buf[8];

	if (moof_load_header (moof)) {
		return (1);
	}

	while (moof_read (moof, buf, 8) == 0) {
		type = pri_get_uint32_le (buf, 0);
		size = pri_get_uint32_le (buf, 4);

#if DEBUG_MOOF >= 2
		fprintf (stderr, "moof: %08lX: type=%08lX size=%08lX\n",
			moof->ofs - 8, type, size
		);
#endif

		if (type == MOOF_MAGIC_INFO) {
			if (moof_load_info (moof, size)) {
				return (1);
			}

			moof->have_info = 1;
		}
		else if (type == MOOF_MAGIC_TMAP) {
			if (moof_load_tmap (moof, size)) {
				return (1);
			}

			moof->have_tmap = 1;
		}
		else if (type == MOOF_MAGIC_FLUX) {
			fprintf (stderr, "moof: ignoring FLUX chunk\n");

			if (moof_skip (moof, size)) {
				return (1);
			}
		}
		else if (type == MOOF_MAGIC_TRKS) {
			if ((moof->have_info == 0) || (moof->have_tmap == 0)) {
				return (1);
			}

			if (moof_load_trks (moof, size)) {
				return (1);
			}
		}
		else if (type == MOOF_MAGIC_META) {
			if (moof_load_meta (moof, size)) {
				return (1);
			}
		}
		else if ((type == 0) && (size == 0)) {
			break;
		}
		else {
			fprintf (stderr,
				"moof: unknown chunk type (%08lX / %08lX)\n",
				type, size
			);

			if (moof_skip (moof, size)) {
				return (1);
			}
		}
	}

	return (0);
}

pri_img_t *pri_load_moof (FILE *fp)
{
	moof_load_t moof;

	memset (&moof, 0, sizeof (moof_load_t));

	moof.fp = fp;

	if ((moof.img = pri_img_new()) == NULL) {
		return (NULL);
	}

	if (moof_load_img (&moof)) {
		pri_img_del (moof.img);
		return (NULL);
	}

	return (moof.img);
}


static
int moof_set_string (unsigned char *dst, const char *src, unsigned max)
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
int moof_save_init (moof_save_t *moof, FILE *fp, const pri_img_t *img)
{
	unsigned  i;
	pri_trk_t *trk;

	memset (moof, 0, sizeof (moof_save_t));

	moof->fp = fp;
	moof->img = img;
	moof->sides = 1;

	for (i = 0; i < 160; i++) {
		trk = pri_img_get_track_const (moof->img, i >> 1, i & 1);

		if (trk == NULL) {
			continue;
		}

		moof->track[i] = trk;
		moof->track_size[i] = trk->size;

		if (i & 1) {
			moof->sides = 2;
		}

		if (moof->track_size[i] > moof->largest_track) {
			moof->largest_track = moof->track_size[i];
		}

		if (trk->clock > moof->clock) {
			moof->clock = trk->clock;
		}
	}

	if (moof->clock == 0) {
		moof->clock = 500000;
	}

	if (moof->sides < 2) {
		moof->disk_type = 1;
	}
	else if (moof->clock > 750000) {
		moof->disk_type = 3;
	}
	else {
		moof->disk_type = 2;
	}

	return (1);
}

static
int moof_save_header (moof_save_t *moof)
{
	unsigned char buf[12];

	pri_set_uint32_le (buf, 0, MOOF_MAGIC_MOOF);
	pri_set_uint32_le (buf, 4, MOOF_MAGIC_2);
	pri_set_uint32_le (buf, 8, 0);

	if (pri_write_ofs (moof->fp, 0, buf, 12)) {
		return (1);
	}

	moof->ofs = 12;

	return (0);
}

static
int moof_save_crc (moof_save_t *moof)
{
	unsigned      n;
	unsigned long crc;
	unsigned long size;
	unsigned char buf[256];

	if (pri_read_ofs (moof->fp, 12, buf, 244)) {
		return (1);
	}

	crc = moof_crc (~0UL, buf, 244);

	size = moof->ofs - 256;

	while (size > 0) {
		n = (size < 256) ? size : 256;

		if (pri_read (moof->fp, buf, n)) {
			return (1);
		}

		crc = moof_crc (crc, buf, n);

		size -= n;
	}

	pri_set_uint32_le (buf, 0, ~crc);

	if (pri_write_ofs (moof->fp, 8, buf, 4)) {
		return (1);
	}

	return (0);
}

static
int moof_save_info (moof_save_t *moof)
{
	unsigned char buf[68];
	char          str[256];

	memset (buf, 0, 68);

	pri_set_uint32_le (buf, 0, MOOF_MAGIC_INFO);
	pri_set_uint32_le (buf, 4, 60);

	buf[8] = 1;	/* version */
	buf[9] = moof->disk_type;
	buf[10] = (moof->img->readonly != 0);
	buf[11] = (moof->img->woz_track_sync != 0);
	buf[12] = 8000000 / moof->clock;

	strcpy (str, "pce-" PCE_VERSION_STR);

	moof_set_string (buf + 13, str, 32);

	pri_set_uint16_le (buf, 46, (moof->largest_track + 4095) / 4096);

	if (pri_write_ofs (moof->fp, moof->ofs, buf, 68)) {
		return (1);
	}

	moof->ofs += 68;

	return (0);
}

static
int moof_save_tmap (moof_save_t *moof)
{
	unsigned      i, n;
	unsigned char buf[168];

	pri_set_uint32_le (buf, 0, MOOF_MAGIC_TMAP);
	pri_set_uint32_le (buf, 4, 160);

	n = 0;

	for (i = 0; i < 160; i++) {
		if (moof->track_size[i] == 0) {
			buf[i + 8] = 0xff;
		}
		else {
			buf[i + 8] = n++;
		}
	}

	if (pri_write_ofs (moof->fp, moof->ofs, buf, 168)) {
		return (1);
	}

	moof->ofs += 168;

	return (0);
}

static
int moof_save_trks (moof_save_t *moof)
{
	unsigned      i, n;
	unsigned long bytes;
	unsigned long ofs;
	pri_trk_t     *trk;
	unsigned char buf[1288];
	unsigned char zero[1];

	memset (buf, 0, sizeof (buf));

	pri_set_uint32_le (buf, 0, MOOF_MAGIC_TRKS);

	ofs = (moof->ofs + 1288 + 511) & ~511UL;

	n = 8;

	for (i = 0; i < 160; i++) {
		if (moof->track_size[i] == 0) {
			continue;
		}

		trk = moof->track[i];
		bytes = (trk->size + 7) / 8;

		pri_set_uint16_le (buf, n + 0, (ofs + 511) / 512);
		pri_set_uint16_le (buf, n + 2, (bytes + 511) / 512);
		pri_set_uint32_le (buf, n + 4, trk->size);

		n += 8;

		if (pri_write_ofs (moof->fp, ofs, trk->data, bytes)) {
			return (1);
		}

		ofs = (ofs + bytes + 511) & ~511UL;

		if (bytes & 511) {
			zero[0] = 0;

			if (pri_write_ofs (moof->fp, ofs - 1, zero, 1)) {
				return (1);
			}
		}
	}

	pri_set_uint32_le (buf, 4, ofs - moof->ofs - 8);

	if (pri_write_ofs (moof->fp, moof->ofs, buf, 1288)) {
		return (1);
	}

	moof->ofs = ofs;

	return (0);
}

static
int moof_save_meta (moof_save_t *moof)
{
	unsigned long size;
	unsigned char buf[8];

	size = moof->img->comment_size;

	if (size == 0) {
		return (0);
	}

	pri_set_uint32_le (buf, 0, MOOF_MAGIC_META);
	pri_set_uint32_le (buf, 4, size);

	if (pri_write_ofs (moof->fp, moof->ofs, buf, 8)) {
		return (1);
	}

	if (pri_write (moof->fp, moof->img->comment, size)) {
		return (1);
	}

	moof->ofs += 8 + size;

	return (0);
}

static
int moof_save_img (moof_save_t *moof)
{
	if (moof_save_header (moof)) {
		return (1);
	}

	if (moof_save_info (moof)) {
		return (1);
	}

	if (moof_save_tmap (moof)) {
		return (1);
	}

	if (moof_save_trks (moof)) {
		return (1);
	}

	if (moof_save_meta (moof)) {
		return (1);
	}

	if (moof_save_crc (moof)) {
		return (1);
	}

	return (0);
}

int pri_save_moof (FILE *fp, const pri_img_t *img)
{
	moof_save_t moof;

	moof_save_init (&moof, fp, img);

	if (moof_save_img (&moof)) {
		return (1);
	}

	return (0);
}


int pri_probe_moof_fp (FILE *fp)
{
	unsigned char buf[8];

	if (pri_read_ofs (fp, 0, buf, 8)) {
		return (0);
	}

	if (pri_get_uint32_le (buf, 0) != MOOF_MAGIC_MOOF) {
		return (0);
	}

	if (pri_get_uint32_le (buf, 4) != MOOF_MAGIC_2) {
		return (0);
	}

	return (1);
}

int pri_probe_moof (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pri_probe_moof_fp (fp);

	fclose (fp);

	return (r);
}
