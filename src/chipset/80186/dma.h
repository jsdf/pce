/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/80186/dma.h                                      *
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


#ifndef PCE_CHIPSET_80186_DMA_H
#define PCE_CHIPSET_80186_DMA_H 1


typedef struct {
	unsigned short control[2];
	unsigned short count[2];
	unsigned long  src[2];
	unsigned long  dst[2];
	unsigned char  dreq[2];

	unsigned char  check;

	unsigned       delay;

	void           *get_mem_ext;
	unsigned char  (*get_mem8) (void *ext, unsigned long addr);
	unsigned short (*get_mem16) (void *ext, unsigned long addr);

	void           *set_mem_ext;
	void           (*set_mem8) (void *ext, unsigned long addr, unsigned char val);
	void           (*set_mem16) (void *ext, unsigned long addr, unsigned short val);

	void           *get_io_ext;
	unsigned char  (*get_io8) (void *ext, unsigned long addr);
	unsigned short (*get_io16) (void *ext, unsigned long addr);

	void           *set_io_ext;
	void           (*set_io8) (void *ext, unsigned long addr, unsigned char val);
	void           (*set_io16) (void *ext, unsigned long addr, unsigned short val);

	unsigned char  int0_val;
	void           *int0_ext;
	void           (*int0) (void *ext, unsigned char val);

	unsigned char  int1_val;
	void           *int1_ext;
	void           (*int1) (void *ext, unsigned char val);
} e80186_dma_t;


void e80186_dma_init (e80186_dma_t *dma);
void e80186_dma_free (e80186_dma_t *dma);

void e80186_dma_set_getmem_fct (e80186_dma_t *dma, void *ext, void *get8, void *get16);
void e80186_dma_set_setmem_fct (e80186_dma_t *dma, void *ext, void *set8, void *set16);
void e80186_dma_set_getio_fct (e80186_dma_t *dma, void *ext, void *get8, void *get16);
void e80186_dma_set_setio_fct (e80186_dma_t *dma, void *ext, void *set8, void *set16);
void e80186_dma_set_int_fct (e80186_dma_t *dma, unsigned idx, void *ext, void *fct);

void e80186_dma_reset (e80186_dma_t *dma);

void e80186_dma_set_dreq0 (e80186_dma_t *dma, unsigned char val);
void e80186_dma_set_dreq1 (e80186_dma_t *dma, unsigned char val);

unsigned long e80186_dma_get_src (const e80186_dma_t *dma, unsigned idx);

unsigned short e80186_dma_get_src_hi (const e80186_dma_t *dma, unsigned idx);
void e80186_dma_set_src_hi (e80186_dma_t *dma, unsigned idx, unsigned short val);

unsigned short e80186_dma_get_src_lo (const e80186_dma_t *dma, unsigned idx);
void e80186_dma_set_src_lo (e80186_dma_t *dma, unsigned idx, unsigned short val);

unsigned long e80186_dma_get_dst (const e80186_dma_t *dma, unsigned idx);

unsigned short e80186_dma_get_dst_hi (const e80186_dma_t *dma, unsigned idx);
void e80186_dma_set_dst_hi (e80186_dma_t *dma, unsigned idx, unsigned short val);

unsigned short e80186_dma_get_dst_lo (const e80186_dma_t *dma, unsigned idx);
void e80186_dma_set_dst_lo (e80186_dma_t *dma, unsigned idx, unsigned short val);

unsigned short e80186_dma_get_control (const e80186_dma_t *dma, unsigned idx);
void e80186_dma_set_control (e80186_dma_t *dma, unsigned idx, unsigned short val);

unsigned short e80186_dma_get_count (const e80186_dma_t *dma, unsigned idx);
void e80186_dma_set_count (e80186_dma_t *dma, unsigned idx, unsigned short val);

void e80186_dma_clock2 (e80186_dma_t *dma, unsigned cnt);

static inline
void e80186_dma_clock (e80186_dma_t *dma, unsigned cnt)
{
	if (dma->check) {
		e80186_dma_clock2 (dma, cnt);
	}
}


#endif
