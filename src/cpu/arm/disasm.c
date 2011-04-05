/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/arm/disasm.c                                         *
 * Created:     2004-11-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arm.h"
#include "internal.h"


static arm_dasm_f arm_dasm_op[256];


enum {
	ARG_NONE,

	ARG_UIMM16,
	ARG_UIMM24,
	ARG_UIMM32,

	ARG_RD,
	ARG_RM,
	ARG_RN,
	ARG_RS,
	ARG_RNI,

	ARG_CD,
	ARG_CN,
	ARG_CM,

	ARG_SH,

	ARG_COPR,
	ARG_COPR_OP1,
	ARG_COPR_OP2,
	ARG_COPR_OP3,

	ARG_CPSR,
	ARG_SPSR,
	ARG_PSR_FLD,

	ARG_AMODE2,
	ARG_AMODE3,
	ARG_AMODE4
};


enum {
	FLG_NONE = 0,
	FLG_COND = 1,
	FLG_S    = 2
};


static const char *arm_cond_names[16] = {
	"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
	"hi", "ls", "ge", "lt", "gt", "le", "", "nv"
};

static const char *arm_shifts[4] = {
	"lsl", "lsr", "asr", "ror"
};


static
unsigned dasm_arg (char *dst1, char *dst2, uint32_t ir, unsigned arg)
{
	switch (arg) {
	case ARG_NONE:
		dst1[0] = 0;
		return (1);

	case ARG_UIMM16:
		sprintf (dst1, "0x%04lx", (unsigned long) (ir & 0xffffU));
		return (1);

	case ARG_UIMM24:
		sprintf (dst1, "0x%06lx", (unsigned long) (ir & 0xffffffUL));
		return (1);

	case ARG_UIMM32:
		sprintf (dst1, "0x%08lx", (unsigned long) ir);
		return (1);

	case ARG_RD:
		if (arm_rd_is_pc (ir)) {
			strcpy (dst1, "pc");
		}
		else {
			sprintf (dst1, "r%u", (unsigned) arm_ir_rd (ir));
		}
		return (1);

	case ARG_RM:
		sprintf (dst1, "r%u", (unsigned) arm_ir_rm (ir));
		return (1);

	case ARG_RN:
		sprintf (dst1, "r%u", (unsigned) arm_ir_rn (ir));
		return (1);

	case ARG_RS:
		sprintf (dst1, "r%u", (unsigned) arm_ir_rs (ir));
		return (1);

	case ARG_RNI:
		sprintf (dst1, "[r%u]", (unsigned) arm_ir_rn (ir));
		return (1);

	case ARG_CD:
		sprintf (dst1, "c%u", (unsigned) arm_ir_rd (ir));
		return (1);

	case ARG_CN:
		sprintf (dst1, "c%u", (unsigned) arm_ir_rn (ir));
		return (1);

	case ARG_CM:
		sprintf (dst1, "c%u", (unsigned) arm_ir_rm (ir));
		return (1);

	case ARG_SH:
		if (arm_ir_i (ir)) {
			unsigned n;
			uint32_t v;

			n = arm_get_bits (ir, 8, 4) << 1;
			v = arm_get_bits (ir, 0, 8);
			v = arm_ror32 (v, n);

			sprintf (dst1, "#0x%08lx", (unsigned long) v);

			return (1);
		}
		else if ((ir & (1UL << 4)) == 0) {
			unsigned n;
			unsigned s;

			n = arm_get_bits (ir, 7, 5);
			s = arm_get_bits (ir, 5, 2);

			if (n == 0) {
				sprintf (dst1, "r%u", (unsigned) (ir & 0x0f));
				if (s == 3) {
					strcat (dst1, " rrx");
				}
			}
			else {
				sprintf (dst1, "r%u %s #%u", (unsigned) (ir & 0x0f), arm_shifts[s], n);
			}

			return (1);
		}
		else if ((ir & (1UL << 7)) == 0) {
			unsigned rs = arm_get_bits (ir, 8, 4);
			unsigned s = arm_get_bits (ir, 5, 2);

			sprintf (dst1, "r%u %s r%u", (unsigned) (ir & 0x0f), arm_shifts[s], rs);

			return (1);
		}
		return (0);

	case ARG_COPR:
		sprintf (dst1, "p%u", (unsigned) arm_get_bits (ir, 8, 4));
		return (1);

	case ARG_COPR_OP1:
		sprintf (dst1, "0x%02X", (unsigned) arm_get_bits (ir, 21, 3));
		return (1);

	case ARG_COPR_OP2:
		sprintf (dst1, "0x%02X", (unsigned) arm_get_bits (ir, 5, 3));
		return (1);

	case ARG_COPR_OP3:
		sprintf (dst1, "0x%02X", (unsigned) arm_get_bits (ir, 4, 4));
		return (1);

	case ARG_CPSR:
		strcpy (dst1, "cpsr");
		return (1);

	case ARG_SPSR:
		strcpy (dst1, "spsr");
		return (1);

	case ARG_PSR_FLD:
		if (arm_get_bit (ir, 22)) {
			strcpy (dst1, "spsr_");
		}
		else {
			strcpy (dst1, "cpsr_");
		}

		if (arm_get_bit (ir, 16)) {
			strcat (dst1, "c");
		}
		if (arm_get_bit (ir, 17)) {
			strcat (dst1, "x");
		}
		if (arm_get_bit (ir, 18)) {
		 strcat (dst1, "s");
		}
		if (arm_get_bit (ir, 19)) {
			strcat (dst1, "f");
		}
		return (1);

	case ARG_AMODE2:
		{
			int  i, p, w;
			char *fmt;

			i = arm_get_bit (ir, 25);
			p = arm_get_bit (ir, 24);
			/* u = arm_get_bit (ir, 23); */
			w = arm_get_bit (ir, 21);

			if (i) {
				if (p) {
					fmt = w ? "[r%u %c r%u %s %u]!" : "[r%u %c r%u %s %u]";
				}
				else {
					fmt = "[r%u] %c r%u %s %u";
				}
				sprintf (dst1, fmt, (unsigned) arm_ir_rn (ir),
					arm_get_bit (ir, 23) ? '+' : '-',
					(unsigned) arm_ir_rm (ir),
					arm_shifts[arm_get_bits (ir, 5, 2)],
					arm_get_bits (ir, 7, 5)
				);
			}
			else {
				unsigned long ofs = arm_extu (ir, 12);

				if (p) {
					if (w) {
						fmt = "[r%u %c 0x%08lx]!";
					}
					else {
						if (ofs == 0) {
							fmt = "[r%u]";
						}
						else {
							fmt = "[r%u %c 0x%08lx]";
						}
					}
				}
				else {
					fmt = "[r%u] %c 0x%08lx";
				}
				sprintf (dst1, fmt, (unsigned) arm_ir_rn (ir),
					arm_get_bit (ir, 23) ? '+' : '-', ofs
				);
			}
		}
		return (1);

	case ARG_AMODE3:
		{
			int  i, p, u, w;
			char *fmt;

			p = arm_get_bit (ir, 24);
			u = arm_get_bit (ir, 23);
			i = arm_get_bit (ir, 22);
			w = arm_get_bit (ir, 21);

			if (i == 0) {
				if (p) {
					fmt = w ? "[r%u %c r%u]!" : "[r%u %c r%u]";
				}
				else {
					fmt = "[r%u] %c r%u";
				}
				sprintf (dst1, fmt, (unsigned) arm_ir_rn (ir),
					u ? '+' : '-',
					(unsigned) arm_ir_rm (ir)
				);
			}
			else {
				unsigned long ofs;

				ofs = (arm_get_bits (ir, 8, 4) << 4) | arm_get_bits (ir, 0, 4);

				if (p) {
					if (w) {
						fmt = "[r%u %c 0x%08lx]!";
					}
					else {
						if (ofs == 0) {
							fmt = "[r%u]";
						}
						else {
							fmt = "[r%u %c 0x%08lx]";
						}
					}
				}
				else {
					fmt = "[r%u] %c 0x%08lx";
				}
				sprintf (dst1, fmt, (unsigned) arm_ir_rn (ir),
					arm_get_bit (ir, 23) ? '+' : '-', ofs
				);
			}
		}
		return (1);

	case ARG_AMODE4:
		{
			unsigned i, j, n;

			sprintf (dst1, "r%u", (unsigned) arm_ir_rn (ir));
			if (arm_get_bit (ir, 21)) {
				strcat (dst1, "!");
			}

			j = sprintf (dst2, "{ ");
			n = 0;

			for (i = 0; i < 16; i++) {
				if (ir & (1UL << i)) {
					if (n > 0) {
						j += sprintf (dst2 + j, ", r%u", i);
					}
					else {
						j += sprintf (dst2 + j, "r%u", i);
					}

					n += 1;
				}
			}
			strcpy (dst2 + j, " }");

			if (arm_get_bit (ir, 22)) {
				strcat (dst2, "^");
			}

			return (2);
		}
	}

	return (0);
}

