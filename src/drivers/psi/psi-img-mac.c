/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-mac.c                                *
 * Created:     2015-04-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015 Hampa Hug <hampa@hampa.ch>                          *
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
#include "psi-io.h"
#include "psi-img-mac.h"


static
int mac_get_file_size (FILE *fp, unsigned long *size)
{
	unsigned long pos;

	pos = ftell (fp);

	if (fseek (fp, 0, SEEK_END) != 0) {
		return (1);
	}

	*size = ftell (fp);

	if (fseek (fp, pos, SEEK_SET) != 0) {
		return (1);
	}

	return (0);
}

static
int mac_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned      c, h, hn, s, sn;
	unsigned long size;
	psi_trk_t     *trk;
	psi_sct_t     *sct;

	if (mac_get_file_size (fp, &size)) {
		return (1);
	}

	if (size == 409600) {
		hn = 1;
	}
	else if (size == 819200) {
		hn = 2;
	}
	else {
		return (1);
	}

	for (c = 0; c < 80; c++) {
		sn = 12 - (c / 16);

		for (h = 0; h < hn; h++) {
			if ((trk = psi_img_get_track (img, c, h, 1)) == NULL) {
				return (1);
			}

			for (s = 0; s < sn; s++) {
				if ((sct = psi_sct_new (c, h, s, 512)) == NULL) {
					return (1);
				}

				if (psi_trk_add_sector (trk, sct)) {
					psi_sct_del (sct);
					return (1);
				}

				psi_sct_set_size (sct, 512, 0);
				psi_sct_set_encoding (sct, PSI_ENC_GCR);

				if (fread (sct->data, 1, 512, fp) != 512) {
					return (1);
				}
			}

			psi_trk_interleave (trk, 2);
		}
	}

	return (0);
}

psi_img_t *psi_load_mac (FILE *fp)
{
	psi_img_t *img;

	if ((img = psi_img_new()) == NULL) {
		return (NULL);
	}

	if (mac_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}

int psi_probe_mac_fp (FILE *fp)
{
	unsigned long size;
	unsigned char buf[1536];

	if (mac_get_file_size (fp, &size)) {
		return (0);
	}

	if ((size != 409600) && (size != 819200)) {
		return (0);
	}

	if (fseek (fp, 0, SEEK_SET)) {
		return (0);
	}

	if (fread (buf, 1, 1536, fp) != 1536) {
		return (0);
	}

	if (psi_get_uint16_be (buf, 1024) == 0x4244) {
		/* HFS volume */
		return (1);
	}

	if (psi_get_uint16_be (buf, 1024) == 0xd2d7) {
		/* MFS volume */
		return (1);
	}

	return (0);
}

int psi_probe_mac (const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "rb")) == NULL) {
		return (0);
	}

	r = psi_probe_mac_fp (fp);

	fclose (fp);

	return (r);
}
