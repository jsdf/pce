/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/cpm80/cmd.c                                         *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2016 Hampa Hug <hampa@hampa.ch>                     *
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
#include "bios.h"
#include "cpm80.h"

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
	{ "i", "port", "input a byte from a port" },
	{ "o", "port val", "output a byte to a port" },
	{ "p", "[cnt]", "execute cnt instructions, skip calls [1]" },
	{ "r", "reg [val]", "set a register" },
	{ "s", "[what]", "print status (cpu|mem)" },
	{ "t", "[cnt]", "execute cnt instructions [1]" },
	{ "u", "[addr [cnt]]", "disassemble" }
};


static
void c80_disasm_str (char *dst, e8080_disasm_t *op, int with_pc, int with_comment)
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
void c80_disasm_cur (e8080_t *c, e8080_disasm_t *op)
{
	if (e8080_get_flags (c) & E8080_FLAG_Z80) {
		z80_disasm_cur (c, op);
	}
	else {
		e8080_disasm_cur (c, op);
	}
}

static
void c80_disasm_mem (e8080_t *c, e8080_disasm_t *op, unsigned short addr)
{
	if (e8080_get_flags (c) & E8080_FLAG_Z80) {
		z80_disasm_mem (c, op, addr);
	}
	else {
		e8080_disasm_mem (c, op, addr);
	}
}

static
void c80_print_cpu_z80_1 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	c80_disasm_cur (c, &op);
	c80_disasm_str (str, &op, 0, 0);

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
void c80_print_cpu_8080_1 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	c80_disasm_cur (c, &op);
	c80_disasm_str (str, &op, 0, 0);

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
void c80_print_cpu_z80_2 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	c80_disasm_cur (c, &op);
	c80_disasm_str (str, &op, 1, 1);

	pce_printf (
		"A=%02X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X SP=%04X PC=%04X PSW=%02X[%c%c%c%c%c]\n"
		"%s\n",
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

	if (c->halt) {
		pce_printf ("HALT=1\n");
	}
}

static
void c80_print_cpu_8080_2 (e8080_t *c)
{
	e8080_disasm_t op;
	char           str[256];

	c80_disasm_cur (c, &op);
	c80_disasm_str (str, &op, 1, 1);

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

void c80_print_cpu (e8080_t *c, int oneline)
{
	if (oneline) {
		if (e8080_get_flags (c) & E8080_FLAG_Z80) {
			c80_print_cpu_z80_1 (c);
		}
		else {
			c80_print_cpu_8080_1 (c);
		}
	}
	else {
		if (e8080_get_flags (c) & E8080_FLAG_Z80) {
			c80_print_cpu_z80_2 (c);
		}
		else {
			c80_print_cpu_8080_2 (c);
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

	c80_print_cpu (c, 0);
}

static
void print_state_mem (cpm80_t *sim)
{
	pce_prt_sep ("MEMORY");
	mem_prt_state (sim->mem, stdout);
}

static
void print_state (cpm80_t *sim, const char *str)
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
		else {
			printf ("unknown component (%s)\n", cmd_get_str (&cmd));
			return;
		}
	}
}

static
int c80_check_break (cpm80_t *sim)
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
void c80_exec (cpm80_t *sim)
{
	unsigned long old;

	old = e8080_get_opcnt (sim->cpu);

	while (e8080_get_opcnt (sim->cpu) == old) {
		c80_clock (sim, 4);

		if (sim->brk) {
			break;
		}
	}
}

static
int c80_exec_to (cpm80_t *sim, unsigned short addr)
{
	while (e8080_get_pc (sim->cpu) != addr) {
		c80_clock (sim, 4);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

static
int c80_exec_off (cpm80_t *sim, unsigned short addr)
{
	while (e8080_get_pc (sim->cpu) == addr) {
		c80_clock (sim, 4);

		if (sim->brk) {
			return (1);
		}
	}

	return (0);
}

void c80_run (cpm80_t *sim)
{
	pce_start (&sim->brk);

	c80_clock_discontinuity (sim);

	while (1) {
		c80_clock (sim, 64);

		if (sim->brk) {
			break;
		}
	}

	pce_stop();
}


static
int c80_hook_undef (void *ext, unsigned char op)
{
	cpm80_t *sim;

	sim = ext;

	pce_log (MSG_DEB,
		"%04X: undefined operation [%02X %02X %02X %02X]\n",
		(unsigned) e8080_get_pc (sim->cpu),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu)),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu) + 1),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu) + 2),
		(unsigned) e8080_get_mem8 (sim->cpu, e8080_get_pc (sim->cpu) + 3)
	);

	pce_usleep (5000);

	return (0);
}

static
int c80_hook_rst (void *ext, unsigned char op)
{
	unsigned  fct, ex;
	cpm80_t *sim;

	sim = ext;

	ex = (op >> 3) & 7;

	if (ex == 0) {
		fct = e8080_get_pc (sim->cpu);

		if (fct < sim->addr_bios) {
			return (0);
		}

		fct = (fct & 0x3f) / 2;

		c80_bios (sim, fct);

		return (1);
	}
	else if (ex == 7) {
		if (mem_get_uint16_le (sim->mem, 56) == 0) {
			c80_stop (sim);
			return (1);
		}
	}

	return (0);
}