static
const char *dasm_cond (uint32_t ir)
{
	return (arm_cond_names[(ir >> 28) & 0x0f]);
}

static
int dasm_op0 (arm_dasm_t *da, const char *op, unsigned flg)
{
	strcpy (da->op, op);

	if (flg & FLG_COND) {
		strcat (da->op, dasm_cond (da->ir));
	}

	if (flg & FLG_S) {
		if (arm_ir_s (da->ir)) {
			strcat (da->op, "s");
		}
	}

	da->argn = 0;

	return (0);
}

static
int dasm_op1 (arm_dasm_t *da, const char *op, unsigned flg,
	unsigned arg1)
{
	dasm_op0 (da, op, flg);

	da->argn = dasm_arg (da->arg[0], da->arg[1], da->ir, arg1);

	return (0);
}

static
int dasm_op2 (arm_dasm_t *da, const char *op, unsigned flg,
	unsigned arg1, unsigned arg2)
{
	dasm_op0 (da, op, flg);

	da->argn = dasm_arg (da->arg[0], da->arg[1], da->ir, arg1);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg2);

	return (0);
}

static
int dasm_op3 (arm_dasm_t *da, const char *op, unsigned flg,
	unsigned arg1, unsigned arg2, unsigned arg3)
{
	dasm_op0 (da, op, flg);

	da->argn = dasm_arg (da->arg[0], da->arg[1], da->ir, arg1);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg2);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg3);

	return (0);
}

