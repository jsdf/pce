/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/thex.c                                               *
 * Created:     2020-11-27 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2021 Hampa Hug <hampa@hampa.ch>                     *
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

#include "thex.h"


typedef struct {
	unsigned char  type;
	unsigned char  size;
	unsigned char  data[256];
	unsigned short cksum;
} record_t;


static
int thex_skip_line (FILE *fp)
{
	int c;

	while ((c = fgetc (fp)) != EOF) {
		if ((c == 0x0a) || (c == 0x0d)) {
			return (0);
		}
	}

	return (1);
}

static
int thex_get_uint8 (FILE *fp, unsigned char *val)
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
unsigned thex_get_cksum (record_t *rec)
{
	unsigned i, c1, c2;

	c1 = 0;
	c2 = c1;
	c1 += rec->type;
	c2 += c1;
	c1 += rec->size;
	c2 += c1;

	for (i = 0; i < rec->size; i++) {
		c1 += rec->data[i];
		c2 += c1;
	}

	c1 += c2;

	return (((c2 & 0xff) << 8) | (c1 & 0xff));
}

static
int thex_get_record (FILE *fp, record_t *rec)
{
	int           c;
	unsigned      i;
	unsigned char buf[2];

	while (1) {
		if ((c = fgetc (fp)) == EOF) {
			return (1);
		}

		if (c == 'T') {
			break;
		}

		if ((c != 0x0a) && (c != 0x0d)) {
			thex_skip_line (fp);
		}
	}

	if (thex_get_uint8 (fp, &rec->type)) {
		return (1);
	}

	if (thex_get_uint8 (fp, &rec->size)) {
		return (1);
	}

	for (i = 0; i < rec->size; i++) {
		if (thex_get_uint8 (fp, rec->data + i)) {
			return (1);
		}
	}

	for (i = 0; i < 2; i++) {
		if (thex_get_uint8 (fp, buf + i)) {
			return (1);
		}
	}

	rec->cksum = (buf[0] << 8) | buf[1];

	thex_skip_line (fp);

	return (0);
}

int thex_load_fp (FILE *fp, void *ext, thex_set_f set)
{
	unsigned      i;
	unsigned long seg, ofs, seg2, ofs2;
	record_t      rec;

	seg = 0;
	ofs = 0;

	while (1) {
		if (thex_get_record (fp, &rec)) {
			break;
		}

		if (rec.cksum != thex_get_cksum (&rec)) {
			return (1);
		}

		switch (rec.type) {
		case 0xff: /* description */
			break;

		case 0xfe: /* end of file */
			return (0);

		case 0x00: /* data */
			for (i = 0; i < rec.size; i++) {
				set (ext, seg + ofs + i, rec.data[i]);
			}
			ofs += rec.size;
			break;

		case 0x01: /* set start address */
			break;

		case 0x02:
		case 0x03:
			if (rec.size < 2) {
				return (1);
			}

			seg2 = 0;
			ofs2 = (rec.data[0] << 8) | rec.data[1];
			ofs &= 0xffff;

			if (rec.type == 0x03) {
				if ((seg != seg2) || (ofs != ofs2)) {
					return (1);
				}
			}

			seg = seg2;
			ofs = ofs2;
			break;

		case 0x04:
		case 0x05:
			if (rec.size < 4) {
				return (1);
			}

			seg2 = 0;
			ofs2 = (rec.data[0] << 8) | rec.data[1];
			ofs2 = (ofs2 << 8) | rec.data[2];
			ofs2 = (ofs2 << 8) | rec.data[3];

			if (rec.type == 0x05) {
				if ((seg != seg2) || (ofs != ofs2)) {
					return (1);
				}
			}

			seg = seg2;
			ofs = ofs2;
			break;

		case 0x06:
		case 0x07:
			if (rec.size < 4) {
				return (1);
			}

			seg2 = (rec.data[0] << 8) | rec.data[1];
			ofs2 = (rec.data[2] << 8) | rec.data[3];
			seg2 <<= 4;
			ofs &= 0xffff;

			if (rec.type == 0x07) {
				if ((seg != seg2) || (ofs != ofs2)) {
					return (1);
				}
			}

			seg = seg2;
			ofs = ofs2;
			break;

		default:
			return (1);
		}
	}

	return (0);
}

