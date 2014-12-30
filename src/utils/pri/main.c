/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/pri.c                                          *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2014 Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/getopt.h>

#include <drivers/psi/psi-img.h>
#include <drivers/psi/psi.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-img.h>
#include <drivers/pri/pri-enc-fm.h>
#include <drivers/pri/pri-enc-mfm.h>


const char    *arg0 = NULL;

int           par_verbose = 0;

int           par_list = 0;
int           par_print_info = 0;

unsigned      par_fmt_inp = PRI_FORMAT_NONE;
unsigned      par_fmt_out = PRI_FORMAT_NONE;

char          par_invert = 0;

char          par_cyl_all = 1;
unsigned long par_cyl[2];

char          par_trk_all = 1;
unsigned long par_trk[2];

unsigned long par_data_rate = 500000;

pri_enc_fm_t  par_enc_fm;

pri_dec_mfm_t par_dec_mfm;
pri_enc_mfm_t par_enc_mfm;


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'c', 1, "cylinder", "c", "Select cylinders [all]" },
	{ 'e', 2, "edit", "what val", "Edit selected track attributes" },
	{ 'f', 0, "info", NULL, "Print image information [no]" },
	{ 'h', 1, "head", "h", "Select heads [all]" },
	{ 'i', 1, "input", "filename", "Load an input file" },
	{ 'I', 1, "input-format", "format", "Set the input format [auto]" },
	{ 'l', 0, "list-short", NULL, "List tracks (short) [no]" },
	{ 'm', 1, "merge", "filename", "Merge an image" },
	{ 'M', 1, "merge-overwrite", "filename", "Merge an image and overwrite tracks" },
	{ 'o', 1, "output", "filename", "Set the output file name [none]" },
	{ 'O', 1, "output-format", "format", "Set the output format [auto]" },
	{ 'p', 1, "operation", "name [...]", "Perform an operation" },
	{ 'r', 1, "data-rate", "rate", "Set the data rate [500000]" },
	{ 's', 2, "set", "par val", "Set a paramter value" },
	{ 't', 2, "track", "c h", "Select tracks [all]" },
	{ 'v', 0, "verbose", NULL, "Verbose operation [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{ 'x', 0, "invert", NULL, "Invert the selection [no]" },
	{ 'z', 0, "clear", NULL, "Clear the selection [yes]" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pri: convert and modify PCE raw image files",
		"usage: pri [options] [input] [options] [output]",
		opts
	);

	fputs (
		"\n"
		"operations are:\n"
		"  auto-align-gcr         Automatically align GCR tracks to the index\n"
		"  comment-add text       Add to the image comment\n"
		"  comment-load filename  Load the image comment from a file\n"
		"  comment-print          Print the image comment\n"
		"  comment-save filename  Save the image comment to a file\n"
		"  comment-set text       Set the image comment\n"
		"  decode <type> <file>   Decode tracks\n"
		"  delete                 Delete tracks\n"
		"  double-step            Remove odd numbered tracks\n"
		"  double-step-even       Remove even numbered tracks\n"
		"  encode <type> <file>   Encode tracks\n"
		"  info                   Print image information\n"
		"  new                    Create new tracks\n"
		"  rotate <bits>          Rotate tracks left\n"
		"  save <filename>        Save raw tracks\n"
		"\n"
		"parameters are:\n"
		"  mfm-auto-gap3, mfm-clock, mfm-iam, mfm-gap1, mfm-gap3, mfm-gap4a,\n"
		"  mfm-min-size, mfm-track-size\n"
		"  fm-auto-gap3, fm-clock, fm-iam, fm-gap1, fm-gap3, fm-gap4a,\n"
		"  fm-track-size\n"
		"\n"
		"decode types are:\n"
		"  auto, fm, fm-raw, gcr, gcr-raw, mfm, mfm-raw, raw\n"
		"\n"
		"encode types are:\n"
		"  auto, fm, fm-sd-300, gcr, mfm, mfm-dd-300, mfm-hd-300, mfm-hd-360\n"
		"\n"
		"file formats are:\n"
		"  pri, tc\n"
		"\n"
		"track attributes are:\n"
		"  clock, data, size\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pri version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2012-2014 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}


static
int pri_parse_range (const char *str, unsigned long *v1, unsigned long *v2, char *all)
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
			*v2 = ~(unsigned long) 0;
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
int pri_sel_match_track (unsigned c, unsigned h)
{
	if (!par_cyl_all && ((c < par_cyl[0]) || (c > par_cyl[1]))) {
		return (par_invert);
	}

	if (!par_trk_all && ((h < par_trk[0]) || (h > par_trk[1]))) {
		return (par_invert);
	}

	return (!par_invert);
}

int pri_for_all_tracks (pri_img_t *img, pri_trk_cb fct, void *opaque)
{
	unsigned long c, h;
	pri_cyl_t     *cyl;
	pri_trk_t     *trk;

	for (c = 0; c < img->cyl_cnt; c++) {
		cyl = img->cyl[c];

		if (cyl == NULL) {
			continue;
		}

		for (h = 0; h < cyl->trk_cnt; h++) {
			trk = cyl->trk[h];

			if (trk == NULL) {
				continue;
			}

			if (pri_sel_match_track (c, h) == 0) {
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
int pri_operation (pri_img_t **img, const char *op, int argc, char **argv)
{
	int  r;
	char **optarg1, **optarg2;

	if (*img == NULL) {
		*img = pri_img_new();

		if (*img == NULL) {
			return (1);
		}
	}

	r = 1;

	if (strcmp (op, "auto-align-gcr") == 0) {
		r = pri_align_gcr_tracks (*img);
	}
	else if (strcmp (op, "comment-add") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pri_comment_add (*img, optarg1[0]);
	}
	else if (strcmp (op, "comment-load") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pri_comment_load (*img, optarg1[0]);
	}
	else if (strcmp (op, "comment-print") == 0) {
		r = pri_comment_show (*img);
	}
	else if (strcmp (op, "comment-save") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pri_comment_save (*img, optarg1[0]);
	}
	else if (strcmp (op, "comment-set") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pri_comment_set (*img, optarg1[0]);
	}
	else if (strcmp (op, "decode") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			fprintf (stderr, "%s: missing decode type\n", arg0);
			return (1);
		}

		if (pce_getopt (argc, argv, &optarg2, NULL) != 0) {
			fprintf (stderr, "%s: missing file name\n", arg0);
			return (1);
		}

		r = pri_decode (*img, optarg1[0], optarg2[0]);
	}
	else if (strcmp (op, "delete") == 0) {
		r = pri_delete_tracks (*img);
	}
	else if (strcmp (op, "double-step") == 0) {
		r = pri_double_step (*img, 1);
	}
	else if (strcmp (op, "double-step-even") == 0) {
		r = pri_double_step (*img, 0);
	}
	else if (strcmp (op, "double-step-odd") == 0) {
		r = pri_double_step (*img, 1);
	}
	else if (strcmp (op, "encode") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			fprintf (stderr, "%s: missing encode type\n", arg0);
			return (1);
		}

		if (pce_getopt (argc, argv, &optarg2, NULL) != 0) {
			fprintf (stderr, "%s: missing file name\n", arg0);
			return (1);
		}

		r = pri_encode (img, optarg1[0], optarg2[0]);
	}
	else if (strcmp (op, "info") == 0) {
		r = pri_print_info (*img);
	}
	else if (strcmp (op, "new") == 0) {
		r = pri_new (*img);
	}
	else if (strcmp (op, "rotate") == 0) {
		unsigned long idx;

		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			fprintf (stderr, "%s: missing start position\n", arg0);
			return (1);
		}

		idx = strtoul (optarg1[0], NULL, 0);

		r = pri_rotate_tracks (*img, idx);
	}
	else if (strcmp (op, "save") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			fprintf (stderr, "%s: missing file name\n", arg0);
			return (1);
		}

		r = pri_decode_raw (*img, optarg1[0]);
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
pri_img_t *pri_load_image (const char *fname)
{
	pri_img_t *img;

	if (par_verbose) {
		fprintf (stderr, "%s: loading image from %s\n", arg0, fname);
	}

	if (strcmp (fname, "-") == 0) {
		img = pri_img_load_fp (stdin, par_fmt_inp);
	}
	else {
		img = pri_img_load (fname, par_fmt_inp);
	}

	if (img == NULL) {
		fprintf (stderr, "%s: loading failed (%s)\n", arg0, fname);
		return (NULL);
	}

	if (par_list) {
		par_list = 0;
		pri_list_tracks (img);
	}

	if (par_print_info) {
		par_print_info = 0;
		pri_print_info (img);
	}

	return (img);
}

static
int pri_save_image (const char *fname, pri_img_t **img)
{
	int r;

	if (*img == NULL) {
		*img = pri_img_new();
	}

	if (*img == NULL) {
		return (1);
	}

	if (par_verbose) {
		fprintf (stderr, "%s: save image to %s\n", arg0, fname);
	}

	if (strcmp (fname, "-") == 0) {
		r = pri_img_save_fp (stdout, *img, par_fmt_out);
	}
	else {
		r = pri_img_save (fname, *img, par_fmt_out);
	}

	if (r) {
		fprintf (stderr, "%s: saving failed (%s)\n",
			arg0, fname
		);

		return (1);
	}

	return (0);
}

static
int pri_set_parameter (const char *name, const char *val)
{
	if (strcmp (name, "mfm-auto-gap3") == 0) {
		par_enc_mfm.auto_gap3 = (strtoul (val, NULL, 0) != 0);
	}
	else if (strcmp (name, "mfm-clock") == 0) {
		par_enc_mfm.clock = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "mfm-iam") == 0) {
		par_enc_mfm.enable_iam = (strtoul (val, NULL, 0) != 0);
	}
	else if (strcmp (name, "mfm-gap1") == 0) {
		par_enc_mfm.gap1 = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "mfm-gap3") == 0) {
		par_enc_mfm.gap3 = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "mfm-gap4a") == 0) {
		par_enc_mfm.gap4a = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "mfm-min-size") == 0) {
		par_dec_mfm.min_sct_size = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "mfm-track-size") == 0) {
		par_enc_mfm.track_size = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "fm-auto-gap3") == 0) {
		par_enc_fm.auto_gap3 = (strtoul (val, NULL, 0) != 0);
	}
	else if (strcmp (name, "fm-clock") == 0) {
		par_enc_fm.clock = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "fm-iam") == 0) {
		par_enc_fm.enable_iam = (strtoul (val, NULL, 0) != 0);
	}
	else if (strcmp (name, "fm-gap1") == 0) {
		par_enc_fm.gap1 = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "fm-gap3") == 0) {
		par_enc_fm.gap3 = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "fm-gap4a") == 0) {
		par_enc_fm.gap4a = strtoul (val, NULL, 0);
	}
	else if (strcmp (name, "fm-track-size") == 0) {
		par_enc_fm.track_size = strtoul (val, NULL, 0);
	}
	else {
		return (1);
	}

	return (0);
}

