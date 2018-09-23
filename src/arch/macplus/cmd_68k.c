/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/cmd_68k.c                                   *
 * Created:     2007-04-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2013 Hampa Hug <hampa@hampa.ch>                     *
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
#include "traps.h"

#include <string.h>

#include <cpu/e68000/e68000.h>

#include <lib/brkpt.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/sysdep.h>

#include <SDL.h>

#include <emscripten.h>

mon_cmd_t par_cmd[] = {
	{ "c", "[cnt]", "clock" },
	{ "gb", "[addr..]", "run with breakpoints at addr" },
	{ "ge", "[exception]", "run until exception" },
	{ "g", "", "run" },
	{ "halt", "[val]", "set halt state [2]" },
	{ "p", "[cnt]", "execute cnt instructions, skip calls [1]" },
	{ "reset", "", "reset" },
	{ "rte", "", "execute to next rte" },
	{ "r", "reg [val]", "get or set a register" },
	{ "s", "[what]", "print status (cpu|mem|scc|via)" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[[-]addr [cnt]]", "disassemble" }
};

unsigned par_cmd_cnt = sizeof (par_cmd) / sizeof (par_cmd[0]);


static
void mac_dasm_str (char *dst, e68_dasm_t *op)
{
	unsigned   i, n;
	char       tmp[256];
	const char *ins;

	strcpy (dst, "");

	for (i = 0; i < op->irn; i++) {
		sprintf (tmp, "%04X ", (unsigned) op->ir[i]);
		strcat (dst, tmp);
	}

	for (i = op->irn; i < 4; i++) {
		strcat (dst, "     ");
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

	ins = mac_get_trap_name (op->ir[0]);

	if (ins == NULL) {
		ins = op->op;
	}

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

static
void mac_prt_state_cpu (e68000_t *c)
{
	unsigned long opcnt, clkcnt;
	unsigned long delay;
	e68_dasm_t    op;
	char          str[256];

	pce_prt_sep ("68000");

	opcnt = e68_get_opcnt (c);
	clkcnt = e68_get_clkcnt (c);
	delay = e68_get_delay (c);

	pce_printf ("CLK=%lx  OP=%lx  DLY=%lu  CPI=%.4f\n",
		clkcnt, opcnt, delay,
		(opcnt > 0) ? ((double) (clkcnt + delay) / (double) opcnt) : 1.0
	);

	pce_printf (" SR=%04X[%c%c]  CC=%02X[%c%c%c%c%c] EX=%02X(%-4s) TRP=%04X IML=%X IPL=%X\n",
		(unsigned) e68_get_sr (c),
		(e68_get_sr_t (c)) ? 'T' : '-',
		(e68_get_sr_s (c)) ? 'S' : '-',
		(unsigned) e68_get_ccr (c),
		(e68_get_sr_c (c)) ? 'C' : '-',
		(e68_get_sr_v (c)) ? 'V' : '-',
		(e68_get_sr_z (c)) ? 'Z' : '-',
		(e68_get_sr_n (c)) ? 'N' : '-',
		(e68_get_sr_x (c)) ? 'X' : '-',
		e68_get_exception (c),
		e68_get_exception_name (c),
		e68_get_last_trap_a (c),
		e68_get_iml (c),
		e68_get_ipl (c)
	);

	pce_printf (" D0=%08lX  D4=%08lX  A0=%08lX  A4=%08lX   PC=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 0),
		(unsigned long) e68_get_dreg32 (c, 4),
		(unsigned long) e68_get_areg32 (c, 0),
		(unsigned long) e68_get_areg32 (c, 4),
		(unsigned long) e68_get_pc (c)
	);

	pce_printf (" D1=%08lX  D5=%08lX  A1=%08lX  A5=%08lX  LPC=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 1),
		(unsigned long) e68_get_dreg32 (c, 5),
		(unsigned long) e68_get_areg32 (c, 1),
		(unsigned long) e68_get_areg32 (c, 5),
		e68_get_last_pc (c, 0)
	);

	pce_printf (" D2=%08lX  D6=%08lX  A2=%08lX  A6=%08lX  USP=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 2),
		(unsigned long) e68_get_dreg32 (c, 6),
		(unsigned long) e68_get_areg32 (c, 2),
		(unsigned long) e68_get_areg32 (c, 6),
		(unsigned long) e68_get_usp (c)
	);

	pce_printf (" D3=%08lX  D7=%08lX  A3=%08lX  A7=%08lX  SSP=%08lX\n",
		(unsigned long) e68_get_dreg32 (c, 3),
		(unsigned long) e68_get_dreg32 (c, 7),
		(unsigned long) e68_get_areg32 (c, 3),
		(unsigned long) e68_get_areg32 (c, 7),
		(unsigned long) e68_get_ssp (c)
	);

	e68_dasm_mem (c, &op, e68_get_pc (c));
	mac_dasm_str (str, &op);

	pce_printf ("%08lX  %s\n", (unsigned long) e68_get_pc (c), str);
}

static
void mac_prt_state_scc (macplus_t *sim)
{
	unsigned i;
	e8530_t  *scc;

	scc = &sim->scc;

	pce_prt_sep ("8530-SCC");

	pce_printf ("  IRQ=%u\n", scc->irq_val);

	for (i = 0; i < 16; i++) {
		pce_printf (
			"WR%02uA=%02X  RR%02uA=%02X  WR%02uB=%02X  RR%02uB=%02X\n",
			i, scc->chn[0].wr[i],
			i, scc->chn[0].rr[i],
			i, scc->chn[1].wr[i],
			i, scc->chn[1].rr[i]
		);
	}
}

static
void mac_prt_state_via (macplus_t *sim)
{
	e6522_t *via;

	via = &sim->via;

	pce_prt_sep ("6522-VIA");

	pce_printf (" PCR=%02X  ACR=%02X  IFR=%02X  IER=%02X  IRQ=%u\n",
		via->pcr, via->acr, via->ifr, via->ier, via->irq_val
	);

	pce_printf ("DDRA=%02X DDRB=%02X  CA1=%X   T1L=%04X SHFT=%02X/%u\n",
		via->ddra, via->ddrb, via->ca1_inp,
		via->t1_latch, via->shift_val, via->shift_cnt
	);

	pce_printf (" IRA=%02X  IRB=%02X  CA2=%X  %cT1V=%04X\n",
		via->ira, via->irb, via->ca2_inp,
		via->t1_hot ? '*' : ' ', via->t1_val
	);

	pce_printf (" ORA=%02X  ORB=%02X  CB1=%X   T2L=%04X\n",
		via->ora, via->orb, 0,
		via->t2_latch
	);

	pce_printf ("  PA=%02X   PB=%02X  CB2=%X  %cT2V=%04X\n",
		sim->via_port_a, sim->via_port_b, 0,
		via->t2_hot ? '*' : ' ', via->t2_val
	);
}

static
void mac_prt_state_mem (macplus_t *sim)
{
	pce_prt_sep ("MEM");
	mem_prt_state (sim->mem, stdout);
}

void mac_prt_state (macplus_t *sim, const char *str)
{
	cmd_t cmd;

	cmd_set_str (&cmd, str);

	if (cmd_match_eol (&cmd)) {
		return;
	}

	while (!cmd_match_eol (&cmd)) {
		if (cmd_match (&cmd, "cpu")) {
			mac_prt_state_cpu (sim->cpu);
		}
		else if (cmd_match (&cmd, "mem")) {
			mac_prt_state_mem (sim);
		}
		else if (cmd_match (&cmd, "scc")) {
			mac_prt_state_scc (sim);
		}
		else if (cmd_match (&cmd, "via")) {
			mac_prt_state_via (sim);
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
int mac_check_break (macplus_t *sim)
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
int mac_exec (macplus_t *sim)
{
	unsigned long old;

	old = e68_get_opcnt (sim->cpu);

	while (e68_get_opcnt (sim->cpu) == old) {
		mac_clock (sim, 0);

		if (mac_check_break (sim)) {
			return (1);
		}
	}

	return (0);
}

/*
 * Execute until a specific PC is reached
 */
static
int mac_exec_to (macplus_t *sim, unsigned long addr)
{
	while (e68_get_pc (sim->cpu) != addr) {
		mac_clock (sim, 0);

		if (mac_check_break (sim)) {
			return (1);
		}
	}

	return (0);
}

/*
 * Run the simulation
 */
void mac_run (macplus_t *sim)
{
	pce_start (&sim->brk);

	mac_clock_discontinuity (sim);

	while (1) {
		mac_clock (par_sim, 0);
		mac_clock (par_sim, 0);

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
 * setup and run the simulation
 */
void mac_run_emscripten (macplus_t *sim)
{
	int brk = 0;

	pce_start (&sim->brk);

	mac_clock_discontinuity (sim);

	#ifdef EMSCRIPTEN
	emscripten_set_main_loop(mac_run_emscripten_step, 100, 1);
	#else
	while (!sim->brk) {
		mac_run_emscripten_step();
	}
	#endif

	// pce_stop();
}


/*
 * run one iteration
 */
void mac_run_emscripten_step ()
{
	int mousex;
	int mousey;
	int mousehack_interval = 100;
 
	const SDL_VideoInfo* videoinfo = SDL_GetVideoInfo();
	int screenw = videoinfo->current_w;
	int screenh = videoinfo->current_h;
 
	// for each 'emscripten step' we'll run a bunch of actual cycles
	// to minimise overhead from emscripten's main loop management
	int i;
	for (i = 0; i < 10000; ++i) {
		// gross hacks to set mouse position in browser
		if (i % mousehack_interval == 0) {
			SDL_GetMouseState (&mousex, &mousey);
			// clamp mouse pos to screen bounds
			mousex = mousex > screenw ? screenw : (mousex < 0 ? 0 : mousex);
			mousey = mousey > screenh ? screenh : (mousey < 0 ? 0 : mousey);
			// internal raw mouse coords
			e68_set_mem16 (par_sim->cpu, 0x0828, (unsigned) mousey);
			e68_set_mem16 (par_sim->cpu, 0x082a, (unsigned) mousex);
			// raw mouse coords
			e68_set_mem16 (par_sim->cpu, 0x082c, (unsigned) mousey);
			e68_set_mem16 (par_sim->cpu, 0x082e, (unsigned) mousex);
			// smoothed mouse coords
			e68_set_mem16 (par_sim->cpu, 0x0830, (unsigned) mousey);
			e68_set_mem16 (par_sim->cpu, 0x0832, (unsigned) mousex);
		}
		mac_clock (par_sim, 0);

		if (par_sim->brk) {
			pce_stop();
			#ifdef EMSCRIPTEN
			emscripten_cancel_main_loop();
			#endif
			return;
		}

		while (par_sim->pause) {
			pce_usleep (50UL * 1000UL);
			trm_check (par_sim->trm);
		}
	}
	// print state
	// mac_prt_state_cpu(par_sim);
}
/*
 * end emscripten specific main loop
 */

#if 0
static
void mac_log_opcode (void *ext, unsigned long ir)
{
	macplus_t *sim = ext;
}
#endif

static
void mac_log_undef (void *ext, unsigned long ir)
{
	unsigned long pc;
	macplus_t     *sim = ext;

	pc = e68_get_last_pc (sim->cpu, 0);

	pce_log (MSG_DEB,
		"%08lX: undefined operation: %04lX [%04X %04X %04X %04X %04X]\n",
		pc, ir,
		(unsigned) e68_get_mem16 (sim->cpu, pc),
		(unsigned) e68_get_mem16 (sim->cpu, pc + 2),
		(unsigned) e68_get_mem16 (sim->cpu, pc + 4),
		(unsigned) e68_get_mem16 (sim->cpu, pc + 6),
		(unsigned) e68_get_mem16 (sim->cpu, pc + 8)
	);

	/* mac_set_msg (sim, "emu.stop", NULL); */
}

static
void mac_log_exception (void *ext, unsigned tn)
{
	unsigned  iw;
	macplus_t *sim = ext;

	iw = e68_get_mem16 (sim->cpu, e68_get_last_pc (sim->cpu, 0));

	switch (tn) {
	case 0x00:
		mac_reset (sim);
		return;

	case 0x0a:
		mac_sony_patch (&sim->sony);
		return;

	case 0x19:
	case 0x1a:
		return;

	case 0x20: /* trap */
		return;

	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2e:
		return;
	}

	pce_log (MSG_DEB,
		"%08lX: exception %02X (%s) IW=%04X\n",
		(unsigned long) e68_get_last_pc (sim->cpu, 0),
		tn, e68_get_exception_name (sim->cpu), iw
	);
}

void mac_log_mem (void *ext, unsigned long addr, unsigned type)
{
#if 0
	macplus_t *sim = ext;

	addr &= 0x00ffffff;
#endif
}


/*
 * c - clock
 */
static
void mac_cmd_c (cmd_t *cmd, macplus_t *sim)
{
	unsigned long cnt;

	cnt = 1;

	cmd_match_uint32 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		mac_clock (sim, 1);
		cnt -= 1;
	}

	mac_prt_state_cpu (sim->cpu);
}

/*
 * gb - run with breakpoints
 */
static
void mac_cmd_g_b (cmd_t *cmd, macplus_t *sim)
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

	mac_clock_discontinuity (sim);

	while (1) {
		if (mac_exec (sim)) {
			break;
		}
	}

	pce_stop();
}

/*
 * ge - run until an exception is raised
 */
static
void mac_cmd_g_e (cmd_t *cmd, macplus_t *sim)
{
	unsigned       cnt;
	unsigned short tn;

	if (!cmd_match_uint16 (cmd, &tn)) {
		tn = 0xffff;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	cnt = e68_get_exception_cnt (sim->cpu);

	pce_start (&sim->brk);

	mac_clock_discontinuity (sim);

	while (1) {
		mac_exec (sim);

		if (mac_check_break (sim)) {
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
void mac_cmd_g (cmd_t *cmd, macplus_t *sim)
{
	if (cmd_match (cmd, "b")) {
		mac_cmd_g_b (cmd, sim);
		return;
	}
	else if (cmd_match (cmd, "e")) {
		mac_cmd_g_e (cmd, sim);
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	mac_run (sim);
}

/*
 * halt - halt the cpu
 */
static
void mac_cmd_halt (cmd_t *cmd, macplus_t *sim)
{
	unsigned short val;

	if (cmd_match_uint16 (cmd, &val) == 0) {
		val = 2;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	e68_set_halt (sim->cpu, val);

	mac_prt_state_cpu (sim->cpu);
}

/*
 * p - step
 */
static
void mac_cmd_p (cmd_t *cmd, macplus_t *sim)
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

	ecnt = e68_get_exception_cnt (sim->cpu);

	pce_start (&sim->brk);

	while (cnt > 0) {
		e68_dasm_mem (sim->cpu, &da, e68_get_pc (sim->cpu));

		if (da.flags & E68_DFLAG_CALL) {
			if (mac_exec_to (sim, e68_get_pc (sim->cpu) + 2 * da.irn)) {
				break;
			}
		}
		else {
			if (mac_exec (sim)) {
				break;
			}

			if (e68_get_exception_cnt (sim->cpu) != ecnt) {
				if (mac_exec_to (sim, sim->cpu->except_addr)) {
					break;
				}
			}
		}

		cnt -= 1;
	}

	pce_stop();

	mac_prt_state_cpu (sim->cpu);
}

/*
 * reset - reset the simulation
 */
static
void mac_cmd_reset (cmd_t *cmd, macplus_t *sim)
{
	if (!cmd_match_end (cmd)) {
		return;
	}

	mac_reset (sim);

	mac_prt_state_cpu (sim->cpu);
}

/*
 * rte - execute until rte
 */
static
void mac_cmd_rte (cmd_t *cmd, macplus_t *sim)
{
	e68_dasm_t dis;

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	while (1) {
		mac_exec (sim);

		if (mac_check_break (sim)) {
			break;
		}

		e68_dasm_mem (sim->cpu, &dis, e68_get_pc (sim->cpu));

		if (dis.flags & E68_DFLAG_RTE) {
			mac_prt_state_cpu (sim->cpu);
			break;
		}
	}

	pce_stop();
}

/*
 * r - display or set register contents
 */
static
void mac_cmd_r (cmd_t *cmd, macplus_t *sim)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		mac_prt_state_cpu (sim->cpu);
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

	mac_prt_state_cpu (sim->cpu);
}

/*
 * s - print state
 */
static
void mac_cmd_s (cmd_t *cmd, macplus_t *sim)
{
	if (cmd_match_eol (cmd)) {
		mac_prt_state_cpu (sim->cpu);
		return;
	}

	mac_prt_state (sim, cmd_get_str (cmd));
}

/*
 * t - execute one instruction
 */
static
void mac_cmd_t (cmd_t *cmd, macplus_t *sim)
{
	unsigned long i, n;

	n = 1;

	while (cmd_match (cmd, "t")) {
		n += 1;
	}

	cmd_match_uint32 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	for (i = 0; i < n; i++) {
		mac_exec (sim);
	}

	pce_stop();

	mac_prt_state_cpu (sim->cpu);
}

/*
 * u- - disassemble to address
 */
static
void mac_cmd_u_to (cmd_t *cmd, macplus_t *sim, unsigned long addr)
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
		mac_dasm_str (str, &op);

		pce_printf ("%08lX  %s\n", pc, str);

		ins_i = (ins_i + 1) % 256;
	}


}

/*
 * u - disassemble
 */
static
void mac_cmd_u (cmd_t *cmd, macplus_t *sim)
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
		mac_cmd_u_to (cmd, sim, addr);
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
		mac_dasm_str (str, &op);

		pce_printf ("%08lX  %s\n", addr, str);

		addr += 2 * op.irn;
	}

	saddr = addr;
}

int mac_cmd (macplus_t *sim, cmd_t *cmd)
{
	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps);
	}
	else if (cmd_match (cmd, "c")) {
		mac_cmd_c (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		mac_cmd_g (cmd, sim);
	}
	else if (cmd_match (cmd, "halt")) {
		mac_cmd_halt (cmd, sim);
	}
	else if (cmd_match (cmd, "p")) {
		mac_cmd_p (cmd, sim);
	}
	else if (cmd_match (cmd, "reset")) {
		mac_cmd_reset (cmd, sim);
	}
	else if (cmd_match (cmd, "rte")) {
		mac_cmd_rte (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		mac_cmd_r (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		mac_cmd_s (cmd, sim);
	}
	else if (cmd_match (cmd, "t")) {
		mac_cmd_t (cmd, sim);
	}
	else if (cmd_match (cmd, "u")) {
		mac_cmd_u (cmd, sim);
	}
	else {
		return (1);
	}

	if (sim->trm != NULL) {
		trm_set_msg_trm (sim->trm, "term.release", "1");
	}

	return (0);
}

void mac_cmd_init (macplus_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	sim->cpu->log_ext = sim;
	sim->cpu->log_opcode = NULL;
	sim->cpu->log_undef = mac_log_undef;
	sim->cpu->log_exception = mac_log_exception;
	sim->cpu->log_mem = mac_log_mem;
}
