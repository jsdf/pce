/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/utils/pceimg/pce-img.c                                 *
 * Created:       2005-11-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2005-2008 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include <config.h>

#include "pce-img.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <devices/block/block.h>
#include <devices/block/blkcow.h>
#include <devices/block/blkraw.h>
#include <devices/block/blkpce.h>
#include <devices/block/blkdosem.h>


#define DSK_NONE   0
#define DSK_RAW    1
#define DSK_PCE    2
#define DSK_DOSEMU 3


static const char *argv0 = NULL;

static int        par_quiet = 0;


static
void prt_help (void)
{
	fputs (
		"usage: pce-img command [options]\n"
		"\n"
		"create [options] [output]\n"
		"  -q, --quiet             Be quiet\n"
		"  -c, --cylinders int     Set number of cylinders [0]\n"
		"  -h, --heads int         Set number of heads [0]\n"
		"  -s, --sectors int       Set number of sectors per track [0]\n"
		"  -g, --geometry 3 * int  Set disk geometry (c h s)\n"
		"  -m, --size int          Set disk size in megabytes [0]\n"
		"  -f, --offset int        Set data offset [0]\n"
		"  -o, --output string     Set the output file name [stdout]\n"
		"\n"
		"convert [options] [input [output]]\n"
		"  -q, --quiet             Be quiet\n"
		"  -c, --cylinders int     Set number of cylinders [0]\n"
		"  -h, --heads int         Set number of heads [0]\n"
		"  -s, --sectors int       Set number of sectors per track [0]\n"
		"  -g, --geometry 3 * int  Set disk geometry (c h s)\n"
		"  -m, --size int          Set disk size in megabytes [0]\n"
		"  -f, --offset int        Set data offset [0]\n"
		"  -i, --input string      Set the input file name [stdin]\n"
		"  -o, --output string     Set the output file name [stdout]\n"
		"  -w, --cow string        Set the COW file name [none]\n"
		"\n"
		"file names: <format>:<name>\n"
		"formats:    raw, pce, dosemu\n",
		stdout
	);

	fflush (stdout);
}

