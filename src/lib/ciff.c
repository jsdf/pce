/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/ciff.c                                               *
 * Created:     2022-02-12 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022 Hampa Hug <hampa@hampa.ch>                          *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include <config.h>

#include <lib/ciff.h>
#include <lib/endian.h>

#include <stdint.h>
#include <stdio.h>


#define CIFF_CRC_POLY 0x1edc6f41


static char     crc_tab_ok = 0;
static uint32_t crc_tab[256];


static
void ciff_init_crc_tab (void)
{
	unsigned i, j;
	uint32_t val;

	if (crc_tab_ok) {
		return;
	}

	for (i = 0; i < 256; i++) {
		val = (uint32_t) i << 24;

		for (j = 0; j < 8; j++) {
			if (val & 0x80000000) {
				val = (val << 1) ^ CIFF_CRC_POLY;
			}
			else {
				val = val << 1;
			}
		}

		crc_tab[i] = val;
	}

	crc_tab_ok = 1;
}

static
void ciff_crc (ciff_t *ciff, const void *buf, uint32_t cnt)
{
	uint32_t      crc;
	const uint8_t *src;

	if (crc_tab_ok == 0) {
		ciff_init_crc_tab();
	}

	src = buf;
	crc = ciff->crc;

	while (cnt-- > 0) {
		crc = (crc << 8) ^ crc_tab[((crc >> 24) ^ *(src++)) & 0xff];
	}

	ciff->crc = crc & 0xffffffff;
}

void ciff_init (ciff_t *ciff, FILE *fp, int use_crc)
{
	ciff->fp = fp;

	ciff->crc = 0;

	ciff->ckid = 0;
	ciff->cksize = 0;

	ciff->size = 0;

	ciff->in_chunk = 0;
	ciff->use_crc = (use_crc != 0);
	ciff->crc_error = 0;
}

void ciff_free (ciff_t *ciff)
{
}

int ciff_read (ciff_t *ciff, void *buf, uint32_t cnt)
{
	uint32_t ret;

	if (cnt > ciff->size) {
		return (1);
	}

	ret = fread (buf, 1, cnt, ciff->fp);

	ciff->size -= ret;

	if (ciff->use_crc) {
		ciff_crc (ciff, buf, ret);
	}

	return (ret != cnt);
}

int ciff_read_id (ciff_t *ciff)
{
	unsigned char buf[8];

	if (ciff->in_chunk) {
		if (ciff_read_crc (ciff)) {
			return (1);
		}
	}

	ciff->crc = 0;
	ciff->size = 8;

	if (ciff_read (ciff, buf, 8)) {
		return (1);
	}

	ciff->ckid = get_uint32_be (buf, 0);
	ciff->cksize = get_uint32_be (buf, 4);

	ciff->size = ciff->cksize;

	ciff->in_chunk = 1;

	return (0);
}

int ciff_read_crc (ciff_t *ciff)
{
	unsigned      cnt;
	uint32_t      crc;
	unsigned char buf[256];

	while (ciff->size > 0) {
		cnt = (ciff->size < 256) ? ciff->size : 256;

		if (ciff_read (ciff, buf, cnt)) {
			return (1);
		}
	}

	if (ciff->use_crc) {
		crc = ciff->crc;
		ciff->size = 4;

		if (ciff_read (ciff, buf, 4)) {
			return (1);
		}

		if (get_uint32_be (buf, 0) != crc) {
			ciff->crc_error = 1;
			return (1);
		}
	}

	ciff->in_chunk = 0;

	return (0);
}

int ciff_read_chunk (ciff_t *ciff, void *buf, uint32_t size)
{
	if (ciff_read (ciff, buf, size)) {
		return (1);
	}

	if (ciff_read_crc (ciff)) {
		return (1);
	}

	return (0);
}

int ciff_write (ciff_t *ciff, const void *buf, uint32_t cnt)
{
	uint32_t ret;

	if (cnt > ciff->size) {
		return (1);
	}

	ret = fwrite (buf, 1, cnt, ciff->fp);

	ciff->size -= ret;

	if (ciff->use_crc) {
		ciff_crc (ciff, buf, cnt);
	}

	return (ret != cnt);
}

int ciff_write_id (ciff_t *ciff, uint32_t ckid, uint32_t size)
{
	unsigned char buf[8];

	if (ciff->in_chunk) {
		if (ciff_write_crc (ciff)) {
			return (1);
		}
	}

	ciff->ckid = ckid;
	ciff->cksize = size;

	set_uint32_be (buf, 0, ckid);
	set_uint32_be (buf, 4, size);

	ciff->crc = 0;
	ciff->size = 8;

	if (ciff_write (ciff, buf, 8)) {
		return (1);
	}

	ciff->size = size;

	ciff->in_chunk = 1;

	return (0);
}

int ciff_write_crc (ciff_t *ciff)
{
	unsigned char buf[4];

	if (ciff->size != 0) {
		return (1);
	}

	if (ciff->use_crc) {
		set_uint32_be (buf, 0, ciff->crc);

		ciff->size = 4;

		if (ciff_write (ciff, buf, 4)) {
			return (1);
		}
	}

	ciff->in_chunk = 0;

	return (0);
}

int ciff_write_chunk (ciff_t *ciff, uint32_t ckid, const void *buf, uint32_t size)
{
	if (ciff_write_id (ciff, ckid, size)) {
		return (1);
	}

	if (ciff_write (ciff, buf, size)) {
		return (1);
	}

	if (ciff_write_crc (ciff)) {
		return (1);
	}

	return (0);
}
