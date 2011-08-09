/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfdc/main.c                                        *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/getopt.h>

#include "main.h"
#include <drivers/block/pfdc.h>
#include <drivers/block/pfdc-img-raw.h>
#include "pfdc-img-io.h"


#define PFDC_TRK_ALTERNATE (1 << 0)
#define PFDC_TRK_BAD_ID    (1 << 1)
#define PFDC_TRK_RANGE     (1 << 2)
#define PFDC_TRK_SIZE      (1 << 3)
#define PFDC_TRK_ENCODING  (1 << 4)


typedef int (*pfdc_trk_cb) (pfdc_img_t *img, pfdc_trk_t *trk,
	unsigned c, unsigned h, void *opaque
);

typedef int (*pfdc_sct_cb) (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque
);


static const char    *arg0 = NULL;

static int           par_verbose = 0;

static int           par_list_tracks = 0;
static int           par_list_sectors = 0;
static int           par_print_info = 0;

static unsigned      par_fmt_inp = PFDC_FORMAT_NONE;
static unsigned      par_fmt_out = PFDC_FORMAT_NONE;

static unsigned      par_filler = 0xf6;

static unsigned long par_cnt;

static int           par_cyl_all = 1;
static unsigned      par_cyl[2];

static int           par_trk_all = 1;
static unsigned      par_trk[2];

static int           par_sct_all = 1;
static unsigned      par_sct[2];

static int           par_rsc_all = 1;
static unsigned      par_rsc[2];

static int           par_alt_all = 1;
static unsigned      par_alt[2];

static unsigned      par_order_cnt = 0;
static unsigned char par_order[256];


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'a', 1, "alternate", "a", "Select alternate sectors" },
	{ 'c', 1, "cylinder", "c", "Select cylinders [all]" },
	{ 'e', 2, "edit", "what val", "Edit selected sector attributes" },
	{ 'f', 0, "info", NULL, "Print image information [no]" },
	{ 'F', 1, "filler", "val", "Set the sector fill byte [0xf6]" },
	{ 'h', 1, "head", "h", "Select heads [all]" },
	{ 'i', 1, "input", "filename", "Load an input file" },
	{ 'I', 1, "input-format", "format", "Set the input format [auto]" },
	{ 'l', 0, "list-tracks", NULL, "List tracks [no]" },
	{ 'L', 0, "list-sectors", NULL, "List sectors [no]" },
	{ 'm', 1, "merge", "filename", "Merge an image" },
	{ 'n', 1, "new-dos", "size", "Create a standard image of <size> KiB" },
	{ 'N', 2, "new", "type size", "Create a standard image of <size> KiB" },
	{ 'o', 1, "output", "filename", "Set the output file name [none]" },
	{ 'O', 1, "output-format", "format", "Set the output format [auto]" },
	{ 'p', 1, "operation", "name [...]", "Perform an operation" },
	{ 'r', 3, "record", "c h s", "Select sectors [all all all]" },
	{ 's', 1, "sectors", "s", "Select logical sectors [all]" },
	{ 'S', 1, "real-sectors", "s", "Select real sectors [all]" },
	{ 'v', 0, "verbose", NULL, "Verbose operation [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pfdc: convert and modify PCE FDC image files",
		"usage: pfdc [options] [input] [options] [output]",
		opts
	);

	fputs (
		"\n"
		"operations are:\n"
		"  comment-add text       Add to the image comment\n"
		"  comment-load filename  Load the image comment from a file\n"
		"  comment-print          Print the image comment\n"
		"  comment-save filename  Save the image comment to a file\n"
		"  comment-set text       Set the image comment\n"
		"  delete                 Delete sectors\n"
		"  info                   Print image information\n"
		"  load filename          Load individual sectors\n"
		"  new                    Create new sectors\n"
		"  reorder s1,s2,...      Reorder sectors in a track\n"
		"  rotate first           Rotate tracks\n"
		"  save filename          Save individual sectors\n"
		"\n"
		"file formats are:\n"
		"  pfdc, ana, imd, raw, td0\n"
		"\n"
		"sector attributes are:\n"
		"  crc-id, crc-data, del-dam, data-rate, fm, gcr, mfm, size, c, h, s\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pfdc version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2010-2011 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}


static
int pfdc_parse_range (const char *str, unsigned *v1, unsigned *v2, int *all)
{
	*v1 = 0;
	*v2 = 0;
	*all = 0;

	if (strcmp (str, "all") == 0) {
		*all = 1;
		return (0);
	}

	while ((*str >= '0') && (*str <= '9')) {
		*v1 = 10 * *v1 + (*str - '0');
		str += 1;
	}

	if (*str == '-') {
		str += 1;

		if (*str == 0) {
			*v2 = ~(unsigned) 0;
			return (0);
		}

		while ((*str >= '0') && (*str <= '9')) {
			*v2 = 10 * *v2 + (*str - '0');
			str += 1;
		}
	}
	else {
		*v2 = *v1;
	}


	if (*str != 0) {
		return (1);
	}

	return (0);
}

static
int pfdc_parse_int_list (const char **str, unsigned *val)
{
	const char *s;

	s = *str;

	if ((*s < '0') || (*s > '9')) {
		return (1);
	}

	*val = 0;

	while ((*s >= '0') && (*s <= '9')) {
		*val = 10 * *val + (*s - '0');
		s += 1;
	}

	if (*s == ',') {
		s += 1;
	}

	*str = s;

	return (0);
}

static
int pfdc_sel_match_track (unsigned c, unsigned h)
{
	if (par_cyl_all == 0) {
		if ((c < par_cyl[0]) || (c > par_cyl[1])) {
			return (0);
		}
	}

	if (par_trk_all == 0) {
		if ((h < par_trk[0]) || (h > par_trk[1])) {
			return (0);
		}
	}

	return (1);
}

static
int pfdc_sel_match (unsigned c, unsigned h, unsigned s, unsigned r, unsigned a)
{
	if (pfdc_sel_match_track (c, h) == 0) {
		return (0);
	}

	if (par_sct_all == 0) {
		if ((s < par_sct[0]) || (s > par_sct[1])) {
			return (0);
		}
	}

	if (par_rsc_all == 0) {
		if ((r < par_rsc[0]) || (r > par_rsc[1])) {
			return (0);
		}
	}

	if (par_alt_all == 0) {
		if ((a < par_alt[0]) || (a > par_alt[1])) {
			return (0);
		}
	}

	return (1);
}

static
int pfdc_for_all_sectors (pfdc_img_t *img, pfdc_sct_cb fct, void *opaque)
{
	unsigned   c, h, s, a;
	pfdc_cyl_t *cyl;
	pfdc_trk_t *trk;
	pfdc_sct_t *sct;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				a = 0;
				while (sct != 0) {
					if (pfdc_sel_match (c, h, sct->s, s, a)) {
						if (fct (img, sct, c, h, s, a, opaque)) {
							return (1);
						}
					}

					sct = sct->next;
					a += 1;
				}
			}
		}
	}

	return (0);
}

