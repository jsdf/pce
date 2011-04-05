/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/wy700.c                                    *
 * Created:     2008-10-13 by John Elliott <jce@seasip.demon.co.uk>          *
 * Copyright:   (C) 2008-2011 Hampa Hug <hampa@hampa.ch>                     *
 *              (C) 2008 John Elliott <jce@seasip.demon.co.uk>               *
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

#include <lib/log.h>

#include <devices/video/cga.h>
#include <devices/video/wy700.h>


#define CGA_CRTC_INDEX    0
#define CGA_CRTC_DATA     1
#define CGA_MODE          4
#define CGA_CSEL          5
#define CGA_STATUS        6
#define CGA_PEN_RESET     7
#define CGA_PEN_SET       8
#define WY700_BANK_OFS    9
#define WY700_BANK_BASE   10
#define WY700_CONTROL     11

#define WY700_UPDATE_DIRTY 1


/*
 * Update graphics 640 * 400 * 2
 */
static
void wy700_update_640x400 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            val;
	unsigned            addr;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 640, 400)) {
		return;
	}

	dst = wy->buf;

	addr = 0;

	for (y = 0; y < 400; y++) {
		src = wy->mem + addr;

		addr += 80;

		for (x = 0; x < (640 / 8); x++) {
			val = src[x];

			for (i = 0; i < 8; i++) {
				if (val & 0x80) {
					dst[0] = 0xff;
					dst[1] = 0xff;
					dst[2] = 0xff;
				}
				else {
					dst[0] = 0x00;
					dst[1] = 0x00;
					dst[2] = 0x00;
				}

				val <<= 1;

				dst += 3;
			}
		}
	}
}

/*
 * Update graphics 1280 * 400 * 2
 */
static
void wy700_update_1280x400 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            val;
	unsigned            addr;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 1280, 400)) {
		return;
	}

	dst = wy->buf;

	addr = 0;

	for (y = 0; y < 400; y++) {
		src = wy->mem + addr;

		addr += 160;

		for (x = 0; x < (1280 / 8); x++) {
			val = src[x];

			for (i = 0; i < 8; i++) {
				if (val & 0x80) {
					dst[0] = 0xff;
					dst[1] = 0xff;
					dst[2] = 0xff;
				}
				else {
					dst[0] = 0x00;
					dst[1] = 0x00;
					dst[2] = 0x00;
				}

				val <<= 1;

				dst += 3;
			}
		}
	}
}

/*
 * Update graphics 1280 * 800 * 2
 */
static
void wy700_update_1280x800 (cga_t *wy)
{
	unsigned            i, x, y;
	unsigned            val;
	unsigned            addr;
	unsigned char       *dst;
	const unsigned char *src;

	if (cga_set_buf_size (wy, 1280, 800)) {
		return;
	}

	dst = wy->buf;

	addr = 0;

	for (y = 0; y < 800; y++) {
		src = wy->mem + addr;

		if (y & 1) {
			src += 0x10000;
			addr += 160;
		}

		for (x = 0; x < (1280 / 8); x++) {
			val = src[x];

			for (i = 0; i < 8; i++) {
				if (val & 0x80) {
					dst[0] = 0xff;
					dst[1] = 0xff;
					dst[2] = 0xff;
				}
				else {
					dst[0] = 0x00;
					dst[1] = 0x00;
					dst[2] = 0x00;
				}

				val <<= 1;

				dst += 3;
			}
		}
	}
}

static
void wy700_update (cga_t *wy)
{
	unsigned char ctl;

	ctl = wy->reg[WY700_CONTROL];

	if (ctl & 0x08) {
		switch (ctl & 0xf0) {
		case 0xc0:
			wy700_update_1280x800 (wy);
			break;

		case 0xa0:
			wy700_update_1280x400 (wy);
			break;

		case 0x80:
			wy700_update_640x400 (wy);
			break;

		default:
			fprintf (stderr, "WY700: unknown mode (%02X)\n", ctl);
			break;
		}

		return;
	}

	cga_update (wy);
}

static
unsigned char wy700_get_control (const cga_t *wy)
{
	return (wy->reg[WY700_CONTROL]);
}

static
void wy700_set_control (cga_t *wy, unsigned char val)
{
	if (wy->reg[WY700_CONTROL] == val) {
		return;
	}

	wy->reg[WY700_CONTROL] = val;

	wy->update_state |= WY700_UPDATE_DIRTY;
}

/*
 * Get a Wyse 700 register
 */
static
unsigned char wy700_reg_get_uint8 (cga_t *wy, unsigned long addr)
{
	switch (addr) {
	case WY700_CONTROL:
		return (wy700_get_control (wy));

	default:
		return (cga_reg_get_uint8 (wy, addr));
	}
}

static
unsigned short wy700_reg_get_uint16 (cga_t *wy, unsigned long addr)
{
	return (wy700_reg_get_uint8 (wy, addr));
}

/*
 * Set a Wyse 700 register
 */
static
void wy700_reg_set_uint8 (cga_t *wy, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case WY700_BANK_OFS:
		wy->reg[WY700_BANK_OFS] = val;
		break;

	case WY700_BANK_BASE:
		wy->reg[WY700_BANK_BASE] = val;
		break;

	case WY700_CONTROL:
		wy700_set_control (wy, val);
		break;

	case CGA_MODE:
		if (val == 0) {
			wy700_set_control (wy, 0);
		}
		cga_reg_set_uint8 (wy, addr, val);
		break;

	default:
		cga_reg_set_uint8 (wy, addr, val);
		break;
	}
}

