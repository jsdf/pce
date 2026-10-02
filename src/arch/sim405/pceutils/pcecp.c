/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   pcecp.c                                                      *
 * Created:     2018-12-22 by Hampa Hug <hampa@hampa.ch>                     *
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
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "pce.h"


#define BUFMAX 16384


const char           *arg0 = NULL;

static char          par_verbose = 0;
static char          par_write = 0;
static char          par_std = 0;
static char          par_endarg = 0;

static char          *par_src = NULL;
static char          *par_dst = NULL;

static unsigned char par_buf[BUFMAX];


static
void print_help (void)
{
	fputs (
		"pcecp: copy files to/from the host\n"
		"\n"
		"usage: pcecp [options] [src [dst]]\n"
		"  -c  Use stdin/stdout instead of a guest file [no]\n"
		"  -h  Print help\n"
		"  -i  Copy a file from the host to the guest [default]\n"
		"  -o  Copy a file from the guest to the host\n"
		"  -v  Verbose operation [no]\n"
		"  -V  Print version\n",
		stdout
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs ("pcecp version " PCEUTILS_VERSION_STR "\n", stdout);
	fflush (stdout);
}

static
int pce_copy_in_fp (const char *inp, FILE *out)
{
	int           r;
	unsigned      cnt;
	unsigned long total;
	time_t        t1, t2;

	if (pce_read_open (inp)) {
		fprintf (stderr, "%s: error opening host file (%s)\n",
			arg0, inp
		);

		return (1);
	}

	t1 = time (NULL);

	total = 0;
	cnt = 0;

	while (1) {
		r = pce_read16 (par_buf + cnt);

		if (r <= 0) {
			if (r < 0) {
				fprintf (stderr, "%s: read error\n", arg0);
			}

			break;
		}

		cnt += r;

		if (cnt >= BUFMAX) {
			total += cnt;

			if (par_verbose) {
				t2 = time (NULL);

				if (t1 != t2) {
					fprintf (stderr, "%lu KiB read\r",
						total / 1024
					);

					t1 = t2;
				}
			}

			if (fwrite (par_buf, 1, cnt, out) != cnt) {
				fprintf (stderr, "%s: write error\n", arg0);
				return (1);
			}

			cnt = 0;
		}
	}

	if (cnt > 0) {
		total += cnt;

		if (fwrite (par_buf, 1, cnt, out) != cnt) {
			fprintf (stderr, "%s: write error\n", arg0);
			return (1);
		}
	}

	pce_read_close();

	fflush (out);

	if (par_verbose) {
		fprintf (stderr, "%lu bytes read\n", total);
	}

	return (0);
}

static
int pce_copy_in (const char *inp, const char *out, int std)
{
	int  r;
	FILE *fp;

	if (std) {
		fp = stdout;
	}
	else {
		fp = fopen (out, "wb");
	}

	if (fp == NULL) {
		fprintf (stderr, "%s: error opening local file (%s)\n",
			arg0, out
		);

		return (1);
	}

	r = pce_copy_in_fp (inp, fp);

	if (fp != stdout) {
		fclose (fp);
	}

	return (r);
}

static
int pce_copy_out_fp (FILE *inp, const char *out)
{
	int           r;
	unsigned      idx, cnt, n;
	unsigned long total;
	time_t        t1, t2;

	if (pce_write_open (out)) {
		fprintf (stderr, "%s: error opening host file (%s)\n",
			arg0, out
		);

		return (1);
	}

	t1 = time (NULL);

	total = 0;
	idx = 0;
	cnt = 0;

	while (1) {
		if (idx >= cnt) {
			if (par_verbose) {
				t2 = time (NULL);

				if (t1 != t2) {
					fprintf (stderr, "%lu KiB written\r",
						total / 1024
					);

					t1 = t2;
				}
			}

			idx = 0;
			cnt = fread (par_buf, 1, BUFMAX, inp);

			if (cnt == 0) {
				break;
			}
		}

		n = cnt - idx;

		if (n > 16) {
			n = 16;
		}

		r = pce_write16 (par_buf + idx, n);

		if (r <= 0) {
			if (r < 0) {
				fprintf (stderr, "%s: write error\n", arg0);
			}

			break;
		}

		idx += r;
		total += r;
	}

	pce_write_close();

	if (par_verbose) {
		fprintf (stderr, "%lu bytes written\n", total);
	}

	return (0);
}

static
int pce_copy_out (const char *inp, const char *out, int std)
{
	int  r;
	FILE *fp;

	if (std) {
		fp = stdin;
	}
	else {
		fp = fopen (inp, "rb");
	}

	if (fp == NULL) {
		fprintf (stderr, "%s: error opening local file (%s)\n",
			arg0, inp
		);

		return (1);
	}

	r = pce_copy_out_fp (fp, out);

	if (fp != stdin) {
		fclose (fp);
	}

	return (r);
}

static
int args (int *argi, char **argv)
{
	const char *s;

	s = argv[*argi] + 1;

	while (*s != 0) {
		if (*s == 'c') {
			par_std = 1;
		}
		else if (*s == 'h') {
			print_help();
			exit (0);
		}
		else if (*s == 'i') {
			par_write = 0;
		}
		else if (*s == 'o') {
			par_write = 1;
		}
		else if (*s == 'v') {
			par_verbose = 1;
		}
		else if (*s == 'V') {
			print_version();
			exit (0);
		}
		else {
			fprintf (stderr, "%s: unknown option (%s)\n", arg0, s);
			return (1);
		}

		s += 1;
	}

	return (0);
}

int main (int argc, char **argv)
{
	int i, r;

	arg0 = argv[0];

	if (pce_check()) {
		fprintf (stderr, "%s: not running under pce\n", arg0);
		return (1);
	}

	i = 1;

	while (i < argc) {
		if ((argv[i][0] == '-') && (par_endarg == 0)) {
			if ((argv[i][1] == '-') && (argv[i][2] == 0)) {
				par_endarg = 1;
			}
			else if (args (&i, argv)) {
				return (1);
			}
		}
		else {
			if (par_src == NULL) {
				par_src = argv[i];
			}
			else if (par_dst == NULL) {
				par_dst = argv[i];
			}
			else {
				return (1);
			}
		}

		i += 1;
	}

	if (par_src == NULL) {
		print_help();
		return (0);
	}

	if (par_dst == NULL) {
		par_dst = par_src;
	}

	if (par_write) {
		r = pce_copy_out (par_src, par_dst, par_std);
	}
	else {
		r = pce_copy_in (par_src, par_dst, par_std);
	}

	if (r) {
		return (1);
	}

	return (0);
}
