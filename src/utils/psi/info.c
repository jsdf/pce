/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/info.c                                         *
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
#include "comment.h"
#include "info.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>


static
void psi_print_range (const char *str1, unsigned v1, unsigned v2, const char *str2)
{
	fputs (str1, stdout);

	if (v1 == v2) {
		printf ("%u", v1);
	}
	else {
		printf ("%u-%u", v1, v2);
	}

	fputs (str2, stdout);
}

int psi_print_info (psi_img_t *img)
{
	int             fc, fh, fs, ff;
	unsigned        c, h, s;
	unsigned        enc, mfm_size;
	unsigned        tcnt[2], scnt[2], ssize[2], srng[2];
	unsigned long   stotal, atotal;
	unsigned long   dsize;
	unsigned long   flags, tflags;
	const psi_cyl_t *cyl;
	const psi_trk_t *trk;
	const psi_sct_t *sct, *alt;

	fc = 1;
	fh = 1;
	fs = 1;

	enc = 0;

	tcnt[0] = 0;
	tcnt[1] = 0;

	scnt[0] = 0;
	scnt[1] = 0;

	srng[0] = 0;
	srng[1] = 0;

	ssize[0] = 0;
	ssize[1] = 0;

	stotal = 0;
	atotal = 0;
	dsize = 0;

	flags = 0;
	tflags = 0;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		if (fc || (cyl->trk_cnt < tcnt[0])) {
			tcnt[0] = cyl->trk_cnt;
		}

		if (fc || (cyl->trk_cnt > tcnt[1])) {
			tcnt[1] = cyl->trk_cnt;
		}

		fc = 0;

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (fh || (trk->sct_cnt < scnt[0])) {
				scnt[0] = trk->sct_cnt;
			}

			if (fh || (trk->sct_cnt > scnt[1])) {
				scnt[1] = trk->sct_cnt;
			}

			fh = 0;

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				if (fs || (sct->n < ssize[0])) {
					ssize[0] = sct->n;
				}

				if (fs || (sct->n > ssize[1])) {
					ssize[1] = sct->n;
				}

				if (fs || (sct->s < srng[0])) {
					srng[0] = sct->s;
				}

				if (fs || (sct->s > srng[1])) {
					srng[1] = sct->s;
				}

				if ((sct->c != c) || (sct->h != h)) {
					tflags |= PSI_TRK_BAD_ID;
				}

				if ((enc != 0) && (enc != sct->encoding)) {
					tflags |= PSI_TRK_ENCODING;
				}

				if (sct->encoding != 0) {
					enc = sct->encoding;
				}

				if (psi_sct_get_read_time (sct) != 0) {
					tflags |= PSI_TRK_TIME;
				}

				if (sct->have_mfm_size) {
					mfm_size = psi_sct_get_mfm_size (sct);

					if ((mfm_size > 8) || ((128U << mfm_size) != sct->n)) {
						flags |= PSI_FLAG_MFM_SIZE;
					}
				}

				fs = 0;

				stotal += 1;
				dsize += sct->n;
				flags |= sct->flags;

				alt = sct->next;

				while (alt != NULL) {
					atotal += 1;
					flags |= sct->flags;
					alt = alt->next;
				}
			}
		}
	}

	printf ("cylinders:     %u\n", img->cyl_cnt);
	psi_print_range ("heads:         ", tcnt[0], tcnt[1], "\n");
	psi_print_range ("sectors:       ", scnt[0], scnt[1], "\n");
	psi_print_range ("sector range:  ", srng[0], srng[1], "\n");
	psi_print_range ("sector size:   ", ssize[0], ssize[1], "\n");

	printf ("flags:        ");

	ff = 1;

	if (tflags & PSI_TRK_BAD_ID) {
		printf (" BAD-ID");
		ff = 0;
	}

	if (tflags & PSI_TRK_ENCODING) {
		printf (" ENCODING");
		ff = 0;
	}

	if (tflags & PSI_TRK_TIME) {
		printf (" TIME");
		ff = 0;
	}

	if (flags & PSI_FLAG_CRC_ID) {
		printf (" CRC-ID");
		ff = 0;
	}

	if (flags & PSI_FLAG_CRC_DATA) {
		printf (" CRC-DATA");
		ff = 0;
	}

	if (flags & PSI_FLAG_DEL_DAM) {
		printf (" DEL-DAM");
		ff = 0;
	}

	if (flags & PSI_FLAG_NO_DAM) {
		printf (" NO-DAM");
		ff = 0;
	}

	if (flags & PSI_FLAG_MFM_SIZE) {
		printf (" MFM-SIZE");
		ff = 0;
	}

	if (ff) {
		printf (" -");
	}

	printf ("\n");

	printf ("total sectors: %lu + %lu\n", stotal, atotal);
	printf ("data size:     %lu (%.2f KiB)\n", dsize, (double) dsize / 1024);

	if (img->comment_size > 0) {
		fputs ("\n", stdout);
		psi_show_comment (img);
	}

	return (0);
}
