/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/ppc405/disasm.c                                      *
 * Created:     2003-11-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

#include "ppc405.h"
#include "internal.h"


static p405_disasm_f p405_opd13[1024];
static p405_disasm_f p405_opd1f[1024];
static p405_disasm_f p405_dis[64];


#define ARG_NONE   0
#define ARG_RA     1
#define ARG_RB     2
#define ARG_RS     3
#define ARG_RT     4
#define ARG_RA0    5
#define ARG_SIMM16 6
#define ARG_UIMM16 7
#define ARG_IMM16S 8
#define ARG_UINT3  9
#define ARG_UINT5  10
#define ARG_UINT8  11
#define ARG_UINT16 12
#define ARG_UINT32 13
#define ARG_CRBIT  14
#define ARG_SPRN   15
#define ARG_DCRN   16


#define OPF_OE 0x0001
#define OPF_RC 0x0002


static
char *p405_cr_name[2][4] = {
	{ "lt", "gt", "eq", "so" },
	{ "ge", "le", "ne", "ns" }
};


static
const char *disasm_get_spr (unsigned sprn)
{
	switch (sprn) {
		case P405_SPRN_CCR0:
			return ("ccr0");

		case P405_SPRN_CTR:
			return ("ctr");

		case P405_SPRN_DAC1:
			return ("dac1");

		case P405_SPRN_DAC2:
			return ("dac2");

		case P405_SPRN_DBCR0:
			return ("dbcr0");

		case P405_SPRN_DBCR1:
			return ("dbcr1");

		case P405_SPRN_DBSR:
			return ("dbsr");

		case P405_SPRN_DCCR:
			return ("dccr");

		case P405_SPRN_DCWR:
			return ("dcwr");

		case P405_SPRN_DVC1:
			return ("dvc1");

		case P405_SPRN_DVC2:
			return ("dvc2");

		case P405_SPRN_DEAR:
			return ("dear");

		case P405_SPRN_ESR:
			return ("esr");

		case P405_SPRN_EVPR:
			return ("evpr");

		case P405_SPRN_IAC1:
			return ("iac1");

		case P405_SPRN_IAC2:
			return ("iac2");

		case P405_SPRN_IAC3:
			return ("iac3");

		case P405_SPRN_IAC4:
			return ("iac4");

		case P405_SPRN_ICCR:
			return ("iccr");

		case P405_SPRN_ICDBDR:
			return ("icdbdr");

		case P405_SPRN_LR:
			return ("lr");

		case P405_SPRN_PID:
			return ("pid");

		case P405_SPRN_PIT:
			return ("pit");

		case P405_SPRN_PVR:
			return ("pvr");

		case P405_SPRN_SGR:
			return ("sgr");

		case P405_SPRN_SLER:
			return ("sler");

		case P405_SPRN_SPRG0:
			return ("sprg0");

		case P405_SPRN_SPRG1:
			return ("sprg1");

		case P405_SPRN_SPRG2:
			return ("sprg2");

		case P405_SPRN_SPRG3:
			return ("sprg3");

		case P405_SPRN_SPRG4:
			return ("sprg4");

		case P405_SPRN_SPRG5:
			return ("sprg5");

		case P405_SPRN_SPRG6:
			return ("sprg6");

		case P405_SPRN_SPRG7:
			return ("sprg7");

		case P405_SPRN_SPRG4R:
			return ("sprg4r");

		case P405_SPRN_SPRG5R:
			return ("sprg5r");

		case P405_SPRN_SPRG6R:
			return ("sprg6r");

		case P405_SPRN_SPRG7R:
			return ("sprg7r");

		case P405_SPRN_SRR0:
			return ("srr0");

		case P405_SPRN_SRR1:
			return ("srr1");

		case P405_SPRN_SRR2:
			return ("srr2");

		case P405_SPRN_SRR3:
			return ("srr3");

		case P405_SPRN_SU0R:
			return ("su0r");

		case P405_SPRN_TBL:
			return ("tbl");

		case P405_SPRN_TBU:
			return ("tbu");

		case P405_SPRN_TCR:
			return ("tcr");

		case P405_SPRN_TSR:
			return ("tsr");

		case P405_SPRN_USPRG0:
			return ("usprg0");

		case P405_SPRN_XER:
			return ("XER");

		case P405_SPRN_ZPR:
			return ("ZPR");
	}

	return (NULL);
}

static
void disasm_dw (p405_disasm_t *dis, uint32_t val)
{
	dis->argn = 1;
	strcpy (dis->op, "dw");
	sprintf (dis->arg1, "%08lx", (unsigned long) val);
}

static
void disasm_arg (char *dst, uint32_t ir, unsigned arg, uint32_t par)
{
	switch (arg) {
		case ARG_NONE:
			dst[0] = 0;
			break;

		case ARG_RA:
			sprintf (dst, "r%d", p405_get_ir_ra (ir));
			break;

		case ARG_RB:
			sprintf (dst, "r%d", p405_get_ir_rb (ir));
			break;

		case ARG_RS:
			sprintf (dst, "r%d", p405_get_ir_rs (ir));
			break;

		case ARG_RT:
			sprintf (dst, "r%d", p405_get_ir_rt (ir));
			break;

		case ARG_RA0:
			if (p405_get_ir_ra (ir) == 0) {
				strcpy (dst, "0");
			}
			else {
				sprintf (dst, "r%d", p405_get_ir_ra (ir));
			}
			break;

		case ARG_SIMM16:
			sprintf (dst, "%08lx", (unsigned long) p405_sext (ir, 16));
			break;

		case ARG_UIMM16:
			sprintf (dst, "%08lx", (unsigned long) p405_uext (ir, 16));
			break;

		case ARG_IMM16S:
			sprintf (dst, "%08lx", (unsigned long) ((ir & 0xffffUL) << 16));
			break;

		case ARG_UINT3:
			sprintf (dst, "%x", (unsigned) (par & 0x07));
			break;

		case ARG_UINT5:
			sprintf (dst, "%02x", (unsigned) (par & 0x1f));
			break;

		case ARG_UINT8:
			sprintf (dst, "%02x", (unsigned) (par & 0xff));
			break;

		case ARG_UINT16:
			sprintf (dst, "%04x", (unsigned) (par & 0xffffU));
			break;

		case ARG_UINT32:
			sprintf (dst, "%08lx", (unsigned long) (par & 0xffffffffUL));
			break;

		case ARG_CRBIT:
			sprintf (dst, "%s[%u]", p405_cr_name[0][par & 0x03], (unsigned) par / 4);
			break;

		case ARG_SPRN: {
				const char *spr = disasm_get_spr (par);
				if (spr != NULL) {
					strcpy (dst, spr);
				}
				else {
					sprintf (dst, "%03x", par);
				}
			}
			break;

		case ARG_DCRN:
			sprintf (dst, "%03x", par);
			break;
	}
}

