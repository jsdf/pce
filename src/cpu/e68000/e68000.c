/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e68000/e68000.c                                      *
 * Created:     2005-07-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e68000.h"
#include "internal.h"


void e68_init (e68000_t *c)
{
	unsigned i;

	c->flags = 0;

	c->mem_ext = NULL;

	c->get_uint8 = NULL;
	c->get_uint16 = NULL;
	c->get_uint32 = NULL;

	c->set_uint8 = NULL;
	c->set_uint16 = NULL;
	c->set_uint32 = NULL;

	c->ram = NULL;
	c->ram_cnt = 0;

	c->reset_ext = NULL;
	c->reset = NULL;
	c->reset_val = 0;

	c->log_ext = NULL;
	c->log_opcode = NULL;
	c->log_undef = NULL;
	c->log_exception = NULL;
	c->log_mem = NULL;

	c->hook_ext = NULL;
	c->hook = NULL;

	c->supervisor = 1;
	c->halt = 2;

	c->ircnt = 0;

	c->int_ipl = 0;
	c->int_nmi = 0;
	c->int_vect = 0;
	c->int_avec = 0;

	c->delay = 1;

	c->excptn = 0;
	c->excpts = "none";

	c->oprcnt = 0;
	c->clkcnt = 0;

	e68_set_opcodes (c);

	c->sr = E68_SR_S;

	for (i = 0; i < 8; i++) {
		e68_set_dreg32 (c, i, 0);
		e68_set_areg32 (c, i, 0);
	}

	e68_set_usp (c, 0);
	e68_set_ssp (c, 0);
	e68_set_pc (c, 0);

	e68_set_vbr (c, 0);

	e68_set_sfc (c, 0);
	e68_set_dfc (c, 0);

	c->last_pc = 0;
	c->last_trap_a = 0;
	c->last_trap_f = 0;
}

e68000_t *e68_new (void)
{
	e68000_t *c;

	c = malloc (sizeof (e68000_t));
	if (c == NULL) {
		return (NULL);
	}

	e68_init (c);

	return (c);
}

void e68_free (e68000_t *c)
{
}

void e68_del (e68000_t *c)
{
	if (c != NULL) {
		e68_free (c);
		free (c);
	}
}


void e68_set_mem_fct (e68000_t *c, void *ext,
	void *get8, void *get16, void *get32,
	void *set8, void *set16, void *set32)
{
	c->mem_ext = ext;

	c->get_uint8 = get8;
	c->get_uint16 = get16;
	c->get_uint32 = get32;

	c->set_uint8 = set8;
	c->set_uint16 = set16;
	c->set_uint32 = set32;
}

void e68_set_ram (e68000_t *c, unsigned char *ram, unsigned long cnt)
{
	if (ram == NULL) {
		cnt = 0;
	}

	c->ram = ram;
	c->ram_cnt = cnt;
}

void e68_set_reset_fct (e68000_t *c, void *ext, void *fct)
{
	c->reset_ext = ext;
	c->reset = fct;
}

void e68_set_hook_fct (e68000_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook = fct;
}

void e68_set_flags (e68000_t *c, unsigned flags, int set)
{
	if (set) {
		c->flags |= flags;
	}
	else {
		c->flags &= ~flags;
	}
}

void e68_set_address_check (e68000_t *c, int check)
{
	if (check) {
		c->flags &= ~E68_FLAG_NOADDR;
	}
	else {
		c->flags |= E68_FLAG_NOADDR;
	}
}

void e68_set_68000 (e68000_t *c)
{
	c->flags = 0;
}

void e68_set_68010 (e68000_t *c)
{
	c->flags = E68_FLAG_68010;
}

unsigned long long e68_get_opcnt (e68000_t *c)
{
	return (c->oprcnt);
}

unsigned long long e68_get_clkcnt (e68000_t *c)
{
	return (c->clkcnt);
}

unsigned long e68_get_delay (e68000_t *c)
{
	return (c->delay);
}

unsigned e68_get_halt (e68000_t *c)
{
	return (c->halt);
}

void e68_set_halt (e68000_t *c, unsigned val)
{
	c->halt = val & 0x03;
}

unsigned e68_get_exception (e68000_t *c)
{
	return (c->excptn);
}

