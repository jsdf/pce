/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pce-img/pce-img.c                                  *
 * Created:     2005-11-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#include "pce-img.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <drivers/block/block.h>
#include <drivers/block/blkcow.h>
#include <drivers/block/blkraw.h>
#include <drivers/block/blkpbi.h>
#include <drivers/block/blkpce.h>
#include <drivers/block/blkpsi.h>
#include <drivers/block/blkqed.h>
#include <drivers/block/blkdosem.h>

#include <drivers/psi/psi-img.h>

#include <lib/getopt.h>
#include <lib/sysdep.h>


const char *arg0 = NULL;
char       par_quiet = 0;

static unsigned           par_type_inp = 0;
static unsigned           par_type_out = 0;

static unsigned long      par_c = 0;
static unsigned long      par_h = 0;
static unsigned long      par_s = 0;
static unsigned long      par_n = 0;
static unsigned long long par_ofs = 0;
static unsigned long      par_min_cluster_size = 0;

static unsigned long      par_buf_size = 0;
static unsigned char      *par_buf = NULL;


static pce_option_t opts_main[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'V', 0, "version", NULL, "Print version information" },
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

	fputs (
		"\ncommands:\n"
		"  commit   Commit changes in COW files\n"
		"  convert  Convert images\n"
		"  cow      Create COW files\n"
		"  create   Create images\n"
		"  rebase   Rebase images\n"
		"\nformats:\n"
		"  dosemu, img, pbi, pimg, psi, qed\n",
		stdout
	);

	fflush (stdout);
}