static
int disasm_op0 (p405_disasm_t *dis, const char *op, uint32_t opf, uint32_t res)
{
	if (dis->ir & res) {
		disasm_dw (dis, dis->ir);
		return (1);
	}

	strcpy (dis->op, op);

	if (opf & OPF_OE) {
		if (p405_get_ir_oe (dis->ir)) {
			strcat (dis->op, "o");
		}
	}

	if (opf & OPF_RC) {
		if (p405_get_ir_rc (dis->ir)) {
			strcat (dis->op, ".");
		}
	}

	dis->argn = 0;

	return (0);
}

static
int disasm_op1 (p405_disasm_t *dis, const char *op, uint32_t opf, uint32_t res,
	unsigned arg1, uint32_t par1)
{
	if (disasm_op0 (dis, op, opf, res)) {
		return (1);
	}

	dis->argn = 1;
	disasm_arg (dis->arg1, dis->ir, arg1, par1);

	return (0);
}

static
int disasm_op2 (p405_disasm_t *dis, const char *op, uint32_t opf, uint32_t res,
	unsigned arg1, unsigned arg2, uint32_t par1, uint32_t par2)
{
	if (disasm_op1 (dis, op, opf, res, arg1, par1)) {
		return (1);
	}

	dis->argn = 2;
	disasm_arg (dis->arg2, dis->ir, arg2, par2);

	return (0);
}

static
int disasm_op3 (p405_disasm_t *dis, const char *op, uint32_t opf, uint32_t res,
	unsigned arg1, unsigned arg2, unsigned arg3,
	uint32_t par1, uint32_t par2, uint32_t par3)
{
	if (disasm_op2 (dis, op, opf, res, arg1, arg2, par1, par2)) {
		return (1);
	}

	dis->argn = 3;
	disasm_arg (dis->arg3, dis->ir, arg3, par3);

	return (0);
}

static
int disasm_op4 (p405_disasm_t *dis, const char *op, uint32_t opf, uint32_t res,
	unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4,
	uint32_t par1, uint32_t par2, uint32_t par3, uint32_t par4)
{
	if (disasm_op3 (dis, op, opf, res, arg1, arg2, arg3, par1, par2, par3)) {
		return (1);
	}

	dis->argn = 4;
	disasm_arg (dis->arg4, dis->ir, arg4, par4);

	return (0);
}

static
int disasm_op5 (p405_disasm_t *dis, const char *op, uint32_t opf, uint32_t res,
	unsigned arg1, unsigned arg2, unsigned arg3, unsigned arg4, unsigned arg5,
	uint32_t par1, uint32_t par2, uint32_t par3, uint32_t par4, uint32_t par5)
{
	if (disasm_op4 (dis, op, opf, res, arg1, arg2, arg3, arg4, par1, par2, par3, par4)) {
		return (1);
	}

	dis->argn = 5;
	disasm_arg (dis->arg5, dis->ir, arg5, par5);

	return (0);
}

static
void disasm_u32 (char *dst, unsigned long val)
{
	sprintf (dst, "%08lx", val & 0xffffffffUL);
}

static
void disasm_undefined (p405_disasm_t *dis)
{
	disasm_dw (dis, dis->ir);
}

/* crxxx bt, ba, bb */
static
void disasm_crxxx (p405_disasm_t *dis, const char *op)
{
	disasm_op3 (dis, op, 0, 0x01, ARG_CRBIT, ARG_CRBIT, ARG_CRBIT,
		p405_get_ir_rt (dis->ir), p405_get_ir_ra (dis->ir), p405_get_ir_rb (dis->ir)
	);
}


/* branch conditional */
static
void disasm_bc (p405_disasm_t *dis, const char *op, const char *dst,
	unsigned bo, unsigned bi, int aa, int lk)
{
	int  bo0, bo1, bo2, bo3;
	char buf[64];

	bo0 = (bo & 0x10) != 0;
	bo1 = (bo & 0x08) != 0;
	bo2 = (bo & 0x04) != 0;
	bo3 = (bo & 0x02) != 0;

	strcpy (dis->op, "b");

	dis->argn = 0;

	if (bo2 == 0) {
		strcat (dis->op, "d");
	}

	if (bo0 == 0) {
		strcat (dis->op, p405_cr_name[!bo1][bi & 0x03]);

		if (bi > 3) {
			sprintf (buf, "[%u]", bi / 4);
			strcat (dis->op, buf);
		}
	}

	strcat (dis->op, op);

	if (lk) {
		strcat (dis->op, "l");
		dis->flags |= P405_DFLAG_CALL;
	}

	if (aa) {
		strcat (dis->op, "a");
	}

	if (bo2 == 0) {
		if (bo3) {
			strcat (dis->op, "=");
		}
		else {
			strcat (dis->op, ">");
		}
	}

	if (dst != NULL) {
		strcpy (dis->arg1, dst);
		dis->argn += 1;
	}
}


static void opd_ud (p405_disasm_t *dis)
{
	disasm_undefined (dis);
}

/* 03: twi to, ra, simm16 */
static
void opd_03 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "twi", 0, 0, ARG_UINT5, ARG_RA, ARG_SIMM16,
		p405_bits (dis->ir, 6, 5), 0, 0
	);
}

/* 07: mulli rt, ra, simm16 */
static
void opd_07 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "mulli", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 08: subfic rt, ra, simm16 */
static
void opd_08 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "subfic", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 0A: cmpli bf, ra, simm16 */
static
void opd_0a (p405_disasm_t *dis)
{
	disasm_op3 (dis, "cmpli", 0, 0,
		ARG_UINT3, ARG_RA, ARG_SIMM16, (dis->ir >> 23) & 0x07, 0, 0
	);
}

