/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/e68000/disasm.c                                      *
 * Created:     2005-07-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2013 Hampa Hug <hampa@hampa.ch>                     *
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


typedef void (*e68_dasm_f) (e68_dasm_t *da, const uint8_t *src);


static e68_dasm_f e68_dasm_op[1024];


enum {
	ARG_NONE,
	ARG_IR,
	ARG_IR1,
	ARG_EA,
	ARG_EA2,
	ARG_DREG0,
	ARG_DREG9,
	ARG_AREG0,
	ARG_AREG9,
	ARG_REG_IR1_12,
	ARG_AREG0PI,
	ARG_AREG9PI,
	ARG_AREG0PD,
	ARG_AREG9PD,
	ARG_AREG0DISP,
	ARG_IMM8,
	ARG_IMM16,
	ARG_IMM32,
	ARG_IMMQ,
	ARG_IRUIMM4,
	ARG_IRSIMM8,
	ARG_DIST8,
	ARG_DIST16,
	ARG_DIST32,
	ARG_REGLST,
	ARG_REVLST,
	ARG_CCR,
	ARG_SR,
	ARG_USP,
	ARG_CREG,
	ARG_DQDR,
	ARG_DHDL,
	ARG_DL,
	ARG_BF,
	ARG_BF_DN
};


static const char *op_bcc[16] = {
	"BRA", "BSR", "BHI", "BLS",
	"BCC", "BCS", "BNE", "BEQ",
	"BVC", "BVS", "BPL", "BMI",
	"BGE", "BLT", "BGT", "BLE"
};

static const char *op_dbcc[16] = {
	"DBT",  "DBF",  "DBHI", "DBLS",
	"DBCC", "DBCS", "DBNE", "DBEQ",
	"DBVC", "DBVS", "DBPL", "DBMI",
	"DBGE", "DBLT", "DBGT", "DBLE"
};

static const char *op_scc[16] = {
	 "ST",  "SF", "SHI", "SLS",
	"SCC", "SCS", "SNE", "SEQ",
	"SVC", "SVS", "SPL", "SMI",
	"SGE", "SLT", "SGT", "SLE"
};

static const char *op_trapcc[16] = {
	 "TRAPT",  "TRAPF", "TRAPHI", "TRAPLS",
	"TRAPCC", "TRAPCS", "TRAPNE", "TRAPEQ",
	"TRAPVC", "TRAPVS", "TRAPPL", "TRAPMI",
	"TRAPGE", "TRAPLT", "TRAPGT", "TRAPLE"
};

static const char *hex_prefix = "$";


static
void dasm_reginterval (char *dst, unsigned i, unsigned n, int *first)
{
	const char *reg;
	char       tmp[32];

	if (i < 8) {
		reg = "D";
	}
	else {
		reg = "A";
		i -= 8;
	}

	if (*first == 0) {
		strcat (dst, "/");
	}

	if (n > 2) {
		sprintf (tmp, "%s%u-%s%u", reg, i, reg, i + n - 1);
	}
	else if (n == 2) {
		sprintf (tmp, "%s%u/%s%u", reg, i, reg, i + 1);
	}
	else {
		sprintf (tmp, "%s%u", reg, i);
	}

	strcat (dst, tmp);

	*first = 0;
}

static
void dasm_reglst (char *dst, uint16_t list, int rev)
{
	unsigned i;
	unsigned ri, rn;
	int      first = 1;

	*dst = 0;

	if (rev) {
		uint16_t t = list;

		for (i = 0; i < 16; i++) {
			list = (list << 1) | (t & 1);
			t = t >> 1;
		}
	}

	ri = 0;
	rn = 0;

	for (i = 0; i < 16; i++) {
		if ((i == 8) && (rn > 0)) {
			dasm_reginterval (dst, ri, rn, &first);
			rn = 0;
		}

		if (list & (1U << i)) {
			if (rn == 0) {
				ri = i;
				rn = 1;
			}
			else {
				rn += 1;
			}
		}
		else {
			if (rn > 0) {
				dasm_reginterval (dst, ri, rn, &first);
				rn = 0;
			}
		}
	}

	if (rn > 0) {
		dasm_reginterval (dst, ri, rn, &first);
	}
}

static
void dasm_creg (e68_dasm_t *da, char *dst, uint16_t reg)
{
	const char *str;

	switch (reg & 0x0fff) {
	case 0x000:
		str = "SFC";
		da->flags |= E68_DFLAG_68010;
		break;

	case 0x001:
		str = "DFC";
		da->flags |= E68_DFLAG_68010;
		break;

	case 0x800:
		str = "USP";
		da->flags |= E68_DFLAG_68010;
		break;

	case 0x801:
		str = "VBR";
		da->flags |= E68_DFLAG_68010;
		break;

	default:
		sprintf (dst, "CR(%u)", reg & 0x0fff);
		da->flags |= E68_DFLAG_68010;
		return;
	}

	strcpy (dst, str);
}

static inline
unsigned dasm_next_word (e68_dasm_t *da, const uint8_t *src)
{
	unsigned val;

	val = e68_get_uint16 (src, 2 * da->irn);

	da->irn += 1;

	return (val);
}

static
void dasm_ea_full (e68_dasm_t *da, char *dst, const uint8_t *src)
{
	uint16_t ext;
	uint32_t v;
	char     br[16], ix[16], bd[16], od[16], scale[16];
	int      bs, is;

	ext = da->ir[1];

	bs = (ext & 0x0080) != 0;
	is = (ext & 0x0040) != 0;

	if (bs) {
		strcpy (br, "-");
	}
	else {
		if ((da->ir[0] & 0x38) == 0x38) {
			strcpy (br, "PC");
		}
		else {
			sprintf (br, "A%u", da->ir[0] & 0x07);
		}
	}

	if (is) {
		strcpy (ix, "-");
	}
	else {
		sprintf (ix, "%s%u%s",
			(ext & 0x8000) ? "A" : "D",
			(ext >> 12) & 7,
			(ext & 0x0800) ? ".L" : ".W"
		);
	}

	if (((ext >> 9) & 3) == 0) {
		strcpy (scale, "");
	}
	else {
		sprintf (scale, "*%u", 1 << ((ext >> 9) & 3));
	}

	switch ((ext >> 4) & 3) {
	case 0:
	case 1:
		strcpy (bd, "-");
		break;

	case 2:
		v = dasm_next_word (da, src);
		sprintf (bd, "%s%s%04X",
			(v & 0x8000) ? "-" : "",
			hex_prefix,
			(v & 0x8000) ? ((~v + 1) & 0xffff) : v
		);
		break;

	case 3:
		v = dasm_next_word (da, src);
		v = (v << 16) | dasm_next_word (da, src);
		sprintf (bd, "%s%s%08X",
			(v & 0x80000000) ? "-" : "",
			hex_prefix,
			(v & 0x80000000) ? ((~v + 1) & 0xffffffff) : v
		);
		break;
	}

	switch (ext & 3) {
	case 0:
	case 1:
		strcpy (od, "-");
		break;
	case 2:
		v = dasm_next_word (da, src);
		sprintf (od, "%s%s%04X",
			(v & 0x8000) ? "-" : "",
			hex_prefix,
			(v & 0x8000) ? ((~v + 1) & 0xffff) : v
		);
		break;

	case 3:
		v = dasm_next_word (da, src);
		v = (v << 16) | dasm_next_word (da, src);
		sprintf (od, "%s%s%08X",
			(v & 0x80000000) ? "-" : "",
			hex_prefix,
			(v & 0x80000000) ? ((~v + 1) & 0xffffffff) : v
		);
		break;
	}

	if (ext & 4) {
		sprintf (dst, "([%s, %s], %s%s, %s)", br, bd, ix, scale, od);
	}
	else {
		sprintf (dst, "([%s, %s, %s%s], %s)", br, bd, ix, scale, od);
	}

	da->flags |= E68_DFLAG_68020;
}

