/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/weak.c                                         *
 * Created:     2015-04-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/pri/pri.h>


static
void set_bits (unsigned char *buf, unsigned long p1, unsigned long p2)
{
	while (p1 <= p2) {
		buf[p1 >> 3] |= 0x80 >> (p1 & 7);
		p1 += 1;
	}
}

static
void clear_bits (unsigned char *buf, unsigned long p1, unsigned long p2)
{
	while (p1 <= p2) {
		buf[p1 >> 3] &= ~(0x80 >> (p1 & 7));
		p1 += 1;
	}
}

static
int pri_weak_clean_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long cnt;
	unsigned char *buf;

	if (pri_trk_get_weak_mask (trk, &buf, &cnt)) {
		return (1);
	}

	if (pri_trk_set_weak_mask (trk, buf, cnt)) {
		free (buf);
		return (1);
	}

	free (buf);

	return (0);
}

int pri_weak_clean (pri_img_t *img)
{
	return (pri_for_all_tracks (img, pri_weak_clean_cb, NULL));
}

static
int pri_weak_close_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long i, n;
	unsigned      j;
	unsigned      cnt;
	int           have1;
	unsigned long last1, next1;
	unsigned char *buf;

	cnt = *(unsigned *)opaque;

	if (cnt == 0) {
		return (0);
	}

	if (pri_trk_get_weak_mask (trk, &buf, &n)) {
		return (1);
	}

	have1 = 0;
	last1 = 0;

	for (i = 0; i < n; i++) {
		if (buf[i] == 0) {
			continue;
		}

		for (j = 0; j < 8; j++) {
			if ((buf[i] & (0x80 >> j)) == 0) {
				continue;
			}

			next1 = 8 * i + j;

			if (have1) {
				if ((next1 - last1 - 1) <= cnt) {
					set_bits (buf, last1, next1);
				}
			}

			have1 = 1;
			last1 = next1;
		}
	}

	if (pri_trk_set_weak_mask (trk, buf, n)) {
		free (buf);
		return (1);
	}

	free (buf);

	return (0);
}

int pri_weak_close (pri_img_t *img, unsigned cnt)
{
	return (pri_for_all_tracks (img, pri_weak_close_cb, &cnt));
}

/*
 * Mark zero bits that are preceded by at least max zero bits as weak.
 */
static
int pri_weak_detect_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long pos, msk, bit;
	unsigned long cnt, max;
	pri_evt_t     *evt;

	cnt = 0;
	max = *(unsigned long *)opaque;

	pos = 0;
	msk = 0;

	pri_trk_set_pos (trk, 0);

	while (trk->wrap == 0) {
		pri_trk_get_bits (trk, &bit, 1);

		if (bit) {
			cnt = 0;
		}
		else if (cnt < max) {
			cnt += 1;
		}
		else {
			if (msk == 0) {
				evt = pri_trk_evt_get_before (trk, PRI_EVENT_WEAK, pos);

				if ((evt != NULL) && ((pos - evt->pos) >= 32)) {
					evt = NULL;
				}

				if (evt == NULL) {
					evt = pri_trk_evt_add (trk, PRI_EVENT_WEAK, pos, 0);

					if (evt == NULL) {
						return (1);
					}
				}

				msk = 0x80000000 >> (pos - evt->pos);
			}

			evt->val |= msk;
		}

		pos += 1;
		msk >>= 1;
	}

	return (0);
}

int pri_weak_detect (pri_img_t *img, unsigned long cnt)
{
	return (pri_for_all_tracks (img, pri_weak_detect_cb, &cnt));
}

static
int pri_weak_expand_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long i, n;
	unsigned      j;
	unsigned long pos, p1, p2;
	unsigned long *val;
	unsigned char *buf1, *buf2;;

	val = opaque;

	if (pri_trk_get_weak_mask (trk, &buf1, &n)) {
		return (1);
	}

	if (pri_trk_get_weak_mask (trk, &buf2, &n)) {
		free (buf1);
		return (1);
	}

	for (i = 0; i < n; i++) {
		if (buf1[i] == 0) {
			continue;
		}

		for (j = 0; j < 8; j++) {
			if ((buf1[i] & (0x80 >> j)) == 0) {
				continue;
			}

			pos = 8 * i + j;

			p1 = (pos < val[0]) ? 0 : (pos - val[0]);
			p2 = pos + val[1];

			if (p2 >= trk->size) {
				p2 = trk->size - 1;
			}

			set_bits (buf2, p1, p2);
		}
	}

	free (buf1);

	if (pri_trk_set_weak_mask (trk, buf2, n)) {
		free (buf2);
		return (1);
	}

	free (buf2);

	return (0);
}