/* 0B: cmpi bf, ra, simm16 */
static
void opd_0b (p405_disasm_t *dis)
{
	disasm_op3 (dis, "cmpi", 0, 0,
		ARG_UINT3, ARG_RA, ARG_SIMM16, (dis->ir >> 23) & 0x07, 0, 0
	);
}

/* 0C: addic rt, ra, simm16 */
static
void opd_0c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "addic", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 0D: addic. rt, ra, simm16 */
static
void opd_0d (p405_disasm_t *dis)
{
	disasm_op3 (dis, "addic.", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 0E: addi rt, ra0, simm16 */
static
void opd_0e (p405_disasm_t *dis)
{
	if (p405_get_ir_ra (dis->ir) == 0) {
		disasm_op2 (dis, "li", 0, 0, ARG_RT, ARG_SIMM16, 0, 0);
	}
	else {
		disasm_op3 (dis, "addi", 0, 0, ARG_RT, ARG_RA0, ARG_SIMM16, 0, 0, 0);
	}
}

/* 0F: addis rt, ra0, simm16 */
static
void opd_0f (p405_disasm_t *dis)
{
	if (p405_get_ir_ra (dis->ir) == 0) {
		disasm_op2 (dis, "lis", 0, 0, ARG_RT, ARG_IMM16S, 0, 0);
	}
	else {
		disasm_op3 (dis, "addis", 0, 0, ARG_RT, ARG_RA0, ARG_IMM16S, 0, 0, 0);
	}
}

/* 10: bc/bcl/bca/bcla target */
static
void opd_10 (p405_disasm_t *dis)
{
	uint32_t bd;
	char     dst[256];

	bd = p405_sext (dis->ir & 0xfffcUL, 16);

	if (p405_get_ir_aa (dis->ir) == 0) {
		bd = (dis->pc + bd) & 0xffffffffUL;
	}

	if (p405_get_ir_lk (dis->ir)) {
		dis->flags |= P405_DFLAG_CALL;
	}

	disasm_u32 (dst, bd);

	disasm_bc (dis, "", dst,
		(dis->ir >> 21) & 0x1f, (dis->ir >> 16) & 0x1f,
		(dis->ir & P405_IR_AA) != 0, (dis->ir & P405_IR_LK) != 0
	);
}

/* 11: sc */
static
void opd_11 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "sc", 0, 0x03fffffdUL);
	dis->flags |= P405_DFLAG_CALL;
}

/* 12: b/bl/ba/bla target */
static
void opd_12 (p405_disasm_t *dis)
{
	uint32_t li;

	strcpy (dis->op, "b");

	li = p405_sext (dis->ir, 26) & 0xfffffffcUL;

	if (p405_get_ir_lk (dis->ir)) {
		strcat (dis->op, "l");
		dis->flags |= P405_DFLAG_CALL;
	}

	if (p405_get_ir_aa (dis->ir)) {
		strcat (dis->op, "a");
		disasm_u32 (dis->arg1, li);
	}
	else {
		disasm_u32 (dis->arg1, dis->pc + li);
	}

	dis->argn = 1;
}

/* 13 000: mcrf bf, bfa */
static
void opd_13_000 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "mcrf", 0, 0x0063f801UL, ARG_UINT3, ARG_UINT3,
		p405_get_ir_rt (dis->ir) >> 2, p405_get_ir_ra (dis->ir) >> 2
	);
}

/* 13 010: bclr/bclrl */
static
void opd_13_010 (p405_disasm_t *dis)
{
	if (p405_get_ir_lk (dis->ir)) {
		dis->flags |= P405_DFLAG_CALL;
	}

	disasm_bc (dis, "lr", NULL,
		(dis->ir >> 21) & 0x1f, (dis->ir >> 16) & 0x1f,
		0, (dis->ir & P405_IR_LK) != 0
	);
}

/* 13 021: crnor bt, ba, bb */
static
void opd_13_021 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "crnor");
}

/* 13 032: rfi */
static
void opd_13_032 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "rfi", 0, 0x03fff801UL);
	dis->flags |= P405_DFLAG_RFI;
}

/* 13 033: rfci */
static
void opd_13_033 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "rfci", 0, 0x03fff801UL);
	dis->flags |= P405_DFLAG_RFI;
}

/* 13 081: crandc bt, ba, bb */
static
void opd_13_081 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "crandc");
}

/* 13 096: isync */
static
void opd_13_096 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "isync", 0, 0);
}

/* 13 0c1: crxor bt, ba, bb */
static
void opd_13_0c1 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "crxor");
}

/* 13 0e1: crnand bt, ba, bb */
static
void opd_13_0e1 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "crnand");
}

/* 13 101: crand bt, ba, bb */
static
void opd_13_101 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "crand");
}

/* 13 121: creqv bt, ba, bb */
static
void opd_13_121 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "creqv");
}

/* 13 1a1: crorc bt, ba, bb */
static
void opd_13_1a1 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "crorc");
}

/* 13 1c1: cror bt, ba, bb */
static
void opd_13_1c1 (p405_disasm_t *dis)
{
	disasm_crxxx (dis, "cror");
}

/* 13 210: bcctr/bcctrl */
static
void opd_13_210 (p405_disasm_t *dis)
{
	if (p405_get_ir_lk (dis->ir)) {
		dis->flags |= P405_DFLAG_CALL;
	}

	disasm_bc (dis, "ctr", NULL,
		(dis->ir >> 21) & 0x1f, (dis->ir >> 16) & 0x1f,
		0, (dis->ir & P405_IR_LK) != 0
	);
}

/* 13: */
static
void opd_13 (p405_disasm_t *dis)
{
	unsigned op2;

	op2 = (dis->ir >> 1) & 0x3ff;

	p405_opd13[op2] (dis);
}

/* 14: rlwimi[.] ra, rs, sh, mb, me */
static
void opd_14 (p405_disasm_t *dis)
{
	disasm_op5 (dis, "rlwimi", OPF_RC, 0,
		ARG_RA, ARG_RS, ARG_UINT5, ARG_UINT5, ARG_UINT5,
		0, 0, (dis->ir >> 11) & 0x1f, (dis->ir >> 6) & 0x1f, (dis->ir >> 1) & 0x1f
	);
}

