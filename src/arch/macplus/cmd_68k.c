/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/cmd_68k.c                                   *
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
#include "traps.h"

#include <string.h>

#include <cpu/e68000/e68000.h>

#include <lib/brkpt.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/log.h>
#include <lib/sysdep.h>


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
	unsigned long long opcnt, clkcnt;
	unsigned long      delay;
	e68_dasm_t         op;
	char               str[256];

	pce_prt_sep ("68000");

	opcnt = e68_get_opcnt (c);
	clkcnt = e68_get_clkcnt (c);
	delay = e68_get_delay (c);

	pce_printf ("CLK=%llx  OP=%llx  DLY=%lu  CPI=%.4f\n",
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
		e68_get_last_pc (c)
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

static
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

	if (bps_check (&sim->bps, pc, stdout)) {
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
void mac_exec (macplus_t *sim)
{
	unsigned long long old;

	old = e68_get_opcnt (sim->cpu);

	while (e68_get_opcnt (sim->cpu) == old) {
		mac_clock (sim, 0);

		if (e68_get_halt (sim->cpu)) {
			break;
		}
	}
}

/*
 * Execute until a specific PC is reached
 */
static
int mac_exec_to (macplus_t *sim, unsigned long addr)
{
	while (e68_get_pc (sim->cpu) != addr) {
		mac_clock (sim, 0);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

/*
 * Execute until the PC changes
 */
static
int mac_exec_off (macplus_t *sim, unsigned long addr)
{
	while (e68_get_pc (sim->cpu) == addr) {
		mac_clock (sim, 0);

		if (sim->brk) {
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
	macplus_t *sim = ext;

	pce_log (MSG_DEB,
		"%08lX: undefined operation [%04lX]\n",
		(unsigned long) e68_get_pc (sim->cpu), ir
	);

	/* mac_set_msg (sim, "emu.stop", NULL); */
}

static
void mac_log_exception (void *ext, unsigned tn)
{
	unsigned  iw;
	macplus_t *sim = ext;

	iw = e68_get_mem16 (sim->cpu, e68_get_pc (sim->cpu));

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
		(unsigned long) e68_get_pc (sim->cpu),
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
 * copy - copy memory
 */
static
void mac_cmd_copy (cmd_t *cmd, macplus_t *sim)
{
	unsigned long src, dst, cnt;
	unsigned char val;

	if (!cmd_match_uint32 (cmd, &src)) {
		cmd_error (cmd, "need a source address");
		return;
	}

	if (!cmd_match_uint32 (cmd, &dst)) {
		cmd_error (cmd, "need a destination address");
		return;
	}

	if (!cmd_match_uint32 (cmd, &cnt)) {
		cmd_error (cmd, "need a byte count");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (cnt == 0) {
		return;
	}

	if (src >= dst) {
		while (cnt > 0) {
			val = e68_get_mem8 (sim->cpu, src);
			e68_set_mem8 (sim->cpu, dst, val);

			src += 1;
			dst += 1;
			cnt -= 1;
		}
	}
	else {
		src += cnt - 1;
		dst += cnt - 1;

		while (cnt > 0) {
			val = e68_get_mem8 (sim->cpu, src);
			e68_set_mem8 (sim->cpu, dst, val);

			src -= 1;
			dst -= 1;
			cnt -= 1;
		}
	}
}

/*
 * d - memory dump
 */
static
void mac_cmd_d (cmd_t *cmd, macplus_t *sim)
{
	unsigned long        i, j;
	unsigned long        cnt;
	unsigned long        addr1, addr2;
	static int           first = 1;
	static unsigned long saddr = 0;
	unsigned long        p, p1, p2;
	char                 buf[256];

	if (first) {
		first = 0;
		saddr = e68_get_pc (sim->cpu);
	}

	addr1 = saddr;
	cnt = 256;

	if (cmd_match_uint32 (cmd, &addr1)) {
		cmd_match_uint32 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	addr2 = (addr1 + cnt - 1) & 0xffffffff;
	if (addr2 < addr1) {
		addr2 = 0xffffffff;
		cnt = addr2 - addr1 + 1;
	}

	saddr = addr1 + cnt;

	p1 = addr1 / 16;
	p2 = addr2 / 16 + 1;

	for (p = p1; p < p2; p++) {
		j = 16 * p;

		sprintf (buf,
			"%08lX  xx xx xx xx xx xx xx xx-xx xx xx xx xx xx xx xx  xxxxxxxxxxxxxxxx\n",
			j
		);

		for (i = 0; i < 16; i++) {
			if ((j >= addr1) && (j <= addr2)) {
				uint8_t  val;
				unsigned val1, val2;

				val = e68_get_mem8 (sim->cpu, j);

				val1 = (val >> 4) & 0x0f;
				val2 = val & 0x0f;

				buf[10 + 3 * i + 0] = (val1 < 10) ? ('0' + val1) : ('A' + val1 - 10);
				buf[10 + 3 * i + 1] = (val2 < 10) ? ('0' + val2) : ('A' + val2 - 10);

				if ((val >= 32) && (val <= 127)) {
					buf[59 + i] = val;
				}
				else {
					buf[59 + i] = '.';
				}
			}
			else {
				buf[10 + 3 * i] = ' ';
				buf[10 + 3 * i + 1] = ' ';
				buf[59 + i] = ' ';
			}

			j += 1;
		}

		pce_puts (buf);
	}
}

/*
 * e - enter bytes into memory
 */
static
void mac_cmd_e (cmd_t *cmd, macplus_t *sim)
{
	unsigned       i;
	unsigned long  addr;
	unsigned short val;
	char           buf[256];

	addr = 0;

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "need an address");
		return;
	}

	while (1) {
		if (cmd_match_uint16 (cmd, &val)) {
			mem_set_uint8_rw (sim->mem, addr, val);
			addr += 1;
		}
		else if (cmd_match_str (cmd, buf, 256)) {
			i = 0;
			while (buf[i] != 0) {
				mem_set_uint8_rw (sim->mem, addr, buf[i]);
				addr += 1;
				i += 1;
			}
		}
		else {
			break;
		}
	}

	if (!cmd_match_end (cmd)) {
		return;
	}
}

/*
 * f - find
 */
static
void mac_cmd_f (cmd_t *cmd, macplus_t *sim)
{
	unsigned long  addr, size;
	unsigned short val;
	unsigned       i;
	unsigned       cnt;
	unsigned char  buf[256];

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "need an address");
		return;
	}

	if (!cmd_match_uint32 (cmd, &size)) {
		cmd_error (cmd, "need a size");
		return;
	}

	cnt = 0;
	while ((cnt < 256) && cmd_match_uint16 (cmd, &val)) {
		buf[cnt] = val;
		cnt += 1;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if ((cnt == 0) || (cnt > size)) {
		return;
	}

	size = size - cnt + 1;

	while (size > 0) {
		i = 0;
		while (i < cnt) {
			if (mem_get_uint8 (sim->mem, addr + i) != buf[i]) {
				break;
			}

			i += 1;
		}

		if (i >= cnt) {
			pce_printf ("%06lX\n", addr);
		}

		addr += 1;
		size -= 1;
	}
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
		mac_exec (sim);

		if (mac_check_break (sim)) {
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
	unsigned short tn;

	if (!cmd_match_uint16 (cmd, &tn)) {
		tn = 0xffff;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	sim->cpu->excptn = 256;

	pce_start (&sim->brk);

	mac_clock_discontinuity (sim);

	while (1) {
		mac_exec (sim);

		if (mac_check_break (sim)) {
			break;
		}

		if (tn == 0xffff) {
			if (e68_get_exception (sim->cpu) != 256) {
				pce_printf ("exception %02X (%s)\n",
					e68_get_exception (sim->cpu),
					e68_get_exception_name (sim->cpu)
				);
				break;
			}
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
 * h - help
 */
static
void mac_cmd_h (cmd_t *cmd, macplus_t *sim)
{
	pce_puts (
		"bc [index]                clear a breakpoint or all\n"
		"bl                        list breakpoints\n"
		"bsa addr [pass [reset]]   set an address breakpoint [pass=1 reset=0]\n"
		"bsx expr [pass [reset]]   set an expression breakpoint [pass=1 reset=0]\n"
		"copy src dst cnt          copy memory\n"
		"c [cnt]                   clock\n"
		"d [addr [cnt]]            dump memory\n"
		"e addr [val...]           enter bytes into memory\n"
		"f addr size [val...]      find bytes in memory\n"
		"g b [addr..]              run with breakpoints at addr\n"
		"g e [exception]           run until exception\n"
		"g                         run\n"
		"halt [val]                set halt state [2]\n"
		"m msg val                 send a message\n"
		"p [cnt]                   execute cnt instructions, skip calls [1]\n"
		"q                         quit\n"
		"reset                     reset\n"
		"rte                       execute to next rte\n"
		"r reg [val]               set a register\n"
		"s [what]                  print status (cpu|mem|scc|via)\n"
		"t [cnt]                   execute cnt instructions [1]\n"
		"u [[-]addr [cnt]]         disassemble\n"
		"v [expr...]               evaluate expressions\n"
		"w name addr cnt           save memory to file\n"
	);
}

/*
 * p - step
 */
static
void mac_cmd_p (cmd_t *cmd, macplus_t *sim)
{
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

	while (cnt > 0) {
		e68_dasm_mem (sim->cpu, &da, e68_get_pc (sim->cpu));

		if (da.flags & E68_DFLAG_CALL) {
			if (mac_exec_to (sim, e68_get_pc (sim->cpu) + 2 * da.irn)) {
				break;
			}
		}
		else {
			if (mac_exec_off (sim, e68_get_pc (sim->cpu))) {
				break;
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

static
void mac_cmd_w (cmd_t *cmd, macplus_t *sim)
{
	unsigned long addr, cnt;
	char           fname[256];
	unsigned char  v;
	FILE           *fp;

	if (!cmd_match_str (cmd, fname, 256)) {
		cmd_error (cmd, "need a file name");
		return;
	}

	if (!cmd_match_uint32 (cmd, &addr)) {
		cmd_error (cmd, "address expected");
		return;
	}

	if (!cmd_match_uint32 (cmd, &cnt)) {
		cmd_error (cmd, "byte count expected");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		pce_printf ("can't open file (%s)\n", fname);
		return;
	}

	while (cnt > 0) {
		v = e68_get_mem8 (sim->cpu, addr);
		fputc (v, fp);

		addr += 1;
		cnt -= 1;
	}

	fclose (fp);
}

int mac_cmd (macplus_t *sim, cmd_t *cmd)
{
	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps, 0);
	}
	else if (cmd_match (cmd, "copy")) {
		mac_cmd_copy (cmd, sim);
	}
	else if (cmd_match (cmd, "c")) {
		mac_cmd_c (cmd, sim);
	}
	else if (cmd_match (cmd, "d")) {
		mac_cmd_d (cmd, sim);
	}
	else if (cmd_match (cmd, "e")) {
		mac_cmd_e (cmd, sim);
	}
	else if (cmd_match (cmd, "f")) {
		mac_cmd_f (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		mac_cmd_g (cmd, sim);
	}
	else if (cmd_match (cmd, "halt")) {
		mac_cmd_halt (cmd, sim);
	}
	else if (cmd_match (cmd, "h")) {
		mac_cmd_h (cmd, sim);
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
	else if (cmd_match (cmd, "w")) {
		mac_cmd_w (cmd, sim);
	}
	else {
		return (1);
	}

	if (sim->trm != NULL) {
		trm_set_msg_trm (sim->trm, "term.release", "1");
	}

	return (0);
}

void mac_cmd_init (macplus_t *sim)
{
	sim->cpu->log_ext = sim;
	sim->cpu->log_opcode = NULL;
	sim->cpu->log_undef = mac_log_undef;
	sim->cpu->log_exception = mac_log_exception;
	sim->cpu->log_mem = mac_log_mem;
}
