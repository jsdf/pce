/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/sony.c                                      *
 * Created:     2007-11-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


/* sony variable offsets */
#define SONY_TRACK       0	/* current track */
#define SONY_WPROT       2	/* FF = write protected, 00 = writeable */
#define SONY_DISKINPLACE 3	/* 0 = no disk, 1 = inserted, 2 = accessed */
#define SONY_INSTALLED   4	/* 0 = unknown, 1 = installed, ff = not installed */
#define SONY_SIDES       5	/* 0 single sided, ff double sided */
#define SONY_QLINK       6
#define SONY_QTYPE       10
#define SONY_QDRIVENO    12
#define SONY_QREFNUM     14	/* driver reference number (0xfffb) */
#define SONY_QFSID       16
#define SONY_TWOSIDEFMT  18	/* ff double-sided, 0 single-sided */
#define SONY_NEWIF       19	/* ff new 800K interface, 0 old 400K interface */
#define SONY_DRIVEERRS   20	/* drive soft errors */

/* parameter block offsets */
#define PB_QLINK        0
#define PB_QTYPE        4
#define PB_IOTRAP       6
#define PB_IOCMDADDR    8
#define PB_IOCOMPLETION 12
#define PB_IORESULT     16
#define PB_IONAMEPTR    18
#define PB_IOVREFNUM    22
#define PB_IOREFNUM     24
#define PB_CSCODE       26
#define PB_CSPARAM      28
#define PB_IOBUFFER     32
#define PB_IOREQCOUNT   36
#define PB_IOACTCOUNT   40
#define PB_IOPOSMODE    44
#define PB_IOPOSOFFSET  46

/* device control entry offsets */
#define DC_CTLPOSITION  16

#define noQueueBit 0x0200

#define noErr      0
#define wPrErr     -44
#define paramErr   -50
#define nsDrvErr   -64
#define noDriveErr -64
#define offLineErr -65


void mac_sony_init (mac_sony_t *sony)
{
	sony->open = 0;
	sony->patched = 0;
	sony->delay = 0;
	sony->clock = 0;
	sony->check = 0;
	sony->icon[0] = 0;
	sony->icon[1] = 0;
}

void mac_sony_free (mac_sony_t *sony)
{
}

static
unsigned long mac_sony_get_vars (macplus_t *sim, unsigned drive)
{
	unsigned long ret;

	ret = e68_get_mem32 (sim->cpu, 0x0134);

	if ((drive >= 1) && (drive <= 4)) {
		ret += 8 + 66 * drive;
	}

	return (ret);
}

static
unsigned long mac_sony_find_pcex (macplus_t *sim, unsigned long addr)
{
	unsigned long cnt;

	if (mem_get_uint32_be (sim->mem, addr) != 0x50434558) {
		return (0);
	}

	cnt = mem_get_uint32_be (sim->mem, addr + 8);
	if (cnt < 4) {
		return (0);
	}

	sim->sony.check = addr + mem_get_uint32_be (sim->mem, addr + 16);
	sim->sony.icon[0] = addr + mem_get_uint32_be (sim->mem, addr + 20);
	sim->sony.icon[1] = addr + mem_get_uint32_be (sim->mem, addr + 24);

	addr += mem_get_uint32_be (sim->mem, addr + 12);

	return (addr);
}

static
unsigned long mac_sony_find (macplus_t *sim, unsigned long addr, unsigned long size)
{
	unsigned long sony;

	while (size > 0) {
		addr += 1;
		size -= 1;

		if (mem_get_uint16_be (sim->mem, addr - 1) != 0x052e) {
			continue;
		}

		if (mem_get_uint32_be (sim->mem, addr + 1) != 0x536f6e79) {
			continue;
		}

		sony = addr - 19;

		if (mem_get_uint16_be (sim->mem, sony) != 0x4f00) {
			continue;
		}

		return (sony);
	}

	return (0);
}