/* 15: rlwinm[.] ra, rs, sh, mb, me */
static
void opd_15 (p405_disasm_t *dis)
{
	unsigned sh, mb, me;

	sh = (dis->ir >> 11) & 0x1f;
	mb = (dis->ir >> 6) & 0x1f;
	me = (dis->ir >> 1) & 0x1f;

	if ((sh == 0) && (me == 31)) {
		disasm_op3 (dis, "clrlwi", OPF_RC, 0, ARG_RA, ARG_RS, ARG_UINT5, 0, 0, mb);
	}
	else if ((sh == 0) && (mb == 0)) {
		disasm_op3 (dis, "clrrwi", OPF_RC, 0, ARG_RA, ARG_RS, ARG_UINT5, 0, 0, 31 - me);
	}
	else if ((mb == 0) && (me == 31)) {
		disasm_op3 (dis, "rotlwi", OPF_RC, 0, ARG_RA, ARG_RS, ARG_UINT5, 0, 0, sh);
	}
	else if ((mb == 0) && (me == (31 - sh))) {
		disasm_op3 (dis, "slwi", OPF_RC, 0, ARG_RA, ARG_RS, ARG_UINT5, 0, 0, sh);
	}
	else if ((mb == (32 - sh)) && (me == 31)) {
		disasm_op3 (dis, "srwi", OPF_RC, 0, ARG_RA, ARG_RS, ARG_UINT5, 0, 0, 32 - sh);
	}
	else {
		disasm_op5 (dis, "rlwinm", OPF_RC, 0,
			ARG_RA, ARG_RS, ARG_UINT5, ARG_UINT5, ARG_UINT5, 0, 0, sh, mb, me
		);
	}
}

/* 17: rlwnm[.] ra, rs, rb, mb, me */
static
void opd_17 (p405_disasm_t *dis)
{
	unsigned mb, me;

	mb = (dis->ir >> 6) & 0x1f;
	me = (dis->ir >> 1) & 0x1f;

	if ((mb == 0) && (me == 31)) {
		disasm_op3 (dis, "rotlw", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
	}
	else {
		disasm_op5 (dis, "rlwnm", OPF_RC, 0,
			ARG_RA, ARG_RS, ARG_RB, ARG_UINT5, ARG_UINT5, 0, 0, 0, mb, me
		);
	}
}

/* 18: ori ra, rs, uimm16 */
static
void opd_18 (p405_disasm_t *dis)
{
	if ((dis->ir & 0x03ffffffUL) == 0) {
		disasm_op0 (dis, "nop", 0, 0);
	}
	else {
		disasm_op3 (dis, "ori", 0, 0, ARG_RA, ARG_RS, ARG_UIMM16, 0, 0, 0);
	}
}

/* 19: oris ra, rs, uimm16 */
static
void opd_19 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "oris", 0, 0, ARG_RA, ARG_RS, ARG_IMM16S, 0, 0, 0);
}

/* 1A: xori ra, rs, uimm16 */
static
void opd_1a (p405_disasm_t *dis)
{
	disasm_op3 (dis, "xori", 0, 0, ARG_RA, ARG_RS, ARG_UIMM16, 0, 0, 0);
}

/* 1B: xoris ra, rs, uimm16 */
static
void opd_1b (p405_disasm_t *dis)
{
	disasm_op3 (dis, "xoris", 0, 0, ARG_RA, ARG_RS, ARG_IMM16S, 0, 0, 0);
}

/* 1C: andi. ra, rs, uimm16 */
static
void opd_1c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "andi.", 0, 0, ARG_RA, ARG_RS, ARG_UIMM16, 0, 0, 0);
}

/* 1D: andis. ra, rs, uimm16 */
static
void opd_1d (p405_disasm_t *dis)
{
	disasm_op3 (dis, "andis.", 0, 0, ARG_RA, ARG_RS, ARG_IMM16S, 0, 0, 0);
}

/* 1F 000: cmpw bf, ra, rb */
static
void opd_1f_000 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "cmpw", 0, 0,
		ARG_UINT3, ARG_RA, ARG_RB, (dis->ir >> 23) & 0x07, 0, 0
	);
}

/* 1F 008: subfc[o][.] rt, ra, rb */
static
void opd_1f_008 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "subc", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RB, ARG_RA, 0, 0, 0);
}

/* 1F 00A: addc[o][.] rt, ra, rb */
static
void opd_1f_00a (p405_disasm_t *dis)
{
	disasm_op3 (dis, "addc", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 00B: mulhwu[.] rt, ra, rb */
static
void opd_1f_00b (p405_disasm_t *dis)
{
	disasm_op3 (dis, "mulhwu", OPF_RC, 0x00000400UL,
		ARG_RT, ARG_RA, ARG_RB, 0, 0, 0
	);
}

/* 1F 013: mfcr rt */
static
void opd_1f_013 (p405_disasm_t *dis)
{
	disasm_op1 (dis, "mfcr", 0, 0x001ff801UL, ARG_RT, 0);
}

/* 1F 014: lwarx rt, ra0, rb */
static
void opd_1f_014 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lwarx", 0, 0x01, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 017: lwzx rt, ra0, rb */
static
void opd_1f_017 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lwzx", 0, 0x01, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 018: slw[.] ra, rs, rb */
static
void opd_1f_018 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "slw", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 01A: cntlzw[.] ra, rs */
static
void opd_1f_01a (p405_disasm_t *dis)
{
	disasm_op2 (dis, "cntlzw", OPF_RC, 0xf800UL, ARG_RA, ARG_RS, 0, 0);
}

/* 1F 01C: and[.] ra, rs, rb */
static
void opd_1f_01c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "and", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 020: cmpwl bf, ra, rb */
static
void opd_1f_020 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "cmpwl", 0, 0,
		ARG_UINT3, ARG_RA, ARG_RB, (dis->ir >> 23) & 0x07, 0, 0
	);
}

/* 1F 028: subf[o][.] rt, ra, rb */
static
void opd_1f_028 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "sub", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RB, ARG_RA, 0, 0, 0);
}

/* 1F 036: dcbst ra0, rb */
static
void opd_1f_036 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcbst", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 037: lwzux rt, ra, rb */
static
void opd_1f_037 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lwzux", 0, 0x01, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 03C: andc[.] ra, rs, rb */
static
void opd_1f_03c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "andc", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 04B: mulhw[.] rt, ra, rb */
static
void opd_1f_04b (p405_disasm_t *dis)
{
	disasm_op3 (dis, "mulhw", OPF_RC, 0x00000400UL,
		ARG_RT, ARG_RA, ARG_RB, 0, 0, 0
	);
}