int thex_load (const char *fname, void *ext, thex_set_f set)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (1);
	}

	r = thex_load_fp (fp, ext, set);

	fclose (fp);

	return (r);
}


static
void thex_put_uint8 (FILE *fp, unsigned char c)
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
int thex_put_record (FILE *fp, record_t *rec)
{
	unsigned i;

	rec->cksum = thex_get_cksum (rec);

	if (fputc ('T', fp) == EOF) {
		return (1);
	}

	thex_put_uint8 (fp, rec->type);
	thex_put_uint8 (fp, rec->size);

	for (i = 0; i < rec->size; i++) {
		thex_put_uint8 (fp, rec->data[i]);
	}

	thex_put_uint8 (fp, (rec->cksum >> 8) & 0xff);
	thex_put_uint8 (fp, rec->cksum & 0xff);

	if (fputc ('\n', fp) == EOF) {
		return (1);
	}

	return (0);
}

static
int thex_put_empty (FILE *fp, unsigned type)
{
	record_t rec;

	rec.type = type;
	rec.size = 0;

	return (thex_put_record (fp, &rec));
}

static
int thex_put_addr_seg (FILE *fp, unsigned seg, unsigned ofs, int check)
{
	record_t rec;

	rec.type = 6 + (check != 0);
	rec.size = 4;

	rec.data[0] = (seg >> 8) & 0xff;
	rec.data[1] = seg & 0xff;
	rec.data[2] = (ofs >> 8) & 0xff;
	rec.data[3] = ofs & 0xff;

	return (thex_put_record (fp, &rec));
}

static
int thex_put_addr (FILE *fp, unsigned long addr, int check)
{
	unsigned i;
	record_t rec;

	rec.type = (check != 0);

	if (addr > 65535) {
		rec.type += 4;
		rec.size = 4;
	}
	else {
		rec.type += 2;
		rec.size = 2;
		addr <<= 16;
	}

	for (i = 0; i < rec.size; i++) {
		rec.data[i] = (addr >> 24) & 0xff;
		addr <<= 8;
	}

	return (thex_put_record (fp, &rec));
}

int thex_save_start (FILE *fp)
{
	if (thex_put_empty (fp, 0xff)) {
		return (1);
	}

	return (0);
}

int thex_save_seg (FILE *fp, unsigned seg, unsigned ofs, unsigned long size, void *ext, thex_get_f get)
{
	unsigned      i;
	unsigned long addr;
	record_t      rec;

	addr = ofs;

	if (thex_put_addr_seg (fp, seg, addr, 0)) {
		return (1);
	}

	while (size > 0) {
		rec.type = 0x00;
		rec.size = (size < 16) ? size : 16;

		for (i = 0; i < rec.size; i++) {
			rec.data[i] = get (ext, ((unsigned long) seg << 4) + addr + i);
		}

		if (thex_put_record (fp, &rec)) {
			return (1);
		}

		addr += rec.size;
		size -= rec.size;
	}

	if (thex_put_addr_seg (fp, seg, addr, 1)) {
		return (1);
	}

	return (0);
}

int thex_save (FILE *fp, unsigned long addr, unsigned long size, void *ext, thex_get_f get)
{
	unsigned i;
	record_t rec;

	if (thex_put_addr (fp, addr, 0)) {
		return (1);
	}

	while (size > 0) {
		rec.type = 0x00;
		rec.size = (size < 16) ? size : 16;

		for (i = 0; i < rec.size; i++) {
			rec.data[i] = get (ext, addr + i);
		}

		if (thex_put_record (fp, &rec)) {
			return (1);
		}

		addr += rec.size;
		size -= rec.size;
	}

	if (thex_put_addr (fp, addr, 1)) {
		return (1);
	}

	return (0);
}

int thex_save_done (FILE *fp)
{
	if (thex_put_empty (fp, 0xfe)) {
		return (1);
	}

	return (0);
}
