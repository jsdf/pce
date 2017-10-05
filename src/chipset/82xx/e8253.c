/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8253.c                                     *
 * Created:     2001-05-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2001-2017 Hampa Hug <hampa@hampa.ch>                     *
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

#include "e8253.h"


static
void cnt_set_out (e8253_counter_t *cnt, unsigned char val)
{
	if (cnt->out_val != val) {
		cnt->out_val = val;

		if (cnt->out != NULL) {
			cnt->out (cnt->out_ext, val);
		}
	}
}


/*****************************************************************************
 * mode 0
 *****************************************************************************/

static
void cnt_mode0_gate (e8253_counter_t *cnt, unsigned char val)
{
	cnt->counting = val;
}

static
void cnt_mode0_load (e8253_counter_t *cnt)
{
	if (cnt->cr_wr == 0) {
		cnt->newval = 1;
	}

	cnt->counting = 0;

	cnt_set_out (cnt, 0);
}

static
void cnt_mode0_clock (e8253_counter_t *cnt)
{
	if (cnt->newval) {
		cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];
		cnt->counting = cnt->gate_val;
		cnt->newval = 0;
	}
	else if (cnt->counting) {
		cnt->ce = (cnt->ce - 1) & 0xffff;

		if (cnt->ce == 0) {
			cnt_set_out (cnt, 1);
		}
	}
}

static
void cnt_mode0_init (e8253_counter_t *cnt)
{
	cnt->gate = cnt_mode0_gate;
	cnt->load = cnt_mode0_load;
	cnt->clock = cnt_mode0_clock;

	cnt->counting = 0;

	cnt_set_out (cnt, 0);
}


/*****************************************************************************
 * mode 1
 *****************************************************************************/

static
void cnt_mode1_gate (e8253_counter_t *cnt, unsigned char val)
{
	if ((cnt->gate_val == 0) && val) {
		cnt->newval = 1;
	}
}

static
void cnt_mode1_load (e8253_counter_t *cnt)
{
}

static
void cnt_mode1_clock (e8253_counter_t *cnt)
{
	if (cnt->newval) {
		cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];
		cnt->counting = 1;
		cnt->newval = 0;
		cnt_set_out (cnt, 0);
	}
	else if (cnt->counting) {
		cnt->ce = (cnt->ce - 1) & 0xffff;

		if (cnt->ce == 0) {
			cnt_set_out (cnt, 1);
		}
	}
}

static
void cnt_mode1_init (e8253_counter_t *cnt)
{
	cnt->gate = cnt_mode1_gate;
	cnt->load = cnt_mode1_load;
	cnt->clock = cnt_mode1_clock;

	cnt->counting = 0;

	cnt_set_out (cnt, 1);
}


/*****************************************************************************
 * mode 2
 *****************************************************************************/

static
void cnt_mode2_gate (e8253_counter_t *cnt, unsigned char val)
{
	if (val) {
		cnt->newval = 1;
	}
	else {
		cnt_set_out (cnt, 1);
	}

	cnt->counting = val;
}

static
void cnt_mode2_load (e8253_counter_t *cnt)
{
	if (cnt->counting) {
		return;
	}

	if (cnt->cr_wr == 0) {
		cnt->newval = 1;
	}
}

static
void cnt_mode2_clock (e8253_counter_t *cnt)
{
	if (cnt->newval) {
		cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];
		cnt->counting = cnt->gate_val;
		cnt->newval = 0;
	}
	else if (cnt->counting) {
		cnt->ce = (cnt->ce - 1) & 0xffff;

		if (cnt->ce == 1) {
			cnt_set_out (cnt, 0);
		}
		else if (cnt->ce == 0) {
			cnt_set_out (cnt, 1);
			cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];
		}
	}
}

static
void cnt_mode2_init (e8253_counter_t *cnt)
{
	cnt->gate = cnt_mode2_gate;
	cnt->load = cnt_mode2_load;
	cnt->clock = cnt_mode2_clock;

	cnt->counting = 0;

	cnt_set_out (cnt, 1);
}


/*****************************************************************************
 * mode 3
 *****************************************************************************/

static void cnt_mode3_clock (e8253_counter_t *cnt);

