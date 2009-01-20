/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/nvram.h                                          *
 * Created:     2003-12-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_NVRAM_H
#define PCE_DEVICES_NVRAM_H 1


#include <devices/memory.h>


typedef struct {
	mem_blk_t *mem;

	FILE      *fp;
	int       close;
} nvram_t;


void nvr_init (nvram_t *nvr, unsigned long base, unsigned long size);
nvram_t *nvr_new (unsigned long base, unsigned long size);
void nvr_free (nvram_t *nvr);
void nvr_del (nvram_t *nvr);

void nvr_set_endian (nvram_t *nvr, int big);

int nvr_set_file (nvram_t *nvr, FILE *fp, int close);
int nvr_set_fname (nvram_t *nvr, const char *fname);

mem_blk_t *nvr_get_mem (nvram_t *nvr);

void nvr_set_uint8 (nvram_t *nvr, unsigned long addr, unsigned char val);
void nvr_set_uint16_be (nvram_t *nvr, unsigned long addr, unsigned short val);
void nvr_set_uint16_le (nvram_t *nvr, unsigned long addr, unsigned short val);
void nvr_set_uint32_be (nvram_t *nvr, unsigned long addr, unsigned long val);
void nvr_set_uint32_le (nvram_t *nvr, unsigned long addr, unsigned long val);

unsigned char nvr_get_uint8 (nvram_t *nvr, unsigned long addr);
unsigned short nvr_get_uint16_be (nvram_t *nvr, unsigned long addr);
unsigned short nvr_get_uint16_le (nvram_t *nvr, unsigned long addr);
unsigned long nvr_get_uint32_be (nvram_t *nvr, unsigned long addr);
unsigned long nvr_get_uint32_le (nvram_t *nvr, unsigned long addr);


#endif
