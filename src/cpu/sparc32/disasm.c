/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/sparc32/disasm.c                                     *
 * Created:     2004-09-28 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sparc32.h"
#include "internal.h"


static s32_dasm_f s32_dasm_op[4][64];


enum {
	ARG_NONE,

	ARG_IR,

	ARG_RD,
	ARG_RD_FP,
	ARG_RD_CP,
	ARG_RD_ASR,
	ARG_RS1,
	ARG_RS1_ASR,
	ARG_RS2,
	ARG_RS2_OR_SIMM13,

	ARG_SIMM13,
	ARG_UIMM22,
	ARG_SIMM30,
	ARG_CONST22,

	ARG_ADDR_DIRECT,
	ARG_ADDR_INDIRECT,

	ARG_TRAP,
	ARG_ASI,

	ARG_REG_PSR,
	ARG_REG_TBR,
	ARG_REG_WIM,
	ARG_REG_Y,
	ARG_REG_FSR,
	ARG_REG_CSR
};


static const char *s32_icc_names[16] = {
	"n", "e",  "le", "l",  "leu", "cs", "neg", "vs",
	"a", "ne", "g",  "ge", "gu",  "cc", "pos", "vc"
};

static const char *s32_fcc_names[16] = {
	"n", "ne", "lg", "ul", "l",   "ug", "g",   "u",
	"a", "e",  "ue", "ge", "uge", "le", "ule", "o"
};


static
void dasm_arg (char *dst, uint32_t ir, unsigned arg)
{
	unsigned      rd, rs1, rs2;
	unsigned long simm13;

	rd = s32_ir_rd (ir);
	rs1 = s32_ir_rs1 (ir);
	rs2 = s32_ir_rs2 (ir);

	simm13 = s32_ir_simm13 (ir);

	switch (arg) {
	case ARG_NONE:
		dst[0] = 0;
		break;

	case ARG_IR:
		sprintf (dst, "0x%08lx", (unsigned long) ir);
		break;

	case ARG_RD:
		sprintf (dst, "%%r%u", rd);
		break;

	case ARG_RD_FP:
		sprintf (dst, "%%f%u", rd);
		break;

	case ARG_RD_CP:
		sprintf (dst, "%%c%u", rd);
		break;

	case ARG_RD_ASR:
		sprintf (dst, "%%asr%u", rd);
		break;

	case ARG_RS1:
		sprintf (dst, "%%r%u", rs1);
		break;

	case ARG_RS1_ASR:
		sprintf (dst, "%%asr%u", rs1);
		break;

	case ARG_RS2:
		sprintf (dst, "%%r%u", rs2);
		break;

	case ARG_RS2_OR_SIMM13:
		if (s32_ir_i (ir)) {
			sprintf (dst, "0x%08lx", simm13);
		}
		else {
			sprintf (dst, "%%r%u", rs2);
		}
		break;

	case ARG_SIMM13:
		sprintf (dst, "0x%08lx", simm13);
		break;

	case ARG_UIMM22:
		sprintf (dst, "%%hi(0x%08lx)", (unsigned long) (s32_ir_uimm22 (ir) << 10));
		break;

	case ARG_CONST22:
		sprintf (dst, "%%hi(0x%08lx)", s32_ir_uimm22 (ir));
		break;

	case ARG_SIMM30:
		sprintf (dst, "0x%08lx", (unsigned long) (s32_ir_simm30 (ir) << 2));
		break;

	case ARG_ADDR_DIRECT:
		if (s32_ir_i (ir)) {
			if (simm13 == 0) {
				sprintf (dst, "%%r%u", rs1);
			}
			else if (rs1 == 0) {
				sprintf (dst, "0x%08lx", simm13);
			}
			else {
				sprintf (dst, "%%r%u + 0x%08lx", rs1, simm13);
			}
		}
		else {
			if (rs2 == 0) {
				sprintf (dst, "%%r%u", rs1);
			}
			else if (rs1 == 0) {
				sprintf (dst, "%%r%u", rs2);
			}
			else {
				sprintf (dst, "%%r%u + %%r%u", rs1, rs2);
			}
		}
		break;

	case ARG_ADDR_INDIRECT:
		if (s32_ir_i (ir)) {
			if (simm13 == 0) {
				sprintf (dst, "[%%r%u]", rs1);
			}
			else if (rs1 == 0) {
				sprintf (dst, "[0x%08lx]", simm13);
			}
			else {
				sprintf (dst, "[%%r%u + 0x%08lx]", rs1, simm13);
			}
		}
		else {
			if (rs2 == 0) {
				sprintf (dst, "[%%r%u]", rs1);
			}
			else if (rs1 == 0) {
				sprintf (dst, "[%%r%u]", rs2);
			}
			else {
				sprintf (dst, "[%%r%u + %%r%u]", rs1, rs2);
			}
		}
		break;

	case ARG_TRAP:
		if (s32_ir_i (ir)) {
			if (simm13 == 0) {
				sprintf (dst, "%%r%u", rs1);
			}
			else if (rs1 == 0) {
				sprintf (dst, "0x%02lx", simm13 & 0x7f);
			}
			else {
				sprintf (dst, "%%r%u + 0x%02lx", rs1, simm13 & 0x7f);
			}
		}
		else {
			if (rs2 == 0) {
				sprintf (dst, "%%r%u", rs1);
			}
			else if (rs1 == 0) {
				sprintf (dst, "%%r%u", rs2);
			}
			else {
				sprintf (dst, "%%r%u + %%r%u", rs1, rs2);
			}
		}
		break;

	case ARG_ASI:
		sprintf (dst, "0x%02x", (unsigned) s32_ir_asi (ir));
		break;

	case ARG_REG_PSR:
		strcpy (dst, "%psr");
		break;

	case ARG_REG_TBR:
		strcpy (dst, "%tbr");
		break;

	case ARG_REG_WIM:
		strcpy (dst, "%wim");
		break;

	case ARG_REG_Y:
		strcpy (dst, "%y");
		break;

	case ARG_REG_FSR:
		strcpy (dst, "%fsr");
		break;

	case ARG_REG_CSR:
		strcpy (dst, "%csr");
		break;
	}
}