static
void mac_sony_unpatch_rom (macplus_t *sim)
{
	unsigned      i, j;
	unsigned long sony, pcex;
	unsigned      sofs, dofs;
	unsigned long sadr, dadr;
	unsigned char *buf;

	if (sim->sony.patched == 0) {
		return;
	}

	sony = sim->sony.sony_addr;
	pcex = sim->sony.pcex_addr;

	if ((sony == 0) || (pcex == 0)) {
		return;
	}

	buf = sim->sony.patch_buf;

	for (i = 0; i < 5; i++) {
		sofs = mem_get_uint16_be (sim->mem, sony + 8 + 2 * i);
		sadr = sony + sofs;

		dofs = mem_get_uint16_be (sim->mem, pcex + 8 + 2 * i);
		dadr = pcex + dofs;

		for (j = 0; j < 6; j++) {
			mem_set_uint8_rw (sim->mem, sadr + j, *(buf++));
		}

#ifdef DEBUG_SONY
		mac_log_deb ("sony: unpatch %u: %06lx -> %06lx\n", i, sadr, dadr);
#endif
	}

	sim->sony.patched = 0;
}

static
void mac_sony_patch_rom (macplus_t *sim, unsigned long sony, unsigned long pcex)
{
	unsigned      i, j;
	unsigned      sofs, dofs;
	unsigned long sadr, dadr;
	unsigned char *buf;

	buf = sim->sony.patch_buf;

	for (i = 0; i < 5; i++) {
		sofs = mem_get_uint16_be (sim->mem, sony + 8 + 2 * i);
		sadr = sony + sofs;

		dofs = mem_get_uint16_be (sim->mem, pcex + 8 + 2 * i);
		dadr = pcex + dofs;

		for (j = 0; j < 6; j++) {
			*(buf++) = mem_get_uint8 (sim->mem, sadr + j);
		}

		mem_set_uint8_rw (sim->mem, sadr + 0, 0x4e);
		mem_set_uint8_rw (sim->mem, sadr + 1, 0xf9);
		mem_set_uint8_rw (sim->mem, sadr + 2, (dadr >> 24) & 0xff);
		mem_set_uint8_rw (sim->mem, sadr + 3, (dadr >> 16) & 0xff);
		mem_set_uint8_rw (sim->mem, sadr + 4, (dadr >> 8) & 0xff);
		mem_set_uint8_rw (sim->mem, sadr + 5, dadr & 0xff);

#ifdef DEBUG_SONY
		mac_log_deb ("sony: patch %u: %06lx -> %06lx\n", i, sadr, dadr);
#endif
	}
}

void mac_sony_patch (macplus_t *sim)
{
	unsigned long pcex;
	unsigned long sony;

	if (sim->sony.patched) {
		return;
	}

	sim->sony.patched = 1;
	sim->sony.pcex_addr = 0;
	sim->sony.sony_addr = 0;

	pcex = mac_sony_find_pcex (sim, 0xf80000);
	if (pcex == 0) {
		pce_log_tag (MSG_ERR, "SONY:", "PCE ROM extension not found\n");
		return;
	}

	pce_log_tag (MSG_INF, "SONY:", "PCE ROM extension at 0x%06lx\n", pcex);

	sony = mac_sony_find (sim, 0x400000, 1024UL * 1024UL);
	if (sony == 0) {
		pce_log_tag (MSG_ERR, "SONY:", "sony driver not found\n");
		return;
	}

	pce_log_tag (MSG_INF, "SONY:", "sony driver at 0x%06lx\n", sony);

	sim->sony.pcex_addr = pcex;
	sim->sony.sony_addr = sony;

	mac_sony_patch_rom (sim, sony, pcex);
}

static
void mac_sony_check (macplus_t *sim)
{
	int           check;
	unsigned      i;
	unsigned long vars;
	unsigned long a7;
	disk_t        *dsk;

	if (sim->sony.open == 0) {
		return;
	}

	if (sim->sony.delay > 0) {
		sim->sony.delay -= 1;

		if (sim->sony.delay == 0) {
			for (i = 0; i < 4; i++) {
				vars = mac_sony_get_vars (sim, i + 1);

				e68_set_mem8 (sim->cpu, vars + SONY_DISKINPLACE, 0x01);
			}
		}
	}

	check = 0;

	for (i = 0; i < 4; i++) {
		dsk = dsks_get_disk (sim->dsks, i + 1);

		if (dsk != NULL) {
			vars = mac_sony_get_vars (sim, i + 1);

			if (dsk_get_block_cnt (dsk) < 1600) {
				e68_set_mem8 (sim->cpu, vars + SONY_TWOSIDEFMT, 0x00);
			}
			else {
				e68_set_mem8 (sim->cpu, vars + SONY_TWOSIDEFMT, 0xff);
			}

			if (dsk_get_readonly (dsk)) {
				e68_set_mem8 (sim->cpu, vars + SONY_WPROT, 0xff);
			}
			else {
				e68_set_mem8 (sim->cpu, vars + SONY_WPROT, 0x00);
			}

			if (e68_get_mem8 (sim->cpu, vars + SONY_DISKINPLACE) == 0x01) {
				check = 1;
			}
		}
	}

	if (check) {
#ifdef DEBUG_SONY
		mac_log_deb ("sony: check\n");
#endif
		if (e68_get_iml (sim->cpu) == 7) {
#ifdef DEBUG_SONY
			mac_log_deb ("sony: check aborted (iml=7)\n");
#endif
			return;
		}

		a7 = e68_get_areg32 (sim->cpu, 7);
		e68_set_mem32 (sim->cpu, a7 - 4, e68_get_pc (sim->cpu));
		e68_set_areg32 (sim->cpu, 7, a7 - 4);

		e68_set_pc (sim->cpu, sim->sony.check);
	}
}

