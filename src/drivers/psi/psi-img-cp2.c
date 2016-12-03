/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/psi/psi-img-cp2.c                                *
 * Created:     2010-10-26 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2016 Hampa Hug <hampa@hampa.ch>                     *
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
#include "psi-img-cp2.h"


#ifndef DEBUG_CP2
#define DEBUG_CP2 0
#endif


#define CP2_ID_SIZE 16


static const char par_cp2_id[] = "SOFTWARE PIRATES";

static const char *par_cp2_ver[] = {
	"Release 3.02$0",
	"Release 3.07$0",
	"Release 4.00$0",
	"Release 5.01$0",
	"Release 6.0\x0a$0",
	NULL
};


static
int cp2_read_uint16 (FILE *fp, unsigned long ofs, unsigned *val)
{
	unsigned char buf[2];

	if (psi_read_ofs (fp, ofs, buf, 2)) {
		return (1);
	}

	*val = (buf[1] << 8) | buf[0];

	return (0);
}

static
int cp2_load_header (FILE *fp, unsigned long *ofs)
{
	unsigned      i;
	unsigned char buf[32];

	if (psi_read_ofs (fp, *ofs, buf, 30)) {
		return (1);
	}

	if (memcmp (buf, par_cp2_id, CP2_ID_SIZE) != 0) {
		fprintf (stderr, "cp2: not a CP2 file\n");
		return (1);
	}

	i = 0;
	while (par_cp2_ver[i] != NULL) {
		if (memcmp (buf + 16, par_cp2_ver[i], 14) == 0) {
			break;
		}

		i += 1;
	}

	if (par_cp2_ver[i] == NULL) {
		fprintf (stderr, "cp2: warning: unknown CP2 version\n");
	}

	*ofs += 30;

	return (0);
}

/*
 * Check if a sector header contains unknown fields
 */
static
void cp2_check_header (FILE *fp, const unsigned char *th, const unsigned char *sh, unsigned s)
{
	unsigned i;
	unsigned sofs;
	int      ok;

	ok = 1;

#if DEBUG_CP2 >= 1
	ok = 0;
#endif

	sofs = (sh[9] << 8) | sh[8];

	if (sh[2] & 0x96) {
		ok = 0;
	}
	else if (sh[10] || sh[11] || (sh[14] & 0x7f) || sh[15]) {
		ok = 0;
	}
	else if (sh[7] > 6) {
		ok = 0;
	}
	else if (sofs < 0x16ad) {
		ok = 0;
	}

	if (ok) {
		return;
	}

	fprintf (fp, "cp2: %u/%u/%u:", th[0], th[1], s);

	for (i = 0; i < 16; i++) {
		if (i == 8) {
			fprintf (fp, " -");
		}
		else if ((i & 3) == 0) {
			fprintf (fp, " ");
		}

		fprintf (fp, " %02X", sh[i]);
	}

	fprintf (fp, "\n");
	fflush (fp);
}

/*
 * Load a CP2 track
 *
 * ofs1:  file offset of track header
 * buf:   the track header + 24 sector headers
 * ofs2:  file offset of sector data
 * size2: maximum size of sector data
 */
