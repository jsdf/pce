/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/main.c                                        *
 * Created:     2012-06-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "cmd.h"
#include "msg.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#ifdef PCE_ENABLE_SDL
#include <SDL.h>
#endif

#include <lib/console.h>
#include <lib/getopt.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/path.h>
#include <lib/sysdep.h>


const char           *par_terminal = NULL;
const char           *par_video = NULL;

monitor_t            par_mon;

rc759_t              *par_sim = NULL;

ini_sct_t            *par_cfg = NULL;

static ini_strings_t par_ini_str;


static pce_option_t opts[] = {
	{ '?', 0, "help", NULL, "Print usage information" },
	{ 'c', 1, "config", "string", "Set the config file name [none]" },
	{ 'd', 1, "path", "string", "Add a directory to the search path" },
	{ 'g', 1, "video", "string", "Set the video device" },
	{ 'i', 1, "ini-prefix", "string", "Add an ini string before the config file" },
	{ 'I', 1, "ini-append", "string", "Add an ini string after the config file" },
	{ 'l', 1, "log", "string", "Set the log file name [none]" },
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
		"pce-rc759: RC759 Piccoline emulator",
		"usage: pce-rc759 [options]",
		opts
	);

	fflush (stdout);
}

static
void print_version (void)
{
	fputs (
		"pce-rc759 version " PCE_VERSION_STR
		"\n\n"
		"Copyright (C) 2012 Hampa Hug <hampa@hampa.ch>\n",
		stdout
	);

	fflush (stdout);
}

static
void rc759_log_banner (void)
{
	pce_log (MSG_MSG,
		"pce-rc759 version " PCE_VERSION_STR "\n"
		"Copyright (C) 2012 Hampa Hug <hampa@hampa.ch>\n"
	);
}

static
void sig_int (int s)
{
	fprintf (stderr, "pce-rc759: sigint\n");
	fflush (stderr);

	if (par_sim->brk == 0) {
		par_sim->brk = PCE_BRK_STOP;
	}
	else {
		par_sim->brk = PCE_BRK_ABORT;
	}
}

static
void sig_term (int s)
{
	fprintf (stderr, "pce-rc759: sigterm\n");
	fflush (stderr);

	par_sim->brk = PCE_BRK_ABORT;
}

static
void sig_segv (int s)
{
	fprintf (stderr, "pce-rc759: segmentation fault\n");
	fflush (stderr);

	if ((par_sim != NULL) && (par_sim->cpu != NULL)) {
		print_state_cpu (par_sim->cpu);
	}

	pce_set_fd_interactive (0, 1);

	exit (1);
}

static
void sim_atexit (void)
{
	pce_set_fd_interactive (0, 1);
}

static
int cmd_get_sym (rc759_t *sim, const char *sym, unsigned long *val)
{
	if (e86_get_reg (sim->cpu, sym, val) == 0) {
		return (0);
	}

	if (strcmp (sym, "intv") == 0) {
		*val = sim->current_int;
		return (0);
	}

	return (1);
}

static
int cmd_set_sym (rc759_t *sim, const char *sym, unsigned long val)
{
	if (e86_set_reg (sim->cpu, sym, val) == 0) {
		return (0);
	}

	return (1);
}

void sim_log_deb (const char *msg, ...)
{
	va_list        va;
	unsigned short cs, ip;

	if (par_sim == NULL) {
		cs = 0;
		ip = 0;
	}
	else {
		cs = e86_get_cs (par_sim->cpu);
		ip = e86_get_ip (par_sim->cpu);
	}

	pce_log (MSG_DEB, "[%04X:%04X] ", cs, ip);

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

		case 'c':
			cfg = optarg[0];
			break;

		case 'd':
			pce_path_set (optarg[0]);
			break;

		case 'g':
			par_video = optarg[0];
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

		case 'R':
			nomon = 1;
			break;

		case 't':
			par_terminal = optarg[0];
			break;

		case 's':
			ini_str_add (&par_ini_str, "cpu.speed = ",
				optarg[0], "\n"
			);
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

	rc759_log_banner();

	if (pce_load_config (par_cfg, cfg)) {
		return (1);
	}

	sct = ini_next_sct (par_cfg, NULL, "rc759");

	if (sct == NULL) {
		sct = par_cfg;
	}

	if (ini_str_eval (&par_ini_str, sct, 1)) {
		return (1);
	}

	atexit (sim_atexit);

#ifdef PCE_ENABLE_SDL
	SDL_Init (0);
#endif

	pce_path_ini (sct);

	par_sim = rc759_new (sct);

	signal (SIGINT, sig_int);
	signal (SIGTERM, sig_term);
	signal (SIGSEGV, sig_segv);

	pce_console_init (stdin, stdout);

	mon_init (&par_mon);
	mon_set_cmd_fct (&par_mon, rc759_cmd, par_sim);
	mon_set_msg_fct (&par_mon, rc759_set_msg, par_sim);
	mon_set_get_mem_fct (&par_mon, par_sim->mem, mem_get_uint8);
	mon_set_set_mem_fct (&par_mon, par_sim->mem, mem_set_uint8);
	mon_set_memory_mode (&par_mon, 1);

	cmd_init (par_sim, cmd_get_sym, cmd_set_sym);
	rc759_cmd_init (par_sim, &par_mon);

	rc759_reset (par_sim);

	if (nomon) {
		while (par_sim->brk != PCE_BRK_ABORT) {
			rc759_run (par_sim);
		}
	}
	else if (run) {
		rc759_run (par_sim);

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

	rc759_del (par_sim);

#ifdef PCE_ENABLE_SDL
	SDL_Quit();
#endif

	mon_free (&par_mon);
	pce_console_done();
	pce_log_done();

	return (0);
}
