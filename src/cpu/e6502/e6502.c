/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e6502/e6502.c                                        *
 * Created:     2004-05-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e6502.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


/* #define E6502_ENABLE_HOOK_ALL 1 */


void e6502_init (e6502_t *c)
{
	unsigned i;

	c->flags = 0;

	c->ea = 0;
	c->ea_page = 0;

	c->rst_val = 1;
	c->irq_val = 0;
	c->nmi_val = 0;
	c->nmi_pnd = 0;

	c->mem_rd_ext = NULL;
	c->mem_wr_ext = NULL;

	c->get_uint8 = NULL;
	c->set_uint8 = NULL;

	for (i = 0; i < 64; i++) {
		c->mem_map_rd[i] = NULL;
		c->mem_map_wr[i] = NULL;
	}

	c->hook_ext = NULL;
	c->hook_all = NULL;
	c->hook_undef = NULL;
	c->hook_brk = NULL;

	for (i = 0; i < 256; i++) {
		c->op[i] = e6502_opcodes[i];
	}

	c->delay = 0;
	c->clkcnt = 0;
	c->inscnt = 0;
}

e6502_t *e6502_new (void)
{
	e6502_t *c;

	c = (e6502_t *) malloc (sizeof (e6502_t));
	if (c == NULL) {
		return (NULL);
	}

	e6502_init (c);

	return (c);
}

void e6502_free (e6502_t *c)
{
}

void e6502_del (e6502_t *c)
{
	if (c != NULL) {
		e6502_free (c);
		free (c);
	}
}

void e6502_set_mem_map_rd (e6502_t *c, unsigned addr1, unsigned addr2, unsigned char *p)
{
	while (addr1 < addr2) {
		c->mem_map_rd[(addr1 >> 10) & 0x3f] = p;

		if (p != NULL) {
			p += 1024;
		}

		addr1 += 1024;
	}
}

void e6502_set_mem_map_wr (e6502_t *c, unsigned addr1, unsigned addr2, unsigned char *p)
{
	while (addr1 < addr2) {
		c->mem_map_wr[(addr1 >> 10) & 0x3f] = p;

		if (p != NULL) {
			p += 1024;
		}

		addr1 += 1024;
	}
}

unsigned e6502_get_flags (e6502_t *c)
{
	return (c->flags);
}

void e6502_set_flags (e6502_t *c, unsigned flags)
{
	c->flags = flags;
}

void e6502_set_ioport (e6502_t *c, int enable)
{
	if (enable) {
		c->flags |= E6502_FLAG_IOPORT;
	}
	else {
		c->flags &= ~E6502_FLAG_IOPORT;
	}
}

void e6502_set_mem_read_fct (e6502_t *c, void *ext, void *get8)
{
	c->mem_rd_ext = ext;
	c->get_uint8 = get8;
}

void e6502_set_mem_write_fct (e6502_t *c, void *ext, void *set8)
{
	c->mem_wr_ext = ext;
	c->set_uint8 = set8;
}

void e6502_set_mem_f (e6502_t *c, void *mem, void *get8, void *set8)
{
	c->mem_rd_ext = mem;
	c->mem_wr_ext = mem;

	c->get_uint8 = get8;
	c->set_uint8 = set8;
}

void e6502_set_hook_all_fct (e6502_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook_all = fct;
}

void e6502_set_hook_undef_fct (e6502_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook_undef = fct;
}

void e6502_set_hook_brk_fct (e6502_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook_brk = fct;
}

void e6502_set_ioport_fct (e6502_t *c, void *ext, void *fct)
{
	c->set_ioport_ext = ext;
	c->set_ioport = fct;
}

unsigned char e6502_get_ioport_8 (e6502_t *c, unsigned long addr)
{
	unsigned char val;

	if (addr == 0) {
		return (c->ioport[0]);
	}

	val = c->ioport[1] & c->ioport[0];
	val |= c->ioport[2] & ~c->ioport[0];

	return (val);
}

void e6502_set_ioport_8 (e6502_t *c, unsigned long addr, unsigned char val)
{
	c->ioport[addr & 1] = val & 0x3f;

	if (c->set_ioport != NULL) {
		val = c->ioport[1] & c->ioport[0];
		val |= c->ioport[2] & ~c->ioport[0];

		c->set_ioport (c->set_ioport_ext, val);
	}
}

void e6502_set_ioport_inp (e6502_t *c, unsigned char val)
{
	c->ioport[2] = val & 0x3f;

	if (c->set_ioport != NULL) {
		val = c->ioport[1] & c->ioport[0];
		val |= c->ioport[2] & ~c->ioport[0];

		c->set_ioport (c->set_ioport_ext, val);
	}
}

void e6502_set_ioport_ddr (e6502_t *c, unsigned char val)
{
	c->ioport[0] = val & 0x3f;

	if (c->set_ioport != NULL) {
		val = c->ioport[1] & c->ioport[0];
		val |= c->ioport[2] & ~c->ioport[0];

		c->set_ioport (c->set_ioport_ext, val);
	}
}

