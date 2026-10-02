/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/track.c                                      *
 * Created:     2012-01-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2022 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "track.h"


void pfi_trk_init (pfi_trk_t *trk)
{
	trk->clock = 0;

	trk->pulse_cnt = 0;
	trk->pulse_max = 0;
	trk->pulse = NULL;

	trk->index_cnt = 0;
	trk->index_max = 0;
	trk->index = NULL;

	trk->cur_pos = 0;
	trk->cur_clk = 0;
	trk->cur_idx = 0;
}

void pfi_trk_free (pfi_trk_t *trk)
{
	if (trk != NULL) {
		free (trk->index);
		free (trk->pulse);
	}
}

pfi_trk_t *pfi_trk_new (void)
{
	pfi_trk_t *trk;

	if ((trk = malloc (sizeof (pfi_trk_t))) == NULL) {
		return (NULL);
	}

	pfi_trk_init (trk);

	return (trk);
}

void pfi_trk_del (pfi_trk_t *trk)
{
	if (trk != NULL) {
		pfi_trk_free (trk);
		free (trk);
	}
}

void pfi_trk_clean (pfi_trk_t *trk)
{
	uint32_t *tmp;

	if (trk->pulse_max > trk->pulse_cnt) {
		tmp = realloc (trk->pulse, trk->pulse_cnt * sizeof (uint32_t));

		if (tmp == NULL) {
			return;
		}

		trk->pulse = tmp;
		trk->pulse_max = trk->pulse_cnt;
	}

	if (trk->index_max > trk->index_cnt) {
		tmp = realloc (trk->index, trk->index_cnt * sizeof (uint32_t));

		if (tmp == NULL) {
			return;
		}

		trk->index = tmp;
		trk->index_max = trk->index_cnt;
	}
}

pfi_trk_t *pfi_trk_clone (const pfi_trk_t *trk, int data, int index)
{
	pfi_trk_t *ret;

	if ((ret = pfi_trk_new()) == NULL) {
		return (NULL);
	}

	ret->clock = trk->clock;

	if (data && (trk->pulse_cnt > 0)) {
		ret->pulse = malloc (trk->pulse_cnt * sizeof (uint32_t));

		if (ret->pulse == NULL) {
			pfi_trk_del (ret);
			return (NULL);
		}

		ret->pulse_cnt = trk->pulse_cnt;
		ret->pulse_max = trk->pulse_cnt;

		memcpy (ret->pulse, trk->pulse, trk->pulse_cnt * sizeof (uint32_t));
	}

	if (index && (trk->index_cnt > 0)) {
		ret->index = malloc (trk->index_cnt * sizeof (uint32_t));

		if (ret->index == NULL) {
			pfi_trk_del (ret);
			return (NULL);
		}

		ret->index_cnt = trk->index_cnt;
		ret->index_max = trk->index_cnt;

		memcpy (ret->index, trk->index, trk->index_cnt * sizeof (uint32_t));
	}

	return (ret);
}

