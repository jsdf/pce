/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/srec.c                                               *
 * Created:     2005-03-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include "srec.h"


typedef struct {
	unsigned char  type;
	unsigned char  cnt;
	unsigned long  addr;
	unsigned       asize;
	unsigned char  data[256];
	unsigned char  cksum;
} record_t;


static unsigned srec_asize[16] = {
	2, 2, 3, 4, 2, 2, 2, 4,
	3, 2, 2, 2, 2, 2, 2, 2
};


static
int srec_skip_line (FILE *fp)
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
int srec_get_hex4 (FILE *fp, unsigned char *val)
{
	int c;

	c = fgetc (fp);

	if ((c >= '0') && (c <= '9')) {
		*val = c - '0';
	}
	else if ((c >= 'A') && (c <= 'F')) {
		*val = c - 'A' + 10;
	}
	else if ((c >= 'a') && (c <= 'f')) {
		*val = c - 'a' + 10;
	}
	else {
		return (1);
	}

	return (0);
}

static
int srec_get_hex8 (FILE *fp, unsigned char *val)
{
	unsigned char v1, v2;

	if (srec_get_hex4 (fp, &v1) || srec_get_hex4 (fp, &v2)) {
		return (1);
	}

	*val = ((v1 & 0x0f) << 4) | (v2 & 0x0f);

	return (0);
}

static
void srec_set_hex4 (FILE *fp, unsigned val)
{
	val &= 0x0f;
	val += (val <= 9) ? '0' : ('A' - 10);

	fputc (val, fp);
}

static
void srec_set_hex8 (FILE *fp, unsigned val)
{
	srec_set_hex4 (fp, (val >> 4) & 0x0f);
	srec_set_hex4 (fp, val & 0x0f);
}

static
unsigned char srec_get_cksum (record_t *rec)
{
	unsigned      i;
	unsigned char ck;

	ck = (rec->cnt + rec->asize + 1) & 0xff;
	ck += rec->addr & 0xff;
	ck += (rec->addr >> 8) & 0xff;
	ck += (rec->addr >> 16) & 0xff;
	ck += (rec->addr >> 24) & 0xff;

	for (i = 0; i < rec->cnt; i++) {
		ck += rec->data[i];
	}

	ck = ~ck & 0xff;

	return (ck);
}

static
void srec_init_record (record_t *rec, unsigned type, unsigned cnt, unsigned long addr)
{
	rec->type = type;
	rec->cnt = cnt;
	rec->addr = addr;

	if (type < 16) {
		rec->asize = srec_asize[type];
	}
	else {
		rec->asize = 2;
	}
}

static
int srec_get_record (FILE *fp, record_t *rec)
{
	unsigned      i;
	int           c;
	unsigned char addr;
	unsigned char type;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			return (1);
		}

		if (c == 'S') {
			if (srec_get_hex4 (fp, &type) == 0) {
				break;
			}
		}

		if ((c != 0x0a) && (c != 0x0d)) {
			srec_skip_line (fp);
		}
	}

	rec->type = type;
	rec->asize = srec_asize[type];

	if (srec_get_hex8 (fp, &rec->cnt)) {
		return (1);
	}

	if (rec->cnt < (rec->asize + 1)) {
		return (1);
	}

	rec->cnt -= rec->asize + 1;
	rec->addr = 0;

	for (i = 0; i < rec->asize; i++) {
		if (srec_get_hex8 (fp, &addr)) {
			return (1);
		}
		rec->addr = (rec->addr << 8) | (addr & 0xff);
	}

	for (i = 0; i < rec->cnt; i++) {
		if (srec_get_hex8 (fp, &rec->data[i])) {
			return (1);
		}
	}

	if (srec_get_hex8 (fp, &rec->cksum)) {
		return (1);
	}

	srec_skip_line (fp);

	return (0);
}

static
void srec_set_record (FILE *fp, record_t *rec)
{
	unsigned i;

	rec->cksum = srec_get_cksum (rec);

	fputc ('S', fp);

	srec_set_hex4 (fp, rec->type);
	srec_set_hex8 (fp, rec->cnt + rec->asize + 1);

	for (i = 0; i < rec->asize; i++) {
		srec_set_hex8 (fp,
			(rec->addr >> (8 * (rec->asize - i - 1))) & 0xff
		);
	}

	for (i = 0; i < rec->cnt; i++) {
		srec_set_hex8 (fp, rec->data[i]);
	}

	srec_set_hex8 (fp, rec->cksum);

	fputs ("\n", fp);
}

static
void srec_set_hdr (FILE *fp, const char *name)
{
	unsigned cnt;
	record_t rec;

	cnt = strlen (name);

	if (cnt > 20) {
		cnt = 20;
	}

	srec_init_record (&rec, 0, cnt, 0);

	memcpy (rec.data, name, cnt);

	srec_set_record (fp, &rec);
}

static
void srec_set_end (FILE *fp, unsigned long saddr)
{
	record_t rec;

	if (saddr & 0xff000000) {
		srec_init_record (&rec, 7, 0, saddr);
	}
	else if (saddr & 0xffff0000) {
		srec_init_record (&rec, 8, 0, saddr);
	}
	else {
		srec_init_record (&rec, 9, 0, saddr);
	}

	srec_set_record (fp, &rec);
}

int srec_load_fp (FILE *fp, void *ext, srec_set_f set)
{
	unsigned i;
	record_t rec;

	while (1) {
		if (srec_get_record (fp, &rec)) {
			return (0);
		}

		if (rec.cksum != srec_get_cksum (&rec)) {
			return (1);
		}

		if (rec.type == 0) {
			/* header record */
			;
		}
		else if ((rec.type == 1) || (rec.type == 2) || (rec.type == 3)) {
			for (i = 0; i < rec.cnt; i++) {
				set (ext, rec.addr + i, rec.data[i]);
			}
		}
		else if (rec.type == 5) {
			/* record count record */
			;
		}
		else if ((rec.type == 7) || (rec.type == 8) || (rec.type == 9)) {
			/* end record */
			return (0);
		}
	}

	return (0);
}

int srec_load (const char *fname, void *ext, srec_set_f set)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");
	if (fp == NULL) {
		return (1);
	}

	r = srec_load_fp (fp, ext, set);

	fclose (fp);

	return (r);
}

int srec_save_start (FILE *fp, const char *name)
{
	srec_set_hdr (fp, name);

	return (0);
}

int srec_save (FILE *fp, unsigned long base, unsigned long size, void *ext, srec_get_f get)
{
	unsigned i;
	record_t rec;

	while (size > 0) {
		rec.cnt = (size < 16) ? size : 16;

		if (base & 0xff000000) {
			rec.type = 3;
			rec.asize = 4;
		}
		else if (base & 0xffff0000) {
			rec.type = 2;
			rec.asize = 3;
		}
		else {
			rec.type = 1;
			rec.asize = 2;
		}

		rec.addr = base & 0xffffffff;

		for (i = 0; i < rec.cnt; i++) {
			rec.data[i] = get (ext, rec.addr + i);
		}

		srec_set_record (fp, &rec);

		base += rec.cnt;
		size -= rec.cnt;
	}

	return (0);
}

int srec_save_done (FILE *fp)
{
	srec_set_end (fp, 0);

	return (0);
}