/* 1F 053: mfmsr rt */
static
void opd_1f_053 (p405_disasm_t *dis)
{
	disasm_op1 (dis, "mfmsr", 0, 0x001ff801UL, ARG_RT, 0);
}

/* 1F 056: dcbf ra0, rb */
static
void opd_1f_056 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcbf", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 057: lbzx rt, ra0, rb */
static
void opd_1f_057 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lbzx", 0, 0, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 068: neg[o][.] rt, ra */
static
void opd_1f_068 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "neg", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, 0, 0);
}

/* 1F 077: lbzux rt, ra, rb */
static
void opd_1f_077 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lbzux", 0, 0x01, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 07C: nor[.] ra, rs, rb */
static
void opd_1f_07c (p405_disasm_t *dis)
{
	if (p405_get_ir_rs (dis->ir) == p405_get_ir_rb (dis->ir)) {
		disasm_op2 (dis, "not", OPF_RC, 0, ARG_RA, ARG_RS, 0, 0);
	}
	else {
		disasm_op3 (dis, "nor", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
	}
}

/* 1F 083: wrtee rs */
static
void opd_1f_083 (p405_disasm_t *dis)
{
	disasm_op1 (dis, "wrtee", 0, 0x001ff801UL, ARG_RS, 0);
}

/* 1F 086: dcbf ra0, rb */
static
void opd_1f_086 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcbf", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 088: subfe[o][.] rt, ra, rb */
static
void opd_1f_088 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "subfe", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 08A: adde[o][.] rt, ra, rb */
static
void opd_1f_08a (p405_disasm_t *dis)
{
	disasm_op3 (dis, "adde", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 090: mtcrf fxm, rs */
static
void opd_1f_090 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "mtcrf", 0, 0x00100801UL,
		ARG_UINT8, ARG_RS, (dis->ir >> 12) & 0xff, 0
	);
}

/* 1F 092: mtmsr rs */
static
void opd_1f_092 (p405_disasm_t *dis)
{
	disasm_op1 (dis, "mtmsr", 0, 0x001ff801UL, ARG_RS, 0);
}

/* 1F 096: stwcx. rs, ra0, rb */
static
void opd_1f_096 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stwcx.", 0, 0, ARG_RS, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 097: stwx rs, ra0, rb */
static
void opd_1f_097 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stwx", 0, 0x01, ARG_RS, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 0A3: wrteei e */
static
void opd_1f_0a3 (p405_disasm_t *dis)
{
	disasm_op1 (dis, "wrteei", 0, 0x03ff7801UL,
		ARG_UINT3, (dis->ir & 0x8000UL) != 0
	);
}

/* 1F 0B7: stwux rs, ra, rb */
static
void opd_1f_0b7 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stwux", 0, 0x01, ARG_RS, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 0C8: subfze[o][.] rt, ra */
static
void opd_1f_0c8 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "subfze", OPF_OE | OPF_RC, 0xf800UL, ARG_RT, ARG_RA, 0, 0);
}

/* 1F 0CA: addze[o][.] rt, ra */
static
void opd_1f_0ca (p405_disasm_t *dis)
{
	disasm_op2 (dis, "addze", OPF_OE | OPF_RC, 0xf800UL, ARG_RT, ARG_RA, 0, 0);
}

/* 1F 0D7: stbx rs, ra0, rb */
static
void opd_1f_0d7 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stbx", 0, 0x01, ARG_RS, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 0E8: subfme[o][.] rt, ra */
static
void opd_1f_0e8 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "subfme", OPF_OE | OPF_RC, 0xf800UL, ARG_RT, ARG_RA, 0, 0);
}

/* 1F 0EA: addme[o][.] rt, ra */
static
void opd_1f_0ea (p405_disasm_t *dis)
{
	disasm_op2 (dis, "addme", OPF_OE | OPF_RC, 0xf800UL, ARG_RT, ARG_RA, 0, 0);
}

/* 1F 0EB: mullw[o][.] rt, ra, rb */
static
void opd_1f_0eb (p405_disasm_t *dis)
{
	disasm_op3 (dis, "mullw", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 0F6: dcbtst ra0, rb */
static
void opd_1f_0f6 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcbtst", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 0F7: stbux rs, ra, rb */
static
void opd_1f_0f7 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stbux", 0, 0x01, ARG_RS, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 106: icbt ra, rb */
static
void opd_1f_106 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "icbt", 0, 0x03e00001UL, ARG_RA, ARG_RB, 0, 0);
}

/* 1F 10A: add[o][.] rt, ra, rb */
static
void opd_1f_10a (p405_disasm_t *dis)
{
	disasm_op3 (dis, "add", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 116: dcbt ra0, rb */
static
void opd_1f_116 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcbt", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 117: lhzx rt, ra0, rb */
static
void opd_1f_117 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhzx", 0, 0, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 11C: eqv[.] ra, rs, rb */
static
void opd_1f_11c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "eqv", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 137: lhzux rt, ra, rb */
static
void opd_1f_137 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhzux", 0, 0x01, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 13C: xor[.] ra, rs, rb */
static
void opd_1f_13c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "xor", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 143: mfdcr rt, dcrn */
static
void opd_1f_143 (p405_disasm_t *dis)
{
	unsigned dcrf, dcrn;

	dcrf = (dis->ir >> 11) & 0x3ff;
	dcrn = ((dcrf & 0x1f) << 5) | ((dcrf >> 5) & 0x1f);

	disasm_op2 (dis, "mfdcr", 0, 0x01, ARG_RT, ARG_DCRN, 0, dcrn);
}

/* 1F 153: mfspr rt, sprn */
static
void opd_1f_153 (p405_disasm_t *dis)
{
	unsigned sprf, sprn;

	sprf = (dis->ir >> 11) & 0x3ff;
	sprn = ((sprf & 0x1f) << 5) | ((sprf >> 5) & 0x1f);

	disasm_op2 (dis, "mfspr", 0, 0x01, ARG_RT, ARG_SPRN, 0, sprn);
}

/* 1F 157: lhax rt, ra0, rb */
static
void opd_1f_157 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhax", 0, 0, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 172: tlbia */
static
void opd_1f_172 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "tlbia", 0, 0x03fff801UL);
}

