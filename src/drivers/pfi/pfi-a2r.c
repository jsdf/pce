/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-a2r.c                                    *
 * Created:     2019-06-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2019-2023 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pfi.h"
#include "pfi-io.h"
#include "pfi-a2r.h"


#ifndef DEBUG_A2R
#define DEBUG_A2R 0
#endif


#define A2R_CLOCK      8000000

#define A2R_MAGIC_A2R  0x00523241
#define A2R_MAGIC_A2R2 0x32523241
#define A2R_MAGIC_A2R3 0x33523241
#define A2R_MAGIC_2    0x0a0d0aff
#define A2R_MAGIC_INFO 0x4f464e49
#define A2R_MAGIC_RWCP 0x50435752
#define A2R_MAGIC_STRM 0x4d525453
#define A2R_MAGIC_META 0x4154454d


typedef struct {
	FILE          *fp;
	pfi_img_t     *img;
	pfi_trk_t     *trk;

	unsigned      version;

	unsigned long c;
	unsigned long h;
	unsigned long loop;

	char          have_info;
	unsigned char disk_type;
	unsigned char write_protected;
	unsigned char synchronized;
	unsigned char sect_count;

	char          creator[33];
} a2r_load_t;

typedef struct {
	FILE          *fp;
	pfi_img_t     *img;
	pfi_trk_t     *trk;
	unsigned long ofs;
	unsigned char location;
	unsigned char disk_type;

	unsigned long buf_cnt;
	unsigned long buf_max;
	unsigned char *buf;
} a2r_save_t;


static
int a2r_read (a2r_load_t *a2r, void *buf, unsigned long cnt, unsigned long *size)
{
	if (cnt > *size) {
		return (1);
	}

	if (pfi_read (a2r->fp, buf, cnt)) {
		return (1);
	}

	*size -= cnt;

	return (0);
}

static
void a2r_get_string (char *dst, const unsigned char *src, unsigned max)
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
void a2r_set_string (unsigned char *dst, const char *src, unsigned max)
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
}

static
int a2r_load_header (a2r_load_t *a2r)
{
	unsigned long id;
	unsigned char buf[8];

	if (pfi_read (a2r->fp, buf, 8)) {
		return (1);
	}

	id = pfi_get_uint32_le (buf, 0);

	if ((id & 0x00ffffff) != A2R_MAGIC_A2R) {
		fprintf (stderr, "a2r: bad magic 1\n");
		return (1);
	}

	switch (id) {
	case A2R_MAGIC_A2R2:
		a2r->version = 2;
		break;

	case A2R_MAGIC_A2R3:
		a2r->version = 3;
		break;

	default:
		fprintf (stderr, "a2r: unsupported version (%02lX)\n",
			(id >> 24) & 0xff
		);
		return (1);
	}

	if (pfi_get_uint32_le (buf, 4) != A2R_MAGIC_2) {
		fprintf (stderr, "a2r: bad magic 2\n");
		return (1);
	}

	return (0);
}

static
int a2r_load_meta (a2r_load_t *a2r, unsigned long size)
{
	unsigned      n;
	unsigned char buf[256];

	while (size > 0) {
		n = (size < 256) ? size : 256;

		if (pfi_read (a2r->fp, buf, n)) {
			return (1);
		}

		if (pfi_img_add_comment (a2r->img, buf, n)) {
			return (1);
		}

		size -= n;
	}

	return (0);
}

