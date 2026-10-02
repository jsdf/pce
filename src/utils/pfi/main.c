/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pfi/main.c                                         *
 * Created:     2012-01-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2022 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/getopt.h>

#include <drivers/pri/pri.h>
#include <drivers/pri/pri-enc-gcr.h>

#include <drivers/psi/psi-img-psi.h>
#include <drivers/psi/psi.h>

#include <drivers/pfi/pfi.h>
#include <drivers/pfi/pfi-io.h>


const char    *arg0 = NULL;

int           par_verbose = 0;

int           par_list = 0;
int           par_list_long = 0;
int           par_print_info = 0;

unsigned      par_fmt_inp = PFI_FORMAT_NONE;
unsigned      par_fmt_out = PFI_FORMAT_NONE;

char          par_invert = 0;

char          par_cyl_all = 1;
char          par_cyl_inv = 0;
unsigned long par_cyl[2];

char          par_trk_all = 1;
char          par_trk_inv = 0;
unsigned long par_trk[2];

unsigned long par_data_rate = 500000;

unsigned long par_pfi_clock = 24027428;

unsigned      par_revolution = 1;

double        par_slack1 = 0.01;
double        par_slack2 = 0.01;

int           par_weak_bits = 0;
unsigned long par_weak_i1 = 0;
unsigned long par_weak_i2 = 0;

unsigned long par_clock_tolerance = 40;

unsigned      par_fold_mode = PFI_FOLD_MAXRUN;
unsigned      par_fold_window = 32;
unsigned long par_fold_max = 16384;

