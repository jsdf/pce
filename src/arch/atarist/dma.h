/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/dma.h                                       *
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


#ifndef PCE_ATARIST_DMA_H
#define PCE_ATARIST_DMA_H 1


#include "acsi.h"

#include <chipset/wd179x.h>
#include <devices/memory.h>


typedef struct {
	unsigned      idx;
	unsigned      cnt;
	unsigned char data[16];
} st_dma_fifo_t;


typedef struct {
	unsigned short mode;
	unsigned short status;
	unsigned char  sector_cnt;
	char           dma_first;
	unsigned long  addr;

	unsigned       byte_cnt;

	unsigned       fifo_idx;
	st_dma_fifo_t  fifo[2];

	memory_t       *mem;
	wd179x_t       *fdc;
	st_acsi_t      *acsi;
} st_dma_t;


int st_dma_init (st_dma_t *dma);

void st_dma_set_memory (st_dma_t *dma, memory_t *mem);
void st_dma_set_fdc (st_dma_t *dma, wd179x_t *fdc);
void st_dma_set_acsi (st_dma_t *dma, st_acsi_t *acsi);

void st_dma_set_dreq (st_dma_t *dma, unsigned char val);

unsigned short st_dma_get_status (st_dma_t *dma);

void st_dma_set_mode (st_dma_t *dma, unsigned short val);

unsigned short st_dma_get_disk (st_dma_t *dma);

void st_dma_set_disk (st_dma_t *dma, unsigned short val);

unsigned char st_dma_get_addr (const st_dma_t *dma, unsigned idx);

void st_dma_set_addr (st_dma_t *dma, unsigned idx, unsigned char val);

void st_dma_reset (st_dma_t *dma);


#endif
