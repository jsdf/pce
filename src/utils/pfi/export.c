/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/export.c                                       *
 * Created:     2012-01-20 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/pfi/pfi.h>


static
int pfi_export_track_cb (pfi_img_t *img, pfi_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long val, ofs;
	FILE          *fp;

	fp = opaque;

	fprintf (fp, "TRACK %lu %lu\n", c, h);
	fprintf (fp, "CLOCK %lu\n", pfi_trk_get_clock (trk));

	pfi_trk_rewind (trk);

	while (pfi_trk_get_pulse (trk, &val, &ofs) == 0) {
		if ((val == 0) || (ofs < val)) {
			fprintf (fp, "INDEX %lu\n", ofs);
		}

		if (val > 0) {
			fprintf (fp, "%3lu\n", val);
		}
	}

	return (0);
}

int pfi_export_tracks (pfi_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "w")) == NULL) {
		return (1);
	}

	r = pfi_for_all_tracks (img, pfi_export_track_cb, fp);

	fclose (fp);

	return (r);
}


static
int pfi_skip_line (FILE *fp)
{
	int c;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			return (0);
		}

		if ((c == 0x0a) || (c == 0x0d)) {
			return (0);
		}
	}
}

static
int pfi_skip_space (FILE *fp)
{
	int c;

	while (1) {
		c = fgetc (fp);

		if ((c == ' ') || (c == '\t')) {
			;
		}
		else {
			return (c);
		}
	}
}

static
int pfi_parse_ident (FILE *fp, char *str, unsigned max, int first)
{
	int      c;
	unsigned i;

	i = 0;
	str[i++] = first;

	while (i < max) {
		c = fgetc (fp);

		if (c == EOF) {
			break;
		}
		else if ((c >= 'a') && (c <= 'z')) {
			str[i++] = c;
		}
		else if ((c >= 'A') && (c <= 'Z')) {
			str[i++] = c;
		}
		else {
			break;
		}
	}

	if (i >= max) {
		return (1);
	}

	str[i] = 0;

	return (0);
}

static
int pfi_parse_ulong (FILE *fp, unsigned long *val, int first)
{
	int c;

	if (first == 0) {
		c = pfi_skip_space (fp);

		if ((c < '0') || (c > '9')) {
			return (1);
		}

		first = c;
	}

	*val = first - '0';

	c = fgetc (fp);

	while ((c >= '0') && (c <= '9')) {
		*val = 10 * *val + (c - '0');

		c = fgetc (fp);
	}

	return (0);
}

static
int pfi_import_tracks_fp (pfi_img_t *img, FILE *fp)
{
	int           c;
	unsigned long tc, th, val, pos;
	char          str[256];
	pfi_trk_t   *trk;

	pos = 0;
	trk = NULL;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			return (0);
		}
		else if (c == '#') {
			if (pfi_skip_line (fp)) {
				return (1);
			}
		}
		else if (((c >= 'a') && (c <= 'z')) || ((c >= 'A' && c <= 'Z'))) {
			if (pfi_parse_ident (fp, str, 256, c)) {
				return (1);
			}

			if (strcasecmp (str, "TRACK") == 0) {
				if (pfi_parse_ulong (fp, &tc, 0)) {
					return (1);
				}

				if (pfi_parse_ulong (fp, &th, 0)) {
					return (1);
				}

				pfi_img_del_track (img, tc, th);

				trk = pfi_img_get_track (img, tc, th, 1);

				if (trk == NULL) {
					return (1);
				}

				pos = 0;
			}
			else if (strcasecmp (str, "CLOCK") == 0) {
				if (pfi_parse_ulong (fp, &val, 0)) {
					return (1);
				}

				if (trk != NULL) {
					pfi_trk_set_clock (trk, val);
				}
			}
			else if (strcasecmp (str, "INDEX") == 0) {
				if (pfi_parse_ulong (fp, &val, 0)) {
					return (1);
				}

				if (trk == NULL) {
					return (1);
				}

				if (pfi_trk_add_index (trk, pos + val)) {
					return (1);
				}
			}
		}
		else if ((c >= '0') && (c <= '9')) {
			if (pfi_parse_ulong (fp, &val, c)) {
				return (1);
			}

			if (trk == NULL) {
				return (1);
			}

			if (pfi_trk_add_pulse (trk, val)) {
				return (1);
			}

			pos += val;
		}
	}
}

int pfi_import_tracks (pfi_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	if ((fp = fopen (fname, "r")) == NULL) {
		return (1);
	}

	r = pfi_import_tracks_fp (img, fp);

	fclose (fp);

	return (r);
}
