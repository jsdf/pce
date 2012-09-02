/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/80186/dma.c                                      *
 * Created:     2012-06-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#include "dma.h"

#include <stdio.h>
#include <stdlib.h>


#define DMA_CTL_DMEM 0x8000
#define DMA_CTL_DDEC 0x4000
#define DMA_CTL_DINC 0x2000
#define DMA_CTL_SMEM 0x1000
#define DMA_CTL_SDEC 0x0800
#define DMA_CTL_SINC 0x0400
#define DMA_CTL_TC   0x0200
#define DMA_CTL_INT  0x0100
#define DMA_CTL_SYN1 0x0080
#define DMA_CTL_SYN0 0x0040
#define DMA_CTL_P    0x0020
#define DMA_CTL_IDRQ 0x0010
#define DMA_CTL_CHG  0x0004
#define DMA_CTL_STRT 0x0002
#define DMA_CTL_WORD 0x0001


void e80186_dma_init (e80186_dma_t *dma)
{
	dma->dreq[0] = 0;
	dma->dreq[1] = 0;

	dma->check = 0;

	dma->get_mem_ext = NULL;
	dma->get_mem8 = NULL;
	dma->get_mem16 = NULL;

	dma->set_mem_ext = NULL;
	dma->set_mem8 = NULL;
	dma->set_mem16 = NULL;

	dma->get_io_ext = NULL;
	dma->get_io8 = NULL;
	dma->get_io16 = NULL;

	dma->set_io_ext = NULL;
	dma->set_io8 = NULL;
	dma->set_io16 = NULL;

	dma->int0_val = 0;
	dma->int0_ext = NULL;
	dma->int0 = NULL;

	dma->int1_val = 0;
	dma->int1_ext = NULL;
	dma->int1 = NULL;
}

void e80186_dma_free (e80186_dma_t *dma)
{
}

void e80186_dma_set_getmem_fct (e80186_dma_t *dma, void *ext, void *get8, void *get16)
{
	dma->get_mem_ext = ext;
	dma->get_mem8 = get8;
	dma->get_mem16 = get16;
}

void e80186_dma_set_setmem_fct (e80186_dma_t *dma, void *ext, void *set8, void *set16)
{
	dma->set_mem_ext = ext;
	dma->set_mem8 = set8;
	dma->set_mem16 = set16;
}

void e80186_dma_set_getio_fct (e80186_dma_t *dma, void *ext, void *get8, void *get16)
{
	dma->get_io_ext = ext;
	dma->get_io8 = get8;
	dma->get_io16 = get16;
}

void e80186_dma_set_setio_fct (e80186_dma_t *dma, void *ext, void *set8, void *set16)
{
	dma->set_io_ext = ext;
	dma->set_io8 = set8;
	dma->set_io16 = set16;
}

void e80186_dma_set_int_fct (e80186_dma_t *dma, unsigned idx, void *ext, void *fct)
{
	if (idx == 0) {
		dma->int0_ext = ext;
		dma->int0 = fct;
	}
	else if (idx == 1) {
		dma->int1_ext = ext;
		dma->int1 = fct;
	}
}

void e80186_dma_reset (e80186_dma_t *dma)
{
	unsigned i;

	for (i = 0; i < 2; i++) {
		dma->control[i] = 0;
		dma->count[i] = 0;
		dma->src[i] = 0;
		dma->dst[i] = 0;
	}

	dma->check = 0;

	dma->delay = 0;
}

static
void e80186_dma_set_int0 (e80186_dma_t *dma, int val)
{
	val = (val != 0);

	if (dma->int0_val == val) {
		return;
	}

	dma->int0_val = val;

	if (dma->int0 != NULL) {
		dma->int0 (dma->int0_ext, val);
	}
}

static
void e80186_dma_set_int1 (e80186_dma_t *dma, int val)
{
	val = (val != 0);

	if (dma->int1_val == val) {
		return;
	}

	dma->int1_val = val;

	if (dma->int1 != NULL) {
		dma->int1 (dma->int1_ext, val);
	}
}

static
void e80186_dma_set_int (e80186_dma_t *dma, unsigned idx, int val)
{
	if (idx == 0) {
		e80186_dma_set_int0 (dma, val);
	}
	else {
		e80186_dma_set_int1 (dma, val);
	}
}

void e80186_dma_set_dreq0 (e80186_dma_t *dma, unsigned char val)
{
	dma->dreq[0] = (val != 0);
	dma->check |= 1;
}

void e80186_dma_set_dreq1 (e80186_dma_t *dma, unsigned char val)
{
	dma->dreq[1] = (val != 0);
	dma->check |= 2;
}

unsigned long e80186_dma_get_src (const e80186_dma_t *dma, unsigned idx)
{
	return (dma->src[idx & 1]);
}

unsigned short e80186_dma_get_src_hi (const e80186_dma_t *dma, unsigned idx)
{
	return ((dma->src[idx & 1] >> 16) & 0x000f);
}

void e80186_dma_set_src_hi (e80186_dma_t *dma, unsigned idx, unsigned short val)
{
	idx &= 1;
	dma->src[idx] &= 0xffff;
	dma->src[idx] |= (unsigned long) (val & 0x000f) << 16;
}

unsigned short e80186_dma_get_src_lo (const e80186_dma_t *dma, unsigned idx)
{
	return (dma->src[idx & 1] & 0xffff);
}

void e80186_dma_set_src_lo (e80186_dma_t *dma, unsigned idx, unsigned short val)
{
	idx &= 1;
	dma->src[idx] &= 0xffff0000;
	dma->src[idx] |= val & 0xffff;
}

