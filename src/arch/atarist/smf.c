/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/smf.c                                       *
 * Created:     2013-06-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "smf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <devices/memory.h>


#define SMF_CHUNK_MTHD 0x4d546864
#define SMF_CHUNK_MTRK 0x4d54726b

#define SMF_CLOCK 8000000
#define SMF_TICKS 256


static void smf_put_meta (st_smf_t *smf, unsigned type, const void *buf, unsigned cnt);


static
int smf_write_header (st_smf_t *smf)
{
	unsigned char buf[16];

	if (fseek (smf->fp, smf->mthd_ofs, SEEK_SET)) {
		return (1);
	}

	buf_set_uint32_be (buf, 0, SMF_CHUNK_MTHD);
	buf_set_uint32_be (buf, 4, 6);
	buf_set_uint16_be (buf, 8, 1);
	buf_set_uint16_be (buf, 10, 1);
	buf_set_uint16_be (buf, 12, SMF_TICKS);

	if (fwrite (buf, 1, 14, smf->fp) != 14) {
		return (1);
	}

	if (fseek (smf->fp, smf->mtrk_ofs, SEEK_SET)) {
		return (1);
	}

	buf_set_uint32_be (buf, 0, SMF_CHUNK_MTRK);
	buf_set_uint32_be (buf, 4, smf->mtrk_size);

	if (fwrite (buf, 1, 8, smf->fp) != 8) {
		return (1);
	}

	if (fseek (smf->fp, smf->mtrk_ofs + 8 + smf->mtrk_size, SEEK_SET)) {
		return (1);
	}

	return (0);
}

static
int smf_close (st_smf_t *smf)
{
	if (smf->fp == NULL) {
		return (0);
	}

	smf_put_meta (smf, 0x2f, NULL, 0);

	if (smf_write_header (smf)) {
		return (1);
	}

	fclose (smf->fp);
	smf->fp = NULL;

	return (0);
}

static
int st_smf_open (st_smf_t *smf, FILE *fp)
{
	const char *str;

	if (smf_close (smf)) {
		return (1);
	}

	smf->fp = fp;

	smf->mthd_ofs = 0;
	smf->mtrk_ofs = 14;
	smf->mtrk_size = 0;

	if (smf_write_header (smf)) {
		smf->fp = NULL;
		return (1);
	}

	str = "Created by pce-atarist version " PCE_VERSION_STR;

	smf_put_meta (smf, 0x01, str, strlen (str));

	return (0);
}

static
int smf_auto_open (st_smf_t *smf)
{
	unsigned i;
	unsigned val;
	char     str[256];

	if (smf->auto_name == NULL) {
		return (smf->fp == NULL);
	}

	i = 0;
	while (smf->auto_name[i] != 0) {
		str[i] = smf->auto_name[i];
		i += 1;
	}

	str[i] = 0;

	val = ++smf->auto_index;

	while (i > 0) {
		i -= 1;

		if (str[i] == '#') {
			str[i] = '0' + (val % 10);
			val = val / 10;
		}
	}

	return (st_smf_set_file (smf, str));
}

static
void smf_put_varint (st_smf_t *smf, unsigned long val)
{
	unsigned      i;
	unsigned char buf[16];

	i = 0;

	do {
		buf[i++] = (val & 0x7f) | 0x80;
		val >>= 7;
	} while (val != 0);

	buf[0] &= 0x7f;

	while (i > 0) {
		fputc (buf[--i], smf->fp);
		smf->mtrk_size += 1;
	}
}

