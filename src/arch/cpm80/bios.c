/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/cpm80/bios.c                                        *
 * Created:     2012-11-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2016 Hampa Hug <hampa@hampa.ch>                     *
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
#include "bios.h"
#include "cpm80.h"

#include <stdlib.h>
#include <string.h>

#include <cpu/e8080/e8080.h>
#include <devices/memory.h>
#include <drivers/block/block.h>
#include <drivers/block/blkpsi.h>
#include <lib/log.h>


#define ORD_CHS 0
#define ORD_HCS 1


#ifndef DEBUG_BIOS
#define DEBUG_BIOS 0
#endif


typedef struct {
	const char     *name;

	unsigned short c;
	unsigned short h;
	unsigned short s;
	unsigned short n;

	unsigned char  order;
	unsigned short spt;
	unsigned short bls;
	unsigned char  exm;
	unsigned short dsm;
	unsigned short drm;
	unsigned short al0;
	unsigned short off;
	unsigned char  *trn0;
	unsigned char  *trn1;
} c80_disk_t;


static unsigned char map_26_6[32] = {
	 1,  7, 13, 19, 25,  5, 11, 17, 23,  3,  9, 15, 21,
	 2,  8, 14, 20, 26,  6, 12, 18, 24,  4, 10, 16, 22
};

static unsigned char map_0[32] = {
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};


static c80_disk_t par_disks[] = {
	{ "A1",   77, 1, 26, 128,  ORD_CHS, 26 * 1, 1024, 0, 242,  63, 0xc000, 2, map_26_6 },
	{ "IBM1", 40, 1,  8, 512,  ORD_CHS,  8 * 4, 1024, 0, 155,  63, 0xc000, 1, NULL },
	{ "IBM2", 40, 2,  8, 512,  ORD_HCS,  8 * 4, 2048, 1, 157,  63, 0x8000, 1, NULL },
	{ "KAY1", 40, 1, 10, 512,  ORD_CHS, 10 * 4, 1024, 0, 194,  63, 0xf000, 1, map_0 },
	{ "KAY2", 40, 2, 10, 512,  ORD_CHS, 10 * 4, 2048, 1, 196,  63, 0xc000, 1, map_0, map_0 + 10 },
	{ "KAY3", 80, 2, 10, 512,  ORD_CHS, 10 * 4, 4096, 3, 196,  95, 0x8000, 2, map_0, map_0 + 20 },
	{ NULL,   40, 2,  9, 512,  ORD_HCS,  9 * 4, 2048, 1, 170,  63, 0x8000, 4, NULL },
	{ NULL,   80, 2,  9, 512,  ORD_HCS,  9 * 4, 2048, 0, 354, 255, 0xf000, 2, NULL },
	{ NULL,   80, 2, 15, 512,  ORD_HCS, 15 * 4, 4096, 1, 295, 255, 0xc000, 2, NULL },
	{ NULL,   80, 2, 18, 512,  ORD_HCS, 18 * 4, 4096, 1, 354, 255, 0xc000, 2, NULL },
	{ 0, 0, 0, 0 }
};


static
void bios_init_traps (cpm80_t *sim, int warm)
{
	unsigned i;
	unsigned addr1, addr2;

	addr1 = sim->addr_bios;
	addr2 = sim->addr_bios + 64;

	mem_set_uint8 (sim->mem, 0x0000, 0xc3);
	mem_set_uint16_le (sim->mem, 0x0001, addr1 + (warm ? 3 : 0));

	for (i = 0; i < 17; i++) {
		mem_set_uint8 (sim->mem, addr1, 0xc3);
		mem_set_uint16_le (sim->mem, addr1 + 1, addr2);
		mem_set_uint8 (sim->mem, addr2 + 0, 0xc7);
		mem_set_uint8 (sim->mem, addr2 + 1, 0xc9);

		addr1 += 3;
		addr2 += 2;
	}
}

int con_ready (cpm80_t *sim)
{
	if (sim->con == NULL) {
		return (0);
	}

	if (sim->con_buf_cnt) {
		return (1);
	}

	if (chr_read (sim->con, &sim->con_buf, 1) == 1) {
		if (sim->con_buf == 0) {
			c80_stop (sim);
			return (0);
		}

		sim->con_buf_cnt = 1;

		return (1);
	}

	return (0);
}

