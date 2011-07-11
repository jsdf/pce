/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pceimg/pce-img.c                                   *
 * Created:     2005-11-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2011 Hampa Hug <hampa@hampa.ch>                     *
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

#include "pce-img.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <drivers/block/block.h>
#include <drivers/block/blkcow.h>
#include <drivers/block/blkfdc.h>
#include <drivers/block/blkraw.h>
#include <drivers/block/blkpce.h>
#include <drivers/block/blkqed.h>
#include <drivers/block/blkdosem.h>

#include <lib/getopt.h>


#define DSK_NONE   0
#define DSK_RAW    1
#define DSK_PCE    2
#define DSK_DOSEMU 3
#define DSK_PFDC   4
#define DSK_QED    5


static const char    *argv0 = NULL;

static int           par_quiet = 0;

static unsigned long par_min_cluster_size = 0;


static pce_option_t opts_main[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{  -1, 0, NULL, NULL, NULL }
};

static pce_option_t opts_create[] = {
	{ 'c', 1, "cylinders", "int", "Set the number of cylinders [0]" },
	{ 'C', 1, "min-cluster-size", "int", "Set the minimum cluster size for QED [0]" },
	{ 'f', 1, "offset", "int", "Set the data offset [0]" },
	{ 'g', 3, "geometry", "3*int", "Set the disk geometry (c h s)" },
	{ 'h', 1, "heads", "int", "Set the number of heads [0]" },
	{ 'm', 1, "megabytes", "int", "Set the disk size in megabytes [0]" },
	{ 'n', 1, "size", "int", "Set the disk size in 512 byte blocks [0]" },
	{ 'o', 1, "output", "string", "Set the output file name [stdout]" },
	{ 'q', 0, "quiet", NULL, "Be quiet [no]" },
	{ 's', 1, "sectors", "int", "Set the number of sectors per track [0]" },
	{  -1, 0, NULL, NULL, NULL }
};

