/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/dma.c                                       *
 * Created:     2013-06-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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
#include "acsi.h"
#include "dma.h"

#include <chipset/wd179x.h>
#include <devices/memory.h>

#ifndef DEBUG_DMA
#define DEBUG_DMA 0
#endif


int st_dma_init (st_dma_t *dma)
{
	dma->mode = 0;
	dma->status = 1;
	dma->addr = 0;

	dma->fifo_idx = 0;

	dma->mem = NULL;
	dma->fdc = NULL;
	dma->acsi = NULL;

	return (0);
}

void st_dma_set_memory (st_dma_t *dma, memory_t *mem)
{
	dma->mem = mem;
}

void st_dma_set_fdc (st_dma_t *dma, wd179x_t *fdc)
{
	dma->fdc = fdc;
}

void st_dma_set_acsi (st_dma_t *dma, st_acsi_t *acsi)
{
	dma->acsi = acsi;
}

static
void st_dma_dreq_read (st_dma_t *dma)
{
	unsigned      i;
	unsigned char val;
	st_dma_fifo_t *fifo;

	if (dma->sector_cnt == 0) {
		return;
	}

#if DEBUG_DMA >= 1
	if (dma->dma_first) {
		st_log_deb ("DMA: READ %s (A=%06lX N=%u)\n",
			(dma->mode & 0x80) ? "FDC" : "ACSI",
			dma->addr, dma->sector_cnt
		);

		dma->dma_first = 0;
	}
#endif

	if (dma->mode & 0x80) {
		val = wd179x_get_data (dma->fdc);
	}
	else {
		val = st_acsi_get_data (dma->acsi);
	}

#if DEBUG_DMA >= 2
	st_log_deb ("DMA: read %02X (bc=%u addr=%06lX)\n",
		val, dma->byte_cnt, dma->addr
	);
#endif

	fifo = &dma->fifo[dma->fifo_idx];

	fifo->data[fifo->cnt++] = val;

	if (fifo->cnt >= 16) {
		for (i = 0; i < 16; i++) {
			mem_set_uint8 (dma->mem, dma->addr, fifo->data[i]);
			dma->addr += 1;
		}

		dma->fifo_idx = (dma->fifo_idx + 1) & 1;
		dma->fifo[dma->fifo_idx].idx = 0;
		dma->fifo[dma->fifo_idx].cnt = 0;
	}

	dma->byte_cnt += 1;

	if (dma->byte_cnt >= 512) {
		dma->byte_cnt = 0;
		dma->sector_cnt -= 1;

		if (dma->sector_cnt == 0) {
			dma->status &= ~2;
		}
	}
}

static
void st_dma_fifo_fill (st_dma_t *dma, unsigned idx)
{
	unsigned      i;
	st_dma_fifo_t *fifo;

	fifo = &dma->fifo[idx];

	for (i = 0; i < 16; i++) {
		fifo->data[i] = mem_get_uint8 (dma->mem, dma->addr);
		dma->addr += 1;
	}

	fifo->idx = 0;
	fifo->cnt = 16;
}

static
void st_dma_dreq_write (st_dma_t *dma)
{
	unsigned char val;
	st_dma_fifo_t *fifo;

	if (dma->sector_cnt == 0) {
		return;
	}

#if DEBUG_DMA >= 1
	if (dma->dma_first) {
		st_log_deb ("DMA: WRITE %s (A=%06lX N=%u)\n",
			(dma->mode & 0x80) ? "FDC" : "ACSI",
			dma->addr - 32, dma->sector_cnt
		);

		dma->dma_first = 0;
	}
#endif

	fifo = &dma->fifo[dma->fifo_idx];

	val = fifo->data[fifo->idx++];

	if (dma->mode & 0x80) {
		wd179x_set_data (dma->fdc, val);
	}
	else {
		st_acsi_set_data (dma->acsi, val);
	}

	dma->byte_cnt += 1;

	if (dma->byte_cnt >= 512) {
		dma->byte_cnt = 0;
		dma->sector_cnt -= 1;

		if (dma->sector_cnt == 0) {
			dma->status &= ~2;
			return;
		}
	}

	if (fifo->idx >= 16) {
		st_dma_fifo_fill (dma, dma->fifo_idx);
		dma->fifo_idx = (dma->fifo_idx + 1) & 1;
	}
}