/* 1F 173: mftb rt, tbrn */
static
void opd_1f_173 (p405_disasm_t *dis)
{
	unsigned tbrf, tbrn;

	tbrf = (dis->ir >> 11) & 0x3ff;
	tbrn = ((tbrf & 0x1f) << 5) | ((tbrf >> 5) & 0x1f);

	switch (tbrn) {
		case P405_TBRN_TBL:
			disasm_op1 (dis, "mftb", 0, 0x01, ARG_RT, 0);
			break;

		case P405_TBRN_TBU:
			disasm_op1 (dis, "mftbu", 0, 0x01, ARG_RT, 0);
			break;

		default:
			disasm_op2 (dis, "mftb", 0, 0x01, ARG_RT, ARG_UINT16, 0, tbrn);
			break;
	}
}

/* 1F 177: lhaux rt, ra0, rb */
static
void opd_1f_177 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhaux", 0, 0, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 197: sthx rs, ra0, rb */
static
void opd_1f_197 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "sthx", 0, 0x01, ARG_RS, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 19C: orc[.] ra, rs, rb */
static
void opd_1f_19c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "orc", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 1B7: sthux rs, ra, rb */
static
void opd_1f_1b7 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "sthux", 0, 0x01, ARG_RS, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 1BC: or[.] ra, rs, rb */
static
void opd_1f_1bc (p405_disasm_t *dis)
{
	if (p405_get_ir_rs (dis->ir) == p405_get_ir_rb (dis->ir)) {
		disasm_op2 (dis, "mr", OPF_RC, 0, ARG_RA, ARG_RS, 0, 0);
	}
	else {
		disasm_op3 (dis, "or", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
	}
}

/* 1F 1C3: mtdcr dcrn, rs */
static
void opd_1f_1c3 (p405_disasm_t *dis)
{
	unsigned dcrf, dcrn;

	dcrf = (dis->ir >> 11) & 0x3ff;
	dcrn = ((dcrf & 0x1f) << 5) | ((dcrf >> 5) & 0x1f);

	disasm_op2 (dis, "mtdcr", 0, 0x01, ARG_DCRN, ARG_RS, dcrn, 0);
}

/* 1F 1C6: dccci ra0, rb */
static
void opd_1f_1c6 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dccci", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 1CB: divwu[o][.] rt, ra, rb */
static
void opd_1f_1cb (p405_disasm_t *dis)
{
	disasm_op3 (dis, "divwu", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 1D3: mtspr sprn, rs */
static
void opd_1f_1d3 (p405_disasm_t *dis)
{
	unsigned sprf, sprn;

	sprf = (dis->ir >> 11) & 0x3ff;
	sprn = ((sprf & 0x1f) << 5) | ((sprf >> 5) & 0x1f);

	disasm_op2 (dis, "mtspr", 0, 0x01, ARG_SPRN, ARG_RS, sprn, 0);
}

/* 1F 1D6: dcbi ra0, rb */
static
void opd_1f_1d6 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcbi", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 1DC: nand[.] ra, rs, rb */
static
void opd_1f_1dc (p405_disasm_t *dis)
{
	disasm_op3 (dis, "nand", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 1EB: divw[o][.] rt, ra, rb */
static
void opd_1f_1eb (p405_disasm_t *dis)
{
	disasm_op3 (dis, "divw", OPF_OE | OPF_RC, 0, ARG_RT, ARG_RA, ARG_RB, 0, 0, 0);
}

/* 1F 200: mcrxr bf */
static
void opd_1f_200 (p405_disasm_t *dis)
{
	disasm_op1 (dis, "mcrxr", 0, 0x007ff801UL, ARG_UINT3,
		p405_get_ir_rt (dis->ir) >> 2
	);
}

/* 1F 215: lswx rt, ra0, rb */
static
void opd_1f_215 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lswx", 0, 0x01, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 216: lwbrx rt, ra0, rb */
static
void opd_1f_216 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lwbrx", 0, 0x01, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 218: srw[.] ra, rs, rb */
static
void opd_1f_218 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "srw", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 236: tlbsync */
static
void opd_1f_236 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "tlbsync", 0, 0x03fff801UL);
}

/* 1F 255: lswi rt, ra0, nb */
static
void opd_1f_255 (p405_disasm_t *dis)
{
	unsigned nb;

	nb = p405_get_ir_rb (dis->ir);
	nb = (nb == 0) ? 32 : nb;

	disasm_op3 (dis, "lswi", 0, 0x01, ARG_RT, ARG_RA0, ARG_UINT8, 0, 0, nb);
}

/* 1F 256: sync */
static
void opd_1f_256 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "sync", 0, 0);
}

/* 1F 295: stswx rs, ra0, rb */
static
void opd_1f_295 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stswx", 0, 0x01, ARG_RS, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 296: stwbrx rs, ra0, rb */
static
void opd_1f_296 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stwbrx", 0, 0x01, ARG_RS, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 2D5: stswi rs, ra0, nb */
static
void opd_1f_2d5 (p405_disasm_t *dis)
{
	unsigned nb;

	nb = p405_get_ir_rb (dis->ir);
	nb = (nb == 0) ? 32 : nb;

	disasm_op3 (dis, "stswi", 0, 0x01, ARG_RS, ARG_RA0, ARG_UINT8, 0, 0, nb);
}

/* 1F 2F6: dcba ra0, rb */
static
void opd_1f_2f6 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcba", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F 316: lhbrx rt, ra0, rb */
static
void opd_1f_316 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhbrx", 0, 0, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 318: sraw[.] ra, rs, rb */
static
void opd_1f_318 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "sraw", OPF_RC, 0, ARG_RA, ARG_RS, ARG_RB, 0, 0, 0);
}

/* 1F 338: srawi[.] ra, rs, sh */
static
void opd_1f_338 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "srawi", OPF_RC, 0,
		ARG_RA, ARG_RS, ARG_UINT5, 0, 0, p405_get_ir_rb (dis->ir)
	);
}

/* 1F 356: eieio */
static
void opd_1f_356 (p405_disasm_t *dis)
{
	disasm_op0 (dis, "eieio", 0, 0);
}