void mac_sony_insert (macplus_t *sim, unsigned drive)
{
	unsigned long vars;
	disk_t        *dsk;

	if ((drive < 1) || (drive > 4)) {
		return;
	}

	dsk = dsks_get_disk (sim->dsks, drive);
	if (dsk == NULL) {
		return;
	}

	vars = mac_sony_get_vars (sim, drive);

	if (e68_get_mem8 (sim->cpu, vars + SONY_DISKINPLACE) == 0x00) {
		e68_set_mem8 (sim->cpu, vars + SONY_DISKINPLACE, 0x01);
	}
}

static
void mac_sony_return (macplus_t *sim, unsigned res, int rts)
{
	unsigned long pblk;
	unsigned      iotrap;

	pblk = e68_get_areg32 (sim->cpu, 0);

	if (res & 0x8000) {
		e68_set_dreg32 (sim->cpu, 0, 0xffff0000 | res);
	}
	else {
		e68_set_dreg32 (sim->cpu, 0, res);
	}

	iotrap = e68_get_mem16 (sim->cpu, pblk + PB_IOTRAP);

	e68_set_mem16 (sim->cpu, pblk + PB_IORESULT, res);

	if (rts || (iotrap & noQueueBit)) {
		e68_set_mem16 (sim->cpu, pblk + PB_IORESULT, res);
	}
	else {
		unsigned long val;

		val = e68_get_mem32 (sim->cpu, 0x0134);
		val = e68_get_mem32 (sim->cpu, val);
		e68_set_areg32 (sim->cpu, 1, val);

		e68_set_pc (sim->cpu, e68_get_mem32 (sim->cpu, 0x08fc));
	}
}

static
void mac_sony_open (macplus_t *sim)
{
#ifdef DEBUG_SONY
	mac_log_deb ("sony: open\n");
#endif

	sim->sony.open = 1;

	mac_sony_return (sim, 0x0000, 1);
}

static
unsigned long mac_sony_get_offset (macplus_t *sim)
{
	unsigned long pblk, dctl;
	unsigned long ofs;

	pblk = e68_get_areg32 (sim->cpu, 0);
	dctl = e68_get_areg32 (sim->cpu, 1);

	switch (e68_get_mem16 (sim->cpu, pblk + PB_IOPOSMODE) & 0x0f) {
	case 0: /* at mark */
		ofs = e68_get_mem32 (sim->cpu, dctl + DC_CTLPOSITION);
		break;

	case 1: /* from start */
		ofs = e68_get_mem32 (sim->cpu, pblk + PB_IOPOSOFFSET);
		break;

	case 2: /* from EOF */
		ofs = 819200 - e68_get_mem32 (sim->cpu, pblk + PB_IOPOSOFFSET);
		break;

	case 3: /* from mark */
		ofs = e68_get_mem32 (sim->cpu, pblk + PB_IOPOSOFFSET);
		ofs += e68_get_mem32 (sim->cpu, dctl + DC_CTLPOSITION);
		break;

	default:
		return (0);
	}

	return (ofs);
}

