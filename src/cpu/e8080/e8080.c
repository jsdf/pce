/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e8080/e8080.c                                        *
 * Created:     2012-11-28 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2024 Hampa Hug <hampa@hampa.ch>                     *
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


#include "e8080.h"
#include "internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* #define E8080_ENABLE_HOOK_ALL 1 */


void e8080_init (e8080_t *c)
{
	unsigned i;

	c->flags = 0;

	c->int_val = 0;
	c->nmi_val = 0;
	c->int_req = 0;

	c->int_cnt = 0;
	c->int_pc = 0;

	c->mem_rd_ext = NULL;
	c->mem_wr_ext = NULL;

	c->get_uint8 = NULL;
	c->set_uint8 = NULL;

	for (i = 0; i < 64; i++) {
		c->mem_map_rd[i] = NULL;
		c->mem_map_wr[i] = NULL;
	}

	c->port_rd_ext = NULL;
	c->port_wr_ext = NULL;

	c->get_port8 = NULL;
	c->set_port8 = NULL;

	c->hook_ext = NULL;
	c->hook_all = NULL;
	c->hook_undef = NULL;
	c->hook_rst = NULL;

	c->delay = 0;
	c->clkcnt = 0;
	c->inscnt = 0;

	e8080_set_opcodes (c);
}

e8080_t *e8080_new (void)
{
	e8080_t *c;

	c = malloc (sizeof (e8080_t));

	if (c == NULL) {
		return (NULL);
	}

	e8080_init (c);

	return (c);
}

void e8080_free (e8080_t *c)
{
}

void e8080_del (e8080_t *c)
{
	if (c != NULL) {
		e8080_free (c);
		free (c);
	}
}

static
void e8080_set_mem_map (unsigned char **map, unsigned addr1, unsigned addr2, unsigned char *p)
{
	if (addr1 & 1023) {
		if (p != NULL) {
			p += 1024 - (addr1 & 1023);
		}

		map[(addr1 >> 10) & 0x3f] = NULL;
		addr1 = (addr1 + 1023) & ~1023U;
	}

	if ((addr2 & 1023) != 1023) {
		map[(addr2 >> 10) & 0x3f] = NULL;
		addr2 = addr2 & ~1023U;

		if (addr2 > 0) {
			addr2 -= 1;
		}
	}

	while (addr1 < addr2) {
		map[(addr1 >> 10) & 0x3f] = p;

		if (p != NULL) {
			p += 1024;
		}

		addr1 += 1024;
	}
}

void e8080_set_mem_map_rd (e8080_t *c, unsigned addr1, unsigned addr2, unsigned char *p)
{
	e8080_set_mem_map (c->mem_map_rd, addr1, addr2, p);
}

void e8080_set_mem_map_wr (e8080_t *c, unsigned addr1, unsigned addr2, unsigned char *p)
{
	e8080_set_mem_map (c->mem_map_wr, addr1, addr2, p);
}

void e8080_set_8080 (e8080_t *c)
{
	c->flags &= ~E8080_FLAG_Z80;

	e8080_set_opcodes (c);
}

void e8080_set_z80 (e8080_t *c)
{
	c->flags |= E8080_FLAG_Z80;

	z80_set_opcodes (c);
}

unsigned e8080_get_flags (e8080_t *c)
{
	return (c->flags);
}

void e8080_set_flags (e8080_t *c, unsigned flags)
{
	c->flags = flags;
}

void e8080_set_mem_read_fct (e8080_t *c, void *ext, void *get8)
{
	c->mem_rd_ext = ext;
	c->get_uint8 = get8;
}

void e8080_set_mem_write_fct (e8080_t *c, void *ext, void *set8)
{
	c->mem_wr_ext = ext;
	c->set_uint8 = set8;
}

void e8080_set_mem_fct (e8080_t *c, void *ext, void *get8, void *set8)
{
	c->mem_rd_ext = ext;
	c->get_uint8 = get8;

	c->mem_wr_ext = ext;
	c->set_uint8 = set8;
}

void e8080_set_port_read_fct (e8080_t *c, void *ext, void *get8)
{
	c->mem_rd_ext = ext;
	c->get_port8 = get8;
}

void e8080_set_port_write_fct (e8080_t *c, void *ext, void *set8)
{
	c->port_wr_ext = ext;
	c->set_port8 = set8;
}

void e8080_set_port_fct (e8080_t *c, void *ext, void *get8, void *set8)
{
	c->port_rd_ext = ext;
	c->get_port8 = get8;

	c->port_wr_ext = ext;
	c->set_port8 = set8;
}

void e8080_set_hook_all_fct (e8080_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook_all = fct;
}

void e8080_set_hook_undef_fct (e8080_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook_undef = fct;
}

void e8080_set_hook_rst_fct (e8080_t *c, void *ext, void *fct)
{
	c->hook_ext = ext;
	c->hook_rst = fct;
}

