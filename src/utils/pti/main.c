/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pti/main.c                                         *
 * Created:     2020-04-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2023 Hampa Hug <hampa@hampa.ch>                     *
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
#include "comment.h"
#include "ops.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/getopt.h>

#include <drivers/pti/pti.h>
#include <drivers/pti/pti-io.h>


int pti_info (pti_img_t *img);


const char    *arg0 = NULL;

int           par_verbose = 0;

int           par_print_info = 0;

unsigned      par_fmt_inp = PTI_FORMAT_NONE;
unsigned      par_fmt_out = PTI_FORMAT_NONE;

unsigned long par_default_clock = PTI_CLOCK_C64_PAL;


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'c', 1, "default-clock","clock", "Set the default clock [c64-pal]" },
	{ 'f', 0, "info", NULL, "Print image information [no]" },
	{ 'i', 1, "input", "filename", "Load an input file" },
	{ 'I', 1, "input-format", "format", "Set the input format [auto]" },
	{ 'm', 1, "cat", "filename", "Concatenate an image" },
	{ 'o', 1, "output", "filename", "Set the output file name [none]" },
	{ 'O', 1, "output-format", "format", "Set the output format [auto]" },
	{ 'p', 1, "operation", "name [...]", "Perform an operation" },
	{ 's', 2, "set", "par val", "Set a parameter value" },
	{ 'v', 0, "verbose", NULL, "Verbose operation [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pti: convert and modify PCE Tape Image files",
		"usage: pti [options] [input] [options] [output]",
		opts
	);

	fputs (
		"\n"
		"operations are:\n"
		"  cat <file>               Add an image to the end of the current image\n"
		"  comment-add text         Add to the image comment\n"
		"  comment-load filename    Load the image comment from a file\n"
		"  comment-print            Print the image comment\n"
		"  comment-save filename    Save the image comment to a file\n"
		"  comment-set text         Set the image comment\n"
		"  duty-cycle <low> <high>  Adjust the duty cycles\n"
		"  decode <type> <file>     Decode tracks\n"
		"  encode <type> <file>     Encode a file\n"
		"  fix-clock <clock>        Set the sample clock without modifying the image\n"
		"  info                     Print image information\n"
		"  invert                   Invert the amplitude\n"
		"  new                      Create a new image\n"
		"  scale <factor>           Scale the image by a factor\n"
		"  set-clock <clock>        Set the sample clock rate\n"
		"  space-add-left <time>    Add space at the beginning\n"
		"  space-add-right <time>   Add space at the end\n"
		"  space-add <time>         Add space at the beginning and end\n"
		"  space-auto <mintime>     Convert constant levels to silence\n"
		"  space-max <time>         Limit the length of silence periods\n"
		"  space-trim-left          Remove space from the beginning\n"
		"  space-trim-right         Remove space from the end\n"
		"  space-trim               Remove space from the beginning and end\n"
		"  trim-left <time>         Cut time from the beginning\n"
		"  trim-right <time>        Cut time from the end\n"
		"\n"
		"sample clocks are:\n"
		"  c16-ntsc, c16-pal, c64-ntsc, c64-pal, pc-pic, pc-cpu, pet,\n"
		"  vic20-ntsc, vic20-pal, <int>\n"
		"\n"
		"parameters are:\n"
		"  default-clock, pcm-srate,\n"
		"  wav-bits, wav-lowpass, wav-lowpass-order, wav-srate\n"
		"\n"
		"decode types are:\n"
		"  cbm-t64, cbm-raw\n"
		"\n"
		"encode types are:\n"
		"  cbm-bas, cbm-prg\n"
		"\n"
		"file formats are:\n"
		"  cas, pcm, pti, t64(r), tap, txt, wav\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pti version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2020-" PCE_YEAR " Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

int pti_parse_double (const char *str, double *val)
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

int pti_parse_long (const char *str, long *val)
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

int pti_parse_ulong (const char *str, unsigned long *val)
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

int pti_parse_uint (const char *str, unsigned *val)
{
	unsigned long tmp;

	if (pti_parse_ulong (str, &tmp)) {
		return (1);
	}

	*val = tmp;

	return (0);
}

int pti_parse_bool (const char *str, int *val)
{
	unsigned long tmp;

	if (pti_parse_ulong (str, &tmp)) {
		return (1);
	}

	*val = tmp != 0;

	return (0);
}

static
int pti_parse_clock (const char *str, unsigned long *val)
{
	if (strcmp (str, "pet") == 0) {
		*val = PTI_CLOCK_PET;
	}
	else if (strcmp (str, "vic20-ntsc") == 0) {
		*val = PTI_CLOCK_VIC20_NTSC;
	}
	else if (strcmp (str, "vic20-pal") == 0) {
		*val = PTI_CLOCK_VIC20_PAL;
	}
	else if (strcmp (str, "c64-ntsc") == 0) {
		*val = PTI_CLOCK_C64_NTSC;
	}
	else if (strcmp (str, "c64-pal") == 0) {
		*val = PTI_CLOCK_C64_PAL;
	}
	else if (strcmp (str, "c16-ntsc") == 0) {
		*val = PTI_CLOCK_C16_NTSC;
	}
	else if (strcmp (str, "c16-pal") == 0) {
		*val = PTI_CLOCK_C16_PAL;
	}
	else if ((strcmp (str, "pc-pit") == 0) || (strcmp (str, "pc") == 0)) {
		*val = PTI_CLOCK_PC_PIT;
	}
	else if (strcmp (str, "pc-cpu") == 0) {
		*val = PTI_CLOCK_PC_CPU;
	}
	else {
		return (1);
	}

	return (0);
}

int pti_parse_freq (const char *str, unsigned long *val)
{
	char *end;

	if ((str == NULL) || (*str == 0)) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	if (pti_parse_clock (str, val) == 0) {
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

int pti_parse_time_sec (const char *str, double *sec)
{
	char *end;

	if ((str == NULL) || (*str == 0)) {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	*sec = strtod (str, &end);

	if ((end == NULL) || (*end == 0)) {
		return (0);
	}

	if (strcmp (end, "s") == 0) {
		;
	}
	else if (strcmp (end, "ms") == 0) {
		*sec /= 1000.0;
	}
	else if (strcmp (end, "us") == 0) {
		*sec /= 1000000.0;
	}
	else {
		fprintf (stderr, "%s: bad value (%s)\n", arg0, str);
		return (1);
	}

	return (0);
}

int pti_parse_time_clk (const char *str, unsigned long *sec, unsigned long *clk, unsigned long clock)
{
	double tmp;

	if (pti_parse_time_sec (str, &tmp)) {
		return (1);
	}

	pti_time_set (sec, clk, tmp, clock);

	return (0);
}

static
int pti_set_format (const char *name, unsigned *val)
{
	if (strcmp (name, "cas") == 0) {
		*val = PTI_FORMAT_CAS;
	}
	else if (strcmp (name, "pcm") == 0) {
		*val = PTI_FORMAT_PCM;
	}
	else if (strcmp (name, "pti") == 0) {
		*val = PTI_FORMAT_PTI;
	}
	else if (strcmp (name, "tap") == 0) {
		*val = PTI_FORMAT_TAP;
	}
	else if (strcmp (name, "txt") == 0) {
		*val = PTI_FORMAT_TXT;
	}
	else if (strcmp (name, "wav") == 0) {
		*val = PTI_FORMAT_WAV;
	}
	else {
		fprintf (stderr, "%s: unknown format (%s)\n", arg0, name);
		*val = PTI_FORMAT_NONE;
		return (1);
	}

	return (0);
}

pti_img_t *pti_load_image (const char *fname)
{
	pti_img_t *img;

	if (par_verbose) {
		fprintf (stderr, "%s: load image from %s\n", arg0, fname);
	}

	if (strcmp (fname, "-") == 0) {
		img = pti_img_load_fp (stdin, par_fmt_inp);
	}
	else {
		img = pti_img_load (fname, par_fmt_inp);
	}

	if (img == NULL) {
		fprintf (stderr, "%s: loading failed (%s)\n", arg0, fname);
		return (NULL);
	}

	if (par_print_info) {
		par_print_info = 0;
		pti_info (img);
	}

	return (img);
}

int pti_save_image (const char *fname, pti_img_t **img)
{
	int r;

	if (*img == NULL) {
		*img = pti_img_new();
	}

	if (*img == NULL) {
		return (1);
	}

	if (par_verbose) {
		fprintf (stderr, "%s: save image to %s\n", arg0, fname);
	}

	if (strcmp (fname, "-") == 0) {
		r = pti_img_save_fp (stdout, *img, par_fmt_out);
	}
	else {
		r = pti_img_save (fname, *img, par_fmt_out);
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
int pti_set_parameter (const char *name, const char *val)
{
	if (strcmp (name, "default-clock") == 0) {
		if (pti_parse_freq (val, &par_default_clock)) {
			return (1);
		}

		pti_set_default_clock (par_default_clock);
	}
	else if (strcmp (name, "pcm-srate") == 0) {
		unsigned long srate;

		if (pti_parse_freq (val, &srate)) {
			return (1);
		}

		pti_pcm_set_default_srate (srate);
	}
	else if (strcmp (name, "wav-bits") == 0) {
		unsigned bits;

		if (pti_parse_uint (val, &bits)) {
			return (1);
		}

		pti_wav_set_bits (bits);
	}
	else if (strcmp (name, "wav-lowpass") == 0) {
		unsigned long cutoff;

		if (pti_parse_ulong (val, &cutoff)) {
			return (1);
		}

		pti_wav_set_lowpass (cutoff);
	}
	else if (strcmp (name, "wav-lowpass-order") == 0) {
		unsigned order;

		if (pti_parse_uint (val, &order)) {
			return (1);
		}

		pti_wav_set_lowpass_order (order);
	}
	else if (strcmp (name, "wav-srate") == 0) {
		unsigned long srate;

		if (pti_parse_freq (val, &srate)) {
			return (1);
		}

		pti_wav_set_srate (srate);
	}
	else if ((strcmp (name, "wav-minimum-volume") == 0) || (strcmp (name, "wav-vol") == 0)) {
		unsigned vol;

		if (pti_parse_uint (val, &vol)) {
			return (1);
		}

		pti_wav_set_min_volume (vol);
	}
	else {
		return (1);
	}

	return (0);
}

int main (int argc, char **argv)
{
	int         r;
	char        **optarg;
	pti_img_t   *img;
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
			if (pti_set_parameter ("default-clock", optarg[0])) {
				return (1);
			}
			break;

		case 'f':
			if (img != NULL) {
				pti_info (img);
			}
			else {
				par_print_info = 1;
			}
			break;

		case 'i':
			if (img != NULL) {
				pti_img_del (img);
			}

			if ((img = pti_load_image (optarg[0])) == NULL) {
				return (1);
			}
			break;

		case 'I':
			if (pti_set_format (optarg[0], &par_fmt_inp)) {
				return (1);
			}
			break;

		case 'm':
			if (pti_cat (&img, optarg[0])) {
				return (1);
			}
			break;

		case 'o':
			if (pti_save_image (optarg[0], &img)) {
				return (1);
			}
			break;

		case 'O':
			if (pti_set_format (optarg[0], &par_fmt_out)) {
				return (1);
			}
			break;

		case 'p':
			if (pti_operation (&img, optarg[0], argc, argv)) {
				return (1);
			}
			break;

		case 's':
			if (pti_set_parameter (optarg[0], optarg[1])) {
				fprintf (stderr, "%s: bad parameter (%s=%s)\n",
					arg0, optarg[0], optarg[1]
				);
				return (1);
			}
			break;

		case 'v':
			par_verbose = 1;
			break;

		case 0:
			if (img == NULL) {
				if ((img = pti_load_image (optarg[0])) == NULL) {
					return (1);
				}
			}
			else {
				if (pti_save_image (optarg[0], &img)) {
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
			img = pti_img_new();
		}

		if (img == NULL) {
			return (1);
		}

		if (par_verbose) {
			fprintf (stderr, "%s: save image to %s\n", arg0, out);
		}

		r = pti_img_save (out, img, par_fmt_out);

		if (r) {
			fprintf (stderr, "%s: saving failed (%s)\n",
				argv[0], out
			);
			return (1);
		}
	}

	return (0);
}
