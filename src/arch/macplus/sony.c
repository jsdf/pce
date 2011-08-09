/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/sony.c                                      *
 * Created:     2007-11-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include "sony.h"

#include <string.h>

#include <devices/memory.h>

#include <drivers/block/block.h>
#include <drivers/block/blkfdc.h>

#include <lib/log.h>


#define MAC_HOOK_SONY        16
#define MAC_HOOK_SONY_OPEN   (MAC_HOOK_SONY + 0)
#define MAC_HOOK_SONY_PRIME  (MAC_HOOK_SONY + 1)
#define MAC_HOOK_SONY_CTRL   (MAC_HOOK_SONY + 2)
#define MAC_HOOK_SONY_STATUS (MAC_HOOK_SONY + 3)
#define MAC_HOOK_SONY_CLOSE  (MAC_HOOK_SONY + 4)

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
#define qLink        0
#define qType        4
#define ioTrap       6
#define ioCmdAddr    8
#define ioCompletion 12
#define ioResult     16
#define ioNamePtr    18
#define ioVRefNum    22
#define ioRefNum     24
#define ioVersNum    26
#define ioPermssn    27
#define ioMisc       28
#define ioBuffer     32
#define ioReqCount   36
#define ioActCount   40
#define ioPosMode    44
#define ioPosOffset  46
#define csCode       26
#define csParam      28

/* device control entry offsets */
#define dCtlDriver   0
#define dCtlFlags    4
#define dCtlQHdr     6
#define dCtlPosition 16
#define dCtlStorage  20
#define dCtlRefNum   24
#define dCtlCurTicks 26

/* positioning modes */
#define fsAtMark    0
#define fsFromStart 1
#define fsFromMark  3
#define rdVerify    64

/* trap word flags */
#define asyncTrpBit 0x0100
#define noQueueBit  0x0200

/* result codes */
#define noErr      0
#define controlErr -17
#define statusErr  -18
#define readErr    -19
#define writeErr   -20
#define abortErr   -27
#define wPrErr     -44
#define paramErr   -50
#define nsDrvErr   -56
#define noDriveErr -64
#define offLineErr -65
#define verErr     -84


void mac_sony_init (mac_sony_t *sony)
{
	unsigned i;

	sony->open = 0;
	sony->patched = 0;

	sony->mem = NULL;
	sony->dsks = NULL;

	sony->check_addr = 0;
	sony->icon_addr[0] = 0;
	sony->icon_addr[1] = 0;

	sony->tag_buf = 0;

	sony->format_hd_as_dd = 0;
	sony->format_cnt = 0;

	for (i = 0; i < SONY_DRIVES; i++) {
		sony->delay_val[i] = 0;
		sony->delay_cnt[i] = 0;
	}
}

void mac_sony_free (mac_sony_t *sony)
{
}

void mac_sony_set_mem (mac_sony_t *sony, memory_t *mem)
{
	sony->mem = mem;
}

void mac_sony_set_disks (mac_sony_t *sony, disks_t *dsks)
{
	sony->dsks = dsks;
}

static
unsigned long mac_sony_get_vars (mac_sony_t *sony, unsigned drive)
{
	unsigned long ret;

	ret = mem_get_uint32_be (sony->mem, 0x0134);

	if ((drive >= 1) && (drive <= SONY_DRIVES)) {
		ret += 8 + 66 * drive;
	}

	return (ret);
}

static
unsigned long mac_sony_find_pcex (mac_sony_t *sony, unsigned long addr)
{
	unsigned long cnt;

	if (mem_get_uint32_be (sony->mem, addr) != 0x50434558) {
		return (0);
	}

	cnt = mem_get_uint32_be (sony->mem, addr + 8);

	if (cnt < 4) {
		return (0);
	}

	sony->check_addr = addr + mem_get_uint32_be (sony->mem, addr + 16);
	sony->icon_addr[0] = addr + mem_get_uint32_be (sony->mem, addr + 20);
	sony->icon_addr[1] = addr + mem_get_uint32_be (sony->mem, addr + 24);

	addr += mem_get_uint32_be (sony->mem, addr + 12);

	return (addr);
}

static
unsigned long mac_sony_find (mac_sony_t *sony, unsigned long addr, unsigned long size)
{
	unsigned long sony_addr;

	while (size > 0) {
		addr += 1;
		size -= 1;

		if (mem_get_uint16_be (sony->mem, addr - 1) != 0x052e) {
			continue;
		}

		if (mem_get_uint32_be (sony->mem, addr + 1) != 0x536f6e79) {
			continue;
		}

		sony_addr = addr - 19;

		if (mem_get_uint16_be (sony->mem, sony_addr) != 0x4f00) {
			continue;
		}

		return (sony_addr);
	}

	return (0);
}