const char *e68_get_exception_name (e68000_t *c)
{
	return (c->excpts);
}

unsigned long e68_get_last_pc (e68000_t *c)
{
	return (c->last_pc);
}

unsigned short e68_get_last_trap_a (e68000_t *c)
{
	return (c->last_trap_a);
}

unsigned short e68_get_last_trap_f (e68000_t *c)
{
	return (c->last_trap_f);
}

int e68_get_reg (e68000_t *c, const char *reg, unsigned long *val)
{
	char     type;
	unsigned idx;

	if (reg[0] == '%') {
		reg += 1;
	}

	if (strcmp (reg, "pc") == 0) {
		*val = e68_get_pc (c);
		return (0);
	}
	else if (strcmp (reg, "lpc") == 0) {
		*val = c->last_pc;
		return (0);
	}
	else if ((reg[0] == 'o') && (reg[1] == 'p')) {
		idx = 0;
		while ((reg[2] >= '0') && (reg[2] <= '9')) {
			idx = 10 * idx + (unsigned) (reg[2] - '0');
			reg += 1;
		}

		if (reg[2] != 0) {
			return (1);
		}

		*val = e68_get_mem16 (c, e68_get_pc (c) + 2 * idx);

		return (0);
	}
	else if (strcmp (reg, "sr") == 0) {
		*val = e68_get_sr (c);
		return (0);
	}
	else if (strcmp (reg, "sp") == 0) {
		*val = e68_get_areg32 (c, 7);
		return (0);
	}
	else if (strcmp (reg, "ccr") == 0) {
		*val = e68_get_ccr (c);
		return (0);
	}
	else if (strcmp (reg, "usp") == 0) {
		*val = e68_get_usp (c);
		return (0);
	}
	else if (strcmp (reg, "ssp") == 0) {
		*val = e68_get_ssp (c);
		return (0);
	}

	if ((reg[0] == 'a') || (reg[0] == 'A')) {
		type = 'a';

	}
	else if ((reg[0] == 'd') || (reg[0] == 'D')) {
		type = 'd';
	}
	else {
		return (1);
	}

	reg += 1;
	idx = 0;

	while ((reg[0] >= '0') && (reg[0] <= '9')) {
		idx = 10 * idx + (unsigned) (reg[0] - '0');
		reg += 1;
	}

	if (type == 'a') {
		*val = e68_get_areg32 (c, idx & 7);
	}
	else {
		*val = e68_get_dreg32 (c, idx & 7);
	}

	if (reg[0] == '.') {
		switch (reg[1]) {
		case 'b':
		case 'B':
			*val &= 0xff;
			break;

		case 'w':
		case 'W':
			*val &= 0xffff;
			break;

		case 'l':
		case 'L':
			*val &= 0xffffffff;
			break;

		default:
			return (1);
		}

		reg += 2;
	}

	if (reg[0] != 0) {
		return (1);
	}

	return (0);
}

int e68_set_reg (e68000_t *c, const char *reg, unsigned long val)
{
	char          type;
	unsigned      idx;
	unsigned long mask;

	if (reg[0] == '%') {
		reg += 1;
	}

	if (strcmp (reg, "pc") == 0) {
		e68_set_pc (c, val);
		return (0);
	}
	else if (strcmp (reg, "lpc") == 0) {
		c->last_pc = val;
		return (0);
	}
	else if (strcmp (reg, "sr") == 0) {
		e68_set_sr (c, val);
		return (0);
	}
	else if (strcmp (reg, "sp") == 0) {
		e68_set_areg32 (c, 7, val);
		return (0);
	}
	else if (strcmp (reg, "ccr") == 0) {
		e68_set_ccr (c, val);
		return (0);
	}
	else if (strcmp (reg, "usp") == 0) {
		e68_set_usp (c, val);
		return (0);
	}
	else if (strcmp (reg, "ssp") == 0) {
		e68_set_ssp (c, val);
		return (0);
	}

	if ((reg[0] == 'a') || (reg[0] == 'A')) {
		type = 'a';

	}
	else if ((reg[0] == 'd') || (reg[0] == 'D')) {
		type = 'd';
	}
	else {
		return (1);
	}

	reg += 1;
	idx = 0;

	while ((reg[0] >= '0') && (reg[0] <= '9')) {
		idx = 10 * idx + (unsigned) (reg[0] - '0');
		reg += 1;
	}

	idx &= 7;

	mask = 0xffffffff;

	if (reg[0] == '.') {
		switch (reg[1]) {
		case 'b':
		case 'B':
			mask = 0x000000ff;
			break;

		case 'w':
		case 'W':
			mask = 0x0000ffff;
			break;

		case 'l':
		case 'L':
			mask = 0xffffffff;
			break;

		default:
			return (1);
		}

		reg += 2;
	}

	if (reg[0] != 0) {
		return (1);
	}

	if (type == 'a') {
		val = (e68_get_areg32 (c, idx) & ~mask) | (val & mask);
		e68_set_areg32 (c, idx, val);
	}
	else {
		val = (e68_get_dreg32 (c, idx) & ~mask) | (val & mask);
		e68_set_dreg32 (c, idx, val);
	}

	return (0);
}

