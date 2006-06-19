/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/simarm/main.c                                     *
 * Created:       2004-11-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2006-06-19 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                       *
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

/* $Id$ */


#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <signal.h>

#include "main.h"


char     *par_cpu = NULL;

unsigned par_xlat = ARM_XLAT_CPU;

simarm_t *par_sim = NULL;

unsigned par_sig_int = 0;


static
void prt_help (void)
{
	fputs (
		"usage: simarm [options]\n"
		"  --help                 Print usage information\n"
		"  --version              Print version information\n"
		"  -c, --config string    Set the config file\n"
		"  -l, --log string       Set the log file\n"
		"  -p, --cpu string       Set the cpu model\n"
		"  -q, --quiet            Quiet operation [no]\n"
		"  -r, --run              Start running immediately\n"
		"  -v, --verbose          Verbose operation [no]\n",
		stdout
	);

	fflush (stdout);
}

static
void prt_version (void)
{
	fputs (
		"pce simarm version " PCE_VERSION_STR
		" (" PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
		"Copyright (C) 1995-2004 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

void sig_int (int s)
{
	signal (s, sig_int);

	par_sig_int = 1;
}

void sig_segv (int s)
{
	fprintf (stderr, "pce: segmentation fault\n");
	fprintf (stderr, "  PC=%08lX\n", (unsigned long) arm_get_pc (par_sim->cpu));
	fflush (stderr);

	exit (1);
}

static
int cmd_match_sym (cmd_t *cmd, unsigned long *val)
{
	uint32_t *reg;

	if (sarm_match_reg (cmd, par_sim, &reg)) {
		*val = *reg;
		return (1);
	}

	return (0);
}

void prt_sep (FILE *fp, const char *str, ...)
{
	unsigned i;
	va_list  va;

	fputs ("-", fp);
	i = 1;

	va_start (va, str);
	i += vfprintf (fp, str, va);
	va_end (va);

	while (i < 78) {
		fputc ('-', fp);
		i += 1;
	}

	fputs ("\n", fp);
}

void prt_state (simarm_t *sim, FILE *fp, const char *str)
{
	cmd_t cmd;

	cmd_set_str (&cmd, str);

	if (cmd_match_eol (&cmd)) {
		return;
	}

	while (!cmd_match_eol (&cmd)) {
		if (cmd_match (&cmd, "cpu")) {
			sarm_prt_state_cpu (sim->cpu, fp);
		}
		else if (cmd_match (&cmd, "mmu")) {
			sarm_prt_state_mmu (sim->cpu, fp);
		}
		else if (cmd_match (&cmd, "timer")) {
			sarm_prt_state_timer (sim->timer, fp);
		}
		else if (cmd_match (&cmd, "intc")) {
			sarm_prt_state_intc (sim, fp);
		}
		else {
			printf ("unknown component (%s)\n", cmd_get_str (&cmd));
			return;
		}
	}
}

static
void prt_prompt (simarm_t *sim, FILE *fp)
{
	fputs ("\x1b[0;37;40m-", fp);
	fflush (fp);
}

void pce_start (void)
{
	pce_set_fd_interactive (0, 0);
	par_sim->brk = 0;
}

void pce_stop (void)
{
	pce_set_fd_interactive (0, 1);
}

void pce_run (void)
{
	pce_start();

	while (1) {
		sarm_clock (par_sim, 16);

		if (par_sim->brk) {
			break;
		}
	}

	pce_stop();
}

static
int do_cmd (simarm_t *sim)
{
	cmd_t cmd;

	while (1) {
		prt_prompt (sim, stdout);
		fflush (stdout);

		cmd_get (&cmd);

		if (cmd_match_eol (&cmd)) {
			;
		}
		else if (cmd_match (&cmd, "q")) {
			break;
		}
		else {
			sarm_do_cmd (&cmd, sim);
		}

		if (sim->brk == PCE_BRK_ABORT) {
			break;
		}
	};

	return (0);
}

static
ini_sct_t *pce_load_config (const char *fname)
{
	ini_sct_t *ini;

	if (fname != NULL) {
		ini = ini_read (fname);
		if (ini != NULL) {
			pce_log (MSG_INF, "pce: using config file '%s'\n", fname);
			return (ini);
		}
	}

	return (NULL);
}

int str_isarg1 (const char *str, const char *arg)
{
	if (strcmp (str, arg) == 0) {
		return (1);
	}

	return (0);
}

int str_isarg2 (const char *str, const char *arg1, const char *arg2)
{
	if (strcmp (str, arg1) == 0) {
		return (1);
	}

	if (strcmp (str, arg2) == 0) {
		return (1);
	}

	return (0);
}

int main (int argc, char *argv[])
{
	int       i;
	int       run;
	char      *cfg;
	ini_sct_t *ini, *sct;

	if (argc == 2) {
		if (str_isarg1 (argv[1], "--help")) {
			prt_help();
			return (0);
		}
		else if (str_isarg1 (argv[1], "--version")) {
			prt_version();
			return (0);
		}
	}

	cfg = NULL;
	run = 0;

	pce_log_init();
	pce_log_add_fp (stderr, 0, MSG_INF);

	i = 1;
	while (i < argc) {
		if (str_isarg2 (argv[i], "-v", "--verbose")) {
			pce_log_set_level (stderr, MSG_DEB);
		}
		else if (str_isarg2 (argv[i], "-q", "--quiet")) {
			pce_log_set_level (stderr, MSG_ERR);
		}
		else if (str_isarg2 (argv[i], "-c", "--config")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}
			cfg = argv[i];
		}
		else if (str_isarg2 (argv[i], "-l", "--log")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}
			pce_log_add_fname (argv[i], MSG_DEB);
		}
		else if (str_isarg2 (argv[i], "-p", "--cpu")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			par_cpu = argv[i];
		}
		else if (str_isarg2 (argv[i], "-r", "--run")) {
			run = 1;
		}
		else {
			printf ("%s: unknown option (%s)\n", argv[0], argv[i]);
			return (1);
		}

		i += 1;
	}

	pce_log (MSG_INF,
		"pce simarm version " PCE_VERSION_STR
		" (compiled " PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
		"Copyright (C) 1995-2004 Hampa Hug <hampa@hampa.ch>\n"
	);

	ini = pce_load_config (cfg);
	if (ini == NULL) {
		pce_log (MSG_ERR, "loading config file failed\n");
		return (1);
	}

	sct = ini_sct_find_sct (ini, "simarm");
	if (sct == NULL) {
		sct = ini;
	}

	par_sim = sarm_new (sct);

	signal (SIGINT, sig_int);
	signal (SIGSEGV, sig_segv);
	signal (SIGPIPE, SIG_IGN);

	cmd_init (stdin, stdout, &cmd_match_sym);
	sarm_cmd_init (par_sim);

	sarm_reset (par_sim);

	if (run) {
		pce_run();
		if (par_sim->brk != 2) {
			fputs ("\n", stdout);
			do_cmd (par_sim);
		}
	}
	else {
		pce_log (MSG_INF, "type 'h' for help\n");
		do_cmd (par_sim);
	}

	sarm_del (par_sim);

	pce_log_done();

	return (0);
}