int con_getc (cpm80_t *sim, unsigned char *c)
{
	if (sim->con_buf_cnt) {
		*c = sim->con_buf;
		sim->con_buf_cnt = 0;
		return (0);
	}

	if (sim->con == NULL) {
		return (1);
	}

	if (chr_read (sim->con, c, 1) != 1) {
		return (1);
	}

	if (*c == 0) {
		c80_stop (sim);
		return (1);
	}

	return (0);
}

int con_putc (cpm80_t *sim, unsigned char c)
{
	if (sim->con == NULL) {
		return (0);
	}

	if (chr_write (sim->con, &c, 1) != 1) {
		return (1);
	}

	return (0);
}

int con_puts (cpm80_t *sim, const char *str)
{
	unsigned n;

	n = strlen (str);

	if (sim->con != NULL) {
		if (chr_write (sim->con, str, n) != n) {
			return (1);
		}
	}

	return (0);
}

int aux_getc (cpm80_t *sim, unsigned char *c)
{
	if (sim->aux == NULL) {
		return (1);
	}

	if (chr_read (sim->aux, c, 1) != 1) {
		return (1);
	}

	return (0);
}

int aux_putc (cpm80_t *sim, unsigned char c)
{
	if (sim->aux == NULL) {
		return (0);
	}

	if (chr_write (sim->aux, &c, 1) != 1) {
		return (1);
	}

	return (0);
}

int lst_putc (cpm80_t *sim, unsigned char c)
{
	if (sim->lst == NULL) {
		return (0);
	}

	if (chr_write (sim->lst, &c, 1) != 1) {
		return (1);
	}

	return (0);
}

static
unsigned bios_get_disk_type (cpm80_t *sim, unsigned drv)
{
	unsigned   i;
	c80_disk_t *dt;
	disk_t     *dsk;

	dsk = dsks_get_disk (sim->dsks, drv);

	if (dsk == NULL) {
		return (0);
	}

	i = 0;
	dt = par_disks;

	while (dt->c != 0) {
		if ((dsk->c == dt->c) && (dsk->h == dt->h) && (dsk->s == dt->s)) {
			return (i + 1);
		}

		i += 1;
		dt += 1;
	}

	fprintf (stderr, "unknown disk type (%u/%u/%u)\n", dsk->c, dsk->h, dsk->s);

	return (0);
}

static
int bios_get_disk (cpm80_t *sim, unsigned drv, disk_t **dsk, c80_disk_t **dt)
{
	unsigned type;

	if (drv >= sim->bios_disk_cnt) {
		return (1);
	}

	type = sim->bios_disk_type[drv];

	if (type == 0) {
		return (1);
	}

	*dt = &par_disks[type - 1];

	*dsk = dsks_get_disk (sim->dsks, drv);

	if (*dsk == NULL) {
		return (1);
	}

	return (0);
}

static
int bios_map_sector (c80_disk_t *dt, unsigned trk, unsigned sct, unsigned *c, unsigned *h, unsigned *s, unsigned *i)
{
	unsigned char *trn;

	if (dt->order == ORD_CHS) {
		*c = trk / dt->h;
		*h = trk % dt->h;
	}
	else {
		*c = trk % dt->c;
		*h = trk / dt->c;

		if (*h & 1) {
			*c = dt->c - *c - 1;
		}
	}

	*s = sct / (dt->n / 128);
	*i = sct % (dt->n / 128);

	trn = NULL;

	if (trk >= dt->off) {
		if (*h == 0) {
			trn = dt->trn0;
		}
		else {
			trn = dt->trn1;
		}
	}

	if (trn != NULL) {
		*s = trn[*s];
	}
	else {
		*s += 1;
	}

	return (0);
}

