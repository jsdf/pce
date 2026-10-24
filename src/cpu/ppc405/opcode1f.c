/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/cpu/ppc405/opcode1f.c                                    *
 * Created:     2003-11-08 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>

#include "ppc405.h"
#include "internal.h"


/* 1F 000: cmp bf, ra, rb */
static
void op_1f_000 (p405_t *c)
{
	unsigned f;
	uint32_t d, s1, s2;

	s1 = p405_get_ra (c, c->ir) ^ 0x80000000;
	s2 = p405_get_rb (c, c->ir) ^ 0x80000000;

	f = (c->ir >> 23) & 0x07;

	if (s1 < s2) {
		d = P405_CR_LT;
	}
	else if (s1 > s2) {
		d = P405_CR_GT;
	}
	else {
		d = P405_CR_EQ;
	}

	if (p405_get_xer_so (c)) {
		d |= P405_CR_SO;
	}

	f = 4 * (7 - f);

	c->cr &= ~(0x0fUL << f);
	c->cr |= d << f;

	p405_set_clk (c, 4, 1);
}

/* 1F 008: subfc[.] rt, ra, rb */
static
void op_1f_008 (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_rb (c, c->ir);
	s2 = p405_get_ra (c, c->ir);

	rt = (s1 - s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, rt <= s1);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 00A: addc[.] rt, ra, rb */
static
void op_1f_00a (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	rt = (s1 + s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, rt < s1);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 00B: mulhwu[.] rt, ra, rb */
static
void op_1f_00b (p405_t *c)
{
	uint64_t rt;

	if (p405_check_reserved (c, 0x00000400UL)) {
		return;
	}

	rt = (uint64_t) p405_get_ra (c, c->ir) * (uint64_t) p405_get_rb (c, c->ir);
	rt = (rt >> 32) & 0xffffffffUL;

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 5);
}

/* 1F 013: mfcr rt */
static
void op_1f_013 (p405_t *c)
{
	if (p405_check_reserved (c, 0x001ff801UL)) {
		return;
	}

	p405_set_rt (c, c->ir, p405_get_cr (c));

	p405_set_clk (c, 4, 1);
}

/* 1F 014: lwarx rt, ra0, rb */
static
void op_1f_014 (p405_t *c)
{
	uint32_t rt, ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	c->reserve = 1;

	if (p405_dload32 (c, ea, &rt)) {
		return;
	}

	p405_set_rt (c, c->ir, rt);

	p405_set_clk (c, 4, 1);
}

/* 1F 017: lwzx rt, ra0, rb */
static
void op_1f_017 (p405_t *c)
{
	uint32_t rt, ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dload32 (c, ea, &rt)) {
		return;
	}

	p405_set_rt (c, c->ir, rt);

	p405_set_clk (c, 4, 1);
}

