/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   pcemsg.c                                                     *
 * Created:     2018-12-23 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>

#include "config.h"
#include "pce.h"


const char *arg0 = NULL;


static
void print_help (void)
{
	fputs (
		"pcemsg: send messages to the emulator\n"
		"\n"
		"usage: pcemsg [options] message [arg]\n"
		"  -h  Print help\n"
		"  -V  Print version information\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs ("pcemsg version " PCEUTILS_VERSION_STR "\n", stdout);
	fflush (stdout);
}

int main (int argc, char **argv)
{
	int        i;
	const char *s;
	const char *msg, *val;

	arg0 = argv[0];

	msg = NULL;
	val = NULL;

	i = 1;
	while (i < argc) {
		if (argv[i][0] == '-') {
			s = argv[i] + 1;

			while (*s != 0) {
				if (*s == 'h') {
					print_help();
					return (0);
				}
				else if (*s == 'V') {
					print_version();
					return (0);
				}
				else {
					fprintf (stderr,
						"%s: unknown option (%s)\n",
						arg0, argv[i]
					);

					return (1);
				}

				s += 1;
			}
		}
		else if (msg == NULL) {
			msg = argv[i];
		}
		else if (val == NULL) {
			val = argv[i];
		}
		else {
			fprintf (stderr, "%s: too many arguments (%s)\n",
				arg0, argv[i]
			);

			return (1);
		}

		i += 1;
	}

	if (msg == NULL) {
		print_help();
		return (1);
	}

	if (val == NULL) {
		val = "";
	}

	if (pce_set_msg (msg, val)) {
		fprintf (stderr, "error\n");
		return (1);
	}

	return (0);
}
