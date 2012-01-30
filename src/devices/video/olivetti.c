/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/video/olivetti.c                                 *
 * Created:     2011-09-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011 Hampa Hug <hampa@hampa.ch>                          *
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
#include <devices/video/olivetti.h>


#define CGA_CRTC_INDEX    0
#define CGA_CRTC_DATA     1
#define CGA_MODE          4
#define CGA_CSEL          5
#define CGA_STATUS        6
#define CGA_PEN_RESET     7
#define CGA_PEN_SET       8
#define M24_SPECIAL       10

#define CGA_MODE_G320     0x02
#define CGA_MODE_G640     0x10

#define M24_SPECIAL_400   0x01
#define M24_SPECIAL_512   0x10

#define M24_UPDATE_DIRTY 1


/*
 * Update graphics 620 * 400 * 2
 */
static
void m24_update_640_400 (cga_t *m24)
{
	unsigned            i, x, y, w, h;
	unsigned            val, idx;
	unsigned char       *dst;
	const unsigned char *col, *src;

	if (cga_set_buf_size (m24, 16 * m24->w, 4 * m24->h)) {
		return;
	}

	dst = m24->buf;

	w = 2 * m24->w;
	h = 4 * m24->h;

	for (y = 0; y < h; y++) {
		src = m24->mem + (y & 3) * 8192 + (y / 4) * w;

		for (x = 0; x < w; x++) {
			val = src[x];

			for (i = 0; i < 8; i++) {
				idx = (val & 0x80) ? m24->pal[0] : 0;
				col = cga_rgb[idx];

				dst[0] = col[0];
				dst[1] = col[1];
				dst[2] = col[2];

				val <<= 1;

				dst += 3;
			}
		}
	}
}

static
void m24_update (cga_t *m24)
{
	unsigned char mode, spec;

	mode = m24->reg[CGA_MODE];
	spec = m24->reg[M24_SPECIAL];

	if (mode & CGA_MODE_G320) {
		if (mode & CGA_MODE_G640) {
			if (spec & M24_SPECIAL_400) {
				m24_update_640_400 (m24);
				return;
			}
		}
	}

	cga_update (m24);
}


static
unsigned char m24_get_special (cga_t *m24)
{
	return (m24->reg[M24_SPECIAL]);
}

static
void m24_set_special (cga_t *m24, unsigned char val)
{
	m24->reg[M24_SPECIAL] = val;

	m24->update_state |= M24_UPDATE_DIRTY;
}

static
unsigned char m24_reg_get_uint8 (cga_t *m24, unsigned long addr)
{
	if (addr == M24_SPECIAL) {
		return (m24_get_special (m24));
	}
	else {
		return (cga_reg_get_uint8 (m24, addr));
	}
}

unsigned short m24_reg_get_uint16 (cga_t *m24, unsigned long addr)
{
	unsigned short ret;

	ret = m24_reg_get_uint8 (m24, addr);

	if ((addr + 1) < m24->regblk->size) {
		ret |= m24_reg_get_uint8 (m24, addr + 1) << 8;
	}

	return (ret);
}

void m24_reg_set_uint8 (cga_t *m24, unsigned long addr, unsigned char val)
{
	if (addr == M24_SPECIAL) {
		m24_set_special (m24, val);
	}
	else {
		cga_reg_set_uint8 (m24, addr, val);
	}
}

void m24_reg_set_uint16 (cga_t *m24, unsigned long addr, unsigned short val)
{
	m24_reg_set_uint8 (m24, addr, val & 0xff);

	if ((addr + 1) < m24->regblk->size) {
		m24_reg_set_uint8 (m24, addr + 1, val >> 8);
	}
}


static
void m24_print_info (cga_t *m24, FILE *fp)
{
	unsigned i;

	fprintf (fp, "DEV: Olivetti M24\n");

	fprintf (fp, "CGA: OFS=%04X  POS=%04X  BG=%02X  PAL=%u\n",
		cga_get_start (m24),
		cga_get_cursor (m24),
		m24->reg[CGA_CSEL] & 0x0f,
		(m24->reg[CGA_CSEL] >> 5) & 1
	);

	fprintf (fp,
		"REG: MODE=%02X  CSEL=%02X  STATUS=%02X  SPEC=%02X"
		"  PAL=[%02X %02X %02X %02X]\n",
		m24->reg[CGA_MODE],
		m24->reg[CGA_CSEL],
		m24->reg[CGA_STATUS],
		m24->reg[M24_SPECIAL],
		m24->pal[0], m24->pal[1], m24->pal[2], m24->pal[3]
	);

	fprintf (fp, "CRTC=[%02X", m24->reg_crt[0]);

	for (i = 1; i < 18; i++) {
		fputs ((i & 7) ? " " : "-", fp);
		fprintf (fp, "%02X", m24->reg_crt[i]);
	}

	fputs ("]\n", fp);

	fflush (fp);
}


void m24_init (cga_t *m24, unsigned long io, unsigned long addr, unsigned long size)
{
	if (size < 32768) {
		size = 32768;
	}

	cga_init (m24, io, addr, size);

	m24->video.print_info = (void *) m24_print_info;

	m24->update = m24_update;

	m24->regblk->set_uint8 = (void *) m24_reg_set_uint8;
	m24->regblk->set_uint16 = (void *) m24_reg_set_uint16;
	m24->regblk->get_uint8 = (void *) m24_reg_get_uint8;
	m24->regblk->get_uint16 = (void *) m24_reg_get_uint16;
}

void m24_free (cga_t *m24)
{
	cga_free (m24);
}

cga_t *m24_new (unsigned long io, unsigned long addr, unsigned long size)
{
	cga_t *m24;

	m24 = malloc (sizeof (cga_t));

	if (m24 == NULL) {
		return (NULL);
	}

	m24_init (m24, io, addr, size);

	return (m24);
}

video_t *m24_new_ini (ini_sct_t *sct)
{
	unsigned long io, addr, size;
	unsigned      blink;
	cga_t         *m24;

	ini_get_uint32 (sct, "io", &io, 0x3d4);
	ini_get_uint32 (sct, "address", &addr, 0xb8000);
	ini_get_uint32 (sct, "size", &size, 32768);
	ini_get_uint16 (sct, "blink", &blink, 0);

	if (size < 32768) {
		size = 32768;
	}

	pce_log_tag (MSG_INF,
		"VIDEO:", "Olivetti io=0x%04lx addr=0x%05lx size=0x%05lx\n",
		io, addr, size
	);

	m24 = m24_new (io, addr, size);

	if (m24 == NULL) {
		return (NULL);
	}

	cga_set_blink_rate (m24, blink);

	return (&m24->video);
}
