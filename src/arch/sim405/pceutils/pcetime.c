/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   pcetime.c                                                    *
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
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "pce.h"


const char  *arg0 = NULL;

static char par_print = 0;
static char par_set = 0;


static
void print_help (void)
{
	fputs (
		"pcetime: get the time from the host\n"
		"\n"
		"usage: pcetime [options]\n"
		"  -h  Print help\n"
		"  -p  Print the current host time\n"
		"  -s  Set the system time to the current host time\n"
		"  -V  Print version\n",
		stdout
	);
}

static
void print_version (void)
{
	fputs ("pcetime version " PCEUTILS_VERSION_STR "\n", stdout);
	fflush (stdout);
}

int main (int argc, char **argv)
{
	int        i;
	const char *s;
	time_t     val;

	arg0 = argv[0];

	if (pce_check()) {
		fprintf (stderr, "%s: not running under pce\n", arg0);
		return (1);
	}

	i = 1;
	while (i < argc) {
		if (argv[i][0] == '-') {
			s = argv[i] + 1;
			while (*s != 0) {
				if (*s == 'h') {
					print_help();
					return (0);
				}
				else if (*s == 'p') {
					par_print = 1;
				}
				else if (*s == 's') {
					par_set = 1;
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
		else {
			fprintf (stderr, "%s: unknown option (%s)\n",
				arg0, argv[i]
			);

			return (1);
		}

		i += 1;
	}

	if ((par_print == 0) && (par_set == 0)) {
		print_help();
		return (0);
	}

	val = (time_t) pce_get_time_unix();

	if (par_print) {
		struct tm *tm;

		tm = gmtime (&val);

		printf ("%04d-%02d-%02d %02d:%02d:%02d UTC\n",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec
		);
	}

	if (par_set) {
		struct timeval tv;

		tv.tv_sec = val;
		tv.tv_usec = 0;

		if (settimeofday (&tv, NULL)) {
			fprintf (stderr, "%s: error setting time\n", arg0);
			return (1);
		}
	}

	return (0);
}
