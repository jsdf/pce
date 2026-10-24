/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img.c                                    *
 * Created:     2012-02-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2021 Hampa Hug <hampa@hampa.ch>                     *
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

#include "psi.h"
#include "psi-img.h"
#include "psi-img-ana.h"
#include "psi-img-cp2.h"
#include "psi-img-dc42.h"
#include "psi-img-imd.h"
#include "psi-img-mac.h"
#include "psi-img-msa.h"
#include "psi-img-pfdc.h"
#include "psi-img-psi.h"
#include "psi-img-raw.h"
#include "psi-img-stx.h"
#include "psi-img-tc.h"
#include "psi-img-td0.h"
#include "psi-img-xdf.h"


static
const char *psi_get_ext (const char *fname)
{
	const char *ext;

	ext = "";

	while (*fname != 0) {
		if (*fname == '.') {
			ext = fname;
		}

		fname += 1;
	}

	return (ext);
}

unsigned psi_guess_type (const char *fname)
{
	const char *ext;

	ext = psi_get_ext (fname);

	if (strcasecmp (ext, ".ana") == 0) {
		return (PSI_FORMAT_ANADISK);
	}
	else if (strcasecmp (ext, ".cp2") == 0) {
		return (PSI_FORMAT_CP2);
	}
	else if (strcasecmp (ext, ".image") == 0) {
		return (PSI_FORMAT_DC42);
	}
	else if (strcasecmp (ext, ".ima") == 0) {
		return (PSI_FORMAT_RAW);
	}
	else if (strcasecmp (ext, ".imd") == 0) {
		return (PSI_FORMAT_IMD);
	}
	else if (strcasecmp (ext, ".img") == 0) {
		return (PSI_FORMAT_RAW);
	}
	else if (strcasecmp (ext, ".msa") == 0) {
		return (PSI_FORMAT_MSA);
	}
	else if (strcasecmp (ext, ".pfdc") == 0) {
		return (PSI_FORMAT_PFDC);
	}
	else if (strcasecmp (ext, ".psi") == 0) {
		return (PSI_FORMAT_PSI);
	}
	else if (strcasecmp (ext, ".raw") == 0) {
		return (PSI_FORMAT_RAW);
	}
	else if (strcasecmp (ext, ".st") == 0) {
		return (PSI_FORMAT_ST);
	}
	else if (strcasecmp (ext, ".stx") == 0) {
		return (PSI_FORMAT_STX);
	}
	else if (strcasecmp (ext, ".tc") == 0) {
		return (PSI_FORMAT_TC);
	}
	else if (strcasecmp (ext, ".td0") == 0) {
		return (PSI_FORMAT_TD0);
	}
	else if (strcasecmp (ext, ".xdf") == 0) {
		return (PSI_FORMAT_XDF);
	}

	return (PSI_FORMAT_PSI);
}

psi_img_t *psi_load_fp (FILE *fp, unsigned type)
{
	int c;

	if ((c = fgetc (fp)) == EOF) {
		if (ferror (fp)) {
			return (NULL);
		}

		return (psi_img_new());
	}

	ungetc (c, fp);

	switch (type) {
	case PSI_FORMAT_ANADISK:
		return (psi_load_anadisk (fp));

	case PSI_FORMAT_CP2:
		return (psi_load_cp2 (fp));

	case PSI_FORMAT_DC42:
		return (psi_load_dc42 (fp));

	case PSI_FORMAT_IMD:
		return (psi_load_imd (fp));

	case PSI_FORMAT_MAC:
		return (psi_load_mac (fp));

	case PSI_FORMAT_MSA:
		return (psi_load_msa (fp));

	case PSI_FORMAT_PFDC:
	case PSI_FORMAT_PFDC0:
	case PSI_FORMAT_PFDC1:
	case PSI_FORMAT_PFDC2:
	case PSI_FORMAT_PFDC4:
		return (psi_load_pfdc (fp));

	case PSI_FORMAT_PSI:
		return (psi_load_psi (fp));

	case PSI_FORMAT_RAW:
		return (psi_load_raw (fp));

	case PSI_FORMAT_RAW_HCS:
		return (psi_load_raw_hcs (fp));

	case PSI_FORMAT_RAW_HTS:
		return (psi_load_raw_hts (fp));

	case PSI_FORMAT_ST:
		return (psi_load_st (fp));

	case PSI_FORMAT_STX:
		return (psi_load_stx (fp));

	case PSI_FORMAT_TC:
		return (psi_load_tc (fp));

	case PSI_FORMAT_TD0:
		return (psi_load_td0 (fp));

	case PSI_FORMAT_XDF:
		return (psi_load_xdf (fp));
	}

	return (NULL);
}