static
int dasm_op4 (arm_dasm_t *da, const char *op, unsigned flg,
	unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4)
{
	dasm_op0 (da, op, flg);

	da->argn = dasm_arg (da->arg[0], da->arg[1], da->ir, arg1);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg2);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg3);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg4);

	return (0);
}

static
int dasm_op5 (arm_dasm_t *da, const char *op, unsigned flg,
	unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5)
{
	dasm_op0 (da, op, flg);

	da->argn = dasm_arg (da->arg[0], da->arg[1], da->ir, arg1);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg2);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg3);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg4);
	da->argn += dasm_arg (da->arg[da->argn], da->arg[da->argn + 1], da->ir, arg5);

	return (0);
}


static void opdud (arm_dasm_t *da)
{
	dasm_op1 (da, "???", FLG_COND, ARG_UIMM32);
}

/* 00 09: mul[cond][s] rn, rm, rs */
static void opd00_09 (arm_dasm_t *da)
{
	dasm_op3 (da, "mul", FLG_COND | FLG_S, ARG_RN, ARG_RM, ARG_RS);
}

/* 00 0B: ldr/str[cond][h|sh|sb|d] rd, addressing_mode */
static void opd00_0b (arm_dasm_t *da)
{
	if (arm_get_bits (da->ir, 5, 2) == 0) {
		opdud (da);
		return;
	}

	if (arm_get_bit (da->ir, 20)) {
		dasm_op2 (da, "ldr", FLG_COND, ARG_RD, ARG_AMODE3);

		if (arm_get_bit (da->ir, 6)) {
			strcat (da->op, "s");
		}

		if (arm_get_bit (da->ir, 5)) {
			strcat (da->op, "h");
		}
		else {
			strcat (da->op, "b");
		}
	}
	else {
		if (arm_get_bit (da->ir, 6)) {
			if (arm_get_bit (da->ir, 5)) {
				dasm_op2 (da, "str", FLG_COND, ARG_RD, ARG_AMODE3);
			}
			else {
				dasm_op2 (da, "ldr", FLG_COND, ARG_RD, ARG_AMODE3);
			}

			strcat (da->op, "d");
		}
		else {
			dasm_op2 (da, "str", FLG_COND, ARG_RD, ARG_AMODE3);

			if (arm_get_bit (da->ir, 5)) {
				strcat (da->op, "h");
			}
			else {
				strcat (da->op, "b");
			}
		}
	}
}

/* 00 ext */
static void opd00_ext (arm_dasm_t *da)
{
	switch (arm_get_shext (da->ir)) {
	case 0x09:
		opd00_09 (da);
		break;

	default:
		opd00_0b (da);
		break;
	}
}

