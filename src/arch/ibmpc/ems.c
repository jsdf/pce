/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/ems.c                                         *
 * Created:     2003-10-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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
#include "ems.h"

#include <stdlib.h>

#include <lib/console.h>
#include <lib/log.h>


/*
 * Allocate a new EMS block
 */
static
ems_block_t *ems_blk_new (unsigned handle, unsigned pages)
{
	unsigned    i;
	ems_block_t *blk;

	blk = malloc (sizeof (ems_block_t));
	if (blk == NULL) {
		return (NULL);
	}

	blk->handle = handle;
	blk->pages = pages;

	blk->data = malloc (16384UL * pages);
	if (blk->data == NULL) {
		free (blk);
		return (NULL);
	}

	blk->map_saved = 0;

	for (i = 0; i < 8; i++) {
		blk->name[i] = 0;
	}

	return (blk);
}

static
void ems_blk_del (ems_block_t *blk)
{
	if (blk != NULL) {
		free (blk->data);
		free (blk);
	}
}


ems_t *ems_new (ini_sct_t *sct)
{
	unsigned      i;
	unsigned long pages;
	unsigned      segm;
	ems_t         *ems;

	if (ini_get_uint32 (sct, "sizek", &pages, 0) == 0) {
		pages *= 1024;
	}
	else {
		ini_get_uint32 (sct, "size", &pages, 0);
	}

	pages = (pages + 16383) / 16384;

	ini_get_uint16 (sct, "segment", &segm, 0xe000);

	ems = malloc (sizeof (ems_t));
	if (ems == NULL) {
		return (NULL);
	}

	for (i = 0; i < 256; i++) {
		ems->blk[i] = NULL;
	}

	ems->blk[0] = ems_blk_new (0, 4);

	ems->pages_max = pages;
	ems->pages_used = 0;

	for (i = 0; i < 4; i++) {
		ems->map_blk[i] = NULL;
		ems->map_page[i] = 0;
	}

	ems->mem = mem_blk_new (16 * segm, 65536, 0);

	mem_blk_set_fct (ems->mem, ems,
		ems_get_uint8, ems_get_uint16, NULL,
		ems_set_uint8, ems_set_uint16, NULL
	);

	pce_log_tag (MSG_INF, "EMS:", "size=%luK (%lu pages) segment=0x%04x\n",
		16UL * pages, ems->pages_max, segm
	);

	return (ems);
}

void ems_del (ems_t *ems)
{
	unsigned i;

	if (ems != NULL) {
		for (i = 0; i < 256; i++) {
			ems_blk_del (ems->blk[i]);
		}

		mem_blk_del (ems->mem);

		free (ems);
	}
}

mem_blk_t *ems_get_mem (ems_t *ems)
{
	return (ems->mem);
}

void ems_reset (ems_t *ems)
{
	unsigned i;

	pc_log_deb ("reset ems\n");

	for (i = 1; i < 256; i++) {
		ems_blk_del (ems->blk[i]);
		ems->blk[i] = NULL;
	}

	for (i = 0; i < 4; i++) {
		ems->map_blk[i] = NULL;
		ems->map_page[i] = 0;
	}

	ems->pages_used = 0;
}

void ems_prt_state (ems_t *ems)
{
	unsigned i, n;

	n = 0;
	for (i = 1; i < 255; i++) {
		if (ems->blk[i] != NULL) {
			n += 1;
		}
	}

	pce_printf ("EMS: %u/%u pages  %luK/%luK  %u handles\n",
		ems->pages_used, ems->pages_max,
		16UL * ems->pages_used, 16UL * ems->pages_max,
		n
	);

	pce_printf ("  mapping: [%u/%u %u/%u %u/%u %u/%u]\n",
		(ems->map_blk[0] != NULL) ? ems->map_blk[0]->handle : 0xfff,
		ems->map_page[0],
		(ems->map_blk[1] != NULL) ? ems->map_blk[1]->handle : 0xfff,
		ems->map_page[1],
		(ems->map_blk[2] != NULL) ? ems->map_blk[2]->handle : 0xfff,
		ems->map_page[2],
		(ems->map_blk[3] != NULL) ? ems->map_blk[3]->handle : 0xfff,
		ems->map_page[3]
	);

	for (i = 0; i < 256; i++) {
		if (ems->blk[i] != NULL) {
			pce_printf ("  %u: %u pages\n", i, ems->blk[i]->pages);
		}
	}
}

