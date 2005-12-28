/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/main.c                                     *
 * Created:       2004-06-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-09 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2005 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <termios.h>

#include "main.h"


char     *par_cpu = NULL;

unsigned par_xlat = P405_XLAT_CPU;

sim405_t *par_sim = NULL;

unsigned par_sig_int = 0;


static
void prt_help (void)
{
	fputs (
		"usage: sim405 [options]\n"
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
		"pce sim405 version " PCE_VERSION_STR
		" (" PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
		"Copyright (C) 1995-2004 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

void sig_int (int s)
{
	signal (SIGINT, sig_int);

	par_sig_int = 1;
}

void sig_segv (int s)
{
	fprintf (stderr, "pce: segmentation fault\n");

	if ((par_sim != NULL) && (par_sim->ppc != NULL)) {
		fprintf (stderr, "  PC=%08lX\n", (unsigned long) p405_get_pc (par_sim->ppc));
	}

	fflush (stderr);

	exit (1);
}

static
int cmd_match_sym (cmd_t *cmd, unsigned long *val)
{
	uint32_t *reg;

	if (ppc_match_reg (cmd, par_sim, &reg)) {
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

void prt_state (sim405_t *sim, FILE *fp, const char *str)
{
	cmd_t cmd;

	cmd_set_str (&cmd, str);

	if (cmd_match_eol (&cmd)) {
		return;
	}

	while (!cmd_match_eol (&cmd)) {
		if (cmd_match (&cmd, "ppc")) {
			prt_state_ppc (sim->ppc, fp);
		}
		else if (cmd_match (&cmd, "spr")) {
			prt_state_spr (sim->ppc, fp);
		}
		else if (cmd_match (&cmd, "uic")) {
			prt_state_uic (&sim->uic, fp);
		}
		else {
			printf ("unknown component (%s)\n", cmd_get_str (&cmd));
			return;
		}
	}
}

static
void prt_prompt (sim405_t *sim, FILE *fp)
{
	unsigned long long clk;

	clk = s405_get_clkcnt (sim);

	fputs ("\x1b[0;37;40m", fp);
	fprintf (fp, "[%08llX] ", clk);
	fflush (fp);
}

void pce_set_fd (int fd, int interactive)
{
	static int            sios_ok = 0;
	static struct termios sios;
	struct termios        tios;

	if (sios_ok == 0) {
		tcgetattr (fd, &sios);
		sios_ok = 1;
	}

	if (interactive) {
		tcsetattr (fd, TCSANOW, &sios);
	}
	else {
		tios = sios;

		tios.c_lflag &= ~(ICANON | ECHO);
		tios.c_cc[VMIN] = 1;
		tios.c_cc[VTIME] = 0;

		tcsetattr (fd, TCSANOW, &tios);
	}
}

void pce_start (void)
{
	pce_set_fd (0, 0);
	par_sim->brk = 0;
}

void pce_stop (void)
{
	pce_set_fd (0, 1);
}

void pce_run (void)
{
	pce_start();

	while (1) {
		s405_clock (par_sim, 64);

		if (par_sim->brk) {
			break;
		}
	}

	pce_stop();
}


int do_cmd (sim405_t *sim)
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
			ppc_do_cmd (&cmd, sim);
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
			pce_log (MSG_INF, "pce:\tusing config file '%s'\n", fname);
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
		"pce sim405 version " PCE_VERSION_STR
		" (compiled " PCE_CFG_DATE " " PCE_CFG_TIME ")\n"
		"Copyright (C) 1995-2004 Hampa Hug <hampa@hampa.ch>\n"
	);

	ini = pce_load_config (cfg);
	if (ini == NULL) {
		pce_log (MSG_ERR, "loading config file failed\n");
		return (1);
	}

	sct = ini_sct_find_sct (ini, "sim405");
	if (sct == NULL) {
		pce_log (MSG_ERR, "section 'sim405' not found in config file\n");
		return (1);
	}

	par_sim = s405_new (sct);

	signal (SIGINT, sig_int);
	signal (SIGSEGV, sig_segv);

	cmd_init (stdin, stdout, &cmd_match_sym);
	ppc_cmd_init (par_sim);

	s405_reset (par_sim);

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

	s405_del (par_sim);

	pce_log_done();

	return (0);
}
