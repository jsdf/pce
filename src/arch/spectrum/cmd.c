/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/spectrum/cmd.c                                      *
 * Created:     2022-02-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022-2024 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#include "main.h"
#include "snapshot.h"
#include "spectrum.h"
#include "video.h"

#include <stdio.h>
#include <string.h>

#include <lib/brkpt.h>
#include <lib/cmd.h>
#include <lib/console.h>
#include <lib/log.h>
#include <lib/monitor.h>
#include <lib/sysdep.h>


static mon_cmd_t par_cmd[] = {
	{ "c", "[cnt]", "clock" },
	{ "gb", "[addr...]", "run with breakpoints" },
	{ "g", "", "run" },
	{ "hm", "", "print help on messages" },
	{ "i", "port", "input a byte from a port" },
	{ "n", "[0|1]", "execute to next instruction" },
	{ "o", "port val", "output a byte to a port" },
	{ "p", "[cnt]", "execute cnt instructions, skip calls [1]" },
	{ "r", "reg [val]", "set a register" },
	{ "s", "[what]", "print status (cpu|mem)" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[addr [cnt]]", "disassemble" },
	{ "xl", "fname", "load a snapshot" },
	{ "xs", "fname", "save a snapshot" }
};


static
void spec_disasm_str (char *dst, e8080_disasm_t *op, int with_pc, int with_comment)
{
	unsigned i, k, n;
	char     comment[256];

	comment[0] = 0;

	if (with_pc) {
		n = sprintf (dst, "%04X  ", (unsigned) op->pc);
	}
	else {
		n = 0;
	}

	for (i = 0; i < op->data_cnt; i++) {
		n += sprintf (dst + n, "%02X ", (unsigned) op->data[i]);
	}

	for (i = op->data_cnt; i < 4; i++) {
		dst[n++] = ' ';
		dst[n++] = ' ';
		dst[n++] = ' ';
	}

	k = n + 8;
	n += sprintf (dst + n, "%s", op->op);

	if (op->arg_cnt > 0) {
		while (n < k) {
			dst[n++] = ' ';
		}

		n += sprintf (dst + n, "%s", op->arg[0]);

		for (i = 1; i < op->arg_cnt; i++) {
			n += sprintf (dst + n, ", %s", op->arg[i]);
		}
	}

	if (with_comment && (comment[0] != 0)) {
		while (n < 40) {
			dst[n++] = ' ';
		}

		dst[n++] = ';';
	}

	dst[n] = 0;
}

static
void spec_disasm_cur (e8080_t *c, e8080_disasm_t *op)
{
	if (e8080_get_flags (c) & E8080_FLAG_Z80) {
		z80_disasm_cur (c, op);
	}
	else {
		e8080_disasm_cur (c, op);
	}
}

static
void spec_disasm_mem (e8080_t *c, e8080_disasm_t *op, unsigned short addr)
{
	if (e8080_get_flags (c) & E8080_FLAG_Z80) {
		z80_disasm_mem (c, op, addr);
	}
	else {
		e8080_disasm_mem (c, op, addr);
	}
}

static
void spec_print_cpu_z80_1 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	spec_disasm_cur (c, &op);
	spec_disasm_str (str, &op, 0, 0);

	if (c->halt) {
		pce_printf ("HALT=1  ");
	}

	pce_printf (
		"A=%02X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X SP=%04X PC=%04X PSW=%02X[%c%c%c%c%c]  %s\n",
		(unsigned) e8080_get_a (c), (unsigned) e8080_get_bc (c),
		(unsigned) e8080_get_de (c), (unsigned) e8080_get_hl (c),
		(unsigned) e8080_get_ix (c), (unsigned) e8080_get_iy (c),
		(unsigned) e8080_get_sp (c), (unsigned) e8080_get_pc (c),
		(unsigned) e8080_get_psw (c),
		(e8080_get_sf (c)) ? 'S' : '-',
		(e8080_get_zf (c)) ? 'Z' : '-',
		(e8080_get_af (c)) ? 'A' : '-',
		(e8080_get_pf (c)) ? 'P' : '-',
		(e8080_get_cf (c)) ? 'C' : '-',
		str
	);
}