void ems_info (ems_t *ems, e8086_t *cpu)
{
	if (ems != NULL) {
		e86_set_ax (cpu, 0x0001);
		e86_set_dx (cpu, 16 * ems->pages_max);
	}
	else {
		e86_set_ax (cpu, 0x0000);
		e86_set_dx (cpu, 0x0000);
	}
}

unsigned char ems_get_uint8 (ems_t *ems, unsigned long addr)
{
	unsigned page, offs;

	if (addr > 65535) {
		return (0xff);
	}

	page = addr / 16384;
	offs = addr % 16384;

	if (ems->map_blk[page] == NULL) {
		return (0xff);
	}

	addr = 16384UL * ems->map_page[page] + offs;

	return (ems->map_blk[page]->data[addr]);
}

void ems_set_uint8 (ems_t *ems, unsigned long addr, unsigned char val)
{
	unsigned page, offs;

	if (addr > 65535) {
		return;
	}

	page = addr / 16384;
	offs = addr % 16384;

	if (ems->map_blk[page] == NULL) {
		return;
	}

	addr = 16384UL * ems->map_page[page] + offs;

	ems->map_blk[page]->data[addr] = val;
}

unsigned short ems_get_uint16 (ems_t *ems, unsigned long addr)
{
	unsigned short ret;

	ret = ems_get_uint8 (ems, addr) + (ems_get_uint8 (ems, addr + 1) << 8);

	return (ret);
}

void ems_set_uint16 (ems_t *ems, unsigned long addr, unsigned short val)
{
	ems_set_uint8 (ems, addr, val & 0xff);
	ems_set_uint8 (ems, addr + 1, (val >> 8) & 0xff);
}


/*
 * Get the page map for a page in the page frame
 */
static
int ems_get_page_map (ems_t *ems, unsigned idx, unsigned short *handle, unsigned *page)
{
	if (idx > 3) {
		return (1);
	}

	if (ems->map_blk[idx] == NULL) {
		*handle = 0;
		*page = 0xffff;
	}
	else {
		*handle = ems->map_blk[idx]->handle;
		*page = ems->map_page[idx];
	}

	return (0);
}

/*
 * Map a page into the page frame
 */
static
int ems_set_page_map (ems_t *ems, unsigned idx, unsigned short handle, unsigned page)
{
	if (idx > 3) {
		return (1);
	}

	ems->map_blk[idx] = NULL;
	ems->map_page[idx] = 0;

	if ((handle == 0) || (page == 0xffff)) {
		return (0);
	}

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		return (1);
	}

	if (page >= ems->blk[handle]->pages) {
		return (1);
	}

	ems->map_blk[idx] = ems->blk[handle];
	ems->map_page[idx] = page;

	return (0);
}


/* 40: get status */
static
void ems_40 (ems_t *ems, e8086_t *cpu)
{
	e86_set_ah (cpu, 0x00);
}

/* 41: get page frame address */
static
void ems_41 (ems_t *ems, e8086_t *cpu)
{
	e86_set_ah (cpu, 0x00);
	e86_set_bx (cpu, mem_blk_get_addr (ems->mem) / 16);
}

/* 42: get page count */
static
void ems_42 (ems_t *ems, e8086_t *cpu)
{
	e86_set_ah (cpu, 0x00);
	e86_set_bx (cpu, ems->pages_max - ems->pages_used);
	e86_set_dx (cpu, ems->pages_max);
}

/* 43: allocate pages */
static
void ems_43 (ems_t *ems, e8086_t *cpu)
{
	unsigned i;
	unsigned cnt;

	cnt = e86_get_bx (cpu);

	if (cnt == 0) {
		e86_set_ah (cpu, 0x89);
		return;
	}

	if (cnt > ems->pages_max) {
		e86_set_ah (cpu, 0x87);
		return;
	}

	if ((ems->pages_used + cnt) > ems->pages_max) {
		e86_set_ah (cpu, 0x88);
		return;
	}

	for (i = 1; i < 255; i++) {
		if (ems->blk[i] == NULL) {
			ems->blk[i] = ems_blk_new (i, cnt);
			ems->pages_used += cnt;
			e86_set_ah (cpu, 0x00);
			e86_set_dx (cpu, i);
			return;
		}
	}

	e86_set_ah (cpu, 0x85);
}