static
int bios_read_sector (cpm80_t *sim, void *buf, unsigned drv, unsigned trk, unsigned sct)
{
	unsigned      c, h, s, idx, cnt;
	disk_t        *dsk;
	c80_disk_t    *dt;
	unsigned char tmp[2048];

	if (bios_get_disk (sim, drv, &dsk, &dt)) {
		return (1);
	}

	if (dt->n > 2048) {
		return (1);
	}

	if (bios_map_sector (dt, trk, sct, &c, &h, &s, &idx)) {
		return (1);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		cnt = dt->n;

		if (dsk_psi_read_chs (dsk->ext, tmp, &cnt, c, h, s, 0) != 0) {
			return (1);
		}

		if (cnt != dt->n) {
			return (1);
		}

		memcpy (buf, tmp + 128 * idx, 128);
	}
	else {
		if (dt->n != 512) {
			return (1);
		}

		if (dsk_read_chs (dsk, tmp, c, h, s, 1)) {
			return (1);
		}

		memcpy (buf, tmp + 128 * idx, 128);
	}

	return (0);
}

static
int bios_write_sector (cpm80_t *sim, const void *buf, unsigned drv, unsigned trk, unsigned sct)
{
	unsigned      c, h, s, idx, cnt;
	disk_t        *dsk;
	c80_disk_t    *dt;
	unsigned char tmp[512];

	if (bios_get_disk (sim, drv, &dsk, &dt)) {
		return (1);
	}

	if (dt->n > 2048) {
		return (1);
	}

	if (bios_map_sector (dt, trk, sct, &c, &h, &s, &idx)) {
		return (1);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		cnt = dt->n;

		if (dsk_psi_read_chs (dsk->ext, tmp, &cnt, c, h, s, 0) != 0) {
			return (1);
		}

		if (cnt != dt->n) {
			return (1);
		}

		memcpy (tmp + 128 * idx, buf, 128);

		if (dsk_psi_write_chs (dsk->ext, tmp, &cnt, c, h, s, 0) != 0) {
			return (1);
		}

		if (cnt != dt->n) {
			return (1);
		}
	}
	else {
		if (dt->n != 512) {
			return (1);
		}

		if (dsk_read_chs (dsk, tmp, c, h, s, 1)) {
			return (1);
		}

		memcpy (tmp + 128 * idx, buf, 128);

		if (dsk_write_chs (dsk, tmp, c, h, s, 1)) {
			return (1);
		}
	}

	return (0);
}

static
int bios_read_system (cpm80_t *sim, unsigned addr, unsigned size)
{
	unsigned      i, j;
	unsigned      track, sector;
	c80_disk_t    *dt;
	unsigned char buf[128];

	if (sim->bios_disk_type[0] == 0) {
		return (1);
	}

	dt = &par_disks[sim->bios_disk_type[0] - 1];

	track = 0;
	sector = 1;

	i = 0;

	while (i < size) {
		if (bios_read_sector (sim, buf, 0, track, sector)) {
			return (1);
		}

		for (j = 0; j < 128; j++) {
			mem_set_uint8 (sim->mem, addr + j, buf[j]);
		}

		sector += 1;

		if (sector >= dt->spt) {
			track += 1;
			sector = 0;
		}

		i += 128;
		addr += 128;
	}

	return (0);
}

static
unsigned bios_setup_disk (cpm80_t *sim, unsigned drv)
{
	unsigned type;

	if (drv >= sim->bios_disk_cnt) {
		return (0);
	}

	type = bios_get_disk_type (sim, drv);

	sim->bios_disk_type[drv] = type;

	return (type);
}