static
void dasm_ea (e68_dasm_t *da, char *dst, const uint8_t *src, unsigned ea, unsigned size)
{
	unsigned      v;
	unsigned long v1, v2;

	switch ((ea >> 3) & 7) {
	case 0x00: /* %Dx */
		sprintf (dst, "D%u", ea & 7);
		break;

	case 0x01: /* %Ax */
		sprintf (dst, "A%u", ea & 7);
		break;

	case 0x02: /* (%Ax) */
		sprintf (dst, "(A%u)", ea & 7);
		break;

	case 0x03: /* (%Ax)+ */
		sprintf (dst, "(A%u)+", ea & 7);
		break;

	case 0x04: /* -(%Ax) */
		sprintf (dst, "-(A%u)", ea & 7);
		break;

	case 0x05: /* XXXX(%Ax) */
		v = dasm_next_word (da, src);
		sprintf (dst, "%s%s%04X(A%u)",
			(v & 0x8000) ? "-" : "",
			hex_prefix,
			((v & 0x8000) ? (~v + 1) : v) & 0xffff,
			ea & 7
		);
		break;

	case 0x06: /* XX(%Ax, Rx.S) */
		v = dasm_next_word (da, src);
		if (v & 0x0100) {
			dasm_ea_full (da, dst, src);
		}
		else {
			sprintf (dst, "%s%s%02X(A%u, %s%u%s*%u)",
				(v & 0x80) ? "-" : "",
				hex_prefix,
				((v & 0x80) ? (~v + 1) : v) & 0xff,
				ea & 7,
				(v & 0x8000) ? "A" : "D",
				(v >> 12) & 7,
				(v & 0x0800) ? ".L" : ".W",
				1 << ((v >> 9) & 3)
			);

			if (v & 0x0600) {
				da->flags |= E68_DFLAG_68020;
			}
		}
		break;

	case 0x07:
		switch (ea & 7) {
		case 0x00: /* XXXX */
			v = dasm_next_word (da, src);
			sprintf (dst, "%s%s%04X",
				(v & 0x8000) ? "-" : "",
				hex_prefix,
				((v & 0x8000) ? (~v + 1) : v) & 0xffff
			);
			break;

		case 0x01: /* XXXXXXXX */
			v1 = dasm_next_word (da, src);
			v2 = dasm_next_word (da, src);
			sprintf (dst, "%s%08lX",
				hex_prefix,
				(v1 << 16) | v2
			);
			break;

		case 0x02: /* XXXX(PC) */
			v = dasm_next_word (da, src);
			v1 = (da->pc + e68_exts16 (v) + 2) & 0xffffffff;
			sprintf (dst, "%s%08lX(PC)", hex_prefix, v1);
			break;

		case 0x03: /* XX(PC, Rx.S) */
			v = dasm_next_word (da, src);
			if (v & 0x0100) {
				dasm_ea_full (da, dst, src);
			}
			else {
				sprintf (dst, "%s%s%02X(PC, %s%u%s*%u)",
					(v & 0x80) ? "-" : "",
					hex_prefix,
					((v & 0x80) ? (~v + 1) : v) & 0xff,
					(v & 0x8000) ? "A" : "D",
					(v >> 12) & 7,
					(v & 0x0800) ? ".L" : ".W",
					1 << ((v >> 9) & 3)
				);
				if (v & 0x0600) {
					da->flags |= E68_DFLAG_68020;
				}
			}
			break;

		case 0x04: /* #XXXX */
			if (size == 8) {
				v = dasm_next_word (da, src) & 0xff;
				sprintf (dst, "#%s%02X", hex_prefix, v);
			}
			else if (size == 16) {
				v = dasm_next_word (da, src);
				sprintf (dst, "#%s%04X", hex_prefix, v);
			}
			else if (size == 32) {
				v1 = dasm_next_word (da, src);
				v2 = dasm_next_word (da, src);
				sprintf (dst, "#%s%08lX",
					hex_prefix,
					(v1 << 16) | v2
				);
			}

			break;

		default:
			sprintf (dst, "<EA>(%02X)", ea & 0x3f);
			break;
		}
		break;

	default:
		sprintf (dst, "<EA>(%02X)", ea & 0x3f);
		break;
	}
}

static
void dasm_arg_bitfield (e68_dasm_t *da, char *dst, const uint8_t *src)
{
	unsigned width;

	da->ir[da->irn] = dasm_next_word (da, src);

	dasm_ea (da, dst, src, src[1] & 0x3f, 8);
	dst += strlen (dst);

	*(dst++) = ' ';
	*(dst++) = '{';

	if ((da->ir[1] >> 11) & 1) {
		sprintf (dst, "D%u", (da->ir[1] >> 6) & 7);
	}
	else {
		sprintf (dst, "%u", (da->ir[1] >> 6) & 31);
	}

	dst += strlen (dst);

	*(dst++) = ':';

	width = (da->ir[1] & 31);

	if (width == 0) {
		width = 32;
	}

	if ((da->ir[1] >> 5) & 1) {
		sprintf (dst, "D%u", width & 7);
	}
	else {
		sprintf (dst, "%u", width);
	}

	dst += strlen (dst);

	strcpy (dst, "}");
}

static
void dasm_arg (e68_dasm_t *da, char *dst, const uint8_t *src, unsigned arg, unsigned size)
{
	uint16_t val16;
	uint32_t val32;

	switch (arg) {
	case ARG_NONE:
		dst[0] = 0;
		break;

	case ARG_IR:
		sprintf (dst, "#%s%04X",
			hex_prefix,
			(unsigned) e68_get_uint16 (src, 0)
		);
		break;

	case ARG_IR1:
		sprintf (dst, "#%s%04X",
			hex_prefix,
			(unsigned) e68_get_uint16 (src, 2)
		);
		break;

	case ARG_EA:
		dasm_ea (da, dst, src, src[1] & 0x3f, size);
		break;

	case ARG_EA2:
		val16 = (src[0] << 8) | src[1];
		dasm_ea (da, dst, src,
			((val16 >> 3) & 0x38) | ((val16 >> 9) & 7), size
		);
		break;

	case ARG_DREG0:
		sprintf (dst, "D%u", da->ir[0] & 7);
		break;

	case ARG_DREG9:
		sprintf (dst, "D%u", (unsigned) ((src[0] >> 1) & 7));
		break;

	case ARG_AREG0:
		sprintf (dst, "A%u", da->ir[0] & 7);
		break;

	case ARG_AREG9:
		sprintf (dst, "A%u", (unsigned) ((src[0] >> 1) & 7));
		break;

	case ARG_REG_IR1_12:
		sprintf (dst, "%s%u", (src[2] & 0x80) ? "A" : "D",
			(unsigned) ((src[2] >> 4) & 7)
		);
		break;

	case ARG_AREG0PI:
		sprintf (dst, "(A%u)+", (unsigned) (da->ir[0] & 7));
		break;

	case ARG_AREG9PI:
		sprintf (dst, "(A%u)+", (unsigned) ((da->ir[0] >> 9) & 7));
		break;

	case ARG_AREG0PD:
		sprintf (dst, "-(A%u)", (unsigned) (da->ir[0] & 7));
		break;

	case ARG_AREG9PD:
		sprintf (dst, "-(A%u)", (unsigned) ((da->ir[0] >> 9) & 7));
		break;

	case ARG_AREG0DISP:
		val16 = e68_get_uint16 (src, 2);
		sprintf (dst, "%s%s%04X(A%u)",
			(val16 & 0x8000) ? "-" : "",
			hex_prefix,
			((val16 & 0x8000) ? (~val16 + 1) : val16) & 0xffff,
			(unsigned) (da->ir[0] & 7)
		);
		da->irn += 1;
		break;

	case ARG_IMM8:
		sprintf (dst, "#%s%02X", hex_prefix, (unsigned) src[3]);
		da->irn += 1;
		break;

	case ARG_IMM16:
		val16 = e68_get_uint16 (src, 2);
		sprintf (dst, "#%s%04X", hex_prefix, (unsigned) val16);
		da->irn += 1;
		break;

	case ARG_IMM32:
		val32 = e68_get_uint32 (src, 2);
		sprintf (dst, "#%s%08lX", hex_prefix, (unsigned long) val32);
		da->irn += 2;
		break;

	case ARG_IMMQ:
		val16 = (src[0] >> 1) & 7;
		val16 = (val16 == 0) ? 8 : val16;
		sprintf (dst, "#%s%X", hex_prefix, (unsigned) val16);
		break;

	case ARG_IRUIMM4:
		sprintf (dst, "#%s%02X",
			hex_prefix,
			(unsigned) (da->ir[0] & 0x0f)
		);
		break;

	case ARG_IRSIMM8:
		sprintf (dst, "#%s%08lX",
			hex_prefix,
			(unsigned long) e68_exts8 (da->ir[0])
		);
		break;

	case ARG_DIST8:
		sprintf (dst, "%s%08lX",
			hex_prefix,
			da->pc + 2UL + e68_exts8 (src[1])
		);
		break;

	case ARG_DIST16:
		val16 = e68_get_uint16 (src, 2 * da->irn);
		da->irn += 1;
		sprintf (dst, "%s%08lX",
			hex_prefix,
			da->pc + 2UL + e68_exts16 (val16)
		);
		break;

	case ARG_DIST32:
		val32 = e68_get_uint32 (src, 2 * da->irn);
		da->irn += 2;
		sprintf (dst, "%s%08lX",
			hex_prefix,
			da->pc + 2UL + val32
		);
		break;

	case ARG_REGLST:
		dasm_reglst (dst, e68_get_uint16 (src, 2), 0);
		da->irn += 1;
		break;

	case ARG_REVLST:
		dasm_reglst (dst, e68_get_uint16 (src, 2), 1);
		da->irn += 1;
		break;

	case ARG_CCR:
		strcpy (dst, "CCR");
		break;

	case ARG_SR:
		strcpy (dst, "SR");
		break;

	case ARG_USP:
		strcpy (dst, "USP");
		break;

	case ARG_CREG:
		dasm_creg (da, dst, e68_get_uint16 (src, 2));
		break;

	case ARG_DQDR:
		sprintf (dst, "D%u:D%u", da->ir[1] & 7, (da->ir[1] >> 12) & 7);
		break;

	case ARG_DHDL:
		sprintf (dst, "D%u:D%u", da->ir[1] & 7, (da->ir[1] >> 12) & 7);
		break;

	case ARG_DL:
	case ARG_BF_DN:
		sprintf (dst, "D%u", (da->ir[1] >> 12) & 7);
		break;

	case ARG_BF:
		dasm_arg_bitfield (da, dst, src);
		break;

	default:
		dst[0] = 0;
		break;
	}
}