static
int pfdc_for_all_tracks (pfdc_img_t *img, pfdc_trk_cb fct, void *opaque)
{
	unsigned   c, h;
	pfdc_cyl_t *cyl;
	pfdc_trk_t *trk;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (pfdc_sel_match_track (c, h) == 0) {
				continue;
			}

			if (fct (img, trk, c, h, opaque)) {
				return (1);
			}
		}
	}

	return (0);
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
const char *pfdc_enc_to_string (unsigned encoding)
{
	switch (encoding) {
	case PFDC_ENC_FM:
		return ("FM");

	case PFDC_ENC_MFM:
		return ("MFM");

	case PFDC_ENC_GCR:
		return ("GCR");

	default:
		return ("UNKNOWN");
	}
}

static
int pfdc_list_sectors_cb (pfdc_img_t *img, pfdc_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	int           alt;
	unsigned      s;
	unsigned      pcmax, phmax, psmax;
	unsigned      lcmax, lhmax, lsmax;
	unsigned      ssmax;
	unsigned long drmax;
	pfdc_sct_t    *sct;

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

	drmax = 0;

	for (s = 0; s < trk->sct_cnt; s++) {
		psmax = (s > psmax) ? s : psmax;

		sct = trk->sct[s];

		while (sct != NULL) {
			lcmax = (sct->c > lcmax) ? sct->c : lcmax;
			lhmax = (sct->h > lhmax) ? sct->h : lhmax;
			lsmax = (sct->s > lsmax) ? sct->s : lsmax;
			ssmax = (sct->n > ssmax) ? sct->n : ssmax;
			drmax = (sct->data_rate > drmax) ? sct->data_rate : drmax;
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

	drmax = count_digits (drmax);

	for (s = 0; s < trk->sct_cnt; s++) {
		sct = trk->sct[s];

		alt = (sct->next != NULL);

		while (sct != NULL) {
			print_ulong (stdout, c, pcmax);
			print_ulong (stdout, h, phmax + 1);
			print_ulong (stdout, s, psmax + 1);

			print_ulong (stdout, sct->c, lcmax + 3);
			print_ulong (stdout, sct->h, lhmax + 1);
			print_ulong (stdout, sct->s, lsmax + 1);

			print_ulong (stdout, sct->n, ssmax + 3);

			printf (" %5s", pfdc_enc_to_string (sct->encoding));

			print_ulong (stdout, sct->data_rate, drmax + 1);

			if (sct->flags || alt) {
				fputs ("  ", stdout);
			}

			if (alt) {
				fputs (" ALT", stdout);
			}

			if (sct->flags & PFDC_FLAG_CRC_ID) {
				fputs (" CRC-ID", stdout);
			}

			if (sct->flags & PFDC_FLAG_CRC_DATA) {
				fputs (" CRC-DATA", stdout);
			}

			if (sct->flags & PFDC_FLAG_DEL_DAM) {
				fputs (" DEL-DAM", stdout);
			}

			fputs ("\n", stdout);

			sct = sct->next;
		}
	}

	return (0);
}

static
int pfdc_list_sectors (pfdc_img_t *img)
{
	return (pfdc_for_all_tracks (img, pfdc_list_sectors_cb, NULL));
}


static
int pfdc_list_track_cb (pfdc_img_t *img, pfdc_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned long sct_flg, trk_flg;
	unsigned      s;
	pfdc_sct_t    *sct;

	sct_flg = 0;
	trk_flg = 0;

	for (s = 0; s < trk->sct_cnt; s++) {
		sct = trk->sct[s];

		if (sct->next != NULL) {
			trk_flg |= PFDC_TRK_ALTERNATE;
		}

		while (sct != NULL) {
			sct_flg |= sct->flags;

			if ((sct->c != c) || (sct->h != h)) {
				trk_flg |= PFDC_TRK_BAD_ID;
			}

			if ((sct->s < 1) || (sct->s > trk->sct_cnt)) {
				trk_flg |= PFDC_TRK_RANGE;
			}

			if (sct->n != 512) {
				trk_flg |= PFDC_TRK_SIZE;
			}

			if (sct->encoding != PFDC_ENC_MFM) {
				trk_flg |= PFDC_TRK_ENCODING;
			}

			sct = sct->next;
		}
	}

	printf ("%u %u %u",
		c, h, trk->sct_cnt
	);

	if (trk_flg || sct_flg) {
		fputs ("  ", stdout);
	}

	if (trk_flg & PFDC_TRK_BAD_ID) {
		fputs (" BAD-ID", stdout);
	}

	if (trk_flg & PFDC_TRK_SIZE) {
		fputs (" SIZE", stdout);
	}

	if (trk_flg & PFDC_TRK_ENCODING) {
		fputs (" ENCODING", stdout);
	}

	if (trk_flg & PFDC_TRK_RANGE) {
		fputs (" RANGE", stdout);
	}

	if (sct_flg & PFDC_FLAG_CRC_ID) {
		fputs (" CRC-ID", stdout);
	}

	if (sct_flg & PFDC_FLAG_CRC_DATA) {
		fputs (" CRC-DATA", stdout);
	}

	if (sct_flg & PFDC_FLAG_DEL_DAM) {
		fputs (" DEL-DAM", stdout);
	}

	fputs ("\n", stdout);

	return (0);
}

static
int pfdc_list_tracks (pfdc_img_t *img)
{
	return (pfdc_for_all_tracks (img, pfdc_list_track_cb, NULL));
}


static
int pfdc_delete_sectors_cb (pfdc_img_t *img, pfdc_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned   i, j, a;
	pfdc_sct_t *sct, *tmp;

	j = 0;

	for (i = 0; i < trk->sct_cnt; i++) {
		a = 0;

		sct = trk->sct[i];

		while ((sct != NULL) && pfdc_sel_match (c, h, sct->s, i, a)) {
			tmp = sct;
			sct = tmp->next;
			tmp->next = NULL;

			pfdc_sct_del (tmp);

			par_cnt += 1;

			a += 1;
		}

		trk->sct[i] = sct;

		if (sct != NULL) {
			while (sct->next != NULL) {
				if (pfdc_sel_match (c, h, sct->s, i, a + 1) == 0) {
					tmp = sct->next;
					sct->next = tmp->next;
					tmp->next = NULL;

					pfdc_sct_del (tmp);

					par_cnt += 1;
				}
				else {
					sct = sct->next;
				}

				a += 1;
			}
		}

		if (trk->sct[i] != NULL) {
			trk->sct[j++] = trk->sct[i];
		}
	}

	trk->sct_cnt = j;

	return (0);
}

static
int pfdc_delete_sectors (pfdc_img_t *img)
{
	int r;

	par_cnt = 0;

	r = pfdc_for_all_tracks (img, pfdc_delete_sectors_cb, NULL);

	if (par_verbose) {
		fprintf (stderr, "%s: delete %lu sectors\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: deleting failed\n", arg0);
	}

	return (r);
}


static
int pfdc_new_alternates (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h)
{
	unsigned a;

	if (par_alt_all) {
		return (0);
	}

	if (par_alt[1] == 0) {
		return (0);
	}

	for (a = 1; a <= par_alt[1]; a++) {
		if (sct->next == NULL) {
			sct->next = pfdc_sct_clone (sct, 0);

			if (sct->next == NULL) {
				return (1);
			}

			par_cnt += 1;
		}

		sct = sct->next;
	}

	return (0);
}

static
int pfdc_new_sectors (pfdc_img_t *img, pfdc_trk_t *trk, unsigned c, unsigned h)
{
	unsigned   s;
	pfdc_sct_t *sct;

	if (par_sct_all) {
		for (s = 0; s < trk->sct_cnt; s++) {
			if (pfdc_new_alternates (img, trk->sct[s], c, h)) {
				return (1);
			}
		}
	}
	else {
		for (s = par_sct[0]; s <= par_sct[1]; s++) {
			sct = pfdc_img_get_sector (img, c, h, s, 0);

			if (sct == NULL) {
				sct = pfdc_sct_new (c, h, s, 512);

				if (sct == NULL) {
					return (1);
				}

				if (pfdc_trk_add_sector (trk, sct)) {
					pfdc_sct_del (sct);
					return (1);
				}

				pfdc_sct_fill (sct, par_filler);

				par_cnt += 1;
			}

			if (pfdc_new_alternates (img, sct, c, h)) {
				return (1);
			}
		}
	}

	return (0);
}

static
int pfdc_new_tracks (pfdc_img_t *img, pfdc_cyl_t *cyl, unsigned c)
{
	unsigned   h, h0, h1;
	pfdc_trk_t *trk;

	if (par_trk_all) {
		h0 = 0;
		h1 = cyl->trk_cnt;
	}
	else {
		h0 = par_trk[0];
		h1 = par_trk[1] + 1;
	}

	for (h = h0; h < h1; h++) {
		trk = pfdc_img_get_track (img, c, h, 1);

		if (trk == NULL) {
			return (1);
		}

		if (pfdc_new_sectors (img, trk, c, h)) {
			return (1);
		}
	}

	return (0);
}

static
int pfdc_new_cylinders (pfdc_img_t *img)
{
	unsigned   c, c0, c1;
	pfdc_cyl_t *cyl;

	if (par_cyl_all) {
		c0 = 0;
		c1 = img->cyl_cnt;
	}
	else {
		c0 = par_cyl[0];
		c1 = par_cyl[1] + 1;
	}

	for (c = c0; c < c1; c++) {
		cyl = pfdc_img_get_cylinder (img, c, 1);

		if (cyl == NULL) {
			return (1);
		}

		if (pfdc_new_tracks (img, cyl, c)) {
			return (1);
		}
	}

	return (0);
}

static
int pfdc_new (pfdc_img_t **img)
{
	int r;

	if (*img == NULL) {
		*img = pfdc_img_new();

		if (*img == NULL) {
			return (1);
		}
	}

	par_cnt = 0;

	r = pfdc_new_cylinders (*img);

	if (par_verbose) {
		fprintf (stderr, "%s: create %lu sectors\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: creating failed\n", arg0);
	}

	return (0);
}


static
int pfdc_reorder_track_cb (pfdc_img_t *img, pfdc_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned   i, j;
	pfdc_sct_t *tmp;

	par_cnt += 1;

	for (i = 0; i < par_order_cnt; i++) {
		if (i >= trk->sct_cnt) {
			return (0);
		}

		for (j = i; j < trk->sct_cnt; j++) {
			if (trk->sct[j]->s == par_order[i]) {
				break;
			}
		}

		if ((j < trk->sct_cnt) && (i != j)) {
			tmp = trk->sct[i];
			trk->sct[i] = trk->sct[j];
			trk->sct[j] = tmp;
		}
	}

	return (0);
}

static
int pfdc_reorder_tracks (pfdc_img_t *img, const char *order)
{
	int      r;
	unsigned i;
	unsigned val;

	i = 0;

	while (i < 256) {
		if (*order == 0) {
			break;
		}

		if (pfdc_parse_int_list (&order, &val)) {
			return (1);
		}

		par_order[i] = val;

		i += 1;
	}

	par_order_cnt = i;

	par_cnt = 0;

	r = pfdc_for_all_tracks (img, pfdc_reorder_track_cb, NULL);

	if (par_verbose) {
		fprintf (stderr, "%s: reorder %lu tracks\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: reordering failed\n", arg0);
	}

	return (r);
}


static
int pfdc_rotate_track_cb (pfdc_img_t *img, pfdc_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned   i, i0;
	unsigned   first;
	pfdc_sct_t *sct;

	if (trk->sct_cnt == 0) {
		return (0);
	}

	par_cnt += 1;

	first = *(unsigned *) opaque;

	i0 = 0;

	for (i = 0; i < trk->sct_cnt; i++) {
		sct = trk->sct[i];

		if (sct->s == first) {
			i0 = i;
			break;
		}

		if ((sct->s >= first) && (sct->s < trk->sct[i0]->s)) {
			i0 = i;
		}
	}

	while (i0 > 0) {
		sct = trk->sct[0];

		for (i = 1; i < trk->sct_cnt; i++) {
			trk->sct[i - 1] = trk->sct[i];
		}

		trk->sct[trk->sct_cnt - 1] = sct;

		i0 -= 1;
	}

	return (0);
}

static
int pfdc_rotate_tracks (pfdc_img_t *img, unsigned first)
{
	int r;

	par_cnt = 0;

	r = pfdc_for_all_tracks (img, pfdc_rotate_track_cb, &first);

	if (par_verbose) {
		fprintf (stderr, "%s: rotate %lu tracks\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: rotating failed\n", arg0);
	}

	return (r);
}


static
int pfdc_load_sectors_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque)
{
	FILE *fp;

	fp = opaque;

	pfdc_sct_fill (sct, 0);

	if (fread (sct->data, 1, sct->n, fp) != sct->n) {
		;
	}

	par_cnt += 1;

	return (0);
}

static
int pfdc_load_sectors (pfdc_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		fprintf (stderr, "%s: can't open file (%s)\n", arg0, fname);
		return (1);
	}

	par_cnt = 0;

	r = pfdc_for_all_sectors (img, pfdc_load_sectors_cb, fp);

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: load %lu sectors\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: loading sectors failed\n", arg0);
	}

	return (r);
}