static pce_option_t opts_convert[] = {
	{ 'c', 1, "cylinders", "int", "Set the number of cylinders [0]" },
	{ 'C', 1, "min-cluster-size", "int", "Set the minimum cluster size for QED [0]" },
	{ 'f', 1, "offset", "int", "Set the data offset [0]" },
	{ 'g', 3, "geometry", "3*int", "Set the disk geometry (c h s)" },
	{ 'h', 1, "heads", "int", "Set the number of heads [0]" },
	{ 'i', 1, "input", "string", "Set the input file name [stdin]" },
	{ 'm', 1, "megabytes", "int", "Set the disk size in megabytes [0]" },
	{ 'n', 1, "size", "int", "Set the disk size in 512 byte blocks [0]" },
	{ 'o', 1, "output", "string", "Set the output file name [stdout]" },
	{ 'q', 0, "quiet", NULL, "Be quiet [no]" },
	{ 's', 1, "sectors", "int", "Set the number of sectors per track [0]" },
	{ 'w', 1, "cow", "string", "Set the COW file name [none]" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pce-img: PCE disk image utility",
		"usage: pce-img [options] command [options]",
		opts_main
	);

	fputs ("\n", stdout);

	pce_getopt_help (
		NULL,
		"usage: pce-img create [options] [output]",
		opts_create
	);

	fputs ("\n", stdout);

	pce_getopt_help (
		NULL,
		"usage: pce-img convert [options] [input [output]]",
		opts_convert
	);

	fputs (
		"\nfile names: <format>:<name>\n"
		"formats:    raw, pce, dosemu, pfdc, qed\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pce-img version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2005-2011 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}


/*
 * Guess the image type based on the file name extension.
 */
static
unsigned pce_get_disk_type_ext (const char *str)
{
	const char *ext;

	ext = NULL;

	while (*str != 0) {
		if (*str == '.') {
			ext = str + 1;
		}

		str += 1;
	}

	if (ext == NULL) {
		return (DSK_NONE);
	}

	if (strcasecmp (ext, "pfdc") == 0) {
		return (DSK_PFDC);
	}
	else if (strcasecmp (ext, "qed") == 0) {
		return (DSK_QED);
	}
	else if (strcasecmp (ext, "raw") == 0) {
		return (DSK_RAW);
	}

	return (DSK_NONE);
}

static
unsigned pce_get_disk_type (const char *str)
{
	unsigned i;
	char     buf[256];

	i = 0;
	while (str[i] != ':') {
		if (str[i] == 0) {
			return (pce_get_disk_type_ext (str));
		}

		if (i >= 256) {
			return (DSK_NONE);
		}

		buf[i] = str[i];

		i += 1;
	}

	buf[i] = 0;

	if (strcmp (buf, "raw") == 0) {
		return (DSK_RAW);
	}

	if (strcmp (buf, "pce") == 0) {
		return (DSK_PCE);
	}

	if (strcmp (buf, "qed") == 0) {
		return (DSK_QED);
	}

	if (strcmp (buf, "dosemu") == 0) {
		return (DSK_DOSEMU);
	}

	if (strcmp (buf, "pfdc") == 0) {
		return (DSK_PFDC);
	}

	return (DSK_NONE);
}

static
const char *pce_get_disk_name (const char *str)
{
	unsigned i;

	i = 0;
	while (str[i] != ':') {
		if (str[i] == 0) {
			return (str);
		}

		i += 1;
	}

	return (str + i + 1);
}

static
int pce_block_is_null (const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tmp;

	tmp = buf;

	for (i = 0; i < cnt; i++) {
		if (tmp[i] != 0) {
			return (0);
		}
	}

	return (1);
}

static
int dsk_create (const char *str, uint32_t n, uint32_t c, uint32_t h, uint32_t s,
	uint64_t ofs)
{
	unsigned   type;
	const char *name;

	type = pce_get_disk_type (str);
	name = pce_get_disk_name (str);

	switch (type) {
	case DSK_RAW:
		return (dsk_img_create (name, n, ofs));

	case DSK_PCE:
		return (dsk_pce_create (name, n, c, h, s, ofs & 0xffffffff));

	case DSK_QED:
		return (dsk_qed_create (name, n, par_min_cluster_size));

	case DSK_DOSEMU:
		return (dsk_dosemu_create (name, c, h, s, ofs & 0xffffffff));

	case DSK_PFDC:
		return (1);
	}

	return (dsk_pce_create (name, n, c, h, s, ofs & 0xffffffff));
}

static
disk_t *dsk_open (const char *str, uint32_t n, uint32_t c, uint32_t h, uint32_t s,
	uint64_t ofs, int ro)
{
	unsigned   type;
	const char *name;

	type = pce_get_disk_type (str);
	name = pce_get_disk_name (str);

	switch (type) {
	case DSK_RAW:
		return (dsk_img_open (name, ofs, ro));

	case DSK_PCE:
		return (dsk_pce_open (name, ro));

	case DSK_QED:
		return (dsk_qed_open (name, ro));

	case DSK_DOSEMU:
		return (dsk_dosemu_open (name, ro));

	case DSK_PFDC:
		return (dsk_fdc_open_pfdc (name, ro));
	}

	return (dsk_auto_open (name, ofs, ro));
}

static
int dsk_copy (disk_t *dst, disk_t *src)
{
	uint32_t      i, n, m;
	uint32_t      prg_i, prg_n;
	unsigned      k;
	uint16_t      msk;
	unsigned char buf[8192];

	n = dsk_get_block_cnt (dst);
	m = dsk_get_block_cnt (src);

	if (m < n) {
		n = m;
	}

	prg_i = 0;
	prg_n = n;

	i = 0;
	while (n > 0) {
		m = (n < 16) ? n : 16;

		if (dsk_read_lba (src, buf, i, m)) {
			return (1);
		}

		msk = 0;
		for (k = 0; k < m; k++) {
			if (pce_block_is_null (buf + 512 * k, 512) == 0) {
				msk |= 1U << k;
			}
		}

		if (msk == 0xffff) {
			if (dsk_write_lba (dst, buf, i, m)) {
				return (1);
			}
		}
		else if (msk != 0) {
			for (k = 0; k < m; k++) {
				if (msk & (1U << k)) {
					if (dsk_write_lba (dst, buf + 512 * k, i + k, 1)) {
						return (1);
					}
				}
			}
		}

		i += m;
		n -= m;

		if (par_quiet == 0) {
			prg_i += m;
			if (prg_i >= 4096) {
				fprintf (stdout, "[%6.2f%%] block %lu of %lu\r",
					(100.0 * (i + 1)) / prg_n,
					(unsigned long) i,
					(unsigned long) prg_n
				);

				prg_i = 0;

				fflush (stdout);
			}
		}
	}

	if (par_quiet == 0) {
		fprintf (stdout, "[%6.2f%%] block %lu of %lu\n",
			100.0, (unsigned long) prg_n, (unsigned long) prg_n
		);
	}

	return (0);
}

static
int main_create (int argc, char **argv)
{
	int      r;
	char     **optarg;
	char     *par_out = NULL;
	uint32_t par_c = 0;
	uint32_t par_h = 0;
	uint32_t par_s = 0;
	uint32_t par_n = 0;
	uint64_t par_ofs = 0;

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts_convert);

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
			par_c = strtoul (optarg[0], NULL, 0);
			break;

		case 'C':
			par_min_cluster_size = strtoul (optarg[0], NULL, 0);
			break;

		case 'f':
			par_ofs = strtoull (optarg[0], NULL, 0);
			break;

		case 'g':
			par_c = strtoul (optarg[0], NULL, 0);
			par_h = strtoul (optarg[1], NULL, 0);
			par_s = strtoul (optarg[2], NULL, 0);
			break;

		case 'h':
			par_h = strtoul (optarg[0], NULL, 0);
			break;

		case 'm':
			par_n = strtoul (optarg[0], NULL, 0);
			par_n *= 2048;
			break;

		case 'n':
			par_n = strtoul (optarg[0], NULL, 0);
			break;

		case 'o':
			par_out = optarg[0];
			break;

		case 'q':
			par_quiet = 1;
			break;

		case 's':
			par_s = strtoul (optarg[0], NULL, 0);
			break;

		case 0:
			if (par_out == NULL) {
				par_out = optarg[0];
			}
			else {
				fprintf (stderr, "%s: too many file names (%s)\n",
					argv0, optarg[0]
				);
				return (1);
			}
			break;

		default:
			return (1);
		}
	}

	dsk_adjust_chs (&par_n, &par_c, &par_h, &par_s);

	if (par_quiet == 0) {
		fprintf (stdout, "geometry: %lu/%lu/%lu (%luM at %llu)\n",
			(unsigned long) par_c,
			(unsigned long) par_h,
			(unsigned long) par_s,
			(unsigned long) (par_n / (2 * 1024)),
			(unsigned long long) par_ofs
		);
	}

	if (par_out == NULL) {
		fprintf (stderr, "%s: need a file name\n", argv0);
		return (1);
	}

	if (dsk_create (par_out, par_n, par_c, par_h, par_s, par_ofs)) {
		fprintf (stderr, "%s: create failed (%s)\n", argv0, par_out);
		return (1);
	}

	return (1);
}