void pfi_trk_reset (pfi_trk_t *trk)
{
	free (trk->pulse);
	trk->pulse = NULL;
	trk->pulse_cnt = 0;
	trk->pulse_max = 0;

	free (trk->index);
	trk->index = NULL;
	trk->index_cnt = 0;
	trk->index_max = 0;

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

int pfi_trk_get_pos (const pfi_trk_t *trk, uint32_t clk, unsigned long *pos, uint32_t *ofs)
{
	unsigned long i;
	uint32_t      curclk;

	curclk = 0;

	for (i = 0; i < trk->pulse_cnt; i++) {
		if ((curclk + trk->pulse[i]) > clk) {
			*pos = i;
			*ofs = clk - curclk;
			return (0);
		}

		curclk += trk->pulse[i];
	}

	return (1);
}

int pfi_trk_get_index (const pfi_trk_t *trk, unsigned idx, unsigned long *pos, uint32_t *ofs)
{
	if (idx >= trk->index_cnt) {
		return (1);
	}

	if (pfi_trk_get_pos (trk, trk->index[idx], pos, ofs)) {
		return (1);
	}

	return (0);
}

unsigned long pfi_trk_get_clk (const pfi_trk_t *trk, unsigned long pos)
{
	unsigned long i, clk;

	if (pos > trk->pulse_cnt) {
		pos = trk->pulse_cnt;
	}

	clk = 0;

	for (i = 0; i < pos; i++) {
		clk += trk->pulse[i];
	}

	return (clk);
}

int pfi_trk_add_index (pfi_trk_t *trk, uint32_t clk)
{
	unsigned i, max;
	uint32_t *tmp;

	for (i = 0; i < trk->index_cnt; i++) {
		if (trk->index[i] == clk) {
			return (0);
		}
	}

	if ((trk->index_cnt + 1) > trk->index_max) {
		max = (trk->index_max < 4) ? 4 : trk->index_max;

		while (max < (trk->index_cnt + 1)) {
			max *= 2;

			if ((max & (max - 1)) != 0) {
				max &= max - 1;
			}
		}

		tmp = realloc (trk->index, max * sizeof (uint32_t));

		if (tmp == NULL) {
			return (1);
		}

		trk->index = tmp;
		trk->index_max = max;
	}

	i = trk->index_cnt;

	while ((i > 0) && (clk < trk->index[i - 1])) {
		trk->index[i] = trk->index[i - 1];
		i -= 1;
	}

	trk->index[i] = clk;

	trk->index_cnt += 1;

	return (0);
}

int pfi_trk_add_pulses (pfi_trk_t *trk, const uint32_t *buf, unsigned long cnt)
{
	unsigned long i;
	unsigned long max;
	uint32_t      *tmp;

	if ((trk->pulse_cnt + cnt) > trk->pulse_max) {
		max = (trk->pulse_max < 256) ? 256 : trk->pulse_max;

		while (max < (trk->pulse_cnt + cnt)) {
			max *= 2;

			if ((max & (max - 1)) != 0) {
				max &= max - 1;
			}
		}

		tmp = realloc (trk->pulse, max * sizeof (uint32_t));

		if (tmp == NULL) {
			return (1);
		}

		trk->pulse = tmp;
		trk->pulse_max = max;
	}

	for (i = 0; i < cnt; i++) {
		trk->pulse[trk->pulse_cnt++] = buf[i];
	}

	return (0);
}

int pfi_trk_add_pulse (pfi_trk_t *trk, uint32_t val)
{
	return (pfi_trk_add_pulses (trk, &val, 1));
}

int pfi_trk_inc_pulse (pfi_trk_t *trk, uint32_t val)
{
	if (trk->pulse_cnt == 0) {
		return (1);
	}

	trk->pulse[trk->pulse_cnt - 1] += val;

	return (0);
}

void pfi_trk_rewind (pfi_trk_t *trk)
{
	trk->cur_pos = 0;
	trk->cur_clk = 0;
	trk->cur_idx = 0;
}

int pfi_trk_get_pulse (pfi_trk_t *trk, uint32_t *val, uint32_t *idx)
{
	if (trk->cur_pos >= trk->pulse_cnt) {
		if (trk->cur_idx < trk->index_cnt) {
			*idx = trk->index[trk->cur_idx] - trk->cur_clk;
			*val = 0;

			trk->cur_idx += 1;

			return (0);
		}

		return (1);
	}

	*val = trk->pulse[trk->cur_pos];
	*idx = -1;

	if (trk->cur_idx < trk->index_cnt) {
		*idx = trk->index[trk->cur_idx] - trk->cur_clk;

		if ((trk->cur_clk + *val) > trk->index[trk->cur_idx]) {
			trk->cur_idx += 1;
		}
	}

	trk->cur_pos += 1;
	trk->cur_clk += *val;

	return (0);
}

pfi_trk_t *pfi_trk_scale (pfi_trk_t *trk, unsigned long mul, unsigned long div)
{
	uint32_t           val, idx, clk;
	unsigned long      v;
	unsigned long long m, d, t, r;
	pfi_trk_t          *ret;

	if ((ret = pfi_trk_clone (trk, 0, 0)) == NULL) {
		return (NULL);
	}

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

		if (v == 0) {
			continue;
		}

		if (pfi_trk_add_pulse (ret, v)) {
			break;
		}

		clk += v;
	}

	if (trk->cur_pos < trk->pulse_cnt) {
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

	for (i = 0; i < trk->index_cnt; i++) {
		if (ofs < 0) {
			if (val <= trk->index[i]) {
				trk->index[j++] = trk->index[i] - val;
			}
		}
		else {
			trk->index[j++] = trk->index[i] + val;
		}
	}

	trk->index_cnt = j;
}