void wy700_reg_set_uint16 (cga_t *wy, unsigned long addr, unsigned short val)
{
	wy700_reg_set_uint8 (wy, addr, val & 0xff);
}

static
unsigned char wy700_mem_get_uint8 (cga_t *wy, unsigned long addr)
{
	unsigned char ctl;

	ctl = wy->reg[WY700_CONTROL];

	if (addr <= 0xffff) {
		if (ctl & 1) {
			addr += 0x10000;
		}
	}
	else {
		addr &= 0x7fff;
	}

	return (wy->mem[addr]);
}

static
unsigned short wy700_mem_get_uint16 (cga_t *wy, unsigned long addr)
{
	unsigned short val;

	val = wy700_mem_get_uint8 (wy, addr);

	if ((addr + 1) < wy->memblk->size) {
		val |= (unsigned) wy700_mem_get_uint8 (wy, addr + 1) << 8;
	}

	return (val);
}

static
void wy700_mem_set_uint8 (cga_t *wy, unsigned long addr, unsigned char val)
{
	unsigned char ctl;

	ctl = wy->reg[WY700_CONTROL];

	if (addr <= 0xffff) {
		if (ctl & 2) {
			addr += 0x10000;
		}
	}
	else {
		addr &= 0x7fff;
	}

	if (wy->mem[addr] == val) {
		return;
	}

	wy->mem[addr] = val;

	wy->update_state |= WY700_UPDATE_DIRTY;
}

static
void wy700_mem_set_uint16 (cga_t *wy, unsigned long addr, unsigned short val)
{
	wy700_mem_set_uint8 (wy, addr, val & 0xff);

	if ((addr + 1) < wy->memblk->size) {
		wy700_mem_set_uint8 (wy, addr + 1, (val >> 8) & 0xff);
	}
}


static
void wy700_print_info (cga_t *wy, FILE *fp)
{
	unsigned i;

	fprintf (fp, "DEV: Wyse 700\n");

	fprintf (fp, "CGA: OFS=%04X  POS=%04X  BG=%02X  PAL=%u\n",
		cga_get_start (wy),
		cga_get_cursor (wy),
		wy->reg[CGA_CSEL] & 0x0f,
		(wy->reg[CGA_CSEL] >> 5) & 1
	);

	fprintf (fp,
		"REG: MODE=%02X  CSEL=%02X  STATUS=%02X"
		"  PAL=[%02X %02X %02X %02X]\n",
		wy->reg[CGA_MODE],
		wy->reg[CGA_CSEL],
		wy->reg[CGA_STATUS],
		wy->pal[0], wy->pal[1], wy->pal[2], wy->pal[3]
	);

	fprintf (fp,
		"REG: CTL=%02X  BANK OFS=%02X  BANK BAS=%02X\n",
		wy->reg[WY700_CONTROL],
		wy->reg[WY700_BANK_OFS],
		wy->reg[WY700_BANK_BASE]
	);

	fprintf (fp, "CRTC=[%02X", wy->reg_crt[0]);
	for (i = 1; i < 18; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", wy->reg_crt[i]);
	}
	fputs ("]\n", fp);

	fflush (fp);
}


void wy700_init (cga_t *wy, unsigned long io, unsigned long addr, unsigned long size)
{
	if (size < 131072) {
		size = 131072;
	}

	cga_init (wy, io, addr, size);

	wy->video.print_info = (void *) wy700_print_info;

	wy->update = wy700_update;

	wy->regblk->set_uint8 = (void *) wy700_reg_set_uint8;
	wy->regblk->set_uint16 = (void *) wy700_reg_set_uint16;
	wy->regblk->get_uint8 = (void *) wy700_reg_get_uint8;
	wy->regblk->get_uint16 = (void *) wy700_reg_get_uint16;

	wy->memblk->set_uint8 = (void *) wy700_mem_set_uint8;
	wy->memblk->set_uint16 = (void *) wy700_mem_set_uint16;
	wy->memblk->get_uint8 = (void *) wy700_mem_get_uint8;
	wy->memblk->get_uint16 = (void *) wy700_mem_get_uint16;
}

void wy700_free (cga_t *wy)
{
	cga_free (wy);
}

cga_t *wy700_new (unsigned long io, unsigned long addr, unsigned long size)
{
	cga_t *wy;

	wy = malloc (sizeof (cga_t));
	if (wy == NULL) {
		return (NULL);
	}

	wy700_init (wy, io, addr, size);

	return (wy);
}

video_t *wy700_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned      blink;
	cga_t         *wy;

	ini_get_uint32 (sct, "io", &io, 0x3d4);
	ini_get_uint32 (sct, "address", &addr, 0xa0000);
	ini_get_uint32 (sct, "size", &size, 131072);
	ini_get_uint16 (sct, "blink", &blink, 0);

	if (size < 131072) {
		size = 131072;
	}

	pce_log_tag (MSG_INF,
		"VIDEO:", "WY-700 io=0x%04lx addr=0x%05lx size=0x%05lx\n",
		io, addr, size
	);

	wy = wy700_new (io, addr, size);
	if (wy == NULL) {
		return (NULL);
	}

	cga_set_blink_rate (wy, blink);

	return (&wy->video);
}