void st_dma_set_dreq (st_dma_t *dma, unsigned char val)
{
	if (val == 0) {
		dma->status &= ~4;
		return;
	}

#if DEBUG_DMA >= 2
	st_log_deb ("DMA: dreq = %u\n", val);
#endif

	dma->status |= 4;

	if (dma->mode & 0x100) {
		st_dma_dreq_write (dma);
	}
	else {
		st_dma_dreq_read (dma);
	}
}

unsigned short st_dma_get_status (st_dma_t *dma)
{
	return (dma->status);
}

void st_dma_set_mode (st_dma_t *dma, unsigned short val)
{
#if DEBUG_DMA >= 2
	st_log_deb ("DMA: set mode = %04X\n", val);
#endif

	if ((dma->mode ^ val) & 0x100) {
		dma->status = 1;
		dma->sector_cnt = 0;
		dma->byte_cnt = 0;
		dma->fifo_idx = 0;
		dma->fifo[0].idx = 0;
		dma->fifo[0].cnt = 0;
		dma->fifo[1].idx = 0;
		dma->fifo[1].cnt = 0;
	}

	dma->mode = val;
}

unsigned short st_dma_get_disk (st_dma_t *dma)
{
	unsigned short val;

	val = 0;

	if ((dma->mode & 0x10) == 0) {
		if (dma->mode & 0x08) {
			return (st_acsi_get_result (dma->acsi));
		}

		switch ((dma->mode >> 1) & 3) {
		case 0:
			val = wd179x_get_status (dma->fdc);
			break;

		case 1:
			val = wd179x_get_track (dma->fdc);
			break;

		case 2:
			val = wd179x_get_sector (dma->fdc);
			break;

		case 3:
			val = wd179x_get_data (dma->fdc);
			break;
		}
	}
	else {
		st_log_deb ("DMA: get sector count: %04X\n", val);
	}

	return (val);
}

void st_dma_set_disk (st_dma_t *dma, unsigned short val)
{
	if ((dma->mode & 0x10) == 0) {
		if (dma->mode & 0x08) {
			st_acsi_set_cmd (dma->acsi, val, (dma->mode & 2) != 0);
			return;
		}

#if DEBUG_DMA >= 2
		st_log_deb ("DMA: set disk %u: %04X\n", (dma->mode >> 1) & 3, val);
#endif

		switch ((dma->mode >> 1) & 3) {
		case 0:
			wd179x_set_cmd (dma->fdc, val & 0xff);
			break;

		case 1:
			wd179x_set_track (dma->fdc, val & 0xff);
			break;

		case 2:
			wd179x_set_sector (dma->fdc, val & 0xff);
			break;

		case 3:
			wd179x_set_data (dma->fdc, val & 0xff);
			break;
		}
	}
	else {
		dma->sector_cnt = val;

		if (dma->sector_cnt > 0) {
			dma->status |= 2;
		}

		dma->dma_first = 1;

		if (dma->mode & 0x100) {
			st_dma_fifo_fill (dma, 0);
			st_dma_fifo_fill (dma, 1);
		}
	}
}

unsigned char st_dma_get_addr (const st_dma_t *dma, unsigned idx)
{
	switch (idx) {
	case 0:
		return ((dma->addr >> 16) & 0xff);

	case 1:
		return ((dma->addr >> 8) & 0xff);

	case 2:
		return (dma->addr & 0xff);
	}

	return (0);
}

void st_dma_set_addr (st_dma_t *dma, unsigned idx, unsigned char val)
{
	val &= 0xff;

	switch (idx) {
	case 0:
		dma->addr = (dma->addr & 0x00ffff) | ((unsigned long) val << 16);
		break;

	case 1:
		dma->addr = (dma->addr & 0xff00ff) | ((unsigned long) val << 8);
		break;

	case 2:
		dma->addr = (dma->addr & 0xffff00) | val;
		break;
	}
}

void st_dma_reset (st_dma_t *dma)
{
	dma->mode = 0;
	dma->status = 1;
	dma->sector_cnt = 0;
	dma->addr = 0;

	dma->fifo_idx = 0;
	dma->fifo[0].idx = 0;
	dma->fifo[0].cnt = 0;
	dma->fifo[1].idx = 0;
	dma->fifo[1].cnt = 0;
}