static
int a2r_load_info (a2r_load_t *a2r, unsigned long size)
{
	unsigned      cnt;
	unsigned char buf[64];

	if (size < 36) {
		return (1);
	}

	cnt = (size < 64) ? size : 64;

	if (pfi_read (a2r->fp, buf, cnt)) {
		return (1);
	}

	if (buf[0] < 1) {
		fprintf (stderr, "a2r: bad info version (%u)\n", buf[0]);
		return (1);
	}

	a2r_get_string (a2r->creator, buf + 1, 32);

	a2r->disk_type = buf[33];
	a2r->write_protected = buf[34];
	a2r->synchronized = buf[35];
	a2r->sect_count = 0;

	if (size >= 37) {
		a2r->sect_count = buf[36];
	}

#if DEBUG_A2R >= 1
	fprintf (stderr, "a2r: creator         = '%s'\n", a2r->creator);
	fprintf (stderr, "a2r: disk type       = %u\n", a2r->disk_type);
	fprintf (stderr, "a2r: write protected = %u\n", a2r->write_protected);
	fprintf (stderr, "a2r: synchronized    = %u\n", a2r->synchronized);
	fprintf (stderr, "a2r: sector count    = %u\n", a2r->sect_count);
#endif

	if (pfi_skip (a2r->fp, size - cnt)) {
		return (1);
	}

	return (0);
}

static
int a2r_load_capture (a2r_load_t *a2r, unsigned long size)
{
	unsigned      i, n;
	unsigned long pulse, index, total;
	unsigned char buf[256];

	pulse = 0;
	total = 0;
	index = 0;

	i = 0;
	n = 0;

	while (size > 0) {
		if (total >= index) {
			if (pfi_trk_add_index (a2r->trk, total)) {
				return (1);
			}

			index += a2r->loop;
		}

		if (i >= n) {
			i = 0;
			n = (size < 256) ? size : 256;

			if (pfi_read (a2r->fp, buf, n)) {
				return (1);
			}
		}

		if (buf[i] < 255) {
			pulse += buf[i];

			if (pulse > 0) {
				if (pfi_trk_add_pulse (a2r->trk, pulse)) {
					return (1);
				}
			}

			total += pulse;
			pulse = 0;
		}
		else {
			pulse += 255;
		}

		i += 1;
		size -= 1;
	}

	pfi_trk_clean (a2r->trk);

	return (0);
}

static
int a2r_load_strm (a2r_load_t *a2r, unsigned long size)
{
	unsigned long cnt;
	unsigned char buf[10];

	while (size > 0) {
		if (a2r_read (a2r, buf, 1, &size)) {
			return (1);
		}

		if (buf[0] == 0xff) {
			break;
		}

		if (a2r_read (a2r, buf + 1, 9, &size)) {
			return (1);
		}

		if (a2r->disk_type == 1) {
			a2r->c = buf[0];
			a2r->h = 0;
		}
		else {
			a2r->c = buf[0] >> 1;
			a2r->h = buf[0] & 1;
		}

		cnt = pfi_get_uint32_le (buf, 2);

		if (cnt > size) {
			return (1);
		}

		a2r->loop = pfi_get_uint32_le (buf, 6);

#if DEBUG_A2R >= 2
		fprintf (stderr, "a2r: %lu/%lu TYPE=%u SIZE=%lu LOOP=%lu\n",
			a2r->c, a2r->h, buf[1], cnt, a2r->loop
		);
#endif

		if ((buf[1] == 1) || (buf[1] == 3)) {
			a2r->trk = pfi_img_get_track (a2r->img, a2r->c, a2r->h, 1);

			if (a2r->trk == NULL) {
				return (1);
			}

			pfi_trk_reset (a2r->trk);
			pfi_trk_set_clock (a2r->trk, A2R_CLOCK);

			if (a2r_load_capture (a2r, cnt)) {
				return (1);
			}
		}
		else {
			fprintf (stderr,
				"a2r: warning: unsupported capture type (%u)\n",
				buf[1]
			);

			if (pfi_skip (a2r->fp, cnt)) {
				return (1);
			}
		}

		size -= cnt;
	}

	if (size > 0) {
		fprintf (stderr,
			"a2r: warning: %lu extra bytes in STRM chunk\n",
			size
		);

		if (pfi_skip (a2r->fp, size)) {
			return (1);
		}
	}

	return (0);
}