static
void spec_print_cpu_8080_1 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	spec_disasm_cur (c, &op);
	spec_disasm_str (str, &op, 0, 0);

	if (c->halt) {
		pce_printf ("HALT=1  ");
	}

	pce_printf (
		"A=%02X BC=%04X DE=%04X HL=%04X SP=%04X PC=%04X PSW=%02X[%c%c%c%c%c]  %s\n",
		(unsigned) e8080_get_a (c), (unsigned) e8080_get_bc (c),
		(unsigned) e8080_get_de (c), (unsigned) e8080_get_hl (c),
		(unsigned) e8080_get_sp (c), (unsigned) e8080_get_pc (c),
		(unsigned) e8080_get_psw (c),
		(e8080_get_sf (c)) ? 'S' : '-',
		(e8080_get_zf (c)) ? 'Z' : '-',
		(e8080_get_af (c)) ? 'A' : '-',
		(e8080_get_pf (c)) ? 'P' : '-',
		(e8080_get_cf (c)) ? 'C' : '-',
		str
	);
}

static
void spec_print_cpu_z80_2 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	spec_disasm_cur (c, &op);
	spec_disasm_str (str, &op, 1, 1);

	pce_printf (
		"A=%02X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X"
		" SP=%04X PC=%04X F=%02X[%c%c%c%c%c] [%c%c]\n"
		"R=%02X I=%02X IM=%u IFF=%u/%u IPC=%04X ICNT=%u HALT=%u\n"
		"%s\n",
		e8080_get_a (c), e8080_get_bc (c), e8080_get_de (c),
		e8080_get_hl (c), e8080_get_ix (c), e8080_get_iy (c),
		e8080_get_sp (c), e8080_get_pc (c),
		e8080_get_psw (c),
		(e8080_get_sf (c)) ? 'S' : '-',
		(e8080_get_zf (c)) ? 'Z' : '-',
		(e8080_get_af (c)) ? 'A' : '-',
		(e8080_get_pf (c)) ? 'P' : '-',
		(e8080_get_cf (c)) ? 'C' : '-',
		c->int_val ? 'I' : '-',
		c->nmi_val ? 'N' : '-',
		e8080_get_r (c),
		e8080_get_i (c),
		e8080_get_im (c),
		e8080_get_iff1 (c), e8080_get_iff2 (c),
		e8080_get_int_pc (c), e8080_get_int_cnt (c),
		e8080_get_halt (c),
		str
	);

}

static
void spec_print_cpu_8080_2 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	spec_disasm_cur (c, &op);
	spec_disasm_str (str, &op, 1, 1);

	pce_printf (
		"A=%02X BC=%04X DE=%04X HL=%04X SP=%04X PC=%04X PSW=%02X[%c%c%c%c%c]\n"
		"%s\n",
		(unsigned) e8080_get_a (c), (unsigned) e8080_get_bc (c),
		(unsigned) e8080_get_de (c), (unsigned) e8080_get_hl (c),
		(unsigned) e8080_get_sp (c), (unsigned) e8080_get_pc (c),
		(unsigned) e8080_get_psw (c),
		(e8080_get_sf (c)) ? 'S' : '-',
		(e8080_get_zf (c)) ? 'Z' : '-',
		(e8080_get_af (c)) ? 'A' : '-',
		(e8080_get_pf (c)) ? 'P' : '-',
		(e8080_get_cf (c)) ? 'C' : '-',
		str
	);

	if (c->halt) {
		pce_printf ("HALT=1\n");
	}
}

void spec_print_cpu (e8080_t *c, int oneline)
{
	if (oneline) {
		if (e8080_get_flags (c) & E8080_FLAG_Z80) {
			spec_print_cpu_z80_1 (c);
		}
		else {
			spec_print_cpu_8080_1 (c);
		}
	}
	else {
		if (e8080_get_flags (c) & E8080_FLAG_Z80) {
			spec_print_cpu_z80_2 (c);
		}
		else {
			spec_print_cpu_8080_2 (c);
		}
	}
}

void print_state_cpu (e8080_t *c)
{
	if (e8080_get_flags (c) & E8080_FLAG_Z80) {
		pce_prt_sep ("Z80");
	}
	else {
		pce_prt_sep ("8080");
	}

	spec_print_cpu (c, 0);
}

static
void print_state_mem (spectrum_t *sim)
{
	pce_prt_sep ("MEMORY");
	mem_prt_state (sim->mem, stdout);
}

