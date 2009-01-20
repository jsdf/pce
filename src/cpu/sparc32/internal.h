/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/sparc32/internal.h                                   *
 * Created:     2004-09-27 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef SPARC32_INTERNAL_H
#define SPARC32_INTERNAL_H 1


#include <stdlib.h>
#include <stdio.h>


/*****************************************************************************
 * MMU
 *****************************************************************************/

int s32_ifetch (sparc32_t *c, uint32_t addr, uint8_t asi, uint32_t *val);

int s32_dload8 (sparc32_t *c, uint32_t addr, uint8_t asi, uint8_t *val);
int s32_dload16 (sparc32_t *c, uint32_t addr, uint8_t asi, uint16_t *val);
int s32_dload32 (sparc32_t *c, uint32_t addr, uint8_t asi, uint32_t *val);

int s32_dstore8 (sparc32_t *c, uint32_t addr, uint8_t asi, uint8_t val);
int s32_dstore16 (sparc32_t *c, uint32_t addr, uint8_t asi, uint16_t val);
int s32_dstore32 (sparc32_t *c, uint32_t addr, uint8_t asi, uint32_t val);


/*****************************************************************************
 * sparc32
 *****************************************************************************/

#define S32_ASI_UTEXT 0x08
#define S32_ASI_STEXT 0x09
#define S32_ASI_UDATA 0x0a
#define S32_ASI_SDATA 0x0b


#define s32_exts(x, n) ( \
	((x) & (1UL << ((n) - 1))) ? \
	(((x) | ~((1UL << (n)) - 1)) & 0xffffffffUL) : \
	((x) & ((1UL << (n)) - 1)) \
	)

#define s32_extu(x, n) ((x) & ((1 << (n)) - 1))

#define s32_ir_rd(ir) (((ir) >> 25) & 0x1f)
#define s32_ir_rs1(ir) (((ir) >> 14) & 0x1f)
#define s32_ir_rs2(ir) ((ir) & 0x1f)
#define s32_ir_i(ir) ((ir) & 0x00002000UL)
#define s32_ir_a(ir) ((ir) & 0x20000000UL)
#define s32_ir_asi(ir) (((ir) >> 5) & 0xff)
#define s32_ir_simm13(ir) (((ir) & 0x00001000UL) ? ((ir) | 0xfffff000UL) : ((ir) & 0x00000fffUL))
#define s32_ir_simm22(ir) (((ir) & 0x00200000UL) ? ((ir) | 0xffe00000UL) : ((ir) & 0x001fffffUL))
#define s32_ir_simm30(ir) (((ir) & 0x20000000UL) ? ((ir) | 0xe0000000UL) : ((ir) & 0x1fffffffUL))
#define s32_ir_uimm22(ir) ((ir) & 0x003fffffUL)

#define s32_get_rd(c, ir) (s32_get_gpr ((c), s32_ir_rd(ir)))
#define s32_get_rs1(c, ir) (s32_get_gpr ((c), s32_ir_rs1(ir)))
#define s32_get_rs2(c, ir) (s32_get_gpr ((c), s32_ir_rs2(ir)))
#define s32_set_rd(c, ir, v) s32_set_gpr ((c), s32_ir_rd(ir), (v))
#define s32_get_rs2_or_simm13(c, ir) ( \
	s32_ir_i(ir) ? s32_ir_simm13(ir) : s32_get_gpr((c), s32_ir_rs2(ir)))
#define s32_get_addr(c, ir) \
	((s32_get_rs1((c), (ir)) + s32_get_rs2_or_simm13((c), (ir))) & 0xffffffffUL)

#define s32_set_clk(c, dpc, clk) do { \
	(c)->pc = (c)->npc; \
	(c)->npc += (dpc); \
	(c)->delay += (clk); \
} while (0)


void s32_regstk_load (sparc32_t *c, unsigned wdw);
void s32_regstk_save (sparc32_t *c, unsigned wdw);

void s32_set_opcodes (sparc32_t *c);


#endif