/* 1F 392: tlbsx rt, ra0, rb */
static
void opd_1f_392 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "tlbsx", OPF_RC, 0, ARG_RT, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 396: sthbrx rs, ra0, rb */
static
void opd_1f_396 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "sthbrx", 0, 0x01, ARG_RS, ARG_RA0, ARG_RB, 0, 0, 0);
}

/* 1F 39A: extsh[.] ra, rs */
static
void opd_1f_39a (p405_disasm_t *dis)
{
	disasm_op2 (dis, "extsh", OPF_RC, 0, ARG_RA, ARG_RS, 0, 0);
}

/* 1F 3B2: tlbre rt, ra, ws */
static
void opd_1f_3b2 (p405_disasm_t *dis)
{
	switch (p405_get_ir_rb (dis->ir)) {
		case 0:
			disasm_op2 (dis, "tlbrehi", 0, 0x01, ARG_RT, ARG_RA, 0, 0);
			break;

		case 1:
			disasm_op2 (dis, "tlbrelo", 0, 0x01, ARG_RT, ARG_RA, 0, 0);
			break;

		default:
			disasm_undefined (dis);
			break;
	}
}

/* 1F 3BA: extsb[.] ra, rs */
static
void opd_1f_3ba (p405_disasm_t *dis)
{
	disasm_op2 (dis, "extsb", OPF_RC, 0, ARG_RA, ARG_RS, 0, 0);
}

/* 1F 3C6: iccci ra, rb */
static
void opd_1f_3c6 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "iccci", 0, 0x03e00001UL, ARG_RA, ARG_RB, 0, 0);
}

/* 1F 3D2: tlbwe rs, ra, ws */
static
void opd_1f_3d2 (p405_disasm_t *dis)
{
	switch (p405_get_ir_rb (dis->ir)) {
		case 0:
			disasm_op2 (dis, "tlbwehi", 0, 0x01, ARG_RS, ARG_RA, 0, 0);
			break;

		case 1:
			disasm_op2 (dis, "tlbwelo", 0, 0x01, ARG_RS, ARG_RA, 0, 0);
			break;

		default:
			disasm_undefined (dis);
			break;
	}
}

/* 1F 3D6: icbi ra, rb */
static
void opd_1f_3d6 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "icbi", 0, 0x03e00001UL, ARG_RA, ARG_RB, 0, 0);
}

/* 1F 3F6: dcbz ra0, rb */
static
void opd_1f_3f6 (p405_disasm_t *dis)
{
	disasm_op2 (dis, "dcbz", 0, 0x03e00001UL, ARG_RA0, ARG_RB, 0, 0);
}

/* 1F: */
static
void opd_1f (p405_disasm_t *dis)
{
	unsigned op2;

	op2 = (dis->ir >> 1) & 0x3ff;

	p405_opd1f[op2] (dis);
}