/* 00: and[cond][s] rd, rn, shifter_operand */
static void opd00 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_ext (da);
	}
	else {
		dasm_op3 (da, "and", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 02 09: mla[cond][s] rd, rm, rs, rn */
static void opd02_09 (arm_dasm_t *da)
{
	dasm_op4 (da, "mla", FLG_COND | FLG_S, ARG_RN, ARG_RM, ARG_RS, ARG_RD);
}

/* 02 ext */
static void opd02_ext (arm_dasm_t *da)
{
	switch (arm_get_shext (da->ir)) {
	case 0x09:
		opd02_09 (da);
		break;

	default:
		opd00_0b (da);
		break;
	}
}

/* 02: eor[cond][s] rd, rn, shifter_operand */
static void opd02 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd02_ext (da);
	}
	else {
		dasm_op3 (da, "eor", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 04: sub[cond][s] rd, rn, shifter_operand */
static void opd04 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op3 (da, "sub", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 06: rsb[cond][s] rd, rn, shifter_operand */
static void opd06 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op3 (da, "rsb", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 08: add[cond][s] rd, rn, shifter_operand */
static void opd08 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		if (arm_get_shext (da->ir) == 0x09) {
			dasm_op4 (da, "umull", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_RM, ARG_RS);
		}
		else {
			opd00_0b (da);
		}
	}
	else {
		dasm_op3 (da, "add", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 0A: adc[cond][s] rd, rn, shifter_operand */
static void opd0a (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		if (arm_get_shext (da->ir) == 0x09) {
			dasm_op4 (da, "umlal", FLG_COND | FLG_S, ARG_RN, ARG_RD, ARG_RM, ARG_RS);
		}
		else {
			opd00_0b (da);
		}
	}
	else {
		dasm_op3 (da, "adc", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 0C: sbc[cond][s] rd, rn, shifter_operand */
static void opd0c (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		if (arm_get_shext (da->ir) == 0x09) {
			dasm_op4 (da, "smull", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_RM, ARG_RS);
		}
		else {
			opd00_0b (da);
		}
	}
	else {
		dasm_op3 (da, "sbc", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 0E: rsc[cond][s] rd, rn, shifter_operand */
static void opd0e (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		if (arm_get_shext (da->ir) == 0x09) {
			dasm_op4 (da, "smlal", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_RM, ARG_RS);
		}
		else {
			opd00_0b (da);
		}
	}
	else {
		dasm_op3 (da, "rsc", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 10 00: mrs[cond] rd, cpsr */
static void opd10_00 (arm_dasm_t *da)
{
	dasm_op2 (da, "mrs", FLG_COND, ARG_RD, ARG_CPSR);
}

/* 10 09: swp[cond] rd, rm, rn */
static void opd10_09 (arm_dasm_t *da)
{
	dasm_op3 (da, "swp", FLG_COND, ARG_RD, ARG_RM, ARG_RNI);
}

/* 10 */
static void opd10 (arm_dasm_t *da)
{
	switch (arm_get_bits (da->ir, 4, 4)) {
	case 0x00:
		opd10_00 (da);
		break;

	case 0x09:
		opd10_09 (da);
		break;

	case 0x0b:
	case 0x0d:
	case 0x0f:
		opd00_0b (da);
		break;

	default:
		opdud (da);
		break;
	}
}

/* 11: tst[cond] rn, shifter_operand */
static void opd11 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op2 (da, "tst", FLG_COND, ARG_RN, ARG_SH);
	}
}

/* 12 00: msr[cond] fields, rm */
static void opd12_00 (arm_dasm_t *da)
{
	dasm_op2 (da, "msr", FLG_COND, ARG_PSR_FLD, ARG_RM);
}

/* 12 01: bx[cond] rm */
static void opd12_01 (arm_dasm_t *da)
{
	dasm_op1 (da, "bx", FLG_COND, ARG_RM);
}

/* 12 03: blx[cond] rm */
static void opd12_03 (arm_dasm_t *da)
{
	dasm_op1 (da, "blx", FLG_COND, ARG_RM);

	da->flags |= ARM_DFLAG_CALL;
}

/* 12 07: bkpt uimm16 */
static void opd12_07 (arm_dasm_t *da)
{
	uint32_t v;

	v = (da->ir & 0x0f) | ((da->ir >> 4) & 0xfff0);

	dasm_op0 (da, "bkpt", FLG_COND);
	da->argn = dasm_arg (da->arg[0], da->arg[1], v, ARG_UIMM16);
}

/* 12 */
static void opd12 (arm_dasm_t *da)
{
	switch (da->ir & 0x0ff000f0) {
	case 0x01200000:
		opd12_00 (da);
		break;

	case 0x01200010:
		opd12_01 (da);
		break;

	case 0x01200030:
		opd12_03 (da);
		break;

	case 0x01200070:
		opd12_07 (da);
		break;

	case 0x012000b0:
	case 0x012000d0:
	case 0x012000f0:
		opd00_0b (da);
		break;

	default:
		opdud (da);
		break;
	}
}

/* 13: teq[cond] rn, shifter_operand */
static void opd13 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op2 (da, "teq", FLG_COND, ARG_RN, ARG_SH);
	}
}

/* 14 00: mrs[cond] rd, spsr */
static void opd14_00 (arm_dasm_t *da)
{
	dasm_op2 (da, "mrs", FLG_COND, ARG_RD, ARG_SPSR);
}

/* 14 09: swp[cond]b rd, rm, rn */
static void opd14_09 (arm_dasm_t *da)
{
	dasm_op3 (da, "swp", FLG_COND, ARG_RD, ARG_RM, ARG_RNI);
	strcat (da->op, "b");
}

/* 14 */
static void opd14 (arm_dasm_t *da)
{
	switch (arm_get_bits (da->ir, 4, 4)) {
	case 0x00:
		opd14_00 (da);
		break;

	case 0x09:
		opd14_09 (da);
		break;

	case 0x0b:
	case 0x0d:
	case 0x0f:
		opd00_0b (da);
		break;

	default:
		opdud (da);
		break;
	}
}

/* 15: cmp[cond] rn, shifter_operand */
static void opd15 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op2 (da, "cmp", FLG_COND, ARG_RN, ARG_SH);
	}
}

/* 16 01: clz rd, rm */
static void opd16_01 (arm_dasm_t *da)
{
	dasm_op2 (da, "clz", FLG_COND, ARG_RD, ARG_RM);
}

/* 16 08: smulxy[cond] rd, rm, rs */
static void opd16_08 (arm_dasm_t *da)
{
	char op[16];

	strcpy (op, "smul");
	strcat (op, arm_get_bit (da->ir, 5) ? "t" : "b");
	strcat (op, arm_get_bit (da->ir, 6) ? "t" : "b");

	dasm_op3 (da, op, FLG_COND, ARG_RN, ARG_RM, ARG_RS);
}

/* 16 */
static void opd16 (arm_dasm_t *da)
{
	switch (da->ir & 0x0ff000f0UL) {
	case 0x01600000UL:
		opd12_00 (da);
		break;

	case 0x01600010UL:
		opd16_01 (da);
		break;

	case 0x01600080UL:
	case 0x016000a0UL:
	case 0x016000c0UL:
	case 0x016000e0UL:
		opd16_08 (da);
		break;

	case 0x016000b0UL:
	case 0x016000d0UL:
	case 0x016000f0UL:
		opd00_0b (da);
		break;

	default:
		opdud (da);
		break;
	}
}

/* 17: cmn[cond] rn, shifter_operand */
static void opd17 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op2 (da, "cmn", FLG_COND, ARG_RN, ARG_SH);
	}
}

/* 18: orr[cond][s] rd, rn, shifter_operand */
static void opd18 (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op3 (da, "orr", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 1A: mov[cond][s] rd, shifter_operand */
static void opd1a (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op2 (da, "mov", FLG_COND | FLG_S, ARG_RD, ARG_SH);
	}
}

/* 1C: bic[cond][s] rd, rn, shifter_operand */
static void opd1c (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op3 (da, "bic", FLG_COND | FLG_S, ARG_RD, ARG_RN, ARG_SH);
	}
}

/* 1E: mvn[cond][s] rd, shifter_operand */
static void opd1e (arm_dasm_t *da)
{
	if (arm_is_shext (da->ir)) {
		opd00_0b (da);
	}
	else {
		dasm_op2 (da, "mvn", FLG_COND | FLG_S, ARG_RD, ARG_SH);
	}
}

/* 32: msr[cond] fields, #imm */
static void opd32 (arm_dasm_t *da)
{
	dasm_op2 (da, "msr", FLG_COND, ARG_PSR_FLD, ARG_SH);
}

/* 40: ldr/str[cond][b][t] rd, address */
static void opd40 (arm_dasm_t *da)
{
	if (arm_get_bit (da->ir, 20)) {
		dasm_op2 (da, "ldr", FLG_COND, ARG_RD, ARG_AMODE2);
	}
	else {
		dasm_op2 (da, "str", FLG_COND, ARG_RD, ARG_AMODE2);
	}

	if (arm_get_bit (da->ir, 22)) {
		strcat (da->op, "b");
	}

	if ((arm_get_bit (da->ir, 24) == 0) && (arm_get_bit (da->ir, 21) == 1)) {
		strcat (da->op, "t");
	}
}

/* 80: ldm/stm[cond][mode] rn[!], registers[^] */
static void opd80 (arm_dasm_t *da)
{
	if (arm_get_bit (da->ir, 20)) {
		dasm_op1 (da, "ldm", FLG_COND, ARG_AMODE4);
	}
	else {
		dasm_op1 (da, "stm", FLG_COND, ARG_AMODE4);
	}

	strcat (da->op, arm_get_bit (da->ir, 23) ? "i" : "d");
	strcat (da->op, arm_get_bit (da->ir, 24) ? "b" : "a");
}

/* A0: b[cond] target */
static void opda0 (arm_dasm_t *da)
{
	uint32_t d;

	d = arm_exts (da->ir, 24);
	d = (da->pc + (d << 2) + 8) & 0xffffffffUL;

	dasm_op0 (da, "b", FLG_COND);
	da->argn = dasm_arg (da->arg[0], da->arg[1], d, ARG_UIMM32);
}

/* B0: bl[cond] target */
static void opdb0 (arm_dasm_t *da)
{
	uint32_t d;

	d = arm_exts (da->ir, 24);
	d = (da->pc + (d << 2) + 8) & 0xffffffffUL;

	dasm_op0 (da, "bl", FLG_COND);
	da->argn = dasm_arg (da->arg[0], da->arg[1], d, ARG_UIMM32);

	da->flags |= ARM_DFLAG_CALL;
}

/* C4: mcrr[cond] coproc, opcode, rd, rn, crm */
static void opdc4 (arm_dasm_t *da)
{
	dasm_op5 (da, "mcrr", FLG_COND, ARG_COPR, ARG_COPR_OP3, ARG_RD, ARG_RN, ARG_RM);
}

/* C5: mrrc[cond] coproc, opcode, rd, rn, crm */
static void opdc5 (arm_dasm_t *da)
{
	dasm_op5 (da, "mrrc", FLG_COND, ARG_COPR, ARG_COPR_OP3, ARG_RD, ARG_RN, ARG_RM);
}

/* E0 00: cdp[cond] coproc, opcode1, crd, crn, crm, opcode2 */
static void opde0_00 (arm_dasm_t *da)
{
	unsigned n;

	if (arm_get_bits (da->ir, 28, 4) == 0x0f) {
		dasm_op0 (da, "cdp2", FLG_NONE);
	}
	else {
		dasm_op0 (da, "cdp", FLG_COND);
	}

	n = dasm_arg (da->arg[0], da->arg[1], da->ir, ARG_COPR);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_COPR_OP1);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_CD);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_CN);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_CM);

	if (arm_get_bits (da->ir, 5, 3) != 0) {
		n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_COPR_OP2);
	}

	da->argn = n;
}

