/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/nvram.c                                          *
 * Created:     2003-12-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>

#include "nvram.h"


void nvr_init (nvram_t *nvr, unsigned long base, unsigned long size)
{
	nvr->mem = mem_blk_new (base, size, 1);
	nvr->mem->ext = nvr;
	nvr_set_endian (nvr, 1);
	mem_blk_clear (nvr->mem, 0x00);

	nvr->fp = NULL;
	nvr->close = 0;
}

nvram_t *nvr_new (unsigned long base, unsigned long size)
{
	nvram_t *nvr;

	nvr = (nvram_t *) malloc (sizeof (nvram_t));
	if (nvr == NULL) {
		return (NULL);
	}

	nvr_init (nvr, base, size);

	return (nvr);
}

void nvr_free (nvram_t *nvr)
{
	if (nvr != NULL) {
		mem_blk_del (nvr->mem);

		if (nvr->close) {
			fclose (nvr->fp);
		}
	}
}

void nvr_del (nvram_t *nvr)
{
	if (nvr != NULL) {
		nvr_free (nvr);
		free (nvr);
	}
}

void nvr_set_endian (nvram_t *nvr, int big)
{
	if (big) {
		nvr->mem->set_uint8 = (mem_set_uint8_f) &nvr_set_uint8;
		nvr->mem->set_uint16 = (mem_set_uint16_f) &nvr_set_uint16_be;
		nvr->mem->set_uint32 = (mem_set_uint32_f) &nvr_set_uint32_be;

		nvr->mem->get_uint8 = (mem_get_uint8_f) &nvr_get_uint8;
		nvr->mem->get_uint16 = (mem_get_uint16_f) &nvr_get_uint16_be;
		nvr->mem->get_uint32 = (mem_get_uint32_f) &nvr_get_uint32_be;
	}
	else {
		nvr->mem->set_uint8 = (mem_set_uint8_f) &nvr_set_uint8;
		nvr->mem->set_uint16 = (mem_set_uint16_f) &nvr_set_uint16_le;
		nvr->mem->set_uint32 = (mem_set_uint32_f) &nvr_set_uint32_le;

		nvr->mem->get_uint8 = (mem_get_uint8_f) &nvr_get_uint8;
		nvr->mem->get_uint16 = (mem_get_uint16_f) &nvr_get_uint16_le;
		nvr->mem->get_uint32 = (mem_get_uint32_f) &nvr_get_uint32_le;
	}
}

int nvr_set_file (nvram_t *nvr, FILE *fp, int close)
{
	if (nvr->close) {
		fclose (nvr->fp);
	}

	if (fp == NULL) {
		nvr->fp = NULL;
		nvr->close = 0;
		return (0);
	}

	nvr->fp = fp;
	nvr->close = close;

	if (fseek (nvr->fp, 0, SEEK_SET)) {
		nvr->fp = NULL;
		nvr->close = 0;

		if (close) {
			fclose (fp);
		}

		return (1);
	}

	mem_blk_clear (nvr->mem, 0x00);

	(void) fread (nvr->mem->data, 1, nvr->mem->size, nvr->fp);

	return (0);
}

int nvr_set_fname (nvram_t *nvr, const char *fname)
{
	FILE *fp;

	fp = fopen (fname, "r+b");
	if (fp == NULL) {
		fp = fopen (fname, "w+b");
	}

	if (fp == NULL) {
		return (1);
	}

	return (nvr_set_file (nvr, fp, 1));
}

mem_blk_t *nvr_get_mem (nvram_t *nvr)
{
	return (nvr->mem);
}

static
void nvr_write (nvram_t *nvr, unsigned long addr, unsigned long size)
{
	if (nvr->fp != NULL) {
		if (fseek (nvr->fp, addr, SEEK_SET) == 0) {
			(void) fwrite (nvr->mem->data + addr, 1, size, nvr->fp);
			fflush (nvr->fp);
		}
	}
}

void nvr_set_uint8 (nvram_t *nvr, unsigned long addr, unsigned char val)
{
	nvr->mem->data[addr] = val;
	nvr_write (nvr, addr, 1);
}

void nvr_set_uint16_be (nvram_t *nvr, unsigned long addr, unsigned short val)
{
	nvr->mem->data[addr] = (val >> 8) & 0xff;
	nvr->mem->data[addr + 1] = val & 0xff;
	nvr_write (nvr, addr, 2);
}

void nvr_set_uint16_le (nvram_t *nvr, unsigned long addr, unsigned short val)
{
	nvr->mem->data[addr] = val & 0xff;
	nvr->mem->data[addr + 1] = (val >> 8) & 0xff;
	nvr_write (nvr, addr, 2);
}

void nvr_set_uint32_be (nvram_t *nvr, unsigned long addr, unsigned long val)
{
	nvr->mem->data[addr] = (val >> 24) & 0xff;
	nvr->mem->data[addr + 1] = (val >> 16) & 0xff;
	nvr->mem->data[addr + 2] = (val >> 8) & 0xff;
	nvr->mem->data[addr + 3] = val & 0xff;
	nvr_write (nvr, addr, 4);
}

void nvr_set_uint32_le (nvram_t *nvr, unsigned long addr, unsigned long val)
{
	nvr->mem->data[addr] = val & 0xff;
	nvr->mem->data[addr + 1] = (val >> 8) & 0xff;
	nvr->mem->data[addr + 2] = (val >> 16) & 0xff;
	nvr->mem->data[addr + 3] = (val >> 24) & 0xff;
	nvr_write (nvr, addr, 4);
}

unsigned char nvr_get_uint8 (nvram_t *nvr, unsigned long addr)
{
	return (mem_blk_get_uint8 (nvr->mem, addr));
}

unsigned short nvr_get_uint16_be (nvram_t *nvr, unsigned long addr)
{
	return (mem_blk_get_uint16_be (nvr->mem, addr));
}

unsigned short nvr_get_uint16_le (nvram_t *nvr, unsigned long addr)
{
	return (mem_blk_get_uint16_le (nvr->mem, addr));
}

unsigned long nvr_get_uint32_be (nvram_t *nvr, unsigned long addr)
{
	return (mem_blk_get_uint32_be (nvr->mem, addr));
}

unsigned long nvr_get_uint32_le (nvram_t *nvr, unsigned long addr)
{
	return (mem_blk_get_uint32_le (nvr->mem, addr));
}