static
int dasm_op0 (e68_dasm_t *da, const char *op)
{
	strcpy (da->op, op);

	da->argn = 0;

	return (0);
}

static
int dasm_op1 (e68_dasm_t *da, const char *op, const uint8_t *src,
	unsigned arg1, unsigned size)
{
	strcpy (da->op, op);

	da->argn = 1;
	dasm_arg (da, da->arg1, src, arg1, size);

	return (0);
}

static
int dasm_op2 (e68_dasm_t *da, const char *op, const uint8_t *src,
	unsigned arg1, unsigned arg2, unsigned size)
{
	strcpy (da->op, op);

	da->argn = 2;
	dasm_arg (da, da->arg1, src, arg1, size);
	dasm_arg (da, da->arg2, src, arg2, size);

	return (0);
}

static
int dasm_op2r (e68_dasm_t *da, const char *op, const uint8_t *src,
	unsigned arg1, unsigned arg2, unsigned size)
{
	strcpy (da->op, op);

	da->argn = 2;
	dasm_arg (da, da->arg2, src, arg2, size);
	dasm_arg (da, da->arg1, src, arg1, size);

	return (0);
}


/* 0000: ORI.B #XX, <EA> */
static void d_0000 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x003f) == 0x003c) {
		dasm_op2 (da, "ORI.B", src, ARG_IMM8, ARG_CCR, 8);
	}
	else {
		dasm_op2 (da, "ORI.B", src, ARG_IMM8, ARG_EA, 8);
	}
}

/* 0040: ORI.W #XXXX, <EA> */
static void d_0040 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x003f) == 0x003c) {
		dasm_op2 (da, "ORI.W", src, ARG_IMM16, ARG_SR, 16);
		da->flags |= E68_DFLAG_PRIV;
	}
	else {
		dasm_op2 (da, "ORI.W", src, ARG_IMM16, ARG_EA, 16);
	}
}

/* 0080: ORI.L #XXXXXXXX, <EA> */
static void d_0080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ORI.L", src, ARG_IMM32, ARG_EA, 32);
}

/* 00C0: CHK2.B <EA>, Rn */
static void d_00c0 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned ir1;

	ir1 = dasm_next_word (da, src);

	if (ir1 & 0x0800) {
		dasm_op2 (da, "CHK2.B", src, ARG_EA, ARG_REG_IR1_12, 8);
	}
	else {
		dasm_op2 (da, "CMP2.B", src, ARG_EA, ARG_REG_IR1_12, 8);
	}

	da->flags |= E68_DFLAG_68020;
}

/* 0100: misc */
static void d_0100 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x01:
		dasm_op2 (da, "MOVEP.W", src, ARG_AREG0DISP, ARG_DREG9, 16);
		break;

	default:
		dasm_op2 (da, "BTST", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* 0140: misc */
static void d_0140 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x01:
		dasm_op2 (da, "MOVEP.L", src, ARG_AREG0DISP, ARG_DREG9, 32);
		break;

	default:
		dasm_op2 (da, "BCHG", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* 0180: misc */
static void d_0180 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x01:
		dasm_op2 (da, "MOVEP.W", src, ARG_DREG9, ARG_AREG0DISP, 16);
		break;

	default:
		dasm_op2 (da, "BCLR", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* 01C0: misc */
static void d_01c0 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x01:
		dasm_op2 (da, "MOVEP.L", src, ARG_DREG9, ARG_AREG0DISP, 32);
		break;

	default:
		dasm_op2 (da, "BSET", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* 0200: ANDI.B #XX, EA */
static void d_0200 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x003f) == 0x003c) {
		dasm_op2 (da, "ANDI.B", src, ARG_IMM8, ARG_CCR, 8);
	}
	else {
		dasm_op2 (da, "ANDI.B", src, ARG_IMM8, ARG_EA, 8);
	}
}

/* 0240: ANDI.W #XXXX, EA */
static void d_0240 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x003f) == 0x003c) {
		dasm_op2 (da, "ANDI.W", src, ARG_IMM16, ARG_SR, 16);
		da->flags |= E68_DFLAG_PRIV;
	}
	else {
		dasm_op2 (da, "ANDI.W", src, ARG_IMM16, ARG_EA, 16);
	}
}

/* 0280: ANDI.L #XXXXXXXX, <EA> */
static void d_0280 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ANDI.L", src, ARG_IMM32, ARG_EA, 32);
}

/* 02C0: CHK2.W <EA>, Rn */
static void d_02c0 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned ir1;

	ir1 = dasm_next_word (da, src);

	if (ir1 & 0x0800) {
		dasm_op2 (da, "CHK2.W", src, ARG_EA, ARG_REG_IR1_12, 16);
	}
	else {
		dasm_op2 (da, "CMP2.W", src, ARG_EA, ARG_REG_IR1_12, 16);
	}

	da->flags |= E68_DFLAG_68020;
}

/* 0400: SUBI.B #XX, <EA> */
static void d_0400 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBI.B", src, ARG_IMM8, ARG_EA, 8);
}

/* 0440: SUBI.W #XXXX, <EA> */
static void d_0440 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBI.W", src, ARG_IMM16, ARG_EA, 16);
}

/* 0480: ADDI.L #XXXXXXXX, <EA> */
static void d_0480 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBI.L", src, ARG_IMM32, ARG_EA, 32);
}

/* 04C0: CHK2.L <EA>, Rn */
static void d_04c0 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned ir1;

	ir1 = dasm_next_word (da, src);

	if (ir1 & 0x0800) {
		dasm_op2 (da, "CHK2.L", src, ARG_EA, ARG_REG_IR1_12, 32);
	}
	else {
		dasm_op2 (da, "CMP2.L", src, ARG_EA, ARG_REG_IR1_12, 32);
	}

	da->flags |= E68_DFLAG_68020;
}

/* 0600: ADDI.B #XX, <EA> */
static void d_0600 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDI.B", src, ARG_IMM8, ARG_EA, 8);
}

/* 0640: ADDI.W #XXXX, <EA> */
static void d_0640 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDI.W", src, ARG_IMM16, ARG_EA, 16);
}