static
void mac_sony_read (macplus_t *sim, unsigned drive)
{
	unsigned long pblk, dctl, vars;
	unsigned long addr;
	unsigned long ofs, cnt;
	unsigned long i, n;
	unsigned      j;
	disk_t        *dsk;
	unsigned char buf[512];
	unsigned      posmode;

	pblk = e68_get_areg32 (sim->cpu, 0);
	dctl = e68_get_areg32 (sim->cpu, 1);

	dsk = dsks_get_disk (sim->dsks, drive);
	if (dsk == NULL) {
		mac_sony_return (sim, offLineErr, 0);
		return;
	}

	posmode = e68_get_mem16 (sim->cpu, pblk + PB_IOPOSMODE);

	if (posmode & 0x40) {
		/* verify */
		mac_sony_return (sim, 0x0000, 0);
		return;
	}

	addr = e68_get_mem32 (sim->cpu, pblk + PB_IOBUFFER) & 0x00ffffff;

	ofs = mac_sony_get_offset (sim);
	cnt = e68_get_mem32 (sim->cpu, pblk + PB_IOREQCOUNT);

	if ((ofs & 511) || (cnt & 511)) {
		mac_log_deb ("sony: non-aligned read\n");
		mac_sony_return (sim, paramErr, 0);
		return;
	}

	n = cnt / 512;

	for (i = 0; i < n; i++) {
		if (dsk_read_lba (dsk, buf, (ofs / 512) + i, 1)) {
			mac_log_deb (
				"sony: read drive %u %08lX + %04lX -> %08lX\n",
				drive, ofs, cnt, addr
			);
			mac_log_deb ("sony: read error\n");
			mac_sony_return (sim, 0xffff, 0);
			return;
		}

		for (j = 0; j < 512; j++) {
			e68_set_mem8 (sim->cpu, addr + 512 * i + j, buf[j]);
		}
	}

#ifdef DEBUG_SONY
	mac_log_deb ("sony: read drive %u: %08lX + %04lX -> %08lX\n", drive, ofs, cnt, addr);
#endif

	vars = mac_sony_get_vars (sim, drive);
	e68_set_mem8 (sim->cpu, vars + SONY_DISKINPLACE, 0x02);

	e68_set_mem16 (sim->cpu, pblk + PB_IORESULT, 0x0000);
	e68_set_mem32 (sim->cpu, pblk + PB_IOACTCOUNT, cnt);

	ofs = e68_get_mem32 (sim->cpu, dctl + DC_CTLPOSITION);
	e68_set_mem32 (sim->cpu, dctl + DC_CTLPOSITION, ofs + cnt);

	mac_sony_return (sim, 0x0000, 0);
}

static
void mac_sony_write (macplus_t *sim, unsigned drive)
{
	unsigned long pblk, dctl;
	unsigned long addr;
	unsigned long ofs, cnt;
	unsigned long i, n;
	unsigned      j;
	disk_t        *dsk;
	unsigned char buf[512];

	pblk = e68_get_areg32 (sim->cpu, 0);
	dctl = e68_get_areg32 (sim->cpu, 1);

	dsk = dsks_get_disk (sim->dsks, drive);
	if (dsk == NULL) {
		mac_sony_return (sim, -65, 0);
		return;
	}

	if (dsk_get_readonly (dsk)) {
		mac_sony_return (sim, -44, 0);
		return;
	}

	addr = e68_get_mem32 (sim->cpu, pblk + PB_IOBUFFER) & 0x00ffffff;

	ofs = mac_sony_get_offset (sim);
	cnt = e68_get_mem32 (sim->cpu, pblk + PB_IOREQCOUNT);

	if ((cnt & 511) || (ofs & 511)) {
		mac_log_deb ("sony: non-aligned write\n");
		mac_sony_return (sim, -50, 0);
		return;
	}

	n = cnt / 512;

	for (i = 0; i < n; i++) {
		for (j = 0; j < 512; j++) {
			buf[j] = e68_get_mem8 (sim->cpu, addr + 512 * i + j);
		}

		if (dsk_write_lba (dsk, buf, (ofs / 512) + i, 1)) {
			mac_log_deb ("sony: write error\n");
			mac_sony_return (sim, 0xffff, 0);
			return;
		}
	}

#ifdef DEBUG_SONY
	mac_log_deb ("sony: write drive %u: %08lX + %04lX <- %08lX\n", drive, ofs, cnt, addr);
#endif

	e68_set_mem16 (sim->cpu, pblk + PB_IORESULT, 0x0000);
	e68_set_mem32 (sim->cpu, pblk + PB_IOACTCOUNT, cnt);

	ofs = e68_get_mem32 (sim->cpu, dctl + DC_CTLPOSITION);
	e68_set_mem32 (sim->cpu, dctl + DC_CTLPOSITION, ofs + cnt);

	mac_sony_return (sim, 0x0000, 0);
}

