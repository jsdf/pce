/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/pri-mac-gcr.c                                  *
 * Created:     2021-09-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021-2022 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "pri-mac-gcr.h"

#include <stdio.h>
#include <string.h>

#include <drivers/pri/pri.h>


int pri_mac_get_nibble (pri_trk_t *trk, unsigned char *val, unsigned *cnt)
{
	unsigned long i;

	i = trk->idx;

	*val = 0;
	*cnt = 0;

	while ((~*val & 0x80) && (*cnt < 10)) {
		if (i >= trk->size) {
			i = 0;
			trk->wrap = 1;
		}

		*val = (*val << 1) | ((trk->data[i >> 3] >> (~i & 7)) & 1);
		*cnt += 1;

		i += 1;
	}

	trk->idx = i;

	return (trk->wrap);
}

/*
 * Find the sector end mark before the first sector ID on a track.
 */
int pri_mac_find_first (pri_trk_t *trk, unsigned long *pos)
{
	unsigned      i, rev, cnt, msk;
	int           data;
	unsigned long end1, end2, start;
	unsigned char v[5];

	*pos = 0;

	end1 = -1;
	end2 = 0;
	start = -1;
	rev = 0;
	msk = 0;
	data = 0;

	for (i = 0; i < 5; i++) {
		v[i] = 0;
	}

	pri_trk_set_pos (trk, 0);

	while (rev < 2) {
		for (i = 1; i < 5; i++) {
			v[i - 1] = v[i];
		}

		if (pri_mac_get_nibble (trk, &v[4], &cnt)) {
			rev += 1;
			trk->wrap = 0;
		}

		if (data && (v[2] == 0xde) && (v[3] == 0xaa)) {
			if (trk->idx < end1) {
				end1 = trk->idx;
			}

			if (trk->idx > end2) {
				end2 = trk->idx;
			}

			msk |= 1;
		}
		else if ((v[0] == 0xd5) && (v[1] == 0xaa)) {
			if (v[2] == 0x96) {
				if (trk->idx < start) {
					start = trk->idx;
				}

				data = 0;
				msk |= 2;
			}

			data = (v[2] == 0xad);
		}
	}

	if (msk == 3) {
		*pos = (end1 < start) ? end1 : end2;
	}

	return (0);
}

/*
 * Find the last sector end mark on a track.
 */
int pri_mac_find_end_last (pri_trk_t *trk, unsigned long *pos)
{
	unsigned      i, rev, cnt;
	int           ret;
	unsigned long last;
	unsigned char v[5];

	*pos = 0;

	for (i = 0; i < 5; i++) {
		v[i] = 0;
	}

	rev = 0;
	last = 0;
	ret = 1;

	pri_trk_set_pos (trk, 0);

	while (rev < 2) {
		for (i = 1; i < 5; i++) {
			v[i - 1] = v[i];
		}

		if (pri_mac_get_nibble (trk, &v[4], &cnt)) {
			rev += 1;
			trk->wrap = 0;
		}

		if ((v[2] == 0xde) && (v[3] == 0xaa)) {
			last = trk->idx;
		}
		else if ((v[0] == 0xd5) && (v[1] == 0xaa) && (v[2] == 0x96)) {
			if (last > *pos) {
				*pos = last;
				ret = 0;
			}
		}
	}

	return (ret);
}

/*
 * Find the lowest sector ID on a track.
 */
int pri_mac_find_id_low (pri_trk_t *trk, unsigned long *pos, unsigned long *end)
{
	unsigned      i, rev;
	unsigned      cnt, min;
	unsigned long pos1, pos2, last;
	unsigned char v[5];
	unsigned long p[5];

	for (i = 0; i < 5; i++) {
		v[i] = 0;
		p[i] = 0;
	}

	min = -1;
	rev = 0;

	pos1 = 0;
	pos2 = 0;
	last = -1;

	pri_trk_set_pos (trk, 0);

	while (rev < 2) {
		for (i = 1; i < 5; i++) {
			v[i - 1] = v[i];
			p[i - 1] = p[i];
		}

		p[4] = trk->idx;

		if (pri_mac_get_nibble (trk, &v[4], &cnt)) {
			rev += 1;
			trk->wrap = 0;
		}

		if ((v[2] == 0xde) && (v[3] == 0xaa)) {
			last = trk->idx;
		}
		else if ((v[0] == 0xd5) && (v[1] == 0xaa) && (v[2] == 0x96)) {
			if ((v[4] < min) && (last < trk->size)) {
				pos1 = p[0];
				pos2 = last;
				min = v[4];
			}
		}
	}

	if (pos != NULL) {
		*pos = pos1;
	}

	if (end != NULL) {
		*end = pos2;
	}

	return (min > 0xff);
}

