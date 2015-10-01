/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim405/main.c                                       *
 * Created:     2004-06-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2015 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

#include "cmd_ppc.h"
#include "sim405.h"

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
	{ 'p', 1, "cpu", "string", "Set the CPU model" },
	{ 'q', 0, "quiet", NULL, "Set the log level to error [no]" },
	{ 'r', 0, "run", NULL, "Start running immediately [no]" },
	{ 'v', 0, "verbose", NULL, "Set the log level to debug [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{  -1, 0, NULL, NULL, NULL }
};


char                 *par_cpu = NULL;

unsigned             par_xlat = P405_XLAT_CPU;

sim405_t             *par_sim = NULL;

unsigned             par_sig_int = 0;

ini_sct_t            *par_cfg = NULL;

static ini_strings_t par_ini_str;


static
void print_help (void)
{
	pce_getopt_help (
		"pce-sim405: PowerPC 405 emulator",
		"usage: pce-sim405 [options]",
		opts
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pce-sim405 version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2004-2015 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

static
void s405_log_banner (void)
{
	pce_log (MSG_MSG,
		"pce-sim405 version " PCE_VERSION_STR "\n"
		"Copyright (C) 2004-2015 Hampa Hug <hampa@hampa.ch>\n"
	);
}

static
void sig_int (int s)
{
	signal (SIGINT, sig_int);

	par_sig_int = 1;
}

static
void sig_terminate (int s)
{
	fprintf (stderr, "pce-sim405: signal %d\n", s);

	if ((par_sim != NULL) && (par_sim->ppc != NULL)) {
		fprintf (stderr, "  PC=%08lX\n",
			(unsigned long) p405_get_pc (par_sim->ppc)
		);
	}

	fflush (stderr);

	pce_set_fd_interactive (0, 1);

	exit (1);
}

static
int cmd_get_sym (sim405_t *sim, const char *sym, unsigned long *val)
{
	if (p405_get_reg (sim->ppc, sym, val) == 0) {
		return (0);
	}

	return (1);
}

static
int cmd_set_sym (sim405_t *sim, const char *sym, unsigned long val)
{
	if (p405_set_reg (sim->ppc, sym, val) == 0) {
		return (0);
	}

	return (1);
}

static
unsigned char s405_get_mem8 (sim405_t *sim, unsigned long addr)
{
	unsigned char val;

	if (p405_get_xlat8 (sim->ppc, addr, par_xlat, &val)) {
		val = 0xff;
	}

	return (val);
}

static
void s405_set_mem8 (sim405_t *sim, unsigned long addr, unsigned char val)
{
	if (p405_set_xlat8 (sim->ppc, addr, par_xlat, val)) {
		; /* TLB miss */
	}
}

static
int pce_load_config (ini_sct_t *ini, const char *fname)
{
	if (fname == NULL) {
		return (0);
	}

	pce_log_tag (MSG_INF, "CONFIG:", "file=\"%s\"\n", fname);

	if (ini_read (par_cfg, fname)) {
		pce_log (MSG_ERR, "*** loading config file failed\n");
		return (1);
	}

	return (0);
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

		case 'p':
			ini_str_add (&par_ini_str, "cpu.model = \"",
				optarg[0], "\"\n"
			);
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

	s405_log_banner();

	if (pce_load_config (par_cfg, cfg)) {
		return (1);
	}

	sct = ini_next_sct (par_cfg, NULL, "sim405");

	if (sct == NULL) {
		sct = par_cfg;
	}

	if (ini_str_eval (&par_ini_str, sct, 1)) {
		return (1);
	}

	pce_path_ini (sct);

	par_sim = s405_new (sct);

	signal (SIGINT, sig_int);
	signal (SIGTERM, sig_terminate);
	signal (SIGSEGV, sig_terminate);

#ifdef SIGPIPE
	signal (SIGPIPE, SIG_IGN);
#endif

	pce_console_init (stdin, stdout);

	mon_init (&mon);
	mon_set_cmd_fct (&mon, ppc_do_cmd, par_sim);
	mon_set_msg_fct (&mon, s405_set_msg, par_sim);
	mon_set_get_mem_fct (&mon, par_sim, s405_get_mem8);
	mon_set_set_mem_fct (&mon, par_sim, s405_set_mem8);
	mon_set_memory_mode (&mon, 0);

	cmd_init (par_sim, cmd_get_sym, cmd_set_sym);
	ppc_cmd_init (par_sim, &mon);

	s405_reset (par_sim);

	if (run) {
		ppc_run (par_sim);
		if (par_sim->brk != PCE_BRK_ABORT) {
			fputs ("\n", stdout);
		}
	}
	else {
		pce_puts ("type 'h' for help\n");
	}

	mon_run (&mon);

	s405_del (par_sim);

	mon_free (&mon);
	pce_console_done();
	pce_log_done();

	return (0);
}