static
void print_state_system (spectrum_t *sim)
{
	pce_prt_sep ("SPECTRUM");

	pce_printf ("IM=%u\n", e8080_get_im (sim->cpu));
	pce_printf ("FE = %02X\n", sim->port_fe);
	pce_printf ("KEY=[%02X %02X %02X %02X %02X %02X %02X %02X]\n",
		sim->keys[0], sim->keys[1], sim->keys[2], sim->keys[3],
		sim->keys[4], sim->keys[5], sim->keys[6], sim->keys[7]
	);
}

static
void print_state_video (spectrum_t *sim)
{
	spec_video_t *vid;

	vid = &sim->video;

	pce_prt_sep ("VIDEO");

	pce_printf ("FRAME=%lu+%lu  ROW=%u+%u  COL=%u  DE=%d\n",
		vid->frame_counter, vid->frame_clock, vid->row, vid->row_clock, 2 * vid->row_clock, vid->de
	);
}

static
void print_state (spectrum_t *sim, const char *str)
{
	cmd_t cmd;

	cmd_set_str (&cmd, str);

	if (cmd_match_eol (&cmd)) {
		return;
	}

	while (!cmd_match_eol (&cmd)) {
		if (cmd_match (&cmd, "cpu")) {
			print_state_cpu (sim->cpu);
		}
		else if (cmd_match (&cmd, "mem")) {
			print_state_mem (sim);
		}
		else if (cmd_match (&cmd, "sys")) {
			print_state_system (sim);
		}
		else if (cmd_match (&cmd, "video")) {
			print_state_video (sim);
		}
		else {
			printf ("unknown component (%s)\n", cmd_get_str (&cmd));
			return;
		}
	}
}

static
int spec_check_break (spectrum_t *sim)
{
	if (bps_check (&sim->bps, 0, e8080_get_pc (sim->cpu), stdout)) {
		return (1);
	}

	if (sim->brk) {
		return (1);
	}

	return (0);
}

static
int spec_exec (spectrum_t *sim)
{
	unsigned n1, n2;

	n1 = e8080_get_opcnt (sim->cpu);

	do {
		spec_clock (sim);

		if (sim->brk) {
			return (1);
		}

		n2 = e8080_get_opcnt (sim->cpu);
	} while (n1 == n2);

	return (0);
}

static
int spec_exec_to (spectrum_t *sim, unsigned short addr)
{
	while (e8080_get_pc (sim->cpu) != addr) {
		spec_exec (sim);

		if (spec_check_break (sim)) {
			return (1);
		}
	}

	return (0);
}

static
int spec_exec_off (spectrum_t *sim, unsigned short addr)
{
	unsigned icnt;

	icnt = e8080_get_int_cnt (sim->cpu);

	while (e8080_get_pc (sim->cpu) == addr) {
		spec_exec (sim);

		if (spec_check_break (sim)) {
			return (1);
		}

		if (e8080_get_int_cnt (sim->cpu) != icnt) {
			if (spec_exec_to (sim, e8080_get_int_pc (sim->cpu))) {
				return (1);
			}

			icnt = e8080_get_int_cnt (sim->cpu);
		}
	}

	return (0);
}

void spec_run (spectrum_t *sim)
{
	pce_start (&sim->brk);

	spec_clock_discontinuity (sim);

	while (1) {
		spec_clock (sim);

		if (sim->brk) {
			break;
		}
	}

	pce_stop();
}


static
int spec_hook_undef (void *ext, unsigned char op)
{
	spectrum_t *sim;

	sim = ext;

	pce_log (MSG_DEB,
		"%04X: undefined operation [%02X %02X %02X %02X]\n",
		(unsigned) e8080_get_pc (sim->cpu),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu)),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu) + 1),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu) + 2),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu) + 3)
	);

	/* pce_usleep (5000); */

	return (0);
}

static
int spec_hook_rst (void *ext, unsigned char op)
{
#if 0
	unsigned  fct, ex;
	spectrum_t *sim;

	sim = ext;

	ex = (op >> 3) & 7;
#endif

	return (0);
}


static
void spec_cmd_c (cmd_t *cmd, spectrum_t *sim)
{
	unsigned short cnt;

	cnt = 1;

	cmd_match_uint16 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		spec_clock (sim);
		cnt -= 1;
	}

	print_state_cpu (sim->cpu);
}

static
void spec_cmd_g_b (cmd_t *cmd, spectrum_t *sim)
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

	spec_clock_discontinuity (sim);

	while (1) {
		spec_exec (sim);

		if (spec_check_break (sim)) {
			break;
		}
	}

	pce_stop();
}