/*
 * Find the longest gap on a track.
 */
int pri_mac_find_gap (pri_trk_t *trk, unsigned long *pos1, unsigned long *pos2)
{
	unsigned      i, rev, cnt;
	unsigned long last, size, max;
	unsigned long p1, p2;
	unsigned char v[3];
	unsigned long p[3];

	p1 = 0;
	p2 = 0;
	last = -1;

	max = 0;
	rev = 0;

	for (i = 0; i < 3; i++) {
		v[i] = 0;
		p[i] = 0;
	}

	pri_trk_set_pos (trk, 0);

	while (rev < 2) {
		for (i = 1; i < 3; i++) {
			v[i - 1] = v[i];
			p[i - 1] = p[i];
		}

		p[2] = trk->idx;

		if (pri_mac_get_nibble (trk, &v[2], &cnt)) {
			rev += 1;
			trk->wrap = 0;
		}

		if ((v[0] == 0xde) && (v[1] == 0xaa)) {
			last = trk->idx;
		}
		else if ((v[0] == 0xd5) && (v[1] == 0xaa) && (v[2] == 0x96)) {
			if (last > trk->size) {
				continue;
			}

			size = (trk->size + p[0] - last) % trk->size;

			if (size > max) {
				max = size;
				p1 = last;
				p2 = p[0];
			}
		}
	}

	if (pos1 != NULL) {
		*pos1 = p1;
	}

	if (pos2 != NULL) {
		*pos2 = p2;
	}

	return (max == 0);
}

int pri_mac_align_mode (const char *str, unsigned *mode)
{
	if (strcmp (str, "none") == 0) {
		*mode = PRI_MAC_ALIGN_NONE;
	}
	else if (strcmp (str, "gap") == 0) {
		*mode = PRI_MAC_ALIGN_GAP;
	}
	else if (strcmp (str, "gap-end") == 0) {
		*mode = PRI_MAC_ALIGN_GAP_END;
	}
	else if (strcmp (str, "sector") == 0) {
		*mode = PRI_MAC_ALIGN_SECTOR_END;
	}
	else if (strcmp (str, "sector-id") == 0) {
		*mode = PRI_MAC_ALIGN_SECTOR;
	}
	else if (strcmp (str, "sector-end") == 0) {
		*mode = PRI_MAC_ALIGN_SECTOR_END;
	}
	else if (strcmp (str, "index") == 0) {
		*mode = PRI_MAC_ALIGN_INDEX;
	}
	else {
		*mode = PRI_MAC_ALIGN_NONE;

		return (1);
	}

	return (0);
}

int pri_mac_align_pos (pri_trk_t *trk, unsigned mode, unsigned long *pos)
{
	*pos = 0;

	switch (mode) {
	case PRI_MAC_ALIGN_NONE:
		return (0);

	case PRI_MAC_ALIGN_GAP:
		pri_mac_find_gap (trk, pos, NULL);
		break;

	case PRI_MAC_ALIGN_GAP_END:
		pri_mac_find_gap (trk, NULL, pos);
		break;

	case PRI_MAC_ALIGN_SECTOR:
		pri_mac_find_id_low (trk, pos, NULL);
		break;

	case PRI_MAC_ALIGN_SECTOR_END:
		pri_mac_find_id_low (trk, NULL, pos);
		break;

	case PRI_MAC_ALIGN_INDEX:
		pri_mac_find_first (trk, pos);
		break;

	default:
		return (1);
	}

	return (0);
}