static
void cnt_mode3_gate (e8253_counter_t *cnt, unsigned char val)
{
	if (val) {
		cnt->newval = 1;
	}
	else {
		cnt_set_out (cnt, 1);
	}

	cnt->counting = val;
}

static
void cnt_mode3_load (e8253_counter_t *cnt)
{
	if (cnt->counting) {
		return;
	}

	if (cnt->cr_wr == 0) {
		cnt->newval = 1;
	}
}

static
void cnt_mode3_clock0 (e8253_counter_t *cnt)
{
	cnt_set_out (cnt, !cnt->out_val);
	cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];

	cnt->clock = cnt_mode3_clock;
}

static
void cnt_mode3_clock (e8253_counter_t *cnt)
{
	if (cnt->newval) {
		cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];
		cnt->counting = cnt->gate_val;
		cnt->newval = 0;
	}
	else if (cnt->counting) {
		cnt->ce = (cnt->ce - 2) & 0xfffe;

		if (cnt->ce == 0) {
			if (cnt->out_val && (cnt->cr[0] & 1)) {
				cnt->clock = cnt_mode3_clock0;
			}
			else {
				cnt_mode3_clock0 (cnt);
			}
		}
	}
}

static
void cnt_mode3_init (e8253_counter_t *cnt)
{
	cnt->gate = cnt_mode3_gate;
	cnt->load = cnt_mode3_load;
	cnt->clock = cnt_mode3_clock;

	cnt->counting = 0;

	cnt_set_out (cnt, 1);
}


/*****************************************************************************
 * mode 4
 *****************************************************************************/

static
void cnt_mode4_gate (e8253_counter_t *cnt, unsigned char val)
{
	cnt->counting = val;
}

static
void cnt_mode4_load (e8253_counter_t *cnt)
{
	if (cnt->cr_wr == 0) {
		cnt->newval = 1;
	}

	cnt_set_out (cnt, 1);
}

static
void cnt_mode4_clock (e8253_counter_t *cnt)
{
	if (cnt->newval) {
		cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];
		cnt->counting = cnt->gate_val;
		cnt->newval = 0;
	}
	else if (cnt->counting) {
		cnt->ce = (cnt->ce - 1) & 0xffff;

		if (cnt->ce == 0) {
			cnt_set_out (cnt, 0);
		}
		else if (cnt->ce == 0xffff) {
			cnt_set_out (cnt, 1);
		}
	}
}

static
void cnt_mode4_init (e8253_counter_t *cnt)
{
	cnt->gate = cnt_mode4_gate;
	cnt->load = cnt_mode4_load;
	cnt->clock = cnt_mode4_clock;

	cnt->counting = 0;

	cnt_set_out (cnt, 1);
}


/*****************************************************************************
 * mode 5
 *****************************************************************************/

static
void cnt_mode5_gate (e8253_counter_t *cnt, unsigned char val)
{
	if ((cnt->gate_val == 0) && val) {
		cnt->newval = 1;
	}
}

static
void cnt_mode5_load (e8253_counter_t *cnt)
{
}

static
void cnt_mode5_clock (e8253_counter_t *cnt)
{
	if (cnt->newval) {
		cnt->ce = ((unsigned) cnt->cr[1] << 8) | cnt->cr[0];
		cnt->counting = 1;
		cnt->newval = 0;
	}
	else if (cnt->counting) {
		cnt->ce = (cnt->ce - 1) & 0xffff;

		if (cnt->ce == 0) {
			cnt_set_out (cnt, 0);
		}
		else if (cnt->ce == 0xffff) {
			cnt_set_out (cnt, 1);
		}
	}
}

static
void cnt_mode5_init (e8253_counter_t *cnt)
{
	cnt->gate = cnt_mode5_gate;
	cnt->load = cnt_mode5_load;
	cnt->clock = cnt_mode5_clock;

	cnt->counting = 0;

	cnt_set_out (cnt, 1);
}


