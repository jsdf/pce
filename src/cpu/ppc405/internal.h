/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/ppc405/internal.h                                  *
 * Created:       2003-11-07 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2006 Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2006 Lukas Ruf <ruf@lpr.ch>                       *
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

/* $Id$ */


#ifndef PPC405_INTERNAL_H
#define PPC405_INTERNAL_H


#include <stdlib.h>
#include <stdio.h>


/*****************************************************************************
 * MMU
 *****************************************************************************/

void p405_tlb_init (p405_tlb_t *tlb);

void p405_tbuf_clear (p405_t *c);

void p405_set_tlb_entry_hi (p405_t *c, unsigned idx, uint32_t tlbhi, uint8_t pid);
void p405_set_tlb_entry_lo (p405_t *c, unsigned idx, uint32_t tlblo);

uint32_t p405_get_tlb_entry_hi (p405_t *c, unsigned idx);
uint32_t p405_get_tlb_entry_lo (p405_t *c, unsigned idx);
uint32_t p405_get_tlb_entry_tid (p405_t *c, unsigned idx);

void p405_tlb_invalidate_all (p405_t *c);

int p405_ifetch (p405_t *c, uint32_t addr, uint32_t *val);

int p405_dload8 (p405_t *c, uint32_t addr, uint8_t *val);
int p405_dload16 (p405_t *c, uint32_t addr, uint16_t *val);
int p405_dload32 (p405_t *c, uint32_t addr, uint32_t *val);

int p405_dstore8 (p405_t *c, uint32_t addr, uint8_t val);
int p405_dstore16 (p405_t *c, uint32_t addr, uint16_t val);
int p405_dstore32 (p405_t *c, uint32_t addr, uint32_t val);


/*****************************************************************************
 * PPC
 *****************************************************************************/

#define P405_PVR_405GP   0x40110000UL
#define P405_PVR_NPE405H 0x41410140UL
#define P405_PVR_NPE405L 0x416100c0UL

#define P405_CACHE_LINE_SIZE 32

#define P405_IR_OE 0x0400
#define P405_IR_RC 0x0001
#define P405_IR_LK 0x0001
#define P405_IR_AA 0x0002

#define P405_EXCPT_MSR (P405_MSR_WE | P405_MSR_EE | P405_MSR_PR | P405_MSR_DWE \
	| P405_MSR_IR | P405_MSR_DR)

#define P405_EA_IDX 0x01
#define P405_EA_UPD 0x02

#define p405_sext(x, n) ( \
	((x) & (1UL << ((n) - 1))) ? \
	(((x) | ~((1UL << (n)) - 1)) & 0xffffffffUL) : \
	((x) & ((1UL << (n)) - 1)) \
	)

#define p405_uext(x, n) ((x) & ((1 << (n)) - 1))

#define p405_br16(x) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#define p405_br32(x) ((((x) & 0xff) << 24) | ((((x) >> 8) & 0xff) << 16) \
 | ((((x) >> 16) & 0xff) << 8) | (((x) >> 24) & 0xff))

#define p405_bits(val, i, n) (((val) >> (32 - (i) - (n))) & ((1UL << (n)) - 1))

#define p405_get_ir_op2(ir) (((ir) >> 1) & 0x3ff)

#define p405_get_ir_rc(ir) (((ir) & P405_IR_RC) != 0)
#define p405_get_ir_oe(ir) (((ir) & P405_IR_OE) != 0)
#define p405_get_ir_lk(ir) (((ir) & P405_IR_LK) != 0)
#define p405_get_ir_aa(ir) (((ir) & P405_IR_AA) != 0)

#define p405_get_ir_ra(ir) (((ir) >> 16) & 0x1f)
#define p405_get_ir_rb(ir) (((ir) >> 11) & 0x1f)
#define p405_get_ir_rs(ir) (((ir) >> 21) & 0x1f)
#define p405_get_ir_rt(ir) (((ir) >> 21) & 0x1f)

#define p405_get_ra(c, ir) ((c)->gpr[p405_get_ir_ra (ir)])
#define p405_get_rb(c, ir) ((c)->gpr[p405_get_ir_rb (ir)])
#define p405_get_rs(c, ir) ((c)->gpr[p405_get_ir_rs (ir)])
#define p405_get_rt(c, ir) ((c)->gpr[p405_get_ir_rt (ir)])
#define p405_get_ra0(c, ir) ((p405_get_ir_ra (ir) != 0) ? p405_get_ra (c, ir) : 0)

#define p405_set_ra(c, ir, val) do { (c)->gpr[p405_get_ir_ra (ir)] = (val); } while (0)
#define p405_set_rb(c, ir, val) do { (c)->gpr[p405_get_ir_rb (ir)] = (val); } while (0)
#define p405_set_rs(c, ir, val) do { (c)->gpr[p405_get_ir_rs (ir)] = (val); } while (0)
#define p405_set_rt(c, ir, val) do { (c)->gpr[p405_get_ir_rt (ir)] = (val); } while (0)

#define p405_set_clk(c, dpc, clk) do { (c)->pc += (dpc); (c)->delay += (clk); } while (0)


int p405_check_reserved (p405_t *c, uint32_t res);
int p405_check_privilege (p405_t *c);
void p405_set_xer_so_ov (p405_t *c, uint64_t r1, uint32_t r2);
void p405_set_xer_oflow (p405_t *c, int of);
void p405_set_cr0 (p405_t *c, uint32_t r);
int p405_get_ea (p405_t *c, uint32_t *val, unsigned flags);
uint64_t p405_mul (uint32_t s1, uint32_t s2);

void p405_op_add (p405_t *c, uint32_t s1, uint32_t s2, int oe, int rc, int co, int ci);
void p405_op_and (p405_t *c, uint32_t s1, uint32_t s2, int rc);
void p405_op_branch (p405_t *c, uint32_t dst, unsigned bo, unsigned bi, int aa, int lk);
void p405_op_cmpl (p405_t *c, uint32_t s1, uint32_t s2, unsigned fld);
void p405_op_crop (p405_t *c, unsigned bt, unsigned ba, unsigned bb, unsigned booltab);
void p405_op_lsw (p405_t *c, unsigned rt, unsigned ra, unsigned rb, uint32_t ea, unsigned cnt);
void p405_op_stsw (p405_t *c, unsigned rs, uint32_t ea, unsigned cnt);
void p405_op_sub (p405_t *c, uint32_t s1, uint32_t s2, int oe, int rc, int co, int ci);
void p405_op_undefined (p405_t *c);

void p405_exception_data_store (p405_t *c, uint32_t ea, int store, int zone);
void p405_exception_instr_store (p405_t *c, int zone);
void p405_exception_external (p405_t *c);
void p405_exception_program (p405_t *c, uint32_t esr);
void p405_exception_program_fpu (p405_t *c);
void p405_exception_syscall (p405_t *c);
void p405_exception_pit (p405_t *c);
void p405_exception_tlb_miss_data (p405_t *c, uint32_t ea, int store);
void p405_exception_tlb_miss_instr (p405_t *c);

void p405_set_opcode13 (p405_t *c);
void p405_set_opcode1f (p405_t *c);
void p405_set_opcodes (p405_t *c);


#endif