/* 44: map a page */
static
void ems_44 (ems_t *ems, e8086_t *cpu)
{
	unsigned fpage, spage, handle;

	fpage = e86_get_al (cpu);
	spage = e86_get_bx (cpu);
	handle = e86_get_dx (cpu);

	if (fpage > 3) {
		e86_set_ah (cpu, 0x8b);
		return;
	}

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	if (spage == 0xffff) {
		ems->map_blk[fpage] = NULL;
		ems->map_page[fpage] = 0;
		e86_set_ah (cpu, 0x00);
		return;
	}

	if (spage >= ems->blk[handle]->pages) {
		e86_set_ah (cpu, 0xa8);
		return;
	}

	ems->map_blk[fpage] = ems->blk[handle];
	ems->map_page[fpage] = spage;

	e86_set_ah (cpu, 0x00);
}

/* 45: deallocate pages */
static
void ems_45 (ems_t *ems, e8086_t *cpu)
{
	unsigned    i;
	unsigned    handle;
	ems_block_t *blk;

	handle = e86_get_dx (cpu);

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	blk = ems->blk[handle];

	if (blk->map_saved) {
		e86_set_ah (cpu, 0x86);
		return;
	}

	for (i = 0; i < 4; i++) {
		if (ems->map_blk[i] == blk) {
			ems->map_blk[i] = NULL;
			ems->map_page[i] = 0;
		}
	}

	ems->pages_used -= blk->pages;

	ems->blk[handle] = NULL;

	ems_blk_del (blk);

	e86_set_ah (cpu, 0x00);
}

/* 46: get version */
static
void ems_46 (ems_t *ems, e8086_t *cpu)
{
	e86_set_ah (cpu, 0x00);
	e86_set_al (cpu, 0x32);
}

/* 47: save page map */
static
void ems_47 (ems_t *ems, e8086_t *cpu)
{
	unsigned    i;
	unsigned    handle;
	ems_block_t *blk;

	handle = e86_get_dx (cpu);

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	blk = ems->blk[handle];

	if (blk->map_saved) {
		e86_set_ah (cpu, 0x8d);
		return;
	}

	for (i = 0; i < 4; i++) {
		ems_get_page_map (ems, i, blk->map_blk + i, blk->map_page + i);
	}

	blk->map_saved = 1;

	e86_set_ah (cpu, 0x00);
}

/* 48: restore page map */
static
void ems_48 (ems_t *ems, e8086_t *cpu)
{
	unsigned    i;
	unsigned    handle;
	ems_block_t *blk;

	handle = e86_get_dx (cpu);

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	blk = ems->blk[handle];

	if (blk->map_saved == 0) {
		e86_set_ah (cpu, 0x8e);
		return;
	}

	for (i = 0; i < 4; i++) {
		ems_set_page_map (ems, i, blk->map_blk[i], blk->map_page[i]);
	}

	blk->map_saved = 0;

	e86_set_ah (cpu, 0x00);
}

/* 4b: get handle count */
static
void ems_4b (ems_t *ems, e8086_t *cpu)
{
	unsigned i, n;

	n = 0;

	for (i = 0; i < 255; i++) {
		if (ems->blk[i] != NULL) {
			n += 1;
		}
	}

	e86_set_ah (cpu, 0x00);
	e86_set_bx (cpu, n);
}

/* 4c: get handle page count */
static
void ems_4c (ems_t *ems, e8086_t *cpu)
{
	unsigned handle;

	handle = e86_get_dx (cpu);

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	e86_set_ah (cpu, 0x00);
	e86_set_bx (cpu, ems->blk[handle]->pages);
}

