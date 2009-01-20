/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/arm/internal.h                                       *
 * Created:     2004-11-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


#ifndef ARM_INTERNAL_H
#define ARM_INTERNAL_H


#include <stdlib.h>
#include <stdio.h>


/*****************************************************************************
 * MMU
 *****************************************************************************/

int arm_ifetch (arm_t *c, uint32_t addr, uint32_t *val);

int arm_dload8 (arm_t *c, uint32_t addr, uint8_t *val);
int arm_dload16 (arm_t *c, uint32_t addr, uint16_t *val);
int arm_dload32 (arm_t *c, uint32_t addr, uint32_t *val);

int arm_dstore8 (arm_t *c, uint32_t addr, uint8_t val);
int arm_dstore16 (arm_t *c, uint32_t addr, uint16_t val);
int arm_dstore32 (arm_t *c, uint32_t addr, uint32_t val);

int arm_dload8_t (arm_t *c, uint32_t addr, uint8_t *val);
int arm_dload16_t (arm_t *c, uint32_t addr, uint16_t *val);
int arm_dload32_t (arm_t *c, uint32_t addr, uint32_t *val);

int arm_dstore8_t (arm_t *c, uint32_t addr, uint8_t val);
int arm_dstore16_t (arm_t *c, uint32_t addr, uint16_t val);
int arm_dstore32_t (arm_t *c, uint32_t addr, uint32_t val);


/*****************************************************************************
 * arm
 *****************************************************************************/

#define arm_get_bit(v, i) (((v) >> (i)) & 1)
#define arm_get_bits(v, i, n) (((v) >> (i)) & ((1UL << (n)) - 1))

#define arm_exts(x, n) ( \
	((x) & (1UL << ((n) - 1))) ? \
	(((x) | ~((1UL << (n)) - 1)) & 0xffffffffUL) : \
	((x) & ((1UL << (n)) - 1)) \
	)

#define arm_extu(x, n) ((x) & ((1UL << (n)) - 1))

#define arm_ir_cond(ir) (((ir) >> 28) & 0x0f)
#define arm_ir_rn(ir) (((ir) >> 16) & 0x0f)
#define arm_ir_rd(ir) (((ir) >> 12) & 0x0f)
#define arm_ir_rs(ir) (((ir) >> 8) & 0x0f)
#define arm_ir_rm(ir) ((ir) & 0x0f)
#define arm_ir_s(ir) (((ir) >> 20) & 0x01)
#define arm_ir_i(ir) (((ir) >> 25) & 0x01)

#define arm_rd_is_pc(ir) ((((ir) >> 12) & 0x0f) == 15)

#define arm_is_shext(ir) (((ir) & 0x02000090UL) == 0x00000090UL)
#define arm_get_shext(ir) (((ir) >> 4) & 0x0f)

static inline
uint32_t arm_get_reg_pc (arm_t *c, unsigned reg, uint32_t pc)
{
	if (reg == 15) {
		return ((arm_get_gpr (c, 15) + pc) & 0xffffffff);
	}

	return (arm_get_gpr (c, reg));
}

#define arm_get_rn(c, ir) (arm_get_reg_pc ((c), arm_ir_rn(ir), 8))
#define arm_get_rd(c, ir) (arm_get_reg_pc ((c), arm_ir_rd(ir), 8))
#define arm_get_rs(c, ir) (arm_get_reg_pc ((c), arm_ir_rs(ir), 8))
#define arm_get_rm(c, ir) (arm_get_reg_pc ((c), arm_ir_rm(ir), 8))

#define arm_set_rn(c, ir, v) do { arm_set_gpr ((c), arm_ir_rn(ir), (v)); } while (0)
#define arm_set_rd(c, ir, v) do { arm_set_gpr ((c), arm_ir_rd(ir), (v)); } while (0)

#define arm_set_clk(c, dpc, clk) do { \
	(c)->reg[15] += (dpc); \
	(c)->delay += (clk); \
} while (0)


#define arm_check_cond_al(ir) (((ir) & 0xf0000000UL) == 0xe0000000UL)

#define arm_is_privileged(c) ((c)->privileged)


static inline
uint32_t arm_ror32 (uint32_t v, unsigned n)
{
	n &= 31;

	if (n > 0) {
		v = ((v >> n) | (v << (32 - n))) & 0xffffffff;
	}

	return (v);
}

static inline
uint32_t arm_asr32 (uint32_t v, unsigned n)
{
	n &= 31;

	if (n > 0) {
		if (v & 0x80000000) {
			v = ((v >> n) | (0xffffffff << (32 - n))) & 0xffffffff;
		}
		else {
			v = v >> n;
		}
	}

	return (v);
}

static inline
void arm_tbuf_flush (arm_t *c)
{
	arm_copr15_t *mmu = arm_get_mmu (c);

	mmu->tbuf_exec.valid = 0;
	mmu->tbuf_read.valid = 0;
	mmu->tbuf_write.valid = 0;
}


int arm_write_cpsr (arm_t *c, uint32_t val, int prvchk);

int arm_check_cond (arm_t *c, unsigned cond);

void arm_set_opcodes (arm_t *c);


#endif