/* 20: lwz rt, ra0, simm16 */
static
void opd_20 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lwz", 0, 0, ARG_RT, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 21: lwzu rt, ra, simm16 */
static
void opd_21 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lwzu", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 22: lbz rt, ra0, simm16 */
static
void opd_22 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lbz", 0, 0, ARG_RT, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 23: lbzu rt, ra, simm16 */
static
void opd_23 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lbzu", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 24: stw rs, ra0, simm16 */
static
void opd_24 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stw", 0, 0, ARG_RS, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 25: stwu rs, ra, simm16 */
static
void opd_25 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stwu", 0, 0, ARG_RS, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 26: stb rs, ra0, simm16 */
static
void opd_26 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stb", 0, 0, ARG_RS, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 27: stbu rs, ra, simm16 */
static
void opd_27 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stbu", 0, 0, ARG_RS, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 28: lhz rt, ra0, simm16 */
static
void opd_28 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhz", 0, 0, ARG_RT, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 29: lhzu rt, ra, simm16 */
static
void opd_29 (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhzu", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 2A: lha rt, ra0, simm16 */
static
void opd_2a (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lha", 0, 0, ARG_RT, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 2B: lhau rt, ra, simm16 */
static
void opd_2b (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lhau", 0, 0, ARG_RT, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 2C: sth rs, ra0, simm16 */
static
void opd_2c (p405_disasm_t *dis)
{
	disasm_op3 (dis, "sth", 0, 0, ARG_RS, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 2D: sthu rs, ra, simm16 */
static
void opd_2d (p405_disasm_t *dis)
{
	disasm_op3 (dis, "sthu", 0, 0, ARG_RS, ARG_RA, ARG_SIMM16, 0, 0, 0);
}

/* 2E: lmw rt, ra0, simm16 */
static
void opd_2e (p405_disasm_t *dis)
{
	disasm_op3 (dis, "lmw", 0, 0, ARG_RT, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

/* 2F: stmw rs, ra0, simm16 */
static
void opd_2f (p405_disasm_t *dis)
{
	disasm_op3 (dis, "stmw", 0, 0, ARG_RS, ARG_RA0, ARG_SIMM16, 0, 0, 0);
}

void p405_disasm (p405_disasm_t *dis, uint32_t pc, uint32_t ir)
{
	unsigned op;

	dis->flags = 0;

	dis->pc = pc;
	dis->ir = ir;

	op = (ir >> 26) & 0x3f;

	p405_dis[op] (dis);
}

void p405_disasm_mem (p405_t *c, p405_disasm_t *dis, uint32_t pc, unsigned xlat)
{
	uint32_t ir;

	dis->flags = 0;

	if (p405_get_xlat32 (c, pc, xlat, &ir)) {
		dis->pc = pc;
		dis->ir = 0xffffffffUL;

		disasm_op0 (dis, "TLB_MISS", 0, 0);

		dis->flags |= P405_DFLAG_TLBM;
	}
	else {
		p405_disasm (dis, pc, ir);
	}
}

static
p405_disasm_f p405_opd13[1024] = {
	&opd_13_000,     &opd_ud,     &opd_ud,     &opd_ud, /* 000 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_13_010,     &opd_ud,     &opd_ud,     &opd_ud, /* 010 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_021,     &opd_ud,     &opd_ud, /* 020 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_13_032, &opd_13_033, /* 030 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 040 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 050 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 060 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 070 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_081,     &opd_ud,     &opd_ud, /* 080 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 090 */
			&opd_ud,     &opd_ud, &opd_13_096,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0b0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_0c1,     &opd_ud,     &opd_ud, /* 0c0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0d0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_0e1,     &opd_ud,     &opd_ud, /* 0e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0f0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_101,     &opd_ud,     &opd_ud, /* 100 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 110 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_121,     &opd_ud,     &opd_ud, /* 120 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 130 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 140 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 150 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 160 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 170 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 180 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 190 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_1a1,     &opd_ud,     &opd_ud, /* 1a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1b0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_13_1c1,     &opd_ud,     &opd_ud, /* 1c0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1d0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1f0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 200 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_13_210,     &opd_ud,     &opd_ud,     &opd_ud, /* 210 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 220 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 230 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 240 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 250 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 260 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 270 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 280 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 290 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2b0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2c0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2d0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2f0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 300 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 310 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 320 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 330 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 340 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 350 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 360 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 370 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 380 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 390 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3b0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3c0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3d0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3f0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
};

static
p405_disasm_f p405_opd1f[1024] = {
	&opd_1f_000,     &opd_ud,     &opd_ud,     &opd_ud, /* 000 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_008,     &opd_ud, &opd_1f_00a, &opd_1f_00b,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_013, /* 010 */
	&opd_1f_014,     &opd_ud,     &opd_ud, &opd_1f_017,
	&opd_1f_018,     &opd_ud, &opd_1f_01a,     &opd_ud,
	&opd_1f_01c,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_020,     &opd_ud,     &opd_ud,     &opd_ud, /* 020 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_028,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 030 */
			&opd_ud,     &opd_ud, &opd_1f_036, &opd_1f_037,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_03c,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 040 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_04b,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_053, /* 050 */
			&opd_ud,     &opd_ud, &opd_1f_056, &opd_1f_057,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 060 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_068,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 070 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_077,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_07c,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_083, /* 080 */
			&opd_ud,     &opd_ud, &opd_1f_086,     &opd_ud,
	&opd_1f_088,     &opd_ud, &opd_1f_08a,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_090,     &opd_ud, &opd_1f_092,     &opd_ud, /* 090 */
			&opd_ud,     &opd_ud, &opd_1f_096, &opd_1f_097,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_0a3, /* 0a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0b0 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_0b7,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0c0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_0c8,     &opd_ud, &opd_1f_0ca,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0d0 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_0d7,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_0e8,     &opd_ud, &opd_1f_0ea, &opd_1f_0eb,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 0f0 */
			&opd_ud,     &opd_ud, &opd_1f_0f6, &opd_1f_0f7,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 100 */
			&opd_ud,     &opd_ud, &opd_1f_106,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_10a,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 110 */
			&opd_ud,     &opd_ud, &opd_1f_116, &opd_1f_117,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_11c,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 120 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 130 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_137,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_13c,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_143, /* 140 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_153, /* 150 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_157,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 160 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_172, &opd_1f_173, /* 170 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_177,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 180 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 190 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_197,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_19c,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1b0 */
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_1b7,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_1bc,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_1c3, /* 1c0 */
			&opd_ud,     &opd_ud, &opd_1f_1c6,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_1cb,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_1d3, /* 1d0 */
			&opd_ud,     &opd_ud, &opd_1f_1d6,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_1dc,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_1eb,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 1f0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_200,     &opd_ud,     &opd_ud,     &opd_ud, /* 200 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_008,     &opd_ud, &opd_1f_00a,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 210 */
			&opd_ud, &opd_1f_215, &opd_1f_216,     &opd_ud,
	&opd_1f_218,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 220 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_028,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 230 */
			&opd_ud,     &opd_ud, &opd_1f_236,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 240 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 250 */
			&opd_ud, &opd_1f_255, &opd_1f_256,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 260 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_068,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 270 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 280 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_088,     &opd_ud, &opd_1f_08a,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 290 */
			&opd_ud, &opd_1f_295, &opd_1f_296,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2b0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2c0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_0c8,     &opd_ud, &opd_1f_0ca,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud, &opd_1f_2d5,     &opd_ud,     &opd_ud, /* 2d0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_0e8,     &opd_ud, &opd_1f_0ea, &opd_1f_0eb,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 2f0 */
			&opd_ud,     &opd_ud, &opd_1f_2f6,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 300 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_10a,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 310 */
			&opd_ud,     &opd_ud, &opd_1f_316,     &opd_ud,
	&opd_1f_318,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 320 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 330 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
	&opd_1f_338,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 340 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 350 */
			&opd_ud,     &opd_ud, &opd_1f_356,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 360 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 370 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 380 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_392,     &opd_ud, /* 390 */
			&opd_ud,     &opd_ud, &opd_1f_396,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_39a,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3a0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_3b2,     &opd_ud, /* 3b0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_3ba,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3c0 */
			&opd_ud,     &opd_ud, &opd_1f_3c6,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_1cb,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud, &opd_1f_3d2,     &opd_ud, /* 3d0 */
			&opd_ud,     &opd_ud, &opd_1f_3d6,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3e0 */
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud, &opd_1f_1eb,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud, /* 3f0 */
			&opd_ud,     &opd_ud, &opd_1f_3f6,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud,
			&opd_ud,     &opd_ud,     &opd_ud,     &opd_ud
};

static
p405_disasm_f p405_dis[64] = {
	&opd_ud, &opd_ud, &opd_ud, &opd_03, &opd_ud, &opd_ud, &opd_ud, &opd_07, /* 00 */
	&opd_08, &opd_ud, &opd_0a, &opd_0b, &opd_0c, &opd_0d, &opd_0e, &opd_0f,
	&opd_10, &opd_11, &opd_12, &opd_13, &opd_14, &opd_15, &opd_ud, &opd_17, /* 10 */
	&opd_18, &opd_19, &opd_1a, &opd_1b, &opd_1c, &opd_1d, &opd_ud, &opd_1f,
	&opd_20, &opd_21, &opd_22, &opd_23, &opd_24, &opd_25, &opd_26, &opd_27, /* 20 */
	&opd_28, &opd_29, &opd_2a, &opd_2b, &opd_2c, &opd_2d, &opd_2e, &opd_2f,
	&opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud, /* 30 */
	&opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud, &opd_ud
};