/* 4d: get all handles page count */
static
void ems_4d (ems_t *ems, e8086_t *cpu)
{
	unsigned       i, n;
	unsigned short segm, offs;

	segm = e86_get_es (cpu);
	offs = e86_get_di (cpu);

	n = 0;

	for (i = 0; i < 255; i++) {
		if (ems->blk[i] != NULL) {
			n += 1;
			e86_set_mem16 (cpu, segm, offs + 4 * i, i);
			e86_set_mem16 (cpu, segm, offs + 4 * i + 2, ems->blk[i]->pages);
		}
	}

	e86_set_ah (cpu, 0x00);
	e86_set_bx (cpu, n);
}

/* 4e00: get page map */
static
void ems_4e00 (ems_t *ems, e8086_t *cpu)
{
	unsigned       i;
	unsigned       seg, ofs;
	unsigned short handle;
	unsigned       page;

	seg = e86_get_es (cpu);
	ofs = e86_get_di (cpu);

	for (i = 0; i < 4; i++) {
		ems_get_page_map (ems, i, &handle, &page);

		e86_set_mem16 (cpu, seg, ofs, handle);
		e86_set_mem16 (cpu, seg, ofs + 2, page);

		ofs += 4;
	}

	e86_set_ah (cpu, 0x00);
}

/* 4e01: set page map */
static
void ems_4e01 (ems_t *ems, e8086_t *cpu)
{
	unsigned       i;
	unsigned       seg, ofs;
	unsigned short handle;
	unsigned       page;

	seg = e86_get_ds (cpu);
	ofs = e86_get_si (cpu);

	for (i = 0; i < 4; i++) {
		handle = e86_get_mem16 (cpu, seg, ofs);
		page = e86_get_mem16 (cpu, seg, ofs + 2);

		if (ems_set_page_map (ems, i, handle, page)) {
			e86_set_ah (cpu, 0xa3);
			return;
		}

		ofs += 4;
	}

	e86_set_ah (cpu, 0x00);
}

/* 4e02: get and set page map */
static
void ems_4e02 (ems_t *ems, e8086_t *cpu)
{
	unsigned       i;
	unsigned       sseg, sofs;
	unsigned       dseg, dofs;
	unsigned short handle;
	unsigned       page;

	dseg = e86_get_es (cpu);
	dofs = e86_get_di (cpu);

	for (i = 0; i < 4; i++) {
		ems_get_page_map (ems, i, &handle, &page);

		e86_set_mem16 (cpu, dseg, dofs, handle);
		e86_set_mem16 (cpu, dseg, dofs + 2, page);

		dofs += 4;
	}

	sseg = e86_get_ds (cpu);
	sofs = e86_get_si (cpu);

	for (i = 0; i < 4; i++) {
		handle = e86_get_mem16 (cpu, sseg, sofs);
		page = e86_get_mem16 (cpu, sseg, sofs + 2);

		if (ems_set_page_map (ems, i, handle, page)) {
			e86_set_ah (cpu, 0xa3);
			return;
		}

		sofs += 4;
	}

	e86_set_ah (cpu, 0x00);
}

/* 4e03: get size of page map save array */
static
void ems_4e03 (ems_t *ems, e8086_t *cpu)
{
	e86_set_ax (cpu, 0x0010);
}

/* 4e: get/set page map */
static
void ems_4e (ems_t *ems, e8086_t *cpu)
{
	switch (e86_get_al (cpu)) {
	case 0x00:
		ems_4e00 (ems, cpu);
		break;

	case 0x01:
		ems_4e01 (ems, cpu);
		break;

	case 0x02:
		ems_4e02 (ems, cpu);
		break;

	case 0x03:
		ems_4e03 (ems, cpu);
		break;

	default:
		pce_log (MSG_MSG, "ems: unknown subfunction: AX=%04X\n",
			e86_get_ax (cpu)
		);

		/* invalid subfunction */
		e86_set_ah (cpu, 0x8f);

		break;
	}
}