int pri_weak_expand (pri_img_t *img, unsigned long left, unsigned long right)
{
	unsigned long val[2];

	val[0] = left;
	val[1] = right;

	return (pri_for_all_tracks (img, pri_weak_expand_cb, val));
}

static
int pri_weak_open_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long i, n;
	unsigned      j;
	unsigned long cnt, run, pos;
	unsigned char *buf;

	cnt = *(unsigned long *) opaque;

	if (pri_trk_get_weak_mask (trk, &buf, &n)) {
		return (1);
	}

	run = 0;

	for (i = 0; i < n; i++) {
		if ((run == 0) && (buf[i] == 0)) {
			continue;
		}

		for (j = 0; j < 8; j++) {
			if (buf[i] & (0x80 >> j)) {
				run += 1;
			}
			else if (run > 0) {
				if (run <= cnt) {
					pos = 8 * i + j;
					clear_bits (buf, pos - run - 1, pos - 1);
				}

				run = 0;
			}
		}
	}

	if (pri_trk_set_weak_mask (trk, buf, n)) {
		free (buf);
		return (1);
	}

	free (buf);

	return (0);
}

int pri_weak_open (pri_img_t *img, unsigned long cnt)
{
	return (pri_for_all_tracks (img, pri_weak_open_cb, &cnt));
}


static
int pri_weak_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long idx, msk, bit, tmp;
	unsigned long type, val;
	unsigned      mode;

	/*
	 * bit 0: random
	 * bit 1: flip
	 * bit 2: set
	 * bit 3: clear
	 */
	mode = *(unsigned *)opaque;

	idx = 0;
	msk = 0;

	pri_trk_set_pos (trk, 0);

	while (trk->wrap == 0) {
		while (pri_trk_get_event (trk, &type, &val) == 0) {
			if (type == PRI_EVENT_WEAK) {
				if (mode & 1) {
					tmp = rand();
					tmp = (tmp << 10) ^ rand();
					tmp = (tmp << 10) ^ rand();
					val &= tmp;
				}

				msk |= val;
			}
		}

		pri_trk_get_bits (trk, &bit, 1);

		if (msk & 0x80000000) {
			if (mode & 2) {
				trk->data[idx >> 3] ^= 0x80 >> (idx & 7);
			}
			else if (mode & 4) {
				trk->data[idx >> 3] |= 0x80 >> (idx & 7);
			}
			else if (mode & 8) {
				trk->data[idx >> 3] &= ~(0x80 >> (idx & 7));
			}
		}

		idx += 1;
		msk <<= 1;
	}

	return (0);
}

int pri_weak_flip (pri_img_t *img, int random)
{
	unsigned mode;

	mode = 2 | (random ? 1 : 0);

	return (pri_for_all_tracks (img, pri_weak_cb, &mode));
}

int pri_weak_set (pri_img_t *img, int val)
{
	unsigned mode;

	mode = val ? 4 : 8;

	return (pri_for_all_tracks (img, pri_weak_cb, &mode));
}

static
int pri_weak_load_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long cnt;
	unsigned char *buf;
	FILE          *fp;

	fp = opaque;

	cnt = (pri_trk_get_size (trk) + 7) / 8;

	if ((buf = malloc (cnt)) == NULL) {
		return (1);
	}

	if (fread (buf, cnt, 1, fp) != 1) {
		free (buf);
		return (1);
	}

	if (pri_trk_set_weak_mask (trk, buf, cnt)) {
		free (buf);
		return (1);
	}

	free (buf);

	return (0);
}

int pri_weak_load (pri_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (1);
	}

	r = pri_for_all_tracks (img, pri_weak_load_cb, fp);

	fclose (fp);

	return (r);
}

static
int pri_weak_save_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	int           r;
	FILE          *fp;
	unsigned long cnt;
	unsigned char *buf;

	if (pri_trk_get_weak_mask (trk, &buf, &cnt)) {
		return (1);
	}

	r = 0;

	fp = opaque;

	if (fwrite (buf, cnt, 1, fp) != 1) {
		r = 1;
	}

	free (buf);

	return (r);
}

int pri_weak_save (pri_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	r = pri_for_all_tracks (img, pri_weak_save_cb, fp);

	fclose (fp);

	return (r);
}