static
void mac_sony_unpatch_rom (mac_sony_t *sony)
{
	unsigned      i, j;
	unsigned      sofs;
	unsigned long sadr;
	unsigned char *buf;

	if (sony->patched == 0) {
		return;
	}

	if ((sony->sony_addr == 0) || (sony->pcex_addr == 0)) {
		return;
	}

	buf = sony->patch_buf;

	for (i = 0; i < 5; i++) {
		sofs = mem_get_uint16_be (sony->mem, sony->sony_addr + 8 + 2 * i);
		sadr = sony->sony_addr + sofs;

		for (j = 0; j < 6; j++) {
			mem_set_uint8_rw (sony->mem, sadr + j, *(buf++));
		}
	}

	sony->patched = 0;
}

static
void mac_sony_patch_rom (mac_sony_t *sony)
{
	unsigned      i, j;
	unsigned      sofs, dofs;
	unsigned long sadr, dadr;
	unsigned char *buf;

	if ((sony->sony_addr == 0) || (sony->pcex_addr == 0)) {
		return;
	}

	buf = sony->patch_buf;

	for (i = 0; i < 5; i++) {
		sofs = mem_get_uint16_be (sony->mem, sony->sony_addr + 8 + 2 * i);
		sadr = sony->sony_addr + sofs;

		dofs = mem_get_uint16_be (sony->mem, sony->pcex_addr + 8 + 2 * i);
		dadr = sony->pcex_addr + dofs;

		for (j = 0; j < 6; j++) {
			*(buf++) = mem_get_uint8 (sony->mem, sadr + j);
		}

		mem_set_uint8_rw (sony->mem, sadr + 0, 0x4e);
		mem_set_uint8_rw (sony->mem, sadr + 1, 0xf9);
		mem_set_uint8_rw (sony->mem, sadr + 2, (dadr >> 24) & 0xff);
		mem_set_uint8_rw (sony->mem, sadr + 3, (dadr >> 16) & 0xff);
		mem_set_uint8_rw (sony->mem, sadr + 4, (dadr >> 8) & 0xff);
		mem_set_uint8_rw (sony->mem, sadr + 5, dadr & 0xff);

#ifdef DEBUG_SONY
		mac_log_deb ("sony: patch %u: %06lx -> %06lx\n", i, sadr, dadr);
#endif
	}
}

void mac_sony_patch (mac_sony_t *sony)
{
	if (sony->patched) {
		return;
	}

	sony->patched = 1;

	sony->pcex_addr = 0;
	sony->sony_addr = 0;

	sony->pcex_addr = mac_sony_find_pcex (sony, 0xf80000);

	if (sony->pcex_addr == 0) {
		pce_log_tag (MSG_ERR, "SONY:", "PCE ROM extension not found\n");
		return;
	}

	pce_log_tag (MSG_INF, "SONY:", "PCE ROM extension at 0x%06lx\n", sony->pcex_addr);

	sony->sony_addr = mac_sony_find (sony, 0x400000, 1024UL * 1024UL);

	if (sony->sony_addr == 0) {
		pce_log_tag (MSG_ERR, "SONY:", "sony driver not found\n");
		return;
	}

	pce_log_tag (MSG_INF, "SONY:", "sony driver at 0x%06lx\n", sony->sony_addr);

	mac_sony_patch_rom (sony);
}

void mac_sony_set_delay (mac_sony_t *sony, unsigned drive, unsigned delay)
{
	if (drive < SONY_DRIVES) {
		sony->delay_val[drive] = delay;
		sony->delay_cnt[drive] = delay;
	}
}

void mac_sony_insert (mac_sony_t *sony, unsigned drive)
{
	unsigned long vars;
	disk_t        *dsk;

	if ((drive < 1) || (drive > SONY_DRIVES)) {
		return;
	}

	dsk = dsks_get_disk (sony->dsks, drive);

	if (dsk == NULL) {
		return;
	}

	vars = mac_sony_get_vars (sony, drive);

	if (mem_get_uint8 (sony->mem, vars + SONY_DISKINPLACE) == 0x00) {
		pce_log_tag (MSG_INF, "SONY:", "insert drive %u\n", drive);
		mem_set_uint8 (sony->mem, vars + SONY_DISKINPLACE, 0x01);
	}
}

/*
 * Check if disks need to be inserted.
 */
