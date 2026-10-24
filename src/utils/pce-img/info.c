/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pce-img/info.c                                     *
 * Created:     2018-02-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <drivers/block/block.h>
#include <drivers/block/blkpbi.h>
#include <drivers/block/blkqed.h>

#include <lib/getopt.h>


static pce_option_t opts_create[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'f', 1, "offset", "int", "Set the data offset [0]" },
	{ 'i', 1, "input", "string", "Set the input (base) file name" },
	{ 'I', 1, "input-type", "string", "Set the input file type [auto]" },
	{ 'q', 0, "quiet", NULL, "Be quiet [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{ 'w', 1, "cow", "string", "Add a COW file" },
	{ 'W', 1, "cow-type", "string", "Set the cow file type [auto]" },
	{  -1, 0, NULL, NULL, NULL }
};

static
void print_help (void)
{
	pce_getopt_help (
		"pce-img info: Show info about block image files",
		"usage: pce-img info [options] [file...]",
		opts_create
	);

	fflush (stdout);
}

static
int print_info (const char *fname)
{
	unsigned long long size;
	disk_t             *dsk;

	par_quiet = 1;

	printf ("File name: %s\n", fname);

	dsk = dsk_open_inp (fname, NULL, 1);

	if (dsk == NULL) {
		printf ("error opening image file\n");
		return (1);
	}

	size = 512ULL * dsk->blocks;

	printf ("Type:      %s\n", pce_get_type_name (dsk->type));

	printf ("Size:      %llu bytes, %lu blocks, %.3f KiB, %.3f MiB, %.3f GiB\n",
		size,
		(unsigned long) dsk->blocks,
		(double) size / 1024.0,
		(double) size / (1024.0 * 1024.0),
		(double) size / (1024.0 * 1024.0 * 1024.0)
	);

	printf ("Geometry:  %lu/%lu/%lu\n",
		(unsigned long) dsk->c,
		(unsigned long) dsk->h,
		(unsigned long) dsk->s
	);

	if (dsk->type == PCE_DISK_PBI) {
		disk_pbi_t *pbi;

		pbi = dsk->ext;

		printf ("Version:   %lu\n",
			(unsigned long) dsk_get_uint32_be (pbi->header, 4)
		);

		printf ("Size:      %llu\n",
			(unsigned long long) pbi->image_size
		);

		printf ("Alloc:     %llu\n",
			(unsigned long long) pbi->file_size
		);

		printf ("L1 offset: %llu\n",
			(unsigned long long) pbi->l1_table_offset
		);

		printf ("L1 table:  %lu bytes, %lu entries, %u bits\n",
			8UL << pbi->l1bits,
			1UL << pbi->l1bits,
			pbi->l1bits
		);

		printf ("L2 table:  %lu bytes, %lu entries, %u bits\n",
			8UL << pbi->l2bits,
			1UL << pbi->l2bits,
			pbi->l2bits
		);

		printf ("Blocks:    %lu bytes, %u bits\n",
			1UL << pbi->blbits,
			pbi->blbits
		);
	}
	else if (dsk->type == PCE_DISK_QED) {
		disk_qed_t *qed;

		qed = dsk->ext;

		printf ("Size:      %llu\n",
			(unsigned long long) qed->image_size
		);

		printf ("Alloc:     %llu\n",
			(unsigned long long) qed->offset
		);

		printf ("L1 offset: %llu\n",
			(unsigned long long) qed->l1_table_offset
		);

		printf ("Tables:    %lu bytes, %lu clusters\n",
			qed->table_size * qed->cluster_size,
			qed->table_size
		);

		printf ("Clusters:  %lu bytes\n",
			qed->cluster_size
		);
	}

	dsk_del (dsk);

	return (1);
}

int main_info (int argc, char **argv)
{
	int  r;
	int  first;
	char **optarg;

	first = 1;

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts_create);

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

		case 'f':
			if (pce_set_ofs (optarg[0])) {
				return (1);
			}
			break;

		case 'i':
			if (first == 0) {
				fputc ('\n', stdout);
			}
			print_info (optarg[0]);
			first = 0;
			break;

		case 'I':
			if (pce_set_type_inp (optarg[0])) {
				return (1);
			}
			break;

		case 'q':
			pce_set_quiet (1);
			break;

		case 0:
			if (first == 0) {
				fputc ('\n', stdout);
			}
			print_info (optarg[0]);
			first = 0;
			break;

		default:
			return (1);
		}
	}

	return (1);
}
