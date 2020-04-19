/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/cmd.c                                       *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "atarist.h"
#include "cmd.h"
#include "dma.h"

#include <string.h>

#include <lib/console.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/sysdep.h>


#ifndef DEBUG_BIOS
#define DEBUG_BIOS 1
#endif
#include <emscripten.h>


mon_cmd_t par_cmd[] = {
	{ "c", "[cnt]", "clock" },
	{ "gb", "[addr..]", "run with breakpoints at addr" },
	{ "ge", "[exception]", "run until exception" },
	{ "g", "", "run" },
	{ "halt", "[val]", "set halt state [2]" },
	{ "hm", "", "print help on messages" },
	{ "p", "[cnt]", "execute cnt instructions, skip calls [1]" },
	{ "reset", "", "reset" },
	{ "rte", "", "execute to next rte" },
	{ "r", "reg [val]", "get or set a register" },
	{ "s", "[what]", "print status (cpu|mem)" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[[-]addr [cnt]]", "disassemble" }
};

unsigned par_cmd_cnt = sizeof (par_cmd) / sizeof (par_cmd[0]);


static
void st_dasm_str (char *dst, e68_dasm_t *op, int opcode)
{
	unsigned   i, n;
	char       tmp[256];
	const char *ins;

	strcpy (dst, "");

	if (opcode) {
		for (i = 0; i < op->irn; i++) {
			sprintf (tmp, "%04X ", (unsigned) op->ir[i]);
			strcat (dst, tmp);
		}

		for (i = op->irn; i < 4; i++) {
			strcat (dst, "     ");
		}
	}

	if (op->flags & E68_DFLAG_PRIV) {
		strcat (dst, "*");
	}
	else if (op->flags & E68_DFLAG_CALL) {
		strcat (dst, ">");
	}
	else if (op->flags & E68_DFLAG_RTS) {
		strcat (dst, "<");
	}
	else {
		strcat (dst, " ");
	}

	ins = op->op;

	switch (op->argn) {
	case 0:
		sprintf (tmp, "%s", ins);
		break;

	case 1:
		sprintf (tmp, "%-8s %s", ins, op->arg1);
		break;

	case 2:
		sprintf (tmp, "%-8s %s, %s", ins, op->arg1, op->arg2);
		break;

	case 3:
		sprintf (tmp, "%-8s %s, %s, %s",
			ins, op->arg1, op->arg2, op->arg3
		);
		break;

	default:
		strcpy (tmp, "---");
		break;
	}

	strcat (dst, tmp);

	if (op->comm[0] != 0) {
		n = strlen (dst);

		dst += n;

		while (n < 50) {
			*(dst++) = ' ';
			n += 1;
		}

		strcpy (dst, "; ");
		strcat (dst, op->comm);
	}
}

void st_print_state_cpu (atari_st_t *sim)
{
	e68000_t   *c;
	e68_dasm_t op;
	char       str[256];

	pce_prt_sep ("68000");

	c = sim->cpu;

	pce_printf ("SR=%04X      [%c%c %c%c%c%c%c]   EX=%02X(%-4s)  TA=%04X       IL=%X/%X\n",
		(unsigned) e68_get_sr (c),
		(e68_get_sr_t (c)) ? 'T' : '-',
		(e68_get_sr_s (c)) ? 'S' : '-',
		(e68_get_sr_x (c)) ? 'X' : '-',
		(e68_get_sr_n (c)) ? 'N' : '-',
		(e68_get_sr_z (c)) ? 'Z' : '-',
		(e68_get_sr_v (c)) ? 'V' : '-',
		(e68_get_sr_c (c)) ? 'C' : '-',
		e68_get_exception (c),
		e68_get_exception_name (c),
		e68_get_last_trap_a (c),
		e68_get_ipl (c),
		e68_get_iml (c)
	);

	pce_printf ("D0=%08lX  D4=%08lX  A0=%08lX  A4=%08lX   PC=%08lX  PC1=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 0),
		(unsigned long) e68_get_dreg32 (c, 4),
		(unsigned long) e68_get_areg32 (c, 0),
		(unsigned long) e68_get_areg32 (c, 4),
		(unsigned long) e68_get_pc (c),
		(unsigned long) e68_get_last_pc (c, 0)
	);

	pce_printf ("D1=%08lX  D5=%08lX  A1=%08lX  A5=%08lX  PPC=%08lX  PC2=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 1),
		(unsigned long) e68_get_dreg32 (c, 5),
		(unsigned long) e68_get_areg32 (c, 1),
		(unsigned long) e68_get_areg32 (c, 5),
		e68_get_ir_pc (c),
		(unsigned long) e68_get_last_pc (c, 1)
	);

	pce_printf ("D2=%08lX  D6=%08lX  A2=%08lX  A6=%08lX  USP=%08lX  PC3=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 2),
		(unsigned long) e68_get_dreg32 (c, 6),
		(unsigned long) e68_get_areg32 (c, 2),
		(unsigned long) e68_get_areg32 (c, 6),
		(unsigned long) e68_get_usp (c),
		(unsigned long) e68_get_last_pc (c, 2)
	);

	pce_printf ("D3=%08lX  D7=%08lX  A3=%08lX  A7=%08lX  SSP=%08lX  PC4=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 3),
		(unsigned long) e68_get_dreg32 (c, 7),
		(unsigned long) e68_get_areg32 (c, 3),
		(unsigned long) e68_get_areg32 (c, 7),
		(unsigned long) e68_get_ssp (c),
		(unsigned long) e68_get_last_pc (c, 3)
	);

	e68_dasm_mem (c, &op, e68_get_pc (c));
	st_dasm_str (str, &op, 1);

	pce_printf ("%08lX  %s\n", (unsigned long) e68_get_pc (c), str);
}

static
void st_print_state_cpu_short (e68000_t *c)
{
	e68_dasm_t op;
	char       str[256];

	e68_dasm_mem (c, &op, e68_get_pc (c));
	st_dasm_str (str, &op, 0);

	pce_printf ("D0=%08lX A0=%08lX A6=%08lX A7=%08lX SR=%04X[%c%c%c%c%c%c%c] %06lX %s\n",
		(unsigned long) e68_get_dreg32 (c, 0),
		(unsigned long) e68_get_areg32 (c, 0),
		(unsigned long) e68_get_areg32 (c, 6),
		(unsigned long) e68_get_areg32 (c, 7),
		(unsigned) e68_get_sr (c),
		(e68_get_sr_t (c)) ? 'T' : '-',
		(e68_get_sr_s (c)) ? 'S' : '-',
		(e68_get_sr_c (c)) ? 'C' : '-',
		(e68_get_sr_v (c)) ? 'V' : '-',
		(e68_get_sr_z (c)) ? 'Z' : '-',
		(e68_get_sr_n (c)) ? 'N' : '-',
		(e68_get_sr_x (c)) ? 'X' : '-',
		(unsigned long) e68_get_pc (c), str
	);
}

static
void st_print_state_mfp_timer (e68901_timer_t *tmr, const char *label)
{
	pce_printf ("TM-%s: CR=%02X DR=%02X/%02X DIV=%04X CLK=%04X OUT=%d (%u/%u)\n",
		label,
		tmr->cr, tmr->dr[0], tmr->dr[1],
		tmr->clk_div_set, tmr->clk_val / tmr->clk_div_inp,
		tmr->out != 0, tmr->clk_val, tmr->clk_div
	);
}

static
void st_print_state_mfp (atari_st_t *sim)
{
	e68901_t *mfp;

	static char parity[4] = { 'N', 'N', 'O', 'E' };

	mfp = &sim->mfp;

	pce_prt_sep ("68901-MFP");

	pce_printf ("GPIP: INP=%02X/%02X OUT=%02X AER=%02X DDR=%02X\n",
		mfp->gpip_inp, mfp->gpip_xor, mfp->gpip_val, mfp->gpip_aer, mfp->gpip_ddr
	);

	pce_printf ("INTR: IRR=%04X/%04X IER=%04X IPR=%04X IMR=%04X ISR=%04X IVR=%02X VEC=%02X IRQ=%d\n",
		mfp->irr1, mfp->irr2, mfp->ier, mfp->ipr, mfp->imr, mfp->isr, mfp->ivr, mfp->vec, mfp->irq_val
	);

	pce_printf ("UART: PAR=%u%c%u UCR=%02X RSR=%02X RDR=%02X TSR=%02X TDR=%02X RCLK=%u/%u SCLK=%u/%u\n",
		8 - ((mfp->ucr >> 5) & 3),
		parity[(mfp->ucr >> 1) & 3],
		(((mfp->ucr >> 3) & 3) + 1) / 2,
		mfp->ucr, mfp->rsr[0], mfp->rdr[0], mfp->tsr[0], mfp->tdr[0],
		mfp->recv_clk_cnt, mfp->recv_clk_max,
		mfp->send_clk_cnt, mfp->send_clk_max
	);

	st_print_state_mfp_timer (mfp->timer + 0, "A");
	st_print_state_mfp_timer (mfp->timer + 1, "B");
	st_print_state_mfp_timer (mfp->timer + 2, "C");
	st_print_state_mfp_timer (mfp->timer + 3, "D");
}

static
void st_print_state_acia (atari_st_t *sim, unsigned idx)
{
	e6850_t *acia;

	if (idx == 0) {
		acia = &sim->acia0;
		pce_prt_sep ("6850-ACIA-0");
	}
	else {
		acia = &sim->acia1;
		pce_prt_sep ("6850-ACIA-1");
	}

	pce_printf ("CR=%02X  SR=%02X  TDR=%02X  RDR=%02X  IRQ=%d\n",
		acia->cr, acia->sr, acia->tdr, acia->rdr, acia->irq_val
	);
}

static
void st_print_state_dma (atari_st_t *sim)
{
	st_dma_t *dma;

	dma = &sim->dma;

	pce_prt_sep ("DMA");

	pce_printf ("MODE=%04X  STATUS=%04X  ADDR=%06lX\n",
		dma->mode, dma->status, dma->addr
	);
}

static
void st_print_state_mem (atari_st_t *sim)
{
	pce_prt_sep ("MEM");
	mem_prt_state (sim->mem, stdout);
}

static
void st_print_state_video (atari_st_t *sim)
{
	unsigned   i;
	st_video_t *vid;

	vid = sim->video;

	pce_prt_sep ("VIDEO");

	pce_printf ("B=%06lX  A=%06lX  HB=%-3d  VB=%-3d\n",
		vid->base, vid->addr, vid->hb_val != 0, vid->vb_val != 0
	);

	pce_printf ("X=%-3u  HB1=%-3u  HB2=%-3u  %lu Hz\n",
		vid->clk, vid->hb1, vid->hb2,
		(8000000UL + vid->hb2 / 2) / vid->hb2
	);

	pce_printf ("Y=%-3u  VB1=%-3u  VB2=%-3u  %lu Hz\n",
		vid->line, vid->vb1, vid->vb2,
		(8000000UL + (vid->hb2 * vid->vb2) / 2) / (vid->hb2 * vid->vb2)
	);

	for (i = 0; i < 8; i++) {
		pce_printf ("PAL%X=%04X [%02X %02X %02X]  PAL%X=%04X [%02X %02X %02X]\n",
			i, vid->palette[i],
			vid->pal_col[i][0], vid->pal_col[i][1], vid->pal_col[i][2],
			i + 8, vid->palette[i + 8],
			vid->pal_col[i + 8][0], vid->pal_col[i + 8][1], vid->pal_col[i + 8][2]
		);
	}
}

static
void st_print_state (atari_st_t *sim, const char *str)
{
	cmd_t cmd;

	cmd_set_str (&cmd, str);

	if (cmd_match_eol (&cmd)) {
		return;
	}

	while (!cmd_match_eol (&cmd)) {
		if (cmd_match (&cmd, "cpu")) {
			st_print_state_cpu (sim);
		}
		else if (cmd_match (&cmd, "dma")) {
			st_print_state_dma (sim);
		}
		else if (cmd_match (&cmd, "mem")) {
			st_print_state_mem (sim);
		}
		else if (cmd_match (&cmd, "acia0")) {
			st_print_state_acia (sim, 0);
		}
		else if (cmd_match (&cmd, "acia1")) {
			st_print_state_acia (sim, 1);
		}
		else if (cmd_match (&cmd, "mfp")) {
			st_print_state_mfp (sim);
		}
		else if (cmd_match (&cmd, "video")) {
			st_print_state_video (sim);
		}
		else {
			pce_printf ("unknown component (%s)\n", cmd_get_str (&cmd));
			return;
		}
	}
}


/*
 * Check if a breakpoint has triggered
 */
static
int st_check_break (atari_st_t *sim)
{
	unsigned long pc;

	pc = e68_get_pc (sim->cpu) & 0x00ffffff;

	if (bps_check (&sim->bps, 0, pc, stdout)) {
		return (1);
	}

	if (sim->brk) {
		return (1);
	}

	return (0);
}

/*
 * Execute one instruction
 */
static
int st_exec (atari_st_t *sim)
{
	unsigned long old;

	old = e68_get_opcnt (sim->cpu);

	while (e68_get_opcnt (sim->cpu) == old) {
		st_clock (sim, 1);

		if (st_check_break (sim)) {
			return (1);
		}
	}

	return (0);
}

/*
 * Execute until a specific PC is reached
 */
static
int st_exec_to (atari_st_t *sim, unsigned long addr)
{
	while (e68_get_pc (sim->cpu) != addr) {
		st_clock (sim, 1);

		if (st_check_break (sim)) {
			return (1);
		}
	}

	return (0);
}

/*
 * Run the simulation
 */
void st_run (atari_st_t *sim)
{
	pce_start (&sim->brk);

	st_clock_discontinuity (sim);

	while (1) {
		st_clock (par_sim, 0);
		st_clock (par_sim, 0);

		if (sim->brk) {
			break;
		}

		while (sim->pause) {
			pce_usleep (50UL * 1000UL);
			trm_check (sim->trm);
		}
	}

	pce_stop();
}



/*
 * emscripten specific main loop
 */

/*
 * store global reference to simulation state struct
 * so that st_run_emscripten_step doesn't require it as a parameter
 */
atari_st_t  *atari_st_sim = NULL;

/*
 * setup and run the simulation
 */
void st_run_emscripten (atari_st_t *sim)
{
	atari_st_sim = sim;

	pce_start (&sim->brk);

	st_clock_discontinuity (sim);


	#ifdef EMSCRIPTEN
	emscripten_set_main_loop(st_run_emscripten_step, 100, 1);
	#else
	while (!sim->brk) {
		st_run_emscripten_step();
	}
	#endif

	// pce_stop();
}


/*
 * run one iteration
 */
void st_run_emscripten_step ()
{
	// for each 'emscripten step' we'll run a bunch of actual cycles
	// to minimise overhead from emscripten's main loop management
	int i;
	for (i = 0; i < 10000; ++i)
	{	
		st_clock (atari_st_sim, 0);
		st_clock (atari_st_sim, 0);

		if (atari_st_sim->brk) {
			pce_stop();
			#ifdef EMSCRIPTEN
			emscripten_cancel_main_loop();
			#endif
			return;
		}
	}
	// emscripten_pause_main_loop();
}
/*
 * end emscripten specific main loop
 */

static
void st_log_trap_bios (atari_st_t *sim, unsigned iw)
{
	unsigned       i;
	unsigned short par[8];

#if (DEBUG_BIOS == 0)
	return;
#endif

	for (i = 0; i < 8; i++) {
		par[i] = mem_get_uint16_be (sim->mem, e68_get_areg32 (sim->cpu, 7) + 2 * i);
	}

	switch (par[0]) {
	case 1:
		//st_log_deb ("bios_constat (%u)\n", par[1]);
		break;

	case 2:
		st_log_deb ("bios_conin (%u)\n", par[1]);
		break;

	case 3:
		st_log_deb ("bios_conout (%u, %u)\n", par[1], par[2]);
		break;

	case 4:
		st_log_deb ("bios_rwabs (%u, 0x%08lx, %u, %u, %u)\n",
			par[1], ((unsigned long) par[2] << 16) | par[3],
			par[4], par[5], par[6]
		);
		break;

	case 5:
		st_log_deb ("bios_setexc (%u, %lu)\n",
			par[1], ((unsigned long) par[2] << 16) | par[3]
		);
		break;

	case 9:
		st_log_deb ("bios_media_change (%u)\n", par[1]);
		break;

	default:
		st_log_deb ("bios (%u, %u, %u, %u)\n", par[0], par[1], par[2], par[3]);
		break;
	}
}

static
void st_log_exception (void *ext, unsigned tn)
{
	unsigned  iw;
	atari_st_t *sim = ext;

	iw = e68_get_mem16 (sim->cpu, e68_get_last_pc (sim->cpu, 0));

	switch (tn) {
	case 0x00:
		st_reset (sim);
		return;

	case 0x02: /* BUSE */
		return;

	case 0x04: /* ILLG */
	case 0x09: /* TRACE */
	case 0x0a: /* AXXX */
	case 0x0b: /* FXXX */
		return;

	case 0x1a: /* AVEC 2 */
	case 0x1c: /* AVEC 4 */
	case 0x1e: /* AVEC 6 */
		return;

	case 0x20: /* trap */
	case 0x21:
	case 0x22:
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2e:
	case 0x2f:
		return;

	case 0x2d:
		st_log_trap_bios (sim, iw);
		return;

	case 0x06:
	case 0x08:
	case 0x42: /* MFP RS232 CTS */
	case 0x44: /* MFP Timer D */
	case 0x45: /* MFP Timer C */
	case 0x46: /* MFP I4 (ACIA0) */
	case 0x48: /* HB counter */
	case 0x49: /* USART send data */
	case 0x4a: /* USART send error */
	case 0x4b: /* USART recv data */
	case 0x4c: /* USART recv error */
	case 0x4d: /* MFP Timer A */
		return;
	}

	pce_log (MSG_DEB,
		"%08lX: exception %02X (%s) IW=%04X\n",
		(unsigned long) e68_get_last_pc (sim->cpu, 0),
		tn, e68_get_exception_name (sim->cpu), iw
	);
}


/*
 * c - clock
 */
static
void st_cmd_c (cmd_t *cmd, atari_st_t *sim)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		st_clock (sim, 1);
		cnt -= 1;
	}

	st_print_state_cpu (sim);
}

