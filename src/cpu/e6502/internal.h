/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e6502/internal.h                                     *
 * Created:     2004-05-23 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_E6502_INTERNAL_H
#define PCE_E6502_INTERNAL_H 1


#include "e6502.h"


#define e6502_set_clk(cpu, cnt, clk) do { \
	(cpu)->pc = ((cpu)->pc + (cnt)) & 0xffff; \
	(cpu)->delay += (clk); \
	} while (0)

#define e6502_get_inst1(c) do { \
	(c)->inst[1] = e6502_get_mem8 (c, (c->pc + 1) & 0xffff); \
	} while (0)

#define e6502_get_inst2(c) do { \
	(c)->inst[1] = e6502_get_mem8 (c, (c->pc + 1) & 0xffff); \
	(c)->inst[2] = e6502_get_mem8 (c, (c->pc + 2) & 0xffff); \
	} while (0)

#define e6502_mk_uint16(lo, hi) \
	((((hi) & 0xff) << 8) + ((lo) & 0xff))

#define e6502_mk_sint16(lo) \
	(((lo) & 0x80) ? ((lo) | 0xff00) : ((lo) & 0xff))


#define e6502_get_idx_ind_x(c) e6502_get_mem8 (c, e6502_get_ea_idx_ind_x (c))
#define e6502_get_zpg(c) e6502_get_mem8 (c, e6502_get_ea_zpg (c))
#define e6502_get_abs(c) e6502_get_mem8 (c, e6502_get_ea_abs (c))
#define e6502_get_ind_idx_y(c) e6502_get_mem8 (c, e6502_get_ea_ind_idx_y (c))
#define e6502_get_zpg_x(c) e6502_get_mem8 (c, e6502_get_ea_zpg_x (c))
#define e6502_get_zpg_y(c) e6502_get_mem8 (c, e6502_get_ea_zpg_y (c))
#define e6502_get_abs_y(c) e6502_get_mem8 (c, e6502_get_ea_abs_y (c))
#define e6502_get_abs_x(c) e6502_get_mem8 (c, e6502_get_ea_abs_x (c))
#define e6502_set_ea(c, v) e6502_set_mem8 ((c), (c)->ea, (v))


int e6502_hook_all (e6502_t *c);
int e6502_hook_undefined (e6502_t *c);
int e6502_hook_brk (e6502_t *c);


unsigned char e6502_get_imm (e6502_t *c);
unsigned short e6502_get_ea_idx_ind_x (e6502_t *c);
unsigned short e6502_get_ea_zpg (e6502_t *c);
unsigned short e6502_get_ea_abs (e6502_t *c);
unsigned short e6502_get_ea_ind_idx_y (e6502_t *c);
unsigned short e6502_get_ea_zpg_x (e6502_t *c);
unsigned short e6502_get_ea_zpg_y (e6502_t *c);
unsigned short e6502_get_ea_abs_y (e6502_t *c);
unsigned short e6502_get_ea_abs_x (e6502_t *c);


extern e6502_opcode_f e6502_opcodes[256];

unsigned char e6502_pull (e6502_t *c);
unsigned short e6502_pull16 (e6502_t *c);
void e6502_push (e6502_t *c, unsigned char val);
void e6502_push16 (e6502_t *c, unsigned short val);
void e6502_trap (e6502_t *c, unsigned short addr);


#endif
