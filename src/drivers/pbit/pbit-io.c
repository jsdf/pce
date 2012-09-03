/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pbit/pbit-io.c                                   *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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

#include "pbit-io.h"
#include "pbit-io-pbit.h"
#include "pbit-io-tc.h"


unsigned pbit_get_uint16_be (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);

	return (val);
}

unsigned pbit_get_uint16_le (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[1] & 0xff;
	val = (val << 8) | (tmp[0] & 0xff);

	return (val);
}

unsigned long pbit_get_uint32_be (const void *buf, unsigned idx)
{
	unsigned long       val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);
	val = (val << 8) | (tmp[2] & 0xff);
	val = (val << 8) | (tmp[3] & 0xff);

	return (val);
}

unsigned long pbit_get_uint32_le (const void *buf, unsigned idx)
{
	unsigned long       val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[3] & 0xff;
	val = (val << 8) | (tmp[2] & 0xff);
	val = (val << 8) | (tmp[1] & 0xff);
	val = (val << 8) | (tmp[0] & 0xff);

	return (val);
}

void pbit_set_uint16_be (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

void pbit_set_uint16_le (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}

void pbit_set_uint32_be (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val & 0xff;
}

void pbit_set_uint32_le (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
	tmp[2] = (val >> 16) & 0xff;
	tmp[3] = (val >> 24) & 0xff;
}


int pbit_read (FILE *fp, void *buf, unsigned long cnt)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pbit_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pbit_write (FILE *fp, const void *buf, unsigned long cnt)
{
	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pbit_skip (FILE *fp, unsigned long cnt)
{
	unsigned long n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pbit_read (fp, buf, n)) {
			return (1);
		}

		cnt -= n;
	}

	return (0);
}


static
unsigned pbit_get_type (unsigned type, const char *fname)
{
	unsigned   i;
	const char *ext;

	if (type != PBIT_FORMAT_NONE) {
		return (type);
	}

	ext = "";

	i = 0;
	while (fname[i] != 0) {
		if (fname[i] == '.') {
			ext = fname + i;
		}

		i += 1;
	}

	if (strcasecmp (ext, ".pbit") == 0) {
		return (PBIT_FORMAT_PBIT);
	}
	else if (strcasecmp (ext, ".tc") == 0) {
		return (PBIT_FORMAT_TC);
	}

	return (PBIT_FORMAT_PBIT);
}


pbit_img_t *pbit_img_load_fp (FILE *fp, unsigned type)
{
	pbit_img_t *img;

	img = NULL;

	switch (type) {
	case PBIT_FORMAT_PBIT:
		img = pbit_load_pbit (fp);
		break;

	case PBIT_FORMAT_TC:
		img = pbit_load_tc (fp);
		break;
	}

	return (img);
}

pbit_img_t *pbit_img_load (const char *fname, unsigned type)
{
	FILE       *fp;
	pbit_img_t *img;

	type = pbit_get_type (type, fname);

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (NULL);
	}

	img = pbit_img_load_fp (fp, type);

	fclose (fp);

	return (img);
}

int pbit_img_save_fp (FILE *fp, const pbit_img_t *img, unsigned type)
{
	switch (type) {
	case PBIT_FORMAT_PBIT:
		return (pbit_save_pbit (fp, img));

	case PBIT_FORMAT_TC:
		return (pbit_save_tc (fp, img));
	}

	return (1);
}

int pbit_img_save (const char *fname, const pbit_img_t *img, unsigned type)
{
	int  r;
	FILE *fp;

	type = pbit_get_type (type, fname);

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = pbit_img_save_fp (fp, img, type);

	fclose (fp);

	return (r);
}
