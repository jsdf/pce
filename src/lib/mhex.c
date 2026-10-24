/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/mhex.c                                               *
 * Created:     2020-05-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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

#include "mhex.h"


static
int mhex_skip_line (FILE *fp)
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
unsigned mhex_get_cksum (unsigned addr, const unsigned char *buf, unsigned cnt)
{
	unsigned val;

	val = (addr & 0xff) + ((addr >> 8) & 0xff);
	val += cnt & 0xff;

	while (cnt-- > 0) {
		val += *(buf++) & 0xff;
	}

	return (val & 0xffff);
}

static
int mhex_read_hex (FILE *fp, unsigned char *buf, unsigned cnt)
{
	int      c;
	unsigned i, j, s;

	cnt *= 2;

	for (i = 0; i < cnt; i++) {
		if ((c = fgetc (fp)) == EOF) {
			return (1);
		}

		j = i >> 1;
		s = (~i & 1) << 2;

		buf[j] &= 0xf0 << s;

		if ((c >= '0') && (c <= '9')) {
			buf[j] |= (c - '0') << s;
		}
		else if ((c >= 'A') && (c <= 'F')) {
			buf[j] |= (c - 'A' + 10) << s;
		}
		else if ((c >= 'a') && (c <= 'f')) {
			buf[j] |= (c - 'a' + 10) << s;
		}
		else {
			return (1);
		}
	}

	return (0);
}

static
int mhex_read_record (FILE *fp, unsigned *addr, unsigned char *buf, unsigned *cnt, int *check)
{
	int           c;
	unsigned      ck;
	unsigned char tmp[2];

	while ((c = fgetc (fp)) != ';') {
		if (c == EOF) {
			return (1);
		}

		if ((c != 0x0a) && (c != 0x0d)) {
			mhex_skip_line (fp);
		}
	}

	if (mhex_read_hex (fp, tmp, 1)) {
		return (1);
	}

	*cnt = tmp[0];

	if (mhex_read_hex (fp, tmp, 2)) {
		return (1);
	}

	*addr = ((tmp[0] & 0xff) << 8) | (tmp[1] & 0xff);

	if (mhex_read_hex (fp, buf, *cnt)) {
		return (1);
	}

	if (mhex_read_hex (fp, tmp, 2)) {
		return (1);
	}

	ck = ((tmp[0] & 0xff) << 8) | (tmp[1] & 0xff);

	*check = (mhex_get_cksum (*addr, buf, *cnt) != ck);

	mhex_skip_line (fp);

	return (0);
}

int mhex_load_fp (FILE *fp, void *ext, mhex_set_f set)
{
	int           check;
	unsigned      i, addr, cnt, rcnt;
	unsigned char buf[256];

	rcnt = 0;

	while (1) {
		if (mhex_read_record (fp, &addr, buf, &cnt, &check)) {
			break;
		}

		rcnt += 1;

		if (check) {
			return (1);
		}

		if (cnt == 0) {
			if ((addr + 1) != rcnt) {
				return (1);
			}

			rcnt = 0;
		}

		for (i = 0; i < cnt; i++) {
			set (ext, addr + i, buf[i]);
		}
	}

	if (rcnt > 0) {
		return (1);
	}

	return (0);
}

int mhex_load (const char *fname, void *ext, mhex_set_f set)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (1);
	}

	r = mhex_load_fp (fp, ext, set);

	fclose (fp);

	return (r);
}

static
void mhex_write_record (FILE *fp, unsigned addr, const unsigned char *buf, unsigned cnt)
{
	unsigned ck;

	ck = mhex_get_cksum (addr, buf, cnt);

	fprintf (fp, ";%02X%04X", cnt & 0xff, addr & 0xffff);

	while (cnt-- > 0) {
		fprintf (fp, "%02X", *(buf++));
	}

	fprintf (fp, "%04X\n", ck & 0xffff);
}

int mhex_save_fp (FILE *fp, unsigned long base, unsigned addr, unsigned size, void *ext, mhex_get_f get)
{
	unsigned      i, cnt, rcnt;
	unsigned char buf[16];

	rcnt = 0;

	while (size > 0) {
		cnt = (size < 16) ? size : 16;

		if (((addr + cnt) & 0xffff) < (addr & 0xffff)) {
			cnt = -addr & 0xffff;
		}

		for (i = 0; i < cnt; i++) {
			buf[i] = get (ext, base + ((addr + i) & 0xffff));
		}

		mhex_write_record (fp, addr, buf, cnt);

		rcnt += 1;
		addr += cnt;
		size -= cnt;
	}

	mhex_write_record (fp, rcnt, NULL, 0);

	return (0);
}

int mhex_save (const char *fname, unsigned long base, unsigned addr, unsigned size, void *ext, mhex_get_f set)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	r = mhex_save_fp (fp, base, addr, size, ext, set);

	fclose (fp);

	return (r);
}