unsigned long e80186_dma_get_dst (const e80186_dma_t *dma, unsigned idx)
{
	return (dma->dst[idx & 1]);
}

unsigned short e80186_dma_get_dst_hi (const e80186_dma_t *dma, unsigned idx)
{
	return ((dma->dst[idx & 1] >> 16) & 0x000f);
}

void e80186_dma_set_dst_hi (e80186_dma_t *dma, unsigned idx, unsigned short val)
{
	idx &= 1;
	dma->dst[idx] &= 0xffff;
	dma->dst[idx] |= (unsigned long) (val & 0x000f) << 16;
}

unsigned short e80186_dma_get_dst_lo (const e80186_dma_t *dma, unsigned idx)
{
	return (dma->dst[idx & 1] & 0xffff);
}

void e80186_dma_set_dst_lo (e80186_dma_t *dma, unsigned idx, unsigned short val)
{
	idx &= 1;
	dma->dst[idx] &= 0xffff0000;
	dma->dst[idx] |= val & 0xffff;
}

unsigned short e80186_dma_get_control (const e80186_dma_t *dma, unsigned idx)
{
	return (dma->control[idx & 1]);
}

void e80186_dma_set_control (e80186_dma_t *dma, unsigned idx, unsigned short val)
{
	idx &= 1;

	e80186_dma_set_int (dma, idx, 0);

	if (val & DMA_CTL_CHG) {
		dma->control[idx] = val;
	}
	else {
		dma->control[idx] = (dma->control[idx] & DMA_CTL_STRT) | (val & ~DMA_CTL_STRT);
	}

	dma->check |= 1U << idx;
}

unsigned short e80186_dma_get_count (const e80186_dma_t *dma, unsigned idx)
{
	return (dma->count[idx & 1]);
}

void e80186_dma_set_count (e80186_dma_t *dma, unsigned idx, unsigned short val)
{
	e80186_dma_set_int (dma, idx, 0);

	dma->count[idx & 1] = val;
}

static
void e80186_dma_clock_chn (e80186_dma_t *dma, unsigned idx)
{
	unsigned short ctl;
	unsigned short val;
	unsigned long  inc;

	dma->check &= ~(1U << idx);

	ctl = dma->control[idx];

	if ((ctl & DMA_CTL_STRT) == 0) {
		return;
	}

	if (((ctl >> 6) & 3) == 0) {
		;
	}
	else if (ctl & DMA_CTL_IDRQ) {
		return;
	}
	else {
		if (dma->dreq[idx] == 0) {
			return;
		}
	}

	dma->check |= 1U << idx;

	val = 0;

	if (ctl & DMA_CTL_SMEM) {
		if (ctl & DMA_CTL_WORD) {
			if (dma->get_mem16 != NULL) {
				val = dma->get_mem16 (dma->get_mem_ext, dma->src[idx]);
			}
		}
		else {
			if (dma->get_mem8 != NULL) {
				val = dma->get_mem8 (dma->get_mem_ext, dma->src[idx]);
			}
		}
	}
	else {
		if (ctl & DMA_CTL_WORD) {
			if (dma->get_io16 != NULL) {
				val = dma->get_io16 (dma->get_io_ext, dma->src[idx]);
			}
		}
		else {
			if (dma->get_io8 != NULL) {
				val = dma->get_io8 (dma->get_io_ext, dma->src[idx]);
			}
		}
	}

	if (ctl & DMA_CTL_DMEM) {
		if (ctl & DMA_CTL_WORD) {
			if (dma->set_mem16 != NULL) {
				dma->set_mem16 (dma->set_mem_ext, dma->dst[idx], val);
			}
		}
		else {
			if (dma->set_mem8 != NULL) {
				dma->set_mem8 (dma->set_mem_ext, dma->dst[idx], val);
			}
		}
	}
	else {
		if (ctl & DMA_CTL_WORD) {
			if (dma->set_io16 != NULL) {
				dma->set_io16 (dma->set_io_ext, dma->dst[idx], val);
			}
		}
		else {
			if (dma->set_io8 != NULL) {
				dma->set_io8 (dma->set_io_ext, dma->dst[idx], val);
			}
		}
	}

	inc = (ctl & DMA_CTL_WORD) ? 2 : 1;

	if ((ctl & DMA_CTL_SINC) && (~ctl & DMA_CTL_SDEC)) {
		dma->src[idx] += inc;
	}
	else if ((~ctl & DMA_CTL_SINC) && (ctl & DMA_CTL_SDEC)) {
		dma->src[idx] -= inc;
	}

	if ((ctl & DMA_CTL_DINC) && (~ctl & DMA_CTL_DDEC)) {
		dma->dst[idx] += inc;
	}
	else if ((~ctl & DMA_CTL_DINC) && (ctl & DMA_CTL_DDEC)) {
		dma->dst[idx] -= inc;
	}

	dma->count[idx] = (dma->count[idx] - 1) & 0xffff;

	if (dma->count[idx] == 0) {
		if (ctl & DMA_CTL_TC) {
			dma->control[idx] &= ~DMA_CTL_STRT;

			if (ctl & DMA_CTL_INT) {
				e80186_dma_set_int (dma, idx, 1);
			}
		}
	}

	dma->delay += 8;

	if (((ctl >> 6) & 3) == 2) {
		dma->delay += 2;
	}
}

void e80186_dma_clock2 (e80186_dma_t *dma, unsigned cnt)
{
	while (cnt >= dma->delay) {
		cnt -= dma->delay;
		dma->delay = 0;

		e80186_dma_clock_chn (dma, 0);
		e80186_dma_clock_chn (dma, 1);

		if (dma->delay == 0) {
			return;
		}
	}

	dma->delay -= cnt;
}