int mac_sony_check (mac_sony_t *sony)
{
	int           check;
	unsigned      i;
	unsigned long vars;
	disk_t        *dsk;

	if (sony->open == 0) {
		return (0);
	}

	check = 0;

	for (i = 0; i < SONY_DRIVES; i++) {
		if (sony->delay_cnt[i] > 0) {
			sony->delay_cnt[i] -= 1;

			if (sony->delay_cnt[i] == 0) {
				mac_sony_insert (sony, i + 1);
			}
		}

		dsk = dsks_get_disk (sony->dsks, i + 1);

		if (dsk != NULL) {
			vars = mac_sony_get_vars (sony, i + 1);

			if (dsk_get_block_cnt (dsk) < 1600) {
				mem_set_uint8 (sony->mem, vars + SONY_TWOSIDEFMT, 0x00);
			}
			else {
				mem_set_uint8 (sony->mem, vars + SONY_TWOSIDEFMT, 0xff);
			}

			if (dsk_get_readonly (dsk)) {
				mem_set_uint8 (sony->mem, vars + SONY_WPROT, 0xff);
			}
			else {
				mem_set_uint8 (sony->mem, vars + SONY_WPROT, 0x00);
			}

			if (mem_get_uint8 (sony->mem, vars + SONY_DISKINPLACE) == 0x01) {
				check = 1;
			}
		}
	}

	return (check);
}


static
void mac_sony_open (mac_sony_t *sony)
{
#ifdef DEBUG_SONY
	mac_log_deb ("sony: open\n");
#endif

	sony->open = 1;

	if (mac_sony_check (sony)) {
		sony->pc = sony->check_addr;
	}
}


static
unsigned long mac_sony_get_pblk (mac_sony_t *sony, unsigned ofs, unsigned size)
{
	if (size == 1) {
		return (mem_get_uint8 (sony->mem, sony->a0 + ofs));
	}
	else if (size == 2) {
		return (mem_get_uint16_be (sony->mem, sony->a0 + ofs));
	}
	else if (size == 4) {
		return (mem_get_uint32_be (sony->mem, sony->a0 + ofs));
	}

	return (0);
}

static
void mac_sony_set_pblk (mac_sony_t *sony, unsigned ofs, unsigned size, unsigned long val)
{
	if (size == 1) {
		mem_set_uint8 (sony->mem, sony->a0 + ofs, val);
	}
	else if (size == 2) {
		mem_set_uint16_be (sony->mem, sony->a0 + ofs, val);
	}
	else if (size == 4) {
		mem_set_uint32_be (sony->mem, sony->a0 + ofs, val);
	}
}

static
unsigned long mac_sony_get_dctl (mac_sony_t *sony, unsigned ofs, unsigned size)
{
	if (size == 1) {
		return (mem_get_uint8 (sony->mem, sony->a1 + ofs));
	}
	else if (size == 2) {
		return (mem_get_uint16_be (sony->mem, sony->a1 + ofs));
	}
	else if (size == 4) {
		return (mem_get_uint32_be (sony->mem, sony->a1 + ofs));
	}

	return (0);
}

static
void mac_sony_set_dctl (mac_sony_t *sony, unsigned ofs, unsigned size, unsigned long val)
{
	if (size == 1) {
		mem_set_uint8 (sony->mem, sony->a1 + ofs, val);
	}
	else if (size == 2) {
		mem_set_uint16_be (sony->mem, sony->a1 + ofs, val);
	}
	else if (size == 4) {
		mem_set_uint32_be (sony->mem, sony->a1 + ofs, val);
	}
}

static
void mac_sony_return (mac_sony_t *sony, unsigned res, int rts)
{
	unsigned      trap;
	unsigned long val;

	sony->d0 = (res & 0x8000) ? (0xffff0000 | res) : 0;

	trap = mac_sony_get_pblk (sony, ioTrap, 2);

	mac_sony_set_pblk (sony, ioResult, 2, res);

	if ((rts == 0) && ((trap & noQueueBit) == 0)) {
		val = mem_get_uint32_be (sony->mem, 0x0134);
		val = mem_get_uint32_be (sony->mem, val);

		sony->a1 = val;
		sony->pc = mem_get_uint32_be (sony->mem, 0x08fc);
	}
}


static
unsigned long mac_sony_get_offset (mac_sony_t *sony)
{
	unsigned      posmode;
	unsigned long ofs;

	posmode = mac_sony_get_pblk (sony, ioPosMode, 2);

	switch (posmode & 0x0f) {
	case fsAtMark:
		ofs = mac_sony_get_dctl (sony, dCtlPosition, 4);
		break;

	case fsFromStart:
		ofs = mac_sony_get_pblk (sony, ioPosOffset, 4);
		break;

	case fsFromMark:
		ofs = mac_sony_get_pblk (sony, ioPosOffset, 4);
		ofs += mac_sony_get_dctl (sony, dCtlPosition, 4);
		break;

	default:
		return (0);
	}

	return (ofs);
}