static
unsigned char e8253_cnt_get_uint8 (e8253_counter_t *cnt)
{
	if (cnt->ol_rd & 1) {
		cnt->ol_rd &= ~1;
		return (cnt->ol[0]);
	}

	if (cnt->ol_rd & 2) {
		cnt->ol_rd &= ~2;
		return (cnt->ol[1]);
	}

	if (cnt->cnt_rd == 0) {
		cnt->cnt_rd = cnt->rw;
	}

	if (cnt->cnt_rd & 1) {
		cnt->cnt_rd &= ~1;
		return (cnt->ce & 0xff);
	}

	cnt->cnt_rd &= ~2;

	return ((cnt->ce >> 8) & 0xff);
}

static
unsigned short e8253_cnt_get_uint16 (e8253_counter_t *cnt)
{
	return (cnt->ce & 0xffff);
}

static
void e8253_cnt_set_uint8 (e8253_counter_t *cnt, unsigned char val)
{
	if (cnt->cr_wr == 0) {
		cnt->cr_wr = cnt->rw;
	}

	if (cnt->cr_wr & 1) {
		cnt->cr_wr &= ~1;
		cnt->cr[0] = val;
	}
	else if (cnt->cr_wr & 2) {
		cnt->cr_wr &= ~2;
		cnt->cr[1] = val;
	}

	if (cnt->load != NULL) {
		cnt->load (cnt);
	}
}

static
void e8253_cnt_set_gate (e8253_counter_t *cnt, unsigned char val)
{
	if (val != cnt->gate_val) {
		if (cnt->gate != NULL) {
			cnt->gate (cnt, val);
		}

		cnt->gate_val = val;
	}
}

static
void cnt_latch (e8253_counter_t *cnt)
{
	if (cnt->ol_rd) {
		return;
	}

	cnt->ol_rd = cnt->rw;

	cnt->ol[0] = cnt->ce & 0xff;
	cnt->ol[1] = (cnt->ce >> 8) & 0xff;
}

static
void cnt_set_control (e8253_counter_t *cnt, unsigned char val)
{
	unsigned char rw;

	if ((val & 0xc0) == 0xc0) {
		/* read back on 8254 */
		return;
	}

	rw = (val >> 4) & 3;

	if (rw == 0) {
		cnt_latch (cnt);
		return;
	}

	cnt->sr = val;

	cnt->cr_wr = rw;
	cnt->ol_rd = 0;
	cnt->cnt_rd = rw;

	cnt->cr[0] = 0;
	cnt->cr[1] = 0;

	cnt->counting = 0;
	cnt->newval = 0;

	cnt->rw = rw;
	cnt->mode = (val >> 1) & 7;
	cnt->bcd = (val & 1);

	cnt->load = NULL;
	cnt->clock = NULL;
	cnt->gate = NULL;

	switch (cnt->mode) {
	case 0:
		cnt_mode0_init (cnt);
		break;

	case 1:
		cnt_mode1_init (cnt);
		break;

	case 2:
	case 6:
		cnt->mode = 2;
		cnt_mode2_init (cnt);
		break;

	case 3:
	case 7:
		cnt->mode = 3;
		cnt_mode3_init (cnt);
		break;

	case 4:
		cnt_mode4_init (cnt);
		break;

	case 5:
		cnt_mode5_init (cnt);
		break;

	default:
		fprintf (stderr, "8253: unsupported mode (%u)\n", cnt->mode);
		break;
	}
}

static
void e8253_counter_reset (e8253_counter_t *cnt)
{
	cnt->ce = 0;
	cnt->cr[0] = 0;
	cnt->cr[1] = 0;
	cnt->cr_wr = 0;

	cnt->ol[0] = 0;
	cnt->ol[1] = 0;
	cnt->ol_rd = 0;
	cnt->cnt_rd = 0;

	cnt->rw = 3;
	cnt->mode = 0;
	cnt->bcd = 0;

	cnt->counting = 0;
	cnt->newval = 0;

	cnt->load = NULL;
	cnt->clock = NULL;
	cnt->gate = NULL;

	cnt_set_out (cnt, 0);
}