static
int main_convert (int argc, char **argv)
{
	int        r;
	char       **optarg;
	const char *par_inp = NULL;
	const char *par_out = NULL;
	const char *par_cow = NULL;
	uint32_t   par_c = 0;
	uint32_t   par_h = 0;
	uint32_t   par_s = 0;
	uint32_t   par_n = 0;
	uint64_t   par_ofs = 0;
	disk_t     *inp, *out, *cow;

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts_convert);

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
			par_c = strtoul (optarg[0], NULL, 0);
			break;

		case 'C':
			par_min_cluster_size = strtoul (optarg[0], NULL, 0);
			break;

		case 'f':
			par_ofs = strtoull (optarg[0], NULL, 0);
			break;

		case 'g':
			par_c = strtoul (optarg[0], NULL, 0);
			par_h = strtoul (optarg[1], NULL, 0);
			par_s = strtoul (optarg[2], NULL, 0);
			break;

		case 'h':
			par_h = strtoul (optarg[0], NULL, 0);
			break;

		case 'i':
			par_inp = optarg[0];
			break;

		case 'm':
			par_n = strtoul (optarg[0], NULL, 0);
			par_n *= 2048;
			break;

		case 'n':
			par_n = strtoul (optarg[0], NULL, 0);
			break;

		case 'o':
			par_out = optarg[0];
			break;

		case 'q':
			par_quiet = 1;
			break;

		case 's':
			par_s = strtoul (optarg[0], NULL, 0);
			break;

		case 'w':
			par_cow = optarg[0];
			break;

		case 0:
			if (par_inp == NULL) {
				par_inp = optarg[0];
			}
			else if (par_out == NULL) {
				par_out = optarg[0];
			}
			else {
				fprintf (stderr, "%s: too many file names (%s)\n",
					argv0, optarg[0]
				);
				return (1);
			}
			break;

		default:
			return (1);
		}
	}

	if (par_inp == NULL) {
		fprintf (stderr, "%s: need an input file name\n", argv0);
		return (1);
	}

	if (par_out == NULL) {
		fprintf (stderr, "%s: need an output file name\n", argv0);
		return (1);
	}

	inp = dsk_open (par_inp, par_n, par_c, par_h, par_s, par_ofs, 1);
	if (inp == NULL) {
		fprintf (stderr, "%s: can't open input file (%s)\n",
			argv0, par_inp
		);
		return (1);
	}

	if (par_cow != NULL) {
		cow = inp;

		inp = dsk_qed_cow_new (cow, par_cow);

		if (inp == NULL) {
			inp = dsk_cow_new (cow, par_cow);
		}

		if (inp == NULL) {
			dsk_del (cow);
			fprintf (stderr, "%s: can't open COW file (%s)\n",
				argv0, par_cow
			);
			return (1);
		}
	}

	/* geometry might be different in file header */
	par_c = inp->c;
	par_h = inp->h;
	par_s = inp->s;
	par_n = inp->blocks;

	if (par_quiet == 0) {
		fprintf (stdout, "geometry: %lu/%lu/%lu (%luM at %llu)\n",
			(unsigned long) par_c,
			(unsigned long) par_h,
			(unsigned long) par_s,
			(unsigned long) (par_n / (2 * 1024)),
			(unsigned long long) par_ofs
		);
	}

	if (dsk_create (par_out, par_n, par_c, par_h, par_s, par_ofs)) {
		fprintf (stderr, "%s: can't create output file (%s)\n",
			argv0, par_out
		);
		return (1);
	}

	out = dsk_open (par_out, par_n, par_c, par_h, par_s, par_ofs, 0);
	if (out == NULL) {
		fprintf (stderr, "%s: can't open output file (%s)\n",
			argv0, par_out
		);
		return (1);
	}

	if (dsk_copy (out, inp)) {
		fprintf (stderr, "%s: copy failed\n", argv0);
		return (1);
	}

	dsk_del (out);
	dsk_del (inp);

	return (0);
}

int main (int argc, char **argv)
{
	int  r;
	char **optarg;

	if (argc < 2) {
		print_help();
		return (1);
	}

	argv0 = argv[0];

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts_main);

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

		case 0:
			if (strcmp (optarg[0], "new") == 0) {
				return (main_create (argc, argv));
			}
			else if (strcmp (optarg[0], "create") == 0) {
				return (main_create (argc, argv));
			}
			else if (strcmp (optarg[0], "conv") == 0) {
				return (main_convert (argc, argv));
			}
			else if (strcmp (optarg[0], "convert") == 0) {
				return (main_convert (argc, argv));
			}
			else {
				fprintf (stderr, "%s: unknown command (%s)\n",
					argv0, optarg[0]
				);
				return (1);
			}
			break;

		default:
			return (1);
		}
	}

	return (1);
}
