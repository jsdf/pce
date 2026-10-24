/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/viking.c                                    *
 * Created:     2015-09-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015-2019 Hampa Hug <hampa@hampa.ch>                     *
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
#include "viking.h"

#include <stdlib.h>
#include <string.h>

#include <devices/memory.h>


#define VIKING_W   1280
#define VIKING_H   960
#define VIKING_FPS 25


static unsigned char st_viking_get_uint8 (st_viking_t *vik, unsigned long addr);
static unsigned short st_viking_get_uint16 (st_viking_t *vik, unsigned long addr);
static unsigned long st_viking_get_uint32 (st_viking_t *vik, unsigned long addr);
static void st_viking_set_uint8 (st_viking_t *vik, unsigned long addr, unsigned char val);
static void st_viking_set_uint16 (st_viking_t *vik, unsigned long addr, unsigned short val);
static void st_viking_set_uint32 (st_viking_t *vik, unsigned long addr, unsigned long val);


int st_viking_init (st_viking_t *vik, unsigned long addr)
{
	vik->mem = NULL;

	if (mem_blk_init (&vik->ram, addr, 256UL * 1024UL, 1)) {
		return (1);
	}

	vik->ptr = vik->ram.data;

	mem_blk_set_fct (&vik->ram, vik,
		st_viking_get_uint8, st_viking_get_uint16, st_viking_get_uint32,
		st_viking_set_uint8, st_viking_set_uint16, st_viking_set_uint32
	);

	if ((vik->rgb = malloc (3UL * VIKING_W * VIKING_H)) == NULL) {
		return (1);
	}

	vik->trm = NULL;

	vik->mod = 1;
	vik->mod_y1 = 0;
	vik->mod_y2 = VIKING_H - 1;

	vik->input_clock = 1000000;
	vik->clock = 0;

	return (0);
}

void st_viking_free (st_viking_t *vik)
{
	st_viking_set_memory (vik, NULL);

	free (vik->rgb);

	mem_blk_free (&vik->ram);
}

st_viking_t *st_viking_new (unsigned long addr)
{
	st_viking_t *vik;

	if ((vik = malloc (sizeof (st_viking_t))) == NULL) {
		return (NULL);
	}

	if (st_viking_init (vik, addr)) {
		free (vik);
		return (NULL);
	}

	return (vik);
}

void st_viking_del (st_viking_t *vik)
{
	if (vik != NULL) {
		st_viking_free (vik);
		free (vik);
	}
}

void st_viking_set_input_clock (st_viking_t *vik, unsigned long clock)
{
	vik->input_clock = clock;
	vik->clock = 0;
}

void st_viking_set_memory (st_viking_t *vik, memory_t *mem)
{
	if (vik->mem != NULL) {
		mem_rmv_blk (vik->mem, &vik->ram);
	}

	vik->mem = mem;

	if (vik->mem != NULL) {
		mem_add_blk (vik->mem, &vik->ram, 0);
	}
}

void st_viking_set_terminal (st_viking_t *vik, terminal_t *trm)
{
	vik->trm = trm;

	if (trm != NULL) {
		vik->mod = 1;
		vik->mod_y1 = 0;
		vik->mod_y2 = VIKING_H - 1;

		trm_open (vik->trm, VIKING_W, VIKING_H);
	}
}

/*
 * Mark the address range [addr1..addr2] as modified
 */
static
void st_viking_mod_line (st_viking_t *vik, unsigned long addr1, unsigned long addr2)
{
	unsigned y1, y2;

	y1 = addr1 / (VIKING_W / 8);
	y2 = addr2 / (VIKING_W / 8);

	if (y1 >= VIKING_H) {
		return;
	}

	if (vik->mod == 0) {
		vik->mod_y1 = y1;
		vik->mod_y2 = y2;
		vik->mod = 1;
	}
	else {
		if (y1 < vik->mod_y1) {
			vik->mod_y1 = y1;
		}

		if (y2 > vik->mod_y2) {
			vik->mod_y2 = y2;
		}
	}
}