void e8080_rst (e8080_t *c, unsigned val)
{
	e8080_set_clk (c, 0, 11);

	e8080_set_sp (c, e8080_get_sp (c) - 2);
	e8080_set_mem16 (c, e8080_get_sp (c), e8080_get_pc (c));
	e8080_set_pc (c, val);
}

void e8080_set_int (e8080_t *c, unsigned char val)
{
	c->int_val = (val != 0);

	if (c->int_val && c->iff) {
		c->int_req = 1;
	}

}

unsigned char e8080_get_port8 (e8080_t *c, unsigned addr)
{
	if (c->get_port8 != NULL) {
		return (c->get_port8 (c->port_rd_ext, addr));
	}

	return (0);
}

void e8080_set_port8 (e8080_t *c, unsigned addr, unsigned char val)
{
	if (c->set_port8 != NULL) {
		c->set_port8 (c->port_wr_ext, addr, val);
	}
}

int e8080_get_reg (e8080_t *c, const char *reg, unsigned long *val)
{
	if (*reg == '%') {
		reg += 1;
	}

	if (strcmp (reg, "a") == 0) {
		*val = e8080_get_a (c);
		return (0);
	}
	else if (strcmp (reg, "b") == 0) {
		*val = e8080_get_b (c);
		return (0);
	}
	else if (strcmp (reg, "c") == 0) {
		*val = e8080_get_c (c);
		return (0);
	}
	else if (strcmp (reg, "d") == 0) {
		*val = e8080_get_d (c);
		return (0);
	}
	else if (strcmp (reg, "e") == 0) {
		*val = e8080_get_e (c);
		return (0);
	}
	else if (strcmp (reg, "h") == 0) {
		*val = e8080_get_h (c);
		return (0);
	}
	else if (strcmp (reg, "l") == 0) {
		*val = e8080_get_l (c);
		return (0);
	}
	else if (strcmp (reg, "bc") == 0) {
		*val = e8080_get_bc (c);
		return (0);
	}
	else if (strcmp (reg, "de") == 0) {
		*val = e8080_get_de (c);
		return (0);
	}
	else if (strcmp (reg, "hl") == 0) {
		*val = e8080_get_hl (c);
		return (0);
	}
	else if (strcmp (reg, "ix") == 0) {
		*val = e8080_get_ix (c);
		return (0);
	}
	else if (strcmp (reg, "iy") == 0) {
		*val = e8080_get_iy (c);
		return (0);
	}
	else if (strcmp (reg, "psw") == 0) {
		*val = e8080_get_psw (c);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		*val = e8080_get_pc (c);
		return (0);
	}
	else if (strcmp (reg, "sp") == 0) {
		*val = e8080_get_sp (c);
		return (0);
	}
	else if (strcmp (reg, "i") == 0) {
		*val = e8080_get_i (c);
		return (0);
	}
	else if (strcmp (reg, "icnt") == 0) {
		*val = e8080_get_int_cnt (c);
		return (0);
	}
	else if (strcmp (reg, "iff") == 0) {
		*val = e8080_get_iff1 (c);
		return (0);
	}
	else if (strcmp (reg, "iff2") == 0) {
		*val = e8080_get_iff2 (c);
		return (0);
	}
	else if (strcmp (reg, "im") == 0) {
		*val = e8080_get_im (c);
		return (0);
	}
	else if (strcmp (reg, "ipc") == 0) {
		*val = e8080_get_int_pc (c);
		return (0);
	}
	else if (strcmp (reg, "r") == 0) {
		*val = e8080_get_r (c);
		return (0);
	}

	return (1);
}