static
void e68_set_supervisor (e68000_t *c, int supervisor)
{
	if (c->supervisor) {
		c->ssp = e68_get_areg32 (c, 7);
	}
	else {
		c->usp = e68_get_areg32 (c, 7);
	}

	if (supervisor) {
		e68_set_areg32 (c, 7, c->ssp);
	}
	else {
		e68_set_areg32 (c, 7, c->usp);
	}

	c->supervisor = (supervisor != 0);
}

void e68_set_sr (e68000_t *c, unsigned short val)
{
	if ((c->sr ^ val) & E68_SR_S) {
		e68_set_supervisor (c, (val & E68_SR_S) != 0);
	}

	c->sr = val & E68_SR_MASK;
}

static
void e68_exception (e68000_t *c, unsigned vct, unsigned fmt, const char *name)
{
	uint16_t sr1, sr2;
	uint32_t addr;

	vct &= 0xff;

	c->excptn = vct;
	c->excpts = name;

	if (c->log_exception != NULL) {
		c->log_exception (c->log_ext, vct);
	}

	sr1 = e68_get_sr (c);
	sr2 = sr1;
	sr2 &= ~E68_SR_T;
	sr2 |= E68_SR_S;
	e68_set_sr (c, sr2);

	if (c->flags & E68_FLAG_68010) {
		e68_push16 (c, ((fmt & 15) << 12) | (vct << 2));
	}

	e68_push32 (c, e68_get_pc (c));
	e68_push16 (c, sr1);

	addr = (e68_get_vbr (c) + (vct << 2)) & 0xffffffff;

	e68_set_pc (c, e68_get_mem32 (c, addr));
}

void e68_exception_reset (e68000_t *c)
{
	uint16_t sr;

	c->excptn = 0;
	c->excpts = "RSET";

	sr = e68_get_sr (c);
	sr &= ~E68_SR_T;
	sr |= E68_SR_S;
	e68_set_sr (c, sr);

	e68_set_areg32 (c, 7, e68_get_mem32 (c, 0));
	e68_set_pc (c, e68_get_mem32 (c, 4));

	e68_set_clk (c, 64);
}

void e68_exception_address (e68000_t *c, uint32_t addr, int data, int wr)
{
	uint16_t val;

	e68_exception (c, 3, 8, "ADDR");

	e68_push16 (c, c->ir[0]);
	e68_push32 (c, addr);

	val = 0;

	if (wr == 0) {
		val |= 0x10;
	}

	if (data) {
		val |= 0x08;
	}

	e68_push16 (c, val);

	e68_set_clk (c, 64);
}

void e68_exception_illegal (e68000_t *c)
{
	if (c->log_undef != NULL) {
		c->log_undef (c->log_ext, c->ir[0]);
	}

	e68_exception (c, 4, 0, "ILLG");
	e68_set_clk (c, 62);
}

void e68_exception_divzero (e68000_t *c)
{
	e68_exception (c, 5, 0, "DIVZ");
	e68_set_clk (c, 66);
}

void e68_exception_check (e68000_t *c)
{
	e68_exception (c, 6, 0, "CHK");
	e68_set_clk (c, 68);
}

void e68_exception_overflow (e68000_t *c)
{
	e68_exception (c, 7, 0, "OFLW");
	e68_set_clk (c, 68);
}

