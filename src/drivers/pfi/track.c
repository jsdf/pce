/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/track.c                                      *
 * Created:     2012-01-25 by Hampa Hug <hampa@hampa.ch>                     *
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

#include "track.h"


pfi_trk_t *pfi_trk_new (void)
{
	pfi_trk_t *trk;

	if ((trk = malloc (sizeof (pfi_trk_t))) == NULL) {
		return (NULL);
	}

	trk->clock = 0;

	trk->size = 0;
	trk->max = 0;
	trk->data = NULL;

	trk->idx_cnt = 0;
	trk->idx = NULL;

	trk->cur_pos = 0;
	trk->cur_clk = 0;
	trk->cur_idx = 0;

	return (trk);
}

void pfi_trk_del (pfi_trk_t *trk)
{
	if (trk != NULL) {
		free (trk->idx);
		free (trk->data);
		free (trk);
	}
}

pfi_trk_t *pfi_trk_clone (const pfi_trk_t *trk)
{
	unsigned  i;
	pfi_trk_t *ret;

	if ((ret = pfi_trk_new()) == NULL) {
		return (NULL);
	}

	*ret = *trk;

	ret->idx = NULL;
	ret->data = NULL;

	if (trk->size > 0) {
		ret->data = malloc (ret->size);

		if (ret->data == NULL) {
			pfi_trk_del (ret);
			return (NULL);
		}

		memcpy (ret->data, trk->data, trk->size);

		ret->max = ret->size;
	}

	if (trk->idx_cnt > 0) {
		ret->idx = malloc (trk->idx_cnt * sizeof (unsigned long));

		if (ret->idx == NULL) {
			pfi_trk_del (ret);
			return (NULL);
		}

		for (i = 0; i < trk->idx_cnt; i++) {
			ret->idx[i] = trk->idx[i];
		}
	}

	return (ret);
}

void pfi_trk_reset (pfi_trk_t *trk)
{
	free (trk->data);
	trk->data = NULL;

	free (trk->idx);
	trk->idx = NULL;

	trk->size = 0;
	trk->max = 0;

	trk->cur_pos = 0;
	trk->cur_clk = 0;
	trk->cur_idx = 0;
}

void pfi_trk_set_clock (pfi_trk_t *trk, unsigned long clock)
{
	trk->clock = clock;
}

unsigned long pfi_trk_get_clock (const pfi_trk_t *trk)
{
	return (trk->clock);
}

int pfi_trk_set_size (pfi_trk_t *trk, unsigned long cnt)
{
	unsigned char *tmp;

	tmp = realloc (trk->data, cnt);

	if (tmp == NULL) {
		return (1);
	}

	if (cnt > trk->size) {
		memset (tmp + trk->size, 0, cnt - trk->size);
	}

	trk->size = cnt;
	trk->max = cnt;
	trk->data = tmp;

	trk->cur_pos = 0;
	trk->cur_clk = 0;
	trk->cur_idx = 0;

	return (0);
}

int pfi_trk_add_data (pfi_trk_t *trk, const void *buf, unsigned long cnt)
{
	unsigned long max;
	unsigned char *tmp;

	if ((trk->size + cnt) > trk->max) {
		max = (trk->max < 128) ? 128 : trk->max;

		while (max < (trk->size + cnt)) {
			max *= 2;

			if ((max & (max - 1)) != 0) {
				max &= max - 1;
			}
		}

		tmp = realloc (trk->data, max);

		if (tmp == NULL) {
			return (1);
		}

		trk->max = max;
		trk->data = tmp;
	}

	memcpy (trk->data + trk->size, buf, cnt);

	trk->size += cnt;

	return (0);
}

int pfi_trk_add_index (pfi_trk_t *trk, unsigned long clk)
{
	unsigned      i, n;
	unsigned long *tmp;

	n = trk->idx_cnt + 1;

	tmp = realloc (trk->idx, n * sizeof (unsigned long));

	if (tmp == NULL) {
		return (1);
	}

	i = trk->idx_cnt;

	while ((i > 0) && (clk < tmp[i - 1])) {
		tmp[i] = tmp[i - 1];
		i -= 1;
	}

	tmp[i] = clk;

	trk->idx = tmp;
	trk->idx_cnt = n;

	return (0);
}