/* 0680: ADDI.L #XXXXXXXX, <EA> */
static void d_0680 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDI.L", src, ARG_IMM32, ARG_EA, 32);
}

/* 0800: BTST #XX, <EA> */
static void d_0800 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "BTST", src, ARG_IMM8, ARG_EA, 8);
}

/* 0840: BCHG #XX, <EA> */
static void d_0840 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "BCHG", src, ARG_IMM8, ARG_EA, 8);
}

/* 0880: BCLR #XX, <EA> */
static void d_0880 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "BCLR", src, ARG_IMM8, ARG_EA, 8);
}

/* 08C0: BSET #XX, <EA> */
static void d_08c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "BSET", src, ARG_IMM8, ARG_EA, 8);
}

/* 0A00: EORI.B #XX, <EA> */
static void d_0a00 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x003f) == 0x003c) {
		dasm_op2 (da, "EORI.B", src, ARG_IMM8, ARG_CCR, 8);
	}
	else {
		dasm_op2 (da, "EORI.B", src, ARG_IMM8, ARG_EA, 8);
	}
}

/* 0A40: EORI.W #XXXX, <EA> */
static void d_0a40 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x003f) == 0x003c) {
		dasm_op2 (da, "EORI.W", src, ARG_IMM16, ARG_SR, 16);
		da->flags |= E68_DFLAG_PRIV;
	}
	else {
		dasm_op2 (da, "EORI.W", src, ARG_IMM16, ARG_EA, 16);
	}
}

/* 0A80: EORI.L #XXXXXXXX, <EA> */
static void d_0a80 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "EORI.L", src, ARG_IMM32, ARG_EA, 32);
}

/* 0C00: CMPI.B #XX, <EA> */
static void d_0c00 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMPI.B", src, ARG_IMM8, ARG_EA, 8);
}

/* 0C40: CMPI.W #XXXX, <EA> */
static void d_0c40 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMPI.W", src, ARG_IMM16, ARG_EA, 16);
}

/* 0C80: CMPI.L #XXXXXXXX, EA */
static void d_0c80 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMPI.L", src, ARG_IMM32, ARG_EA, 32);
}

/* 0E00: MOVS.B Rn, <EA> / MOVES.B <EA>, Rn*/
static void d_0e00 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned ir1;

	ir1 = dasm_next_word (da, src);

	if (ir1 & 0x0800) {
		dasm_op2 (da, "MOVS.B", src, ARG_REG_IR1_12, ARG_EA, 8);
	}
	else {
		dasm_op2 (da, "MOVS.B", src, ARG_EA, ARG_REG_IR1_12, 8);
	}

	da->flags |= E68_DFLAG_68010;
}

/* 0E40: MOVS.W Rn, <EA> / MOVES.W <EA>, Rn*/
static void d_0e40 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned ir1;

	ir1 = dasm_next_word (da, src);

	if (ir1 & 0x0800) {
		dasm_op2 (da, "MOVS.W", src, ARG_REG_IR1_12, ARG_EA, 16);
	}
	else {
		dasm_op2 (da, "MOVS.W", src, ARG_EA, ARG_REG_IR1_12, 16);
	}

	da->flags |= E68_DFLAG_68010;
}

/* 0E80: MOVS.L Rn, <EA> / MOVES.W <EA>, Rn*/
static void d_0e80 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned ir1;

	ir1 = dasm_next_word (da, src);

	if (ir1 & 0x0800) {
		dasm_op2 (da, "MOVS.L", src, ARG_REG_IR1_12, ARG_EA, 32);
	}
	else {
		dasm_op2 (da, "MOVS.L", src, ARG_EA, ARG_REG_IR1_12, 32);
	}

	da->flags |= E68_DFLAG_68010;
}

/* 1000: MOVE.B EA, EA */
static void d_1000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVE.B", src, ARG_EA, ARG_EA2, 8);
}

/* 2000: MOVE.L EA, EA */
static void d_2000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVE.L", src, ARG_EA, ARG_EA2, 32);
}

/* 2040: MOVEA.L EA, Ax */
static void d_2040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVEA.L", src, ARG_EA, ARG_AREG9, 32);
}

/* 3000: MOVE.W EA, EA */
static void d_3000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVE.W", src, ARG_EA, ARG_EA2, 16);
}

/* 3040: MOVEA.W EA, Ax */
static void d_3040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVEA.W", src, ARG_EA, ARG_AREG9, 16);
}

/* 4000: NEGX.B <EA> */
static void d_4000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NEGX.B", src, ARG_EA, 8);
	da->flags |= E68_DFLAG_DEP_CC;
}

/* 4040: NEGX.W <EA> */
static void d_4040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NEGX.W", src, ARG_EA, 16);
	da->flags |= E68_DFLAG_DEP_CC;
}

/* 4080: NEGX.L <EA> */
static void d_4080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NEGX.L", src, ARG_EA, 32);
	da->flags |= E68_DFLAG_DEP_CC;
}

/* 40C0: MOVE.W SR, <EA> */
static void d_40c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVE.W", src, ARG_SR, ARG_EA, 16);
}

/* 4180: CHK <EA>, Dx */
static void d_4180 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CHK", src, ARG_EA, ARG_DREG9, 16);
}

/* 41C0: LEA EA, Ax */
static void d_41c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "LEA", src, ARG_EA, ARG_AREG9, 32);
}

/* 4200: CLR.B <EA> */
static void d_4200 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "CLR.B", src, ARG_EA, 8);
}

/* 4240: CLR.W <EA> */
static void d_4240 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "CLR.W", src, ARG_EA, 16);
}

/* 4280: CLR.L <EA> */
static void d_4280 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "CLR.L", src, ARG_EA, 32);
}

/* 42C0: MOVE.W CCR, <EA> */
static void d_42c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVE.W", src, ARG_CCR, ARG_EA, 16);
	da->flags |= E68_DFLAG_68010;
}

/* 4400: NEG.B <EA> */
static void d_4400 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NEG.B", src, ARG_EA, 8);
}

/* 4440: NEG.W <EA> */
static void d_4440 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NEG.W", src, ARG_EA, 16);
}

/* 4480: NEG.L <EA> */
static void d_4480 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NEG.L", src, ARG_EA, 32);
}

/* 44C0: MOVE.W <EA>, CCR */
static void d_44c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVE.W", src, ARG_EA, ARG_CCR, 16);
}

/* 4600: NOT.B <EA> */
static void d_4600 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NOT.B", src, ARG_EA, 8);
}

/* 4640: NOT.W <EA> */
static void d_4640 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NOT.W", src, ARG_EA, 16);
}

/* 4680: NOT.L <EA> */
static void d_4680 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NOT.L", src, ARG_EA, 32);
}

/* 46C0: MOVE.W <EA>, SR */
static void d_46c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVE.W", src, ARG_EA, ARG_SR, 16);
	da->flags |= E68_DFLAG_PRIV;
}

/* 4800: NBCD.B <EA> */
static void d_4800 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "NBCD.B", src, ARG_EA, 8);
	da->flags |= E68_DFLAG_DEP_CC;
}

/* 4840: misc */
static void d_4840 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op1 (da, "SWAP", src, ARG_DREG0, 32);
		break;

	default:
		dasm_op1 (da, "PEA", src, ARG_EA, 32);
		break;
	}
}

/* 4880: misc */
static void d_4880 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op1 (da, "EXT.W", src, ARG_DREG0, 16);
		break;

	case 0x04:
		dasm_op2 (da, "MOVEM.W", src, ARG_REVLST, ARG_EA, 16);
		break;

	default:
		dasm_op2 (da, "MOVEM.W", src, ARG_REGLST, ARG_EA, 16);
		break;
	}
}

/* 48C0: misc */
static void d_48c0 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op1 (da, "EXT.L", src, ARG_DREG0, 32);
		break;

	case 0x04:
		dasm_op2 (da, "MOVEM.L", src, ARG_REVLST, ARG_EA, 32);
		break;

	default:
		dasm_op2 (da, "MOVEM.L", src, ARG_REGLST, ARG_EA, 32);
		break;
	}
}