static
void spec_cmd_g (cmd_t *cmd, spectrum_t *sim)
{
	if (cmd_match (cmd, "b")) {
		spec_cmd_g_b (cmd, sim);
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	spec_run (sim);
}

static
void spec_cmd_hm (cmd_t *cmd)
{
	pce_puts (
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
		"emu.term.fullscreen  \"0\" | \"1\"\n"
		"emu.term.fullscreen.toggle\n"
		"emu.term.grab\n"
		"emu.term.release\n"
		"emu.term.screenshot  [<filename>]\n"
		"emu.term.title       <title>\n"
	);
}

static
void spec_cmd_i (cmd_t *cmd, spectrum_t *sim)
{
	unsigned short port;

	while (cmd_match_uint16 (cmd, &port)) {
		pce_printf ("%04X: %02X\n",
			port, e8080_get_port8 (sim->cpu, port)
		);
	}

	cmd_match_end (cmd);
}

static
void spec_cmd_n (cmd_t *cmd, spectrum_t *sim)
{
	int            s;
	unsigned       pc;
	unsigned short i, n;
	e8080_disasm_t dis;

	n = 1;
	s = 0;

	if (cmd_match_uint16 (cmd, &n)) {
		s = 1;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	spec_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		spec_disasm_cur (sim->cpu, &dis);

		if (s) {
			spec_print_cpu (sim->cpu, 1);
		}

		pc = (e8080_get_pc (sim->cpu) + dis.data_cnt) & 0xffff;

		if (spec_exec_to (sim, pc)) {
			pce_printf ("T %04X\n", pc);
			break;
		}
	}

	if (s) {
		spec_print_cpu (sim->cpu, 1);
	}

	pce_stop();

	print_state_cpu (sim->cpu);
}

static
void spec_cmd_o (cmd_t *cmd, spectrum_t *sim)
{
	unsigned short port, val;

	if (!cmd_match_uint16 (cmd, &port)) {
		cmd_error (cmd, "need a port address");
		return;
	}

	if (!cmd_match_uint16 (cmd, &val)) {
		cmd_error (cmd, "need a value");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	e8080_set_port8 (sim->cpu, port, val);
}

static
void spec_cmd_p (cmd_t *cmd, spectrum_t *sim)
{
	int            s;
	unsigned short i, n;
	unsigned       pc, to, op;

	if (cmd_match (cmd, "o")) {
		spec_cmd_n (cmd, sim);
		return;
	}

	n = 1;
	s = 0;

	if (cmd_match_uint16 (cmd, &n)) {
		s = 1;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	spec_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		if (s) {
			spec_print_cpu (sim->cpu, 1);
		}

		pc = e8080_get_pc (sim->cpu);
		op = e8080_get_mem8 (sim->cpu, pc);
		to = pc;

		if ((op == 0xcd) || ((op & 0xc7) == 0xc4)) {
			/* call */
			to = (pc + 3) & 0xffff;
		}
		else if ((op & 0xc7) == 0xc7) {
			/* rst */
			to = (pc + 1) & 0xffff;
		}
		else if (op == 0x10) {
			/* djnz */
			to = (pc + 2) & 0xffff;
		}

		if (to != pc) {
			if (spec_exec_to (sim, to)) {
				pce_printf ("T %04X\n", pc);
				break;
			}
		}
		else {
			if (spec_exec_off (sim, pc)) {
				pce_printf ("F %04X\n", pc);
				break;
			}
		}
	}

	if (s) {
		spec_print_cpu (sim->cpu, 1);
	}

	pce_stop();

	print_state_cpu (sim->cpu);
}

static
void spec_cmd_r (cmd_t *cmd, spectrum_t *sim)
{
	unsigned long val;
	char          sym[256];

	if (cmd_match_eol (cmd)) {
		print_state_cpu (sim->cpu);
		return;
	}

	if (!cmd_match_ident (cmd, sym, 256)) {
		cmd_error (cmd, "missing register");
		return;
	}

	if (e8080_get_reg (sim->cpu, sym, &val)) {
		cmd_error (cmd, "bad register\n");
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

	e8080_set_reg (sim->cpu, sym, val);

	print_state_cpu (sim->cpu);
}

static
void spec_cmd_s (cmd_t *cmd, spectrum_t *sim)
{
	if (cmd_match_eol (cmd)) {
		print_state_cpu (sim->cpu);
		return;
	}

	print_state (sim, cmd_get_str (cmd));
}

static
void spec_cmd_t (cmd_t *cmd, spectrum_t *sim)
{
	int            s;
	unsigned short i, n;

	n = 1;
	s = 0;

	if (cmd_match_uint16 (cmd, &n)) {
		s = 1;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_start (&sim->brk);

	spec_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		if (s) {
			spec_print_cpu (sim->cpu, 1);
		}

		if (spec_exec (sim)) {
			break;
		}
	}

	if (s) {
		spec_print_cpu (sim->cpu, 1);
	}

	pce_stop();

	print_state_cpu (sim->cpu);
}

static
void spec_cmd_u (cmd_t *cmd, spectrum_t *sim)
{
	int                   to;
	unsigned              i;
	unsigned short        addr, cnt, taddr;
	static unsigned int   first = 1;
	static unsigned short saddr = 0;
	e8080_disasm_t        op;
	char                  str[256];

	if (first) {
		first = 0;
		saddr = e8080_get_pc (sim->cpu);
	}

	to = 0;
	addr = saddr;
	cnt = 16;

	if (cmd_match (cmd, "t")) {
		to = 1;
	}

	if (cmd_match_uint16 (cmd, &addr)) {
		cmd_match_uint16 (cmd, &cnt);
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (to) {
		if (addr < (2 * cnt)) {
			taddr = 0;
		}
		else {
			taddr = addr - 2 * cnt;
		}

		while (taddr <= addr) {
			spec_disasm_mem (sim->cpu, &op, taddr);
			spec_disasm_str (str, &op, 1, 1);

			pce_printf ("%s\n", str);

			taddr += op.data_cnt;
		}
	}
	else {
		for (i = 0; i < cnt; i++) {
			spec_disasm_mem (sim->cpu, &op, addr);
			spec_disasm_str (str, &op, 1, 1);

			pce_printf ("%s\n", str);

			addr += op.data_cnt;
		}
	}

	saddr = addr;
}

static
void spec_cmd_xl (cmd_t *cmd, spectrum_t *sim)
{
	char name[256];

	if (!cmd_match_str (cmd, name, 256)) {
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (spec_snap_load (sim, name)) {
		pce_printf ("load error (%s)\n", name);
	}
}

static
void spec_cmd_xs (cmd_t *cmd, spectrum_t *sim)
{
	char name[256];

	if (!cmd_match_str (cmd, name, 256)) {
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	if (spec_snap_save (sim, name)) {
		pce_printf ("load error (%s)\n", name);
	}
}

static
void spec_cmd_x (cmd_t *cmd, spectrum_t *sim)
{
	if (cmd_match (cmd, "l")) {
		spec_cmd_xl (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		spec_cmd_xs (cmd, sim);
	}
	else {
		cmd_error (cmd, "\n");
	}
}

int spec_cmd (spectrum_t *sim, cmd_t *cmd)
{
	if (sim->term != NULL) {
		trm_check (sim->term);
	}

	if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps);
	}
	else if (cmd_match (cmd, "c")) {
		spec_cmd_c (cmd, sim);
	}
	else if (cmd_match (cmd, "g")) {
		spec_cmd_g (cmd, sim);
	}
	else if (cmd_match (cmd, "hm")) {
		spec_cmd_hm (cmd);
	}
	else if (cmd_match (cmd, "i")) {
		spec_cmd_i (cmd, sim);
	}
	else if (cmd_match (cmd, "n")) {
		spec_cmd_n (cmd, sim);
	}
	else if (cmd_match (cmd, "o")) {
		spec_cmd_o (cmd, sim);
	}
	else if (cmd_match (cmd, "p")) {
		spec_cmd_p (cmd, sim);
	}
	else if (cmd_match (cmd, "r")) {
		spec_cmd_r (cmd, sim);
	}
	else if (cmd_match (cmd, "s")) {
		spec_cmd_s (cmd, sim);
	}
	else if (cmd_match (cmd, "t")) {
		spec_cmd_t (cmd, sim);
	}
	else if (cmd_match (cmd, "u")) {
		spec_cmd_u (cmd, sim);
	}
	else if (cmd_match (cmd, "x")) {
		spec_cmd_x (cmd, sim);
	}
	else {
		return (1);
	}

	return (0);
}

void spec_cmd_init (spectrum_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	e8080_set_hook_undef_fct (sim->cpu, sim, spec_hook_undef);
	e8080_set_hook_rst_fct (sim->cpu, sim, spec_hook_rst);
}
