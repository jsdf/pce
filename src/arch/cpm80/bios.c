/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/cpm80/bios.c                                        *
 * Created:     2012-11-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2021 Hampa Hug <hampa@hampa.ch>                     *
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
#include <lib/load.h>
#include <lib/log.h>


#ifndef DEBUG_BIOS
#define DEBUG_BIOS 0
#endif


#define ORD_CHS 0
#define ORD_HCS 1
#define ORD_HTS 2


typedef struct {
	unsigned short c;
	unsigned short h;
	unsigned short s;
	unsigned short i;
} c80_chsi_t;


typedef struct {
	const char     *name;

	unsigned short c;
	unsigned short h;
	unsigned short s;
	unsigned short n;

	unsigned char  order;	/* track order */
	unsigned short spt;	/* sectors per track */
	unsigned short bls;	/* block size */
	unsigned char  exm;	/* extent mask */
	unsigned short dsm;	/* number of blocks on disk */
	unsigned short drm;	/* number of dir entries */
	unsigned short al0;	/* directory allocation bitmap */
	unsigned short off;	/* system tracks */
	char           cpmtrn;	/* let cpm translate sectors */
	unsigned char  *trn0;
	unsigned char  *trn1;
} c80_disk_t;


static unsigned char map_26_6[] = {
	 1,  7, 13, 19, 25,  5, 11, 17, 23,  3,  9, 15, 21,
	 2,  8, 14, 20, 26,  6, 12, 18, 24,  4, 10, 16, 22
};

static unsigned char map_0[32] = {
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};


