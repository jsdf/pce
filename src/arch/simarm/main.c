/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/main.c                                       *
 * Created:     2004-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2022 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

#include "cmd_arm.h"
#include "simarm.h"

#include <lib/cfg.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/getopt.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/path.h>
#include <lib/sysdep.h>

#include <libini/libini.h>


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'c', 1, "config", "string", "Set the config file name [none]" },
	{ 'd', 1, "path", "string", "Add a directory to the search path" },
	{ 'i', 1, "ini-prefix", "string", "Add an ini string before the config file" },
	{ 'I', 1, "ini-append", "string", "Add an ini string after the config file" },
	{ 'l', 1, "log", "string", "Set the log file name [none]" },
	{ 'q', 0, "quiet", NULL, "Set the log level to error [no]" },
	{ 'r', 0, "run", NULL, "Start running immediately [no]" },
	{ 'v', 0, "verbose", NULL, "Set the log level to debug [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{  -1, 0, NULL, NULL, NULL }
};


unsigned             par_xlat = ARM_XLAT_CPU;

simarm_t             *par_sim = NULL;

unsigned             par_sig_int = 0;

ini_sct_t            *par_cfg = NULL;

static ini_strings_t par_ini_str;


static
void print_help (void)
{
	pce_getopt_help (
		"pce-simarm: ARM emulator",
		"usage: pce-simarm [options]",
		opts
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pce-simarm version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 1995-" PCE_YEAR " Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

static
void sim_log_banner (void)
{
	pce_log_inf (
		"pce-simarm version " PCE_VERSION_STR "\n"
		"Copyright (C) 2004-" PCE_YEAR " Hampa Hug <hampa@hampa.ch>\n"
	);
}

static
void sig_int (int s)
{
	fprintf (stderr, "\npce-simarm: sigint\n");
	fflush (stderr);

	par_sim->brk = PCE_BRK_STOP;
}

static
void sig_term (int s)
{
	fprintf (stderr, "\npce-simarm: sigterm\n");
	fflush (stderr);

	if (par_sim->brk == PCE_BRK_ABORT) {
		pce_set_fd_interactive (0, 1);
		exit (1);
	}

	par_sim->brk = PCE_BRK_ABORT;
}

static
void sig_segv (int s)
{
	fprintf (stderr, "pce-simarm: segmentation fault\n");

	if ((par_sim != NULL) && (par_sim->cpu != NULL)) {
		sarm_prt_state_cpu (par_sim->cpu, stderr);
	}

	fflush (stderr);

	pce_set_fd_interactive (0, 1);

	exit (1);
}

static
int cmd_get_sym (simarm_t *sim, const char *sym, unsigned long *val)
{
	if (arm_get_reg (sim->cpu, sym, val) == 0) {
		return (0);
	}

	return (1);
}

static
int cmd_set_sym (simarm_t *sim, const char *sym, unsigned long val)
{
	if (arm_set_reg (sim->cpu, sym, val) == 0) {
		return (0);
	}

	return (1);
}

static
unsigned char sim_get_mem8 (simarm_t *sim, unsigned long addr)
{
	unsigned char val;

	if (arm_get_mem8 (sim->cpu, addr, par_xlat, &val)) {
		val = 0xff;
	}

	return (val);
}

static
void sim_set_mem8 (simarm_t *sim, unsigned long addr, unsigned char val)
{
	if (arm_set_mem8 (sim->cpu, addr, par_xlat, val)) {
		; /* TLB miss */
	}
}

int main (int argc, char *argv[])
{
	int       r;
	char      **optarg;
	int       run;
	char      *cfg;
	ini_sct_t *sct;
	monitor_t mon;

	cfg = NULL;
	run = 0;

	pce_log_init();
	pce_log_add_fp (stderr, 0, MSG_INF);

	par_cfg = ini_sct_new (NULL);

	if (par_cfg == NULL) {
		return (1);
	}

	ini_str_init (&par_ini_str);

	while (1) {
		r = pce_getopt (argc, argv, &optarg, opts);

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
			cfg = optarg[0];
			break;

		case 'd':
			pce_path_set (optarg[0]);
			break;

		case 'i':
			if (ini_read_str (par_cfg, optarg[0])) {
				fprintf (stderr,
					"%s: error parsing ini string (%s)\n",
					argv[0], optarg[0]
				);
				return (1);
			}
			break;

		case 'I':
			ini_str_add (&par_ini_str, optarg[0], "\n", NULL);
			break;

		case 'l':
			pce_log_add_fname (optarg[0], MSG_DEB);
			break;

		case 'q':
			pce_log_set_level (stderr, MSG_ERR);
			break;

		case 'r':
			run = 1;
			break;

		case 'v':
			pce_log_set_level (stderr, MSG_DEB);
			break;

		case 0:
			fprintf (stderr, "%s: unknown option (%s)\n",
				argv[0], optarg[0]
			);
			return (1);

		default:
			return (1);
		}
	}

	sim_log_banner();

	if (pce_load_config (par_cfg, cfg)) {
		return (1);
	}

	sct = ini_next_sct (par_cfg, NULL, "simarm");

	if (sct == NULL) {
		sct = par_cfg;
	}

	if (ini_str_eval (&par_ini_str, sct, 1)) {
		return (1);
	}

	pce_path_ini (sct);

	par_sim = sarm_new (sct);

	signal (SIGINT, sig_int);
	signal (SIGTERM, sig_term);
	signal (SIGSEGV, sig_segv);

#ifdef SIGPIPE
	signal (SIGPIPE, SIG_IGN);
#endif

	pce_console_init (stdin, stdout);

	mon_init (&mon);
	mon_set_cmd_fct (&mon, sarm_do_cmd, par_sim);
	mon_set_msg_fct (&mon, sarm_set_msg, par_sim);
	mon_set_get_mem_fct (&mon, par_sim, sim_get_mem8);
	mon_set_set_mem_fct (&mon, par_sim, sim_set_mem8);
	mon_set_memory_mode (&mon, 0);

	cmd_init (par_sim, cmd_get_sym, cmd_set_sym);
	sarm_cmd_init (par_sim, &mon);

	sarm_reset (par_sim);

	if (run) {
		sarm_run (par_sim);
		if (par_sim->brk != PCE_BRK_ABORT) {
			fputs ("\n", stdout);
		}
	}
	else {
		pce_puts ("type 'h' for help\n");
	}

	mon_run (&mon);

	sarm_del (par_sim);

	mon_free (&mon);
	pce_console_done();
	pce_log_done();

	return (0);
}