static
unsigned bios_setup_dpb (cpm80_t *sim, unsigned drv)
{
	unsigned   type;
	unsigned   dph, dir, dpb, chk, all, end;
	unsigned   bsh;
	c80_disk_t *dt;

	if (drv >= sim->bios_disk_cnt) {
		return (0);
	}

	type = sim->bios_disk_type[drv];

	if (type == 0) {
		return (0);
	}

	dt = &par_disks[type - 1];

	dir = sim->addr_bios + 128;
	dph = sim->addr_bios + 256 + 160 * drv;
	dpb = dph + 16;
	chk = dpb + 16;
	all = chk + (dt->drm + 1) / 4;
	end = all + (dt->dsm + 1 + 7) / 8;

	if ((end - dph) > 160) {
		return (0);
	}

	if ((end & 0xffff) < (sim->addr_bios & 0xffff)) {
		return (0);
	}

	mem_set_uint16_le (sim->mem, dph + 0, 0);
	mem_set_uint16_le (sim->mem, dph + 8, dir);
	mem_set_uint16_le (sim->mem, dph + 10, dpb);
	mem_set_uint16_le (sim->mem, dph + 12, chk);
	mem_set_uint16_le (sim->mem, dph + 14, all);

	if (mem_get_uint8 (sim->mem, sim->addr_bdos + 1) == 0x0e) {
		if (type == 1) {
			unsigned i;
			/* hack for cp/m 1.4 */

			dt->trn0 = map_0;
			dt->trn1 = map_0;

			mem_set_uint16_le (sim->mem, dph + 0, end);

			for (i = 0; i < 26; i++) {
				mem_set_uint8 (sim->mem, end + i, map_26_6[i]);
			}
		}
	}

	bsh = 0;
	while ((128U << bsh) < dt->bls) {
		bsh += 1;
	}

	mem_set_uint16_le (sim->mem, dpb + 0, dt->spt);
	mem_set_uint8     (sim->mem, dpb + 2, bsh);
	mem_set_uint8     (sim->mem, dpb + 3, (1 << bsh) - 1);
	mem_set_uint8     (sim->mem, dpb + 4, dt->exm);
	mem_set_uint16_le (sim->mem, dpb + 5, dt->dsm);
	mem_set_uint16_le (sim->mem, dpb + 7, dt->drm);
	mem_set_uint8     (sim->mem, dpb + 9, dt->al0 >> 8);
	mem_set_uint8     (sim->mem, dpb + 10, dt->al0 & 0xff);
	mem_set_uint16_le (sim->mem, dpb + 11, (dt->drm + 1) / 4);
	mem_set_uint16_le (sim->mem, dpb + 13, dt->off);

	return (dph);
}

/*
 * function 0: boot
 */
static
void bios_boot (cpm80_t *sim, int warm)
{
	unsigned      addr, size;
	unsigned char drv;
	unsigned char buf[128];
	char          str[128];

	/* endless loop, in case boot fails */
	mem_set_uint8 (sim->mem, 0x0000, 0xc3);
	mem_set_uint16_le (sim->mem, 0x0001, 0x0000);
	e8080_set_pc (sim->cpu, sim->addr_ccp);

	if (warm == 0) {
		con_puts (sim,
			"\x0d\x0a"
			"PCE-CPM80 BIOS VERSION " PCE_VERSION_STR
			"\x0d\x0a\x0d\x0a"
		);

		drv = 0;
	}
	else {
		drv = mem_get_uint8 (sim->mem, 4);
	}

	sim->addr_ccp = 0;
	sim->addr_bdos = 0;
	sim->addr_bios = 0;

	if (bios_setup_disk (sim, 0) == 0) {
		con_puts (sim, "UNKNOWN STARTUP DISK\x0d\x0a");
		return;
	}

	if (bios_read_sector (sim, buf, 0, 0, 0)) {
		con_puts (sim, "BOOT BLOCK READ ERROR\x0d\x0a");
		return;
	}

	if (memcmp (buf, "PCE-CPM80", 10) != 0) {
		con_puts (sim, "NO SYSTEM\x0d\x0a");
		return;
	}

	addr = buf_get_uint16_le (buf, 16);
	size = buf_get_uint16_le (buf, 18);

	sim->addr_ccp = buf_get_uint16_le (buf, 20);
	sim->addr_bdos = buf_get_uint16_le (buf, 22);
	sim->addr_bios = buf_get_uint16_le (buf, 24);

	if (warm == 0) {
		sprintf (str,
			"CCP=%04X  BDOS=%04X  BIOS=%04X  TPA=%uK\x0d\x0a",
			sim->addr_ccp,
			sim->addr_bdos,
			sim->addr_bios,
			(sim->addr_bdos - 0x0100) / 1024
		);

		con_puts (sim, str);
	}

	if (bios_read_system (sim, addr, size)) {
		con_puts (sim, "READ ERROR");
		return;
	}

	bios_init_traps (sim, 1);

	/* iobyte */
	mem_set_uint8 (sim->mem, 0x0003, 0x00);

	/* user / drive */
	mem_set_uint8 (sim->mem, 0x0004, 0x00);

	mem_set_uint8 (sim->mem, 0x0005, 0xc3);
	mem_set_uint16_le (sim->mem, 0x0006, sim->addr_bdos + 6);

	e8080_set_c (sim->cpu, drv);
	e8080_set_pc (sim->cpu, sim->addr_ccp);
}