static
int pri_set_format (const char *name, unsigned *val)
{
	if (strcmp (name, "pbit") == 0) {
		*val = PRI_FORMAT_PBIT;
	}
	else if (strcmp (name, "pri") == 0) {
		*val = PRI_FORMAT_PRI;
	}
	else if (strcmp (name, "tc") == 0) {
		*val = PRI_FORMAT_TC;
	}
	else {
		fprintf (stderr, "%s: unknown format (%s)\n", arg0, name);
		*val = PRI_FORMAT_NONE;
		return (1);
	}

	return (0);
}

int main (int argc, char **argv)
{
	int        r;
	char       **optarg;
	pri_img_t *img;

	arg0 = argv[0];

	img = NULL;

	pri_encode_fm_init (&par_enc_fm, 250000, 300);

	pri_decode_mfm_init (&par_dec_mfm);
	pri_encode_mfm_init (&par_enc_mfm, 500000, 300);

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

		case 'c':
			if (pri_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all)) {
				return (1);
			}
			break;

		case 'e':
			if (img != NULL) {
				if (pri_edit_tracks (img, optarg[0], optarg[1])) {
					return (1);
				}
			}
			break;

		case 'f':
			if (img != NULL) {
				pri_print_info (img);
			}
			else {
				par_print_info = 1;
			}
			break;

		case 'h':
			if (pri_parse_range (optarg[0], &par_trk[0], &par_trk[1], &par_trk_all)) {
				return (1);
			}
			break;

		case 'i':
			if (img != NULL) {
				pri_img_del (img);
			}

			img = pri_load_image (optarg[0]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'I':
			if (pri_set_format (optarg[0], &par_fmt_inp)) {
				return (1);
			}
			break;

		case 'l':
			if (img != NULL) {
				pri_list_tracks (img);
			}
			else {
				par_list = 1;
			}
			break;

		case 'm':
		case 'M':
			if (img != NULL) {
				if (pri_merge_image (img, optarg[0], r == 'M')) {
					fprintf (stderr, "%s: merge failed\n", arg0);
					return (1);
				}
			}
			else {
				img = pri_load_image (optarg[0]);

				if (img == NULL) {
					fprintf (stderr, "%s: merge failed\n", arg0);
					return (1);
				}
			}
			break;

		case 'o':
			if (pri_save_image (optarg[0], &img)) {
				return (1);
			}
			break;

		case 'O':
			if (pri_set_format (optarg[0], &par_fmt_out)) {
				return (1);
			}
			break;

		case 'p':
			if (pri_operation (&img, optarg[0], argc, argv)) {
				return (1);
			}
			break;

		case 'r':
			par_data_rate = strtoul (optarg[0], NULL, 0);

			if (par_data_rate <= 1000) {
				par_data_rate *= 1000;
			}
			break;

		case 's':
			if (pri_set_parameter (optarg[0], optarg[1])) {
				return (1);
			}
			break;

		case 't':
			if (pri_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all)) {
				return (1);
			}
			if (pri_parse_range (optarg[1], &par_trk[0], &par_trk[1], &par_trk_all)) {
				return (1);
			}
			break;

		case 'v':
			par_verbose = 1;
			break;

		case 'x':
			par_invert = !par_invert;
			break;

		case 'z':
			par_invert = 0;
			par_cyl_all = 1;
			par_trk_all = 1;
			break;

		case 0:
			if (img == NULL) {
				img = pri_load_image (optarg[0]);

				if (img == NULL) {
					return (1);
				}
			}
			else {
				if (pri_save_image (optarg[0], &img)) {
					return (1);
				}
			}
			break;

		default:
			return (1);
		}
	}

	return (0);
}