static
int pfdc_save_sectors_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque)
{
	FILE *fp;

	fp = opaque;

	if (fwrite (sct->data, 1, sct->n, fp) != sct->n) {
		return (1);
	}

	par_cnt += 1;

	return (0);
}

static
int pfdc_save_sectors (pfdc_img_t *img, const char *fname)
{
	int  r;
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		fprintf (stderr, "%s: can't create file (%s)\n", arg0, fname);
		return (1);
	}

	par_cnt = 0;

	r = pfdc_for_all_sectors (img, pfdc_save_sectors_cb, fp);

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: save %lu sectors to %s\n",
			arg0, par_cnt, fname
		);
	}

	if (r) {
		fprintf (stderr, "%s: saving sectors failed\n", arg0);
	}

	return (r);
}


static
int pfdc_merge_sectors_equal (const pfdc_sct_t *s1, const pfdc_sct_t *s2)
{
	if ((s1->c != s2->c) || (s1->h != s2->h) || (s1->s != s2->s)) {
		return (0);
	}

	if (s1->n != s2->n) {
		return (0);
	}

	if (memcmp (s1->data, s2->data, s1->n) != 0) {
		return (0);
	}

	return (1);
}

static
int pfdc_merge_sectors_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *opaque)
{
	pfdc_img_t *dimg;
	pfdc_sct_t *dsct, *dtmp;

	dimg = opaque;

	dsct = pfdc_img_get_sector (dimg, c, h, sct->s, 0);

	if (dsct == NULL) {
		dsct = pfdc_sct_clone (sct, 1);

		if (dsct == NULL) {
			return (1);
		}

		if (pfdc_img_add_sector (dimg, dsct, c, h)) {
			pfdc_sct_del (dsct);
			return (1);
		}

		while (dsct != NULL) {
			par_cnt += 1;
			dsct = dsct->next;
		}

		return (0);
	}

	dtmp = dsct;

	while (dtmp != NULL) {
		if (pfdc_merge_sectors_equal (dtmp, sct)) {
			break;
		}

		dtmp = dtmp->next;
	}

	if (dtmp == NULL) {
		dtmp = pfdc_sct_clone (sct, 0);

		if (dtmp == NULL) {
			return (1);
		}

		pfdc_sct_add_alternate (dsct, dtmp);

		par_cnt += 1;
	}

	return (0);
}