static
int dasm_op0 (s32_dasm_t *da, const char *op)
{
	strcpy (da->op, op);

	da->argn = 0;

	return (0);
}

static
int dasm_op1 (s32_dasm_t *da, const char *op, unsigned arg1)
{
	strcpy (da->op, op);

	da->argn = 1;
	dasm_arg (da->arg1, da->ir, arg1);

	return (0);
}

static
int dasm_op2 (s32_dasm_t *da, const char *op, unsigned arg1, unsigned arg2)
{
	strcpy (da->op, op);

	da->argn = 2;
	dasm_arg (da->arg1, da->ir, arg1);
	dasm_arg (da->arg2, da->ir, arg2);

	return (0);
}

static
int dasm_op3 (s32_dasm_t *da, const char *op, unsigned arg1, unsigned arg2, unsigned arg3)
{
	strcpy (da->op, op);

	da->argn = 3;
	dasm_arg (da->arg1, da->ir, arg1);
	dasm_arg (da->arg2, da->ir, arg2);
	dasm_arg (da->arg3, da->ir, arg3);

	return (0);
}


static void opd_ud (s32_dasm_t *da)
{
	dasm_op1 (da, "dw", ARG_IR);
}

/* 0 00: unimp uimm22 */
static void opd000 (s32_dasm_t *da)
{
	dasm_op1 (da, "unimp", ARG_CONST22);
}

/* 0 02: bicc[,a] simm22 */
static void opd002 (s32_dasm_t *da)
{
	strcpy (da->op, "b");
	strcat (da->op, s32_icc_names[(da->ir >> 25) & 0x0f]);

	if (s32_ir_a (da->ir)) {
		strcat (da->op, ",a");
	}

	da->argn = 1;
	dasm_arg (da->arg1, da->pc + (s32_ir_simm22 (da->ir) << 2), ARG_IR);
}

/* 0 04: sethi rd, uimm22 */
static void opd004 (s32_dasm_t *da)
{
	if ((s32_ir_rd (da->ir) == 0) && (s32_ir_uimm22 (da->ir) == 0)) {
		dasm_op0 (da, "nop");
	}
	else {
		dasm_op2 (da, "sethi", ARG_UIMM22, ARG_RD);
	}
}

