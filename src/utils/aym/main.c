/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/aym/main.c                                         *
 * Created:     2015-05-21 by Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/getopt.h>

#include <drivers/sound/sound.h>


const char           *arg0 = NULL;

char                 par_verbose = 0;

unsigned long        par_srate = 48000;

static unsigned long par_lowpass = 0;
static char          par_highpass = 0;

static const char    *par_driver = "oss:lowpass=0";
static char          par_driver_tmp[256];

static unsigned long par_mark = 0;


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'd', 0, "decode", NULL, "Decode AYM to text" },
	{ 'e', 0, "encode", NULL, "Encode text to AYM" },
	{ 'h', 0, "highpass", NULL, "Enable the highpass filter [no]" },
	{ 'i', 1, "input", "filename", "Set the input file name" },
	{ 'l', 1, "lowpass", "freq", "Set lowpass frequency [0]" },
	{ 'm', 1, "mark", "pos", "Mark a file position when decoding" },
	{ 'o', 1, "output", "filename", "Set the output file name" },
	{ 'p', 0, "play", NULL, "Play an AYM file" },
	{ 'r', 1, "srate", "rate", "Set the sample rate [48000]" },
	{ 's', 1, "driver", "driver", "Set the sound driver [oss]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{ 'w', 1, "wav", "file", "Save to a WAV file" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"aym: encode and decode PCE AYM files",
		"usage: aym [options] [input [output]]",
		opts
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"aym version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2015 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

unsigned long aym_get_uint32_be (const void *buf, unsigned i)
{
	unsigned long       val;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + i;

	val = tmp[0] & 0xff;
	val = (val << 8) | (tmp[1] & 0xff);
	val = (val << 8) | (tmp[2] & 0xff);
	val = (val << 8) | (tmp[3] & 0xff);

	return (val);
}

void aym_set_uint32_be (void *buf, unsigned i, unsigned long val)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = (val >> 24) & 0xff;
	tmp[1] = (val >> 16) & 0xff;
	tmp[2] = (val >> 8) & 0xff;
	tmp[3] = val;
}

int main (int argc, char **argv)
{
	int        r, op;
	char       **optarg;
	const char *inp, *out;

	if (argc < 2) {
		print_help();
		return (1);
	}

	arg0 = argv[0];

	op = 0;

	inp = NULL;
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

		case 'd':
			op = 0;
			break;

		case 'e':
			op = 1;
			break;

		case 'h':
			par_highpass = 1;
			break;

		case 'i':
			inp = optarg[0];
			break;

		case 'l':
			par_lowpass = strtoul (optarg[0], NULL, 0);
			break;

		case 'm':
			par_mark = strtoul (optarg[0], NULL, 0);
			break;

		case 'o':
			out = optarg[0];
			break;

		case 'p':
			op = 2;
			break;

		case 'r':
			par_srate = strtoul (optarg[0], NULL, 0);
			break;

		case 's':
			par_driver = optarg[0];
			break;

		case 'w':
			op = 2;
			sprintf (par_driver_tmp, "wav:wav=%s:wavfilter=0", optarg[0]);
			par_driver = par_driver_tmp;
			break;

		case 0:
			if (inp == NULL) {
				inp = optarg[0];
			}
			else if (out == NULL) {
				out = optarg[0];
			}
			else {
				fprintf (stderr, "%s: too many files (%s)\n", arg0, optarg[0]);
				return (1);
			}
			break;

		default:
			return (1);
		}
	}

	if (op == 0) {
		r = aym_decode (inp, out, par_mark);
	}
	else if (op == 1) {
		r = aym_encode (inp, out);
	}
	else {
		r = aym_play (inp, par_driver, par_lowpass, par_highpass);
	}

	if (r) {
		return (1);
	}

	return (0);
}
