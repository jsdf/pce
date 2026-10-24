/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/pfi-io.c                                     *
 * Created:     2012-01-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2017 Hampa Hug <hampa@hampa.ch>                     *
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

#include "pfi-io.h"
#include "pfi-a2r.h"
#include "pfi-pfi.h"
#include "pfi-kryo.h"
#include "pfi-scp.h"


unsigned pfi_get_uint16_be (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);

	return (val);
}

unsigned pfi_get_uint16_le (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[1] & 0xff;
	val = (val << 8) | (tmp[0] & 0xff);

	return (val);
}

unsigned long pfi_get_uint32_be (const void *buf, unsigned idx)
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

unsigned long pfi_get_uint32_le (const void *buf, unsigned idx)
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

void pfi_set_uint16_be (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

void pfi_set_uint16_le (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}

void pfi_set_uint32_be (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val & 0xff;
}

void pfi_set_uint32_le (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
	tmp[2] = (val >> 16) & 0xff;
	tmp[3] = (val >> 24) & 0xff;
}


int pfi_set_pos (FILE *fp, unsigned long ofs)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	return (0);
}

int pfi_read (FILE *fp, void *buf, unsigned long cnt)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pfi_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pfi_write (FILE *fp, const void *buf, unsigned long cnt)
{
	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pfi_write_ofs (FILE *fp, unsigned long ofs, const void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pfi_skip (FILE *fp, unsigned long cnt)
{
	unsigned long n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pfi_read (fp, buf, n)) {
			return (1);
		}

		cnt -= n;
	}

	return (0);
}


static
unsigned pfi_get_type (unsigned type, const char *fname)
{
	unsigned   i;
	const char *ext;

	if (type != PFI_FORMAT_NONE) {
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

	if (strcasecmp (ext, ".pfi") == 0) {
		return (PFI_FORMAT_PFI);
	}
	else if (strcasecmp (ext, ".raw") == 0) {
		return (PFI_FORMAT_KRYOFLUX);
	}
	else if (strcasecmp (ext, ".scp") == 0) {
		return (PFI_FORMAT_SCP);
	}
	else if (strcasecmp (ext, ".a2r") == 0) {
		return (PFI_FORMAT_A2R);
	}

	return (PFI_FORMAT_PFI);
}


pfi_img_t *pfi_img_load_fp (FILE *fp, unsigned type)
{
	pfi_img_t *img;

	img = NULL;

	switch (type) {
	case PFI_FORMAT_PFI:
		img = pfi_load_pfi (fp);
		break;

	case PFI_FORMAT_KRYOFLUX:
		img = pfi_load_kryo (fp);
		break;

	case PFI_FORMAT_SCP:
		img = pfi_load_scp (fp);
		break;

	case PFI_FORMAT_A2R:
		img = pfi_load_a2r (fp);
		break;
	}

	return (img);
}

pfi_img_t *pfi_img_load (const char *fname, unsigned type)
{
	FILE      *fp;
	pfi_img_t *img;

	if (strcmp (fname, "-") == 0) {
		fp = stdin;
	}
	else {
		type = pfi_get_type (type, fname);

		if (type == PFI_FORMAT_KRYOFLUX) {
			return (pfi_load_kryo_set (fname));
		}

		if ((fp = fopen (fname, "rb")) == NULL) {
			return (NULL);
		}
	}

	img = pfi_img_load_fp (fp, type);

	if (fp != stdin) {
		fclose (fp);
	}

	return (img);
}

int pfi_img_save_fp (FILE *fp, pfi_img_t *img, unsigned type)
{
	switch (type) {
	case PFI_FORMAT_PFI:
		return (pfi_save_pfi (fp, img));

	case PFI_FORMAT_KRYOFLUX:
		return (1);

	case PFI_FORMAT_SCP:
		return (pfi_save_scp (fp, img));

	case PFI_FORMAT_A2R:
		return (pfi_save_a2r (fp, img));
	}

	return (1);
}

int pfi_img_save (const char *fname, pfi_img_t *img, unsigned type)
{
	int  r;
	FILE *fp;

	if (strcmp (fname, "-") == 0) {
		fp = stdout;
	}
	else {
		type = pfi_get_type (type, fname);

		if (type == PFI_FORMAT_KRYOFLUX) {
			return (pfi_save_kryo_set (fname, (pfi_img_t *) img));
		}

		if ((fp = fopen (fname, "wb")) == NULL) {
			return (1);
		}
	}

	r = pfi_img_save_fp (fp, img, type);

	if (fp != stdout) {
		fclose (fp);
	}

	return (r);
}