/* 0 06: fbcc[,a] simm22 */
static void opd006 (s32_dasm_t *da)
{
	strcpy (da->op, "fb");
	strcat (da->op, s32_fcc_names[(da->ir >> 25) & 0x0f]);

	if (s32_ir_a (da->ir)) {
		strcat (da->op, ",a");
	}

	da->argn = 1;
	dasm_arg (da->arg1, da->pc + (s32_ir_simm22 (da->ir) << 2), ARG_IR);
}

/* 0 07: cbcc[,a] simm22 */
static void opd007 (s32_dasm_t *da)
{
	strcpy (da->op, "cbcc");
/*  strcat (da->op, s32_ccc_names[(da->ir >> 25) & 0x0f]); */

	if (s32_ir_a (da->ir)) {
		strcat (da->op, ",a");
	}

	da->argn = 1;
	dasm_arg (da->arg1, da->pc + (s32_ir_simm22 (da->ir) << 2), ARG_IR);
}

/* 1 00: call disp30 */
static void opd100 (s32_dasm_t *da)
{
	dasm_op0 (da, "call");
	da->argn = 1;
	dasm_arg (da->arg1, da->pc + (da->ir << 2), ARG_IR);
	da->flags |= S32_DFLAG_CALL;
}

/* 2 00: add rs1, reg_or_imm, rd */
static void opd200 (s32_dasm_t *da)
{
	dasm_op3 (da, "add", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 01: and rs1, reg_or_imm, rd */
static void opd201 (s32_dasm_t *da)
{
	dasm_op3 (da, "and", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 02: or rs1, reg_or_imm, rd */
static void opd202 (s32_dasm_t *da)
{
	if (s32_ir_rs1 (da->ir) == 0) {
		if ((s32_ir_i (da->ir) == 0) && (s32_ir_rs2 (da->ir) == 0)) {
			dasm_op1 (da, "clr", ARG_RD);
		}
		else {
			dasm_op2 (da, "mov", ARG_RS2_OR_SIMM13, ARG_RD);
		}
	}
	else {
		dasm_op3 (da, "or", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
	}
}

/* 2 03: xor rs1, reg_or_imm, rd */
static void opd203 (s32_dasm_t *da)
{
	dasm_op3 (da, "xor", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 04: sub rs1, reg_or_imm, rd */
static void opd204 (s32_dasm_t *da)
{
	if ((s32_ir_rs1 (da->ir) == 0) && (s32_ir_i (da->ir) == 0)) {
		if (s32_ir_rs2 (da->ir) == s32_ir_rd (da->ir)) {
			dasm_op1 (da, "neg", ARG_RD);
		}
		else {
			dasm_op2 (da, "neg", ARG_RS2_OR_SIMM13, ARG_RD);
		}
	}
	else {
		dasm_op3 (da, "sub", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
	}
}

/* 2 05: andn rs1, reg_or_imm, rd */
static void opd205 (s32_dasm_t *da)
{
	dasm_op3 (da, "andn", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 06: orn rs1, reg_or_imm, rd */
static void opd206 (s32_dasm_t *da)
{
	dasm_op3 (da, "orn", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 07: xnor rs1, reg_or_imm, rd */
static void opd207 (s32_dasm_t *da)
{
	if ((s32_ir_i (da->ir) == 0) && (s32_ir_rs2 (da->ir) == 0)) {
		if (s32_ir_rs1 (da->ir) == s32_ir_rd (da->ir)) {
			dasm_op1 (da, "not", ARG_RD);
		}
		else {
			dasm_op2 (da, "not", ARG_RS1, ARG_RD);
		}
	}
	else {
		dasm_op3 (da, "xnor", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
	}
}

/* 2 08: addx rs1, reg_or_imm, rd */
static void opd208 (s32_dasm_t *da)
{
	dasm_op3 (da, "addx", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 0A: umul rs1, reg_or_imm, rd */
static void opd20a (s32_dasm_t *da)
{
	dasm_op3 (da, "umul", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 0B: smul rs1, reg_or_imm, rd */
static void opd20b (s32_dasm_t *da)
{
	dasm_op3 (da, "smul", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 0C: subx rs1, reg_or_imm, rd */
static void opd20c (s32_dasm_t *da)
{
	dasm_op3 (da, "subx", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 0E: udiv rs1, reg_or_imm, rd */
static void opd20e (s32_dasm_t *da)
{
	dasm_op3 (da, "udiv", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 0F: sdiv rs1, reg_or_imm, rd */
static void opd20f (s32_dasm_t *da)
{
	dasm_op3 (da, "sdiv", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 10: addcc rs1, reg_or_imm, rd */
static void opd210 (s32_dasm_t *da)
{
	dasm_op3 (da, "addcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 11: andcc rs1, reg_or_imm, rd */
static void opd211 (s32_dasm_t *da)
{
	dasm_op3 (da, "andcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 12: orcc rs1, reg_or_imm, rd */
static void opd212 (s32_dasm_t *da)
{
	unsigned rd, rs1;

	rd = s32_ir_rd (da->ir);
	rs1 = s32_ir_rs1 (da->ir);

	if ((rs1 == 0) && (rd == 0) && (s32_ir_i (da->ir) == 0)) {
		dasm_op1 (da, "tst", ARG_RS2_OR_SIMM13);
	}
	else {
		dasm_op3 (da, "orcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
	}
}

/* 2 13: xorcc rs1, reg_or_imm, rd */
static void opd213 (s32_dasm_t *da)
{
	dasm_op3 (da, "xorcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 14: subcc rs1, reg_or_imm, rd */
static void opd214 (s32_dasm_t *da)
{
	if (s32_ir_rd (da->ir) == 0) {
		dasm_op2 (da, "cmp", ARG_RS1, ARG_RS2_OR_SIMM13);
	}
	else {
		dasm_op3 (da, "subcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
	}
}

/* 2 15: andncc rs1, reg_or_imm, rd */
static void opd215 (s32_dasm_t *da)
{
	dasm_op3 (da, "andncc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 16: orncc rs1, reg_or_imm, rd */
static void opd216 (s32_dasm_t *da)
{
	dasm_op3 (da, "orncc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 17: xnorcc rs1, reg_or_imm, rd */
static void opd217 (s32_dasm_t *da)
{
	dasm_op3 (da, "xnorcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 18: addxcc rs1, reg_or_imm, rd */
static void opd218 (s32_dasm_t *da)
{
	dasm_op3 (da, "addxcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 1A: umulcc rs1, reg_or_imm, rd */
static void opd21a (s32_dasm_t *da)
{
	dasm_op3 (da, "umulcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 1B: smulcc rs1, reg_or_imm, rd */
static void opd21b (s32_dasm_t *da)
{
	dasm_op3 (da, "smulcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 1C: subxcc rs1, reg_or_imm, rd */
static void opd21c (s32_dasm_t *da)
{
	dasm_op3 (da, "subxcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 1E: udivcc rs1, reg_or_imm, rd */
static void opd21e (s32_dasm_t *da)
{
	dasm_op3 (da, "udivcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 1F: sdivcc rs1, reg_or_imm, rd */
static void opd21f (s32_dasm_t *da)
{
	dasm_op3 (da, "sdivcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 20: taddcc rs1, reg_or_imm, rd */
static void opd220 (s32_dasm_t *da)
{
	dasm_op3 (da, "taddcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 21: tsubcc rs1, reg_or_imm, rd */
static void opd221 (s32_dasm_t *da)
{
	dasm_op3 (da, "tsubcc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 22: taddcctv rs1, reg_or_imm, rd */
static void opd222 (s32_dasm_t *da)
{
	dasm_op3 (da, "taddcctv", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 23: tsubcctv rs1, reg_or_imm, rd */
static void opd223 (s32_dasm_t *da)
{
	dasm_op3 (da, "tsubcctv", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 24: mulscc rs1, reg_or_imm, rd */
static void opd224 (s32_dasm_t *da)
{
	dasm_op3 (da, "mulscc", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 25: sll rs1, reg_or_imm, rd */
static void opd225 (s32_dasm_t *da)
{
	dasm_op3 (da, "sll", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 26: srl rs1, reg_or_imm, rd */
static void opd226 (s32_dasm_t *da)
{
	dasm_op3 (da, "srl", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 27: sra rs1, reg_or_imm, rd */
static void opd227 (s32_dasm_t *da)
{
	dasm_op3 (da, "sra", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
}

/* 2 28: rd %asr, rd */
static void opd228 (s32_dasm_t *da)
{
	if (s32_ir_rs1 (da->ir) == 0) {
		dasm_op2 (da, "rd", ARG_REG_Y, ARG_RD);
	}
	else {
		dasm_op2 (da, "rd", ARG_RS1_ASR, ARG_RD);
	}
}

/* 2 29: rd %psr, rd */
static void opd229 (s32_dasm_t *da)
{
	dasm_op2 (da, "rd", ARG_REG_PSR, ARG_RD);
}

/* 2 2A: rd %wim, rd */
static void opd22a (s32_dasm_t *da)
{
	dasm_op2 (da, "rd", ARG_REG_WIM, ARG_RD);
}

/* 2 2B: rd %tbr, rd */
static void opd22b (s32_dasm_t *da)
{
	dasm_op2 (da, "rd", ARG_REG_TBR, ARG_RD);
}

/* 2 30: wr rs1, reg_or_imm, %asr */
static void opd230 (s32_dasm_t *da)
{
	if (s32_ir_rd (da->ir) == 0) {
		dasm_op3 (da, "wr", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_REG_Y);
	}
	else {
		dasm_op3 (da, "wr", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD_ASR);
	}
}

/* 2 31: wr rs1, reg_or_imm, %psr */
static void opd231 (s32_dasm_t *da)
{
	dasm_op3 (da, "wr", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_REG_PSR);
}

/* 2 32: wr rs1, reg_or_imm, %wim */
static void opd232 (s32_dasm_t *da)
{
	dasm_op3 (da, "wr", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_REG_WIM);
}

/* 2 33: wr rs1, reg_or_imm, %tbr */
static void opd233 (s32_dasm_t *da)
{
	dasm_op3 (da, "wr", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_REG_TBR);
}

/* 2 34: fpop1 rs1, rs2, rd */
static void opd234 (s32_dasm_t *da)
{
	dasm_op3 (da, "fpop1", ARG_RS1, ARG_RS2, ARG_RD);
}

/* 2 35: fpop2 rs1, rs2, rd */
static void opd235 (s32_dasm_t *da)
{
	dasm_op3 (da, "fpop2", ARG_RS1, ARG_RS2, ARG_RD);
}

/* 2 36: cpop1 rs1, rs2, rd */
static void opd236 (s32_dasm_t *da)
{
	dasm_op3 (da, "cpop1", ARG_RS1, ARG_RS2, ARG_RD);
}

/* 2 37: cpop2 rs1, rs2, rd */
static void opd237 (s32_dasm_t *da)
{
	dasm_op3 (da, "cpop2", ARG_RS1, ARG_RS2, ARG_RD);
}

/* 2 38: jmpl addr, rd */
static void opd238 (s32_dasm_t *da)
{
	unsigned rd, rs1, i;
	uint32_t simm13;

	rd = s32_ir_rd (da->ir);
	rs1 = s32_ir_rs1 (da->ir);
	i = s32_ir_i (da->ir);
	simm13 = s32_ir_simm13 (da->ir);

	if (rd == 0) {
		if ((rs1 == 31) && (i != 0) && (simm13 == 8)) {
			dasm_op0 (da, "ret");
		}
		else if ((rs1 == 15) && (i != 0) && (simm13 == 8)) {
			dasm_op0 (da, "retl");
		}
		else {
			dasm_op1 (da, "jmp", ARG_ADDR_DIRECT);
		}
	}
	else if (rd == 15) {
		dasm_op1 (da, "call", ARG_ADDR_DIRECT);
		da->flags |= S32_DFLAG_CALL;
	}
	else {
		dasm_op2 (da, "jmpl", ARG_ADDR_DIRECT, ARG_RD);
	}
}

/* 2 39: rett rs1, reg_or_imm */
static void opd239 (s32_dasm_t *da)
{
	dasm_op1 (da, "rett", ARG_ADDR_DIRECT);
}

/* 2 3a: ticc rs1, reg_or_imm */
static void opd23a (s32_dasm_t *da)
{
	strcpy (da->op, "t");
	strcat (da->op, s32_icc_names[(da->ir >> 25) & 0x0f]);

	da->argn = 1;
	dasm_arg (da->arg1, da->ir, ARG_TRAP);
}

/* 2 3c: save rs1, reg_or_imm, rd */
static void opd23c (s32_dasm_t *da)
{
	unsigned rd, rs1, rs2, i;

	rd = s32_ir_rd (da->ir);
	rs1 = s32_ir_rs1 (da->ir);
	rs2 = s32_ir_rs2 (da->ir);
	i = s32_ir_i (da->ir);

	if ((i == 0) && (rd == 0) && (rs1 == 0) && (rs2 == 0)) {
		dasm_op0 (da, "save");
	}
	else {
		dasm_op3 (da, "save", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
	}
}

/* 2 3d: restore rs1, reg_or_imm, rd */
static void opd23d (s32_dasm_t *da)
{
	unsigned rd, rs1, rs2, i;

	rd = s32_ir_rd (da->ir);
	rs1 = s32_ir_rs1 (da->ir);
	rs2 = s32_ir_rs2 (da->ir);
	i = s32_ir_i (da->ir);

	if ((i == 0) && (rd == 0) && (rs1 == 0) && (rs2 == 0)) {
		dasm_op0 (da, "restore");
	}
	else {
		dasm_op3 (da, "restore", ARG_RS1, ARG_RS2_OR_SIMM13, ARG_RD);
	}
}

/* 3 00: ld [rs1 + reg_or_imm], rd */
static void opd300 (s32_dasm_t *da)
{
	dasm_op2 (da, "ld", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 01: ldub [rs1 + reg_or_imm], rd */
static void opd301 (s32_dasm_t *da)
{
	dasm_op2 (da, "ldub", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 02: lduh [rs1 + reg_or_imm], rd */
static void opd302 (s32_dasm_t *da)
{
	dasm_op2 (da, "lduh", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 03: ldd [rs1 + reg_or_imm], rd */
static void opd303 (s32_dasm_t *da)
{
	dasm_op2 (da, "ldd", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 04: st rd, [rs1 + reg_or_imm] */
static void opd304 (s32_dasm_t *da)
{
	dasm_op2 (da, "st", ARG_RD, ARG_ADDR_INDIRECT);
}

/* 3 05: stb rd, [rs1 + reg_or_imm] */
static void opd305 (s32_dasm_t *da)
{
	dasm_op2 (da, "stb", ARG_RD, ARG_ADDR_INDIRECT);
}

/* 3 06: sth rd, [rs1 + reg_or_imm] */
static void opd306 (s32_dasm_t *da)
{
	dasm_op2 (da, "sth", ARG_RD, ARG_ADDR_INDIRECT);
}

/* 3 07: std rd, [rs1 + reg_or_imm] */
static void opd307 (s32_dasm_t *da)
{
	dasm_op2 (da, "std", ARG_RD, ARG_ADDR_INDIRECT);
}

/* 3 09: ldsb [rs1 + reg_or_imm], rd */
static void opd309 (s32_dasm_t *da)
{
	dasm_op2 (da, "ldsb", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 0A: ldsh [rs1 + reg_or_imm], rd */
static void opd30a (s32_dasm_t *da)
{
	dasm_op2 (da, "ldsh", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 0D: ldstub [rs1 + reg_or_imm], rd */
static void opd30d (s32_dasm_t *da)
{
	dasm_op2 (da, "ldstub", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 0F: swap [rs1 + reg_or_imm], rd */
static void opd30f (s32_dasm_t *da)
{
	dasm_op2 (da, "swap", ARG_ADDR_INDIRECT, ARG_RD);
}

/* 3 10: lda [rs1 + rs2]asi, rd */
static void opd310 (s32_dasm_t *da)
{
	dasm_op3 (da, "lda", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 11: lduba [rs1 + rs2]asi, rd */
static void opd311 (s32_dasm_t *da)
{
	dasm_op3 (da, "lduba", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 12: lduha [rs1 + rs2]asi, rd */
static void opd312 (s32_dasm_t *da)
{
	dasm_op3 (da, "lduha", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 13: ldda [rs1 + rs2]asi, rd */
static void opd313 (s32_dasm_t *da)
{
	dasm_op3 (da, "ldda", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 14: sta rd, [rs1 + rs2]asi */
static void opd314 (s32_dasm_t *da)
{
	dasm_op3 (da, "sta", ARG_RD, ARG_ADDR_INDIRECT, ARG_ASI);
}

/* 3 15: stba rd, [rs1 + rs2]asi */
static void opd315 (s32_dasm_t *da)
{
	dasm_op3 (da, "stba", ARG_RD, ARG_ADDR_INDIRECT, ARG_ASI);
}

/* 3 16: stha rd, [rs1 + rs2]asi */
static void opd316 (s32_dasm_t *da)
{
	dasm_op3 (da, "stha", ARG_RD, ARG_ADDR_INDIRECT, ARG_ASI);
}

/* 3 17: stda rd, [rs1 + rs2]asi */
static void opd317 (s32_dasm_t *da)
{
	dasm_op3 (da, "stda", ARG_RD, ARG_ADDR_INDIRECT, ARG_ASI);
}

/* 3 19: ldsba [rs1 + rs2]asi, rd */
static void opd319 (s32_dasm_t *da)
{
	dasm_op3 (da, "ldsba", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 1a: ldsha [rs1 + rs2]asi, rd */
static void opd31a (s32_dasm_t *da)
{
	dasm_op3 (da, "ldsha", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 1D: ldstuba [rs1 + rs2]asi, rd */
static void opd31d (s32_dasm_t *da)
{
	dasm_op3 (da, "ldstuba", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 1F: swapa [rs1 + rs2]asi, rd */
static void opd31f (s32_dasm_t *da)
{
	dasm_op3 (da, "swapa", ARG_ADDR_INDIRECT, ARG_ASI, ARG_RD);
}

/* 3 20: ldf [rs1 + rs2], rd */
static void opd320 (s32_dasm_t *da)
{
	dasm_op2 (da, "ldf", ARG_ADDR_INDIRECT, ARG_RD_FP);
}

/* 3 21: ldf [rs1 + rs2], %fsr */
static void opd321 (s32_dasm_t *da)
{
	dasm_op2 (da, "ld", ARG_ADDR_INDIRECT, ARG_REG_FSR);
}

/* 3 23: lddf [rs1 + rs2], rd */
static void opd323 (s32_dasm_t *da)
{
	dasm_op2 (da, "lddf", ARG_ADDR_INDIRECT, ARG_RD_FP);
}

/* 3 24: stf rd, [rs1 + rs2] */
static void opd324 (s32_dasm_t *da)
{
	dasm_op2 (da, "stf", ARG_RD_FP, ARG_ADDR_INDIRECT);
}

/* 3 25: stf %fsr, [rs1 + rs2] */
static void opd325 (s32_dasm_t *da)
{
	dasm_op2 (da, "stf", ARG_REG_FSR, ARG_ADDR_INDIRECT);
}

/* 3 27: stdf rd, [rs1 + rs2] */
static void opd327 (s32_dasm_t *da)
{
	dasm_op2 (da, "stdf", ARG_RD_FP, ARG_ADDR_INDIRECT);
}

/* 3 30: ldc [rs1 + rs2], rd */
static void opd330 (s32_dasm_t *da)
{
	dasm_op2 (da, "ldc", ARG_ADDR_INDIRECT, ARG_RD_CP);
}

/* 3 31: ldc [rs1 + rs2], %csr */
static void opd331 (s32_dasm_t *da)
{
	dasm_op2 (da, "ld", ARG_ADDR_INDIRECT, ARG_REG_CSR);
}

/* 3 33: lddc [rs1 + rs2], rd */
static void opd333 (s32_dasm_t *da)
{
	dasm_op2 (da, "lddc", ARG_ADDR_INDIRECT, ARG_RD_CP);
}

/* 3 34: stc rd, [rs1 + rs2] */
static void opd334 (s32_dasm_t *da)
{
	dasm_op2 (da, "stc", ARG_RD_CP, ARG_ADDR_INDIRECT);
}

/* 3 35: stc %csr, [rs1 + rs2] */
static void opd335 (s32_dasm_t *da)
{
	dasm_op2 (da, "st", ARG_REG_CSR, ARG_ADDR_INDIRECT);
}

/* 3 37: stc rd, [rs1 + rs2] */
static void opd337 (s32_dasm_t *da)
{
	dasm_op2 (da, "stdc", ARG_RD_CP, ARG_ADDR_INDIRECT);
}


void s32_dasm (s32_dasm_t *da, uint32_t pc, uint32_t ir)
{
	da->flags = 0;

	da->pc = pc;
	da->ir = ir;

	switch ((ir >> 30) & 0x03) {
	case 0:
		s32_dasm_op[0][(ir >> 22) & 0x07] (da);
		break;

	case 1:
		s32_dasm_op[1][0] (da);
		break;

	case 2:
		s32_dasm_op[2][(ir >> 19) & 0x3f] (da);
		break;

	case 3:
		s32_dasm_op[3][(ir >> 19) & 0x3f] (da);
		break;
	}
}

void s32_dasm_mem (sparc32_t *c, s32_dasm_t *da, uint32_t pc, uint8_t asi, unsigned xlat)
{
	uint32_t ir;

	da->flags = 0;

	if (s32_get_mem32 (c, pc, asi, xlat, &ir)) {
		da->pc = pc;
		da->ir = 0xffffffffUL;

		dasm_op0 (da, "TLB_MISS");

		da->flags |= S32_DFLAG_TLBM;
	}
	else {
		s32_dasm (da, pc, ir);
	}
}

static
s32_dasm_f s32_dasm_op[4][64] = {
	{
		opd000, opd_ud, opd002, opd_ud, /* 0 00 */
		opd004, opd_ud, opd006, opd007,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud, /* 0 10 */
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud, /* 0 20 */
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud, /* 0 30 */
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud
	},
	{
		opd100, opd100, opd100, opd100, /* 1 00 */
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100, /* 1 10 */
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100, /* 1 20 */
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100, /* 1 30 */
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
		opd100, opd100, opd100, opd100,
	},
	{
		opd200, opd201, opd202, opd203, /* 2 00 */
		opd204, opd205, opd206, opd207,
		opd208, opd_ud, opd20a, opd20b,
		opd20c, opd_ud, opd20e, opd20f,
		opd210, opd211, opd212, opd213, /* 2 10 */
		opd214, opd215, opd216, opd217,
		opd218, opd_ud, opd21a, opd21b,
		opd21c, opd_ud, opd21e, opd21f,
		opd220, opd221, opd222, opd223, /* 2 20 */
		opd224, opd225, opd226, opd227,
		opd228, opd229, opd22a, opd22b,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd230, opd231, opd232, opd233, /* 2 30 */
		opd234, opd235, opd236, opd237,
		opd238, opd239, opd23a, opd_ud,
		opd23c, opd23d, opd_ud, opd_ud
	},
	{
		opd300, opd301, opd302, opd303, /* 3 00 */
		opd304, opd305, opd306, opd307,
		opd_ud, opd309, opd30a, opd_ud,
		opd_ud, opd30d, opd_ud, opd30f,
		opd310, opd311, opd312, opd313, /* 3 10 */
		opd314, opd315, opd316, opd317,
		opd_ud, opd319, opd31a, opd_ud,
		opd_ud, opd31d, opd_ud, opd31f,
		opd320, opd321, opd_ud, opd323, /* 3 20 */
		opd324, opd325, opd_ud, opd327,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd330, opd331, opd_ud, opd333, /* 3 30 */
		opd334, opd335, opd_ud, opd337,
		opd_ud, opd_ud, opd_ud, opd_ud,
		opd_ud, opd_ud, opd_ud, opd_ud
	}
};