/* 49C0: misc */
static void d_49c0 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op1 (da, "EXTB.L", src, ARG_DREG0, 32);
		da->flags |= E68_DFLAG_68020;
		break;

	default:
		d_41c0 (da, src);
		break;
	}
}

/* 4A00: TST.B <EA> */
static void d_4a00 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "TST.B", src, ARG_EA, 8);
}

/* 4A40: TST.W <EA> */
static void d_4a40 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "TST.W", src, ARG_EA, 16);
}

/* 4A80: TST.L <EA> */
static void d_4a80 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "TST.L", src, ARG_EA, 32);
}

/* 4AC0: misc */
static void d_4ac0 (e68_dasm_t *da, const uint8_t *src)
{
	if (da->ir[0] == 0x4afc) {
		dasm_op0 (da, "ILLEGAL");
		da->flags |= E68_DFLAG_CALL;
	}
	else {
		dasm_op1 (da, "TAS", src, ARG_EA, 8);
	}
}

/* 4C00: MULU.L <EA>, [Dh:]Dl */
static void d_4c00 (e68_dasm_t *da, const uint8_t *src)
{
	da->ir[1] = dasm_next_word (da, src);

	if (da->ir[1] & 0x0400) {
		dasm_op2 (da, "MULU.L", src, ARG_EA, ARG_DHDL, 32);
	}
	else {
		dasm_op2 (da, "MULU.L", src, ARG_EA, ARG_DL, 32);
	}

	da->flags |= E68_DFLAG_68020;
}

/* 4C40: DIVU[L].L <EA>, Dq:Dr */
static void d_4c40 (e68_dasm_t *da, const uint8_t *src)
{
	da->ir[1] = dasm_next_word (da, src);

	if (da->ir[1] & 0x0400) {
		dasm_op2 (da, "DIVUL.L", src, ARG_EA, ARG_DQDR, 32);
	}
	else {
		dasm_op2 (da, "DIVU.L", src, ARG_EA, ARG_DQDR, 32);
	}

	da->flags |= E68_DFLAG_68020;
}

/* 4C80: misc */
static void d_4c80 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;

	default:
		dasm_op2r (da, "MOVEM.W", src, ARG_EA, ARG_REGLST, 16);
		break;
	}
}

/* 4CC0: misc */
static void d_4cc0 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;

	default:
		dasm_op2r (da, "MOVEM.L", src, ARG_EA, ARG_REGLST, 32);
		break;
	}
}

/* 4E40: misc */
static void d_4e40 (e68_dasm_t *da, const uint8_t *src)
{
	if (da->ir[0] == 0x4e70) {
		dasm_op0 (da, "RESET");
		da->flags |= E68_DFLAG_PRIV;
	}
	else if (da->ir[0] == 0x4e71) {
		dasm_op0 (da, "NOP");
	}
	else if (da->ir[0] == 0x4e72) {
		dasm_op1 (da, "STOP", src, ARG_IMM16, 16);
		da->flags |= E68_DFLAG_PRIV;
	}
	else if (da->ir[0] == 0x4e73) {
		dasm_op0 (da, "RTE");
		da->flags |= E68_DFLAG_PRIV;
		da->flags |= E68_DFLAG_RTE;
	}
	else if (da->ir[0] == 0x4e74) {
		dasm_op1 (da, "RTD", src, ARG_IMM16, 16);
		da->flags |= E68_DFLAG_RTS;
		da->flags |= E68_DFLAG_68010;
	}
	else if (da->ir[0] == 0x4e75) {
		dasm_op0 (da, "RTS");
		da->flags |= E68_DFLAG_RTS;
	}
	else if (da->ir[0] == 0x4e76) {
		dasm_op0 (da, "TRAPV");
		da->flags |= E68_DFLAG_DEP_CC;
	}
	else if (da->ir[0] == 0x4e77) {
		dasm_op0 (da, "RTR");
	}
	else if (da->ir[0] == 0x4e7a) {
		dasm_next_word (da, src);
		dasm_op2 (da, "MOVEC", src, ARG_CREG, ARG_REG_IR1_12, 16);
		da->flags |= E68_DFLAG_68010;
	}
	else if (da->ir[0] == 0x4e7b) {
		dasm_next_word (da, src);
		dasm_op2 (da, "MOVEC", src, ARG_REG_IR1_12, ARG_CREG, 16);
		da->flags |= E68_DFLAG_68010;
	}
	else if ((da->ir[0] & 0x0030) == 0x0000) {
		dasm_op1 (da, "TRAP", src, ARG_IRUIMM4, 0);
	}
	else if ((da->ir[0] & 0x0038) == 0x0010) {
		dasm_op2 (da, "LINK", src, ARG_AREG0, ARG_IMM16, 16);
	}
	else if ((da->ir[0] & 0x0038) == 0x0018) {
		dasm_op1 (da, "UNLK", src, ARG_AREG0, 0);
	}
	else if ((da->ir[0] & 0x0038) == 0x0020) {
		dasm_op2 (da, "MOVE", src, ARG_AREG0, ARG_USP, 0);
		da->flags |= E68_DFLAG_PRIV;
	}
	else if ((da->ir[0] & 0x0038) == 0x0028) {
		dasm_op2 (da, "MOVE", src, ARG_USP, ARG_AREG0, 0);
		da->flags |= E68_DFLAG_PRIV;
	}
	else {
		dasm_op1 (da, "DW", src, ARG_IR, 16);
	}
}

/* 4E80: JSR <EA> */
static void d_4e80 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "JSR", src, ARG_EA, 0);
	da->flags |= E68_DFLAG_CALL;
}

/* 4EC0: JMP <EA> */
static void d_4ec0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "JMP", src, ARG_EA, 0);
	da->flags |= E68_DFLAG_JUMP;
}

/* 5000: ADDQ.B #X, <EA> */
static void d_5000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDQ.B", src, ARG_IMMQ, ARG_EA, 8);
}

/* 5040: ADDQ.W #X, <EA> */
static void d_5040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDQ.W", src, ARG_IMMQ, ARG_EA, 16);
}

/* 5080: ADDQ.L #X, <EA> */
static void d_5080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDQ.L", src, ARG_IMMQ, ARG_EA, 32);
}

/* 5100: SUBQ.B #X, <EA> */
static void d_5100 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBQ.B", src, ARG_IMMQ, ARG_EA, 8);
}

/* 5140: SUBQ.W #X, <EA> */
static void d_5140 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBQ.W", src, ARG_IMMQ, ARG_EA, 16);
}

/* 5180: SUBQ.L #X, <EA> */
static void d_5180 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBQ.L", src, ARG_IMMQ, ARG_EA, 32);
}

/* 50C0: misc */
static void d_50c0 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned c, op;

	c = (da->ir[0] >> 8) & 0x0f;
	op = da->ir[0] & 0x3f;

	if (c > 1) {
		da->flags |= E68_DFLAG_DEP_CC;
	}

	if (op == 0x3a) {
		/* TRAPcc.W */
		dasm_op1 (da, op_trapcc[c], src, ARG_IMM16, 0);
		da->flags |= E68_DFLAG_68020;
	}
	else if (op == 0x3b) {
		/* TRAPcc.L */
		dasm_op1 (da, op_trapcc[c], src, ARG_IMM32, 0);
		da->flags |= E68_DFLAG_68020;
	}
	else if (op == 0x3c) {
		/* TRAPcc */
		dasm_op0 (da, op_trapcc[c]);
		da->flags |= E68_DFLAG_68020;
	}
	else if ((op & 0x38) == 0x08) {
		/* DBcc */
		dasm_op2 (da, op_dbcc[c], src, ARG_DREG0, ARG_DIST16, 0);
		da->flags |= E68_DFLAG_JUMP;
	}
	else {
		/* Scc */
		dasm_op1 (da, op_scc[c], src, ARG_EA, 8);
	}
}

