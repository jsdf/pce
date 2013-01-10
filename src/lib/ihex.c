/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/ihex.c                                               *
 * Created:     2004-06-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>

#include "ihex.h"


typedef struct {
	unsigned char  type;
	unsigned char  cnt;
	unsigned short addr;
	unsigned char  data[256];
	unsigned char  cksum;
} record_t;


#define IHEX_REC_DATA 0x00
#define IHEX_REC_EOFR 0x01
#define IHEX_REC_ESAR 0x02
#define IHEX_REC_SSAR 0x03
#define IHEX_REC_ELAR 0x04
#define IHEX_REC_SLAR 0x05


static
int ihex_skip_line (FILE *fp)
{
	int c;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			return (1);
		}

		if ((c == 0x0a) || (c == 0x0d)) {
			return (0);
		}
	}

	return (0);
}

static
int ihex_get_hex8 (FILE *fp, unsigned char *val)
{
	unsigned i;
	int      c;

	*val = 0;

	for (i = 0; i < 2; i++) {
		c = fgetc (fp);

		if ((c >= '0') && (c <= '9')) {
			*val = (*val << 4) | (c - '0');
		}
		else if ((c >= 'A') && (c <= 'F')) {
			*val = (*val << 4) | (c - 'A' + 10);
		}
		else if ((c >= 'a') && (c <= 'f')) {
			*val = (*val << 4) | (c - 'a' + 10);
		}
		else {
			return (1);
		}
	}

	return (0);
}

static
unsigned char ihex_get_cksum (record_t *rec)
{
	unsigned      i;
	unsigned char ck;

	ck = rec->cnt & 0xff;
	ck += rec->addr & 0xff;
	ck += (rec->addr >> 8) & 0xff;
	ck += rec->type & 0xff;

	for (i = 0; i < rec->cnt; i++) {
		ck += rec->data[i];
	}

	ck = (~ck + 1) & 0xff;

	return (ck);
}

static
int ihex_get_record (FILE *fp, record_t *rec)
{
	unsigned      i;
	int           c;
	unsigned char a1, a2;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			return (1);
		}

		if (c == ':') {
			break;
		}

		if ((c != 0x0a) && (c != 0x0d)) {
			ihex_skip_line (fp);
		}
	}

	if (ihex_get_hex8 (fp, &rec->cnt)) {
		return (1);
	}

	if (ihex_get_hex8 (fp, &a1) || ihex_get_hex8 (fp, &a2)) {
		return (1);
	}

	rec->addr = (a1 << 8) | a2;

	if (ihex_get_hex8 (fp, &rec->type)) {
		return (1);
	}

	for (i = 0; i < rec->cnt; i++) {
		if (ihex_get_hex8 (fp, &rec->data[i])) {
			return (1);
		}
	}

	if (ihex_get_hex8 (fp, &rec->cksum)) {
		return (1);
	}

	ihex_skip_line (fp);

	return (0);
}


static
void ihex_set_hex8 (FILE *fp, unsigned char c)
{
	int tmp;

	tmp = (c >> 4) & 0x0f;
	tmp += (tmp <= 9) ? '0' : ('A' - 10);
	fputc (tmp, fp);

	tmp = c & 0x0f;
	tmp += (tmp <= 9) ? '0' : ('A' - 10);
	fputc (tmp, fp);
}

static
void ihex_set_record (FILE *fp, record_t *rec)
{
	unsigned i;

	rec->cksum = ihex_get_cksum (rec);

	fprintf (fp, ":%02X%04X%02X",
		(unsigned) rec->cnt, (unsigned) rec->addr & 0xffffU, (unsigned) rec->type
	);

	for (i = 0; i < rec->cnt; i++) {
		ihex_set_hex8 (fp, rec->data[i]);
	}

	ihex_set_hex8 (fp, rec->cksum);

	fputs ("\n", fp);
}

static
void ihex_set_esar (FILE *fp, unsigned seg)
{
	record_t rec;

	rec.type = IHEX_REC_ESAR;
	rec.cnt = 2;
	rec.addr = 0;
	rec.data[0] = (seg >> 8) & 0xff;
	rec.data[1] = seg & 0xff;

	ihex_set_record (fp, &rec);
}