static
void mac_sony_prime (macplus_t *sim)
{
	unsigned long pblk;
	unsigned long sony;
	unsigned      iotrap;
	unsigned      vref;

	pblk = e68_get_areg32 (sim->cpu, 0);

	iotrap = e68_get_mem16 (sim->cpu, pblk + PB_IOTRAP);
	vref = e68_get_mem16 (sim->cpu, pblk + PB_IOVREFNUM);

	if ((vref < 1) || (vref > 4)) {
		mac_sony_return (sim, nsDrvErr, 0);
		return;
	}

	sony = mac_sony_get_vars (sim, vref);

	if (e68_get_mem8 (sim->cpu, sony + SONY_DISKINPLACE) == 0) {
		mac_sony_return (sim, offLineErr, 0);
		return;
	}

	e68_set_mem8 (sim->cpu, sony + SONY_DISKINPLACE, 0x02);

	switch (iotrap & 0xff) {
	case 2: /* read */
		mac_sony_read (sim, vref);
		break;

	case 3: /* write */
		mac_sony_write (sim, vref);
		break;

	default:
		mac_sony_return (sim, 0xffef, 0);
		break;
	}
}

static
void mac_sony_format (macplus_t *sim, unsigned drive)
{
	unsigned long i, n;
	disk_t        *dsk;
	unsigned char buf[512];

	dsk = dsks_get_disk (sim->dsks, drive);
	if (dsk == NULL) {
		mac_sony_return (sim, 0xffbf, 0);
		return;
	}

	memset (buf, 0x00, 512);

	n = dsk_get_block_cnt (dsk);

	for (i = 0; i < n; i++) {
		dsk_write_lba (dsk, buf, i, 1);
	}

	mac_sony_return (sim, 0x0000, 0);
}

static
void mac_sony_get_disk_icon (macplus_t *sim, unsigned drive)
{
	disk_t        *dsk;
	unsigned long pblk;
	unsigned long addr, addr1, addr2;

	pblk = e68_get_areg32 (sim->cpu, 0);

	addr1 = sim->sony.icon[0];
	addr2 = sim->sony.icon[1];

	dsk = dsks_get_disk (sim->dsks, drive);

	if (dsk == NULL) {
		addr = addr1;
	}
	else {
		switch (dsk_get_block_cnt (dsk)) {
		case 800:
		case 1600:
			addr = addr1;
			break;

		default:
			addr = addr2;
			break;
		}
	}

	e68_set_mem32 (sim->cpu, pblk + PB_CSPARAM, addr);

	mac_sony_return (sim, 0x0000, 0);
}

static
void mac_sony_control (macplus_t *sim)
{
	unsigned long pblk;
	unsigned long sony;
	unsigned      vref, cscode;
	unsigned      ret;
	disk_t        *dsk;

	pblk = e68_get_areg32 (sim->cpu, 0);

	vref = e68_get_mem16 (sim->cpu, pblk + PB_IOVREFNUM);
	cscode = e68_get_mem16 (sim->cpu, pblk + PB_CSCODE);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control (%04X) %02X\n", cscode, vref);
#endif

	if ((vref < 1) || (vref > 4)) {
		mac_sony_return (sim, 0xffc8, 0);
		return;
	}

	ret = 0xffef;

	switch (cscode) {
	case 1: /* kill io */
		mac_sony_return (sim, 0xffff, 1);
		return;

	case 5: /* verify disk */
		dsk = dsks_get_disk (sim->dsks, vref);
		ret = (dsk == NULL) ? 0xffbf : 0x0000;
		break;

	case 7: /* eject disk */
		mac_log_deb ("sony: eject drive %u\n", vref);
		sony = mac_sony_get_vars (sim, vref);
		e68_set_mem8 (sim->cpu, sony + SONY_DISKINPLACE, 0x00);
		e68_set_mem8 (sim->cpu, sony + SONY_WPROT, 0x00);
		e68_set_mem8 (sim->cpu, sony + SONY_TWOSIDEFMT, 0x00);
		ret = 0x0000;
		break;

	case 9: /* track cache control */
		ret = 0xffc8;
		break;

	case 6: /* format disk */
		mac_sony_format (sim, vref);
		return;

	case 21: /* get drive icon */
		mac_sony_get_disk_icon (sim, vref);
		return;

	case 22: /* get media icon */
		mac_sony_get_disk_icon (sim, vref);
		return;

	case 23: /* get drive info */
		ret = 0x0000;

		if (vref == 1) {
			e68_set_mem32 (sim->cpu, pblk + PB_CSPARAM, 0x03);
		}
		else if (vref <= 4) {
			e68_set_mem32 (sim->cpu, pblk + PB_CSPARAM, 0x103);
		}
		else {
			ret = 0xffef;
		}
		break;

	default:
		ret = 0xffef;
		break;
	}

	mac_sony_return (sim, ret, 0);
}

