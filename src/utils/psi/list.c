/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/list.c                                         *
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
#include "list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>


static
const char *psi_enc_to_string (unsigned encoding)
{
	switch (encoding) {
	case PSI_ENC_FM_DD:
		return ("FM");

	case PSI_ENC_FM_HD:
		return ("FM-HD");

	case PSI_ENC_MFM_DD:
		return ("MFM");

	case PSI_ENC_MFM_HD:
		return ("MFM-HD");

	case PSI_ENC_MFM_ED:
		return ("MFM-ED");

	case PSI_ENC_GCR:
		return ("GCR");

	default:
		return ("UNKNOWN");
	}
}

static
unsigned count_digits (unsigned long val)
{
	unsigned n;

	n = 0;

	do {
		val = val / 10;
		n += 1;
	} while (val > 0);

	return (n);
}

static
void print_ulong (FILE *fp, unsigned long val, unsigned w)
{
	unsigned i;
	char     buf[128];

	i = 0;

	do {
		buf[i++] = (val % 10) + '0';
		val = val / 10;

		if (w > 0) {
			w -= 1;
		}
	} while ((val > 0) && (i < 128));

	while (w > 0) {
		fputc (' ', fp);
		w -= 1;
	}

	while (i > 0) {
		i -= 1;
		fputc (buf[i], fp);
	}
}

static
int psi_list_sectors_cb (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	int           alt;
	unsigned      s, mfm_size;
	unsigned      pcmax, phmax, psmax;
	unsigned      lcmax, lhmax, lsmax;
	unsigned      ssmax;
	unsigned long flags;
	psi_sct_t     *sct;

	if ((c > 0) || (h > 0)) {
		fputs ("\n", stdout);
	}

	pcmax = c;
	phmax = h;
	psmax = 0;

	lcmax = 0;
	lhmax = 0;
	lsmax = 0;

	ssmax = 0;

	for (s = 0; s < trk->sct_cnt; s++) {
		psmax = (s > psmax) ? s : psmax;

		sct = trk->sct[s];

		while (sct != NULL) {
			lcmax = (sct->c > lcmax) ? sct->c : lcmax;
			lhmax = (sct->h > lhmax) ? sct->h : lhmax;
			lsmax = (sct->s > lsmax) ? sct->s : lsmax;
			ssmax = (sct->n > ssmax) ? sct->n : ssmax;

			sct = sct->next;
		}
	}

	pcmax = count_digits (pcmax);
	phmax = count_digits (phmax);
	psmax = count_digits (psmax);

	lcmax = count_digits (lcmax);
	lhmax = count_digits (lhmax);
	lsmax = count_digits (lsmax);

	ssmax = count_digits (ssmax);

	for (s = 0; s < trk->sct_cnt; s++) {
		sct = trk->sct[s];

		alt = (sct->next != NULL);

		while (sct != NULL) {
			flags = sct->flags;

			print_ulong (stdout, c, pcmax);
			print_ulong (stdout, h, phmax + 1);
			print_ulong (stdout, s, psmax + 1);

			print_ulong (stdout, sct->c, lcmax + 3);
			print_ulong (stdout, sct->h, lhmax + 1);
			print_ulong (stdout, sct->s, lsmax + 1);

			print_ulong (stdout, sct->n, ssmax + 3);

			printf ("  %s", psi_enc_to_string (sct->encoding));

			mfm_size = psi_sct_get_mfm_size (sct);

			if (sct->have_mfm_size) {
				if ((mfm_size > 8) || ((128U << mfm_size) != sct->n)) {
					flags |= PSI_FLAG_MFM_SIZE;
				}
			}

			if (sct->position != 0xffffffff) {
				flags |= 0x80000000;
			}

			if (sct->read_time != 0) {
				flags |= 0x80000000;
			}

			if (flags || alt) {
				fputs ("  ", stdout);
			}

			if (alt) {
				fputs (" ALT", stdout);
			}

			if (flags & PSI_FLAG_CRC_ID) {
				fputs (" CRC-ID", stdout);
			}

			if (flags & PSI_FLAG_CRC_DATA) {
				fputs (" CRC-DATA", stdout);
			}

			if (flags & PSI_FLAG_DEL_DAM) {
				fputs (" DEL-DAM", stdout);
			}

			if (flags & PSI_FLAG_NO_DAM) {
				fputs (" NO-DAM", stdout);
			}

			if (flags & PSI_FLAG_MFM_SIZE) {
				fprintf (stdout, " MFM-SIZE=%02X", mfm_size);
			}

			if (sct->position != 0xffffffff) {
				fprintf (stdout, " POS=%-5lu", sct->position);
			}

			if (sct->read_time != 0) {
				fprintf (stdout, " TIME=%lu", sct->read_time);
			}

			fputs ("\n", stdout);

			sct = sct->next;
		}
	}

	return (0);
}

