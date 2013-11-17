/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/main.c                                         *
 * Created:     2010-08-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "delete.h"
#include "edit.h"
#include "info.h"
#include "list.h"
#include "load.h"
#include "merge.h"
#include "new.h"
#include "reorder.h"
#include "save.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/getopt.h>

#include <drivers/psi/psi.h>
#include <drivers/psi/psi-img-raw.h>
#include <drivers/psi/psi-img-stx.h>
#include <drivers/psi/psi-img.h>


const char    *arg0 = NULL;
int           par_verbose = 0;
unsigned      par_filler = 0xf6;
unsigned long par_cnt;

char          par_invert = 0;

char          par_cyl_all = 1;
char          par_cyl_inv = 0;
unsigned      par_cyl[2];

char          par_trk_all = 1;
char          par_trk_inv = 0;
unsigned      par_trk[2];

char          par_sct_all = 1;
char          par_sct_inv = 0;
unsigned      par_sct[2];

char          par_rsc_all = 1;
char          par_rsc_inv = 0;
unsigned      par_rsc[2];

char          par_alt_all = 1;
char          par_alt_inv = 0;
unsigned      par_alt[2];


static int      par_list_tracks = 0;
static int      par_list_sectors = 0;
static int      par_print_info = 0;

static unsigned par_fmt_inp = PSI_FORMAT_NONE;
static unsigned par_fmt_out = PSI_FORMAT_NONE;


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'a', 1, "alternate", "a", "Select alternate sectors" },
	{ 'c', 1, "cylinder", "c", "Select cylinders [all]" },
	{ 'D', 0, "dump", NULL, "Dump extra information [no]" },
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
	{ 'x', 0, "invert", NULL, "Invert the selection [no]" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"psi: convert and modify PCE sector image files",
		"usage: psi [options] [input] [options] [output]",
		opts
	);

	fputs (
		"\noperations are:\n"
		"  comment-add text       Add to the image comment\n"
		"  comment-load filename  Load the image comment from a file\n"
		"  comment-print          Print the image comment\n"
		"  comment-save filename  Save the image comment to a file\n"
		"  comment-set text       Set the image comment\n"
		"  delete                 Delete sectors\n"
		"  info                   Print image information\n"
		"  interleave n           Set the sector interleave to n\n"
		"  load filename          Load individual sectors\n"
		"  new                    Create new sectors\n"
		"  reorder s1,s2,...      Reorder sectors in a track\n"
		"  rotate first           Rotate tracks\n"
		"  save filename          Save individual sectors\n"
		"  sort                   Sort sectors on tracks\n"
		"  sort-reverse           Sort sectors on tracks in reverse order\n"
		"  tags-load filename     Load sector tags\n"
		"  tags-save filename     Save sector tags\n",
		stdout
	);

	fputs (
		"\nfile formats are:\n"
		"  ana, cp2, dc42, imd, msa, pfdc, psi, raw, st, td0, xdf\n",
		stdout
	);

	fputs (
		"\nsector attributes are:\n"
		"  c, crc-id, crc-data, del-dam, fm, fm-hd, gcr, h,\n"
		"  mfm, mfm-hd, mfm-ed, no-dam, position, round-time,\n"
		"  s, size, tags, time\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"psi version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2010-2013 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}