/* E0 01: mcr[cond] coproc, opcode1, rd, crn, crm, opcode2 */
static void opde0_01 (arm_dasm_t *da)
{
	unsigned n;

	if (arm_get_bits (da->ir, 28, 4) == 0x0f) {
		dasm_op0 (da, "mcr2", FLG_NONE);
	}
	else {
		dasm_op0 (da, "mcr", FLG_COND);
	}

	n = dasm_arg (da->arg[0], da->arg[1], da->ir, ARG_COPR);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_COPR_OP1);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_RD);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_CN);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_CM);

	if (arm_get_bits (da->ir, 5, 3) != 0) {
		n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_COPR_OP2);
	}

	da->argn = n;
}

/* E0 11: mrc[cond] coproc, opcode1, rd, crn, crm, opcode2 */
static void opde0_11 (arm_dasm_t *da)
{
	unsigned n;

	if (arm_get_bits (da->ir, 28, 4) == 0x0f) {
		dasm_op0 (da, "mrc2", FLG_NONE);
	}
	else {
		dasm_op0 (da, "mrc", FLG_COND);
	}

	n = dasm_arg (da->arg[0], da->arg[1], da->ir, ARG_COPR);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_COPR_OP1);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_RD);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_CN);
	n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_CM);

	if (arm_get_bits (da->ir, 5, 3) != 0) {
		n += dasm_arg (da->arg[n], da->arg[n + 1], da->ir, ARG_COPR_OP2);
	}

	da->argn = n;
}

