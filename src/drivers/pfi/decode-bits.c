/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/decode-bits.c                                *
 * Created:     2012-01-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfi.h"
#include "decode-bits.h"


int pfi_dec_init (pfi_dec_t *buf, unsigned long max)
{

	buf->buf = NULL;
	buf->weak = NULL;
	buf->clk = NULL;

	buf->cnt = 0;
	buf->max = 0;

	buf->index = 0;

	if (max > 0) {
		if (pfi_dec_alloc_bits (buf, max)) {
			pfi_dec_free (buf);
			return (1);
		}
	}

	return (0);
}

void pfi_dec_free (pfi_dec_t *buf)
{
	free (buf->clk);
	free (buf->weak);
	free (buf->buf);
}

int pfi_dec_alloc_bits (pfi_dec_t *buf, unsigned long cnt)
{
	unsigned long max;
	void          *tmp;

	if ((buf->cnt + cnt) <= buf->max) {
		return (0);
	}

	max = (buf->max < 4096) ? 4096 : buf->max;

	while ((buf->cnt + cnt) > max) {
		max *= 2;
	}

	if ((tmp = realloc (buf->buf, (max + 7) / 8)) == NULL) {
		return (1);
	}

	buf->buf = tmp;

	if ((tmp = realloc (buf->weak, (max + 7) / 8)) == NULL) {
		return (1);
	}

	buf->weak = tmp;

	if ((tmp = realloc (buf->clk, sizeof (unsigned long) * max)) == NULL) {
		return (1);
	}

	buf->clk = tmp;

	buf->max = max;

	return (0);
}

int pfi_dec_add_bit (pfi_dec_t *buf, int val, int weak, unsigned long clk)
{
	unsigned long i;
	unsigned char m;

	if (pfi_dec_alloc_bits (buf, 1)) {
		return (1);
	}

	i = buf->cnt / 8;
	m = 0x80 >> (buf->cnt & 7);

	if (val) {
		buf->buf[i] |= m;
	}
	else {
		buf->buf[i] &= ~m;
	}

	if (weak) {
		buf->weak[i] |= m;
	}
	else {
		buf->weak[i] &= ~m;
	}

	buf->clk[buf->cnt] = clk;

	buf->cnt += 1;

	return (0);
}

void pfi_dec_clock_average (pfi_dec_t *bit)
{
	unsigned long i;
	unsigned long v[5];

	v[0] = bit->clk[0];
	v[1] = bit->clk[0];
	v[2] = bit->clk[0];
	v[3] = bit->clk[0];
	v[4] = (bit->cnt >= 1) ? bit->clk[1] : bit->clk[0];

	for (i = 0; i < bit->cnt; i++) {
		v[0] = v[1];
		v[1] = v[2];
		v[2] = v[3];
		v[3] = v[4];
		v[4] = ((i + 2) < bit->cnt) ? bit->clk[i + 2] : v[4];

		bit->clk[i] = (v[0] + v[1] + v[2] + v[3] + v[4] + 2) / 5;
	}
}

void pfi_dec_clock_median (pfi_dec_t *bit)
{
	unsigned long i, j, k;
	unsigned long v[3], t[3];

	v[0] = bit->clk[0];
	v[1] = bit->clk[0];
	v[2] = bit->clk[0];

	for (i = 0; i < bit->cnt; i++) {
		v[0] = v[1];
		v[1] = v[2];
		v[2] = ((i + 1) < bit->cnt) ? bit->clk[i + 1] : v[2];

		for (j = 0; j < 3; j++) {
			k = j;

			while ((k > 0) && (v[j] < t[k - 1])) {
				t[k] = t[k - 1];
				k -= 1;
			}

			t[k] = v[j];
		}

		bit->clk[i] = t[1];
	}
}

int pfi_trk_decode_bits (pfi_trk_t *trk, pfi_dec_t *dst, unsigned long rate, unsigned rev)
{
	unsigned      idx;
	uint32_t      val, ofs;
	unsigned char weak;
	double        cell, cell0, pulse;
	double        adjust1, adjust2, limit1, limit2, phase;

	pfi_trk_rewind (trk);

	if (rev < 1) {
		rev = 1;
	}

	cell0 = (double) trk->clock / rate;

	ofs = 0;
	idx = 0;

	cell = cell0;
	pulse = 0.0;

	adjust1 = 0.9995;
	adjust2 = 1.0005;

	limit1 = 0.9 * cell0;
	limit2 = 1.1 * cell0;

	phase = 0.5;

#if 0
	phase = -0.25;
	adjust1 *= adjust1;
	adjust2 *= adjust2;
	limit1 = 0.9 * cell0;
	limit2 = 1.1 * cell0;
#endif

	weak = 0;

	while (pfi_trk_get_pulse (trk, &val, &ofs) == 0) {
		if ((val == 0) || (ofs < val)) {
			idx += 1;

			if (idx == rev) {
				dst->cnt = 0;
			}
			else if (idx == (rev + 1)) {
				dst->index = dst->cnt;
			}
			else if (idx > (rev + 1)) {
				break;
			}
		}

		if (val == 0) {
			continue;
		}

		pulse += (double) val / cell;

		while (pulse > 1.5) {
			if (pulse < 1.6) {
				weak = 3;
			}

			pfi_dec_add_bit (dst, 0, weak & 1, trk->clock / cell);

			weak >>= 1;
			pulse -= 1.0;
		}

		if (pulse > 1.4) {
			weak = 3;
		}

		pfi_dec_add_bit (dst, 1, weak & 1, trk->clock / cell);

		weak >>= 1;
		pulse -= 1.0;

		if (pulse < 0.0) {
			cell *= adjust1;

			if (cell < limit1) {
				cell = limit1;
			}
		}
		else if (pulse > 0.0) {
			cell *= adjust2;

			if (cell > limit2) {
				cell = limit2;
			}
		}

		pulse *= phase;
	}

	return (0);
}