int psi_list_sectors (psi_img_t *img)
{
	return (psi_for_all_tracks (img, psi_list_sectors_cb, NULL));
}


static
int psi_list_track_cb (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned long sct_flg, trk_flg;
	unsigned      s, mfm_size;
	unsigned      enc;
	psi_sct_t     *sct;

	sct_flg = 0;
	trk_flg = 0;

	enc = 0;

	for (s = 0; s < trk->sct_cnt; s++) {
		sct = trk->sct[s];

		if (sct->next != NULL) {
			trk_flg |= PSI_TRK_ALTERNATE;
		}

		while (sct != NULL) {
			sct_flg |= sct->flags;

			if ((sct->c != c) || (sct->h != h)) {
				trk_flg |= PSI_TRK_BAD_ID;
			}

			if ((sct->s < 1) || (sct->s > trk->sct_cnt)) {
				trk_flg |= PSI_TRK_RANGE;
			}

			if (sct->n != 512) {
				trk_flg |= PSI_TRK_SIZE;
			}

			if ((enc != 0) && (sct->encoding != enc)) {
				trk_flg |= PSI_TRK_ENCODING;
			}

			if (psi_sct_get_read_time (sct) != 0) {
				trk_flg |= PSI_TRK_TIME;
			}

			if (sct->have_mfm_size) {
				mfm_size = psi_sct_get_mfm_size (sct);

				if ((mfm_size > 8) || ((128U << mfm_size) != sct->n)) {
					sct_flg |= PSI_FLAG_MFM_SIZE;
				}
			}

			if (sct->encoding != 0) {
				enc = sct->encoding;
			}

			sct = sct->next;
		}
	}

	printf ("%2u %u  %2u",
		c, h, trk->sct_cnt
	);

	if (trk_flg || sct_flg) {
		fputs ("  ", stdout);
	}

	if (trk_flg & PSI_TRK_BAD_ID) {
		fputs (" BAD-ID", stdout);
	}

	if (trk_flg & PSI_TRK_SIZE) {
		fputs (" SIZE", stdout);
	}

	if (trk_flg & PSI_TRK_ENCODING) {
		fputs (" ENCODING", stdout);
	}

	if (trk_flg & PSI_TRK_RANGE) {
		fputs (" RANGE", stdout);
	}

	if (trk_flg & PSI_TRK_TIME) {
		fputs (" TIME", stdout);
	}

	if (sct_flg & PSI_FLAG_CRC_ID) {
		fputs (" CRC-ID", stdout);
	}

	if (sct_flg & PSI_FLAG_CRC_DATA) {
		fputs (" CRC-DATA", stdout);
	}

	if (sct_flg & PSI_FLAG_DEL_DAM) {
		fputs (" DEL-DAM", stdout);
	}

	if (sct_flg & PSI_FLAG_NO_DAM) {
		fputs (" NO-DAM", stdout);
	}

	if (sct_flg & PSI_FLAG_MFM_SIZE) {
		fputs (" MFM-SIZE", stdout);
	}

	fputs ("\n", stdout);

	return (0);
}

int psi_list_tracks (psi_img_t *img)
{
	return (psi_for_all_tracks (img, psi_list_track_cb, NULL));
}