static
int a2r_load_rwcp (a2r_load_t *a2r, unsigned long size)
{
	unsigned      i;
	unsigned      loc, type, icnt;
	unsigned long cnt, res, clk;
	unsigned char buf[16];

	if (a2r_read (a2r, buf, 16, &size)) {
		return (1);
	}

	if (buf[0] != 1) {
		fprintf (stderr, "a2r: bad RWCP version (%u)\n", buf[0]);
		return (1);
	}

	res = pfi_get_uint32_le (buf, 1);

	if (res < 256) {
		return (1);
	}

	clk = 1000000000000ULL / res;

#if DEBUG_A2R >= 1
	fprintf (stderr, "a2r: resolution = %lu (%lu MHz)\n", res, clk);
#endif

	while (1) {
		if (a2r_read (a2r, buf, 1, &size)) {
			return (1);
		}

		if (buf[0] == 0x58) {
			break;
		}

		if (buf[0] != 0x43) {
			return (1);
		}

		if (a2r_read (a2r, buf, 4, &size)) {
			return (1);
		}

		type = buf[0];
		loc = pfi_get_uint16_le (buf, 1);
		icnt = buf[3];

		if (a2r->disk_type == 1) {
			a2r->c = loc;
			a2r->h = 0;
		}
		else {
			a2r->c = loc >> 1;
			a2r->h = loc & 1;
		}

		a2r->trk = pfi_img_get_track (a2r->img, a2r->c, a2r->h, 1);

		if (a2r->trk == NULL) {
			return (1);
		}

		pfi_trk_reset (a2r->trk);
		pfi_trk_set_clock (a2r->trk, clk);

		a2r->loop = 0;

		for (i = 0; i < icnt; i++) {
			if (a2r_read (a2r, buf, 4, &size)) {
				return (1);
			}

			if (i == 0) {
				a2r->loop = pfi_get_uint32_le (buf, 0);
			}
		}

		if (a2r_read (a2r, buf, 4, &size)) {
			return (1);
		}

		cnt = pfi_get_uint32_le (buf, 0);

		if (cnt > size) {
			return (1);
		}

#if DEBUG_A2R >= 1
		fprintf (stderr, "a2r: %lu/%lu TYPE=%u SIZE=%lu IDX=%u\n",
			a2r->c, a2r->h, type, cnt, icnt
		);
#endif

		if ((type == 1) || (type == 3)) {
			if (a2r_load_capture (a2r, cnt)) {
				return (1);
			}
		}
		else {
			fprintf (stderr,
				"a2r: warning: unsupported capture type (%u)\n",
				type
			);

			if (pfi_skip (a2r->fp, cnt)) {
				return (1);
			}
		}

		size -= cnt;
	}

	if (size > 0) {
		fprintf (stderr,
			"a2r: warning: %lu extra bytes in RWCP chunk\n",
			size
		);

		if (pfi_skip (a2r->fp, size)) {
			return (1);
		}
	}

	return (0);
}

static
int a2r_load_image (a2r_load_t *a2r)
{
	unsigned long type, size;
	unsigned char buf[8];

	if (a2r_load_header (a2r)) {
		return (1);
	}

	a2r->have_info = 0;

	while (pfi_read (a2r->fp, buf, 8) == 0) {
		type = pfi_get_uint32_le (buf, 0);
		size = pfi_get_uint32_le (buf, 4);

#if DEBUG_A2R >= 2
		fprintf (stderr, "chunk %08lX + %08lX\n", type, size);
#endif

		if (type == A2R_MAGIC_INFO) {
			if (a2r_load_info (a2r, size)) {
				return (1);
			}

			a2r->have_info = 1;
		}
		else if (type == A2R_MAGIC_RWCP) {
			if (a2r->have_info == 0) {
				return (1);
			}

			if (a2r_load_rwcp (a2r, size)) {
				return (1);
			}
		}
		else if (type == A2R_MAGIC_STRM) {
			if (a2r->have_info == 0) {
				return (1);
			}

			if (a2r_load_strm (a2r, size)) {
				return (1);
			}
		}
		else if (type == A2R_MAGIC_META) {
			if (a2r_load_meta (a2r, size)) {
				return (1);
			}
		}
		else {
			fprintf (stderr, "a2r: unknown chunk type (%08lX)\n",
				type
			);

			if (pfi_skip (a2r->fp, size)) {
				return (1);
			}
		}
	}

	return (0);
}

