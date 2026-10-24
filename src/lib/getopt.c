/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/getopt.c                                             *
 * Created:     2009-14-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/getopt.h>


static int        atend = 0;
static int        index1 = -1;
static int        index2 = -1;
static const char *curopt = NULL;


static
int opt_cmp (const pce_option_t *opt1, const pce_option_t *opt2)
{
	int c1, c2;

	c1 = (opt1->name1 <= 255) ? tolower (opt1->name1) : opt1->name1;
	c2 = (opt2->name1 <= 255) ? tolower (opt2->name1) : opt2->name1;

	if (c1 < c2) {
		return (-1);
	}
	else if (c1 > c2) {
		return (1);
	}
	else if (opt1->name1 < opt2->name1) {
		return (1);
	}
	else if (opt1->name1 > opt2->name1) {
		return (-1);
	}

	return (0);
}

static
unsigned opt_get_width (const pce_option_t *opt)
{
	unsigned n;

	if (opt->optdesc == NULL) {
		return (0);
	}

	n = 0;

	if (opt->name1 <= 255) {
		n += 2;

		if (opt->name2 != NULL) {
			n += 2;
		}
	}

	if (opt->name2 != NULL) {
		n += 2 + strlen (opt->name2);
	}

	if (opt->argdesc != NULL) {
		n += 1 + strlen (opt->argdesc);
	}

	return (n);
}

static
unsigned opt_max_width (const pce_option_t *opt)
{
	unsigned i, n, w;

	w = 0;

	i = 0;
	while (opt[i].name1 >= 0) {
		n = opt_get_width (&opt[i]);

		if (n > w) {
			w = n;
		}

		i += 1;
	}

	return (w);
}

static
void sort_options (pce_option_t *opt)
{
	unsigned     i, j;
	pce_option_t tmp;

	if (opt[0].name1 < 0) {
		return;
	}

	i = 1;
	while (opt[i].name1 >= 0) {
		if (opt_cmp (&opt[i], &opt[i - 1]) >= 0) {
			i += 1;
			continue;
		}

		j = i - 1;

		tmp = opt[i];
		opt[i] = opt[j];

		while ((j > 0) && (opt_cmp (&tmp, &opt[j - 1]) < 0)) {
			opt[j] = opt[j - 1];
			j -= 1;
		}

		opt[j] = tmp;

		i += 1;
	}
}

static
void print_option (const pce_option_t *opt, unsigned w)
{
	unsigned n;

	n = 0;

	if (opt->name1 <= 255) {
		printf ("  -%c", opt->name1);
		n += 2;

		if (opt->name2 != NULL) {
			printf (", ");
			n += 2;
		}
	}
	else {
		printf ("  ");
	}

	if (opt->name2 != NULL) {
		printf ("--%s", opt->name2);
		n += 2 + strlen (opt->name2);
	}

	if (opt->argdesc != NULL) {
		printf (" %s", opt->argdesc);
		n += 1 + strlen (opt->argdesc);
	}

	while (n < w) {
		fputc (' ', stdout);
		n += 1;
	}

	printf ("%s\n", opt->optdesc);
}

void pce_getopt_help (const char *tag, const char *usage, pce_option_t *opt)
{
	unsigned w;

	sort_options (opt);

	w = opt_max_width (opt);

	if (tag != NULL) {
		printf ("%s\n\n", tag);
	}

	if (usage != NULL) {
		printf ("%s\n", usage);
	}

	while (opt->name1 >= 0) {
		print_option (opt, w + 2);
		opt += 1;
	}
}

static
pce_option_t *find_option_name1 (pce_option_t *opt, int name1)
{
	while (opt->name1 >= 0) {
		if (opt->name1 == name1) {
			return (opt);
		}

		opt += 1;
	}

	return (NULL);
}

static
pce_option_t *find_option_name2 (pce_option_t *opt, const char *name2)
{
	while (opt->name1 >= 0) {
		if (strcmp (opt->name2, name2) == 0) {
			return (opt);
		}

		opt += 1;
	}

	return (NULL);
}

int pce_getoptarg (int argc, char **argv, char ***optarg, unsigned cnt)
{
	if (index1 < 0) {
		atend = 0;
		index1 = 0;
		index2 = 1;
		curopt = NULL;
	}

	if ((index2 + cnt) > argc) {
		return (GETOPT_MISSING);
	}

	index1 = index2;
	index2 += cnt;

	*optarg = argv + index1;

	return (0);
}

int pce_getopt (int argc, char **argv, char ***optarg, pce_option_t *opt)
{
	pce_option_t *ret;

	if ((argc == 0) && (argv == NULL)) {
		index1 = -1;
		return (0);
	}

	if (index1 < 0) {
		atend = 0;
		index1 = 0;
		index2 = 1;
		curopt = NULL;
	}

	if (atend || (opt == NULL)) {
		if (index2 >= argc) {
			return (GETOPT_DONE);
		}

		index1 = index2;
		index2 += 1;

		*optarg = argv + index1;

		return (0);
	}

	if ((curopt == NULL) || (*curopt == 0)) {
		if (index2 >= argc) {
			return (GETOPT_DONE);
		}

		index1 = index2;
		index2 += 1;

		curopt = argv[index1];

		if ((curopt[0] != '-') || (curopt[1] == 0)) {
			*optarg = argv + index1;
			curopt = NULL;
			return (0);
		}

		if (curopt[1] == '-') {
			if (curopt[2] == 0) {
				atend = 1;

				if (index2 >= argc) {
					return (GETOPT_DONE);
				}

				index1 = index2;
				index2 += 1;

				*optarg = argv + index1;

				return (0);
			}

			ret = find_option_name2 (opt, curopt + 2);

			if (ret == NULL) {
				fprintf (stderr, "%s: unknown option (%s)\n",
					argv[0], curopt
				);
				return (GETOPT_UNKNOWN);
			}

			if ((index2 + ret->argcnt) > argc) {
				fprintf (stderr,
					"%s: missing option argument (%s)\n",
					argv[0], curopt
				);
				return (GETOPT_MISSING);
			}

			*optarg = argv + index2;
			index2 += ret->argcnt;
			curopt = NULL;

			return (ret->name1);
		}

		curopt += 1;
	}

	ret = find_option_name1 (opt, *curopt);

	if (ret == NULL) {
		fprintf (stderr, "%s: unknown option (-%c)\n",
			argv[0], *curopt
		);
		return (GETOPT_UNKNOWN);
	}

	if ((index2 + ret->argcnt) > argc) {
		fprintf (stderr,
			"%s: missing option argument (-%c)\n",
			argv[0], *curopt
		);
		return (GETOPT_MISSING);
	}

	*optarg = argv + index2;
	index2 += ret->argcnt;
	curopt += 1;

	return (ret->name1);
}