static
void e8253_counter_init (e8253_counter_t *cnt)
{
	cnt->ce = 0;
	cnt->cr[0] = 0;
	cnt->cr[1] = 0;
	cnt->cr_wr = 0;

	cnt->ol[0] = 0;
	cnt->ol[1] = 0;
	cnt->ol_rd = 0;
	cnt->cnt_rd = 0;

	cnt->rw = 3;
	cnt->mode = 0;
	cnt->bcd = 0;

	cnt->counting = 0;
	cnt->newval = 0;
	cnt->gate_val = 0;

	cnt->load = NULL;
	cnt->clock = NULL;
	cnt->gate = NULL;

	cnt->out_ext = NULL;
	cnt->out_val = 0;
	cnt->out = NULL;
}


void e8253_init (e8253_t *pit)
{
	e8253_counter_init (&pit->counter[0]);
	e8253_counter_init (&pit->counter[1]);
	e8253_counter_init (&pit->counter[2]);
}

void e8253_free (e8253_t *pit)
{
}

e8253_t *e8253_new (void)
{
	e8253_t *pit;

	if ((pit = malloc (sizeof (e8253_t))) == NULL) {
		return (NULL);
	}

	e8253_init (pit);

	return (pit);
}

void e8253_del (e8253_t *pit)
{
	if (pit != NULL) {
		e8253_free (pit);
		free (pit);
	}
}

void e8253_set_out_fct (e8253_t *pit, unsigned cntr, void *ext, void *fct)
{
	if (cntr <= 2) {
		pit->counter[cntr].out_ext = ext;
		pit->counter[cntr].out = fct;
	}
}

void e8253_set_gate (e8253_t *pit, unsigned cntr, unsigned char val)
{
	if (cntr <= 2) {
		e8253_cnt_set_gate (&pit->counter[cntr], val);
	}
}

void e8253_set_gate0 (e8253_t *pit, unsigned char val)
{
	e8253_cnt_set_gate (&pit->counter[0], val);
}

void e8253_set_gate1 (e8253_t *pit, unsigned char val)
{
	e8253_cnt_set_gate (&pit->counter[1], val);
}

void e8253_set_gate2 (e8253_t *pit, unsigned char val)
{
	e8253_cnt_set_gate (&pit->counter[2], val);
}

unsigned char e8253_get_uint8 (e8253_t *pit, unsigned long addr)
{
	if (addr < 3) {
		return (e8253_cnt_get_uint8 (&pit->counter[addr]));
	}

	return (0xff);
}

unsigned short e8253_get_uint16 (e8253_t *pit, unsigned long addr)
{
	if (addr < 3) {
		return (e8253_cnt_get_uint16 (&pit->counter[addr]));
	}

	return (0xffff);
}

unsigned long e8253_get_uint32 (e8253_t *pit, unsigned long addr)
{
	return (e8253_get_uint8 (pit, addr));
}

void e8253_set_uint8 (e8253_t *pit, unsigned long addr, unsigned char val)
{
	unsigned idx;

	if (addr < 3) {
		e8253_cnt_set_uint8 (&pit->counter[addr], val);
	}
	else if (addr == 3) {
		idx = (val >> 6) & 3;

		if (idx < 3) {
			cnt_set_control (&pit->counter[idx], val);
		}
		else {
			; /* Read back on 8254 */
		}
	}
}

void e8253_set_uint16 (e8253_t *pit, unsigned long addr, unsigned short val)
{
	e8253_set_uint8 (pit, addr, val & 0xff);
}

void e8253_set_uint32 (e8253_t *pit, unsigned long addr, unsigned long val)
{
	e8253_set_uint8 (pit, addr, val & 0xff);
}

void e8253_reset (e8253_t *pit)
{
	e8253_counter_reset (&pit->counter[0]);
	e8253_counter_reset (&pit->counter[1]);
	e8253_counter_reset (&pit->counter[2]);
}

void e8253_clock (e8253_t *pit, unsigned n)
{
	e8253_counter_t *cnt;

	while (n > 0) {
		cnt = pit->counter;

		if (cnt[0].clock != NULL) {
			cnt[0].clock (cnt + 0);
		}

		if (cnt[1].clock != NULL) {
			cnt[1].clock (cnt + 1);
		}

		if (cnt[2].clock != NULL) {
			cnt[2].clock (cnt + 2);
		}

		n -= 1;
	}
}
