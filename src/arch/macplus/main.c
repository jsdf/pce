/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/main.c                                      *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include "cmd_68k.h"
#include "macplus.h"
#include "msg.h"
#include "sony.h"

#include <stdarg.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>

#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/getopt.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/path.h>
#include <lib/sysdep.h>

#include <libini/libini.h>

#ifdef PCE_ENABLE_SDL
#include <SDL.h>
#endif


const char *par_terminal = NULL;

unsigned   par_disk_delay_valid = 0;
unsigned   par_disk_delay[SONY_DRIVES];

macplus_t  *par_sim = NULL;

unsigned   par_sig_int = 0;

monitor_t  par_mon;

ini_sct_t  *par_cfg = NULL;

static ini_strings_t par_ini_str;


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'b', 1, "disk-delay-1", "delay", "Set the disk delay for drive 1 [30]" },
	{ 'B', 2, "disk-delay", "drive delay", "Set the disk delay [30]" },
	{ 'c', 1, "config", "string", "Set the config file name [none]" },
	{ 'd', 1, "path", "string", "Add a directory to the search path" },
	{ 'i', 1, "ini-prefix", "string", "Add an ini string before the config file" },
	{ 'I', 1, "ini-append", "string", "Add an ini string after the config file" },
	{ 'l', 1, "log", "string", "Set the log file name [none]" },
	{ 'p', 1, "cpu", "string", "Set the CPU model" },
	{ 'q', 0, "quiet", NULL, "Set the log level to error [no]" },
	{ 'r', 0, "run", NULL, "Start running immediately [no]" },
	{ 'R', 0, "no-monitor", NULL, "Never stop running [no]" },
	{ 's', 1, "speed", "int", "Set the CPU speed" },
	{ 't', 1, "terminal", "string", "Set the terminal device" },
	{ 'v', 0, "verbose", NULL, "Set the log level to debug [no]" },
	{ 'V', 0, "version", NULL, "Print version information" },
	{  -1, 0, NULL, NULL, NULL }
};


static
void print_help (void)
{
	pce_getopt_help (
		"pce-macplus: Macintosh Plus emulator",
		"usage: pce-macplus [options]",
		opts
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pce-macplus version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2007-2010 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

static
void mac_log_banner (void)
{
	pce_log (MSG_INF,
		"pce-macplus version " PCE_VERSION_STR "\n"
		"Copyright (C) 2007-2010 Hampa Hug <hampa@hampa.ch>\n"
	);
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
	int       run, nomon;
	unsigned  drive;
	char      *cfg;
	ini_sct_t *sct;

	cfg = NULL;
	run = 0;
	nomon = 0;

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

		case 'b':
			par_disk_delay_valid |= 1;
			par_disk_delay[0] = (unsigned) strtoul (optarg[0], NULL, 0);
			break;

		case 'B':
			drive = strtoul (optarg[0], NULL, 0);

			if ((drive < 1) || (drive >= SONY_DRIVES)) {
				fprintf (stderr, "%s: bad drive number (%u)\n",
					argv[0], drive
				);
				return (1);
			}

			drive -= 1;

			par_disk_delay_valid |= 1U << drive;
			par_disk_delay[drive] = (unsigned) strtoul (optarg[1], NULL, 0);
			break;

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

		case 'R':
			nomon = 1;
			break;

		case 's':
			ini_str_add (&par_ini_str, "cpu.speed = ",
				optarg[0], "\n"
			);
			break;

		case 't':
			par_terminal = optarg[0];
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

	mac_log_banner();

	if (pce_load_config (par_cfg, cfg)) {
		return (1);
	}

	sct = ini_next_sct (par_cfg, NULL, "macplus");

	if (sct == NULL) {
		sct = par_cfg;
	}

	if (ini_str_eval (&par_ini_str, sct, 1)) {
		return (1);
	}

	atexit (mac_atexit);

#ifdef PCE_ENABLE_SDL
	SDL_Init (0);
#endif

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

#ifdef PCE_ENABLE_SDL
	SDL_Quit();
#endif

	mon_free (&par_mon);
	pce_console_done();
	pce_log_done();

	return (0);
}
