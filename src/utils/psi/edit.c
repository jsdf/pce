/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/edit.c                                         *
 * Created:     2013-06-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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
#include "edit.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>


static
int psi_edit_c_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->c = (*(unsigned long *) p) & 0xff;
	par_cnt += 1;
	return (0);
}

static
int psi_edit_crcid_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_flags (sct, PSI_FLAG_CRC_ID, (*(unsigned long *) p) != 0);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_crcdata_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, (*(unsigned long *) p) != 0);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_data_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_fill (sct, *(unsigned long *) p);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_deldam_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_flags (sct, PSI_FLAG_DEL_DAM, (*(unsigned long *) p) != 0);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_fm_dd_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_encoding (sct, PSI_ENC_FM_DD);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_fm_hd_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_encoding (sct, PSI_ENC_FM_HD);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_gcr_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_encoding (sct, PSI_ENC_GCR);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_h_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->h = (*(unsigned long *) p) & 0xff;
	par_cnt += 1;
	return (0);
}

static
int psi_edit_mfm_dd_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_encoding (sct, PSI_ENC_MFM_DD);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_mfm_ed_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_encoding (sct, PSI_ENC_MFM_ED);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_mfm_hd_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_encoding (sct, PSI_ENC_MFM_HD);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_mfm_size_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_mfm_size (sct, *(unsigned long *)p);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_nodam_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, (*(unsigned long *) p) != 0);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_position_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->position = (*(unsigned long *) p) & 0xffffffff;
	par_cnt += 1;
	return (0);
}

static
int psi_edit_round_time_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	unsigned long time1, time2, range, diff;

	time1 = 8UL * psi_sct_get_size (sct);
	time2 = psi_sct_get_read_time (sct);
	range = *(unsigned long *) p;

	if ((time1 == 0) || (time2 == 0)) {
		return (0);
	}

	diff = (time1 < time2) ? (time2 - time1) : (time1 - time2);
	diff = (100UL * 100UL * diff + (time1 / 2)) / time1;

	if (diff < range) {
		psi_sct_set_read_time (sct, 0);
		par_cnt += 1;
	}

	return (0);
}

static
int psi_edit_s_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->s = (*(unsigned long *) p) & 0xff;
	par_cnt += 1;
	return (0);
}

static
int psi_edit_size_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_size (sct, *(unsigned long *) p, par_filler);
	par_cnt += 1;
	return (0);
}

static
int psi_edit_tags_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	unsigned char buf[12];

	if ((*(unsigned long *) p) != 0) {
		psi_sct_get_tags (sct, buf, 12);
		psi_sct_set_tags (sct, buf, 12);
	}
	else {
		psi_sct_set_tags (sct, buf, 0);
	}

	par_cnt += 1;

	return (0);
}

static
int psi_edit_time_cb (psi_img_t *img, psi_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	psi_sct_set_read_time (sct, *(unsigned long *) p);
	par_cnt += 1;
	return (0);
}

int psi_edit_sectors (psi_img_t *img, const char *what, const char *val)
{
	int           r;
	unsigned long v;
	psi_sct_cb    fct;

	v = strtoul (val, NULL, 0);

	if (strcmp (what, "c") == 0) {
		fct = psi_edit_c_cb;
	}
	else if (strcmp (what, "crc") == 0) {
		fct = psi_edit_crcdata_cb;
	}
	else if (strcmp (what, "crc-data") == 0) {
		fct = psi_edit_crcdata_cb;
	}
	else if (strcmp (what, "crc-id") == 0) {
		fct = psi_edit_crcid_cb;
	}
	else if (strcmp (what, "data") == 0) {
		fct = psi_edit_data_cb;
	}
	else if (strcmp (what, "del-dam") == 0) {
		fct = psi_edit_deldam_cb;
	}
	else if (strcmp (what, "fm") == 0) {
		fct = psi_edit_fm_dd_cb;
	}
	else if (strcmp (what, "fm-dd") == 0) {
		fct = psi_edit_fm_dd_cb;
	}
	else if (strcmp (what, "fm-hd") == 0) {
		fct = psi_edit_fm_hd_cb;
	}
	else if (strcmp (what, "gcr") == 0) {
		fct = psi_edit_gcr_cb;
	}
	else if (strcmp (what, "h") == 0) {
		fct = psi_edit_h_cb;
	}
	else if (strcmp (what, "mfm") == 0) {
		fct = psi_edit_mfm_dd_cb;
	}
	else if (strcmp (what, "mfm-dd") == 0) {
		fct = psi_edit_mfm_dd_cb;
	}
	else if (strcmp (what, "mfm-hd") == 0) {
		fct = psi_edit_mfm_hd_cb;
	}
	else if (strcmp (what, "mfm-ed") == 0) {
		fct = psi_edit_mfm_ed_cb;
	}
	else if (strcmp (what, "mfm-size") == 0) {
		fct = psi_edit_mfm_size_cb;
	}
	else if (strcmp (what, "no-dam") == 0) {
		fct = psi_edit_nodam_cb;
	}
	else if (strcmp (what, "position") == 0) {
		fct = psi_edit_position_cb;
	}
	else if (strcmp (what, "round-time") == 0) {
		fct = psi_edit_round_time_cb;
	}
	else if (strcmp (what, "s") == 0) {
		fct = psi_edit_s_cb;
	}
	else if (strcmp (what, "size") == 0) {
		fct = psi_edit_size_cb;
	}
	else if (strcmp (what, "tags") == 0) {
		fct = psi_edit_tags_cb;
	}
	else if (strcmp (what, "time") == 0) {
		fct = psi_edit_time_cb;
	}
	else {
		fprintf (stderr, "%s: unknown field (%s)\n", arg0, what);
		return (1);
	}

	par_cnt = 0;

	r = psi_for_all_sectors (img, fct, &v);

	if (par_verbose) {
		fprintf (stderr, "%s: edit %lu sectors (%s = %lu)\n",
			arg0, par_cnt, what, v
		);
	}

	if (r) {
		fprintf (stderr, "%s: editing failed (%s = %lu)\n",
			arg0, what, v
		);
	}

	return (r);
}