/* 6000: Bcc dist */
static void d_6000 (e68_dasm_t *da, const uint8_t *src)
{
	unsigned c;

	c = (da->ir[0] >> 8) & 15;

	if (src[1] == 0) {
		dasm_op1 (da, op_bcc[c], src, ARG_DIST16, 0);
		strcat (da->op, ".W");
	}
	else if (src[1] == 0xff) {
		dasm_op1 (da, op_bcc[c], src, ARG_DIST32, 0);
		strcat (da->op, ".L");
		da->flags |= E68_DFLAG_68020;
	}
	else {
		dasm_op1 (da, op_bcc[c], src, ARG_DIST8, 0);
		strcat (da->op, ".S");
	}

	if (c == 1) {
		da->flags |= E68_DFLAG_CALL;
	}
	else {
		da->flags |= E68_DFLAG_JUMP;
	}

	if (c > 1) {
		da->flags |= E68_DFLAG_DEP_CC;
	}
}

/* 7000: MOVEQ #XX, Dx */
static void d_7000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MOVEQ", src, ARG_IRSIMM8, ARG_DREG9, 0);
}

/* 8000: OR.B <EA>, Dx */
static void d_8000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "OR.B", src, ARG_EA, ARG_DREG9, 8);
}

/* 8040: OR.W <EA>, Dx */
static void d_8040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "OR.W", src, ARG_EA, ARG_DREG9, 16);
}

/* 8080: OR.L <EA>, Dx */
static void d_8080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "OR.L", src, ARG_EA, ARG_DREG9, 32);
}

/* 80C0: DIVU.W <EA>, Dx */
static void d_80c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "DIVU.W", src, ARG_EA, ARG_DREG9, 16);
}

/* 8100: misc */
static void d_8100 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "SBCD.B", src, ARG_DREG0, ARG_DREG9, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "SBCD.B", src, ARG_AREG0PD, ARG_AREG9PD, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "OR.B", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* 8140: misc */
static void d_8140 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;

	default:
		dasm_op2 (da, "OR.W", src, ARG_DREG9, ARG_EA, 16);
		break;
	}
}

/* 8180: misc */
static void d_8180 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
	case 0x01:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;

	default:
		dasm_op2 (da, "OR.L", src, ARG_DREG9, ARG_EA, 32);
		break;
	}
}

/* 81C0: DIVS.W <EA>, Dx */
static void d_81c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "DIVS.W", src, ARG_EA, ARG_DREG9, 16);
}

/* 9000: SUB.B <EA>, Dx */
static void d_9000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUB.B", src, ARG_EA, ARG_DREG9, 8);
}

/* 9040: SUB.W <EA>, Dx */
static void d_9040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUB.W", src, ARG_EA, ARG_DREG9, 16);
}

/* 9080: SUB.L <EA>, Dx */
static void d_9080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUB.L", src, ARG_EA, ARG_DREG9, 32);
}

/* 90C0: SUBA.W <EA>, Ax */
static void d_90c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBA.W", src, ARG_EA, ARG_AREG9, 16);
}

/* 9100: misc */
static void d_9100 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "SUBX.B", src, ARG_DREG0, ARG_DREG9, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "SUBX.B", src, ARG_AREG0PD, ARG_AREG9PD, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "SUB.B", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* 9140: misc */
static void d_9140 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "SUBX.W", src, ARG_DREG0, ARG_DREG9, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "SUBX.W", src, ARG_AREG0PD, ARG_AREG9PD, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "SUB.W", src, ARG_DREG9, ARG_EA, 16);
		break;
	}
}

/* 9180: SUB.L Dx, <EA> */
static void d_9180 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "SUBX.L", src, ARG_DREG0, ARG_DREG9, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "SUBX.L", src, ARG_AREG0PD, ARG_AREG9PD, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "SUB.L", src, ARG_DREG9, ARG_EA, 32);
		break;
	}
}

/* 91C0: SUBA.L <EA>, Ax */
static void d_91c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "SUBA.L", src, ARG_EA, ARG_AREG9, 32);
}

/* A000: AXXX */
static void d_a000 (e68_dasm_t *da, const uint8_t *src)
{
	char buf[16];

	sprintf (buf, "%04X", da->ir[0]);

	dasm_op0 (da, buf);

	da->flags |= E68_DFLAG_CALL;
}

/* B000: CMP.B <EA>, Dx */
static void d_b000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMP.B", src, ARG_EA, ARG_DREG9, 8);
}

/* B040: CMP.W <EA>, Dx */
static void d_b040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMP.W", src, ARG_EA, ARG_DREG9, 16);
}

/* B080: CMP.L <EA>, Dx */
static void d_b080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMP.L", src, ARG_EA, ARG_DREG9, 32);
}

/* B0C0: CMPA.W <EA>, Ax */
static void d_b0c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMPA.W", src, ARG_EA, ARG_AREG9, 16);
}

/* B100: misc */
static void d_b100 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x0038) == 0x0008) {
		dasm_op2 (da, "CMPM.B", src, ARG_AREG0PI, ARG_AREG9PI, 8);
	}
	else {
		dasm_op2 (da, "EOR.B", src, ARG_DREG9, ARG_EA, 8);
	}
}

/* B140: misc */
static void d_b140 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x0038) == 0x0008) {
		dasm_op2 (da, "CMPM.W", src, ARG_AREG0PI, ARG_AREG9PI, 16);
	}
	else {
		dasm_op2 (da, "EOR.W", src, ARG_DREG9, ARG_EA, 16);
	}
}

/* B180: misc */
static void d_b180 (e68_dasm_t *da, const uint8_t *src)
{
	if ((da->ir[0] & 0x0038) == 0x0008) {
		dasm_op2 (da, "CMPM.L", src, ARG_AREG0PI, ARG_AREG9PI, 32);
	}
	else {
		dasm_op2 (da, "EOR.L", src, ARG_DREG9, ARG_EA, 32);
	}
}

/* B1C0: CMPA.L <EA>, Ax */
static void d_b1c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "CMPA.L", src, ARG_EA, ARG_AREG9, 32);
}

/* C000: AND.B EA, Dx */
static void d_c000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "AND.B", src, ARG_EA, ARG_DREG9, 8);
}

/* C040: AND.W EA, Dx */
static void d_c040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "AND.W", src, ARG_EA, ARG_DREG9, 16);
}

/* C080: AND.L EA, Dx */
static void d_c080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "AND.L", src, ARG_EA, ARG_DREG9, 32);
}

/* C0C0: MULU.W <EA>, Dx */
static void d_c0c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MULU.W", src, ARG_EA, ARG_DREG9, 16);
}

/* C100: misc */
static void d_c100 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ABCD.B", src, ARG_DREG0, ARG_DREG9, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "ABCD.B", src, ARG_AREG0PD, ARG_AREG9PD, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "AND.B", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* C140: misc */
static void d_c140 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "EXG", src, ARG_DREG9, ARG_DREG0, 32);
		break;

	case 0x01:
		dasm_op2 (da, "EXG", src, ARG_AREG9, ARG_AREG0, 32);
		break;

	default:
		dasm_op2 (da, "AND.W", src, ARG_DREG9, ARG_EA, 16);
		break;
	}
}

/* C180: misc */
static void d_c180 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;

	case 0x01:
		dasm_op2 (da, "EXG", src, ARG_DREG9, ARG_AREG0, 32);
		break;

	default:
		dasm_op2 (da, "AND.L", src, ARG_DREG9, ARG_EA, 32);
		break;
	}
}

/* C1C0: MULS.W <EA>, Dx */
static void d_c1c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "MULS.W", src, ARG_EA, ARG_DREG9, 16);
}

/* D000: ADD.B EA, Dx */
static void d_d000 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADD.B", src, ARG_EA, ARG_DREG9, 8);
}

/* D040: ADD.W EA, Dx */
static void d_d040 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADD.W", src, ARG_EA, ARG_DREG9, 16);
}

/* D080: ADD.L EA, Dx */
static void d_d080 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADD.L", src, ARG_EA, ARG_DREG9, 32);
}

/* D0C0: ADDA.W <EA>, Ax */
static void d_d0c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDA.W", src, ARG_EA, ARG_AREG9, 16);
}

/* D100: misc */
static void d_d100 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ADDX.B", src, ARG_DREG0, ARG_DREG9, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "ADDX.B", src, ARG_AREG0PD, ARG_AREG9PD, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "ADD.B", src, ARG_DREG9, ARG_EA, 8);
		break;
	}
}

/* D140: ADD.W Dx, EA */
static void d_d140 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ADDX.W", src, ARG_DREG0, ARG_DREG9, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "ADDX.W", src, ARG_AREG0PD, ARG_AREG9PD, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "ADD.W", src, ARG_DREG9, ARG_EA, 16);
		break;
	}
}

