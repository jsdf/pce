/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/main.c                                      *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2010 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdarg.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>

#ifdef PCE_ENABLE_SDL
#include <SDL.h>
#endif


const char *par_cpu = NULL;
const char *par_terminal = NULL;

unsigned   par_disk_delay_valid = 0;
unsigned   par_disk_delay[SONY_DRIVES];

macplus_t  *par_sim = NULL;

unsigned   par_sig_int = 0;

monitor_t  par_mon;


static
void prt_help (void)
{
	fputs (
		"usage: pce-macplus [options]\n"
		"  -b, --disk-delay-1 delay      Set the disk delay for drive 1 [30]\n"
		"  -B, --disk-delay drive delay  Set the disk delay [30]\n"
		"  -c, --config string           Set the config file\n"
		"  -d, --path string             Add a directory to the search path\n"
		"  -l, --log string              Set the log file [none]\n"
		"  -p, --cpu string              Set the cpu model [68000]\n"
		"  -q, --quiet                   Quiet operation [no]\n"
		"  -r, --run                     Start running immediately [no]\n"
		"  -R, --no-monitor              Never stop running [no]\n"
		"  -t, --terminal string         Set the terminal type\n"
		"  -v, --verbose                 Verbose operation [no]\n",
		stdout
	);

	fflush (stdout);
}