static
unsigned char st_viking_get_uint8 (st_viking_t *vik, unsigned long addr)
{
	return (vik->ptr[addr]);
}

static
unsigned short st_viking_get_uint16 (st_viking_t *vik, unsigned long addr)
{
	unsigned short val;

	val = vik->ptr[addr];
	val = (val << 8) | vik->ptr[addr + 1];

	return (val);
}

static
unsigned long st_viking_get_uint32 (st_viking_t *vik, unsigned long addr)
{
	unsigned long val;

	val = vik->ptr[addr];
	val = (val << 8) | vik->ptr[addr + 1];
	val = (val << 8) | vik->ptr[addr + 2];
	val = (val << 8) | vik->ptr[addr + 3];

	return (val);
}

static
void st_viking_set_uint8 (st_viking_t *vik, unsigned long addr, unsigned char val)
{
	vik->ptr[addr] = val;

	st_viking_mod_line (vik, addr, addr);
}

static
void st_viking_set_uint16 (st_viking_t *vik, unsigned long addr, unsigned short val)
{
	vik->ptr[addr + 0] = (val >> 8) & 0xff;
	vik->ptr[addr + 1] = val & 0xff;

	st_viking_mod_line (vik, addr, addr + 1);
}

static
void st_viking_set_uint32 (st_viking_t *vik, unsigned long addr, unsigned long val)
{
	vik->ptr[addr + 0] = (val >> 24) & 0xff;
	vik->ptr[addr + 1] = (val >> 16) & 0xff;
	vik->ptr[addr + 2] = (val >> 8) & 0xff;
	vik->ptr[addr + 3] = val & 0xff;

	st_viking_mod_line (vik, addr, addr + 3);
}

static
void st_viking_update (st_viking_t *vik)
{
	unsigned            i, j, k;
	unsigned char       val;
	const unsigned char *src;
	unsigned char       *dst;

	if (vik->mod == 0) {
		return;
	}

	if (vik->mod_y1 >= VIKING_H) {
		return;
	}

	if (vik->mod_y2 >= VIKING_H) {
		vik->mod_y2 = VIKING_H - 1;
	}

	src = vik->ptr + vik->mod_y1 * (VIKING_W / 8);
	dst = vik->rgb + vik->mod_y1 * (VIKING_W / 8) * 3UL;

	for (j = vik->mod_y1; j <= vik->mod_y2; j++) {
		for (i = 0; i < (VIKING_W / 8); i++) {
			val = *(src++);

			for (k = 0; k < 8; k++) {
				if (val & 0x80) {
					*(dst++) = 0x00;
					*(dst++) = 0x00;
					*(dst++) = 0x00;
				}
				else {
					*(dst++) = 0xff;
					*(dst++) = 0xff;
					*(dst++) = 0xff;
				}

				val <<= 1;
			}
		}
	}
}

void st_viking_redraw (st_viking_t *vik)
{
	vik->mod = 1;
	vik->mod_y1 = 0;
	vik->mod_y2 = VIKING_H - 1;
}

void st_viking_reset (st_viking_t *vik)
{
	vik->clock = 0;

	vik->mod = 1;
	vik->mod_y1 = 0;
	vik->mod_y2 = VIKING_H - 1;
}

void st_viking_clock (st_viking_t *vik, unsigned cnt)
{
	unsigned      n;
	unsigned char *p;

	vik->clock += VIKING_FPS * (unsigned long) cnt;

	if (vik->clock < vik->input_clock) {
		return;
	}

	if ((vik->trm != NULL) && vik->mod) {
		st_viking_update (vik);

		n = vik->mod_y2 - vik->mod_y1 + 1;
		p = vik->rgb + 3UL * (VIKING_W / 8) * vik->mod_y1;

		trm_set_size (vik->trm, VIKING_W, VIKING_H);
		trm_set_lines (vik->trm, p, vik->mod_y1, n);
		trm_update (vik->trm);

		vik->mod = 0;
		vik->mod_y1 = 0;
		vik->mod_y2 = 0;
	}

	vik->clock -= vik->input_clock;
}