/* D180: ADD.L Dx, EA */
static void d_d180 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ADDX.L", src, ARG_DREG0, ARG_DREG9, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x01:
		dasm_op2 (da, "ADDX.L", src, ARG_AREG0PD, ARG_AREG9PD, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	default:
		dasm_op2 (da, "ADD.L", src, ARG_DREG9, ARG_EA, 32);
		break;
	}
}

/* D1C0: ADDA.L EA, Ax */
static void d_d1c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "ADDA.L", src, ARG_EA, ARG_AREG9, 32);
}

/* E000: misc */
static void d_e000 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ASR.B", src, ARG_IMMQ, ARG_DREG0, 8);
		break;

	case 0x01:
		dasm_op2 (da, "LSR.B", src, ARG_IMMQ, ARG_DREG0, 8);
		break;

	case 0x02:
		dasm_op2 (da, "ROXR.B", src, ARG_IMMQ, ARG_DREG0, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x03:
		dasm_op2 (da, "ROR.B", src, ARG_IMMQ, ARG_DREG0, 8);
		break;

	case 0x04:
		dasm_op2 (da, "ASR.B", src, ARG_DREG9, ARG_DREG0, 8);
		break;

	case 0x05:
		dasm_op2 (da, "LSR.B", src, ARG_DREG9, ARG_DREG0, 8);
		break;

	case 0x06:
		dasm_op2 (da, "ROXR.B", src, ARG_DREG9, ARG_DREG0, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x07:
		dasm_op2 (da, "ROR.B", src, ARG_DREG9, ARG_DREG0, 8);
		break;

	default:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;
	}
}

/* E040: misc */
static void d_e040 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ASR.W", src, ARG_IMMQ, ARG_DREG0, 16);
		break;

	case 0x01:
		dasm_op2 (da, "LSR.W", src, ARG_IMMQ, ARG_DREG0, 16);
		break;

	case 0x02:
		dasm_op2 (da, "ROXR.W", src, ARG_IMMQ, ARG_DREG0, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x03:
		dasm_op2 (da, "ROR.W", src, ARG_IMMQ, ARG_DREG0, 16);
		break;

	case 0x04:
		dasm_op2 (da, "ASR.W", src, ARG_DREG9, ARG_DREG0, 16);
		break;

	case 0x05:
		dasm_op2 (da, "LSR.W", src, ARG_DREG9, ARG_DREG0, 16);
		break;

	case 0x06:
		dasm_op2 (da, "ROXR.W", src, ARG_DREG9, ARG_DREG0, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x07:
		dasm_op2 (da, "ROR.W", src, ARG_DREG9, ARG_DREG0, 16);
		break;

	default:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;
	}
}

/* E080: misc */
static void d_e080 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ASR.L", src, ARG_IMMQ, ARG_DREG0, 32);
		break;

	case 0x01:
		dasm_op2 (da, "LSR.L", src, ARG_IMMQ, ARG_DREG0, 32);
		break;

	case 0x02:
		dasm_op2 (da, "ROXR.L", src, ARG_IMMQ, ARG_DREG0, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x03:
		dasm_op2 (da, "ROR.L", src, ARG_IMMQ, ARG_DREG0, 32);
		break;

	case 0x04:
		dasm_op2 (da, "ASR.L", src, ARG_DREG9, ARG_DREG0, 32);
		break;

	case 0x05:
		dasm_op2 (da, "LSR.L", src, ARG_DREG9, ARG_DREG0, 32);
		break;

	case 0x06:
		dasm_op2 (da, "ROXR.L", src, ARG_DREG9, ARG_DREG0, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x07:
		dasm_op2 (da, "ROR.L", src, ARG_DREG9, ARG_DREG0, 32);
		break;

	default:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
	}
}

/* E0C0: ASR.W <EA> */
static void d_e0c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "ASR.W", src, ARG_EA, 16);
}

/* E100: misc */
static void d_e100 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ASL.B", src, ARG_IMMQ, ARG_DREG0, 8);
		break;

	case 0x01:
		dasm_op2 (da, "LSL.B", src, ARG_IMMQ, ARG_DREG0, 8);
		break;

	case 0x02:
		dasm_op2 (da, "ROXL.B", src, ARG_IMMQ, ARG_DREG0, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x03:
		dasm_op2 (da, "ROL.B", src, ARG_IMMQ, ARG_DREG0, 8);
		break;

	case 0x04:
		dasm_op2 (da, "ASL.B", src, ARG_DREG9, ARG_DREG0, 8);
		break;

	case 0x05:
		dasm_op2 (da, "LSL.B", src, ARG_DREG9, ARG_DREG0, 8);
		break;

	case 0x06:
		dasm_op2 (da, "ROXL.B", src, ARG_DREG9, ARG_DREG0, 8);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x07:
		dasm_op2 (da, "ROL.B", src, ARG_DREG9, ARG_DREG0, 8);
		break;

	default:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;
	}
}

/* E140: misc */
static void d_e140 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ASL.W", src, ARG_IMMQ, ARG_DREG0, 16);
		break;

	case 0x01:
		dasm_op2 (da, "LSL.W", src, ARG_IMMQ, ARG_DREG0, 16);
		break;

	case 0x02:
		dasm_op2 (da, "ROXL.W", src, ARG_IMMQ, ARG_DREG0, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x03:
		dasm_op2 (da, "ROL.W", src, ARG_IMMQ, ARG_DREG0, 16);
		break;

	case 0x04:
		dasm_op2 (da, "ASL.W", src, ARG_DREG9, ARG_DREG0, 16);
		break;

	case 0x05:
		dasm_op2 (da, "LSL.W", src, ARG_DREG9, ARG_DREG0, 16);
		break;

	case 0x06:
		dasm_op2 (da, "ROXL.W", src, ARG_DREG9, ARG_DREG0, 16);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x07:
		dasm_op2 (da, "ROL.W", src, ARG_DREG9, ARG_DREG0, 16);
		break;

	default:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;
	}
}

/* E180: misc */
static void d_e180 (e68_dasm_t *da, const uint8_t *src)
{
	switch ((da->ir[0] >> 3) & 7) {
	case 0x00:
		dasm_op2 (da, "ASL.L", src, ARG_IMMQ, ARG_DREG0, 32);
		break;

	case 0x01:
		dasm_op2 (da, "LSL.L", src, ARG_IMMQ, ARG_DREG0, 32);
		break;

	case 0x02:
		dasm_op2 (da, "ROXL.L", src, ARG_IMMQ, ARG_DREG0, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x03:
		dasm_op2 (da, "ROL.L", src, ARG_IMMQ, ARG_DREG0, 32);
		break;

	case 0x04:
		dasm_op2 (da, "ASL.L", src, ARG_DREG9, ARG_DREG0, 32);
		break;

	case 0x05:
		dasm_op2 (da, "LSL.L", src, ARG_DREG9, ARG_DREG0, 32);
		break;

	case 0x06:
		dasm_op2 (da, "ROXL.L", src, ARG_DREG9, ARG_DREG0, 32);
		da->flags |= E68_DFLAG_DEP_CC;
		break;

	case 0x07:
		dasm_op2 (da, "ROL.L", src, ARG_DREG9, ARG_DREG0, 32);
		break;

	default:
		dasm_op1 (da, "DW", src, ARG_IR, 16);
		break;
	}
}

/* E1C0: ASL.W <EA> */
static void d_e1c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "ASL.W", src, ARG_EA, 16);
}

/* E2C0: LSR.W <EA> */
static void d_e2c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "LSR.W", src, ARG_EA, 16);
}

/* E3C0: LSL.W <EA> */
static void d_e3c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "LSL.W", src, ARG_EA, 16);
}

/* E4C0: ROXR.W <EA> */
static void d_e4c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "ROXR.W", src, ARG_EA, 16);
	da->flags |= E68_DFLAG_DEP_CC;
}

/* E5C0: ROXL.W <EA> */
static void d_e5c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "ROXL.W", src, ARG_EA, 16);
	da->flags |= E68_DFLAG_DEP_CC;
}