/* E0 */
static void opde0 (arm_dasm_t *da)
{
	switch (da->ir & 0x00100010UL) {
	case 0x00000000UL:
	case 0x00100000UL:
		opde0_00 (da);
		break;

	case 0x00000010UL:
		opde0_01 (da);
		break;

	case 0x00100010UL:
		opde0_11 (da);
		break;

	default:
		opdud (da);
		break;
	}
}

/* F0: swi[cond] immediate */
static void opdf0 (arm_dasm_t *da)
{
	dasm_op1 (da, "swi", FLG_COND, ARG_UIMM24);
}


void arm_dasm (arm_dasm_t *da, uint32_t pc, uint32_t ir)
{
	da->flags = 0;

	da->pc = pc;
	da->ir = ir;

	arm_dasm_op[(ir >> 20) & 0xff] (da);
}

void arm_dasm_mem (arm_t *c, arm_dasm_t *da, uint32_t pc, unsigned xlat)
{
	uint32_t ir;

	da->flags = 0;

	if (arm_get_mem32 (c, pc, xlat, &ir)) {
		da->pc = pc;
		da->ir = 0xffffffffUL;

		dasm_op0 (da, "<xlat>", FLG_NONE);

		da->flags |= ARM_DFLAG_TLBM;
	}
	else {
		arm_dasm (da, pc, ir);
	}
}