static
int cp2_load_track (FILE *fp, psi_img_t *img,
	unsigned long ofs1, const unsigned char *buf,
	unsigned long ofs2, unsigned size2)
{
	unsigned            i, n;
	unsigned            ssize;
	unsigned long       sofs;
	const unsigned char *th, *sh;
	psi_trk_t           *trk;
	psi_sct_t           *sct;

	th = buf;
	sh = buf + 3;

	/* sector count */
	n = th[2];

	if (n == 0) {
		return (0);
	}
	else if (n > 24) {
		return (1);
	}

	trk = psi_img_get_track (img, th[0], th[1], 1);

	if (trk == NULL) {
		return (1);
	}

	/*
	 * sh[0]: read result
	 * sh[1]: ST0
	 * sh[2]: ST1
	 * sh[3]: ST2
	 * sh[4] - sh[7]: sector id
	 */

	for (i = 0; i < n; i++) {
		cp2_check_header (stderr, th, sh, i + 1);

		if (sh[7] <= 6) {
			ssize = 128 << sh[7];
		}
		else {
			ssize = 0;
		}

		sct = psi_sct_new (sh[4], sh[5], sh[6], ssize);

		if (sct == NULL) {
			return (1);
		}

		psi_sct_set_mfm_size (sct, sh[7]);

		if (psi_trk_add_sector (trk, sct)) {
			psi_sct_del (sct);
			return (1);
		}

		sofs = (sh[9] << 8) | sh[8];

		if (sh[14] & 0x32) {
			/* unknown flags */
			psi_sct_fill (sct, 0);
		}
		else if (sh[3] & 0x01) {
			/* missing dam */
			psi_sct_fill (sct, 0);
		}
		else if ((ssize < 256) || (ssize > 4096)) {
			/* small and large sectors are not stored */
			psi_sct_fill (sct, 0);
		}
		else if (sofs < 0x16ad) {
			/* bad data offset */
			psi_sct_fill (sct, 0);
		}
		else {
			sofs -= 0x16ad;

			if ((sofs + ssize) > size2) {
				fprintf (stderr,
					"cp2: %u/%u/%u: sector data too big (%u)\n",
					th[0], th[1], i + 1, ssize
				);

				psi_sct_fill (sct, 0);
			}
			else {
				if (psi_read_ofs (fp, ofs2 + sofs, sct->data, ssize)) {
					return (1);
				}
			}
		}

		if (sh[2] & 0x20) {
			/* crc error */

			if (sh[3] & 0x20) {
				psi_sct_set_flags (sct, PSI_FLAG_CRC_DATA, 1);
			}
			else {
				psi_sct_set_flags (sct, PSI_FLAG_CRC_ID, 1);
			}
		}

		if (sh[3] & 0x01) {
			psi_sct_set_flags (sct, PSI_FLAG_NO_DAM, 1);
		}

		if (sh[3] & 0x40) {
			psi_sct_set_flags (sct, PSI_FLAG_DEL_DAM, 1);
		}

		sh += 16;
	}

	return (0);
}

static
int cp2_load_segment (FILE *fp, psi_img_t *img, unsigned long *ofs)
{
	unsigned      ofs1;
	unsigned      size1, size2;
	unsigned char buf[3 + 16 * 24];

#if DEBUG_CP2 >= 2
	fprintf (stderr, "%08lX: segment\n", *ofs);
#endif

	if (cp2_read_uint16 (fp, *ofs, &size1)) {
		return (0);
	}

	if (size1 == 0) {
		return (0);
	}

	if (cp2_read_uint16 (fp, *ofs + size1 + 2, &size2)) {
		return (1);
	}

#if DEBUG_CP2 >= 2
	fprintf (stderr, "%08lX: segment [0x%04x 0x%04x]\n",
		*ofs, size1, size2
	);
#endif

	ofs1 = 0;

	while ((ofs1 + 3 + 16 * 24) <= size1) {
		if (psi_read_ofs (fp, *ofs + ofs1 + 2, buf, 3 + 16 * 24)) {
			return (1);
		}

#if DEBUG_CP2 >= 2
		fprintf (stderr, "%08lX: track c=%u h=%u scnt=%u\n",
			*ofs + ofs1 + 2, buf[0], buf[1], buf[2]
		);
#endif

		if (cp2_load_track (fp, img, *ofs + ofs1 + 2, buf, *ofs + size1 + 4, size2)) {
			return (1);
		}

		ofs1 += 3 + 16 * 24;
	}

	*ofs += size1 + size2 + 4;

	return (0);
}

static
int cp2_load_fp (FILE *fp, psi_img_t *img)
{
	unsigned long ofs1, ofs2;

	ofs1 = 0;

	if (cp2_load_header (fp, &ofs1)) {
		return (1);
	}

	while (1) {
		ofs2 = ofs1;

		if (cp2_load_segment (fp, img, &ofs1)) {
			return (1);
		}

		if (ofs1 == ofs2) {
			break;
		}
	}

	return (0);
}

psi_img_t *psi_load_cp2 (FILE *fp)
{
	psi_img_t *img;

	img = psi_img_new();

	if (img == NULL) {
		return (NULL);
	}

	if (cp2_load_fp (fp, img)) {
		psi_img_del (img);
		return (NULL);
	}

	return (img);
}


int psi_save_cp2 (FILE *fp, const psi_img_t *img)
{
	return (1);
}