static
void mac_sony_status (macplus_t *sim)
{
	unsigned long pblk;
	unsigned long dctl;
	unsigned      cscode, vref;
	unsigned long src, dst;
	unsigned      i;
	uint16_t      val;

	pblk = e68_get_areg32 (sim->cpu, 0);
	dctl = e68_get_areg32 (sim->cpu, 1);

	cscode = e68_get_mem16 (sim->cpu, pblk + PB_CSCODE);
	vref = e68_get_mem16 (sim->cpu, pblk + PB_IOVREFNUM);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: status cs=%04X vref=%04X)\n", cscode, vref);
#endif

	if ((vref < 1) || (vref > 4)) {
		mac_sony_return (sim, 0xffc8, 0);
		return;
	}

	switch (cscode) {
	case 6: /* return format list */
		{
			unsigned long ptr;
			disk_t        *dsk;

			dsk = dsks_get_disk (sim->dsks, vref);

			if (dsk == NULL) {
				mac_sony_return (sim, noDriveErr, 0);
				return;
			}

			ptr = e68_get_mem32 (sim->cpu, pblk + PB_CSPARAM + 2);
			e68_set_mem32 (sim->cpu, ptr, dsk_get_block_cnt (dsk));
			e68_set_mem32 (sim->cpu, ptr + 4, 0);
			e68_set_mem16 (sim->cpu, pblk + PB_CSPARAM, 1);
			mac_sony_return (sim, 0x0000, 0);
		}
		return;

	case 8: /* drive status */
#ifdef DEBUG_SONY
		mac_log_deb ("sony: status DriveStatus vref=%04X)\n", vref);
#endif

		src = mac_sony_get_vars (sim, vref);
		dst = pblk + PB_CSPARAM;

		for (i = 0; i < 11; i++) {
			val = e68_get_mem16 (sim->cpu, src + 2 * i);
			e68_set_mem16 (sim->cpu, dst + 2 * i, val);
		}

		mac_sony_return (sim, 0x0000, 0);
		return;
	}

	mac_sony_return (sim, 0xffee, 0);
}

int mac_sony_hook (macplus_t *sim, unsigned val)
{
	switch (val) {
	case MAC_HOOK_SONY_OPEN:
		mac_hook_skip (sim, 4);
		mac_sony_open (sim);
		return (0);

	case MAC_HOOK_SONY_PRIME:
		mac_hook_skip (sim, 4);
		mac_sony_prime (sim);
		return (0);

	case MAC_HOOK_SONY_CTRL:
		mac_hook_skip (sim, 4);
		mac_sony_control (sim);
		return (0);

	case MAC_HOOK_SONY_STATUS:
		mac_hook_skip (sim, 4);
		mac_sony_status (sim);
		return (0);

	case MAC_HOOK_SONY_CLOSE:
		mac_hook_skip (sim, 4);
		return (0);
	}

	return (1);
}

void mac_sony_reset (macplus_t *sim)
{
	mac_sony_unpatch_rom (sim);

	sim->sony.open = 0;

	sim->sony.delay = par_disk_delay;

	if (sim->sony.delay == 0) {
		sim->sony.delay = 1;
	}
}

void mac_sony_clock (macplus_t *sim, unsigned long n)
{
	sim->sony.clock += n;

	if (sim->sony.clock >= MAC_CPU_CLOCK) {
		sim->sony.clock -= MAC_CPU_CLOCK;

		mac_sony_check (sim);
	}
}
