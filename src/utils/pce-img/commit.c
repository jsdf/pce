/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pce-img/commit.c                                   *
 * Created:     2013-01-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2018 Hampa Hug <hampa@hampa.ch>                     *
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

#include <lib/getopt.h>


static pce_option_t opts_commit[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'i', 1, "input", "string", "Set the input file name [none]" },
	{ 'I', 1, "input-type", "string", "Set the input file type [auto]" },
	{ 'q', 0, "quiet", NULL, "Be quiet [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{ 'w', 1, "cow", "string", "Add a COW file [none]" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pce-img commit: commit changes",
		"usage: pce-img commit [options] [image] [cow...]",
		opts_commit
	);

	fflush (stdout);
}

int main_commit (int argc, char **argv)
{
	int    r;
	char   **optarg;
	disk_t *inp;

	inp = NULL;

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts_commit);

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

		case 'i':
			if ((inp = dsk_open_inp (optarg[0], inp, 0)) == NULL) {
				return (1);
			}
			break;

		case 'I':
			if (pce_set_type_inp (optarg[0])) {
				return (1);
			}
			break;

		case 'q':
			pce_set_quiet (1);
			break;

		case 'w':
			if ((inp = pce_cow_open (inp, optarg[0])) == NULL) {
				return (1);
			}
			break;

		case 0:
			if (inp == NULL) {
				if ((inp = dsk_open_inp (optarg[0], inp, 0)) == NULL) {
					return (1);
				}
			}
			else {
				if ((inp = pce_cow_open (inp, optarg[0])) == NULL) {
					return (1);
				}
			}
			break;

		default:
			return (1);
		}
	}

	if (inp == NULL) {
		return (0);
	}

	if (dsk_commit (inp)) {
		fprintf (stderr, "%s: commit failed\n", arg0);
		return (1);
	}

	dsk_del (inp);

	return (0);
}