static
int pfdc_merge_image (pfdc_img_t *img, const char *fname)
{
	int        r;
	pfdc_img_t *src;

	src = pfdc_img_load (fname, par_fmt_inp);

	if (src == NULL) {
		fprintf (stderr, "%s: loading image failed (%s)\n",
			arg0, fname
		);
		return (1);
	}

	par_cnt = 0;

	r = pfdc_for_all_sectors (src, pfdc_merge_sectors_cb, img);

	pfdc_img_del (src);

	if (par_verbose) {
		fprintf (stderr, "%s: merge %lu sectors from %s\n",
			arg0, par_cnt, fname
		);
	}

	if (r) {
		fprintf (stderr, "%s: merging failed\n", arg0);
	}

	return (r);
}

static
int pfdc_show_comment (pfdc_img_t *img)
{
	unsigned i;

	fputs ("comments:\n", stdout);

	for (i = 0; i < img->comment_size; i++) {
		fputc (img->comment[i], stdout);
	}

	fputs ("\n", stdout);

	return (0);
}

static
int pfdc_set_comment (pfdc_img_t *img, const char *str)
{
	const unsigned char *tmp;

	if ((str == NULL) || (*str == 0)) {
		pfdc_img_set_comment (img, NULL, 0);
		return (0);
	}

	tmp = (const unsigned char *) str;

	if (pfdc_img_set_comment (img, tmp, strlen (str))) {
		return (1);
	}

	return (0);
}