/* 51: reallocate pages */
static
void ems_51 (ems_t *ems, e8086_t *cpu)
{
	unsigned      i;
	unsigned      handle;
	unsigned      cnt;
	unsigned char *data;
	ems_block_t   *blk;

	handle = e86_get_dx (cpu);
	cnt = e86_get_bx (cpu);

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	blk = ems->blk[handle];

	if (cnt > ems->pages_max) {
		e86_set_ah (cpu, 0x87);
		return;
	}

	if ((ems->pages_used - blk->pages + cnt) > ems->pages_max) {
		e86_set_ah (cpu, 0x88);
		return;
	}

	if (cnt > 0) {
		data = realloc (blk->data, 16384UL * cnt);

		if (data == NULL) {
			e86_set_ah (cpu, 0x88);
			return;
		}
	}
	else {
		free (blk->data);
		data = NULL;
	}

	ems->pages_used -= blk->pages;
	ems->pages_used += cnt;

	blk->data = data;
	blk->pages = cnt;

	for (i = 0; i < 4; i++) {
		if (ems->map_blk[i] == blk) {
			if (ems->map_page[i] >= blk->pages) {
				ems->map_blk[i] = NULL;
				ems->map_page[i] = 0;
			}
		}
	}

	e86_set_ah (cpu, 0x00);
	e86_set_bx (cpu, cnt);
}

/* 5300: get handle name */
static
void ems_5300 (ems_t *ems, e8086_t *cpu)
{
	unsigned       i;
	unsigned       handle;
	unsigned       seg, ofs;
	unsigned char *str;

	handle = e86_get_dx (cpu);

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	seg = e86_get_es (cpu);
	ofs = e86_get_di (cpu);

	str = ems->blk[handle]->name;

	for (i = 0; i < 8; i++) {
		e86_set_mem8 (cpu, seg, (ofs + i) & 0xffff, str[i]);
	}

	e86_set_ah (cpu, 0x00);
}

/* 5301: set handle name */
static
void ems_5301 (ems_t *ems, e8086_t *cpu)
{
	unsigned       i;
	unsigned       handle;
	unsigned       seg, ofs;
	unsigned char *str;

	handle = e86_get_dx (cpu);

	if ((handle > 255) || (ems->blk[handle] == NULL)) {
		e86_set_ah (cpu, 0x83);
		return;
	}

	seg = e86_get_es (cpu);
	ofs = e86_get_si (cpu);

	str = ems->blk[handle]->name;

	/* should check if the name already exists */

	for (i = 0; i < 8; i++) {
		str[i] = e86_get_mem8 (cpu, seg, (ofs + i) & 0xffff);
	}

	e86_set_ah (cpu, 0x00);
}

/* 53: get/set handle name */
static
void ems_53 (ems_t *ems, e8086_t *cpu)
{
	switch (e86_get_al (cpu)) {
	case 0x00:
		ems_5300 (ems, cpu);
		break;

	case 0x01:
		ems_5301 (ems, cpu);
		break;

	default:
		pce_log (MSG_MSG, "ems: unknown subfunction: AX=%04X\n",
			e86_get_ax (cpu)
		);

		/* invalid subfunction */
		e86_set_ah (cpu, 0x8f);

		break;
	}
}

void ems_handler (ems_t *ems, e8086_t *cpu)
{
	if (ems == NULL) {
		return;
	}

	switch (e86_get_ah (cpu)) {
	case 0x40:
		ems_40 (ems, cpu);
		break;

	case 0x41:
		ems_41 (ems, cpu);
		break;

	case 0x42:
		ems_42 (ems, cpu);
		break;

	case 0x43:
		ems_43 (ems, cpu);
		break;

	case 0x44:
		ems_44 (ems, cpu);
		break;

	case 0x45:
		ems_45 (ems, cpu);
		break;

	case 0x46:
		ems_46 (ems, cpu);
		break;

	case 0x47:
		ems_47 (ems, cpu);
		break;

	case 0x48:
		ems_48 (ems, cpu);
		break;

	case 0x4b:
		ems_4b (ems, cpu);
		break;

	case 0x4c:
		ems_4c (ems, cpu);
		break;

	case 0x4d:
		ems_4d (ems, cpu);
		break;

	case 0x4e:
		ems_4e (ems, cpu);
		break;

	case 0x51:
		ems_51 (ems, cpu);
		break;

	case 0x53:
		ems_53 (ems, cpu);
		break;

	default:
		pce_log (MSG_MSG, "ems: unknown function: AX=%04X\n",
			e86_get_ax (cpu)
		);
		e86_set_ah (cpu, 0x84);
		break;
	}
}