/*
 * gb - run with breakpoints
 */
static
void st_cmd_g_b (cmd_t *cmd, atari_st_t *sim)
{
	unsigned long addr;
	breakpoint_t  *bp;

	while (cmd_match_uint32 (cmd, &addr)) {
		bp = bp_addr_new (addr);
		bps_bp_add (&sim->bps, bp);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	st_clock_discontinuity (sim);

	while (1) {
		if (st_exec (sim)) {
			break;
		}
	}

	pce_stop();
}

/*
 * ge - run until an exception is raised
 */
static
void st_cmd_g_e (cmd_t *cmd, atari_st_t *sim)
{
	unsigned short tn;
	unsigned       cnt;

	if (!cmd_match_uint16 (cmd, &tn)) {
		tn = 0xffff;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	cnt = e68_get_exception_cnt (sim->cpu);

	pce_start (&sim->brk);

	st_clock_discontinuity (sim);

	while (1) {
		st_exec (sim);

		if (st_check_break (sim)) {
			break;
		}

		if (e68_get_exception_cnt (sim->cpu) == cnt) {
			continue;
		}

		if (tn == 0xffff) {
			pce_printf ("exception %02X (%s)\n",
				e68_get_exception (sim->cpu),
				e68_get_exception_name (sim->cpu)
			);
			break;
		}
		else {
			if (e68_get_exception (sim->cpu) == tn) {
				pce_printf ("exception %02X (%s)\n",
					e68_get_exception (sim->cpu),
					e68_get_exception_name (sim->cpu)
				);
				break;
			}
		}

	}

	pce_stop();
}

/*
 * g - run
 */
static
void st_cmd_g (cmd_t *cmd, atari_st_t *sim)
{
	if (cmd_match (cmd, "b")) {
		st_cmd_g_b (cmd, sim);
		return;
	}
	else if (cmd_match (cmd, "e")) {
		st_cmd_g_e (cmd, sim);
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	st_run (sim);
}

/*
 * halt - halt the cpu
 */
static
void st_cmd_halt (cmd_t *cmd, atari_st_t *sim)
{
	unsigned short val;

	if (cmd_match_uint16 (cmd, &val) == 0) {
		val = 2;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	e68_set_halt (sim->cpu, val);

	st_print_state_cpu (sim);
}

/*
 * hm - help on messages
 */
static
void st_cmd_hm (cmd_t *cmd)
{
	pce_puts (
		"emu.exit\n"
		"emu.stop\n"
		"emu.pause            \"0\" | \"1\"\n"
		"emu.pause.toggle\n"
		"emu.reset\n"
		"\n"
		"emu.cpu.model        \"68000\" | \"68010\" | \"68020\"\n"
		"emu.cpu.speed        <factor>\n"
		"emu.cpu.speed.step   <adjustment>\n"
			"\n"
		"emu.disk.commit      [<drive>]\n"
		"emu.disk.eject       <drive>\n"
		"emu.disk.insert      <drive>:<fname>\n"
			"\n"
		"emu.par.driver       <driver>\n"
		"emu.par.file         <filename>\n"
			"\n"
		"emu.ser.driver       <driver>\n"
		"emu.ser.file         <filename>\n"
			"\n"
		"term.fullscreen      \"0\" | \"1\"\n"
		"term.fullscreen.toggle\n"
		"term.grab\n"
		"term.release\n"
		"term.screenshot      [<filename>]\n"
		"term.title           <title>\n"
	);
}

/*
 * p - step
 */
static
void st_cmd_p (cmd_t *cmd, atari_st_t *sim)
{
	unsigned      ecnt;
	unsigned long cnt;
	e68_dasm_t    da;

	cnt = 1;

	while (cmd_match (cmd, "p")) {
		cnt += 1;
	}

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	st_clock_discontinuity (sim);

	while (cnt > 0) {
		e68_dasm_mem (sim->cpu, &da, e68_get_pc (sim->cpu));

		if (da.flags & E68_DFLAG_CALL) {
			if (st_exec_to (sim, e68_get_pc (sim->cpu) + 2 * da.irn)) {
				break;
			}
		}
		else {
			ecnt = e68_get_exception_cnt (sim->cpu);

			if (st_exec (sim)) {
				break;
			}

			if (e68_get_exception_cnt (sim->cpu) != ecnt) {
				if (st_exec_to (sim, sim->cpu->except_addr)) {
					break;
				}
			}
		}

		cnt -= 1;
	}

	pce_stop();

	st_print_state_cpu (sim);
}

/*
 * n - run to following instruction
 */
static
void st_cmd_n (cmd_t *cmd, atari_st_t *sim)
{
	unsigned long cnt;
	e68_dasm_t    da;

	cnt = 1;

	while (cmd_match (cmd, "n")) {
		cnt += 1;
	}

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	st_clock_discontinuity (sim);

	while (cnt > 0) {
		e68_dasm_mem (sim->cpu, &da, e68_get_pc (sim->cpu));

		if (st_exec_to (sim, e68_get_pc (sim->cpu) + 2 * da.irn)) {
			break;
		}

		cnt -= 1;
	}

	pce_stop();

	st_print_state_cpu (sim);
}

/*
 * reset - reset the simulation
 */
static
void st_cmd_reset (cmd_t *cmd, atari_st_t *sim)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	st_reset (sim);

	st_print_state_cpu (sim);
}

/*
 * rte - execute until rte
 */
static
void st_cmd_rte (cmd_t *cmd, atari_st_t *sim)
{
	e68_dasm_t dis;

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	while (1) {
		st_exec (sim);

		if (st_check_break (sim)) {
			break;
		}

		e68_dasm_mem (sim->cpu, &dis, e68_get_pc (sim->cpu));

		if (dis.flags & E68_DFLAG_RTE) {
			st_print_state_cpu (sim);
			break;
		}
	}

	pce_stop();
}

/*
 * r - display or set register contents
 */
static
void st_cmd_r (cmd_t *cmd, atari_st_t *sim)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		st_print_state_cpu (sim);
		return;
	}

	if (!cmd_match_ident (cmd, sym, 256)) {
		cmd_error (cmd, "missing register\n");
		return;
	}

	if (e68_get_reg (sim->cpu, sym, &val)) {
		pce_printf ("bad register (%s)\n", sym);
		return;
	}

	if (cmd_match_eol (cmd)) {
		pce_printf ("%08lX\n", val);
		return;
	}

	if (!cmd_match_uint32 (cmd, &val)) {
		cmd_error (cmd, "missing value\n");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	e68_set_reg (sim->cpu, sym, val);

	st_print_state_cpu (sim);
}

/*
 * s - print state
 */
static
void st_cmd_s (cmd_t *cmd, atari_st_t *sim)
{
	if (cmd_match_eol (cmd)) {
		st_print_state_cpu (sim);
		return;
	}

	st_print_state (sim, cmd_get_str (cmd));
}

/*
 * t - execute one instruction
 */
static
void st_cmd_t (cmd_t *cmd, atari_st_t *sim)
{
	unsigned long i, n, k;

	n = 1;
	k = 0;

	while (cmd_match (cmd, "t")) {
		n += 1;
	}

	cmd_match_uint32 (cmd, &n);
	cmd_match_uint32 (cmd, &k);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	st_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		if ((n > 1) && (i >= k)) {
			st_print_state_cpu_short (sim->cpu);
		}

		st_exec (sim);
	}

	pce_stop();

	st_print_state_cpu (sim);
}

/*
 * u- - disassemble to address
 */
static
void st_cmd_u_to (cmd_t *cmd, atari_st_t *sim, unsigned long addr)
{
	unsigned long cnt;
	unsigned      ins_i, ins_j, ins_n;
	unsigned      sync;
	unsigned long pc;
	unsigned long insn[256];
	e68_dasm_t    op;
	char          str[256];

	cnt = 16;

	if (cmd_match_uint32 (cmd, &addr)) {
		cmd_match_uint32 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (cnt > 256) {
		cnt = 256;
	}

	ins_i = 0;
	ins_j = 0;
	ins_n = 0;
	sync = 5;

	pc = (addr - 12 * cnt) & ~1UL;

	while (pc <= addr) {
		if (sync == 0) {
			insn[ins_j] = pc;
			ins_j = (ins_j + 1) % 256;
			if (ins_j == ins_i) {
				ins_i = (ins_i + 1) % 256;
			}
			else {
				ins_n += 1;
			}
		}
		else {
			sync -= 1;
		}

		e68_dasm_mem (sim->cpu, &op, pc);

		pc += 2 * op.irn;
	}

	if (ins_n > cnt) {
		ins_i = (ins_i + (ins_n - cnt)) % 256;
	}

	while (ins_i != ins_j) {
		pc = insn[ins_i];

		e68_dasm_mem (sim->cpu, &op, pc);
		st_dasm_str (str, &op, 1);

		pce_printf ("%08lX  %s\n", pc, str);

		ins_i = (ins_i + 1) % 256;
	}


}

/*
 * u - disassemble
 */
static
void st_cmd_u (cmd_t *cmd, atari_st_t *sim)
{
	unsigned             i;
	unsigned long        addr, cnt;
	static unsigned int  first = 1;
	static unsigned long saddr = 0;
	e68_dasm_t           op;
	char                 str[256];

	if (first) {
		first = 0;
		saddr = e68_get_pc (sim->cpu);
	}

	addr = saddr;
	cnt = 16;

	if (cmd_match (cmd, "-")) {
		st_cmd_u_to (cmd, sim, addr);
		return;
	}

	if (cmd_match_uint32 (cmd, &addr)) {
		cmd_match_uint32 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	for (i = 0; i < cnt; i++) {
		e68_dasm_mem (sim->cpu, &op, addr);
		st_dasm_str (str, &op, 1);

		pce_printf ("%08lX  %s\n", addr, str);

		addr += 2 * op.irn;
	}

	saddr = addr;
}

int st_cmd (atari_st_t *sim, cmd_t *cmd)
{
	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps);
	}
	else if (cmd_match (cmd, "c")) {
		st_cmd_c (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		st_cmd_g (cmd, sim);
	}
	else if (cmd_match (cmd, "halt")) {
		st_cmd_halt (cmd, sim);
	}
	else if (cmd_match (cmd, "hm")) {
		st_cmd_hm (cmd);
	}
	else if (cmd_match (cmd, "p")) {
		st_cmd_p (cmd, sim);
	}
	else if (cmd_match (cmd, "n")) {
		st_cmd_n (cmd, sim);
	}
	else if (cmd_match (cmd, "reset")) {
		st_cmd_reset (cmd, sim);
	}
	else if (cmd_match (cmd, "rte")) {
		st_cmd_rte (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		st_cmd_r (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		st_cmd_s (cmd, sim);
	}
	else if (cmd_match (cmd, "t")) {
		st_cmd_t (cmd, sim);
	}
	else if (cmd_match (cmd, "u")) {
		st_cmd_u (cmd, sim);
	}
	else {
		return (1);
	}

	if (sim->trm != NULL) {
		trm_set_msg_trm (sim->trm, "term.release", "1");
	}

	return (0);
}

void st_cmd_init (atari_st_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	sim->cpu->log_ext = sim;
	sim->cpu->log_opcode = NULL;
	sim->cpu->log_undef = NULL;
	sim->cpu->log_exception = st_log_exception;
	sim->cpu->log_mem = NULL;
}
