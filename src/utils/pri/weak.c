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