static
void ihex_set_ulba (FILE *fp, unsigned long addr)
{
	record_t rec;

	rec.type = 0x04;
	rec.cnt = 2;
	rec.addr = 0;
	rec.data[0] = (addr >> 24) & 0xff;
	rec.data[1] = (addr >> 16) & 0xff;

	ihex_set_record (fp, &rec);
}

static
void ihex_set_end (FILE *fp)
{
	record_t rec;

	rec.type = 0x01;
	rec.cnt = 0;
	rec.addr = 0;

	ihex_set_record (fp, &rec);
}

int ihex_load_fp (FILE *fp, void *ext, ihex_set_f set)
{
	unsigned      i;
	unsigned      mode;
	unsigned long addr, ulba;
	record_t      rec;

	mode = 0;
	ulba = 0;

	while (1) {
		if (ihex_get_record (fp, &rec)) {
			break;
		}

		if (rec.cksum != ihex_get_cksum (&rec)) {
			return (1);
		}

		if (rec.type == IHEX_REC_EOFR) {
			return (0);
		}
		else if (rec.type == IHEX_REC_ELAR) {
			if (rec.cnt == 2) {
				mode = 0;
				ulba = (rec.data[0] << 8) | rec.data[1];
				ulba = ulba << 16;
			}
			else {
				return (1);
			}
		}
		else if (rec.type == IHEX_REC_ESAR) {
			if (rec.cnt == 2) {
				mode = 1;
				ulba = (rec.data[0] << 8) | rec.data[1];
				ulba = ulba << 4;
			}
			else {
				return (1);
			}
		}
		else if (rec.type == IHEX_REC_DATA) {
			addr = ulba + (rec.addr & 0xffffU);

			for (i = 0; i < rec.cnt; i++) {
				if (mode == 0) {
					addr = ulba + rec.addr + i;
				}
				else {
					addr = ulba + ((rec.addr + i) & 0xffffU);
				}

				set (ext, addr, rec.data[i]);
			}
		}
	}

	return (0);
}

int ihex_load (const char *fname, void *ext, ihex_set_f set)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");
	if (fp == NULL) {
		return (1);
	}

	r = ihex_load_fp (fp, ext, set);

	fclose (fp);

	return (r);
}


int ihex_save (FILE *fp, unsigned seg, unsigned ofs, unsigned long size, void *ext, ihex_get_f get)
{
	unsigned       i;
	unsigned       cnt;
	unsigned short oldseg;
	record_t       rec;

	oldseg = 0;

	while (size > 0) {
		if (seg != oldseg) {
			ihex_set_esar (fp, seg);
			oldseg = seg;
		}

		cnt = (size < 16) ? size : 16;

		if (((ofs + cnt) & 0xffff) < ofs) {
			cnt = -ofs & 0xffff;
		}

		rec.type = 0x00;
		rec.addr = ofs;
		rec.cnt = cnt;

		for (i = 0; i < rec.cnt; i++) {
			rec.data[i] = get (ext, ((unsigned long) seg << 4) + ofs);

			ofs = (ofs + 1) & 0xffff;
		}

		ihex_set_record (fp, &rec);

		if (ofs == 0) {
			seg += 0x1000;
		}

		size -= rec.cnt;
	}

	return (0);
}

int ihex_save_linear (FILE *fp, unsigned long base, unsigned long size, void *ext, ihex_get_f get)
{
	unsigned      i;
	unsigned long ulba, addr;
	record_t      rec;

	ulba = 0;
	addr = base;

	while (size > 0) {
		rec.type = 0x00;
		rec.addr = addr & 0xffffU;
		rec.cnt = (size < 16) ? size : 16;

		for (i = 0; i < rec.cnt; i++) {
			rec.data[i] = get (ext, addr + i);
		}

		if ((ulba & 0xffff0000UL) != (addr & 0xffff0000UL)) {
			ihex_set_ulba (fp, addr);
			ulba = addr & 0xffff0000UL;
		}

		ihex_set_record (fp, &rec);

		addr += rec.cnt;
		size -= rec.cnt;
	}

	return (0);
}

int ihex_save_done (FILE *fp)
{
	ihex_set_end (fp);
	return (0);
}