static
arm_dasm_f arm_dasm_op[256] = {
	opd00, opd00, opd02, opd02, opd04, opd04, opd06, opd06,  /* 00 */
	opd08, opd08, opd0a, opd0a, opd0c, opd0c, opd0e, opd0e,
	opd10, opd11, opd12, opd13, opd14, opd15, opd16, opd17,  /* 10 */
	opd18, opd18, opd1a, opd1a, opd1c, opd1c, opd1e, opd1e,
	opd00, opd00, opd02, opd02, opd04, opd04, opd06, opd06,  /* 20 */
	opd08, opd08, opd0a, opd0a, opd0c, opd0c, opd0e, opd0e,
	opdud, opd11, opd32, opd13, opdud, opd15, opdud, opd17,  /* 30 */
	opd18, opd18, opd1a, opd1a, opd1c, opd1c, opd1e, opd1e,
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,  /* 40 */
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,  /* 50 */
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,  /* 60 */
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,  /* 70 */
	opd40, opd40, opd40, opd40, opd40, opd40, opd40, opd40,
	opd80, opd80, opd80, opd80, opd80, opd80, opd80, opd80,  /* 80 */
	opd80, opd80, opd80, opd80, opd80, opd80, opd80, opd80,
	opd80, opd80, opd80, opd80, opd80, opd80, opd80, opd80,  /* 90 */
	opd80, opd80, opd80, opd80, opd80, opd80, opd80, opd80,
	opda0, opda0, opda0, opda0, opda0, opda0, opda0, opda0,  /* a0 */
	opda0, opda0, opda0, opda0, opda0, opda0, opda0, opda0,
	opdb0, opdb0, opdb0, opdb0, opdb0, opdb0, opdb0, opdb0,  /* b0 */
	opdb0, opdb0, opdb0, opdb0, opdb0, opdb0, opdb0, opdb0,
	opdud, opdud, opdud, opdud, opdc4, opdc5, opdud, opdud,  /* c0 */
	opdud, opdud, opdud, opdud, opdud, opdud, opdud, opdud,
	opdud, opdud, opdud, opdud, opdud, opdud, opdud, opdud,  /* d0 */
	opdud, opdud, opdud, opdud, opdud, opdud, opdud, opdud,
	opde0, opde0, opde0, opde0, opde0, opde0, opde0, opde0,  /* e0 */
	opde0, opde0, opde0, opde0, opde0, opde0, opde0, opde0,
	opdf0, opdf0, opdf0, opdf0, opdf0, opdf0, opdf0, opdf0,  /* f0 */
	opdf0, opdf0, opdf0, opdf0, opdf0, opdf0, opdf0, opdf0
};