/* E6C0: ROR.W <EA> */
static void d_e6c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "ROR.W", src, ARG_EA, 16);
}

/* E7C0: ROL.W <EA> */
static void d_e7c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "ROL.W", src, ARG_EA, 16);
}

/* E8C0: BFTST <EA> {offset:width} */
static void d_e8c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "BFTST", src, ARG_BF, 8);
}

/* E9C0: BFEXTU <EA> {offset:width}, Dn */
static void d_e9c0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "BFEXTU", src, ARG_BF, ARG_BF_DN, 8);
}

/* EAC0: BFCHG <EA> {offset:width} */
static void d_eac0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "BFCHG", src, ARG_BF, 8);
}

/* EBC0: BFEXTS <EA> {offset:width}, Dn */
static void d_ebc0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "BFEXTS", src, ARG_BF, ARG_BF_DN, 8);
}

/* ECC0: BFCLR <EA> {offset:width} */
static void d_ecc0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "BFCLR", src, ARG_BF, 8);
}

/* EEC0: BFSET <EA> {offset:width} */
static void d_eec0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "BFSET", src, ARG_BF, 8);
}

/* EFC0: BFINS Dn, <EA> {offset:width} */
static void d_efc0 (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op2 (da, "BFINS", src, ARG_BF_DN, ARG_BF, 8);
}

/* F000: FXXX */
static void d_f000 (e68_dasm_t *da, const uint8_t *src)
{
	char buf[16];

	sprintf (buf, "%04X", da->ir[0]);

	dasm_op0 (da, buf);

	da->flags |= E68_DFLAG_CALL;
}

static void di_und (e68_dasm_t *da, const uint8_t *src)
{
	dasm_op1 (da, "DW", src, ARG_IR, 16);
}


void e68_dasm (e68_dasm_t *da, uint32_t pc, const unsigned char *src)
{
	unsigned i;
	unsigned opc;

	da->flags = 0;

	da->pc = pc;

	da->comm[0] = 0;

	da->ir[0] = e68_get_uint16 (src, 0);
	da->irn = 1;

	opc = ((src[0] & 0xff) << 2) | ((src[1] & 0xc0) >> 6);

	e68_dasm_op[opc] (da, src);

	for (i = 0; i < da->irn; i++) {
		da->ir[i] = e68_get_uint16 (src, 2 * i);
	}
}

void e68_dasm_mem (e68000_t *c, e68_dasm_t *da, uint32_t pc)
{
	unsigned i;
	uint8_t  src[16];

	for (i = 0; i < 16; i++) {
		src[i] = e68_get_mem8 (c, pc + i);
	}

	e68_dasm (da, pc, src);
}

void e68_dasm_cur (e68000_t *c, e68_dasm_t *da)
{
	e68_dasm_mem (c, da, e68_get_pc (c));
}

static
e68_dasm_f e68_dasm_op[1024] = {
	d_0000, d_0040, d_0080, d_00c0, d_0100, d_0140, d_0180, d_01c0, /* 0000 */
	d_0200, d_0240, d_0280, d_02c0, d_0100, d_0140, d_0180, d_01c0, /* 0200 */
	d_0400, d_0440, d_0480, d_04c0, d_0100, d_0140, d_0180, d_01c0, /* 0400 */
	d_0600, d_0640, d_0680, di_und, d_0100, d_0140, d_0180, d_01c0, /* 0600 */
	d_0800, d_0840, d_0880, d_08c0, d_0100, d_0140, d_0180, d_01c0, /* 0800 */
	d_0a00, d_0a40, d_0a80, di_und, d_0100, d_0140, d_0180, d_01c0, /* 0A00 */
	d_0c00, d_0c40, d_0c80, di_und, d_0100, d_0140, d_0180, d_01c0, /* 0C00 */
	d_0e00, d_0e40, d_0e80, d_4ec0, d_0100, d_0140, d_0180, d_01c0, /* 0E00 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1000 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1200 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1400 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1600 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1800 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1A00 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1C00 */
	d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, d_1000, /* 1E00 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2000 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2200 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2400 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2600 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2800 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2A00 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2C00 */
	d_2000, d_2040, d_2000, d_2000, d_2000, d_2000, d_2000, d_2000, /* 2E00 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3000 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3200 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3400 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3600 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3800 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3A00 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3C00 */
	d_3000, d_3040, d_3000, d_3000, d_3000, d_3000, d_3000, d_3000, /* 3E00 */
	d_4000, d_4040, d_4080, d_40c0, di_und, di_und, d_4180, d_41c0, /* 4000 */
	d_4200, d_4240, d_4280, d_42c0, di_und, di_und, d_4180, d_41c0, /* 4200 */
	d_4400, d_4440, d_4480, d_44c0, di_und, di_und, d_4180, d_41c0, /* 4400 */
	d_4600, d_4640, d_4680, d_46c0, di_und, di_und, d_4180, d_41c0, /* 4600 */
	d_4800, d_4840, d_4880, d_48c0, di_und, di_und, d_4180, d_49c0, /* 4800 */
	d_4a00, d_4a40, d_4a80, d_4ac0, di_und, di_und, d_4180, d_41c0, /* 4A00 */
	d_4c00, d_4c40, d_4c80, d_4cc0, di_und, di_und, d_4180, d_41c0, /* 4C00 */
	di_und, d_4e40, d_4e80, d_4ec0, di_und, di_und, d_4180, d_41c0, /* 4E00 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5000 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5200 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5400 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5600 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5800 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5A00 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5C00 */
	d_5000, d_5040, d_5080, d_50c0, d_5100, d_5140, d_5180, d_50c0, /* 5E00 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6000 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6200 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6400 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6600 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6800 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6A00 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6C00 */
	d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, d_6000, /* 6E00 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7000 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7200 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7400 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7600 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7800 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7A00 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7C00 */
	d_7000, d_7000, d_7000, d_7000, di_und, di_und, di_und, di_und, /* 7E00 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8000 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8200 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8400 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8600 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8800 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8A00 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8C00 */
	d_8000, d_8040, d_8080, d_80c0, d_8100, d_8140, d_8180, d_81c0, /* 8E00 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9000 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9200 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9400 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9600 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9800 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9A00 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9C00 */
	d_9000, d_9040, d_9080, d_90c0, d_9100, d_9140, d_9180, d_91c0, /* 9E00 */
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, /* A000 */
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000,
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000,
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000,
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000,
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000,
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000,
	d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000, d_a000,
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* B000 */
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* B200 */
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* B400 */
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* B600 */
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* B800 */
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* BA00 */
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* BC00 */
	d_b000, d_b040, d_b080, d_b0c0, d_b100, d_b140, d_b180, d_b1c0, /* BE00 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* C000 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* C200 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* C400 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* C600 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* C800 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* CA00 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* CC00 */
	d_c000, d_c040, d_c080, d_c0c0, d_c100, d_c140, d_c180, d_c1c0, /* CE00 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* D000 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* D200 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* D400 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* D600 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* D800 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* DA00 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* DC00 */
	d_d000, d_d040, d_d080, d_d0c0, d_d100, d_d140, d_d180, d_d1c0, /* DE00 */
	d_e000, d_e040, d_e080, d_e0c0, d_e100, d_e140, d_e180, d_e1c0, /* E000 */
	d_e000, d_e040, d_e080, d_e2c0, d_e100, d_e140, d_e180, d_e3c0, /* E200 */
	d_e000, d_e040, d_e080, d_e4c0, d_e100, d_e140, d_e180, d_e5c0, /* E400 */
	d_e000, d_e040, d_e080, d_e6c0, d_e100, d_e140, d_e180, d_e7c0, /* E600 */
	d_e000, d_e040, d_e080, d_e8c0, d_e100, d_e140, d_e180, d_e9c0, /* E800 */
	d_e000, d_e040, d_e080, d_eac0, d_e100, d_e140, d_e180, d_ebc0, /* EA00 */
	d_e000, d_e040, d_e080, d_ecc0, d_e100, d_e140, d_e180, di_und, /* EC00 */
	d_e000, d_e040, d_e080, d_eec0, d_e100, d_e140, d_e180, d_efc0, /* EE00 */
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, /* F000 */
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000,
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000,
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000,
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000,
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000,
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000,
	d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000, d_f000
};