static
void prt_version (void)
{
	fputs (
		"pce-img version " PCE_VERSION_STR
		" (" PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
		"Copyright (C) 1995-2008 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}


static
int opt_check (int argc, char **argv, const char *opt1, const char *opt2,
	int i, int n)
{
	if ((opt1 != NULL) && (strcmp (argv[i], opt1) == 0)) {
		;
	}
	else if ((opt2 != NULL) && (strcmp (argv[i], opt2) == 0)) {
		;
	}
	else {
		return (0);
	}

	if ((i + n) >= argc) {
		return (0);
	}

	return (1);
}

static
unsigned pce_get_disk_type (const char *str)
{
	unsigned i;
	char     buf[256];

	i = 0;
	while (str[i] != ':') {
		if (str[i] == 0) {
			return (DSK_NONE);
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

	if (strcmp (buf, "dosemu") == 0) {
		return (DSK_DOSEMU);
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
		return (dsk_img_create (name, n, c, h, s, ofs));

	case DSK_PCE:
		return (dsk_pce_create (name, n, c, h, s, ofs & 0xffffffff));

	case DSK_DOSEMU:
		return (dsk_dosemu_create (name, c, h, s, ofs & 0xffffffff));
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
		return (dsk_img_open (name, n, c, h, s, ofs, ro));

	case DSK_PCE:
		return (dsk_pce_open (name, ro));

	case DSK_DOSEMU:
		return (dsk_dosemu_open (name, ro));
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
			if (prg_i > 16384) {
				fprintf (stdout, "block %lu of %lu (%.2f%%)\r",
					(unsigned long) i,
					(unsigned long) prg_n,
					(100.0 * (i + 1)) / prg_n
				);
				prg_i = 0;

				fflush (stdout);
			}
		}
	}

	if (par_quiet == 0) {
		fprintf (stdout, "block %lu of %lu (%.2f%%)\n",
			(unsigned long) prg_n, (unsigned long) prg_n, 100.0
		);
	}

	return (0);
}

static
int main_create (int argc, char **argv)
{
	int      i;
	char     *par_out = NULL;
	uint32_t par_c = 0;
	uint32_t par_h = 0;
	uint32_t par_s = 0;
	uint32_t par_n = 0;
	uint64_t par_ofs = 0;

	i = 1;
	while (i < argc) {
		if (opt_check (argc, argv, "-q", "--quiet", i, 0)) {
			par_quiet = 1;
		}
		else if (opt_check (argc, argv, "-c", "--cylinders", i, 1)) {
			i += 1;
			par_c = strtoul (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-h", "--heads", i, 1)) {
			i += 1;
			par_h = strtoul (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-s", "--sectors", i, 1)) {
			i += 1;
			par_s = strtoul (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-g", "--geometry", i, 3)) {
			par_c = strtoul (argv[i + 1], NULL, 0);
			par_h = strtoul (argv[i + 2], NULL, 0);
			par_s = strtoul (argv[i + 3], NULL, 0);

			i += 3;
		}
		else if (opt_check (argc, argv, "-f", "--offset", i, 1)) {
			i += 1;
			par_ofs = strtoull (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-m", "--size", i, 1)) {
			i += 1;

			par_n = strtoul (argv[i], NULL, 0);
			par_n *= 2048;
		}
		else if (opt_check (argc, argv, "-o", "--output", i, 1)) {
			i += 1;
			par_out = argv[i];
		}
		else if (argv[i][0] != '-') {
			if (par_out != NULL) {
				fprintf (stderr, "%s: unknown option (%s)\n",
					argv0, argv[i]
				);
				return (1);
			}

			par_out = argv[i];
		}
		else {
			fprintf (stderr, "%s: unknown option (%s)\n",
				argv0, argv[i]
			);
			return (1);
		}

		i += 1;
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
	int        i;
	const char *par_inp = NULL;
	const char *par_out = NULL;
	const char *par_cow = NULL;
	uint32_t   par_c = 0;
	uint32_t   par_h = 0;
	uint32_t   par_s = 0;
	uint32_t   par_n = 0;
	uint64_t   par_ofs = 0;
	disk_t     *inp, *out, *cow;

	i = 1;
	while (i < argc) {
		if (opt_check (argc, argv, "-q", "--quiet", i, 0)) {
			par_quiet = 1;
		}
		else if (opt_check (argc, argv, "-c", "--cylinders", i, 1)) {
			i += 1;
			par_c = strtoul (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-h", "--heads", i, 1)) {
			i += 1;
			par_h = strtoul (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-s", "--sectors", i, 1)) {
			i += 1;
			par_s = strtoul (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-m", "--size", i, 1)) {
			i += 1;

			par_n = strtoul (argv[i], NULL, 0);
			par_n *= 2048;
		}
		else if (opt_check (argc, argv, "-g", "--geometry", i, 3)) {
			par_c = strtoul (argv[i + 1], NULL, 0);
			par_h = strtoul (argv[i + 2], NULL, 0);
			par_s = strtoul (argv[i + 3], NULL, 0);

			i += 3;
		}
		else if (opt_check (argc, argv, "-f", "--offset", i, 1)) {
			i += 1;
			par_ofs = strtoull (argv[i], NULL, 0);
		}
		else if (opt_check (argc, argv, "-w", "--cow", i, 1)) {
			i += 1;
			par_cow = argv[i];
		}
		else if (opt_check (argc, argv, "-i", "--input", i, 1)) {
			i += 1;
			par_inp = argv[i];
		}
		else if (opt_check (argc, argv, "-o", "--output", i, 1)) {
			i += 1;
			par_out = argv[i];
		}
		else if (argv[i][0] != '-') {
			if (par_inp == NULL) {
				par_inp = argv[i];
			}
			else if (par_out == NULL) {
				par_out = argv[i];
			}
			else {
				fprintf (stderr, "%s: too many files (%s)\n",
					argv0, argv[i]
				);
				return (1);
			}
		}
		else {
			fprintf (stderr, "%s: unknown option (%s)\n",
				argv0, argv[i]
			);
			return (1);
		}

		i += 1;
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

		inp = dsk_cow_new (cow, par_cow);
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
	if (argc < 2) {
		return (1);
	}

	argv0 = argv[0];

	if (argc == 2) {
		if (opt_check (argc, argv, "-h", "--help", 1, 0)) {
			prt_help();
			return (0);
		}
		else if (opt_check (argc, argv, "-v", "--version", 1, 0)) {
			prt_version();
			return (0);
		}
	}

	if (strcmp (argv[1], "new") == 0) {
		return (main_create (argc - 1, argv + 1));
	}
	else if (strcmp (argv[1], "create") == 0) {
		return (main_create (argc - 1, argv + 1));
	}
	else if (strcmp (argv[1], "conv") == 0) {
		return (main_convert (argc - 1, argv + 1));
	}
	else if (strcmp (argv[1], "convert") == 0) {
		return (main_convert (argc - 1, argv + 1));
	}

	return (1);
}
