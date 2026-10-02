/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-img.c                                    *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pri-img.h"
#include "pri-img-mfm.h"
#include "pri-img-moof.h"
#include "pri-img-pbit.h"
#include "pri-img-pri.h"
#include "pri-img-tc.h"
#include "pri-img-woz.h"


unsigned pri_get_uint16_be (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);

	return (val);
}

unsigned pri_get_uint16_le (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[1] & 0xff;
	val = (val << 8) | (tmp[0] & 0xff);

	return (val);
}

unsigned long pri_get_uint32_be (const void *buf, unsigned idx)
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

unsigned long pri_get_uint32_le (const void *buf, unsigned idx)
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

void pri_set_uint16_be (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

void pri_set_uint16_le (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}

void pri_set_uint32_be (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val & 0xff;
}

void pri_set_uint32_le (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
	tmp[2] = (val >> 16) & 0xff;
	tmp[3] = (val >> 24) & 0xff;
}


int pri_set_ofs (FILE *fp, unsigned long ofs)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	return (0);
}

int pri_read (FILE *fp, void *buf, unsigned long cnt)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pri_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pri_write (FILE *fp, const void *buf, unsigned long cnt)
{
	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pri_write_ofs (FILE *fp, unsigned long ofs, const void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pri_skip (FILE *fp, unsigned long cnt)
{
	unsigned long n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pri_read (fp, buf, n)) {
			return (1);
		}

		cnt -= n;
	}

	return (0);
}


unsigned pri_guess_type (const char *fname)
{
	unsigned   i;
	const char *ext;

	ext = "";

	i = 0;
	while (fname[i] != 0) {
		if (fname[i] == '.') {
			ext = fname + i;
		}

		i += 1;
	}

	if (strcasecmp (ext, ".pbit") == 0) {
		return (PRI_FORMAT_PBIT);
	}
	else if (strcasecmp (ext, ".pri") == 0) {
		return (PRI_FORMAT_PRI);
	}
	else if (strcasecmp (ext, ".tc") == 0) {
		return (PRI_FORMAT_TC);
	}
	else if (strcasecmp (ext, ".woz") == 0) {
		return (PRI_FORMAT_WOZ);
	}
	else if (strcasecmp (ext, ".moof") == 0) {
		return (PRI_FORMAT_MOOF);
	}
	else if (strcasecmp (ext, ".mfm") == 0) {
		return (PRI_FORMAT_MFM);
	}

	return (PRI_FORMAT_NONE);
}

static
unsigned pri_get_type (unsigned type, const char *fname)
{
	if (type != PRI_FORMAT_NONE) {
		return (type);
	}

	type = pri_guess_type (fname);

	if (type != PRI_FORMAT_NONE) {
		return (type);
	}

	return (PRI_FORMAT_PRI);
}


pri_img_t *pri_img_load_fp (FILE *fp, unsigned type)
{
	pri_img_t *img;

	img = NULL;

	switch (type) {
	case PRI_FORMAT_MFM:
		img = pri_load_mfm (fp);
		break;

	case PRI_FORMAT_MOOF:
		img = pri_load_moof (fp);
		break;

	case PRI_FORMAT_PBIT:
		img = pri_load_pbit (fp);
		break;

	case PRI_FORMAT_PRI:
		img = pri_load_pri (fp);
		break;

	case PRI_FORMAT_TC:
		img = pri_load_tc (fp);
		break;

	case PRI_FORMAT_WOZ:
		img = pri_load_woz (fp);
		break;
	}

	return (img);
}

pri_img_t *pri_img_load (const char *fname, unsigned type)
{
	FILE      *fp;
	pri_img_t *img;

	type = pri_get_type (type, fname);

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (NULL);
	}

	img = pri_img_load_fp (fp, type);

	fclose (fp);

	return (img);
}

int pri_img_save_fp (FILE *fp, const pri_img_t *img, unsigned type)
{
	switch (type) {
	case PRI_FORMAT_MFM:
		return (pri_save_mfm (fp, img));

	case PRI_FORMAT_MOOF:
		return (pri_save_moof (fp, img));

	case PRI_FORMAT_PBIT:
		return (pri_save_pbit (fp, img));

	case PRI_FORMAT_PRI:
		return (pri_save_pri (fp, img));

	case PRI_FORMAT_TC:
		return (pri_save_tc (fp, img));

	case PRI_FORMAT_WOZ:
		return (pri_save_woz (fp, img));
	}

	return (1);
}

int pri_img_save (const char *fname, const pri_img_t *img, unsigned type)
{
	int  r;
	FILE *fp;

	type = pri_get_type (type, fname);

	if ((fp = fopen (fname, "w+b")) == NULL) {
		return (1);
	}

	r = pri_img_save_fp (fp, img, type);

	fclose (fp);

	return (r);
}

unsigned pri_probe_fp (FILE *fp)
{
	if (pri_probe_pri_fp (fp)) {
		return (PRI_FORMAT_PRI);
	}

	if (pri_probe_pbit_fp (fp)) {
		return (PRI_FORMAT_PBIT);
	}

	if (pri_probe_tc_fp (fp)) {
		return (PRI_FORMAT_TC);
	}

	if (pri_probe_woz_fp (fp)) {
		return (PRI_FORMAT_WOZ);
	}

	if (pri_probe_moof_fp (fp)) {
		return (PRI_FORMAT_MOOF);
	}

	if (pri_probe_mfm_fp (fp)) {
		return (PRI_FORMAT_MFM);
	}

	return (PRI_FORMAT_NONE);
}

unsigned pri_probe (const char *fname)
{
	unsigned ret;
	FILE     *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (PRI_FORMAT_NONE);
	}

	ret = pri_probe_fp (fp);

	fclose (fp);

	return (ret);
}