unsigned      par_pfi_fold_revolution = 0;
unsigned long par_pfi_fold_window = 1024;
unsigned long par_pfi_fold_compare = 2048;
int           par_pfi_fold_right = 0;


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'c', 1, "cylinder", "c", "Select cylinders [all]" },
	{ 'f', 0, "info", NULL, "Print image information [no]" },
	{ 'h', 1, "head", "h", "Select heads [all]" },
	{ 'i', 1, "input", "filename", "Load an input file" },
	{ 'I', 1, "input-format", "format", "Set the input format [auto]" },
	{ 'l', 0, "list-short", NULL, "List tracks (short) [no]" },
	{ 'L', 0, "list-long", NULL, "List tracks (long) [no]" },
	{ 'o', 1, "output", "filename", "Set the output file name [none]" },
	{ 'O', 1, "output-format", "format", "Set the output format [auto]" },
	{ 'p', 1, "operation", "name [...]", "Perform an operation" },
	{ 'r', 1, "data-rate", "rate", "Set the data rate [500000]" },
	{ 'R', 1, "revolution", "n", "Use the nth revolution [1]" },
	{ 's', 2, "set", "par val", "Set a parameter value" },
	{ 't', 2, "track", "c h", "Select tracks [all]" },
	{ 'v', 0, "verbose", NULL, "Verbose operation [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{ 'x', 0, "invert", NULL, "Invert the selection [no]" },
	{ 'z', 0, "clear", NULL, "Clear the selection" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pfi: convert and modify PCE Flux Image files",
		"usage: pfi [options] [input] [options] [output]",
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
		"  decode <type> <file>   Decode tracks\n"
		"  delete                 Delete tracks\n"
		"  double-step            Remove odd numbered tracks\n"
		"  double-step-even       Remove even numbered tracks\n"
		"  encode <type> <file>   Encode a file\n"
		"  export <filename>      Export tracks as text\n"
		"  fold <revolutions>     Fold tracks\n"
		"  import <filename>      Import tracks as text\n"
		"  info                   Print image information\n"
		"  quantize <rate>        Quantize the pulses to <rate>\n"
		"  revolutions <range>    Extract revolutions\n"
		"  scale <factor>         Scale tracks by factor\n"
		"  set-clock <clock>      Set the clock rate\n"
		"  set-rpm <rpm>          Set average RPM\n"
		"  set-rpm-mac            Set Macintosh RPMs at 500 kbit/s\n"
		"  set-rpm-mac-490        Set Macintosh RPMs at 489.6 kbit/s\n"
		"  set-rpm-mac-500        Set Macintosh RPMs at 500 kbit/s\n"
		"  slack <ms>             Limit the slack space\n"
		"  shift-index <offset>   Shift the index by offset clock cycles\n"
		"  shift-index-us <us>    Shift the index by us microseconds\n"
		"  wpcom                  Simulate write precompensation\n"
		"\n"
		"parameters are:\n"
		"  clock-tolerance\n"
		"  fold-max\n"
		"  fold-mode\n"
		"  pfi-clock\n"
		"  pfi-fold-compare\n"
		"  pfi-fold-revolution\n"
		"  pfi-fold-right\n"
		"  pfi-fold-window\n"
		"  pll\n"
		"  slack1\n"
		"  slack2\n"
		"  weak-bits\n"
		"\n"
		"decode types are:\n"
		"  pri, pri-mac, pri-mac-490, pri-mac-500,"
		" raw, gcr-raw, mfm-raw\n"
		"\n"
		"encode types are:\n"
		"  pri\n"
		"\n"
		"file formats are:\n"
		"  a2r, pfi, raw, scp\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pfi version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2012-" PCE_YEAR " Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

int pfi_parse_double (const char *str, double *val)
{
	char *end;

	if ((str == NULL) || (*str == 0)) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	*val = strtod (str, &end);

	if ((end != NULL) && (*end == 0)) {
		return (0);
	}

	fprintf (stderr, "%s: bad value (%s)\n", arg0, str);

	return (1);
}

int pfi_parse_long (const char *str, long *val)
{
	char *end;

	if ((str == NULL) || (*str == 0)) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	*val = strtol (str, &end, 0);

	if ((end != NULL) && (*end == 0)) {
		return (0);
	}

	fprintf (stderr, "%s: bad value (%s)\n", arg0, str);

	return (1);
}

int pfi_parse_ulong (const char *str, unsigned long *val)
{
	char *end;

	if ((str == NULL) || (*str == 0)) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	*val = strtoul (str, &end, 0);

	if ((end != NULL) && (*end == 0)) {
		return (0);
	}

	fprintf (stderr, "%s: bad value (%s)\n", arg0, str);

	return (1);
}

int pfi_parse_uint (const char *str, unsigned *val)
{
	unsigned long tmp;

	if (pfi_parse_ulong (str, &tmp)) {
		return (1);
	}

	*val = tmp;

	return (0);
}

int pfi_parse_bool (const char *str, int *val)
{
	unsigned long tmp;

	if (pfi_parse_ulong (str, &tmp)) {
		return (1);
	}

	*val = tmp != 0;

	return (0);
}

int pfi_parse_rate (const char *str, unsigned long *val)
{
	char *end;

	if ((str == NULL) || (*str == 0)) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	if (strcmp (str, "a2r") == 0) {
		*val = 8000000;
		return (0);
	}
	else if ((strcmp (str, "kryoflux") == 0) || (strcmp (str, "kryo") == 0)) {
		*val = 24027428;
		return (0);
	}
	else if (strcmp (str, "scp") == 0) {
		*val = 40000000;
		return (0);
	}

	*val = strtoul (str, &end, 0);

	if ((end == NULL) || (*end == 0)) {
		return (0);
	}

	if ((*end == 'k') || (*end == 'K')) {
		*val *= 1000;
		end += 1;
	}
	else if ((*end == 'm') || (*end == 'M')) {
		*val *= 1000000;
		end += 1;
	}

	if (*end != 0) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	return (0);
}

int pfi_parse_range (const char *str, unsigned long *v1, unsigned long *v2, char *all)
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

int pfi_parse_time (const char *str, double *val, double def)
{
	char *end;

	if ((str == NULL) || (*str == 0)) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	*val = strtod (str, &end);

	if (end == NULL) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	while ((*end == ' ') || (*end == '\t')) {
		end += 1;
	}

	if (*end == 0) {
		*val *= def;
	}
	else if (strcmp (end, "s") == 0) {
		;
	}
	else if (strcmp (end, "ms") == 0) {
		*val /= 1000.0;
	}
	else if (strcmp (end, "us") == 0) {
		*val /= 1000.0 * 1000.0;
	}
	else if (strcmp (end, "ns") == 0) {
		*val /= 1000.0 * 1000.0 * 1000.0;
	}
	else {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	return (0);
}


static
int pfi_sel_match_track (unsigned c, unsigned h)
{
	if (!par_cyl_all && ((c < par_cyl[0]) || (c > par_cyl[1]))) {
		return (par_invert);
	}

	if (!par_trk_all && ((h < par_trk[0]) || (h > par_trk[1]))) {
		return (par_invert);
	}

	return (!par_invert);
}

int pfi_for_all_tracks (pfi_img_t *img, pfi_trk_cb fct, void *opaque)
{
	unsigned long c, h;
	pfi_cyl_t     *cyl;
	pfi_trk_t     *trk;

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

			if (pfi_sel_match_track (c, h) == 0) {
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
int pfi_operation (pfi_img_t **img, const char *op, int argc, char **argv)
{
	int  r;
	char **optarg1, **optarg2;

	if (*img == NULL) {
		if ((*img = pfi_img_new()) == NULL) {
			return (1);
		}
	}

	r = 1;

	if (strcmp (op, "comment-add") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pfi_comment_add (*img, optarg1[0]);
	}
	else if (strcmp (op, "comment-load") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pfi_comment_load (*img, optarg1[0]);
	}
	else if (strcmp (op, "comment-print") == 0) {
		r = pfi_comment_show (*img);
	}
	else if (strcmp (op, "comment-save") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pfi_comment_save (*img, optarg1[0]);
	}
	else if (strcmp (op, "comment-set") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pfi_comment_set (*img, optarg1[0]);
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

		r = pfi_decode (*img, optarg1[0], par_data_rate, optarg2[0]);
	}
	else if (strcmp (op, "delete") == 0) {
		r = pfi_delete_tracks (*img);
	}
	else if (strcmp (op, "double-step") == 0) {
		r = pfi_double_step (*img, 1);
	}
	else if (strcmp (op, "double-step-even") == 0) {
		r = pfi_double_step (*img, 0);
	}
	else if (strcmp (op, "double-step-odd") == 0) {
		r = pfi_double_step (*img, 1);
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

		r = pfi_encode (*img, optarg1[0], optarg2[0]);
	}
	else if (strcmp (op, "fold") == 0) {
		r = pfi_fold (*img);
	}
	else if (strcmp (op, "info") == 0) {
		r = pfi_print_info (*img);
	}
	else if (strcmp (op, "quantize") == 0) {
		unsigned long rate;

		if (pce_getoptarg (argc, argv, &optarg1, 1) != 0) {
			return (1);
		}

		if (pfi_parse_rate (optarg1[0], &rate)) {
			return (1);
		}

		r = pfi_quantize (*img, rate);
	}
	else if (strcmp (op, "revolutions") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			fprintf (stderr, "%s: missing revolutions\n", arg0);
			return (1);
		}

		r = pfi_revolutions (*img, optarg1[0]);
	}
	else if (strcmp (op, "scale") == 0) {
		double factor;

		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		if (pfi_parse_double (optarg1[0], &factor)) {
			return (1);
		}

		r = pfi_scale_tracks (*img, factor);
	}
	else if (strcmp (op, "set-rpm") == 0) {
		double rpm;

		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		if (pfi_parse_double (optarg1[0], &rpm)) {
			return (1);
		}

		r = pfi_set_rpm (*img, rpm);
	}
	else if (strcmp (op, "set-rpm-mac") == 0) {
		r = pfi_set_rpm_mac_500 (*img);
	}
	else if (strcmp (op, "set-rpm-mac-490") == 0) {
		r = pfi_set_rpm_mac_490 (*img);
	}
	else if (strcmp (op, "set-rpm-mac-500") == 0) {
		r = pfi_set_rpm_mac_500 (*img);
	}
	else if (strcmp (op, "set-clock") == 0) {
		unsigned long clk;

		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		if (pfi_parse_rate (optarg1[0], &clk)) {
			return (1);
		}

		r = pfi_set_clock (*img, clk);
	}
	else if (strcmp (op, "shift-index") == 0) {
		long ofs;

		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		if (pfi_parse_long (optarg1[0], &ofs)) {
			return (1);
		}

		r = pfi_shift_index (*img, ofs);
	}
	else if (strcmp (op, "shift-index-us") == 0) {
		long us;

		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		if (pfi_parse_long (optarg1[0], &us)) {
			return (1);
		}

		r = pfi_shift_index_us (*img, us);
	}
	else if (strcmp (op, "slack") == 0) {
		if (pce_getopt (argc, argv, &optarg1, NULL) != 0) {
			return (1);
		}

		r = pfi_slack (*img, optarg1[0]);
	}
	else if (strcmp (op, "wpcom") == 0) {
		r = pfi_wpcom (*img);
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
pfi_img_t *pfi_load_image (const char *fname)
{
	pfi_img_t *img;

	if (par_verbose) {
		fprintf (stderr, "%s: load image from %s\n", arg0, fname);
	}

	if (strcmp (fname, "-") == 0) {
		img = pfi_img_load_fp (stdin, par_fmt_inp);
	}
	else {
		img = pfi_img_load (fname, par_fmt_inp);
	}

	if (img == NULL) {
		fprintf (stderr, "%s: loading failed (%s)\n", arg0, fname);
		return (NULL);
	}

	if (par_list) {
		par_list = 0;
		pfi_list_tracks (img, par_list_long);
	}

	if (par_print_info) {
		par_print_info = 0;
		pfi_print_info (img);
	}

	return (img);
}

static
int pfi_save_image (const char *fname, pfi_img_t **img)
{
	int r;

	if (*img == NULL) {
		*img = pfi_img_new();
	}

	if (*img == NULL) {
		return (1);
	}

	if (par_verbose) {
		fprintf (stderr, "%s: save image to %s\n", arg0, fname);
	}

	if (strcmp (fname, "-") == 0) {
		r = pfi_img_save_fp (stdout, *img, par_fmt_out);
	}
	else {
		r = pfi_img_save (fname, *img, par_fmt_out);
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
int pfi_set_parameter (const char *name, const char *val)
{
	if (strcmp (name, "clock-tolerance") == 0) {
		if (pfi_parse_ulong (val, &par_clock_tolerance)) {
			return (1);
		}
	}
	else if (strcmp (name, "fold-max") == 0) {
		if (pfi_parse_ulong (val, &par_fold_max)) {
			return (1);
		}
	}
	else if (strcmp (name, "fold-mode") == 0) {
		if (strcmp (val, "none") == 0) {
			par_fold_mode = PFI_FOLD_NONE;
		}
		else if (strcmp (val, "maxrun") == 0) {
			par_fold_mode = PFI_FOLD_MAXRUN;
		}
		else if (strcmp (val, "mindiff") == 0) {
			par_fold_mode = PFI_FOLD_MINDIFF;
		}
		else {
			return (1);
		}
	}
	else if (strcmp (name, "fold-window") == 0) {
		if (pfi_parse_uint (val, &par_fold_window)) {
			return (1);
		}
	}
	else if (strcmp (name, "pfi-clock") == 0) {
		if (pfi_parse_rate (val, &par_pfi_clock)) {
			return (1);
		}
	}
	else if (strcmp (name, "pfi-fold-compare") == 0) {
		if (pfi_parse_ulong (val, &par_pfi_fold_compare)) {
			return (1);
		}
	}
	else if (strcmp (name, "pfi-fold-revolution") == 0) {
		if (pfi_parse_uint (val, &par_pfi_fold_revolution)) {
			return (1);
		}
	}
	else if (strcmp (name, "pfi-fold-right") == 0) {
		if (pfi_parse_bool (val, &par_pfi_fold_right)) {
			return (1);
		}
	}
	else if (strcmp (name, "pfi-fold-window") == 0) {
		if (pfi_parse_ulong (val, &par_pfi_fold_window)) {
			return (1);
		}
	}
	else if (strcmp (name, "slack") == 0) {
		if (pfi_parse_time (val, &par_slack1, 0.001)) {
			return (1);
		}

		par_slack2 = par_slack1;
	}
	else if (strcmp (name, "slack1") == 0) {
		if (pfi_parse_time (val, &par_slack1, 0.001)) {
			return (1);
		}
	}
	else if (strcmp (name, "slack2") == 0) {
		if (pfi_parse_time (val, &par_slack2, 0.001)) {
			return (1);
		}
	}
	else if (strcmp (name, "weak-bits") == 0) {
		if (pfi_parse_bool (val, &par_weak_bits)) {
			return (1);
		}
	}
	else if (strcmp (name, "weak-bits-margin") == 0) {
		if (pfi_parse_ulong (val, &par_weak_i1)) {
			return (1);
		}

		par_weak_i2 = par_weak_i1;
	}
	else if (strcmp (name, "weak-bits-i1") == 0) {
		if (pfi_parse_ulong (val, &par_weak_i1)) {
			return (1);
		}
	}
	else if (strcmp (name, "weak-bits-i2") == 0) {
		if (pfi_parse_ulong (val, &par_weak_i2)) {
			return (1);
		}
	}
	else {
		return (1);
	}

	return (0);
}

static
int pfi_set_format (const char *name, unsigned *val)
{
	if (strcmp (name, "pfi") == 0) {
		*val = PFI_FORMAT_PFI;
	}
	else if (strcmp (name, "raw") == 0) {
		*val = PFI_FORMAT_KRYOFLUX;
	}
	else if (strcmp (name, "scp") == 0) {
		*val = PFI_FORMAT_SCP;
	}
	else {
		fprintf (stderr, "%s: unknown format (%s)\n", arg0, name);
		*val = PFI_FORMAT_NONE;
		return (1);
	}

	return (0);
}

int main (int argc, char **argv)
{
	int         r;
	char        **optarg;
	pfi_img_t   *img;
	const char  *out;

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

		case 'c':
			if (pfi_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all)) {
				return (1);
			}
			break;

		case 'f':
			if (img != NULL) {
				pfi_print_info (img);
			}
			else {
				par_print_info = 1;
			}
			break;

		case 'h':
			if (pfi_parse_range (optarg[0], &par_trk[0], &par_trk[1], &par_trk_all)) {
				return (1);
			}
			break;

		case 'i':
			if (img != NULL) {
				pfi_img_del (img);
			}

			img = pfi_load_image (optarg[0]);

			if (img == NULL) {
				return (1);
			}
			break;

		case 'I':
			if (pfi_set_format (optarg[0], &par_fmt_inp)) {
				return (1);
			}
			break;

		case 'l':
			if (img != NULL) {
				pfi_list_tracks (img, 0);
			}
			else {
				par_list = 1;
				par_list_long = 0;
			}
			break;

		case 'L':
			if (img != NULL) {
				pfi_list_tracks (img, 1);
			}
			else {
				par_list = 1;
				par_list_long = 1;
			}
			break;

		case 'o':
			if (pfi_save_image (optarg[0], &img)) {
				return (1);
			}
			break;

		case 'O':
			if (pfi_set_format (optarg[0], &par_fmt_out)) {
				return (1);
			}
			break;

		case 'p':
			if (pfi_operation (&img, optarg[0], argc, argv)) {
				return (1);
			}
			break;

		case 'r':
			if (pfi_parse_rate (optarg[0], &par_data_rate)) {
				return (1);
			}
			break;

		case 'R':
			if (pfi_parse_uint (optarg[0], &par_revolution)) {
				return (1);
			}
			break;

		case 's':
			if (pfi_set_parameter (optarg[0], optarg[1])) {
				fprintf (stderr, "%s: bad parameter (%s=%s)\n",
					arg0, optarg[0], optarg[1]
				);
				return (1);
			}
			break;

		case 't':
			if (pfi_parse_range (optarg[0], &par_cyl[0], &par_cyl[1], &par_cyl_all)) {
				return (1);
			}
			if (pfi_parse_range (optarg[1], &par_trk[0], &par_trk[1], &par_trk_all)) {
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
			par_cyl_all = 1;
			par_cyl_inv = 0;
			par_trk_all = 1;
			par_trk_inv = 0;
			break;

		case 0:
			if (img == NULL) {
				img = pfi_load_image (optarg[0]);

				if (img == NULL) {
					return (1);
				}
			}
			else {
				if (pfi_save_image (optarg[0], &img)) {
					return (1);
				}
			}
			break;

		default:
			return (1);
		}
	}

	if (out != NULL) {
		if (img == NULL) {
			img = pfi_img_new();
		}

		if (img == NULL) {
			return (1);
		}

		if (par_verbose) {
			fprintf (stderr, "%s: save image to %s\n", arg0, out);
		}

		r = pfi_img_save (out, img, par_fmt_out);

		if (r) {
			fprintf (stderr, "%s: saving failed (%s)\n",
				argv[0], out
			);
			return (1);
		}
	}

	return (0);
}