/*
 * Get the disk type from a number of blocks
 */
static
unsigned mac_sony_get_disk_type (unsigned long blk)
{
	if (blk < ((800 + 1440) / 2)) {
		return (0);
	}
	else if (blk < ((1440 + 1600) / 2)) {
		return (2);
	}
	else if (blk < ((1600 + 2880) / 2)) {
		return (1);
	}
	else if (blk < (2 * 2880)) {
		return (3);
	}

	return (255);
}

static
int mac_sony_get_chs (unsigned long blk, unsigned long lba, unsigned *c, unsigned *h, unsigned *s)
{
	unsigned type;
	unsigned i, hn, sn;

	type = mac_sony_get_disk_type (blk);

	if ((type == 0) || (type == 1)) {
		hn = (type == 0) ? 1 : 2;
		sn = 12;
		*c = 0;

		for (i = 0; i < 5; i++) {
			if (lba < (16 * hn * sn)) {
				*s = (lba % sn);
				*h = (lba / sn) % hn;
				*c += lba / (sn * hn);
				return (0);
			}

			lba -= 16 * hn * sn;
			*c += 16;
			sn -= 1;
		}

		return (1);
	}
	else if (type == 2) {
		*s = (lba % 9) + 1;
		*h = (lba / 9) % 2;
		*c = lba / 18;
	}
	else if (type == 3) {
		*s = (lba % 18) + 1;
		*h = (lba / 18) % 2;
		*c = lba / 36;
	}
	else {
		mac_log_deb ("sony: chs error (blk=%lu, lba=%lu)\n", blk, lba);
		return (1);
	}

	return (0);
}

static
int mac_sony_read_block (disk_t *dsk, void *buf, void *tag, unsigned long idx)
{
	unsigned c, h, s;
	unsigned cnt;

	if (dsk_get_type (dsk) != PCE_DISK_FDC) {
		memset (tag, 0, 12);

		if (dsk_read_lba (dsk, buf, idx, 1)) {
			return (1);
		}

		return (0);
	}

	if (mac_sony_get_chs (dsk_get_block_cnt (dsk), idx, &c, &h, &s)) {
		return (1);
	}

	dsk_fdc_read_tags (dsk->ext, tag, 12, c, h, s, 0);

	cnt = 512;

	if (dsk_fdc_read_chs (dsk->ext, buf, &cnt, c, h, s, 0) != PCE_BLK_FDC_OK) {
		mac_log_deb ("sony: read error at %u/%u/%u\n", c, h, s);
		return (1);
	}

	return (0);
}

static
int mac_sony_write_block (disk_t *dsk, const void *buf, const void *tag, unsigned long idx)
{
	unsigned c, h, s;
	unsigned type;
	unsigned cnt;

	if (dsk_get_type (dsk) != PCE_DISK_FDC) {
		if (dsk_write_lba (dsk, buf, idx, 1)) {
			return (1);
		}

		return (0);
	}

	if (mac_sony_get_chs (dsk_get_block_cnt (dsk), idx, &c, &h, &s)) {
		return (1);
	}

	type = mac_sony_get_disk_type (dsk_get_block_cnt (dsk));

	if ((type == 0) || (type == 1)) {
		dsk_fdc_write_tags (dsk->ext, tag, 12, c, h, s, 0);
	}

	cnt = 512;

	if (dsk_fdc_write_chs (dsk->ext, buf, &cnt, c, h, s, 0) != PCE_BLK_FDC_OK) {
		mac_log_deb ("sony: write error at %u/%u/%u\n", c, h, s);
		return (1);
	}

	return (0);
}

static
void mac_sony_prime_read (mac_sony_t *sony, unsigned drive)
{
	unsigned long addr, vars;
	unsigned long ofs, cnt;
	unsigned long i, n;
	unsigned      j;
	disk_t        *dsk;
	unsigned char buf[512];
	unsigned char tag[12];
	unsigned      posmode;

	ofs = mac_sony_get_offset (sony);
	cnt = mac_sony_get_pblk (sony, ioReqCount, 4);
	addr = mac_sony_get_pblk (sony, ioBuffer, 4) & 0x00ffffff;
	posmode = mac_sony_get_pblk (sony, ioPosMode, 2);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: prime: read (drive=%u, ofs=0x%08lx, cnt=0x%04lx, addr=0x%08lx)\n",
		drive, ofs, cnt, addr
	);
