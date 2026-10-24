/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/cmd.c                                         *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2022 Hampa Hug <hampa@hampa.ch>                     *
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
#include "vic20.h"

#include <string.h>

#include <lib/console.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/sysdep.h>


static mon_cmd_t par_cmd[] = {
	{ "c", "[cnt]", "clock" },
	{ "gb", "[addr...]", "run with breakpoints" },
	{ "g", "", "run" },
	{ "hm", "", "print help on messages" },
	{ "p", "[cnt]", "execute cnt instructions, skip calls [1]" },
	{ "r", "reg [val]", "get or set a register" },
	{ "s", "[what]", "print status (cpu|mem)" },
	{ "trace", "[on|off|expr]", "turn trace on or off" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[addr [cnt]]", "disassemble" },
	{ "vsync", "[cnt]", "clock until vsync" }
};


static
void v20_disasm_str (char *dst, e6502_disasm_t *op)
{
	unsigned i, n;

	n = sprintf (dst, "%04X ", (unsigned) op->pc);

	for (i = 0; i < op->dat_n; i++) {
		n += sprintf (dst + n, " %02X", (unsigned) op->dat[i]);
	}

	while (n < 15) {
		dst[n++] = ' ';
	}

	if (op->flags & E6502_OPF_UND) {
		dst[n++] = '*';
	}
	else {
		dst[n++] = ' ';
	}

	n += sprintf (dst + n, "%s", op->op);

	if (op->arg_n > 0) {
		while (n < 21) {
			dst[n++] = ' ';
		}

		n += sprintf (dst + n, "%s", op->arg1);
	}

	dst[n] = 0;
}

void v20_print_state_cpu (e6502_t *c)
{
	e6502_disasm_t op;
	char           str[256];

	pce_prt_sep ("6502");

	pce_printf (
		"A=%02X  X=%02X  Y=%02X  S=%02X  P=%02X "
		"[%c%c-%c%c%c%c%c]  PC=%04X  CLK=%lX\n",
		(unsigned) e6502_get_a (c),
		(unsigned) e6502_get_x (c),
		(unsigned) e6502_get_y (c),
		(unsigned) e6502_get_s (c),
		(unsigned) e6502_get_p (c),
		(e6502_get_nf (c)) ? 'N' : '-',
		(e6502_get_vf (c)) ? 'V' : '-',
		(e6502_get_bf (c)) ? 'B' : '-',
		(e6502_get_df (c)) ? 'D' : '-',
		(e6502_get_if (c)) ? 'I' : '-',
		(e6502_get_cf (c)) ? 'C' : '-',
		(e6502_get_zf (c)) ? 'Z' : '-',
		(unsigned) e6502_get_pc (c),
		e6502_get_clock (c)
	);

	e6502_disasm_cur (c, &op);
	v20_disasm_str (str, &op);

	pce_printf ("%s\n", str);
}

void v20_print_trace (e6502_t *c)
{
	e6502_disasm_t op;
	char           str[256];

	e6502_disasm_cur (c, &op);

	switch (op.arg_n) {
	case 0:
		strcpy (str, op.op);
		break;

	case 1:
		sprintf (str, "%-4s %s", op.op, op.arg1);
		break;

	default:
		strcpy (str, "****");
		break;
	}

	pce_printf ("%c%c-%c%c%c%c%c P=%02X S=%02X  A=%02X X=%02X Y=%02X  %04X  %s\n",
		(e6502_get_nf (c)) ? 'N' : '-',
		(e6502_get_vf (c)) ? 'V' : '-',
		(e6502_get_bf (c)) ? 'B' : '-',
		(e6502_get_df (c)) ? 'D' : '-',
		(e6502_get_if (c)) ? 'I' : '-',
		(e6502_get_cf (c)) ? 'C' : '-',
		(e6502_get_zf (c)) ? 'Z' : '-',
		(unsigned) e6502_get_p (c),
		(unsigned) e6502_get_s (c),
		(unsigned) e6502_get_a (c),
		(unsigned) e6502_get_x (c),
		(unsigned) e6502_get_y (c),
		(unsigned) e6502_get_pc (c),
		str
	);
}

static
void v20_print_state_mem (vic20_t *sim)
{
	pce_prt_sep ("6502 MEM");
	mem_prt_state (sim->mem, stdout);
}

static
void v20_print_state_via (vic20_t *sim, unsigned idx)
{
	unsigned char pa, pb;
	e6522_t       *via;

	if (idx == 1) {
		via = &sim->via1;
		pce_prt_sep ("6522 VIA 1");
	}
	else if (idx == 2) {
		via = &sim->via2;
		pce_prt_sep ("6522 VIA 2");
	}
	else {
		return;
	}

	pa = (via->ora & via->ddra) | (via->ira & ~via->ddra);
	pb = (via->orb & via->ddrb) | (via->irb & ~via->ddrb);

	pce_printf ("DDA=%02X  ORA=%02X  IRA=%02X  VAL=%02X\n",
		via->ddra, via->ora, via->ira, pa
	);

	pce_printf ("DDB=%02X  ORB=%02X  IRB=%02X  VAL=%02X\n",
		via->ddrb, via->orb, via->irb, pb
	);

	pce_printf ("CA1=%X   CB1=%X\n",
		via->ca1_inp, via->cb1_inp
	);

	pce_printf ("ACR=%02X  PCR=%02X\n", via->acr, via->pcr);

	pce_printf ("T1V=%04X  T1L=%04X  T1H=%d\n",
		via->t1_val, via->t1_latch, via->t1_hot
	);

	pce_printf ("T2V=%04X  T2L=%04X  T2H=%d\n",
		via->t2_val, via->t2_latch, via->t2_hot
	);

	pce_printf ("IER=%02X  IFR=%02X  IRQ=%d\n",
		via->ier, via->ifr, via->irq_val
	);
}

static
void v20_print_state (vic20_t *sim, const char *str)
{
	cmd_t cmd;

	cmd_set_str (&cmd, str);

	if (cmd_match_eol (&cmd)) {
		return;
	}

	while (!cmd_match_eol (&cmd)) {
		if (cmd_match (&cmd, "cpu")) {
			v20_print_state_cpu (sim->cpu);
		}
		else if (cmd_match (&cmd, "mem")) {
			v20_print_state_mem (sim);
		}
		else if (cmd_match (&cmd, "via1")) {
			v20_print_state_via (sim, 1);
		}
		else if (cmd_match (&cmd, "via2")) {
			v20_print_state_via (sim, 2);
		}
		else if (cmd_match (&cmd, "via")) {
			v20_print_state_via (sim, 1);
			v20_print_state_via (sim, 2);
		}
		else if (cmd_match (&cmd, "vic")) {
			v20_video_print_state (&sim->video);
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
int v20_check_break (vic20_t *sim)
{
	unsigned pc;

	pc = e6502_get_pc (sim->cpu) & 0xffff;

	if (bps_check (&sim->bps, 0, pc, stdout)) {
		return (1);
	}

	if (sim->brk) {
		return (1);
	}

	return (0);
}

static
void v20_exec (vic20_t *sim)
{
	unsigned long old;

	old = e6502_get_opcnt (sim->cpu);

	while (e6502_get_opcnt (sim->cpu) == old) {
		v20_clock (sim);
	}
}

static
int v20_exec_to (vic20_t *sim, unsigned short addr)
{
	while (e6502_get_pc (sim->cpu) != addr) {
		v20_clock (sim);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

static
int v20_exec_off (vic20_t *sim, unsigned short addr)
{
	while (e6502_get_pc (sim->cpu) == addr) {
		v20_clock (sim);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

void v20_run (vic20_t *sim)
{
	pce_start (&sim->brk);

	v20_clock_resync (sim);

	while (1) {
		v20_clock (sim);

		if (sim->brk) {
			break;
		}
	}

	pce_stop();
}


static
int v20_op_undef (void *ext, unsigned char op)
{
	vic20_t *sim;

	sim = ext;

	pce_log (MSG_DEB,
		"%04X: undefined operation [%02X]\n",
		(unsigned) e6502_get_pc (sim->cpu), (unsigned) op
	);

	sim->brk = PCE_BRK_STOP;

	return (0);
}

static
int v20_op_brk (void *ext, unsigned char op)
{
	vic20_t *sim;

	sim = ext;
	sim->brk = PCE_BRK_STOP;

	return (1);
}

static
void v20_cmd_bsave (cmd_t *cmd, vic20_t *sim)
{
	unsigned i, val;
	unsigned addr1, addr2;
	char     fname[256];
	FILE     *fp;

	if (!cmd_match_str (cmd, fname, 256)) {
		cmd_error (cmd, "need a file name");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	addr1 = mem_get_uint16_le (sim->mem, 0x2b);
	addr2 = mem_get_uint16_le (sim->mem, 0x2d);

	if ((fp = fopen (fname, "wb")) == NULL) {
		pce_printf ("can't open file (%s)\n", fname);
		return;
	}

	fputc (addr1 & 0xff, fp);
	fputc ((addr1 >> 8) & 0xff, fp);

	for (i = addr1; i < addr2; i++) {
		val = mem_get_uint8 (sim->mem, i);

		fputc (val, fp);
	}

	fclose (fp);
}

static
void v20_cmd_vsync (cmd_t *cmd, vic20_t *sim)
{
	unsigned short cnt;
	unsigned       vs;

	cnt = 1;

	cmd_match_uint16 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	v20_clock_resync (sim);

	while (cnt-- > 0) {
		vs = sim->video.vsync_cnt;

		while (sim->video.vsync_cnt == vs) {
			v20_clock (sim);
		}
	}

	v20_print_state_cpu (sim->cpu);
}

static
void v20_cmd_c (cmd_t *cmd, vic20_t *sim)
{
	unsigned short cnt;

	cnt = 1;

	cmd_match_uint16 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	v20_clock_resync (sim);

	while (cnt-- > 0) {
		v20_clock (sim);
	}

	v20_print_state_cpu (sim->cpu);
}

static
void v20_cmd_gb (cmd_t *cmd, vic20_t *sim)
{
	unsigned short addr;
	breakpoint_t   *bp;

	while (cmd_match_uint16 (cmd, &addr)) {
		bp = bp_addr_new (addr);
		bps_bp_add (&sim->bps, bp);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	v20_clock_resync (sim);

	while (1) {
		if (sim->trace) {
			v20_print_trace (sim->cpu);
		}

		v20_exec (sim);

		if (v20_check_break (sim)) {
			break;
		}
	}

	pce_stop();
}

static
void v20_cmd_g (cmd_t *cmd, vic20_t *sim)
{
	if (cmd_match (cmd, "b")) {
		v20_cmd_gb (cmd, sim);
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	v20_run (sim);
}

static
void v20_cmd_hm (cmd_t *cmd)
{
	pce_puts (
		"emu.config.save\n    <filename>\n"
		"emu.exit\n"
		"emu.reset\n"
		"emu.stop\n"
		"\n"
		"emu.cas.commit\n"
		"emu.cas.create       <filename>\n"
		"emu.cas.play\n"
		"emu.cas.load         [<pos>]\n"
		"emu.cas.read         <filename>\n"
		"emu.cas.record\n"
		"emu.cas.space\n"
		"emu.cas.state\n"
		"emu.cas.stop\n"
		"emu.cas.truncate\n"
		"emu.cas.write        <filename>\n"
		"\n"
		"emu.cpu.speed        <factor>\n"
		"emu.cpu.speed.step   <adjustment>\n"
		"\n"
		"emu.term.fullscreen  \"0\" | \"1\"\n"
		"emu.term.fullscreen.toggle\n"
		"emu.term.grab\n"
		"emu.term.release\n"
		"emu.term.screenshot  [<filename>]\n"
		"emu.term.title       <title>\n"
		"\n"
		"emu.video.brightness <brightness>\n"
		"emu.video.framedrop  <count>\n"
		"emu.video.hue        <hue>\n"
		"emu.video.saturation <saturation>\n"
	);
}

static
void v20_cmd_n (cmd_t *cmd, vic20_t *sim)
{
	unsigned       pc;
	e6502_disasm_t dis;

	if (!cmd_match_end (cmd)) {
		return;
	}

	e6502_disasm_cur (sim->cpu, &dis);

	pc = (e6502_get_pc (sim->cpu) + dis.dat_n) & 0xffff;

	pce_start (&sim->brk);

	v20_clock_resync (sim);

	while (e6502_get_pc (sim->cpu) != pc) {
		if (sim->trace) {
			v20_print_trace (sim->cpu);
		}

		v20_exec (sim);

		if (v20_check_break (sim)) {
			break;
		}
	}

	pce_stop();

	v20_print_state_cpu (sim->cpu);
}

static
void v20_cmd_p (cmd_t *cmd, vic20_t *sim)
{
	unsigned short cnt;
	e6502_disasm_t dis;

	cnt = 1;

	cmd_match_uint16 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	v20_clock_resync (sim);

	while (cnt > 0) {
		e6502_disasm_cur (sim->cpu, &dis);

		if (sim->trace) {
			v20_print_trace (sim->cpu);
		}

		if (dis.flags & E6502_OPF_JSR) {
			if (v20_exec_to (sim, dis.pc + dis.dat_n)) {
				break;
			}
		}
		else {
			if (v20_exec_off (sim, dis.pc)) {
				break;
			}
		}

		cnt -= 1;
	}

	pce_stop();

	v20_print_state_cpu (sim->cpu);
}

static
void v20_cmd_r (cmd_t *cmd, vic20_t *sim)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		v20_print_state_cpu (sim->cpu);
		return;
	}

	if (!cmd_match_ident (cmd, sym, 256)) {
		cmd_error (cmd, "missing register\n");
		return;
	}

	if (e6502_get_reg (sim->cpu, sym, &val)) {
		pce_printf ("bad register\n");
		return;
	}

	if (cmd_match_eol (cmd)) {
		pce_printf ("%02lX\n", val);
		return;
	}

	if (!cmd_match_uint32 (cmd, &val)) {
		cmd_error (cmd, "missing value\n");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	e6502_set_reg (sim->cpu, sym, val);

	v20_print_state_cpu (sim->cpu);
}

static
void v20_cmd_s (cmd_t *cmd, vic20_t *sim)
{
	if (cmd_match_eol (cmd)) {
		v20_print_state_cpu (sim->cpu);
		return;
	}

	v20_print_state (sim, cmd_get_str (cmd));
}

static
void v20_cmd_trace (cmd_t *cmd, vic20_t *sim)
{
	unsigned short v;

	if (cmd_match_eol (cmd)) {
		pce_printf ("trace is %s\n", sim->trace ? "on" : "off");
		return;
	}

	if (cmd_match (cmd, "on")) {
		sim->trace = 1;
	}
	else if (cmd_match (cmd, "off")) {
		sim->trace = 0;
	}
	else if (cmd_match_uint16 (cmd, &v)) {
		sim->trace = (v != 0);
	}
	else {
		cmd_error (cmd, "on or off expected\n");
	}
}

static
void v20_cmd_t (cmd_t *cmd, vic20_t *sim)
{
	unsigned short i, n;

	n = 1;

	cmd_match_uint16 (cmd, &n);

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	v20_clock_resync (sim);

	for (i = 0; i < n; i++) {
		if (sim->trace) {
			v20_print_trace (sim->cpu);
		}

		v20_exec (sim);

		if (v20_check_break (sim)) {
			break;
		}
	}

	pce_stop();

	v20_print_state_cpu (sim->cpu);
}

static
void v20_cmd_u (cmd_t *cmd, vic20_t *sim)
{
	unsigned              i;
	unsigned short        addr, cnt;
	static unsigned int   first = 1;
	static unsigned short saddr = 0;
	e6502_disasm_t        op;
	char                  str[256];

	if (first) {
		first = 0;
		saddr = e6502_get_pc (sim->cpu);
	}

	addr = saddr;
	cnt = 16;

	if (cmd_match_uint16 (cmd, &addr)) {
		cmd_match_uint16 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	for (i = 0; i < cnt; i++) {
		e6502_disasm_mem (sim->cpu, &op, addr);
		v20_disasm_str (str, &op);

		pce_printf ("%s\n", str);

		addr += op.dat_n;
	}

	saddr = addr;
}

int v20_cmd (vic20_t *sim, cmd_t *cmd)
{
	if (sim->trm != NULL) {
		trm_check (sim->trm);
	}

	if (cmd_match (cmd, "bsave")) {
		v20_cmd_bsave (cmd, sim);
	}
	else if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps);
	}
	else if (cmd_match (cmd, "c")) {
		v20_cmd_c (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		v20_cmd_g (cmd, sim);
	}
	else if (cmd_match (cmd, "hm")) {
		v20_cmd_hm (cmd);
	}
	else if (cmd_match (cmd, "n")) {
		v20_cmd_n (cmd, sim);
	}
	else if (cmd_match (cmd, "p")) {
		v20_cmd_p (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		v20_cmd_r (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		v20_cmd_s (cmd, sim);
	}
	else if (cmd_match (cmd, "trace")) {
		v20_cmd_trace (cmd, sim);
	}
	else if (cmd_match (cmd, "t")) {
		v20_cmd_t (cmd, sim);
	}
	else if (cmd_match (cmd, "u")) {
		v20_cmd_u (cmd, sim);
	}
	else if (cmd_match (cmd, "vsync")) {
		v20_cmd_vsync (cmd, sim);
	}
	else {
		return (1);
	}

	return (0);
}

void v20_cmd_init (vic20_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	sim->cpu->hook_ext = sim;
	sim->cpu->hook_all = NULL;
	sim->cpu->hook_undef = v20_op_undef;
	sim->cpu->hook_brk = v20_op_brk;
}