static
void c80_cmd_c (cpm80_t *sim, cmd_t *cmd)
{
	unsigned short cnt;

	cnt = 1;

	cmd_match_uint16 (cmd, &cnt);

	if (!cmd_match_end (cmd)) {
		return;
	}

	while (cnt > 0) {
		c80_clock (sim, 1);
		cnt -= 1;
	}

	print_state_cpu (sim->cpu);
}

static
void c80_cmd_g_b (cpm80_t *sim, cmd_t *cmd)
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

	c80_clock_discontinuity (sim);

	while (1) {
		c80_exec (sim);

		if (c80_check_break (sim)) {
			break;
		}
	}

	pce_stop();
}

static
void c80_cmd_g (cpm80_t *sim, cmd_t *cmd)
{
	if (cmd_match (cmd, "b")) {
		c80_cmd_g_b (sim, cmd);
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	c80_run (sim);
}

static
void c80_cmd_i (cpm80_t *sim, cmd_t *cmd)
{
	unsigned short port;

	if (!cmd_match_uint16 (cmd, &port)) {
		cmd_error (cmd, "need a port address");
		return;
	}

	if (!cmd_match_end (cmd)) {
		return;
	}

	pce_printf ("%04X: %02X\n", port, e8080_get_port8 (sim->cpu, port));
}

static
void c80_cmd_o (cpm80_t *sim, cmd_t *cmd)
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
void c80_cmd_p (cpm80_t *sim, cmd_t *cmd)
{
	int            s;
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

	c80_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		c80_disasm_cur (sim->cpu, &dis);

		if (s) {
			c80_print_cpu (sim->cpu, 1);
		}

		if (dis.flags & E8080_OPF_CALL) {
			if (c80_exec_to (sim, dis.pc + dis.data_cnt)) {
				break;
			}
		}
		else {
			if (c80_exec_off (sim, dis.pc)) {
				break;
			}
		}
	}

	if (s) {
		c80_print_cpu (sim->cpu, 1);
	}

	pce_stop();

	print_state_cpu (sim->cpu);
}

static
void c80_cmd_r (cpm80_t *sim, cmd_t *cmd)
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
void c80_cmd_s (cpm80_t *sim, cmd_t *cmd)
{
	if (cmd_match_eol (cmd)) {
		print_state_cpu (sim->cpu);
		return;
	}

	print_state (sim, cmd_get_str (cmd));
}

static
void c80_cmd_t (cpm80_t *sim, cmd_t *cmd)
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

	c80_clock_discontinuity (sim);

	for (i = 0; i < n; i++) {
		if (s) {
			c80_print_cpu (sim->cpu, 1);
		}

		c80_exec (sim);
	}

	if (s) {
		c80_print_cpu (sim->cpu, 1);
	}

	pce_stop();

	print_state_cpu (sim->cpu);
}

static
void c80_cmd_u (cpm80_t *sim, cmd_t *cmd)
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
			c80_disasm_mem (sim->cpu, &op, taddr);
			c80_disasm_str (str, &op, 1, 1);

			pce_printf ("%s\n", str);

			taddr += op.data_cnt;
		}
	}
	else {
		for (i = 0; i < cnt; i++) {
			c80_disasm_mem (sim->cpu, &op, addr);
			c80_disasm_str (str, &op, 1, 1);

			pce_printf ("%s\n", str);

			addr += op.data_cnt;
		}
	}

	saddr = addr;
}

int c80_cmd (cpm80_t *sim, cmd_t *cmd)
{
	if (cmd_match (cmd, "b")) {
		cmd_do_b (cmd, &sim->bps);
	}
	else if (cmd_match (cmd, "c")) {
		c80_cmd_c (sim, cmd);
	}
	else if (cmd_match (cmd, "g")) {
		c80_cmd_g (sim, cmd);
	}
	else if (cmd_match (cmd, "i")) {
		c80_cmd_i (sim, cmd);
	}
	else if (cmd_match (cmd, "o")) {
		c80_cmd_o (sim, cmd);
	}
	else if (cmd_match (cmd, "p")) {
		c80_cmd_p (sim, cmd);
	}
	else if (cmd_match (cmd, "r")) {
		c80_cmd_r (sim, cmd);
	}
	else if (cmd_match (cmd, "s")) {
		c80_cmd_s (sim, cmd);
	}
	else if (cmd_match (cmd, "t")) {
		c80_cmd_t (sim, cmd);
	}
	else if (cmd_match (cmd, "u")) {
		c80_cmd_u (sim, cmd);
	}
	else {
		return (1);
	}

	return (0);
}

void c80_cmd_init (cpm80_t *sim, monitor_t *mon)
{
	mon_cmd_add (mon, par_cmd, sizeof (par_cmd) / sizeof (par_cmd[0]));
	mon_cmd_add_bp (mon);

	e8080_set_hook_undef_fct (sim->cpu, sim, c80_hook_undef);
	e8080_set_hook_rst_fct (sim->cpu, sim, c80_hook_rst);
}