void e68_exception_privilege (e68000_t *c)
{
	e68_exception (c, 8, 0, "PRIV");
	e68_set_clk (c, 62);
}

void e68_exception_axxx (e68000_t *c)
{
	e68_exception (c, 10, 0, "AXXX");
	e68_set_clk (c, 62);
}

void e68_exception_fxxx (e68000_t *c)
{
	e68_exception (c, 11, 0, "FXXX");
	e68_set_clk (c, 62);
}

void e68_exception_format (e68000_t *c)
{
	e68_exception (c, 14, 0, "FRMT");
	e68_set_clk (c, 62);
}

void e68_exception_avec (e68000_t *c, unsigned level)
{
	e68_exception (c, 24 + level, 0, "AVEC");
	e68_set_iml (c, level & 7);
	e68_set_clk (c, 62);
}

void e68_exception_trap (e68000_t *c, unsigned n)
{
	e68_exception (c, 32 + n, 0, "TRAP");
	e68_set_clk (c, 62);
}

void e68_exception_intr (e68000_t *c, unsigned level, unsigned vect)
{
	e68_exception (c, vect, 0, "INTR");
	e68_set_iml (c, level & 7);
	e68_set_clk (c, 62);
}

void e68_interrupt (e68000_t *c, unsigned level, unsigned vect, int avec)
{
	if (level > 0) {
		c->halt &= ~1U;
	}

	if ((level == 7) && (c->int_ipl != 7)) {
		c->int_nmi = 1;
	}

	c->int_ipl = level;
	c->int_vect = vect;
	c->int_avec = avec;
}

static
void e68_set_reset (e68000_t *c, unsigned char val)
{
	if ((c->reset_val != 0) == (val != 0)) {
		return;
	}

	c->reset_val = (val != 0);

	if (c->reset != NULL) {
		c->reset (c->reset_ext, val);
	}
}

void e68_reset (e68000_t *c)
{
	unsigned i;

	if (c->reset_val) {
		return;
	}

	e68_set_reset (c, 1);

	e68_set_sr (c, E68_SR_S);

	for (i = 0; i < 8; i++) {
		e68_set_dreg32 (c, i, 0);
		e68_set_areg32 (c, i, 0);
	}

	e68_set_usp (c, 0);
	e68_set_ssp (c, 0);

	e68_set_vbr (c, 0);

	e68_set_sfc (c, 0);
	e68_set_dfc (c, 0);

	c->halt = 0;

	e68_exception_reset (c);

	e68_set_reset (c, 0);
}

void e68_execute (e68000_t *c)
{
	unsigned opc;
	unsigned n;

	c->last_pc = e68_get_pc (c);

	if (c->pc & 1) {
		e68_exception_address (c, c->pc, 0, 0);
	}

	c->ir[0] = e68_get_mem16 (c, c->pc);
	c->ircnt = 1;

	if (c->log_opcode != NULL) {
		c->log_opcode (c->log_ext, c->ir[0]);
	}

	opc = (c->ir[0] >> 6) & 0x3ff;

	n = c->opcodes[opc] (c);

	e68_set_pc (c, e68_get_pc (c) + 2 * n);

	c->oprcnt += 1;

	if (c->int_nmi) {
		e68_exception_avec (c, 7);
		c->int_nmi = 0;
	}
	else if (c->int_ipl > 0) {
		unsigned iml;

		iml = e68_get_iml (c);

		if (iml < c->int_ipl) {
			if (c->int_avec) {
				e68_exception_avec (c, c->int_ipl);
			}
			else {
				e68_exception_intr (c, c->int_ipl, c->int_vect);
			}
		}
	}
}

void e68_clock (e68000_t *c, unsigned long n)
{
	while (n >= c->delay) {
		n -= c->delay;

		c->clkcnt += c->delay;
		c->delay = 0;

		if (c->halt) {
			return;
		}

		e68_execute (c);

		if (c->delay == 0) {
			fprintf (stderr, "warning: delay == 0 at %08lx\n",
				(unsigned long) e68_get_pc (c)
			);
			fflush (stderr);
			break;
		}
	}

	c->clkcnt += n;
	c->delay -= n;
}