static c80_disk_t par_disks[] = {
	{ "IBM250",   77, 1, 26, 128, ORD_HTS, 26 * 1, 1024, 0,  243,   64, 0xc000, 2, 1, map_26_6 },
	{ "IBM500",   77, 2, 26, 128, ORD_HTS, 26 * 1, 2048, 1,  247,  128, 0xc000, 2, 0, NULL },
	{ "RX01",     77, 1, 26, 128, ORD_HTS, 26 * 1, 1024, 0,  243,   64, 0xc000, 2, 1, map_26_6 },
	{ "RX02",     77, 1, 26, 256, ORD_HTS, 26 * 2, 2048, 1,  243,  128, 0xc000, 2, 0, NULL },
	{ "IBM160",   40, 1,  8, 512, ORD_HTS,  8 * 4, 1024, 0,  156,   64, 0xc000, 1, 0, NULL },
	{ "IBM320",   40, 2,  8, 512, ORD_HTS,  8 * 4, 2048, 1,  158,   64, 0x8000, 1, 0, NULL },
	{ "IBM180",   40, 1,  9, 512, ORD_HTS,  9 * 4, 1024, 0,  175,   64, 0xc000, 1, 0, NULL },
	{ "IBM360",   40, 2,  9, 512, ORD_HTS,  9 * 4, 2048, 1,  177,  128, 0xc000, 1, 0, NULL },
	{ "IBM720",   80, 2,  9, 512, ORD_HTS,  9 * 4, 2048, 0,  355,  256, 0xf000, 2, 0, NULL },
	{ "IBM1200",  80, 2, 15, 512, ORD_HTS, 15 * 4, 4096, 1,  296,  256, 0xc000, 2, 0, NULL },
	{ "IBM1440",  80, 2, 18, 512, ORD_HTS, 18 * 4, 4096, 1,  355,  256, 0xc000, 2, 0, NULL },
	{ "PCE512",  256, 1,  4, 512, ORD_CHS,  4 * 4, 2048, 1,  254,  128, 0xc000, 1, 0, NULL },
	{ "PCE1024", 256, 1,  8, 512, ORD_CHS,  8 * 4, 2048, 0,  510,  128, 0xc000, 1, 0, NULL },
	{ "PCE2048", 256, 1, 16, 512, ORD_CHS, 16 * 4, 4096, 1,  510,  256, 0xc000, 1, 0, NULL },
	{ "PCE4096", 256, 1, 32, 512, ORD_CHS, 32 * 4, 4096, 1, 1020,  512, 0xf000, 1, 0, NULL },
	{ "PCE8192", 256, 1, 64, 512, ORD_CHS, 64 * 4, 4096, 1, 2040, 1024, 0xff00, 1, 0, NULL },
	{ "KAY1",     40, 1, 10, 512, ORD_CHS, 10 * 4, 1024, 0,  195,   64, 0xf000, 1, 0, map_0 },
	{ "KAY2",     40, 2, 10, 512, ORD_CHS, 10 * 4, 2048, 1,  197,   64, 0xc000, 1, 0, map_0, map_0 + 10 },
	{ "KAY3",     80, 2, 10, 512, ORD_CHS, 10 * 4, 4096, 3,  197,   96, 0x8000, 2, 0, map_0, map_0 + 20 },
	{ NULL, 0, 0, 0, 0 }
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

static
int con_read_buf (cpm80_t *sim)
{
	int c;

	if (sim->con_buf_cnt > 0) {
		return (0);
	}

	if (sim->con_read != NULL) {
		if ((c = fgetc (sim->con_read)) != EOF) {
			sim->con_buf = c & 0xff;
			sim->con_buf_cnt = 1;
			return (0);
		}

		fclose (sim->con_read);
		sim->con_read = NULL;
	}

	if (sim->con != NULL) {
		if (chr_read (sim->con, &sim->con_buf, 1) == 1) {
			if (sim->con_buf == 0) {
				c80_stop (sim);
				return (0);
			}

			sim->con_buf_cnt = 1;

			return (0);
		}
	}

	return (1);
}

static
int aux_read_buf (cpm80_t *sim)
{
	int c;

	if (sim->aux_buf_cnt > 0) {
		return (0);
	}

	if (sim->aux_read != NULL) {
		if ((c = fgetc (sim->aux_read)) != EOF) {
			sim->aux_buf = c & 0xff;
			sim->aux_buf_cnt = 1;
			return (0);
		}

		fclose (sim->aux_read);
		sim->aux_read = NULL;
	}

	if (sim->aux != NULL) {
		if (chr_read (sim->aux, &sim->aux_buf, 1) == 1) {
			sim->aux_buf_cnt = 1;
			return (0);
		}
	}

	return (1);
}

int con_ready (cpm80_t *sim)
{
	con_read_buf (sim);

	return (sim->con_buf_cnt > 0);
}

int con_getc (cpm80_t *sim, unsigned char *c)
{
	con_read_buf (sim);

	if (sim->con_buf_cnt > 0) {
		*c = sim->con_buf;
		sim->con_buf_cnt = 0;
		return (0);
	}

	return (1);
}

int con_putc (cpm80_t *sim, unsigned char c)
{
	if (sim->con_write != NULL) {
		fputc (c, sim->con_write);
	}
	else if (sim->con != NULL) {
		if (chr_write (sim->con, &c, 1) != 1) {
			return (1);
		}
	}

	return (0);
}

int con_puts (cpm80_t *sim, const char *str)
{
	unsigned n;

	if (sim->con_write != NULL) {
		fputs (str, sim->con_write);
	}
	else if (sim->con != NULL) {
		n = strlen (str);

		if (chr_write (sim->con, str, n) != n) {
			return (1);
		}
	}

	return (0);
}

int aux_ready (cpm80_t *sim)
{
	aux_read_buf (sim);

	return (sim->aux_buf_cnt > 0);
}

int aux_getc (cpm80_t *sim, unsigned char *c)
{
	aux_read_buf (sim);

	if (sim->aux_buf_cnt > 0) {
		*c = sim->aux_buf;
		sim->aux_buf_cnt = 0;
		return (0);
	}

	return (1);
}

int aux_putc (cpm80_t *sim, unsigned char c)
{
	if (sim->aux_write != NULL) {
		fputc (c, sim->aux_write);
	}
	else if (sim->aux != NULL) {
		if (chr_write (sim->aux, &c, 1) != 1) {
			return (1);
		}
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
int bios_get_disk (cpm80_t *sim, unsigned drv, disk_t **dsk, c80_disk_t **dt)
{
	unsigned type;

	if (drv >= sim->bios_disk_cnt) {
		return (1);
	}

	if ((type = sim->bios_disk_type[drv]) == 0) {
		return (1);
	}

	*dt = &par_disks[type - 1];

	if ((*dsk = dsks_get_disk (sim->dsks, drv)) == NULL) {
		return (1);
	}

	return (0);
}

/*
 * Map a CP/M sector to a cylinder, head, sector and subsector.
 */
static
int bios_map_sector (const c80_disk_t *dt, unsigned trk, unsigned sct, c80_chsi_t *chs)
{
	unsigned char *trn;

	if ((dt->order == ORD_HTS) || (dt->order == ORD_HCS)) {
		chs->c = trk % dt->c;
		chs->h = trk / dt->c;

		if (dt->order == ORD_HTS) {
			if (chs->h & 1) {
				chs->c = dt->c - chs->c - 1;
			}
		}
	}
	else {
		/* ORD_CHS */
		chs->c = trk / dt->h;
		chs->h = trk % dt->h;
	}

	chs->s = sct / (dt->n / 128);
	chs->i = sct % (dt->n / 128);

	if (dt->cpmtrn) {
		/* we let the bdos do the skew translation via bios_sectran */
		return (0);
	}

	trn = NULL;

	if (trk >= dt->off) {
		if (chs->h & 1) {
			trn = dt->trn1;
		}
		else {
			trn = dt->trn0;
		}
	}

	if (trn != NULL) {
		chs->s = trn[chs->s];
	}
	else {
		chs->s += 1;
	}

	return (0);
}

static
int bios_read_sector (cpm80_t *sim, void *buf, unsigned drv, unsigned trk, unsigned sct)
{
	unsigned      cnt;
	disk_t        *dsk;
	c80_disk_t    *dt;
	c80_chsi_t    chs;
	unsigned char tmp[2048];

	if (bios_get_disk (sim, drv, &dsk, &dt)) {
		return (1);
	}

	if (dt->n > 2048) {
		return (1);
	}

	if (bios_map_sector (dt, trk, sct, &chs)) {
		return (1);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		cnt = dt->n;

		if (dsk_psi_read_chs (dsk->ext, tmp, &cnt, chs.c, chs.h, chs.s, 0) != 0) {
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

		if (dsk_read_chs (dsk, tmp, chs.c, chs.h, chs.s, 1)) {
			return (1);
		}

	}

	memcpy (buf, tmp + 128 * chs.i, 128);

	return (0);
}

static
int bios_write_sector (cpm80_t *sim, const void *buf, unsigned drv, unsigned trk, unsigned sct)
{
	unsigned      cnt;
	disk_t        *dsk;
	c80_disk_t    *dt;
	c80_chsi_t    chs;
	unsigned char tmp[512];

	if (bios_get_disk (sim, drv, &dsk, &dt)) {
		return (1);
	}

	if (dt->n > 2048) {
		return (1);
	}

	if (bios_map_sector (dt, trk, sct, &chs)) {
		return (1);
	}

	if (dsk_get_type (dsk) == PCE_DISK_PSI) {
		cnt = dt->n;

		if (dsk_psi_read_chs (dsk->ext, tmp, &cnt, chs.c, chs.h, chs.s, 0) != 0) {
			return (1);
		}

		if (cnt != dt->n) {
			return (1);
		}

		memcpy (tmp + 128 * chs.i, buf, 128);

		if (dsk_psi_write_chs (dsk->ext, tmp, &cnt, chs.c, chs.h, chs.s, 0) != 0) {
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

		if (dsk_read_chs (dsk, tmp, chs.c, chs.h, chs.s, 1)) {
			return (1);
		}

		memcpy (tmp + 128 * chs.i, buf, 128);

		if (dsk_write_chs (dsk, tmp, chs.c, chs.h, chs.s, 1)) {
			return (1);
		}
	}

	return (0);
}

static
unsigned bios_get_sector_size (const c80_disk_t *dt, disk_t *dsk)
{
	c80_chsi_t chs;
	psi_img_t  *img;
	psi_sct_t  *sct;

	if (dsk->type != PCE_DISK_PSI) {
		return (512);
	}

	img = ((disk_psi_t *) dsk->ext)->img;

	if (img == NULL) {
		return (0);
	}

	bios_map_sector (dt, dt->off, 0, &chs);

	if (dt->cpmtrn) {
		chs.s = dt->trn0[chs.s];
	}

	sct = psi_img_get_sector (img, chs.c, chs.h, chs.s, 0);

	if (sct == NULL) {
		return (0);
	}

	return (sct->n);
}

static
unsigned bios_get_disk_type (cpm80_t *sim, unsigned drv)
{
	unsigned   i, ss;
	c80_disk_t *dt;
	disk_t     *dsk;

	if ((dsk = dsks_get_disk (sim->dsks, drv)) == NULL) {
		return (0);
	}

	i = 0;
	dt = par_disks;

	while (dt->c != 0) {
		if ((dsk->c == dt->c) && (dsk->h == dt->h) && (dsk->s == dt->s)) {
			ss = bios_get_sector_size (dt, dsk);

			if ((ss == 0) || (ss == dt->n)) {
				return (i + 1);
			}
		}

		i += 1;
		dt += 1;
	}

	fprintf (stderr, "unknown disk type (%u/%u/%u)\n", dsk->c, dsk->h, dsk->s);

	return (0);
}

static
unsigned bios_setup_disk (cpm80_t *sim, unsigned drv)
{
	unsigned      i;
	unsigned      type;
	unsigned long dph, dir, dpb, chk, all, trn, end;
	unsigned      bsh;
	c80_disk_t    *dt;

	if (drv >= sim->bios_disk_cnt) {
		return (0);
	}

	sim->bios_disk_type[drv] = 0;

	if ((type = bios_get_disk_type (sim, drv)) == 0) {
		mem_set_uint8 (sim->mem, 4, 0);
		return (0);
	}

	sim->bios_disk_type[drv] = type;

	dt = &par_disks[type - 1];

	if (sim->bios_disk_dph[drv] != 0) {
		return (sim->bios_disk_dph[drv]);
	}

	dir = sim->addr_bios + 128;
	dph = sim->bios_index;
	dpb = dph + 16;
	chk = dpb + 16;
	all = chk + dt->drm / 4;
	trn = 0;
	end = all + (dt->dsm + 7) / 8;

	if (dt->cpmtrn) {
		trn = end;
		end = trn + dt->spt;
	}

	if (end > sim->bios_limit) {
		fprintf (stderr, "bios: drive %u: no space for dph\n", drv);
		return (0);
	}

	sim->bios_disk_dph[drv] = dph;
	sim->bios_index = end;

	mem_set_uint16_le (sim->mem, dph + 0, trn);
	mem_set_uint16_le (sim->mem, dph + 8, dir);
	mem_set_uint16_le (sim->mem, dph + 10, dpb);
	mem_set_uint16_le (sim->mem, dph + 12, chk);
	mem_set_uint16_le (sim->mem, dph + 14, all);

	bsh = 0;
	while ((128U << bsh) < dt->bls) {
		bsh += 1;
	}

	mem_set_uint16_le (sim->mem, dpb + 0, dt->spt);
	mem_set_uint8     (sim->mem, dpb + 2, bsh);
	mem_set_uint8     (sim->mem, dpb + 3, (1 << bsh) - 1);
	mem_set_uint8     (sim->mem, dpb + 4, dt->exm);
	mem_set_uint16_le (sim->mem, dpb + 5, dt->dsm - 1);
	mem_set_uint16_le (sim->mem, dpb + 7, dt->drm - 1);
	mem_set_uint8     (sim->mem, dpb + 9, dt->al0 >> 8);
	mem_set_uint8     (sim->mem, dpb + 10, dt->al0 & 0xff);
	mem_set_uint16_le (sim->mem, dpb + 11, dt->drm / 4);
	mem_set_uint16_le (sim->mem, dpb + 13, dt->off);

	if (dt->cpmtrn) {
		for (i = 0; i < dt->spt; i++) {
			mem_set_uint8 (sim->mem, trn + i, dt->trn0[i]);
		}
	}

	return (dph);
}

/*
 * function 0: boot
 */
static
void bios_boot (cpm80_t *sim, int warm)
{
	unsigned      i;
	unsigned      tpa;
	unsigned char drv;
	char          str[128];
	unsigned char save[128];
	unsigned long ram;

	for (i = 0; i < 128; i++) {
		save[i] = mem_get_uint8 (sim->mem, 0x80 + i);
	}

	/* endless loop, in case boot fails */
	mem_set_uint8 (sim->mem, 0xc0, 0xc3);
	mem_set_uint16_le (sim->mem, 0xc1, 0x00c0);
	e8080_set_pc (sim->cpu, 0x00c0);

	if (warm == 0) {
		con_puts (sim, "\x0d\x0aPCE-CPM80 BIOS\x0d\x0a\x0d\x0a");
		drv = sim->boot;
	}
	else {
		drv = mem_get_uint8 (sim->mem, 4);
#if DEBUG_BIOS >=1
		fprintf (stderr, "bios: boot (%c:)\n", 'A' + drv);
#endif
	}

	if (pce_load_mem (sim->mem, sim->cpm, NULL, 0)) {
		con_puts (sim, "ERROR 01: ");
		con_puts (sim, sim->cpm);
		return;
	}

	if (mem_get_uint16_le (sim->mem, 0x80) != 0x3141) {
		con_puts (sim, "ERROR 02: ");
		con_puts (sim, sim->cpm);
		return;
	}

	sim->cpm_version = mem_get_uint8 (sim->mem, 0x82);
	sim->zcpr_version = mem_get_uint8 (sim->mem, 0x83);
	sim->addr_ccp = mem_get_uint16_le (sim->mem, 0x84);
	sim->addr_bdos = mem_get_uint16_le (sim->mem, 0x86);
	sim->addr_bios = mem_get_uint16_le (sim->mem, 0x88);
	sim->bios_limit = mem_get_uint16_le (sim->mem, 0x8a);

	if (sim->bios_limit == 0) {
		sim->bios_limit = 0x10000;
	}

	ram = mem_blk_get_size (sim->ram);

	if (sim->bios_limit > ram) {
		sim->bios_limit = ram;
	}

	if (warm == 0) {
		tpa = sim->addr_bdos - 0x0100;

		sprintf (str,
			"CCP=%04X BDOS=%04X BIOS=%04X END=%04lX\x0d\x0a"
			"\x0d\x0a"
			"CP/M %u.%u [%u.%02uK]",
			sim->addr_ccp, sim->addr_bdos, sim->addr_bios, sim->bios_limit,
			(sim->cpm_version >> 4) & 0x0f, sim->cpm_version & 0x0f,
			tpa / 1024, 100 * (tpa % 1024) / 1024
		);

		con_puts (sim, str);

		if (sim->zcpr_version > 0) {
			sprintf (str, " ZCPR%u", sim->zcpr_version);
			con_puts (sim, str);
		}

		con_puts (sim, "\x0d\x0a");

		/* iobyte */
		mem_set_uint8 (sim->mem, 0x0003, 0x00);

		/* user / drive */
		mem_set_uint8 (sim->mem, 0x0004, drv);
	}

	bios_init_traps (sim, 1);

	sim->bios_index = sim->addr_bios + 256;
	sim->bios_limit = mem_blk_get_size (sim->ram);

	for (i = 0; i < sim->bios_disk_cnt; i++) {
		sim->bios_disk_type[i] = 0;
		sim->bios_disk_dph[i] = 0;
	}

	mem_set_uint8 (sim->mem, 0x0005, 0xc3);
	mem_set_uint16_le (sim->mem, 0x0006, sim->addr_bdos + 6);

	for (i = 0; i < 128; i++) {
		mem_set_uint8 (sim->mem, 0x80 + i, save[i]);
	}

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
#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: %c: HOME\n", 'A' + sim->bios_dsk);
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

	addr = bios_setup_disk (sim, sim->bios_dsk);

#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: %c: SELDSK D=%u DPH=%04lX-%04lX+%04lX\n",
		'A' + sim->bios_dsk, sim->bios_dsk, addr, sim->bios_index,
		sim->bios_limit - sim->bios_index
	);
#endif

	if (addr == 0) {
		e8080_set_a (sim->cpu, 0xff);
	}

	e8080_set_hl (sim->cpu, addr);
}

/*
 * function 10: seltrk
 */
static
void bios_seltrk (cpm80_t *sim)
{
	sim->bios_trk = e8080_get_c (sim->cpu);

#if DEBUG_BIOS >= 3
	sim_log_deb ("BIOS: %c: SELTRK T=%02X\n",
		'A' + sim->bios_dsk, sim->bios_trk
	);
#endif
}

/*
 * function 11: setsec
 */
static
void bios_setsec (cpm80_t *sim)
{
	sim->bios_sec = e8080_get_c (sim->cpu);

#if DEBUG_BIOS >= 3
	sim_log_deb ("BIOS: %c: SETSEC S=%02X\n",
		'A' + sim->bios_dsk, sim->bios_sec
	);
#endif
}

/*
 * function 12: setdma
 */
static
void bios_setdma (cpm80_t *sim)
{
	sim->bios_dma = e8080_get_bc (sim->cpu);

#if DEBUG_BIOS >= 3
	sim_log_deb ("BIOS: %c: SETDMA A=%04X\n",
		'A' + sim->bios_dsk, sim->bios_dma
	);
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

#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: %c: READ T=%02X S=%02X A=%04X\n",
		'A' + sim->bios_dsk, sim->bios_trk, sim->bios_sec, sim->bios_dma
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

#if DEBUG_BIOS >= 2
	sim_log_deb ("BIOS: %c: WRITE T=%02X S=%02X A=%04X\n",
		'A' + sim->bios_dsk, sim->bios_trk, sim->bios_sec, sim->bios_dma
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
	e8080_set_a (sim->cpu, 0xff);
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
		new = mem_get_uint8 (sim->mem, tab + sec);
	}

#if DEBUG_BIOS >= 3
	sim_log_deb ("BIOS: %c: SECTRAN %u -> %u (T=%04X)\n",
		'A' + sim->bios_dsk, sec, new, tab
	);
#endif

	e8080_set_hl (sim->cpu, new);
}

void c80_bios (cpm80_t *sim, unsigned fct)
{
	if (fct > 16) {
		return;
	}

#if DEBUG_BIOS >= 4
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
		if (sim->cpm_version >= 0x20) {
			bios_listst (sim);
		}
		break;

	case 16:
		if (sim->cpm_version >= 0x20) {
			bios_sectran (sim);
		}
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