int e8080_set_reg (e8080_t *c, const char *reg, unsigned long val)
{
	if (*reg == '%') {
		reg += 1;
	}

	if (strcmp (reg, "a") == 0) {
		e8080_set_a (c, val);
		return (0);
	}
	else if (strcmp (reg, "b") == 0) {
		e8080_set_b (c, val);
		return (0);
	}
	else if (strcmp (reg, "c") == 0) {
		e8080_set_c (c, val);
		return (0);
	}
	else if (strcmp (reg, "d") == 0) {
		e8080_set_d (c, val);
		return (0);
	}
	else if (strcmp (reg, "e") == 0) {
		e8080_set_e (c, val);
		return (0);
	}
	else if (strcmp (reg, "h") == 0) {
		e8080_set_h (c, val);
		return (0);
	}
	else if (strcmp (reg, "l") == 0) {
		e8080_set_l (c, val);
		return (0);
	}
	else if (strcmp (reg, "bc") == 0) {
		e8080_set_bc (c, val);
		return (0);
	}
	else if (strcmp (reg, "de") == 0) {
		e8080_set_de (c, val);
		return (0);
	}
	else if (strcmp (reg, "hl") == 0) {
		e8080_set_hl (c, val);
		return (0);
	}
	else if (strcmp (reg, "ix") == 0) {
		e8080_set_ix (c, val);
		return (0);
	}
	else if (strcmp (reg, "iy") == 0) {
		e8080_set_iy (c, val);
		return (0);
	}
	else if (strcmp (reg, "psw") == 0) {
		e8080_set_psw (c, val);
		return (0);
	}
	else if (strcmp (reg, "pc") == 0) {
		e8080_set_pc (c, val);
		return (0);
	}
	else if (strcmp (reg, "sp") == 0) {
		e8080_set_sp (c, val);
		return (0);
	}
	else if (strcmp (reg, "i") == 0) {
		e8080_set_i (c, val);
		return (0);
	}
	else if (strcmp (reg, "icnt") == 0) {
		e8080_set_int_cnt (c, val);
		return (0);
	}
	else if (strcmp (reg, "iff") == 0) {
		e8080_set_iff1 (c, val);
		return (0);
	}
	else if (strcmp (reg, "iff2") == 0) {
		e8080_set_iff2 (c, val);
		return (0);
	}
	else if (strcmp (reg, "im") == 0) {
		e8080_set_im (c, val);
		return (0);
	}
	else if (strcmp (reg, "ipc") == 0) {
		e8080_set_int_pc (c, val);
		return (0);
	}
	else if (strcmp (reg, "r") == 0) {
		e8080_set_r (c, val);
		return (0);
	}

	return (1);
}

unsigned long e8080_get_clock (e8080_t *c)
{
	return (c->clkcnt);
}

unsigned long e8080_get_opcnt (e8080_t *c)
{
	return (c->inscnt);
}

unsigned e8080_get_delay (e8080_t *c)
{
	return (c->delay);
}

int e8080_hook_all (e8080_t *c)
{
	if (c->hook_all != NULL) {
		return (c->hook_all (c->hook_ext, c->inst[0]));
	}

	return (0);
}

int e8080_hook_undefined (e8080_t *c)
{
	if (c->hook_undef != NULL) {
		return (c->hook_undef (c->hook_ext, c->inst[0]));
	}

	return (0);
}

int e8080_hook_rst (e8080_t *c)
{
	if (c->hook_rst != NULL) {
		return (c->hook_rst (c->hook_ext, c->inst[0]));
	}

	return (0);
}

void e8080_reset (e8080_t *c)
{
	unsigned i;

	c->delay = 7;

	for (i = 0; i < 8; i++) {
		c->reg[i] = 0;
		c->reg2[i] = 0;
	}

	e8080_set_psw (c, 0x02);
	e8080_set_ix (c, 0x0000);
	e8080_set_iy (c, 0x0000);
	e8080_set_sp (c, 0x0000);
	e8080_set_pc (c, 0x0000);
	e8080_set_i (c, 0x00);
	e8080_set_r (c, 0x00);

	c->psw2 = 0x02;

	c->iff = 0;
	c->iff2 = 0;

	c->int_req = 0;
	c->int_pc = 0;
	c->im = 0;

	c->halt = 0;
}

void e8080_interrupt (e8080_t *c)
{
	unsigned addr;

	e8080_inc_r (c);
	c->inscnt += 1;

	if (c->halt) {
		c->halt = 0;
		e8080_set_pc (c, e8080_get_pc (c) + 1);
	}

	c->int_cnt += 1;
	c->int_pc = e8080_get_pc (c);

	c->int_req = 0;

	if (c->im == 1) {
		e8080_rst (c, 0x38);

		c->iff = 0;
		c->iff2 = 0;
	}
	else if (c->im == 2) {
		addr = (e8080_get_i (c) << 8) | 0xff;
		addr = e8080_get_mem16 (c, addr);

		e8080_rst (c, addr);

		c->iff = 0;
		c->iff2 = 0;
	}
	else {
		fprintf (stderr, "8080: interrupt mode %u\n", c->im);
	}
}

void e8080_execute (e8080_t *c)
{
	unsigned short pc;
	unsigned char  iff;

	if (c->halt) {
		if (c->int_req && c->iff) {
			e8080_interrupt (c);
		}
		else {
			c->delay += 1;
		}
		return;
	}

	pc = e8080_get_pc (c);

	c->inst[0] = e8080_get_mem8 (c, pc);

	e8080_inc_r (c);

#ifdef E8080_ENABLE_HOOK_ALL
	if (c->hook_all != NULL) {
		if (e8080_hook_all (c)) {
			return;
		}
	}
#endif

	iff = c->iff;

	c->op[c->inst[0]] (c);

	c->inscnt += 1;

	if (c->int_req && iff && c->iff) {
		e8080_interrupt (c);
	}
}

void e8080_clock (e8080_t *c, unsigned n)
{
	while (n >= c->delay) {
		n -= c->delay;
		c->clkcnt += c->delay;
		c->delay = 0;
		e8080_execute (c);
	}

	c->delay -= n;
	c->clkcnt += n;
}