static
int pfdc_add_comment (pfdc_img_t *img, const char *str)
{
	unsigned char       c;
	const unsigned char *tmp;

	if (img->comment_size > 0) {
		c = 0x0a;

		if (pfdc_img_add_comment (img, &c, 1)) {
			return (1);
		}
	}

	tmp = (const unsigned char *) str;

	if (pfdc_img_add_comment (img, tmp, strlen (str))) {
		return (1);
	}

	return (0);
}

static
int pfdc_save_comment (pfdc_img_t *img, const char *fname)
{
	unsigned cnt;
	FILE     *fp;

	fp = fopen (fname, "w");

	if (fp == NULL) {
		return (1);
	}

	cnt = img->comment_size;

	if (cnt > 0) {
		if (fwrite (img->comment, 1, cnt, fp) != cnt) {
			fclose (fp);
			return (1);
		}

		fputc (0x0a, fp);
	}

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: save comments to %s\n", arg0, fname);
	}

	return (0);
}

static
int pfdc_load_comment (pfdc_img_t *img, const char *fname)
{
	int           c, cr;
	unsigned      i, nl;
	FILE          *fp;
	unsigned char buf[256];

	fp = fopen (fname, "r");

	if (fp == NULL) {
		return (1);
	}

	pfdc_img_set_comment (img, NULL, 0);

	cr = 0;
	nl = 0;
	i = 0;

	while (1) {
		c = fgetc (fp);

		if (c == EOF) {
			break;
		}

		if (c == 0x0d) {
			if (cr) {
				nl += 1;
			}

			cr = 1;
		}
		else if (c == 0x0a) {
			nl += 1;
			cr = 0;
		}
		else {
			if (cr) {
				nl += 1;
			}

			if (i > 0) {
				while (nl > 0) {
					buf[i++] = 0x0a;
					nl -= 1;

					if (i >= 256) {
						pfdc_img_add_comment (img, buf, i);
						i = 0;
					}
				}
			}

			nl = 0;
			cr = 0;

			buf[i++] = c;

			if (i >= 256) {
				pfdc_img_add_comment (img, buf, i);
				i = 0;
			}
		}
	}

	if (i > 0) {
		pfdc_img_add_comment (img, buf, i);
		i = 0;
	}

	fclose (fp);

	if (par_verbose) {
		fprintf (stderr, "%s: load comments from %s\n", arg0, fname);
	}

	return (0);
}

static
void pfdc_print_range (const char *str1, unsigned v1, unsigned v2, const char *str2)
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