/*
 * function 2: const
 */
static
void bios_const (cpm80_t *sim)
{
	e8080_set_a (sim->cpu, con_ready (sim) ? 0xff : 0x00);
}

/*
 * function 3: conin
 */
static
void bios_conin (cpm80_t *sim)
{
	unsigned char c;

	if (con_getc (sim, &c)) {
		c = 0x1a;
		e8080_set_pc (sim->cpu, e8080_get_pc (sim->cpu) - 1);
		c80_idle (sim);
	}

	e8080_set_a (sim->cpu, c);
}

/*
 * function 4: conout
 */
static
void bios_conout (cpm80_t *sim)
{
	unsigned char c;

	c = e8080_get_c (sim->cpu);

	if (con_putc (sim, c)) {
		e8080_set_pc (sim->cpu, e8080_get_pc (sim->cpu) - 1);
		c80_idle (sim);
	}
}

/*
 * function 5: list
 */
static
void bios_list (cpm80_t *sim)
{
	unsigned char c;

	c = e8080_get_c (sim->cpu);

	if (lst_putc (sim, c)) {
		e8080_set_pc (sim->cpu, e8080_get_pc (sim->cpu) - 1);
		c80_idle (sim);
	}
}

/*
 * function 6: punch
 */
static
void bios_punch (cpm80_t *sim)
{
	unsigned char c;

	c = e8080_get_c (sim->cpu);

	if (aux_putc (sim, c)) {
		e8080_set_pc (sim->cpu, e8080_get_pc (sim->cpu) - 1);
		c80_idle (sim);
	}
}

/*
 * function 7: reader
 */
static
void bios_reader (cpm80_t *sim)
{
	unsigned char c;

	if (aux_getc (sim, &c)) {
		c = 0x1a;
		e8080_set_pc (sim->cpu, e8080_get_pc (sim->cpu) - 1);
		c80_idle (sim);
	}

	e8080_set_a (sim->cpu, c);
}

/*
 * function 8: home
 */
static
void bios_home (cpm80_t *sim)
{
#if DEBUG_BIOS >= 1
	sim_log_deb ("BIOS: HOME\n");
#endif

	sim->bios_trk = 0;
}

/*
 * function 9: seldsk
 */
static
void bios_seldsk (cpm80_t *sim)
{
	unsigned addr;

	sim->bios_dsk = e8080_get_c (sim->cpu);

#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: SELDSK=%02X\n", sim->bios_dsk);
#endif

	bios_setup_disk (sim, sim->bios_dsk);

	addr = bios_setup_dpb (sim, sim->bios_dsk);

	e8080_set_hl (sim->cpu, addr);
}

/*
 * function 10: seltrk
 */
static
void bios_seltrk (cpm80_t *sim)
{
	sim->bios_trk = e8080_get_c (sim->cpu);

#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: SELTRK=%02X\n", sim->bios_trk);
#endif
}

/*
 * function 11: setsec
 */
static
void bios_setsec (cpm80_t *sim)
{
	sim->bios_sec = e8080_get_c (sim->cpu);

#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: SETSEC=%02X\n", sim->bios_sec);
#endif
}

/*
 * function 12: setdma
 */
static
void bios_setdma (cpm80_t *sim)
{
	sim->bios_dma = e8080_get_bc (sim->cpu);

#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: SETDMA=%04X\n", sim->bios_dma);
#endif
}

/*
 * function 13: read
 */