static
int psi_parse_range (const char *str, unsigned *v1, unsigned *v2, char *all, char *inv)
{
	*v1 = 0;
	*v2 = 0;
	*all = 0;
	*inv = 0;

	if (*str == '-') {
		*inv = 1;
		str += 1;
	}

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

int psi_parse_int_list (const char **str, unsigned *val)
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
int psi_sel_compare (unsigned v, unsigned v1, unsigned v2, char all, char inv)
{
	int r;

	r = (all || ((v >= v1) && (v <= v2)));

	if (inv) {
		r = !r;
	}

	return (r);
}

static
int psi_sel_match_track (unsigned c, unsigned h)
{
	if (psi_sel_compare (c, par_cyl[0], par_cyl[1], par_cyl_all, par_cyl_inv) == 0) {
		return (par_invert);
	}

	if (psi_sel_compare (h, par_trk[0], par_trk[1], par_trk_all, par_trk_inv) == 0) {
		return (par_invert);
	}

	return (!par_invert);
}

int psi_sel_match (unsigned c, unsigned h, unsigned s, unsigned r, unsigned a)
{
	if (psi_sel_match_track (c, h) == par_invert) {
		return (par_invert);
	}

	if (psi_sel_compare (s, par_sct[0], par_sct[1], par_sct_all, par_sct_inv) == 0) {
		return (par_invert);
	}

	if (psi_sel_compare (r, par_rsc[0], par_rsc[1], par_rsc_all, par_rsc_inv) == 0) {
		return (par_invert);
	}

	if (psi_sel_compare (a, par_alt[0], par_alt[1], par_alt_all, par_alt_inv) == 0) {
		return (par_invert);
	}

	return (!par_invert);
}

int psi_for_all_sectors (psi_img_t *img, psi_sct_cb fct, void *opaque)
{
	unsigned  c, h, s, a;
	psi_cyl_t *cyl;
	psi_trk_t *trk;
	psi_sct_t *sct;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			for (s = 0; s < trk->sct_cnt; s++) {
				sct = trk->sct[s];

				a = 0;
				while (sct != 0) {
					if (psi_sel_match (c, h, sct->s, s, a)) {
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

int psi_for_all_tracks (psi_img_t *img, psi_trk_cb fct, void *opaque)
{
	unsigned  c, h;
	psi_cyl_t *cyl;
	psi_trk_t *trk;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (psi_sel_match_track (c, h) == 0) {
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
int psi_operation (psi_img_t **img, const char *op, int argc, char **argv)
{
	int  r;
	char **optarg;

	if (*img == NULL) {
		*img = psi_img_new();

		if (*img == NULL) {
			return (1);
		}
	}

	r = -1;

	if (strcmp (op, "comment-print") == 0) {
		r = psi_show_comment (*img);
	}
	else if (strcmp (op, "delete") == 0) {
		r = psi_delete_sectors (*img);
	}
	else if (strcmp (op, "info") == 0) {
		r = psi_print_info (*img);
	}
	else if (strcmp (op, "new") == 0) {
		r = psi_new (img);
	}
	else if (strcmp (op, "sort") == 0) {
		r = psi_sort_tracks (*img, 0);
	}
	else if (strcmp (op, "sort-reverse") == 0) {
		r = psi_sort_tracks (*img, 1);
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
		r = psi_add_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-load") == 0) {
		r = psi_load_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-save") == 0) {
		r = psi_save_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "comment-set") == 0) {
		r = psi_set_comment (*img, optarg[0]);
	}
	else if (strcmp (op, "interleave") == 0) {
		unsigned il;

		il = strtoul (optarg[0], NULL, 0);
		r = psi_interleave_tracks (*img, il);
	}
	else if (strcmp (op, "load") == 0) {
		r = psi_load_sectors (*img, optarg[0]);
	}
	else if (strcmp (op, "reorder") == 0) {
		r = psi_reorder_tracks (*img, optarg[0]);
	}
	else if (strcmp (op, "rotate") == 0) {
		unsigned long first;

		first = strtoul (optarg[0], NULL, 0);
		r = psi_rotate_tracks (*img, first);
	}
	else if (strcmp (op, "save") == 0) {
		r = psi_save_sectors (*img, optarg[0]);
	}
	else if (strcmp (op, "tags-load") == 0) {
		r = psi_load_tags (*img, optarg[0]);
	}
	else if (strcmp (op, "tags-save") == 0) {
		r = psi_save_tags (*img, optarg[0]);
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
psi_img_t *psi_load_image (const char *fname)
{
	psi_img_t *img;

	if (par_verbose) {
		fprintf (stderr, "%s: load image from %s\n", arg0, fname);
	}

	img = psi_load (fname, par_fmt_inp);

	if (img == NULL) {
		fprintf (stderr, "%s: loading failed (%s)\n", arg0, fname);
		return (NULL);
	}

	if (par_list_tracks) {
		par_list_tracks = 0;
		psi_list_tracks (img);
	}

	if (par_list_sectors) {
		par_list_sectors = 0;
		psi_list_sectors (img);
	}

	if (par_print_info) {
		par_print_info = 0;
		psi_print_info (img);
	}

	return (img);
}

static
int psi_set_format (const char *name, unsigned *val)
{
	if (strcmp (name, "ana") == 0) {
		*val = PSI_FORMAT_ANADISK;
	}
	else if (strcmp (name, "dc42") == 0) {
		*val = PSI_FORMAT_DC42;
	}
	else if (strcmp (name, "imd") == 0) {
		*val = PSI_FORMAT_IMD;
	}
	else if (strcmp (name, "pfdc") == 0) {
		*val = PSI_FORMAT_PFDC;
	}
	else if (strcmp (name, "pfdc0") == 0) {
		*val = PSI_FORMAT_PFDC0;
	}
	else if (strcmp (name, "pfdc1") == 0) {
		*val = PSI_FORMAT_PFDC1;
	}
	else if (strcmp (name, "pfdc2") == 0) {
		*val = PSI_FORMAT_PFDC2;
	}
	else if (strcmp (name, "pfdc4") == 0) {
		*val = PSI_FORMAT_PFDC4;
	}
	else if (strcmp (name, "psi") == 0) {
		*val = PSI_FORMAT_PSI;
	}
	else if (strcmp (name, "raw") == 0) {
		*val = PSI_FORMAT_RAW;
	}
	else if (strcmp (name, "st") == 0) {
		*val = PSI_FORMAT_ST;
	}
	else if (strcmp (name, "tc") == 0) {
		*val = PSI_FORMAT_TC;
	}
	else if (strcmp (name, "td0") == 0) {
		*val = PSI_FORMAT_TD0;
	}
	else if (strcmp (name, "xdf") == 0) {
		*val = PSI_FORMAT_XDF;
	}
	else {
		fprintf (stderr, "%s: unknown format (%s)\n", arg0, name);
		*val = PSI_FORMAT_NONE;
		return (1);
	}

	return (0);
}

int main (int argc, char **argv)
{
	int        r;
	char       **optarg;
	psi_img_t *img;
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
			if (psi_parse_range (optarg[0], &par_alt[0], &par_alt[1], &par_alt_all, &par_alt_inv)) {
				return (1);
			}
			break;

		case 'c':
			if (psi_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all, &par_cyl_inv)) {
				return (1);
			}
			break;

		case 'D':
			par_stx_dump_tracks = 1;
			break;

		case 'e':
			if (img != NULL) {
				if (psi_edit_sectors (img, optarg[0], optarg[1])) {
					return (1);
				}
			}
			break;

		case 'f':
			if (img != NULL) {
				psi_print_info (img);
			}
			else {
				par_print_info = 1;
			}
			break;

		case 'F':
			par_filler = strtoul (optarg[0], NULL, 0);
			break;

		case 'h':
			if (psi_parse_range (optarg[0], &par_trk[0], &par_trk[1], &par_trk_all, &par_trk_inv)) {
				return (1);
			}
			break;

		case 'i':
			if (img != NULL) {
				psi_img_del (img);
			}

			img = psi_load_image (optarg[0]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'I':
			if (psi_set_format (optarg[0], &par_fmt_inp)) {
				return (1);
			}
			break;

		case 'l':
			if (img != NULL) {
				psi_list_tracks (img);
			}
			else {
				par_list_tracks = 1;
			}
			break;

		case 'L':
			if (img != NULL) {
				psi_list_sectors (img);
			}
			else {
				par_list_sectors = 1;
			}
			break;

		case 'm':
			if (img != NULL) {
				if (psi_merge_image (img, optarg[0])) {
					return (1);
				}
			}
			else {
				img = psi_load_image (optarg[0]);

				if (img == NULL) {
					return (1);
				}
			}
			break;

		case 'n':
			if (img != NULL) {
				psi_img_del (img);
			}

			img = psi_new_image ("dos", optarg[0]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'N':
			if (img != NULL) {
				psi_img_del (img);
			}

			img = psi_new_image (optarg[0], optarg[1]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'o':
			out = optarg[0];
			break;

		case 'O':
			if (psi_set_format (optarg[0], &par_fmt_out)) {
				return (1);
			}
			break;

		case 'p':
			if (psi_operation (&img, optarg[0], argc, argv)) {
				return (1);
			}
			break;

		case 'r':
			if (psi_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all, &par_cyl_inv)) {
				return (1);
			}

			if (psi_parse_range (optarg[1], &par_trk[0], &par_trk[1], &par_trk_all, &par_trk_inv)) {
				return (1);
			}

			if (psi_parse_range (optarg[2], &par_sct[0], &par_sct[1], &par_sct_all, &par_sct_inv)) {
				return (1);
			}
			break;

		case 's':
			if (psi_parse_range (optarg[0], &par_sct[0], &par_sct[1], &par_sct_all, &par_sct_inv)) {
				return (1);
			}
			break;

		case 'S':
			if (psi_parse_range (optarg[0], &par_rsc[0], &par_rsc[1], &par_rsc_all, &par_rsc_inv)) {
				return (1);
			}
			break;

		case 'v':
			par_verbose = 1;
			break;

		case 'x':
			par_invert = !par_invert;
			break;

		case 0:
			if (img == NULL) {
				img = psi_load_image (optarg[0]);

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

		r = psi_save (out, img, par_fmt_out);

		if (r) {
			fprintf (stderr, "%s: saving failed (%s)\n",
				argv[0], out
			);
			return (1);
		}
	}

	return (0);
}