/* 1F 018: slw[.] ra, rs, rb */
static
void op_1f_018 (p405_t *c)
{
	uint32_t ra, rb;

	rb = p405_get_rb (c, c->ir);

	if (rb & 0x20) {
		ra = 0;
	}
	else {
		ra = (p405_get_rs (c, c->ir) << (rb & 0x1f)) & 0xffffffffUL;
	}

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 01A: cntlzw[.] ra, rs */
static
void op_1f_01a (p405_t *c)
{
	uint32_t ra, rs, msk;

	if (p405_check_reserved (c, 0x0000f800UL)) {
		return;
	}

	rs = p405_get_rs (c, c->ir);
	ra = 0;

	if (rs != 0) {
		msk = 0x80000000UL;
		while ((rs & msk) == 0) {
			ra += 1;
			msk = msk >> 1;
		}
	}
	else {
		ra = 32;
	}

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 01C: and[.] ra, rs, rb */
static
void op_1f_01c (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_rs (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	rt = s1 & s2;

	p405_set_ra (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 020: cmpl bf, ra, rb */
static
void op_1f_020 (p405_t *c)
{
	unsigned f;
	uint32_t d, s1, s2;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	f = (c->ir >> 23) & 0x07;

	if (s1 < s2) {
		d = P405_CR_LT;
	}
	else if (s1 > s2) {
		d = P405_CR_GT;
	}
	else {
		d = P405_CR_EQ;
	}

	if (c->xer & P405_XER_SO) {
		d = P405_CR_SO;
	}

	f = 4 * (7 - f);

	c->cr &= ~(0x0fUL << f);
	c->cr |= d << f;

	p405_set_clk (c, 4, 1);
}

/* 1F 028: subf[.] rt, ra, rb */
static
void op_1f_028 (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_rb (c, c->ir);
	s2 = p405_get_ra (c, c->ir);

	rt = (s1 - s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 036: dcbst ra0, rb */
static
void op_1f_036 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 037: lwzux rt, ra, rb */
static
void op_1f_037 (p405_t *c)
{
	uint32_t  rt, ea;

	if (p405_get_ea (c, &ea, 1, 1)) {
		return;
	}

	if (p405_dload32 (c, ea, &rt)) {
		return;
	}

	p405_set_ra (c, c->ir, ea);
	p405_set_rt (c, c->ir, rt);

	p405_set_clk (c, 4, 1);
}

/* 1F 03C: andc[.] ra, rs, rb */
static
void op_1f_03c (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_rs (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	rt = s1 & ~s2;

	p405_set_ra (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 04B: mulhw[.] rt, ra, rb */
static
void op_1f_04b (p405_t *c)
{
	uint64_t rt;

	if (p405_check_reserved (c, 0x00000400UL)) {
		return;
	}

	rt = p405_mul (p405_get_ra (c, c->ir), p405_get_rb (c, c->ir));
	rt = (rt >> 32) & 0xffffffffUL;

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 5);
}

/* 1F 053: mfmsr rt */
static
void op_1f_053 (p405_t *c)
{
	if (p405_check_reserved (c, 0x001ff801UL)) {
		return;
	}

	p405_set_rt (c, c->ir, p405_get_msr (c));

	p405_set_clk (c, 4, 1);
}

/* 1F 056: dcbf ra0, rb */
static
void op_1f_056 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 057: lbzx rt, ra0, rb */
static
void op_1f_057 (p405_t *c)
{
	uint8_t  rt;
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dload8 (c, ea, &rt)) {
		return;
	}

	p405_set_rt (c, c->ir, p405_uext (rt, 8));

	p405_set_clk (c, 4, 1);
}

/* 1F 068: neg[.] rt, ra */
static
void op_1f_068 (p405_t *c)
{
	uint32_t rt;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	rt = -p405_get_ra (c, c->ir) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 077: lbzux rt, ra, rb */
static
void op_1f_077 (p405_t *c)
{
	uint8_t  rt;
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 1)) {
		return;
	}

	if (p405_dload8 (c, ea, &rt)) {
		return;
	}

	p405_set_ra (c, c->ir, ea);
	p405_set_rt (c, c->ir, p405_uext (rt, 8));

	p405_set_clk (c, 4, 1);
}

/* 1F 07c: nor[.] ra, rs, rb */
static
void op_1f_07c (p405_t *c)
{
	uint32_t ra;

	ra = ~(p405_get_rs (c, c->ir) | p405_get_rb (c, c->ir)) & 0xffffffffUL;

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 083: wrtee rs */
static
void op_1f_083 (p405_t *c)
{
	if (p405_check_reserved (c, 0x001ff801UL)) {
		return;
	}

	if (p405_check_privilege (c)) {
		return;
	}

	p405_set_msr_ee (c, p405_get_rs (c, c->ir) & 0x8000UL);

	p405_set_clk (c, 4, 1);
}

/* 1F 086: dcbf ra0, rb */
static
void op_1f_086 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 088: subfe[.] rt, ra, rb */
static
void op_1f_088 (p405_t *c)
{
	uint32_t rt, s1, s2, s3;

	s1 = p405_get_rb (c, c->ir);
	s2 = ~p405_get_ra (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (s1 + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, (rt < s1) || ((rt == s1) && s3));

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 08A: adde[.] rt, ra, rb */
static
void op_1f_08a (p405_t *c)
{
	uint32_t rt, s1, s2, s3;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (s1 + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, (rt < s1) || ((rt == s1) && s3));

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 090: mtcrf fxm, rs */
static
void op_1f_090 (p405_t *c)
{
	unsigned i, fxm;
	uint32_t rs, msk;

	if (p405_check_reserved (c, 0x00100801UL)) {
		return;
	}

	rs = p405_get_rs (c, c->ir);
	fxm = (c->ir >> 12) & 0xff;

	msk = 0;

	for (i = 0; i < 8; i++) {
		if (fxm & (0x80 >> i)) {
			msk |= 0xf0000000UL >> (4 * i);
		}
	}

	p405_set_cr (c, (p405_get_cr (c) & ~msk) | (rs & msk));

	p405_set_clk (c, 4, 1);
}

/* 1F 092: mtmsr rs */
static
void op_1f_092 (p405_t *c)
{
	if (p405_check_reserved (c, 0x001ff801UL)) {
		return;
	}

	p405_set_msr (c, p405_get_rs (c, c->ir));

	p405_set_clk (c, 4, 1);
}

/* 1F 096: stwcx. rs, ra0, rb */
static
void op_1f_096 (p405_t *c)
{
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	p405_set_crf (c, 0, 0);
	p405_set_cr_so (c, 0, p405_get_xer_ov (c));

	if (c->reserve) {
		if (p405_dstore32 (c, ea, p405_get_rs (c, c->ir))) {
			return;
		}

		c->reserve = 0;

		p405_set_cr_eq (c, 0, 1);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 097: stwx rs, ra0, rb */
static
void op_1f_097 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dstore32 (c, ea, p405_get_rs (c, c->ir))) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 0A3: wrteei e */
static
void op_1f_0a3 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03ff7801UL)) {
		return;
	}

	if (p405_check_privilege (c)) {
		return;
	}

	p405_set_msr_ee (c, c->ir & 0x8000UL);

	p405_set_clk (c, 4, 1);
}

/* 1F 0B7: stwux rs, ra, rb */
static
void op_1f_0b7 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 1)) {
		return;
	}

	if (p405_dstore32 (c, ea, p405_get_rs (c, c->ir))) {
		return;
	}

	p405_set_ra (c, c->ir, ea);

	p405_set_clk (c, 4, 1);
}

/* 1F 0C8: subfze[.] rt, ra */
static
void op_1f_0c8 (p405_t *c)
{
	uint32_t rt, s2, s3;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	s2 = ~p405_get_ra (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (0 + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, (rt == 0) && s3);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 0CA: addze[.] rt, ra */
static
void op_1f_0ca (p405_t *c)
{
	uint32_t rt, s1, s2;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_xer_ca (c);

	rt = (s1 + s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, rt < s1);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 0D7: stbx rs, ra0, rb */
static
void op_1f_0d7 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dstore8 (c, ea, p405_uext (p405_get_rs (c, c->ir), 8))) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 0E8: subfme[.] rt, ra */
static
void op_1f_0e8 (p405_t *c)
{
	uint32_t rt, s2, s3;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	s2 = ~p405_get_ra (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (0xffffffff + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, s2 || s3);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 0EA: addme[.] rt, ra */
static
void op_1f_0ea (p405_t *c)
{
	uint32_t rt, s1, s2;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_xer_ca (c);

	rt = (s1 + s2 - 1) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_ca (c, (s1 > 0) || (s2 > 0));

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 0EB: mullw[.] rt, ra, rb */
static
void op_1f_0eb (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	rt = s1 * s2;

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 5);
}

/* 1F 0F6: dcbtst ra0, rb */
static
void op_1f_0f6 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 0F7: stbux rs, ra, rb */
static
void op_1f_0f7 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 1)) {
		return;
	}

	if (p405_dstore8 (c, ea, p405_uext (p405_get_rs (c, c->ir), 8))) {
		return;
	}

	p405_set_ra (c, c->ir, ea);

	p405_set_clk (c, 4, 1);
}

/* 1F 106: icbt ra, rb */
static
void op_1f_106 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 10A: add[.] rt, ra, rb */
static
void op_1f_10a (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	rt = (s1 + s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 116: dcbt ra0, rb */
static
void op_1f_116 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 117: lhzx rt, ra0, rb */
static
void op_1f_117 (p405_t *c)
{
	uint16_t rt;
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dload16 (c, ea, &rt)) {
		return;
	}

	p405_set_rt (c, c->ir, p405_uext (rt, 16));

	p405_set_clk (c, 4, 1);
}

/* 1F 11C: eqv[.] ra, rs, rb */
static
void op_1f_11c (p405_t *c)
{
	uint32_t rs, ra, rb;

	rs = p405_get_rs (c, c->ir);
	rb = p405_get_rb (c, c->ir);
	ra = ~(rs ^ rb) & 0xffffffffUL;
	p405_set_ra (c, c->ir, ra);

	if (c->ir & P405_IR_RC) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 137: lhzux rt, ra, rb */
static
void op_1f_137 (p405_t *c)
{
	uint16_t rt;
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 1)) {
		return;
	}

	if (p405_dload16 (c, ea, &rt)) {
		return;
	}

	p405_set_ra (c, c->ir, ea);
	p405_set_rt (c, c->ir, p405_uext (rt, 16));

	p405_set_clk (c, 4, 1);
}

/* 1F 13C: xor[.] ra, rs, rb */
static
void op_1f_13c (p405_t *c)
{
	uint32_t rs, ra, rb;

	rs = p405_get_rs (c, c->ir);
	rb = p405_get_rb (c, c->ir);
	ra = rs ^ rb;
	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 143: mfdcr rt, dcrn */
static
void op_1f_143 (p405_t *c)
{
	unsigned dcrf, dcrn;

	if (p405_check_privilege (c)) {
		return;
	}

	dcrf = (c->ir >> 11) & 0x3ff;
	dcrn = ((dcrf & 0x1f) << 5) | ((dcrf >> 5) & 0x1f);

	p405_set_rt (c, c->ir, p405_get_dcr (c, dcrn));

	p405_set_clk (c, 4, 1);
}

/* 1F 153: mfspr rt, sprn */
static
void op_1f_153 (p405_t *c)
{
	unsigned sprf, sprn;
	uint32_t rt;

	sprf = (c->ir >> 11) & 0x3ff;

	if (sprf & 0x200) {
		if (p405_check_privilege (c)) {
			return;
		}
	}

	sprn = ((sprf & 0x1f) << 5) | ((sprf >> 5) & 0x1f);

	rt = 0;

	switch (sprn) {
	case P405_SPRN_CTR:
		rt = p405_get_ctr (c);
		break;

	case P405_SPRN_DBCR0:
		rt = p405_get_dbcr0 (c);
		break;

	case P405_SPRN_DBCR1:
		rt = p405_get_dbcr1 (c);
		break;

	case P405_SPRN_DBSR:
		rt = p405_get_dbsr (c);
		break;

	case P405_SPRN_DCCR:
		rt = p405_get_dccr (c);
		break;

	case P405_SPRN_DCWR:
		rt = p405_get_dcwr (c);
		break;

	case P405_SPRN_DEAR:
		rt = p405_get_dear (c);
		break;

	case P405_SPRN_ESR:
		rt = p405_get_esr (c);
		break;

	case P405_SPRN_EVPR:
		rt = p405_get_evpr (c);
		break;

	case P405_SPRN_ICCR:
		rt = p405_get_iccr (c);
		break;

	case P405_SPRN_LR:
		rt = p405_get_lr (c);
		break;

	case P405_SPRN_PID:
		rt = p405_get_pid (c);
		break;

	case P405_SPRN_PIT:
		rt = p405_get_pit (c, 0);
		break;

	case P405_SPRN_PVR:
		rt = p405_get_pvr (c);
		break;

	case P405_SPRN_SPRG0:
		rt = p405_get_sprg (c, 0);
		break;

	case P405_SPRN_SPRG1:
		rt = p405_get_sprg (c, 1);
		break;

	case P405_SPRN_SPRG2:
		rt = p405_get_sprg (c, 2);
		break;

	case P405_SPRN_SPRG3:
		rt = p405_get_sprg (c, 3);
		break;

	case P405_SPRN_SPRG4:
		rt = p405_get_sprg (c, 4);
		break;

	case P405_SPRN_SPRG5:
		rt = p405_get_sprg (c, 5);
		break;

	case P405_SPRN_SPRG6:
		rt = p405_get_sprg (c, 6);
		break;

	case P405_SPRN_SPRG7:
		rt = p405_get_sprg (c, 7);
		break;

	case P405_SPRN_SPRG4R:
		rt = p405_get_sprg (c, 4);
		break;

	case P405_SPRN_SPRG5R:
		rt = p405_get_sprg (c, 5);
		break;

	case P405_SPRN_SPRG6R:
		rt = p405_get_sprg (c, 6);
		break;

	case P405_SPRN_SPRG7R:
		rt = p405_get_sprg (c, 7);
		break;

	case P405_SPRN_SRR0:
		rt = p405_get_srr (c, 0);
		break;

	case P405_SPRN_SRR1:
		rt = p405_get_srr (c, 1);
		break;

	case P405_SPRN_SRR2:
		rt = p405_get_srr (c, 2);
		break;

	case P405_SPRN_SRR3:
		rt = p405_get_srr (c, 3);
		break;

	case P405_SPRN_TBL:
	case P405_SPRN_TBLU:
		rt = p405_get_tbl (c);
		break;

	case P405_SPRN_TBU:
	case P405_SPRN_TBUU:
		rt = p405_get_tbu (c);
		break;

	case P405_SPRN_TCR:
		rt = p405_get_tcr (c);
		break;

	case P405_SPRN_XER:
		rt = p405_get_xer (c);
		break;

	case P405_SPRN_ZPR:
		rt = p405_get_zpr (c);
		break;

	default:
		p405_op_undefined (c);
		return;
	}

	p405_set_rt (c, c->ir, rt);

	p405_set_clk (c, 4, 1);
}

/* 1F 157: lhax rt, ra, rb */
static
void op_1f_157 (p405_t *c)
{
	uint16_t rt;
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dload16 (c, ea, &rt)) {
		return;
	}

	p405_set_rt (c, c->ir, p405_sext (rt, 16));

	p405_set_clk (c, 4, 1);
}

/* 1F 172: tlbia */
static
void op_1f_172 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03fff801UL)) {
		return;
	}

	p405_tlb_invalidate_all (c);

	p405_set_clk (c, 4, 1);
}

/* 1F 173: mftb rt, tbrn */
static
void op_1f_173 (p405_t *c)
{
	unsigned tbrf, tbrn;

	tbrf = (c->ir >> 11) & 0x3ff;
	tbrn = ((tbrf & 0x1f) << 5) | ((tbrf >> 5) & 0x1f);

	switch (tbrn) {
	case P405_TBRN_TBL:
		p405_set_rt (c, c->ir, p405_get_tbl (c));
		break;

	case P405_TBRN_TBU:
		p405_set_rt (c, c->ir, p405_get_tbu (c));
		break;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 177: lhaux rt, ra, rb */
static
void op_1f_177 (p405_t *c)
{
	uint16_t rt;
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 1)) {
		return;
	}

	if (p405_dload16 (c, ea, &rt)) {
		return;
	}

	p405_set_ra (c, c->ir, ea);
	p405_set_rt (c, c->ir, p405_sext (rt, 16));

	p405_set_clk (c, 4, 1);
}

/* 1F 197: sthx rs, ra0, rb */
static
void op_1f_197 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dstore16 (c, ea, p405_uext (p405_get_rs (c, c->ir), 16))) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 19C: orc[.] ra, rs, rb */
static
void op_1f_19c (p405_t *c)
{
	uint32_t ra;

	ra = (p405_get_rs (c, c->ir) | ~p405_get_rb (c, c->ir)) & 0xffffffffUL;

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 1B7: sthux rs, ra, rb */
static
void op_1f_1b7 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 1)) {
		return;
	}

	if (p405_dstore16 (c, ea, p405_uext (p405_get_rs (c, c->ir), 16))) {
		return;
	}

	p405_set_ra (c, c->ir, ea);

	p405_set_clk (c, 4, 1);
}

/* 1F 1BC: or[.] ra, rs, rb */
static
void op_1f_1bc (p405_t *c)
{
	uint32_t rs, ra, rb;

	rs = p405_get_rs (c, c->ir);
	rb = p405_get_rb (c, c->ir);
	ra = rs | rb;
	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 1C3: mtdcr dcrn, rs */
static
void op_1f_1c3 (p405_t *c)
{
	unsigned dcrf, dcrn;

	if (p405_check_privilege (c)) {
		return;
	}

	dcrf = (c->ir >> 11) & 0x3ff;
	dcrn = ((dcrf & 0x1f) << 5) | ((dcrf >> 5) & 0x1f);

	p405_set_dcr (c, dcrn, p405_get_rs (c, c->ir));

	p405_set_clk (c, 4, 1);
}

/* 1F 1C6: dccci ra0, rb */
static
void op_1f_1c6 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 1CB: divwu[.] rt, ra, rb */
static
void op_1f_1cb (p405_t *c)
{
	uint32_t rt, ra, rb;

	ra = p405_get_ra (c, c->ir);
	rb = p405_get_rb (c, c->ir);

	if (rb == 0) {
		p405_set_clk (c, 4, 1);
		return;
	}

	rt = ra / rb;

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 35);
}

/* 1F 1D3: mtspr sprn, rs */
static
void op_1f_1d3 (p405_t *c)
{
	unsigned sprf, sprn;
	uint32_t rs;

	sprf = (c->ir >> 11) & 0x3ff;

	if (sprf & 0x200) {
		if (p405_check_privilege (c)) {
			return;
		}
	}

	sprn = ((sprf & 0x1f) << 5) | ((sprf >> 5) & 0x1f);

	rs = p405_get_rs (c, c->ir);

	switch (sprn) {
	case P405_SPRN_CTR:
		p405_set_ctr (c, rs);
		break;

	case P405_SPRN_DBCR0:
		p405_set_dbcr0 (c, rs);
		break;

	case P405_SPRN_DBCR1:
		p405_set_dbcr1 (c, rs);
		break;

	case P405_SPRN_DBSR:
		p405_set_dbsr (c, p405_get_dbsr (c) & ~rs);
		break;

	case P405_SPRN_DCCR:
		p405_set_dccr (c, rs);
		break;

	case P405_SPRN_DCWR:
		p405_set_dcwr (c, rs);
		break;

	case P405_SPRN_DEAR:
		p405_set_dear (c, rs);
		break;

	case P405_SPRN_ESR:
		p405_set_esr (c, rs);
		break;

	case P405_SPRN_EVPR:
		p405_set_evpr (c, rs);
		break;

	case P405_SPRN_ICCR:
		p405_set_iccr (c, rs);
		break;

	case P405_SPRN_LR:
		p405_set_lr (c, rs);
		break;

	case P405_SPRN_PID:
		p405_set_pid (c, rs);
		p405_tbuf_clear (c);
		break;

	case P405_SPRN_PIT:
		p405_set_pit (c, 0, rs);
		p405_set_pit (c, 1, rs);
		break;

	case P405_SPRN_PVR:
		/* p405_set_pvr (c, rs); */
		break;

	case P405_SPRN_SPRG0:
		p405_set_sprg (c, 0, rs);
		break;

	case P405_SPRN_SPRG1:
		p405_set_sprg (c, 1, rs);
		break;

	case P405_SPRN_SPRG2:
		p405_set_sprg (c, 2, rs);
		break;

	case P405_SPRN_SPRG3:
		p405_set_sprg (c, 3, rs);
		break;

	case P405_SPRN_SPRG4:
		p405_set_sprg (c, 4, rs);
		break;

	case P405_SPRN_SPRG5:
		p405_set_sprg (c, 5, rs);
		break;

	case P405_SPRN_SPRG6:
		p405_set_sprg (c, 6, rs);
		break;

	case P405_SPRN_SPRG7:
		p405_set_sprg (c, 7, rs);
		break;

	case P405_SPRN_SPRG4R:
		p405_set_sprg (c, 4, rs);
		break;

	case P405_SPRN_SPRG5R:
		p405_set_sprg (c, 5, rs);
		break;

	case P405_SPRN_SPRG6R:
		p405_set_sprg (c, 6, rs);
		break;

	case P405_SPRN_SPRG7R:
		p405_set_sprg (c, 7, rs);
		break;

	case P405_SPRN_SRR0:
		p405_set_srr (c, 0, rs);
		break;

	case P405_SPRN_SRR1:
		p405_set_srr (c, 1, rs);
		break;

	case P405_SPRN_SRR2:
		p405_set_srr (c, 2, rs);
		break;

	case P405_SPRN_SRR3:
		p405_set_srr (c, 3, rs);
		break;

	case P405_SPRN_TBL:
		p405_set_tbl (c, rs);
		break;

	case P405_SPRN_TCR:
		p405_set_tcr (c, rs);
		break;

	case P405_SPRN_TBU:
		p405_set_tbu (c, rs);
		break;

	case P405_SPRN_TSR:
		p405_set_tsr (c, p405_get_tsr (c) & ~rs);
		break;

	case P405_SPRN_XER:
		p405_set_xer (c, rs);
		break;

	case P405_SPRN_ZPR:
		p405_set_zpr (c, rs);
		p405_tbuf_clear (c);
		break;

	default:
		p405_op_undefined (c);
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 1D6: dcbi ra0, rb */
static
void op_1f_1d6 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 1DC: nand[.] ra, rs, rb */
static
void op_1f_1dc (p405_t *c)
{
	uint32_t ra;

	ra = ~(p405_get_rs (c, c->ir) & p405_get_rb (c, c->ir)) & 0xffffffffUL;

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 1EB: divw[.] rt, ra, rb */
static
void op_1f_1eb (p405_t *c)
{
	int      sa, sb;
	uint32_t rt, ra, rb;

	ra = p405_get_ra (c, c->ir);
	rb = p405_get_rb (c, c->ir);

	if ((rb == 0) || ((ra == 0x80000000) && (rb == 0xffffffff))) {
		p405_set_clk (c, 4, 1);
		return;
	}

	sa = (ra & 0x80000000) != 0;
	sb = (rb & 0x80000000) != 0;

	if (sa) {
		ra = -ra & 0xffffffff;
	}

	if (sb) {
		rb = -rb & 0xffffffff;
	}

	rt = ra / rb;

	if (sa != sb) {
		rt = -rt & 0xffffffff;
	}

	p405_set_rt (c, c->ir, rt);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 35);
}

/* 1F 200: mcrxr bf */
static
void op_1f_200 (p405_t *c)
{
	unsigned bf;

	if (p405_check_reserved (c, 0x007ff801UL)) {
		return;
	}

	bf = p405_get_ir_rt (c->ir) >> 2;

	p405_set_crf (c, bf, (p405_get_xer (c) >> 28) & 0x0f);
	p405_set_xer (c, p405_get_xer (c) & 0x0fffffffUL);

	p405_set_clk (c, 4, 1);
}

/* 1F 208: subfco[.] rt, ra, rb */
static
void op_1f_208 (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_rb (c, c->ir);
	s2 = p405_get_ra (c, c->ir);

	rt = (s1 - s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000);
	p405_set_xer_ca (c, rt <= s1);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 20A: addco[.] rt, ra, rb */
static
void op_1f_20a (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	rt = (s1 + s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000);
	p405_set_xer_ca (c, rt < s1);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 215: lswx rt, ra0, rb */
static
void op_1f_215 (p405_t *c)
{
	p405_op_lsw (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_rb (c->ir),
		p405_get_ra0 (c, c->ir) + p405_get_rb (c, c->ir), p405_get_xer (c) & 0x7f
	);
}

/* 1F 216: lwbrx rt, ra0, rb */
static
void op_1f_216 (p405_t *c)
{
	uint32_t rt, ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dload32 (c, ea, &rt)) {
		return;
	}

	p405_set_rt (c, c->ir, p405_br32 (rt));

	p405_set_clk (c, 4, 1);
}

/* 1F 217: lfsx */
static
void op_1f_217 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 218: srw[.] ra, rs, rb */
static
void op_1f_218 (p405_t *c)
{
	uint32_t ra, rb;

	rb = p405_get_rb (c, c->ir);

	if (rb & 0x20) {
		ra = 0;
	}
	else {
		ra = p405_get_rs (c, c->ir) >> (rb & 0x1f);
	}

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 228: subfo[.] rt, ra, rb */
static
void op_1f_228 (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_rb (c, c->ir);
	s2 = p405_get_ra (c, c->ir);

	rt = (s1 - s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 236: tlbsync */
static
void op_1f_236 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03fff801UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 237: lfsux */
static
void op_1f_237 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 255: lswi rt, ra0, nb */
static
void op_1f_255 (p405_t *c)
{
	unsigned nb;

	nb = p405_get_ir_rb (c->ir);

	p405_op_lsw (c,
		p405_get_ir_rt (c->ir), p405_get_ir_ra (c->ir), p405_get_ir_ra (c->ir),
		p405_get_ra0 (c, c->ir), (nb == 0) ? 32 : nb
	);
}

/* 1F 256: sync */
static
void op_1f_256 (p405_t *c)
{
	p405_set_clk (c, 4, 1);
}

/* 1F 257: lfdx */
static
void op_1f_257 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 268: nego[.] rt, ra */
static
void op_1f_268 (p405_t *c)
{
	uint32_t rt;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	rt = -p405_get_ra (c, c->ir) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, rt == 0x80000000);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 277: lfdux */
static
void op_1f_277 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 288: subfeo[.] rt, ra, rb */
static
void op_1f_288 (p405_t *c)
{
	uint32_t rt, s1, s2, s3;

	s1 = p405_get_rb (c, c->ir);
	s2 = ~p405_get_ra (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (s1 + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000);
	p405_set_xer_ca (c, (rt < s1) || ((rt == s1) && s3));

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 28A: addeo[.] rt, ra, rb */
static
void op_1f_28a (p405_t *c)
{
	uint32_t rt, s1, s2, s3;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (s1 + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000);
	p405_set_xer_ca (c, (rt < s1) || ((rt == s1) && (s3 > 0)));

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 295: stswx rs, ra0, rb */
static
void op_1f_295 (p405_t *c)
{
	p405_op_stsw (c, p405_get_ir_rt (c->ir),
		p405_get_ra0 (c, c->ir) + p405_get_rb (c, c->ir), p405_get_xer (c) & 0x7f
	);
}

/* 1F 296: stwbrx rs, ra0, rb */
static
void op_1f_296 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dstore32 (c, ea, p405_br32 (p405_get_rs (c, c->ir)))) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 297: stfsx */
static
void op_1f_297 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 2B7: stfsux */
static
void op_1f_2b7 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 2C8: subfzeo[.] rt, ra */
static
void op_1f_2c8 (p405_t *c)
{
	uint32_t rt, s2, s3;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	s2 = ~p405_get_ra (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (0 + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, rt & (rt ^ s2) & 0x80000000);
	p405_set_xer_ca (c, (rt == 0) && s3);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 2CA: addzeo[.] rt, ra */
static
void op_1f_2ca (p405_t *c)
{
	uint32_t rt, s1, s2;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_xer_ca (c);

	rt = (s1 + s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & rt & 0x80000000);
	p405_set_xer_ca (c, rt < s1);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 2D5: stswi rs, ra0, nb */
static
void op_1f_2d5 (p405_t *c)
{
	unsigned nb;

	nb = p405_get_ir_rb (c->ir);

	p405_op_stsw (c, p405_get_ir_rt (c->ir), p405_get_ra0 (c, c->ir),
		(nb == 0) ? 32 : nb
	);
}

/* 1F 2D7: stfdx */
static
void op_1f_2d7 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 2E8: subfmeo[.] rt, ra */
static
void op_1f_2e8 (p405_t *c)
{
	uint32_t rt, s2, s3;

	if (p405_check_reserved (c, 0x0000f800)) {
		return;
	}

	s2 = ~p405_get_ra (c, c->ir);
	s3 = p405_get_xer_ca (c);

	rt = (0xffffffff + s2 + s3) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, ~rt & (rt ^ s2) & 0x80000000);
	p405_set_xer_ca (c, s2 || s3);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 2EA: addmeo[.] rt, ra */
static
void op_1f_2ea (p405_t *c)
{
	uint32_t rt, s1, s2;

	if (p405_check_reserved (c, 0xf800UL)) {
		return;
	}

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_xer_ca (c);

	rt = (s1 + s2 - 1) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & ~rt & 0x80000000);
	p405_set_xer_ca (c, (s1 > 0) || (s2 > 0));

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 2EB: mullwo[.] rt, ra, rb */
static
void op_1f_2eb (p405_t *c)
{
	uint32_t rt;
	uint64_t val1, val2;

	val1 = p405_mul (p405_get_ra (c, c->ir), p405_get_rb (c, c->ir));
	rt = val1 & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	val2 = (rt & 0x80000000) ? (val1 | 0xffffffff00000000ULL) : rt;

	p405_set_xer_oflow (c, val1 != val2);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 5);
}

/* 1F 2F6: dcba ra0, rb */
static
void op_1f_2f6 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 2F7: stfdux */
static
void op_1f_2f7 (p405_t *c)
{
	p405_set_clk (c, 0, 1);
	p405_exception_program_fpu (c);
}

/* 1F 30A: addo[.] rt, ra, rb */
static
void op_1f_30a (p405_t *c)
{
	uint32_t rt, s1, s2;

	s1 = p405_get_ra (c, c->ir);
	s2 = p405_get_rb (c, c->ir);

	rt = (s1 + s2) & 0xffffffff;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 316: lhbrx rt, ra0, rb */
static
void op_1f_316 (p405_t *c)
{
	uint16_t rt;
	uint32_t ea;

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dload16 (c, ea, &rt)) {
		return;
	}

	p405_set_rt (c, c->ir, p405_br16 (rt));

	p405_set_clk (c, 4, 1);
}

/* 1F 318: sraw[.] ra, rs, rb */
static
void op_1f_318 (p405_t *c)
{
	unsigned sh;
	uint32_t ra, rs, rb;

	rs = p405_get_rs (c, c->ir);
	rb = p405_get_rb (c, c->ir);

	sh = rb & 0x1f;

	if (rb & 0x20) {
		if (rs & 0x80000000UL) {
			ra = 0xffffffffUL;
			p405_set_xer_ca (c, 1);
		}
		else {
			ra = 0;
			p405_set_xer_ca (c, 0);
		}
	}
	else {
		if (rs & 0x80000000UL) {
			ra = ((0xffffffff00000000ULL | rs) >> sh) & 0xffffffffUL;
			p405_set_xer_ca (c, ((ra << sh) & 0xffffffffUL) != rs);
		}
		else {
			ra = rs >> sh;
			p405_set_xer_ca (c, 0);
		}
	}

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 338: srawi[.] ra, rs, sh */
static
void op_1f_338 (p405_t *c)
{
	unsigned sh;
	uint32_t ra, rs;

	rs = p405_get_rs (c, c->ir);
	sh = p405_get_ir_rb (c->ir);

	if (rs & 0x80000000UL) {
		ra = ((0xffffffff00000000ULL | rs) >> sh) & 0xffffffffUL;
		p405_set_xer_ca (c, ((ra << sh) & 0xffffffffUL) != rs);
	}
	else {
		ra = rs >> sh;
		p405_set_xer_ca (c, 0);
	}

	p405_set_ra (c, c->ir, ra);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 356: eieio */
static
void op_1f_356 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03fff800UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 392: tlbsx rt, ra0, rb */
static
void op_1f_392 (p405_t *c)
{
	int           rc;
	uint32_t      rt, ea;
	p405_tlbe_t *ent;

	ea = (p405_get_ra0 (c, c->ir) + p405_get_rb (c, c->ir)) & 0xffffffffUL;

	rc = p405_get_ir_rc (c->ir);
	rt = p405_get_tlb_index (c, ea);
	ent = p405_get_tlb_entry_idx (c, rt);

	if (rc) {
		p405_set_crf (c, 0, p405_get_xer_so (c) ? P405_CR_SO : 0);
	}

	if ((ent != NULL) && p405_get_tlbe_v (ent)) {
		p405_set_rt (c, c->ir, rt);

		if (rc) {
			p405_set_cr_eq (c, 0, 1);
		}
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 396: sthbrx rs, ra0, rb */
static
void op_1f_396 (p405_t *c)
{
	uint32_t ea;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	if (p405_dstore16 (c, ea, p405_br16 (p405_get_rs (c, c->ir)))) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 39A: extsh[.] ra, rs */
static
void op_1f_39a (p405_t *c)
{
	uint32_t ra;

	if (p405_check_reserved (c, 0xf800UL)) {
		return;
	}

	ra = p405_sext (p405_get_rs (c, c->ir), 16);
	p405_set_ra (c, c->ir, ra);

	if (c->ir & P405_IR_RC) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 3B2: tlbre rt, ra, ws */
static
void op_1f_3b2 (p405_t *c)
{
	unsigned ws, ra;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_check_privilege (c)) {
		return;
	}

	ws = p405_get_ir_rb (c->ir);
	ra = p405_get_ra (c, c->ir);

	if (ws == 1) {
		p405_set_rt (c, c->ir, p405_get_tlb_entry_lo (c, ra));
	}
	else if (ws == 0) {
		p405_set_rt (c, c->ir, p405_get_tlb_entry_hi (c, ra) & 0xfffffff0);
		p405_set_pid (c, p405_get_tlb_entry_tid (c, ra));
	}
	else {
		p405_op_undefined (c);
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 3BA: extsb[.] ra, rs */
static
void op_1f_3ba (p405_t *c)
{
	uint32_t ra;

	if (p405_check_reserved (c, 0xf800UL)) {
		return;
	}

	ra = p405_sext (p405_get_rs (c, c->ir), 8);
	p405_set_ra (c, c->ir, ra);

	if (c->ir & P405_IR_RC) {
		p405_set_cr0 (c, ra);
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 3C6: iccci ra, rb */
static
void op_1f_3c6 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 3CB: divwuo[.] rt, ra, rb */
static
void op_1f_3cb (p405_t *c)
{
	uint32_t rt, ra, rb;

	ra = p405_get_ra (c, c->ir);
	rb = p405_get_rb (c, c->ir);

	if (rb == 0) {
		p405_set_xer_oflow (c, 1);
		p405_set_clk (c, 4, 1);
		return;
	}

	rt = ra / rb;

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, 0);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 35);
}

/* 1F 3D2: tlbwe rs, ra, ws */
static
void op_1f_3d2 (p405_t *c)
{
	unsigned ws, rs, ra;

	if (p405_check_reserved (c, 0x01)) {
		return;
	}

	if (p405_check_privilege (c)) {
		return;
	}

	ws = p405_get_ir_rb (c->ir);
	ra = p405_get_ra (c, c->ir);
	rs = p405_get_rs (c, c->ir);

	if (ws == 1) {
		p405_set_tlb_entry_lo (c, ra, rs);
	}
	else if (ws == 0) {
		p405_set_tlb_entry_hi (c, ra, rs, p405_get_pid (c));
	}
	else {
		p405_op_undefined (c);
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 3D6: icbi ra, rb */
static
void op_1f_3d6 (p405_t *c)
{
	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F 3EB: divwo[.] rt, ra, rb */
static
void op_1f_3eb (p405_t *c)
{
	int      sa, sb;
	uint32_t rt, ra, rb;

	ra = p405_get_ra (c, c->ir);
	rb = p405_get_rb (c, c->ir);

	if ((rb == 0) || ((ra == 0x80000000) && (rb == 0xffffffff))) {
		p405_set_xer_oflow (c, 1);
		p405_set_clk (c, 4, 1);
		return;
	}

	sa = (ra & 0x80000000) != 0;
	sb = (rb & 0x80000000) != 0;

	if (sa) {
		ra = -ra & 0xffffffff;
	}

	if (sb) {
		rb = -rb & 0xffffffff;
	}

	rt = ra / rb;

	if (sa != sb) {
		rt = -rt & 0xffffffff;
	}

	p405_set_rt (c, c->ir, rt);

	p405_set_xer_oflow (c, 0);

	if (p405_get_ir_rc (c->ir)) {
		p405_set_cr0 (c, rt);
	}

	p405_set_clk (c, 4, 35);
}

/* 1F 3F6: dcbz ra0, rb */
static
void op_1f_3f6 (p405_t *c)
{
	unsigned i;
	uint32_t ea;

	if (p405_check_reserved (c, 0x03e00001UL)) {
		return;
	}

	/* alignment exception if cache block non-cacheable or write-through */

	if (p405_get_ea (c, &ea, 1, 0)) {
		return;
	}

	ea &= ~(unsigned long) (P405_CACHE_LINE_SIZE - 1);

	i = 0;
	while (i < P405_CACHE_LINE_SIZE) {
		if (p405_dstore32 (c, ea + i, 0)) {
			return;
		}

		i += 4;
	}

	p405_set_clk (c, 4, 1);
}

/* 1F: */
static
void op_1f (p405_t *c)
{
	unsigned op2;

	op2 = (c->ir >> 1) & 0x3ff;

	c->opcodes.op1f[op2] (c);
}


static
p405_opcode_list_t p405_opcodes_1f[] = {
	{ 0x000, op_1f_000 },
	{ 0x008, op_1f_008 },
	{ 0x00a, op_1f_00a },
	{ 0x00b, op_1f_00b },
	{ 0x013, op_1f_013 },
	{ 0x014, op_1f_014 },
	{ 0x017, op_1f_017 },
	{ 0x018, op_1f_018 },
	{ 0x01a, op_1f_01a },
	{ 0x01c, op_1f_01c },
	{ 0x020, op_1f_020 },
	{ 0x028, op_1f_028 },
	{ 0x036, op_1f_036 },
	{ 0x037, op_1f_037 },
	{ 0x03c, op_1f_03c },
	{ 0x04b, op_1f_04b },
	{ 0x053, op_1f_053 },
	{ 0x056, op_1f_056 },
	{ 0x057, op_1f_057 },
	{ 0x068, op_1f_068 },
	{ 0x077, op_1f_077 },
	{ 0x07c, op_1f_07c },
	{ 0x083, op_1f_083 },
	{ 0x086, op_1f_086 },
	{ 0x088, op_1f_088 },
	{ 0x08a, op_1f_08a },
	{ 0x090, op_1f_090 },
	{ 0x092, op_1f_092 },
	{ 0x096, op_1f_096 },
	{ 0x097, op_1f_097 },
	{ 0x0a3, op_1f_0a3 },
	{ 0x0b7, op_1f_0b7 },
	{ 0x0c8, op_1f_0c8 },
	{ 0x0ca, op_1f_0ca },
	{ 0x0d7, op_1f_0d7 },
	{ 0x0e8, op_1f_0e8 },
	{ 0x0ea, op_1f_0ea },
	{ 0x0eb, op_1f_0eb },
	{ 0x0f6, op_1f_0f6 },
	{ 0x0f7, op_1f_0f7 },
	{ 0x106, op_1f_106 },
	{ 0x10a, op_1f_10a },
	{ 0x116, op_1f_116 },
	{ 0x117, op_1f_117 },
	{ 0x11c, op_1f_11c },
	{ 0x137, op_1f_137 },
	{ 0x13c, op_1f_13c },
	{ 0x143, op_1f_143 },
	{ 0x153, op_1f_153 },
	{ 0x157, op_1f_157 },
	{ 0x172, op_1f_172 },
	{ 0x173, op_1f_173 },
	{ 0x177, op_1f_177 },
	{ 0x197, op_1f_197 },
	{ 0x19c, op_1f_19c },
	{ 0x1b7, op_1f_1b7 },
	{ 0x1bc, op_1f_1bc },
	{ 0x1c3, op_1f_1c3 },
	{ 0x1c6, op_1f_1c6 },
	{ 0x1cb, op_1f_1cb },
	{ 0x1d3, op_1f_1d3 },
	{ 0x1d6, op_1f_1d6 },
	{ 0x1dc, op_1f_1dc },
	{ 0x1eb, op_1f_1eb },
	{ 0x200, op_1f_200 },
	{ 0x208, op_1f_208 },
	{ 0x20a, op_1f_20a },
	{ 0x215, op_1f_215 },
	{ 0x216, op_1f_216 },
	{ 0x217, op_1f_217 },
	{ 0x218, op_1f_218 },
	{ 0x228, op_1f_228 },
	{ 0x236, op_1f_236 },
	{ 0x237, op_1f_237 },
	{ 0x255, op_1f_255 },
	{ 0x256, op_1f_256 },
	{ 0x257, op_1f_257 },
	{ 0x268, op_1f_268 },
	{ 0x277, op_1f_277 },
	{ 0x288, op_1f_288 },
	{ 0x28a, op_1f_28a },
	{ 0x295, op_1f_295 },
	{ 0x296, op_1f_296 },
	{ 0x297, op_1f_297 },
	{ 0x2b7, op_1f_2b7 },
	{ 0x2c8, op_1f_2c8 },
	{ 0x2ca, op_1f_2ca },
	{ 0x2d5, op_1f_2d5 },
	{ 0x2d7, op_1f_2d7 },
	{ 0x2e8, op_1f_2e8 },
	{ 0x2ea, op_1f_2ea },
	{ 0x2eb, op_1f_2eb },
	{ 0x2f6, op_1f_2f6 },
	{ 0x2f7, op_1f_2f7 },
	{ 0x30a, op_1f_30a },
	{ 0x316, op_1f_316 },
	{ 0x318, op_1f_318 },
	{ 0x338, op_1f_338 },
	{ 0x356, op_1f_356 },
	{ 0x392, op_1f_392 },
	{ 0x396, op_1f_396 },
	{ 0x39a, op_1f_39a },
	{ 0x3b2, op_1f_3b2 },
	{ 0x3ba, op_1f_3ba },
	{ 0x3c6, op_1f_3c6 },
	{ 0x3cb, op_1f_3cb },
	{ 0x3d2, op_1f_3d2 },
	{ 0x3d6, op_1f_3d6 },
	{ 0x3eb, op_1f_3eb },
	{ 0x3f6, op_1f_3f6 },
	{ 0x000, NULL}
};

void p405_set_opcode1f (p405_t *c)
{
	unsigned           i;
	p405_opcode_list_t *lst;
	p405_opcode_f      *p;

	c->opcodes.op[0x1f] = op_1f;

	p = c->opcodes.op1f;

	for (i = 0; i < 1024; i++) {
		p[i] = p405_op_undefined;
	}

	lst = p405_opcodes_1f;

	while (lst->fct != NULL) {
		p[lst->op] = lst->fct;
		lst += 1;
	}
}