static
int pfdc_print_info (pfdc_img_t *img)
{
	int              fc, fh, fs, ff;
	unsigned         c, h, s;
	unsigned         tcnt[2], scnt[2], ssize[2], srng[2];
	unsigned long    stotal, atotal;
	unsigned long    dsize;
	unsigned long    flags, tflags;
	const pfdc_cyl_t *cyl;
	const pfdc_trk_t *trk;
	const pfdc_sct_t *sct, *alt;

	fc = 1;
	fh = 1;
	fs = 1;

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
					tflags |= PFDC_TRK_BAD_ID;
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
	pfdc_print_range ("heads:         ", tcnt[0], tcnt[1], "\n");
	pfdc_print_range ("sectors:       ", scnt[0], scnt[1], "\n");
	pfdc_print_range ("sector range:  ", srng[0], srng[1], "\n");
	pfdc_print_range ("sector size:   ", ssize[0], ssize[1], "\n");

	printf ("flags:        ");

	ff = 1;

	if (flags & PFDC_FLAG_CRC_ID) {
		printf (" CRC-ID");
		ff = 0;
	}

	if (flags & PFDC_FLAG_CRC_DATA) {
		printf (" CRC-DATA");
		ff = 0;
	}

	if (flags & PFDC_FLAG_DEL_DAM) {
		printf (" DEL-DAM");
		ff = 0;
	}

	if (tflags & PFDC_TRK_BAD_ID) {
		printf (" BAD-ID");
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
		pfdc_show_comment (img);
	}

	return (0);
}


static
int pfdc_edit_crcid_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_set_flags (sct, PFDC_FLAG_CRC_ID, (*(unsigned long *) p) != 0);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_crcdata_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_set_flags (sct, PFDC_FLAG_CRC_DATA, (*(unsigned long *) p) != 0);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_deldam_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_set_flags (sct, PFDC_FLAG_DEL_DAM, (*(unsigned long *) p) != 0);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_fm_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_set_encoding (sct, PFDC_ENC_FM, sct->data_rate);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_gcr_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_set_encoding (sct, PFDC_ENC_GCR, sct->data_rate);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_mfm_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_set_encoding (sct, PFDC_ENC_MFM, sct->data_rate);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_c_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->c = (*(unsigned long *) p) & 0xff;
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_h_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->h = (*(unsigned long *) p) & 0xff;
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_s_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->s = (*(unsigned long *) p) & 0xff;
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_size_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_set_size (sct, *(unsigned long *) p, par_filler);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_data_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	pfdc_sct_fill (sct, *(unsigned long *) p);
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_datarate_cb (pfdc_img_t *img, pfdc_sct_t *sct,
	unsigned c, unsigned h, unsigned s, unsigned a, void *p)
{
	sct->data_rate = *(unsigned long *) p;
	par_cnt += 1;
	return (0);
}

static
int pfdc_edit_sectors (pfdc_img_t *img, const char *what, const char *val)
{
	int           r;
	unsigned long v;
	pfdc_sct_cb   fct;

	v = strtoul (val, NULL, 0);

	if (strcmp (what, "c") == 0) {
		fct = pfdc_edit_c_cb;
	}
	else if (strcmp (what, "crc") == 0) {
		fct = pfdc_edit_crcdata_cb;
	}
	else if (strcmp (what, "crc-data") == 0) {
		fct = pfdc_edit_crcdata_cb;
	}
	else if (strcmp (what, "crc-id") == 0) {
		fct = pfdc_edit_crcid_cb;
	}
	else if (strcmp (what, "data") == 0) {
		fct = pfdc_edit_data_cb;
	}
	else if (strcmp (what, "del-dam") == 0) {
		fct = pfdc_edit_deldam_cb;
	}
	else if (strcmp (what, "data-rate") == 0) {
		fct = pfdc_edit_datarate_cb;
	}
	else if (strcmp (what, "fm") == 0) {
		fct = pfdc_edit_fm_cb;
	}
	else if (strcmp (what, "gcr") == 0) {
		fct = pfdc_edit_gcr_cb;
	}
	else if (strcmp (what, "h") == 0) {
		fct = pfdc_edit_h_cb;
	}
	else if (strcmp (what, "mfm") == 0) {
		fct = pfdc_edit_mfm_cb;
	}
	else if (strcmp (what, "s") == 0) {
		fct = pfdc_edit_s_cb;
	}
	else if (strcmp (what, "size") == 0) {
		fct = pfdc_edit_size_cb;
	}
	else {
		fprintf (stderr, "%s: unknown field (%s)\n", arg0, what);
		return (1);
	}

	par_cnt = 0;

	r = pfdc_for_all_sectors (img, fct, &v);

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


static
int pfdc_operation (pfdc_img_t **img, const char *op, int argc, char **argv)
{
	int  r;
	char **optarg;

	if (*img == NULL) {
		*img = pfdc_img_new();

		if (*img == NULL) {
			return (1);
		}
	}

	r = -1;

	if (strcmp (op, "comment-print") == 0) {
		r = pfdc_show_comment (*img);
	}
	else if (strcmp (op, "delete") == 0) {
		r = pfdc_delete_sectors (*img);
	}
	else if (strcmp (op, "info") == 0) {
		r = pfdc_print_info (*img);
	}
	else if (strcmp (op, "new") == 0) {
		r = pfdc_new (img);
	}

	if (r != -1) {
		return (r);
	}

	r = pce_getopt (argc, argv, &optarg, opts);

	if ((r != 0) && (r != 'p')) {
		fprintf (stderr, "%s: bad operation argument (%s)\n",
			arg0, op
		);
		return (1);
	}

	r = 1;

	if (strcmp (op, "comment-add") == 0) {
		r = pfdc_add_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-load") == 0) {
		r = pfdc_load_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-save") == 0) {
		r = pfdc_save_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-set") == 0) {
		r = pfdc_set_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "load") == 0) {
		r = pfdc_load_sectors (*img, optarg[0]);
	}
	else if (strcmp (op, "reorder") == 0) {
		r = pfdc_reorder_tracks (*img, optarg[0]);
	}
	else if (strcmp (op, "rotate") == 0) {
		unsigned long first;

		first = strtoul (optarg[0], NULL, 0);
		r = pfdc_rotate_tracks (*img, first);
	}
	else if (strcmp (op, "save") == 0) {
		r = pfdc_save_sectors (*img, optarg[0]);
	}
	else {
		fprintf (stderr, "%s: unknown operation (%s)\n", arg0, op);
		return (1);
	}

	if (r) {
		fprintf (stderr, "%s: operation failed (%s)\n", arg0, op);
	}

	return (r);
}