int e6502_get_reg (e6502_t *c, const char *reg, unsigned long *val)
{
	if (*reg == '%') {
		reg += 1;
	}

	if (strcmp (reg, "a") == 0) {
		*val = e6502_get_a (c);
		return (0);
	}
	else if (strcmp (reg, "x") == 0) {
		*val = e6502_get_x (c);
		return (0);
	}
	else if (strcmp (reg, "y") == 0) {
		*val = e6502_get_y (c);
		return (0);
	}
	else if (strcmp (reg, "p") == 0) {
		*val = e6502_get_p (c);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		*val = e6502_get_pc (c);
		return (0);
	}
	else if (strcmp (reg, "lpc") == 0) {
		*val = e6502_get_lpc (c);
		return (0);
	}
	else if (strcmp (reg, "s") == 0) {
		*val = e6502_get_s (c);
		return (0);
	}

	return (1);
}

int e6502_set_reg (e6502_t *c, const char *reg, unsigned long val)
{
	if (*reg == '%') {
		reg += 1;
	}

	if (strcmp (reg, "a") == 0) {
		e6502_set_a (c, val);
		return (0);
	}
	else if (strcmp (reg, "x") == 0) {
		e6502_set_x (c, val);
		return (0);
	}
	else if (strcmp (reg, "y") == 0) {
		e6502_set_y (c, val);
		return (0);
	}
	else if (strcmp (reg, "p") == 0) {
		e6502_set_p (c, val);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		e6502_set_pc (c, val);
		return (0);
	}
	else if (strcmp (reg, "lpc") == 0) {
		e6502_set_lpc (c, val);
		return (0);
	}
	else if (strcmp (reg, "s") == 0) {
		e6502_set_s (c, val);
		return (0);
	}

	return (1);
}

unsigned long long e6502_get_clock (e6502_t *c)
{
	return (c->clkcnt);
}

unsigned long long e6502_get_opcnt (e6502_t *c)
{
	return (c->inscnt);
}

unsigned long e6502_get_delay (e6502_t *c)
{
	return (c->delay);
}

void e6502_set_reset (e6502_t *c, unsigned char val)
{
	if ((c->rst_val != 0) && (val == 0)) {
		e6502_reset (c);
	}

	c->rst_val = (val != 0);
}

void e6502_set_irq (e6502_t *c, unsigned char val)
{
	c->irq_val = (val != 0);
}

void e6502_set_nmi (e6502_t *c, unsigned char val)
{
	if ((c->nmi_val == 0) && (val != 0)) {
		c->nmi_pnd = 1;
	}

	c->nmi_val = (val != 0);
}


int e6502_hook_all (e6502_t *c)
{
	if (c->hook_all != NULL) {
		return (c->hook_all (c->hook_ext, c->inst[0]));
	}

	return (0);
}

int e6502_hook_undefined (e6502_t *c)
{
	if (c->hook_undef != NULL) {
		return (c->hook_undef (c->hook_ext, c->inst[0]));
	}

	return (0);
}

int e6502_hook_brk (e6502_t *c)
{
	if (c->hook_brk != NULL) {
		return (c->hook_brk (c->hook_ext, c->inst[0]));
	}

	return (0);
}

void e6502_reset (e6502_t *c)
{
	c->delay = 7;
	c->rst_val = 0;
	c->nmi_pnd = 0;

	c->ioport[0] = 0;
	c->ioport[1] = 0;

	e6502_set_a (c, 0x00);
	e6502_set_x (c, 0x00);
	e6502_set_y (c, 0x00);
	e6502_set_s (c, 0x00);
	e6502_set_p (c, E6502_FLG_R | E6502_FLG_I);
	e6502_set_pc (c, e6502_get_mem16 (c, 0xfffc));
	e6502_set_lpc (c, 0);
}

void e6502_execute (e6502_t *c)
{
	unsigned short pc;

	c->check_irq = (e6502_get_if (c) == 0);

	pc = e6502_get_pc (c);

	c->inst[0] = e6502_get_mem8 (c, pc);

#ifdef E6502_ENABLE_HOOK_ALL
	if (c->hook_all != NULL) {
		if (e6502_hook_all (c)) {
			return;
		}
	}
#endif

	c->op[c->inst[0]] (c);

	if (c->nmi_pnd) {
		c->nmi_pnd = 0;
		e6502_trap (c, 0xfffa);
	}
	else if (c->check_irq) {
		if (c->irq_val != 0) {
			e6502_trap (c, 0xfffe);
		}
	}

	e6502_set_lpc (c, pc);

	c->inscnt += 1;
}

void e6502_clock (e6502_t *c, unsigned n)
{
	if (c->rst_val) {
		return;
	}

	while (n >= c->delay) {
		n -= c->delay;
		c->clkcnt += c->delay;
		c->delay = 0;
		e6502_execute (c);
	}

	c->delay -= n;
	c->clkcnt += n;
}