psi_img_t *psi_load (const char *fname, unsigned type)
{
	FILE      *fp;
	psi_img_t *img;

	if (type == PSI_FORMAT_NONE) {
		type = psi_probe (fname);
	}

	if (type == PSI_FORMAT_NONE) {
		type = psi_guess_type (fname);
	}

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (NULL);
	}

	img = psi_load_fp (fp, type);

	fclose (fp);

	return (img);
}


int psi_save_fp (FILE *fp, const psi_img_t *img, unsigned type)
{
	switch (type) {
	case PSI_FORMAT_ANADISK:
		return (psi_save_anadisk (fp, img));

	case PSI_FORMAT_CP2:
		return (psi_save_cp2 (fp, img));

	case PSI_FORMAT_DC42:
		return (psi_save_dc42 (fp, img));

	case PSI_FORMAT_IMD:
		return (psi_save_imd (fp, img));

	case PSI_FORMAT_MAC:
		return (psi_save_raw (fp, img));

	case PSI_FORMAT_MSA:
		return (psi_save_msa (fp, img));

	case PSI_FORMAT_PFDC:
		return (psi_save_pfdc (fp, img, -1));

	case PSI_FORMAT_PFDC0:
		return (psi_save_pfdc (fp, img, 0));

	case PSI_FORMAT_PFDC1:
		return (psi_save_pfdc (fp, img, 1));

	case PSI_FORMAT_PFDC2:
		return (psi_save_pfdc (fp, img, 2));

	case PSI_FORMAT_PFDC4:
		return (psi_save_pfdc (fp, img, 4));

	case PSI_FORMAT_PSI:
		return (psi_save_psi (fp, img));

	case PSI_FORMAT_RAW:
		return (psi_save_raw (fp, img));

	case PSI_FORMAT_RAW_HCS:
		return (psi_save_raw_hcs (fp, img));

	case PSI_FORMAT_RAW_HTS:
		return (psi_save_raw_hts (fp, img));

	case PSI_FORMAT_ST:
		return (psi_save_st (fp, img));

	case PSI_FORMAT_STX:
		return (psi_save_stx (fp, img));

	case PSI_FORMAT_TD0:
		return (psi_save_td0 (fp, img));

	case PSI_FORMAT_XDF:
		return (psi_save_xdf (fp, img));
	}

	return (1);
}

int psi_save (const char *fname, const psi_img_t *img, unsigned type)
{
	int  r;
	FILE *fp;

	if (type == PSI_FORMAT_NONE) {
		type = psi_guess_type (fname);
	}

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	r = psi_save_fp (fp, img, type);

	fclose (fp);

	return (r);
}


unsigned psi_probe_fp (FILE *fp)
{
	if (psi_probe_psi_fp (fp)) {
		return (PSI_FORMAT_PSI);
	}

	if (psi_probe_pfdc_fp (fp)) {
		return (PSI_FORMAT_PFDC);
	}

	if (psi_probe_td0_fp (fp)) {
		return (PSI_FORMAT_TD0);
	}

	if (psi_probe_imd_fp (fp)) {
		return (PSI_FORMAT_IMD);
	}

	if (psi_probe_dc42_fp (fp)) {
		return (PSI_FORMAT_DC42);
	}

	if (psi_probe_mac_fp (fp)) {
		return (PSI_FORMAT_MAC);
	}

	if (psi_probe_msa_fp (fp)) {
		return (PSI_FORMAT_MSA);
	}

	if (psi_probe_stx_fp (fp)) {
		return (PSI_FORMAT_STX);
	}

	if (psi_probe_raw_fp (fp)) {
		return (PSI_FORMAT_RAW);
	}

	return (PSI_FORMAT_NONE);
}

unsigned psi_probe (const char *fname)
{
	unsigned ret;
	FILE     *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (PSI_FORMAT_NONE);
	}

	ret = psi_probe_fp (fp);

	fclose (fp);

	if (ret == PSI_FORMAT_RAW) {
		if (strcasecmp (psi_get_ext (fname), ".st") == 0) {
			ret = PSI_FORMAT_ST;
		}
	}

	return (ret);
}
