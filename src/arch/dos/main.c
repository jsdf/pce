/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/main.c                                          *
 * Created:     2012-12-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "dos.h"
#include "exec.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include <lib/getopt.h>
#include <lib/sysdep.h>


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'c', 0, "command", NULL, "Set the DOS command" },
	{ 'd', 2, "drive", "char string", "Attach a host path to a DOS drive" },
	{ 'l', 0, "log-int", NULL, "Log interrupts [no]" },
	{ 'm', 1, "memory", "int", "Set the memory size in KiB [640]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{  -1, 0, NULL, NULL, NULL }
};


const char *arg0 = NULL;

static const char *par_drives[26];


static
void print_help (void)
{
	pce_getopt_help (
		"pce-dos: Run DOS applications",
		"usage: pce-dos [options] [program [options]]",
		opts
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pce-dos version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2012-2015 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

static
void sig_int (int s)
{
	fprintf (stderr, "pce-dos: sigint\n");
	fflush (stderr);
	exit (1);
}

static
void sig_term (int s)
{
	fprintf (stderr, "pce-dos: sigterm\n");
	fflush (stderr);
	exit (1);
}

static
void sig_segv (int s)
{
	fprintf (stderr, "pce-dos: segmentation fault\n");
	fflush (stderr);
	exit (1);
}

static
int set_drive (const char *drv, const char *name)
{
	unsigned d;

	if ((drv[0] != 0) && (drv[1] == 0)) {
		if ((drv[0] >= 'a') && (drv[0] <= 'z')) {
			d = drv[0] - 'a';
		}
		else if ((drv[0] >= 'A') && (drv[0] <= 'Z')) {
			d = drv[0] - 'Z';
		}
		else {
			return (1);
		}
	}
	else {
		d = strtoul (drv, NULL, 0);
	}

	if (d > sizeof (par_drives)) {
		return (1);
	}

	par_drives[d] = name;

	return (0);
}

int main (int argc, char **argv)
{
	int      r;
	unsigned i;
	char     **optarg;
	char     *prog_dos, *prog_host;
	char     log_int;
	unsigned mem;
	dos_t    sim;

	arg0 = argv[0];

	mem = 640;
	log_int = 0;

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts);

		if (r == GETOPT_DONE) {
			return (1);
		}

		if (r < 0) {
			return (1);
		}

		if ((r == 0) || (r == 'c')) {
			break;
		}

		switch (r) {
		case '?':
			print_help();
			return (0);

		case 'V':
			print_version();
			return (0);

		case 'd':
			if (set_drive (optarg[0], optarg[1])) {
				return (1);
			}
			break;

		case 'l':
			log_int = 1;
			break;

		case 'm':
			mem = strtoul (optarg[0], NULL, 0);
			break;

		default:
			return (1);
		}
	}

	signal (SIGINT, sig_int);
	signal (SIGTERM, sig_term);
	signal (SIGSEGV, sig_segv);

	if (sim_init (&sim, mem)) {
		return (1);
	}

	sim.log_int = log_int || 0;

	for (i = 0; i < 26; i++) {
		if (par_drives[i] != NULL) {
			if (sim_set_drive (&sim, i, par_drives[i])) {
				return (1);
			}
		}
	}

	if ((prog_dos = sim_get_dos_full_name (&sim, optarg[0])) == NULL) {
		return (1);
	}

	if ((prog_host = sim_get_host_name (&sim, prog_dos)) == NULL) {
		return (1);
	}

	if (sim_init_env (&sim, prog_dos)) {
		return (1);
	}

	if (sim_exec (&sim, prog_host)) {
		fprintf (stderr, "%s: loading exe file failed (%s)\n", arg0, optarg[0]);
		return (1);
	}

	if (sim_init_args (&sim, (const char **) optarg + 1)) {
		fprintf (stderr, "%s: argument list too long\n", arg0);
		return (1);
	}

	sim_run (&sim);

	sim_free (&sim);

	return (0);
}