pfi_img_t *pfi_load_a2r (FILE *fp)
{
	a2r_load_t a2r;

	a2r.fp = fp;
	a2r.img = NULL;
	a2r.trk = NULL;
	a2r.have_info = 0;

	if ((a2r.img = pfi_img_new()) == NULL) {
		return (NULL);
	}

	if (a2r_load_image (&a2r)) {
		pfi_img_del (a2r.img);
		return (NULL);
	}

	return (a2r.img);
}


static
int a2r_get_disk_type (a2r_save_t *a2r)
{
	unsigned c;

	if (a2r->img->cyl_cnt < 90) {
		a2r->disk_type = 2;
		return (0);
	}

	for (c = 0; c < 80; c++) {
		if (pfi_img_get_track_const (a2r->img, c, 1) != NULL) {
			a2r->disk_type = 2;
			return (0);
		}
	}

	a2r->disk_type = 1;

	return (0);
}

static
int a2r_save_header (a2r_save_t *a2r)
{
	unsigned char buf[8];

	pfi_set_uint32_le (buf, 0, A2R_MAGIC_A2R2);
	pfi_set_uint32_le (buf, 4, A2R_MAGIC_2);

	if (pfi_write_ofs (a2r->fp, a2r->ofs, buf, 8)) {
		return (1);
	}

	a2r->ofs += 8;

	return (0);
}

static
int a2r_save_info (a2r_save_t *a2r)
{
	unsigned char buf[44];

	pfi_set_uint32_le (buf, 0, A2R_MAGIC_INFO);
	pfi_set_uint32_le (buf, 4, 36);

	/* version */
	buf[8] = 1;

	a2r_set_string (buf + 9, "pce-" PCE_VERSION_STR, 32);

	buf[41] = a2r->disk_type;
	buf[42] = 1; /* write protected */
	buf[43] = 1; /* cross track sync */

	if (pfi_write_ofs (a2r->fp, a2r->ofs, buf, 44)) {
		return (1);
	}

	a2r->ofs += 44;

	return (0);
}

static
int a2r_add_byte (a2r_save_t *a2r, unsigned val)
{
	unsigned long max;
	unsigned char *tmp;

	if (a2r->buf_cnt >= a2r->buf_max) {
		max = (a2r->buf_max < 4096) ? 4096 : a2r->buf_max;

		while (max <= a2r->buf_cnt) {
			max *= 2;
		}

		if ((tmp = realloc (a2r->buf, max)) == NULL) {
			return (1);
		}

		a2r->buf = tmp;
		a2r->buf_max = max;
	}

	a2r->buf[a2r->buf_cnt++] = val;

	return (0);
}

static
int a2r_save_track (a2r_save_t *a2r)
{
	uint32_t           pulse, idx;
	unsigned           index;
	unsigned long      loop;
	unsigned long      rem;
	unsigned long long tmp;
	unsigned char      buf[10];
	pfi_trk_t          *trk;

	trk = a2r->trk;

	pfi_trk_rewind (trk);

	a2r->buf_cnt = 0;

	index = 0;
	loop = 0;
	rem = 0;

	while (pfi_trk_get_pulse (trk, &pulse, &idx) == 0) {
		if ((pulse == 0) || (idx < pulse)) {
			index += 1;
		}

		if ((index < 1) || (pulse == 0)) {
			continue;
		}

		tmp = (unsigned long long) A2R_CLOCK * pulse + rem;
		rem = tmp % trk->clock;
		pulse = tmp / trk->clock;

		if (index == 1) {
			loop += pulse;
		}

		while (pulse >= 255) {
			if (a2r_add_byte (a2r, 255)) {
				return (1);
			}

			pulse -= 255;
		}

		if (a2r_add_byte (a2r, pulse)) {
			return (1);
		}
	}

	buf[0] = a2r->location;
	buf[1] = (index > 2) ? 3 : 1;

	pfi_set_uint32_le (buf, 2, a2r->buf_cnt);
	pfi_set_uint32_le (buf, 6, loop);

	if (pfi_write_ofs (a2r->fp, a2r->ofs, buf, 10)) {
		return (1);
	}

	a2r->ofs += 10;

	if (pfi_write_ofs (a2r->fp, a2r->ofs, a2r->buf, a2r->buf_cnt)) {
		return (1);
	}

	a2r->ofs += a2r->buf_cnt;

	return (0);
}