static
int pfdc_new_dos (pfdc_img_t *img, unsigned long size)
{
	unsigned      c, h, s;
	unsigned      cyl_cnt, trk_cnt, sct_cnt;
	unsigned long rate;
	pfdc_cyl_t    *cyl;
	pfdc_trk_t    *trk;
	pfdc_sct_t    *sct;

	pfdc_img_erase (img);

	if (size == 0) {
		return (0);
	}

	if (pfdc_get_geometry_from_size (1024 * size, &cyl_cnt, &trk_cnt, &sct_cnt)) {
		return (1);
	}

	if (sct_cnt > 16) {
		rate = 500000;
	}
	else if (sct_cnt > 12) {
		rate = 300000;
	}
	else {
		rate = 250000;
	}

	for (c = 0; c < cyl_cnt; c++) {
		cyl = pfdc_img_get_cylinder (img, c, 1);

		if (cyl == NULL) {
			return (1);
		}

		for (h = 0; h < trk_cnt; h++) {
			trk = pfdc_img_get_track (img, c, h, 1);

			if (trk == NULL) {
				return (1);
			}

			for (s = 0; s < sct_cnt; s++) {
				sct = pfdc_sct_new (c, h, s + 1, 512);

				if (sct == NULL) {
					return (1);
				}

				if (pfdc_trk_add_sector (trk, sct)) {
					pfdc_sct_del (sct);
					return (1);
				}

				pfdc_sct_set_encoding (sct, PFDC_ENC_MFM, rate);
				pfdc_sct_fill (sct, par_filler);
			}
		}
	}

	return (0);
}

static
int pfdc_new_mac (pfdc_img_t *img, unsigned long size)
{
	unsigned   c, h, s;
	unsigned   trk_cnt, sct_cnt;
	pfdc_cyl_t *cyl;
	pfdc_trk_t *trk;
	pfdc_sct_t *sct;

	pfdc_img_erase (img);

	if (size == 400) {
		trk_cnt = 1;
	}
	else if (size == 800) {
		trk_cnt = 2;
	}
	else {
		return (1);
	}

	sct_cnt = 13;

	for (c = 0; c < 80; c++) {
		cyl = pfdc_img_get_cylinder (img, c, 1);

		if (cyl == NULL) {
			return (1);
		}

		if ((c & 15) == 0) {
			sct_cnt -= 1;
		}

		for (h = 0; h < trk_cnt; h++) {
			trk = pfdc_img_get_track (img, c, h, 1);

			if (trk == NULL) {
				return (1);
			}

			for (s = 0; s < sct_cnt; s++) {
				sct = pfdc_sct_new (c, h, s, 512);

				if (sct == NULL) {
					return (1);
				}

				if (pfdc_trk_add_sector (trk, sct)) {
					pfdc_sct_del (sct);
					return (1);
				}

				pfdc_sct_set_encoding (sct, PFDC_ENC_GCR, 250000);
				pfdc_sct_fill (sct, par_filler);
			}
		}
	}

	return (0);
}

static
pfdc_img_t *pfdc_new_image (const char *type, const char *size)
{
	int           r;
	unsigned long n;
	pfdc_img_t    *img;

	n = strtoul (size, NULL, 0);

	img = pfdc_img_new();

	if (strcmp (type, "dos") == 0) {
		r = pfdc_new_dos (img, n);
	}
	else if (strcmp (type, "mac") == 0) {
		r = pfdc_new_mac (img, n);
	}
	else {
		r = 1;
	}

	if (r) {
		fprintf (stderr, "%s: bad image type/size (%s/%s)\n",
			arg0, type, size
		);

		pfdc_img_del (img);

		return (NULL);
	}

	return (img);
}

static
pfdc_img_t *pfdc_load_image (const char *fname)
{
	pfdc_img_t *img;

	if (par_verbose) {
		fprintf (stderr, "%s: load image from %s\n", arg0, fname);
	}

	img = pfdc_img_load (fname, par_fmt_inp);

	if (img == NULL) {
		fprintf (stderr, "%s: loading failed (%s)\n", arg0, fname);
		return (NULL);
	}

	if (par_list_tracks) {
		par_list_tracks = 0;
		pfdc_list_tracks (img);
	}

	if (par_list_sectors) {
		par_list_sectors = 0;
		pfdc_list_sectors (img);
	}

	if (par_print_info) {
		par_print_info = 0;
		pfdc_print_info (img);
	}

	return (img);
}

