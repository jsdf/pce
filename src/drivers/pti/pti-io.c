/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pti/pti-io.c                                     *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
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
#include <string.h>

#include "pti.h"
#include "pti-io.h"
#include "pti-img-cas.h"
#include "pti-img-pcm.h"
#include "pti-img-pti.h"
#include "pti-img-tap.h"
#include "pti-img-txt.h"
#include "pti-img-wav.h"


unsigned pti_get_uint16_be (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);

	return (val);
}

unsigned pti_get_uint16_le (const void *buf, unsigned idx)
{
	unsigned            val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + idx;

	val = tmp[1] & 0xff;
	val = (val << 8) | (tmp[0] & 0xff);

	return (val);
}

unsigned long pti_get_uint32_be (const void *buf, unsigned idx)
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

unsigned long pti_get_uint32_le (const void *buf, unsigned idx)
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

void pti_set_uint16_be (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

void pti_set_uint16_le (void *buf, unsigned idx, unsigned val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}

void pti_set_uint32_be (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val & 0xff;
}

void pti_set_uint32_le (void *buf, unsigned idx, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + idx;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
	tmp[2] = (val >> 16) & 0xff;
	tmp[3] = (val >> 24) & 0xff;
}


int pti_set_pos (FILE *fp, unsigned long ofs)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	return (0);
}

int pti_read (FILE *fp, void *buf, unsigned long cnt)
{
	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pti_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fread (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pti_write (FILE *fp, const void *buf, unsigned long cnt)
{
	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pti_write_ofs (FILE *fp, unsigned long ofs, const void *buf, unsigned long cnt)
{
	if (fseek (fp, ofs, SEEK_SET)) {
		return (1);
	}

	if (fwrite (buf, 1, cnt, fp) != cnt) {
		return (1);
	}

	return (0);
}

int pti_skip (FILE *fp, unsigned long cnt)
{
	unsigned long n;
	unsigned char buf[256];

	while (cnt > 0) {
		n = (cnt < 256) ? cnt : 256;

		if (pti_read (fp, buf, n)) {
			return (1);
		}

		cnt -= n;
	}

	return (0);
}


void pti_set_default_clock (unsigned long val)
{
	pti_cas_set_default_clock (val);
	pti_pcm_set_default_clock (val);
	pti_tap_set_default_clock (val);
	pti_wav_set_default_clock (val);
}

static
unsigned pti_guess_type (unsigned type, const char *fname)
{
	unsigned   i;
	const char *ext;

	if (type != PTI_FORMAT_NONE) {
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

	if (strcasecmp (ext, ".cas") == 0) {
		return (PTI_FORMAT_CAS);
	}
	else if (strcasecmp (ext, ".pcm") == 0) {
		return (PTI_FORMAT_PCM);
	}
	else if (strcasecmp (ext, ".pti") == 0) {
		return (PTI_FORMAT_PTI);
	}
	else if (strcasecmp (ext, ".tap") == 0) {
		return (PTI_FORMAT_TAP);
	}
	else if (strcasecmp (ext, ".txt") == 0) {
		return (PTI_FORMAT_TXT);
	}
	else if (strcasecmp (ext, ".wav") == 0) {
		return (PTI_FORMAT_WAV);
	}

	return (PTI_FORMAT_NONE);
}

pti_img_t *pti_img_load_fp (FILE *fp, unsigned type)
{
	pti_img_t *img;

	img = NULL;

	switch (type) {
	case PTI_FORMAT_CAS:
		img = pti_load_cas (fp, 0);
		break;

	case PTI_FORMAT_PCM:
		img = pti_load_pcm (fp, NULL);
		break;

	case PTI_FORMAT_PTI:
		img = pti_load_pti (fp);
		break;

	case PTI_FORMAT_TAP:
		img = pti_load_tap (fp, 0);
		break;

	case PTI_FORMAT_TXT:
		img = pti_load_txt (fp);
		break;

	case PTI_FORMAT_WAV:
		img = pti_load_wav (fp, 0);
		break;
	}

	return (img);
}

pti_img_t *pti_img_load (const char *fname, unsigned type)
{
	FILE      *fp;
	pti_img_t *img;

	if (strcmp (fname, "-") == 0) {
		fp = stdin;
	}
	else {
		type = pti_guess_type (type, fname);

		if ((fp = fopen (fname, "rb")) == NULL) {
			return (NULL);
		}
	}

	img = pti_img_load_fp (fp, type);

	if (fp != stdin) {
		fclose (fp);
	}

	return (img);
}

int pti_img_save_fp (FILE *fp, pti_img_t *img, unsigned type)
{
	switch (type) {
	case PTI_FORMAT_CAS:
		return (pti_save_cas (fp, img));

	case PTI_FORMAT_PCM:
		return (pti_save_pcm (fp, img, NULL));

	case PTI_FORMAT_PTI:
		return (pti_save_pti (fp, img));

	case PTI_FORMAT_TAP:
		return (pti_save_tap (fp, img));

	case PTI_FORMAT_TXT:
		return (pti_save_txt (fp, img));

	case PTI_FORMAT_WAV:
		return (pti_save_wav (fp, img, NULL));
	}

	return (1);
}

int pti_img_save (const char *fname, pti_img_t *img, unsigned type)
{
	int  r;
	FILE *fp;

	if (strcmp (fname, "-") == 0) {
		fp = stdout;
	}
	else {
		type = pti_guess_type (type, fname);

		if ((fp = fopen (fname, "wb")) == NULL) {
			return (1);
		}
	}

	r = pti_img_save_fp (fp, img, type);

	if (fp != stdout) {
		fclose (fp);
	}

	return (r);
}