static
int a2r_save_strm (a2r_save_t *a2r)
{
	unsigned      i, c, h;
	unsigned long ofs;
	unsigned char buf[8];

	ofs = a2r->ofs;

	a2r->ofs += 8;

	for (i = 0; i < 256; i++) {
		if (a2r->disk_type == 1) {
			c = i;
			h = 0;
		}
		else {
			c = i >> 1;
			h = i & 1;
		}

		a2r->trk = pfi_img_get_track (a2r->img, c, h, 0);

		if (a2r->trk  == NULL) {
			continue;
		}

		a2r->location = i;

		if (a2r_save_track (a2r)) {
			return (1);
		}
	}

	buf[0] = 0xff;

	if (pfi_write_ofs (a2r->fp, a2r->ofs, buf, 1)) {
		return (1);
	}

	a2r->ofs += 1;

	pfi_set_uint32_le (buf, 0, A2R_MAGIC_STRM);
	pfi_set_uint32_le (buf, 4, a2r->ofs - ofs - 8);

	if (pfi_write_ofs (a2r->fp, ofs, buf, 8)) {
		return (1);
	}

	return (0);
}

static
int a2r_save_meta (a2r_save_t *a2r)
{
	unsigned long size;
	unsigned char buf[8];

	size = a2r->img->comment_size;

	if (size == 0) {
		return (0);
	}

	pfi_set_uint32_le (buf, 0, A2R_MAGIC_META);
	pfi_set_uint32_le (buf, 4, size);

	if (pfi_write_ofs (a2r->fp, a2r->ofs, buf, 8)) {
		return (1);
	}

	if (pfi_write (a2r->fp, a2r->img->comment, size)) {
		return (1);
	}

	a2r->ofs += 8 + size;

	return (0);
}

static
int a2r_save_img (a2r_save_t *a2r)
{
	if (a2r_get_disk_type (a2r)) {
		return (1);
	}

	a2r->ofs = 0;

	if (a2r_save_header (a2r)) {
		return (1);
	}

	if (a2r_save_info (a2r)) {
		return (1);
	}

	if (a2r_save_strm (a2r)) {
		return (1);
	}

	if (a2r_save_meta (a2r)) {
		return (1);
	}

	return (0);
}

int pfi_save_a2r (FILE *fp, pfi_img_t *img)
{
	int        r;
	a2r_save_t a2r;

	a2r.fp = fp;
	a2r.img = img;
	a2r.ofs = 0;
	a2r.buf_cnt = 0;
	a2r.buf_max = 0;
	a2r.buf = NULL;

	r = a2r_save_img (&a2r);

	free (a2r.buf);

	return (r);
}

int pfi_probe_a2r_fp (FILE *fp)
{
	unsigned char buf[8];

	if (pfi_read (fp, buf, 8)) {
		return (0);
	}

	if (pfi_get_uint32_le (buf, 0) != A2R_MAGIC_A2R2) {
		return (0);
	}

	if (pfi_get_uint32_le (buf, 4) != A2R_MAGIC_2) {
		return (0);
	}

	return (1);
}

int pfi_probe_a2r (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = pfi_probe_a2r_fp (fp);

	fclose (fp);

	return (r);
}