static
void prt_version (void)
{
	fputs (
		"pce-macplus version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2007-2010 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

void sig_int (int s)
{
	par_sig_int = 1;
}

void sig_segv (int s)
{
	pce_set_fd_interactive (0, 1);

	fprintf (stderr, "pce-macplus: segmentation fault\n");
	fflush (stderr);

	exit (1);
}

void sig_term (int s)
{
	pce_set_fd_interactive (0, 1);

	fprintf (stderr, "pce-macplus: signal %d\n", s);
	fflush (stderr);

	exit (1);
}

static
void mac_atexit (void)
{
	pce_set_fd_interactive (0, 1);
}

static
int cmd_get_sym (macplus_t *sim, const char *sym, unsigned long *val)
{
	if (e68_get_reg (sim->cpu, sym, val) == 0) {
		return (0);
	}

	return (1);
}

static
int cmd_set_sym (macplus_t *sim, const char *sym, unsigned long val)
{
	if (e68_set_reg (sim->cpu, sym, val) == 0) {
		return (0);
	}

	return (1);
}

void mac_log_deb (const char *msg, ...)
{
	va_list       va;
	unsigned long pc;

	if (par_sim != NULL) {
		pc = e68_get_pc (par_sim->cpu);
	}
	else {
		pc = 0;
	}

	pce_log (MSG_DEB, "[%06lX] ", pc & 0xffffff);

	va_start (va, msg);
	pce_log_va (MSG_DEB, msg, va);
	va_end (va);
}

static
ini_sct_t *pce_load_config (const char *fname)
{
	ini_sct_t *ini;

	if (fname != NULL) {
		ini = ini_read (fname);
		if (ini != NULL) {
			pce_log_tag (MSG_INF, "PCE:",
				"using config file '%s'\n", fname
			);
			return (ini);
		}
	}

	return (NULL);
}

static
int str_isarg (const char *str, const char *arg1, const char *arg2)
{
	if (*str != '-') {
		return (0);
	}

	if (arg1 != NULL) {
		if (str[0] == '-') {
			if (strcmp (str + 1, arg1) == 0) {
				return (1);
			}
		}
	}

	if (arg2 != NULL) {
		if ((str[0] == '-') && (str[1] == '-')) {
			if (strcmp (str + 2, arg2) == 0) {
				return (1);
			}
		}
	}

	return (0);
}

int main (int argc, char *argv[])
{
	int       i;
	int       run, nomon;
	char      *cfg;
	ini_sct_t *ini, *sct;

	if (argc == 2) {
		if (str_isarg (argv[1], "?", "help")) {
			prt_help();
			return (0);
		}
		else if (str_isarg (argv[1], "V", "version")) {
			prt_version();
			return (0);
		}
	}

	cfg = NULL;
	run = 0;
	nomon = 0;

	pce_log_init();
	pce_log_add_fp (stderr, 0, MSG_INF);

	i = 1;
	while (i < argc) {
		if (str_isarg (argv[i], "b", "disk-delay-1")) {
			i += 1;
			if (i >= argc) {
				fprintf (stderr, "%s: missing delay\n", argv[0]);
				return (1);
			}

			par_disk_delay_valid |= 1;
			par_disk_delay[0] = (unsigned) strtoul (argv[i], NULL, 0);
		}
		else if (str_isarg (argv[i], "B", "disk-delay")) {
			unsigned drive;

			if ((i + 2) >= argc) {
				fprintf (stderr, "%s: missing delay\n", argv[0]);
				return (1);
			}

			drive = strtoul (argv[i + 1], NULL, 0);

			if ((drive < 1) || (drive >= SONY_DRIVES)) {
				fprintf (stderr, "%s: bad drive number (%u)\n",
					argv[0], drive
				);
				return (1);
			}

			drive -= 1;

			par_disk_delay_valid |= 1U << drive;
			par_disk_delay[drive] = (unsigned) strtoul (argv[i + 2], NULL, 0);

			i += 2;
		}
		else if (str_isarg (argv[i], "c", "config")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}
			cfg = argv[i];
		}
		else if (str_isarg (argv[i], "d", "path")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			pce_path_set (argv[i]);
		}
		else if (str_isarg (argv[i], "l", "log")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}
			pce_log_add_fname (argv[i], MSG_DEB);
		}
		else if (str_isarg (argv[i], "p", "cpu")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			par_cpu = argv[i];
		}
		else if (str_isarg (argv[i], "q", "quiet")) {
			pce_log_set_level (stderr, MSG_ERR);
		}
		else if (str_isarg (argv[i], "r", "run")) {
			run = 1;
		}
		else if (str_isarg (argv[i], "R", "no-monitor")) {
			nomon = 1;
		}
		else if (str_isarg (argv[i], "t", "terminal")) {
			i += 1;
			if (i >= argc) {
				return (1);
			}

			par_terminal = argv[i];
		}
		else if (str_isarg (argv[i], "v", "verbose")) {
			pce_log_set_level (stderr, MSG_DEB);
		}
		else {
			printf ("%s: unknown option (%s)\n", argv[0], argv[i]);
			return (1);
		}

		i += 1;
	}

	pce_log (MSG_INF,
		"pce-macplus version " PCE_VERSION_STR "\n"
		"Copyright (C) 2007-2010 Hampa Hug <hampa@hampa.ch>\n"
	);

	ini = pce_load_config (cfg);
	if (ini == NULL) {
		pce_log (MSG_ERR, "loading config file failed\n");
		return (1);
	}

	sct = ini_next_sct (ini, NULL, "macplus");
	if (sct == NULL) {
		sct = ini;
	}

	atexit (mac_atexit);

	pce_path_ini (sct);

	signal (SIGINT, &sig_int);
	signal (SIGSEGV, &sig_segv);
	signal (SIGTERM, &sig_term);

	pce_console_init (stdin, stdout);

	par_sim = mac_new (sct);

	cmd_init (par_sim, cmd_get_sym, cmd_set_sym);
	mac_cmd_init (par_sim);

	mon_init (&par_mon);
	mon_set_cmd_fct (&par_mon, mac_cmd, par_sim);
	mon_set_msg_fct (&par_mon, mac_set_msg, par_sim);

	mac_reset (par_sim);

	if (nomon) {
		while (par_sim->brk != PCE_BRK_ABORT) {
			mac_run (par_sim);
		}
	}
	else if (run) {
		mac_run (par_sim);
		if (par_sim->brk != PCE_BRK_ABORT) {
			pce_puts ("\n");
		}
	}
	else {
		pce_puts ("type 'h' for help\n");
	}

	if (par_sim->brk != PCE_BRK_ABORT) {
		mon_run (&par_mon);
	}

	mac_del (par_sim);

	mon_free (&par_mon);
	pce_console_done();
	pce_log_done();

	return (0);
}