#endif

	dsk = dsks_get_disk (sony->dsks, drive);

	if (dsk == NULL) {
		mac_sony_return (sony, offLineErr, 0);
		return;
	}

	if (posmode & 0x40) {
		/* verify */
		mac_sony_return (sony, noErr, 0);
		return;
	}

	if ((ofs & 511) || (cnt & 511)) {
		mac_log_deb ("sony: non-aligned read\n");
		mac_sony_return (sony, paramErr, 0);
		return;
	}

	n = cnt / 512;

	for (i = 0; i < n; i++) {
		if (mac_sony_read_block (dsk, buf, tag, (ofs / 512) + i)) {
			mac_log_deb ("sony: read error\n");
			mac_sony_return (sony, 0xffff, 0);
			return;
		}

		for (j = 0; j < 512; j++) {
			mem_set_uint8 (sony->mem, addr + 512 * i + j, buf[j]);
		}

		for (j = 0; j < 12; j++) {
			mem_set_uint8 (sony->mem, 0x2fc + j, tag[j]);
		}

		if (sony->tag_buf != 0) {
			for (j = 0; j < 12; j++) {
				mem_set_uint8 (sony->mem, sony->tag_buf + 12 * i + j, tag[j]);
			}
		}
	}

	vars = mac_sony_get_vars (sony, drive);
	mem_set_uint8 (sony->mem, vars + SONY_DISKINPLACE, 0x02);

	mac_sony_set_pblk (sony, ioActCount, 4, cnt);

	ofs = mac_sony_get_dctl (sony, dCtlPosition, 4);
	mac_sony_set_dctl (sony, dCtlPosition, 4, ofs + cnt);

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_prime_write (mac_sony_t *sony, unsigned drive)
{
	unsigned long addr;
	unsigned long ofs, cnt;
	unsigned      relblk;
	unsigned long i, n;
	unsigned      j;
	disk_t        *dsk;
	unsigned char buf[512];
	unsigned char tag[12];

	ofs = mac_sony_get_offset (sony);
	cnt = mac_sony_get_pblk (sony, ioReqCount, 4);
	addr = mac_sony_get_pblk (sony, ioBuffer, 4) & 0x00ffffff;

#ifdef DEBUG_SONY
	mac_log_deb ("sony: prime: write (drive=%u, ofs=0x%08lx, cnt=0x%04lx, addr=0x%08lx)\n",
		drive, ofs, cnt, addr
	);
#endif

	dsk = dsks_get_disk (sony->dsks, drive);

	if (dsk == NULL) {
		mac_sony_return (sony, offLineErr, 0);
		return;
	}

	if (dsk_get_readonly (dsk)) {
		mac_sony_return (sony, wPrErr, 0);
		return;
	}

	if ((cnt & 511) || (ofs & 511)) {
		mac_log_deb ("sony: non-aligned write\n");
		mac_sony_return (sony, paramErr, 0);
		return;
	}

	memset (tag, 0, 12);

	relblk = mem_get_uint16_be (sony->mem, 0x302);

	n = cnt / 512;

	for (i = 0; i < n; i++) {
		for (j = 0; j < 512; j++) {
			buf[j] = mem_get_uint8 (sony->mem, addr + 512 * i + j);
		}

		if (sony->tag_buf != 0) {
			for (j = 0; j < 12; j++) {
				tag[j] = mem_get_uint8 (sony->mem, sony->tag_buf + 12 * i + j);
				mem_set_uint8 (sony->mem, 0x2fc + j, tag[j]);
			}
		}
		else {
			mem_set_uint16_be (sony->mem, 0x302, relblk + i);

			for (j = 0; j < 12; j++) {
				tag[j] = mem_get_uint8 (sony->mem, 0x2fc + j);
			}
		}

		if (mac_sony_write_block (dsk, buf, tag, (ofs / 512) + i)) {
			mac_log_deb ("sony: write error\n");
			mac_sony_return (sony, 0xffff, 0);
			return;
		}
	}

	mac_sony_set_pblk (sony, ioActCount, 4, cnt);

	ofs = mac_sony_get_dctl (sony, dCtlPosition, 4);
	mac_sony_set_dctl (sony, dCtlPosition, 4, ofs + cnt);

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_prime (mac_sony_t *sony)
{
	unsigned long vars;
	unsigned      trap, vref;

	trap = mac_sony_get_pblk (sony, ioTrap, 2);
	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	vars = mac_sony_get_vars (sony, vref);

	if (mem_get_uint8 (sony->mem, vars + SONY_DISKINPLACE) == 0) {
		mac_sony_return (sony, offLineErr, 0);
		return;
	}

	mem_set_uint8 (sony->mem, vars + SONY_DISKINPLACE, 0x02);

	switch (trap & 0xff) {
	case 2: /* read */
		mac_sony_prime_read (sony, vref);
		break;

	case 3: /* write */
		mac_sony_prime_write (sony, vref);
		break;

	default:
		mac_log_deb ("sony: prime: unknown (trap=0x%04x)\n", trap);
		mac_sony_return (sony, 0xffef, 0);
		break;
	}
}


static
int mac_sony_format (disk_t *dsk, unsigned long blk)
{
	unsigned long i, n;
	unsigned      c, h, s, hn, sn;
	unsigned      type;
	unsigned char buf[512];
	pfdc_sct_t    *sct;
	disk_fdc_t    *fdc;

	if (dsk_get_type (dsk) != PCE_DISK_FDC) {
		n = dsk_get_block_cnt (dsk);

		if (n != blk) {
			return (1);
		}

		memset (buf, 0x00, 512);

		for (i = 0; i < n; i++) {
			dsk_write_lba (dsk, buf, i, 1);
		}

		return (0);
	}

	fdc = dsk->ext;

	memset (buf, 0, 12);

	type = mac_sony_get_disk_type (blk);

	dsk_fdc_erase_disk (fdc);

	if ((type == 0) || (type == 1)) {
		dsk_fdc_set_encoding (fdc, PFDC_ENC_GCR, 250000);

		hn = (type == 0) ? 1 : 2;
		sn = 13;

		for (c = 0; c < 80; c++) {
			if ((c & 15) == 0) {
				sn -= 1;
			}

			for (h = 0; h < hn; h++) {
				for (s = 0; s < sn; s++) {
					dsk_fdc_format_sector (fdc, c, h, c, h, s, 512, 0);

					sct = pfdc_img_get_sector (fdc->img, c, h, s, 1);

					if (sct != NULL) {
						pfdc_sct_set_tags (sct, buf, 12);
					}
				}
			}
		}
	}
	else if ((type == 2) || (type == 3)) {
		dsk_fdc_set_encoding (fdc, PFDC_ENC_MFM, (type == 2) ? 250000 : 500000);

		sn = (type == 2) ? 9 : 18;

		for (c = 0; c < 80; c++) {
			for (h = 0; h < 2; h++) {
				for (s = 0; s < sn; s++) {
					dsk_fdc_format_sector (fdc, c, h, c, h, s + 1, 512, 0);
				}
			}
		}
	}
	else {
		return (1);
	}

	mac_log_deb ("sony: formatted disk (%lu blocks)\n", dsk_get_block_cnt (dsk));

	return (0);
}

static
void mac_sony_ctl_verify (mac_sony_t *sony)
{
	unsigned vref;
	disk_t   *dsk;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control: verify (drive=%u)\n", vref);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	dsk = dsks_get_disk (sony->dsks, vref);

	if (dsk == NULL) {
		mac_sony_return (sony, noDriveErr, 0);
		return;
	}

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_ctl_eject (mac_sony_t *sony)
{
	unsigned      vref;
	unsigned long vars;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control eject (drive=%u)\n", vref);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	vars = mac_sony_get_vars (sony, vref);

	mem_set_uint8 (sony->mem, vars + SONY_DISKINPLACE, 0x00);
	mem_set_uint8 (sony->mem, vars + SONY_WPROT, 0x00);
	mem_set_uint8 (sony->mem, vars + SONY_TWOSIDEFMT, 0x00);

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_ctl_set_tag_buf (mac_sony_t *sony)
{
	unsigned long tagbuf;

	tagbuf = mac_sony_get_pblk (sony, csParam, 4);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control: set tag buffer (buf=0x%08lx)\n", tagbuf);
#endif

	sony->tag_buf = tagbuf & 0x00ffffff;

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_ctl_format (mac_sony_t *sony)
{
	unsigned      vref, format;
	unsigned long blk;
	disk_t        *dsk;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);
	format = mac_sony_get_pblk (sony, csParam, 2);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control format (drive=%u, format=%u)\n", vref, format);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	dsk = dsks_get_disk (sony->dsks, vref);

	if (dsk == NULL) {
		mac_sony_return (sony, noDriveErr, 0);
		return;
	}

	if (dsk->readonly) {
		mac_sony_return (sony, wPrErr, 0);
		return;
	}

	if ((format > 0) && (format <= sony->format_cnt)) {
		blk = sony->format_list[2 * (format - 1)];
	}
	else {
		blk = dsk_get_block_cnt (dsk);
	}

	if (mac_sony_format (dsk, blk)) {
		mac_sony_return (sony, paramErr, 0);
		return;
	}

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_ctl_get_icon (mac_sony_t *sony, int which)
{
	unsigned      vref;
	unsigned long addr, addr1, addr2;
	disk_t        *dsk;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control: get %s icon (drive=%u)\n",
		which ? "media" : "drive", vref
	);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	addr1 = sony->icon_addr[0];
	addr2 = sony->icon_addr[1];

	dsk = dsks_get_disk (sony->dsks, vref);

	if (dsk == NULL) {
		addr = addr1;
	}
	else {
		switch (dsk_get_block_cnt (dsk)) {
		case 800:
		case 1600:
		case 1440:
		case 2880:
			addr = addr1;
			break;

		default:
			addr = addr2;
			break;
		}
	}

	mac_sony_set_pblk (sony, csParam, 4, addr);

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_ctl_get_drive_info (mac_sony_t *sony)
{
	unsigned vref, val;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control: get drive info (drive=%u)\n", vref);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	val = vref - 1;
	val = ((val << 3) & 8) | ((val >> 1) & 1);

	mac_sony_set_pblk (sony, csParam, 4, (val << 8) | 0x04);

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_ctl_format_copy (mac_sony_t *sony)
{
	unsigned      i;
	unsigned      vref, format, type;
	unsigned long data, tags;
	unsigned long idx, blk;
	unsigned char buf[512];
	unsigned char tag[12];
	disk_t        *dsk;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);

	format = mac_sony_get_pblk (sony, csParam, 2);
	data = mac_sony_get_pblk (sony, csParam + 2, 4);
	tags = mac_sony_get_pblk (sony, csParam + 6, 4);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: control: format/copy (drive=%u, format=%u, data=0x%08lx, tags=0x%08lx)\n",
		vref, format, data, tags
	);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	dsk = dsks_get_disk (sony->dsks, vref);

	if (dsk == NULL) {
		mac_sony_return (sony, noDriveErr, 0);
		return;
	}

	if (dsk->readonly) {
		mac_sony_return (sony, wPrErr, 0);
		return;
	}

	if ((format > 0) && (format <= sony->format_cnt)) {
		blk = sony->format_list[2 * (format - 1)];
	}
	else {
		mac_sony_return (sony, paramErr, 0);
		return;
	}

	type = mac_sony_get_disk_type (blk);

	if (mac_sony_format (dsk, blk)) {
		mac_sony_return (sony, paramErr, 0);
		return;
	}

	data &= 0x00ffffff;
	tags &= 0x00ffffff;

	memset (tag, 0, 12);

	for (idx = 0; idx < blk; idx++) {
		for (i = 0; i < 512; i++) {
			buf[i] = mem_get_uint8 (sony->mem, data + i);
		}

		if ((type != 2) && (type != 3)) {
			for (i = 0; i < 12; i++) {
				tag[i] = mem_get_uint8 (sony->mem, tags + i);
			}
		}

		if (mac_sony_write_block (dsk, buf, tag, idx)) {
			mac_sony_return (sony, 0xffff, 0);
			return;
		}

		data += 512;
		tags += 12;
	}

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_control (mac_sony_t *sony)
{
	unsigned cscode;

	cscode = mac_sony_get_pblk (sony, csCode, 2);

	switch (cscode) {
	case 1: /* kill io */
		mac_sony_return (sony, abortErr, 1);
		break;

	case 5: /* verify disk */
		mac_sony_ctl_verify (sony);
		break;

	case 7: /* eject disk */
		mac_sony_ctl_eject (sony);
		return;

	case 8: /* set tag buffer */
		mac_sony_ctl_set_tag_buf (sony);
		break;

	case 9: /* track cache control */
		mac_sony_return (sony, 0xffc8, 0);
		break;

	case 6: /* format disk */
		mac_sony_ctl_format (sony);
		return;

	case 21: /* get drive icon */
		mac_sony_ctl_get_icon (sony, 0);
		return;

	case 22: /* get media icon */
		mac_sony_ctl_get_icon (sony, 1);
		return;

	case 23: /* get drive info */
		mac_sony_ctl_get_drive_info (sony);
		return;

	case 21315: /* FmtCopy */
		mac_sony_ctl_format_copy (sony);
		return;

	default:
		mac_log_deb ("sony: control: unknown (opcode=0x%04x)\n", cscode);
		mac_sony_return (sony, controlErr, 0);
		return;
	}
}


static
void mac_sony_status_format_list (mac_sony_t *sony)
{
	unsigned      i;
	unsigned long ptr;
	unsigned      vref, cnt;
	unsigned long blk;
	unsigned long *list;
	disk_t        *dsk;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);
	cnt = mac_sony_get_pblk (sony, csParam, 2);
	ptr = mac_sony_get_pblk (sony, csParam + 2, 4);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: status: get format list (drive=%u, maxfmt=%u)\n",
		vref, cnt
	);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	dsk = dsks_get_disk (sony->dsks, vref);

	if (dsk == NULL) {
		mac_sony_return (sony, noDriveErr, 0);
		return;
	}

	blk = dsk_get_block_cnt (dsk);

	list = sony->format_list;

	if (dsk_get_type (dsk) == PCE_DISK_FDC) {
		sony->format_cnt = 4;

		list[0] = 800;
		list[1] = 0x810a0050;
		list[2] = 1600;
		list[3] = 0x820a0050;
		list[4] = 1440;
		list[5] = 0x82090050;
		list[6] = 2880;
		list[7] = 0x92120050;

		switch (mac_sony_get_disk_type (blk)) {
		case 0:
			list[1] |= 0x40000000;
			break;

		case 1:
			list[3] |= 0x40000000;
			break;

		case 2:
			list[5] |= 0x40000000;
			break;

		case 3:
		default:
			if (sony->format_hd_as_dd) {
				list[7] |= 0x40000000;
			}
			else {
				sony->format_cnt = 1;
				list[0] = 2880;
				list[1] = 0xd2120050;
			}
			break;
		}
	}
	else {
		sony->format_cnt = 1;
		list[0] = blk;
		list[1] = 0;
	}

	if (cnt > sony->format_cnt) {
		cnt = sony->format_cnt;
	}

	for (i = 0; i < cnt; i++) {
		mem_set_uint32_be (sony->mem, ptr + 8 * i + 0, list[2 * i + 0]);
		mem_set_uint32_be (sony->mem, ptr + 8 * i + 4, list[2 * i + 1]);
	}

	mac_sony_set_pblk (sony, csParam, 2, cnt);

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_status_drive_status (mac_sony_t *sony)
{
	unsigned      i;
	unsigned      vref, val;
	unsigned long src;
	disk_t        *dsk;

	vref = mac_sony_get_pblk (sony, ioVRefNum, 2);

#ifdef DEBUG_SONY
	mac_log_deb ("sony: status: get drive status (drive=%u)\n", vref);
#endif

	if ((vref < 1) || (vref > SONY_DRIVES)) {
		mac_sony_return (sony, nsDrvErr, 0);
		return;
	}

	dsk = dsks_get_disk (sony->dsks, vref);

	if (dsk == NULL) {
		mac_sony_return (sony, noDriveErr, 0);
		return;
	}

	src = mac_sony_get_vars (sony, vref);

	for (i = 0; i < 11; i++) {
		val = mem_get_uint16_be (sony->mem, src + 2 * i);
		mac_sony_set_pblk (sony, csParam + 2 * i, 2, val);
	}

	mac_sony_return (sony, noErr, 0);
}

static
void mac_sony_status (mac_sony_t *sony)
{
	unsigned cscode;

	cscode = mac_sony_get_pblk (sony, csCode, 2);

	switch (cscode) {
	case 6: /* return format list */
		mac_sony_status_format_list (sony);
		break;

	case 8: /* drive status */
		mac_sony_status_drive_status (sony);
		break;

	default:
		mac_sony_return (sony, statusErr, 0);
		mac_log_deb ("sony: status: unknown (cs=0x%04x)\n", cscode);
		break;
	}
}

int mac_sony_hook (mac_sony_t *sony, unsigned val)
{
	switch (val) {
	case MAC_HOOK_SONY_OPEN:
		mac_sony_open (sony);
		return (0);

	case MAC_HOOK_SONY_PRIME:
		mac_sony_prime (sony);
		return (0);

	case MAC_HOOK_SONY_CTRL:
		mac_sony_control (sony);
		return (0);

	case MAC_HOOK_SONY_STATUS:
		mac_sony_status (sony);
		return (0);

	case MAC_HOOK_SONY_CLOSE:
		return (0);
	}

	return (1);
}

void mac_sony_reset (mac_sony_t *sony)
{
	unsigned i;

	mac_sony_unpatch_rom (sony);

	sony->open = 0;

	for (i = 0; i < SONY_DRIVES; i++) {
		sony->delay_cnt[i] = sony->delay_val[i];
	}
}