int pfi_trk_add_pulse (pfi_trk_t *trk, unsigned long cnt)
{
	unsigned      n;
	unsigned char buf[16];

	if (cnt == 0) {
		return (0);
	}

	n = 0;

	if (cnt < 8) {
		buf[n++] = 4;
		buf[n++] = cnt;
	}
	else if (cnt < (1UL << 8)) {
		buf[n++] = cnt;
	}
	else if (cnt < (1UL << 10)) {
		buf[n++] = ((cnt >> 8) & 3) | 4;
		buf[n++] = cnt & 0xff;
	}
	else if (cnt < (1UL << 16)) {
		buf[n++] = 1;
		buf[n++] = (cnt >> 8) & 0xff;
		buf[n++] = cnt & 0xff;
	}
	else if (cnt < (1UL << 24)) {
		buf[n++] = 2;
		buf[n++] = (cnt >> 16) & 0xff;
		buf[n++] = (cnt >> 8) & 0xff;
		buf[n++] = cnt & 0xff;
	}
	else {
		buf[n++] = 3;
		buf[n++] = (cnt >> 24) & 0xff;
		buf[n++] = (cnt >> 16) & 0xff;
		buf[n++] = (cnt >> 8) & 0xff;
		buf[n++] = cnt & 0xff;
	}

	if (pfi_trk_add_data (trk, buf, n)) {
		return (1);
	}

	return (0);
}


void pfi_trk_rewind (pfi_trk_t *trk)
{
	trk->cur_pos = 0;
	trk->cur_clk = 0;
	trk->cur_idx = 0;
}

int pfi_trk_get_pulse (pfi_trk_t *trk, unsigned long *val, unsigned long *idx)
{
	unsigned      cnt;
	unsigned long pos, ret;

	pos = trk->cur_pos;

	if (pos >= trk->size) {
		if (trk->cur_idx < trk->idx_cnt) {
			*idx = trk->idx[trk->cur_idx] - trk->cur_clk;
			*val = 0;

			trk->cur_idx += 1;

			return (0);
		}

		return (1);
	}

	if (trk->data[pos] == 0) {
		return (1);
	}

	cnt = trk->data[pos++];

	if (cnt < 4) {
		if ((pos + cnt + 1) > trk->size) {
			return (1);
		}

		ret = trk->data[pos++];

		while (cnt > 0) {
			ret = (ret << 8) | trk->data[pos++];
			cnt -= 1;
		}
	}
	else if (cnt < 8) {
		if ((pos + 1) > trk->size) {
			return (1);
		}

		ret = ((cnt << 8) | trk->data[pos++]) & 0x03ff;
	}
	else {
		ret = cnt;
	}

	*val = ret;
	*idx = -1;

	if (trk->cur_idx < trk->idx_cnt) {
		*idx = trk->idx[trk->cur_idx] - trk->cur_clk;

		if ((trk->cur_clk + ret) > trk->idx[trk->cur_idx]) {
			trk->cur_idx += 1;
		}
	}

	trk->cur_pos = pos;
	trk->cur_clk += ret;

	return (0);
}

pfi_trk_t *pfi_trk_scale (pfi_trk_t *trk, unsigned long mul, unsigned long div)
{
	unsigned long      val, idx, clk;
	unsigned long      v;
	unsigned long long m, d, t, r;
	pfi_trk_t          *ret;

	if ((ret = pfi_trk_new()) == NULL) {
		return (NULL);
	}

	pfi_trk_set_clock (ret, trk->clock);

	pfi_trk_rewind (trk);

	m = mul;
	d = div;
	r = 0;

	clk = 0;

	while (pfi_trk_get_pulse (trk, &val, &idx) == 0) {
		if ((val == 0) || (idx < val)) {
			if (pfi_trk_add_index (ret, clk + (m * idx) / d)) {
				break;
			}
		}

		if (val == 0) {
			continue;
		}

		t = m * val + r;
		v = t / d;
		r = t % d;

		if (pfi_trk_add_pulse (ret, v)) {
			break;
		}

		clk += v;
	}

	if (trk->cur_pos < trk->size) {
		pfi_trk_del (ret);
		return (NULL);
	}

	return (ret);
}

void pfi_trk_shift_index (pfi_trk_t *trk, long ofs)
{
	unsigned      i, j;
	unsigned long val;

	val = (ofs < 0) ? -ofs : ofs;

	j = 0;

	for (i = 0; i < trk->idx_cnt; i++) {
		if (ofs < 0) {
			if (val <= trk->idx[i]) {
				trk->idx[j++] = trk->idx[i] - val;
			}
		}
		else {
			trk->idx[j++] = trk->idx[i] + val;
		}
	}
}
