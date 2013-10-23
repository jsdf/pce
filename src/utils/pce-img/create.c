/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pce-img/create.c                                   *
 * Created:     2013-01-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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

#include <lib/getopt.h>


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
	{ 'w', 1, "cow", "string", "Add a COW file" },
	{  -1, 0, NULL, NULL, NULL }
};

static
void print_help (void)
{
	pce_getopt_help (
		"pce-img create: Create disk images",
		"usage: pce-img create [options] [output]",
		opts_create
	);

	fflush (stdout);
}

int main_create (int argc, char **argv)
{
	int    r;
	char   **optarg;
	disk_t *out;

	out = NULL;

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

		case 'c':
			pce_set_c (optarg[0]);
			break;

		case 'C':
			pce_set_min_cluster_size (optarg[0]);
			break;

		case 'f':
			pce_set_ofs (optarg[0]);
			break;

		case 'g':
			pce_set_c (optarg[0]);
			pce_set_h (optarg[1]);
			pce_set_s (optarg[2]);
			break;

		case 'h':
			pce_set_h (optarg[0]);
			break;

		case 'm':
			pce_set_n (optarg[0], 2048);
			break;

		case 'n':
			pce_set_n (optarg[0], 1);
			break;

		case 'o':
			if ((out = dsk_open_out (optarg[0], out, -1)) == NULL) {
				return (1);
			}
			break;

		case 'q':
			pce_set_quiet (1);
			break;

		case 's':
			pce_set_s (optarg[0]);
			break;

		case 'w':
			if ((out = dsk_cow (optarg[0], out)) == NULL) {
				return (1);
			}
			break;

		case 0:
			if ((out = dsk_open_out (optarg[0], out, -1)) == NULL) {
				return (1);
			}
			break;

		default:
			return (1);
		}
	}

	if (out != NULL) {
		dsk_del (out);
	}

	return (1);
}