static
void bios_read (cpm80_t *sim)
{
	unsigned      i;
	unsigned char buf[128];

#if DEBUG_BIOS >= 1
	sim_log_deb ("BIOS: READ DMA=%04X D=%02X T=%02X S=%02X\n",
		sim->bios_dma, sim->bios_dsk, sim->bios_trk, sim->bios_sec
	);
#endif

	if (bios_read_sector (sim, buf, sim->bios_dsk, sim->bios_trk, sim->bios_sec)) {
		e8080_set_a (sim->cpu, 1);
		return;
	}

	for (i = 0; i < 128; i++) {
		mem_set_uint8 (sim->mem, sim->bios_dma + i, buf[i]);
	}

	e8080_set_a (sim->cpu, 0);
}

/*
 * function 14: write
 */
static
void bios_write (cpm80_t *sim)
{
	unsigned      i;
	unsigned char buf[128];

#if DEBUG_BIOS >= 1
	sim_log_deb ("BIOS: WRITE DMA=%04X D=%02X T=%02X S=%02X\n",
		sim->bios_dma, sim->bios_dsk, sim->bios_trk, sim->bios_sec
	);
#endif

	for (i = 0; i < 128; i++) {
		buf[i] = mem_get_uint8 (sim->mem, sim->bios_dma + i);
	}

	if (bios_write_sector (sim, buf, sim->bios_dsk, sim->bios_trk, sim->bios_sec)) {
		e8080_set_a (sim->cpu, 1);
		return;
	}

	e8080_set_a (sim->cpu, 0);
}

/*
 * function 15: listst
 */
static
void bios_listst (cpm80_t *sim)
{
	e8080_set_a (sim->cpu, 0);
}

/*
 * function 16: sectran
 */
static
void bios_sectran (cpm80_t *sim)
{
	unsigned sec, new, tab;

	sec = e8080_get_bc (sim->cpu);
	tab = e8080_get_de (sim->cpu);

	if (tab == 0) {
		new = sec;
	}
	else {
		if (sec > 25) {
			sim_log_deb ("bios: sectran: %u\n", sec);
		}

		new = mem_get_uint8 (sim->mem, tab + sec);
	}

#if DEBUG_BIOS >= 1
	sim_log_deb ("BIOS: SECTRANS %u -> %u\n", sec, new);
#endif

	e8080_set_hl (sim->cpu, new);
}

void c80_bios (cpm80_t *sim, unsigned fct)
{
	if (fct > 16) {
		return;
	}

#if DEBUG_BIOS >= 3
	sim_log_deb ("bios: function %02X\n", fct);
#endif

	switch (fct) {
	case 0:
		bios_boot (sim, 0);
		break;

	case 1:
		bios_boot (sim, 1);
		break;

	case 2:
		bios_const (sim);
		break;

	case 3:
		bios_conin (sim);
		break;

	case 4:
		bios_conout (sim);
		break;

	case 5:
		bios_list (sim);
		break;

	case 6:
		bios_punch (sim);
		break;

	case 7:
		bios_reader (sim);
		break;

	case 8:
		bios_home (sim);
		break;

	case 9:
		bios_seldsk (sim);
		break;

	case 10:
		bios_seltrk (sim);
		break;

	case 11:
		bios_setsec (sim);
		break;

	case 12:
		bios_setdma (sim);
		break;

	case 13:
		bios_read (sim);
		break;

	case 14:
		bios_write (sim);
		break;

	case 15:
		bios_listst (sim);
		break;

	case 16:
		bios_sectran (sim);
		break;

	default:
		sim_log_deb ("bios: unknown function: %02X\n", fct);
		break;
	}
}

void c80_bios_init (cpm80_t *sim)
{
	unsigned i;
	unsigned size;

	if (sim->ram != NULL) {
		size = mem_blk_get_size (sim->ram);
	}
	else {
		size = 20 * 1024;
	}

	sim->addr_bios = size - 0x600;

	sim->bios_dma = 0x0080;
	sim->bios_dsk = 0;
	sim->bios_trk = 0;
	sim->bios_sec = 0;

	sim->bios_disk_cnt = CPM80_DRIVE_MAX;

	for (i = 0; i < sim->bios_disk_cnt; i++) {
		sim->bios_disk_type[i] = 0;
	}

	bios_init_traps (sim, 0);
}