static
void smf_put_event (st_smf_t *smf)
{
	unsigned      i;
	unsigned long delta;

	if (smf->clock_inited) {
		delta = (smf->evt_clk - smf->clock) / (SMF_CLOCK / SMF_TICKS / 2);
	}
	else {
		delta = 0;
	}

	if ((smf->fp == NULL) || (delta > (15 * 2 * SMF_TICKS))) {
		if (smf_auto_open (smf)) {
			return;
		}

		delta = 0;
	}

	smf->clock = smf->evt_clk;
	smf->clock_inited = 1;

	smf_put_varint (smf, delta);

	fputc (smf->status, smf->fp);

	if (smf->status == 0xf0) {
		smf_put_varint (smf, smf->buf_idx);
	}

	for (i = 0; i < smf->buf_idx; i++) {
		fputc (smf->buf[i], smf->fp);
	}

	smf->mtrk_size += smf->buf_idx + 1;

	fflush (smf->fp);
}

static
void smf_put_meta (st_smf_t *smf, unsigned type, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tmp;

	tmp = buf;

	fputc (0, smf->fp);
	fputc (0xff, smf->fp);
	fputc (type, smf->fp);

	smf_put_varint (smf, cnt);

	if (cnt > 0) {
		for (i = 0; i < cnt; i++) {
			fputc (tmp[i], smf->fp);
		}
	}

	smf->mtrk_size += cnt + 3;

	fflush (smf->fp);
}

void st_smf_set_uint8 (st_smf_t *smf, unsigned char val, unsigned long clk)
{
	if (smf->status == 0xf0) {
		if (smf->buf_idx >= sizeof (smf->buf)) {
			return;
		}

		smf->buf[smf->buf_idx++] = val;
		smf->buf_cnt = smf->buf_idx;

		if (val == 0xf7) {
			smf_put_event (smf);
			smf->status = 0;
			smf->buf_idx = 0;
			smf->buf_cnt = 0;
		}
	}
	else if (val & 0x80) {
		smf->status = val;
		smf->evt_clk = clk;
		smf->buf_idx = 0;
		smf->buf_cnt = 0;

		if (val == 0xf0) {
			return;
		}

		switch (val & 0xf0) {
		case 0x80:
		case 0x90:
		case 0xa0:
		case 0xb0:
		case 0xe0:
			smf->buf_cnt = 2;
			break;

		case 0xc0:
		case 0xd0:
			smf->buf_cnt = 1;
			break;

		default:
			if ((val == 0xf1) || (val == 0xf3)) {
				smf->buf_cnt = 1;
			}
			else if (val == 0xf2) {
				smf->buf_cnt = 2;
			}
			else {
				smf->buf_cnt = 0;
			}
			break;
		}

		if (smf->buf_cnt == 0) {
			smf_put_event (smf);
		}
	}
	else if (smf->buf_idx < smf->buf_cnt) {
		if (smf->buf_idx == 0) {
			smf->evt_clk = clk;
		}

		smf->buf[smf->buf_idx++] = val;

		if (smf->buf_idx >= smf->buf_cnt) {
			smf_put_event (smf);
			smf->buf_idx = 0;
		}
	}
}

void st_smf_init (st_smf_t *smf)
{
	smf->fp = NULL;

	smf->auto_name = NULL;
	smf->auto_index = 0;

	smf->status = 0;
	smf->buf_idx = 0;
	smf->buf_cnt = 0;
}

void st_smf_free (st_smf_t *smf)
{
	smf_close (smf);
}

int st_smf_set_file (st_smf_t *smf, const char *fname)
{
	FILE *fp;

	smf_close (smf);

	if ((fp = fopen (fname, "wb")) == NULL) {
		return (1);
	}

	if (st_smf_open (smf, fp)) {
		fclose (fp);
		return (1);
	}

	return (0);
}

int st_smf_set_auto (st_smf_t *smf, const char *fname)
{
	int      ok;
	unsigned n;

	if (smf->auto_name != NULL) {
		free (smf->auto_name);
		smf->auto_name = NULL;
	}

	ok = 0;

	n = 0;
	while (fname[n] != 0) {
		if (fname[n] == '#') {
			ok = 1;
		}

		n += 1;
	}

	if (ok == 0) {
		return (st_smf_set_file (smf, fname));
	}

	if ((smf->auto_name = malloc (n + 1)) == NULL) {
		return (1);
	}

	strcpy (smf->auto_name, fname);

	return (0);
}