static
int pfdc_set_format (const char *name, unsigned *val)
{
	if (strcmp (name, "pfdc") == 0) {
		*val = PFDC_FORMAT_PFDC;
	}
	else if (strcmp (name, "pfdc0") == 0) {
		*val = PFDC_FORMAT_PFDC0;
	}
	else if (strcmp (name, "pfdc1") == 0) {
		*val = PFDC_FORMAT_PFDC1;
	}
	else if (strcmp (name, "pfdc2") == 0) {
		*val = PFDC_FORMAT_PFDC;
	}
	else if (strcmp (name, "ana") == 0) {
		*val = PFDC_FORMAT_ANA;
	}
	else if (strcmp (name, "dc42") == 0) {
		*val = PFDC_FORMAT_DC42;
	}
	else if (strcmp (name, "imd") == 0) {
		*val = PFDC_FORMAT_IMD;
	}
	else if (strcmp (name, "raw") == 0) {
		*val = PFDC_FORMAT_RAW;
	}
	else if (strcmp (name, "td0") == 0) {
		*val = PFDC_FORMAT_TD0;
	}
	else {
		fprintf (stderr, "%s: unknown format (%s)\n", arg0, name);
		*val = PFDC_FORMAT_NONE;
		return (1);
	}

	return (0);
}

int main (int argc, char **argv)
{
	int        r;
	char       **optarg;
	pfdc_img_t *img;
	const char *out;

	arg0 = argv[0];

	img = NULL;
	out = NULL;

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts);

		if (r == GETOPT_DONE) {
			break;
		}

		if (r < 0) {
			return (1);
		}

		switch (r) {
		case '?':
			print_help();
			return (0);

		case 'V':
			print_version();
			return (0);

		case 'a':
			if (pfdc_parse_range (optarg[0], &par_alt[0], &par_alt[1], &par_alt_all)) {
				return (1);
			}
			break;

		case 'c':
			if (pfdc_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all)) {
				return (1);
			}
			break;

		case 'e':
			if (img != NULL) {
				if (pfdc_edit_sectors (img, optarg[0], optarg[1])) {
					return (1);
				}
			}
			break;

		case 'f':
			if (img != NULL) {
				pfdc_print_info (img);
			}
			else {
				par_print_info = 1;
			}
			break;

		case 'F':
			par_filler = strtoul (optarg[0], NULL, 0);
			break;

		case 'h':
			if (pfdc_parse_range (optarg[0], &par_trk[0], &par_trk[1], &par_trk_all)) {
				return (1);
			}
			break;

		case 'i':
			if (img != NULL) {
				pfdc_img_del (img);
			}

			img = pfdc_load_image (optarg[0]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'I':
			if (pfdc_set_format (optarg[0], &par_fmt_inp)) {
				return (1);
			}
			break;

		case 'l':
			if (img != NULL) {
				pfdc_list_tracks (img);
			}
			else {
				par_list_tracks = 1;
			}
			break;

		case 'L':
			if (img != NULL) {
				pfdc_list_sectors (img);
			}
			else {
				par_list_sectors = 1;
			}
			break;

		case 'm':
			if (img != NULL) {
				if (pfdc_merge_image (img, optarg[0])) {
					return (1);
				}
			}
			else {
				img = pfdc_load_image (optarg[0]);

				if (img == NULL) {
					return (1);
				}
			}
			break;

		case 'n':
			if (img != NULL) {
				pfdc_img_del (img);
			}

			img = pfdc_new_image ("dos", optarg[0]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'N':
			if (img != NULL) {
				pfdc_img_del (img);
			}

			img = pfdc_new_image (optarg[0], optarg[1]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'o':
			out = optarg[0];
			break;

		case 'O':
			if (pfdc_set_format (optarg[0], &par_fmt_out)) {
				return (1);
			}
			break;

		case 'p':
			if (pfdc_operation (&img, optarg[0], argc, argv)) {
				return (1);
			}
			break;

		case 'r':
			if (pfdc_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all)) {
				return (1);
			}

			if (pfdc_parse_range (optarg[1], &par_trk[0], &par_trk[1], &par_trk_all)) {
				return (1);
			}

			if (pfdc_parse_range (optarg[2], &par_sct[0], &par_sct[1], &par_sct_all)) {
				return (1);
			}
			break;

		case 's':
			if (pfdc_parse_range (optarg[0], &par_sct[0], &par_sct[1], &par_sct_all)) {
				return (1);
			}
			break;

		case 'S':
			if (pfdc_parse_range (optarg[0], &par_rsc[0], &par_rsc[1], &par_rsc_all)) {
				return (1);
			}
			break;

		case 'v':
			par_verbose = 1;
			break;

		case 0:
			if (img == NULL) {
				img = pfdc_load_image (optarg[0]);

				if (img == NULL) {
					return (1);
				}
			}
			else if (out == NULL) {
				out = optarg[0];
			}
			else {
				fprintf (stderr, "%s: unknown option (%s)\n",
					arg0, optarg[0]
				);

				return (1);
			}
			break;

		default:
			return (1);
		}
	}

	if ((img != NULL) && (out != NULL)) {
		if (par_verbose) {
			fprintf (stderr, "%s: save image to %s\n", arg0, out);
		}

		r = pfdc_img_save (out, img, par_fmt_out);

		if (r) {
			fprintf (stderr, "%s: saving failed (%s)\n",
				argv[0], out
			);
			return (1);
		}
	}

	return (0);
}