void print_version (void)
{
	fputs (
		"pce-img version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2005-2018 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

void print_disk_info (disk_t *dsk, const char *name)
{
	if (par_quiet) {
		return;
	}

	fprintf (stdout, "%s: %.3f MiB (%lu/%lu/%lu)\n",
		name,
		(double) dsk->blocks / (2 * 1024),
		(unsigned long) dsk->c,
		(unsigned long) dsk->h,
		(unsigned long) dsk->s
	);
}

static
unsigned pce_get_type (const char *str)
{
	if ((strcmp (str, "raw") == 0) || (strcmp (str, "img") == 0)) {
		return (DSK_RAW);
	}

	if ((strcmp (str, "pce") == 0) || (strcmp (str, "pimg") == 0)) {
		return (DSK_PCE);
	}

	if ((strcmp (str, "qed") == 0) || (strcmp (str, "cow") == 0)) {
		return (DSK_QED);
	}

	if (strcmp (str, "pbi") == 0) {
		return (DSK_PBI);
	}

	if (strcmp (str, "dosemu") == 0) {
		return (DSK_DOSEMU);
	}

	if (strcmp (str, "psi") == 0) {
		return (DSK_PSI);
	}

	return (DSK_NONE);
}

/*
 * Guess the image type based on the file name extension.
 */
static
unsigned pce_get_type_ext (const char *str, unsigned type)
{
	unsigned   ret;
	const char *ext;

	if (type != DSK_NONE) {
		return (type);
	}

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

	ret = pce_get_type (ext);

	if (ret != DSK_NONE) {
		return (ret);
	}

	if (psi_guess_type (str) != PSI_FORMAT_NONE) {
		return (DSK_PSI);
	}

	return (DSK_NONE);
}

void pce_set_quiet (int val)
{
	par_quiet = (val != 0);
}

int pce_set_n (const char *str)
{
	unsigned long long size;
	char               *end;

	if (strcmp (str, "auto") == 0) {
		par_n = par_c * par_h * par_s;
	}
	else {
		size = strtoull (str, &end, 0);

		if (toupper (*end) == 'K') {
			end += 1;
			size *= 1024;
		}
		else if (toupper (*end) == 'M') {
			end += 1;
			size *= 1024Ul * 1024UL;
		}
		else if (toupper (*end) == 'G') {
			end += 1;
			size *= 1024Ul * 1024UL * 1024UL;
		}
		else if (toupper (*end) == 'T') {
			end += 1;
			size *= 1024Ul * 1024UL * 1024UL;
			size *= 1024;
		}
		else if (toupper (*end) == 'B') {
			end += 1;
		}
		else {
			size *= 512;
		}

		if (*end != 0) {
			fprintf (stderr, "%s: bad image size (%s)\n", arg0, str	);
			return (1);
		}

		par_n = size / 512;
	}

	return (0);
}

int pce_set_c (const char *str)
{
	char          *end;
	unsigned long h, s;

	if (strcmp (str, "auto") == 0) {
		s = (par_s > 0) ? par_s : 63;
		h = (par_h > 0) ? par_h : 16;

		par_c = par_n / (h * s);
	}
	else {
		par_c = strtoul (str, &end, 0);

		if (*end != 0) {
			fprintf (stderr, "%s: bad cylinder (%s)\n", arg0, str);
			return (1);
		}
	}

	return (0);
}

int pce_set_h (const char *str)
{
	char          *end;
	unsigned long c, s;

	if (strcmp (str, "auto") == 0) {
		s = (par_s > 0) ? par_s : 63;
		c = (par_c > 0) ? par_c : 1024;

		par_h = par_n / (c * s);
	}
	else {
		par_h = strtoul (str, &end, 0);

		if (*end != 0) {
			fprintf (stderr, "%s: bad head (%s)\n", arg0, str);
			return (1);
		}
	}

	return (0);
}

int pce_set_s (const char *str)
{
	char          *end;
	unsigned long c, h;

	if (strcmp (str, "auto") == 0) {
		h = (par_h > 0) ? par_h : 16;
		c = (par_c > 0) ? par_c : 1024;

		par_s = par_n / (c * h);
	}
	else {
		par_s = strtoul (str, &end, 0);

		if (*end != 0) {
			fprintf (stderr, "%s: bad sector (%s)\n", arg0, str);
			return (1);
		}
	}

	return (0);
}

int pce_set_geo (const char *c, const char *h, const char *s)
{
	if (pce_set_c (c)) {
		return (1);
	}

	if (pce_set_h (h)) {
		return (1);
	}

	if (pce_set_s (s)) {
		return (1);
	}

	return (0);
}

int pce_set_ofs (const char *str)
{
	char *end;

	par_ofs = strtoul (str, &end, 0);

	if (*end != 0) {
		fprintf (stderr, "%s: bad offset (%s)\n", arg0, str);
		return (1);
	}

	return (0);
}

int pce_set_min_cluster_size (const char *str)
{
	char *end;

	par_min_cluster_size = strtoul (str, &end, 0);

	if (*end != 0) {
		fprintf (stderr, "%s: bad cluster size (%s)\n", arg0, str);
		return (1);
	}

	return (0);
}

int pce_set_type_inp (const char *str)
{
	par_type_inp = pce_get_type (str);

	if (par_type_inp == DSK_NONE) {
		return (1);
	}

	return (0);
}

int pce_set_type_out (const char *str)
{
	par_type_out = pce_get_type (str);

	if (par_type_out == DSK_NONE) {
		return (1);
	}

	return (0);
}

unsigned char *pce_get_buf (unsigned long size)
{
	unsigned char *ret;

	if (size > par_buf_size) {
		if ((ret = realloc (par_buf, size)) == NULL) {
			return (NULL);
		}

		par_buf = ret;
		par_buf_size = size;

		return (ret);
	}

	return (par_buf);
}

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

int pce_block_is_uniform_32 (const void *buf, unsigned cnt, unsigned long *val)
{
	unsigned            i;
	const unsigned char *tmp;

	tmp = buf;

	for (i = 4; i < cnt; i++) {
		if (tmp[i] != tmp[i & 3]) {
			return (0);
		}
	}

	*val = dsk_get_uint32_be (buf, 0);

	return (1);
}

int pce_file_exists (const char *name)
{
	FILE *fp;

	if ((fp = fopen (name, "rb")) == NULL) {
		return (0);
	}

	fclose (fp);

	return (1);
}

void pce_set_disk_parameters (disk_t *dsk)
{
	if (dsk == NULL) {
		return;
	}

	par_c = dsk->c;
	par_h = dsk->h;
	par_s = dsk->s;
	par_n = dsk->blocks;
}

int dsk_create (const char *name, unsigned type)
{
	int r;

	if ((type = pce_get_type_ext (name, type)) == DSK_NONE) {
		return (1);
	}

	if (par_n == 0) {
		par_n = (unsigned long) par_c * par_h * par_s;
	}

	switch (type) {
	case DSK_RAW:
		r = dsk_img_create (name, par_n, par_ofs);
		break;

	case DSK_PCE:
		r = dsk_pce_create (name, par_n, par_c, par_h, par_s, par_ofs & 0xffffffff);
		break;

	case DSK_PBI:
		r = dsk_pbi_create (name, par_n, par_c, par_h, par_s, par_min_cluster_size);
		break;

	case DSK_QED:
		r = dsk_qed_create (name, par_n, par_min_cluster_size);
		break;

	case DSK_DOSEMU:
		r = dsk_dosemu_create (name, par_c, par_h, par_s, par_ofs & 0xffffffff);
		break;

	case DSK_PSI:
		r = dsk_psi_create (name, PSI_FORMAT_NONE, par_c, par_h, par_s);
		break;

	default:
		r = dsk_pce_create (name, par_n, par_c, par_h, par_s, par_ofs & 0xffffffff);
		break;
	}

	return (r);
}

static
disk_t *dsk_open (const char *name, unsigned type, int ro)
{
	disk_t *dsk;

	type = pce_get_type_ext (name, type);

	switch (type) {
	case DSK_RAW:
		dsk = dsk_img_open (name, par_ofs, ro);
		break;

	case DSK_PCE:
		dsk = dsk_pce_open (name, ro);
		break;

	case DSK_PBI:
		dsk = dsk_pbi_open (name, ro);
		break;

	case DSK_QED:
		dsk = dsk_qed_open (name, ro);
		break;

	case DSK_DOSEMU:
		dsk = dsk_dosemu_open (name, ro);
		break;

	case DSK_PSI:
		dsk = dsk_psi_open (name, PSI_FORMAT_NONE, ro);
		break;

	default:
		dsk = dsk_auto_open (name, par_ofs, ro);
		break;
	}

	if (dsk != NULL) {
		pce_set_disk_parameters (dsk);
		print_disk_info (dsk, name);
	}

	return (dsk);
}

disk_t *dsk_open_inp (const char *name, disk_t *dsk, int ro)
{
	if (dsk != NULL) {
		dsk_del (dsk);
	}

	return (dsk_open (name, par_type_inp, ro));
}

disk_t *dsk_open_out (const char *name, disk_t *dsk, int create)
{
	if (dsk != NULL) {
		dsk_del (dsk);
	}

	if (create) {
		if (create < 0) {
			if (dsk_create (name, par_type_out)) {
				return (NULL);
			}
		}
		else if (pce_file_exists (name) == 0) {
			if (dsk_create (name, par_type_out)) {
				return (NULL);
			}
		}
	}

	return (dsk_open (name, par_type_out, 0));
}

disk_t *dsk_cow_create (const char *name, disk_t *dsk)
{
	disk_t *cow;

	if (dsk == NULL) {
		return (NULL);
	}

	if (par_n == 0) {
		par_n = (unsigned long) par_c * par_h * par_s;
	}

	if (dsk_qed_create (name, par_n, par_min_cluster_size)) {
		return (NULL);
	}

	cow = dsk_qed_cow_new (dsk, name);

	if (cow == NULL) {
		dsk_del (dsk);

		fprintf (stderr, "%s: can't create COW file (%s)\n",
			arg0, name
		);

		return (NULL);
	}

	print_disk_info (cow, name);

	return (cow);
}

disk_t *dsk_cow (const char *name, disk_t *dsk)
{
	disk_t *cow;

	if (dsk == NULL) {
		return (NULL);
	}

	cow = dsk_qed_cow_new (dsk, name);

	if (cow == NULL) {
		cow = dsk_cow_new (dsk, name);
	}

	if (cow == NULL) {
		dsk_del (dsk);

		fprintf (stderr, "%s: can't open COW file (%s)\n",
			arg0, name
		);

		return (NULL);
	}

	pce_set_disk_parameters (cow);
	print_disk_info (cow, name);

	return (cow);
}

int main (int argc, char **argv)
{
	int  r;
	char **optarg;

	if (argc < 2) {
		print_help();
		return (1);
	}

	arg0 = argv[0];

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
			if (strcmp (optarg[0], "commit") == 0) {
				return (main_commit (argc, argv));
			}
			else if (strcmp (optarg[0], "convert") == 0) {
				return (main_convert (argc, argv));
			}
			else if (strcmp (optarg[0], "cow") == 0) {
				return (main_cow (argc, argv));
			}
			else if (strcmp (optarg[0], "create") == 0) {
				return (main_create (argc, argv));
			}
			else if (strcmp (optarg[0], "new") == 0) {
				return (main_create (argc, argv));
			}
			else if (strcmp (optarg[0], "rebase") == 0) {
				return (main_rebase (argc, argv));
			}
			else {
				fprintf (stderr, "%s: unknown command (%s)\n",
					arg0, optarg[0]
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
