/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/ppc405/opcodes.c                                   *
 * Created:       2003-11-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-02-20 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include "ppc405.h"
#include "internal.h"


int p405_check_reserved (p405_t *c, uint32_t res)
{
  if (c->ir & res) {
    p405_op_undefined (c);
    return (1);
  }

  return (0);
}

int p405_check_privilege (p405_t *c)
{
  if (p405_get_msr_pr (c)) {
    p405_exception_program (c, P405_ESR_PPR);
    return (1);
  }

  return (0);
}

void p405_set_xer_so_ov (p405_t *c, uint64_t r1, uint32_t r2)
{
  int c1, c2;

  c1 = (r1 & ~0xffffffffULL) != 0;
  c2 = (r2 & 0x80000000UL) != 0;

  if (c1 != c2) {
    c->xer |= (P405_XER_SO | P405_XER_OV);
  }
  else {
    c->xer &= ~P405_XER_OV;
  }
}

void p405_set_xer_oflow (p405_t *c, int of)
{
  if (of) {
    c->xer |= (P405_XER_SO | P405_XER_OV);
  }
  else {
    c->xer &= ~P405_XER_OV;
  }
}

void p405_set_cr0 (p405_t *c, uint32_t r)
{
  c->cr &= 0x0fffffffUL;

  if (r & 0x80000000UL) {
    c->cr |= P405_CR0_LT;
  }
  else if (r & 0x7fffffffUL) {
    c->cr |= P405_CR0_GT;
  }
  else {
    c->cr |= P405_CR0_EQ;
  }

  if (c->xer & P405_XER_SO) {
    c->cr |= P405_CR0_SO;
  }
}

int p405_get_ea (p405_t *c, uint32_t *val, unsigned flags)
{
  if (flags & P405_EA_UPD) {
    if (p405_get_ir_ra (c->ir) == 0) {
      p405_op_undefined (c);
      return (1);
    }
  }

  if (flags & P405_EA_IDX) {
    *val = (p405_get_ra0 (c, c->ir) + p405_get_rb (c, c->ir)) & 0xffffffffUL;
  }
  else {
    *val = (p405_get_ra0 (c, c->ir) + p405_sext (c->ir, 16)) & 0xffffffffUL;
  }

  return (0);
}

uint32_t p405_get_mask (unsigned mb, unsigned me)
{
  uint32_t msk;

  if (mb <= me) {
    msk = ((2UL << (me - mb)) - 1) << (31 - me);
  }
  else {
    msk = ((2UL << me) - 1) << (31 - me);
    msk |= (2UL << (31 - mb)) - 1;
  }

  return (msk);
}

uint64_t p405_mul (uint32_t s1, uint32_t s2)
{
  int      ss1, ss2;
  uint64_t ret;

  ss1 = (s1 & 0x80000000UL) != 0;
  ss2 = (s2 & 0x80000000UL) != 0;

  if (ss1) {
    s1 = (~s1 + 1) & 0xffffffffUL;
  }

  if (ss2) {
    s2 = (~s2 + 1) & 0xffffffffUL;
  }

  ret = (uint64_t) s1 * (uint64_t) s2;

  if (ss1 != ss2) {
    ret = (~ret + 1) & 0xffffffffffffffffULL;
  }

  return (ret);
}


/*****************************************************************************/

/* generic add rt, src1, src2 */
void p405_op_add (p405_t *c, uint32_t s1, uint32_t s2, int oe, int rc, int co, int ci)
{
  uint64_t rt64;
  uint32_t rt;

  rt64 = (uint64_t) s1 + (uint64_t) s2;

  if (ci) {
    rt64 += p405_get_xer_ca (c);
  }

  rt = rt64 & 0xffffffffUL;

  p405_set_rt (c, c->ir, rt);

  if (oe) {
    p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000UL);
  }

  if (co) {
    p405_set_xer_ca (c, rt64 > 0xffffffffUL);
  }

  if (rc) {
    p405_set_cr0 (c, rt);
  }

  p405_set_clk (c, 4, 1);
}

/* generic and ra, src1, src2 */
void p405_op_and (p405_t *c, uint32_t s1, uint32_t s2, int rc)
{
  uint32_t rt;

  rt = s1 & s2;

  p405_set_ra (c, c->ir, rt);

  if (rc) {
    p405_set_cr0 (c, rt);
  }

  p405_set_clk (c, 4, 1);
}

void p405_op_branch (p405_t *c, uint32_t dst, unsigned bo, unsigned bi, int aa, int lk)
{
  int bo0, bo1, bo2, bo3, crb;

  bo0 = (bo & 0x10) != 0;
  bo1 = (bo & 0x08) != 0;
  bo2 = (bo & 0x04) != 0;
  bo3 = (bo & 0x02) != 0;

  crb = (c->cr & (0x80000000UL >> bi)) != 0;

  if (bo2 == 0) {
    c->ctr = (c->ctr - 1) & 0xffffffffUL;
  }

  if (lk) {
    p405_set_lr (c, (c->pc + 4) & 0xffffffffUL);
  }

  if ((bo2 || ((c->ctr == 0) == bo3)) && (bo0 || (crb == bo1))) {
    if (aa) {
      c->pc = dst;
    }
    else {
      c->pc = (c->pc + dst) & 0xffffffffUL;
    }
  }
  else {
    c->pc = (c->pc + 4) & 0xffffffffUL;
  }

  p405_set_clk (c, 0, 1);
}

/* generic logical compare */
void p405_op_cmpl (p405_t *c, uint32_t s1, uint32_t s2, unsigned fld)
{
  unsigned val;

  val = 0;

  s1 &= 0xffffffffUL;
  s2 &= 0xffffffffUL;

  if (s1 < s2) {
    val |= P405_CR_LT;
  }
  else if (s1 > s2) {
    val |= P405_CR_GT;
  }
  else {
    val |= P405_CR_EQ;
  }

  if (c->xer & P405_XER_SO) {
    val |= P405_CR_SO;
  }

  fld = 4 * (7 - (fld & 0x07));

  c->cr &= ~(0x0fUL << fld);
  c->cr |= (uint32_t) val << fld;

  p405_set_clk (c, 4, 1);
}

/* generic cr bit op */
void p405_op_crop (p405_t *c, unsigned bt, unsigned ba, unsigned bb, unsigned booltab)
{
  unsigned msk;
  uint32_t mt, ma, mb;

  if (c->ir & P405_IR_RC) {
    p405_op_undefined (c);
    return;
  }

  mt = 0x80000000UL >> bt;
  ma = 0x80000000UL >> ba;
  mb = 0x80000000UL >> bb;

  msk = (c->cr & ma) ? 0x04 : 0x01;

  if (c->cr & mb) {
    msk = msk << 1;
  }

  if (booltab & msk) {
    c->cr |= mt;
  }
  else {
    c->cr &= ~mt;
  }

  p405_set_clk (c, 4, 1);
}

/* generic load string word */
void p405_op_lsw (p405_t *c, unsigned rt, unsigned ra, unsigned rb, uint32_t ea, unsigned cnt)
{
  unsigned i;
  unsigned shl, rfinal, clk;
  uint8_t  val;
  uint32_t msk;

  rfinal = (rt + (cnt + 3) / 4) & 0x1f;

  msk = 0;
  shl = 24;

  clk = ((ea & 0x03) == 0) ? 0 : 1;

  for (i = 0; i < cnt; i++) {
    if ((ea & 0x03) == 0) {
      clk += 1;
    }

    if (((rt != ra) && (rt != rb)) || (rt == rfinal)) {
      if (p405_dload8 (c, ea, &val)) {
        return;
      }

      p405_set_gpr (c, rt, (p405_get_gpr (c, rt) & msk) | ((uint32_t) val << shl));
    }

    if (shl == 0) {
      msk = 0;
      shl = 24;
      rt = (rt + 1) & 0x1f;
    }
    else {
      msk = (msk >> 8) | 0xff000000UL;
      shl -= 8;
    }

    ea += 1;
  }

  p405_set_clk (c, 4, (clk > 0) ? clk : 1);
}

/* generic store string word */
void p405_op_stsw (p405_t *c, unsigned rs, uint32_t ea, unsigned cnt)
{
  unsigned i;
  unsigned sh, clk;

  sh = 24;

  clk = ((ea & 0x03) == 0) ? 0 : 1;

  for (i = 0; i < cnt; i++) {
    if ((ea & 0x03) == 0) {
      clk += 1;
    }

    if (p405_dstore8 (c, ea, (p405_get_gpr (c, rs) >> sh) & 0xff)) {
      return;
    }

    if (sh == 0) {
      sh = 24;
      rs = (rs + 1) & 0x1f;
    }
    else {
      sh -= 8;
    }

    ea += 1;
  }

  p405_set_clk (c, 4, (clk > 0) ? clk : 1);
}

/* generic sub rt, src1, src2 */
void p405_op_sub (p405_t *c, uint32_t s1, uint32_t s2, int oe, int rc, int co, int ci)
{
  uint64_t rt64;
  uint32_t rt;

  s2 = ~s2 & 0xffffffffUL;

  rt64 = (uint64_t) s1 + (uint64_t) s2;

  if (ci) {
    rt64 += p405_get_xer_ca (c);
  }
  else {
    rt64 += 1;
  }

  rt = rt64 & 0xffffffffUL;

  p405_set_rt (c, c->ir, rt);

  if (oe) {
    p405_set_xer_oflow (c, (rt ^ s1) & (rt ^ s2) & 0x80000000UL);
  }

  if (co) {
    p405_set_xer_ca (c, rt64 > 0xffffffffUL);
  }

  if (rc) {
    p405_set_cr0 (c, rt);
  }

  p405_set_clk (c, 4, 1);
}

void p405_op_undefined (p405_t *c)
{
  p405_undefined (c);
  p405_exception_program (c, P405_ESR_PIL);
  p405_set_clk (c, 0, 1);
}

/*****************************************************************************/

/* 03: twi to, ra, simm16 */
static
void op_03 (p405_t *c)
{
  int      cond;
  unsigned to;
  uint32_t ra1, im1, ra2, im2;

  to = p405_bits (c->ir, 6, 5);
  ra1 = p405_get_ra (c, c->ir);
  im1 = p405_sext (c->ir, 16);

  ra2 = (ra1 + 0x80000000UL) & 0xffffffffUL;
  im2 = (im1 + 0x80000000UL) & 0xffffffffUL;

  if ((to & 0x10) && (ra2 < im2)) {
    cond = 1;
  }
  else if ((to & 0x08) && (ra2 > im2)) {
    cond = 1;
  }
  else if ((to & 0x04) && (ra1 == im2)) {
    cond = 1;
  }
  else if ((to & 0x02) && (ra1 < im1)) {
    cond = 1;
  }
  else if ((to & 0x01) && (ra1 > im1)) {
    cond = 1;
  }
  else {
    cond = 0;
  }

  if (cond) {
    p405_exception_program (c, P405_ESR_PTR);
  }
  else {
    p405_set_clk (c, 4, 1);
  }
}

/* 07: mulli rt, ra, simm16 */
static
void op_07 (p405_t *c)
{
  uint64_t rt;

  rt = p405_mul (p405_get_ra (c, c->ir), p405_sext (c->ir, 16));
  p405_set_rt (c, c->ir, rt & 0xffffffffUL);

  p405_set_clk (c, 4, 1);
}

/* 08: subfic rt, ra, simm16 */
static
void op_08 (p405_t *c)
{
  p405_op_sub (c, p405_sext (c->ir, 16), p405_get_ra (c, c->ir),
    0, 0, 1, 0
  );
}

/* 0A: cmpli bf, ra, imm16 */
static
void op_0a (p405_t *c)
{
  p405_op_cmpl (c,
    p405_get_ra (c, c->ir),
    p405_uext (c->ir, 16),
    (c->ir >> 23) & 0x07
  );
}

/* 0B: cmpi bf, ra, simm16 */
static
void op_0b (p405_t *c)
{
  p405_op_cmpl (c,
    p405_get_ra (c, c->ir) + 0x80000000UL,
    p405_sext (c->ir, 16) + 0x80000000UL,
    (c->ir >> 23) & 0x07
  );
}

/* 0C: addic rt, ra, simm16 */
static
void op_0c (p405_t *c)
{
  p405_op_add (c,
    p405_get_ra (c, c->ir), p405_sext (c->ir, 16),
    0, 0, 1, 0
  );
}

/* 0D: addic. rt, ra, simm16 */
static
void op_0d (p405_t *c)
{
  p405_op_add (c,
    p405_get_ra (c, c->ir), p405_sext (c->ir, 16),
    0, 1, 1, 0
  );
}

/* 0E: addi rt, ra0, simm16 */
static
void op_0e (p405_t *c)
{
  p405_op_add (c,
    p405_get_ra0 (c, c->ir), p405_sext (c->ir, 16),
    0, 0, 0, 0
  );
}

/* 0F: addis rt, ra0, imm16 */
static
void op_0f (p405_t *c)
{
  p405_op_add (c,
    p405_get_ra0 (c, c->ir), p405_uext (c->ir, 16) << 16,
    0, 0, 0, 0
  );
}

/* 10: bc/bca/bcl/bcla target */
static
void op_10 (p405_t *c)
{
  p405_op_branch (c,
    p405_sext (c->ir & 0xfffcUL, 16),
    (c->ir >> 21) & 0x1f, (c->ir >> 16) & 0x1f,
    (c->ir & P405_IR_AA) != 0, (c->ir & P405_IR_LK) != 0
  );
}

/* 11: sc */
static
void op_11 (p405_t *c)
{
  if (p405_check_reserved (c, 0x03fffffdUL)) {
    return;
  }

  p405_exception_syscall (c);

//  p405_set_clk (c, 0, 1);
}

/* 12: b/ba/bl/bla target */
static
void op_12 (p405_t *c)
{
  uint32_t li;

  li = p405_sext (c->ir, 26) & 0xfffffffcUL;

  if (c->ir & 0x01) {
    p405_set_lr (c, (c->pc + 4) & 0xffffffffUL);
  }

  if (c->ir & 0x02) {
    c->pc = li;
  }
  else {
    c->pc = (c->pc + li) & 0xffffffffUL;
  }

  p405_set_clk (c, 0, 1);
}

/* 14: rlwimi[.] ra, rs, sh, mb, me */
static
void op_14 (p405_t *c)
{
  uint32_t ra, rs, msk;
  unsigned sh, mb, me;

  ra = p405_get_ra (c, c->ir);
  rs = p405_get_rs (c, c->ir);
  sh = (c->ir >> 11) & 0x1f;
  mb = (c->ir >> 6) & 0x1f;
  me = (c->ir >> 1) & 0x1f;

  if (sh != 0) {
    rs = ((rs << sh) | (rs >> (32 - sh))) & 0xffffffffUL;
  }

  msk = p405_get_mask (mb, me);

  ra = (rs & msk) | (ra & ~msk);

  p405_set_ra (c, c->ir, ra);

  if (p405_get_ir_rc (c->ir)) {
    p405_set_cr0 (c, ra);
  }

  p405_set_clk (c, 4, 1);
}

/* 15: rlwinm[.] ra, rs, sh, mb, me */
static
void op_15 (p405_t *c)
{
  uint32_t ra, rs;
  unsigned sh, mb, me;

  ra = p405_get_ra (c, c->ir);
  rs = p405_get_rs (c, c->ir);
  sh = (c->ir >> 11) & 0x1f;
  mb = (c->ir >> 6) & 0x1f;
  me = (c->ir >> 1) & 0x1f;

  if (sh != 0) {
    rs = ((rs << sh) | (rs >> (32 - sh))) & 0xffffffffUL;
  }

  ra = rs & p405_get_mask (mb, me);

  p405_set_ra (c, c->ir, ra);

  if (p405_get_ir_rc (c->ir)) {
    p405_set_cr0 (c, ra);
  }

  p405_set_clk (c, 4, 1);
}

/* 17: rlwnm[.] ra, rs, rb, mb, me */
static
void op_17 (p405_t *c)
{
  uint32_t ra, rs;
  unsigned sh, mb, me;

  ra = p405_get_ra (c, c->ir);
  rs = p405_get_rs (c, c->ir);
  sh = p405_get_rb (c, c->ir) & 0x1f;
  mb = (c->ir >> 6) & 0x1f;
  me = (c->ir >> 1) & 0x1f;

  if (sh != 0) {
    rs = ((rs << sh) | (rs >> (32 - sh))) & 0xffffffffUL;
  }

  ra = rs & p405_get_mask (mb, me);

  p405_set_ra (c, c->ir, ra);

  if (p405_get_ir_rc (c->ir)) {
    p405_set_cr0 (c, ra);
  }

  p405_set_clk (c, 4, 1);
}

/* 18: ori ra, rs, uimm16 */
static
void op_18 (p405_t *c)
{
  p405_set_ra (c, c->ir, p405_get_rs (c, c->ir) | p405_uext (c->ir, 16));
  p405_set_clk (c, 4, 1);
}

/* 19: oris ra, rs, uimm16 */
static
void op_19 (p405_t *c)
{
  p405_set_ra (c, c->ir, p405_get_rs (c, c->ir) | (p405_uext (c->ir, 16) << 16));
  p405_set_clk (c, 4, 1);
}

/* 1A: xori ra, rs, uimm16 */
static
void op_1a (p405_t *c)
{
  p405_set_ra (c, c->ir, p405_get_rs (c, c->ir) ^ p405_uext (c->ir, 16));
  p405_set_clk (c, 4, 1);
}

/* 1B: xoris ra, rs, uimm16 */
static
void op_1b (p405_t *c)
{
  p405_set_ra (c, c->ir, p405_get_rs (c, c->ir) ^ (p405_uext (c->ir, 16) << 16));
  p405_set_clk (c, 4, 1);
}

/* 1C: andi. ra, rs, uimm16 */
static
void op_1c (p405_t *c)
{
  p405_op_and (c, p405_get_rs (c, c->ir), p405_uext (c->ir, 16), 1);
}

/* 1D: andis. ra, rs, uimm16 */
static
void op_1d (p405_t *c)
{
  p405_op_and (c, p405_get_rs (c, c->ir), p405_uext (c->ir, 16) << 16, 1);
}

/* 20: lwz rt, ra0, simm16 */
static
void op_20 (p405_t *c)
{
  uint32_t rt, ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  if (p405_dload32 (c, ea, &rt)) {
    return;
  }

  p405_set_rt (c, c->ir, rt);

  p405_set_clk (c, 4, 1);
}

/* 21: lwzu rt, ra, simm16 */
static
void op_21 (p405_t *c)
{
  uint32_t rt, ea;

  if (p405_get_ea (c, &ea, P405_EA_UPD)) {
    return;
  }

  if (p405_dload32 (c, ea, &rt)) {
    return;
  }

  p405_set_ra (c, c->ir, ea);
  p405_set_rt (c, c->ir, rt);

  p405_set_clk (c, 4, 1);
}

/* 22: lbz rt, ra0, simm16 */
static
void op_22 (p405_t *c)
{
  uint8_t  rt;
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  if (p405_dload8 (c, ea, &rt)) {
    return;
  }

  p405_set_rt (c, c->ir, p405_uext (rt, 8));

  p405_set_clk (c, 4, 1);
}

/* 23: lbzu rt, ra, simm16 */
static
void op_23 (p405_t *c)
{
  uint8_t  rt;
  uint32_t ea;

  if (p405_get_ea (c, &ea, P405_EA_UPD)) {
    return;
  }

  if (p405_dload8 (c, ea, &rt)) {
    return;
  }

  p405_set_ra (c, c->ir, ea);
  p405_set_rt (c, c->ir, p405_uext (rt, 8));

  p405_set_clk (c, 4, 1);
}

/* 24: stw rs, ra0, simm16 */
static
void op_24 (p405_t *c)
{
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  if (p405_dstore32 (c, ea, p405_get_rs (c, c->ir))) {
    return;
  }

  p405_set_clk (c, 4, 1);
}

/* 25: stwu rs, ra, simm16 */
static
void op_25 (p405_t *c)
{
  uint32_t ea;

  if (p405_get_ea (c, &ea, P405_EA_UPD)) {
    return;
  }

  if (p405_dstore32 (c, ea, p405_get_rs (c, c->ir))) {
    return;
  }

  p405_set_ra (c, c->ir, ea);

  p405_set_clk (c, 4, 1);
}

/* 26: stb rs, ra0, simm16 */
static
void op_26 (p405_t *c)
{
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  if (p405_dstore8 (c, ea, p405_uext (p405_get_rs (c, c->ir), 8))) {
    return;
  }

  p405_set_clk (c, 4, 1);
}

/* 27: stbu rs, ra, simm16 */
static
void op_27 (p405_t *c)
{
  uint32_t ea;

  if (p405_get_ea (c, &ea, P405_EA_UPD)) {
    return;
  }

  if (p405_dstore8 (c, ea, p405_uext (p405_get_rs (c, c->ir), 8))) {
    return;
  }

  p405_set_ra (c, c->ir, ea);

  p405_set_clk (c, 4, 1);
}

/* 28: lhz rt, ra0, simm16 */
static
void op_28 (p405_t *c)
{
  uint16_t rt;
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  if (p405_dload16 (c, ea, &rt)) {
    return;
  }

  p405_set_rt (c, c->ir, p405_uext (rt, 16));

  p405_set_clk (c, 4, 1);
}

/* 29: lhzu rt, ra, simm16 */
static
void op_29 (p405_t *c)
{
  uint16_t rt;
  uint32_t ea;

  if (p405_get_ea (c, &ea, P405_EA_UPD)) {
    return;
  }

  if (p405_dload16 (c, ea, &rt)) {
    return;
  }

  p405_set_ra (c, c->ir, ea);
  p405_set_rt (c, c->ir, p405_uext (rt, 16));

  p405_set_clk (c, 4, 1);
}

/* 2A: lha rt, ra0, simm16 */
static
void op_2a (p405_t *c)
{
  uint16_t rt;
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  if (p405_dload16 (c, ea, &rt)) {
    return;
  }

  p405_set_rt (c, c->ir, p405_sext (rt, 16));

  p405_set_clk (c, 4, 1);
}

/* 2B: lhau rt, ra, simm16 */
static
void op_2b (p405_t *c)
{
  uint16_t rt;
  uint32_t ea;

  if (p405_get_ea (c, &ea, P405_EA_UPD)) {
    return;
  }

  if (p405_dload16 (c, ea, &rt)) {
    return;
  }

  p405_set_ra (c, c->ir, ea);
  p405_set_rt (c, c->ir, p405_sext (rt, 16));

  p405_set_clk (c, 4, 1);
}

/* 2C: sth rs, ra0, simm16 */
static
void op_2c (p405_t *c)
{
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  if (p405_dstore16 (c, ea, p405_uext (p405_get_rs (c, c->ir), 16))) {
    return;
  }

  p405_set_clk (c, 4, 1);
}

/* 2D: sthu rs, ra, simm16 */
static
void op_2d (p405_t *c)
{
  uint32_t ea;

  if (p405_get_ea (c, &ea, P405_EA_UPD)) {
    return;
  }

  if (p405_dstore16 (c, ea, p405_uext (p405_get_rs (c, c->ir), 16))) {
    return;
  }

  p405_set_ra (c, c->ir, ea);

  p405_set_clk (c, 4, 1);
}

/* 2E: lmw rt, ra, simm16 */
static
void op_2e (p405_t *c)
{
  unsigned rt, ra, cnt;
  uint32_t val;
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  rt = p405_get_ir_rt (c->ir);
  ra = p405_get_ir_ra (c->ir);

  if (ra >= rt) {
    p405_op_undefined (c);
    return;
  }

  cnt = 32 - rt;

  while (rt <= 31) {
    if ((rt != ra) || (rt == 31)) {
      if (p405_dload32 (c, ea, &val)) {
        return;
      }

      p405_set_gpr (c, rt, val);
    }

    rt += 1;
    ea += 4;
  }

  p405_set_clk (c, 4, cnt);
}

/* 2F: stmw rs, ra, simm16 */
static
void op_2f (p405_t *c)
{
  unsigned rs, cnt;
  uint32_t ea;

  if (p405_get_ea (c, &ea, 0)) {
    return;
  }

  rs = p405_get_ir_rs (c->ir);

  cnt = 32 - rs;

  while (rs <= 31) {
    if (p405_dstore32 (c, ea, p405_get_gpr (c, rs))) {
      return;
    }

    rs += 1;
    ea += 4;
  }

  p405_set_clk (c, 4, cnt);
}


static
p405_opcode_f p405_opcodes[64] = {
    NULL,   NULL,   NULL, &op_03,   NULL,   NULL,   NULL, &op_07,   /* 00 */
  &op_08,   NULL, &op_0a, &op_0b, &op_0c, &op_0d, &op_0e, &op_0f,
  &op_10, &op_11, &op_12,   NULL, &op_14, &op_15,   NULL, &op_17,   /* 10 */
  &op_18, &op_19, &op_1a, &op_1b, &op_1c, &op_1d,   NULL,   NULL,
  &op_20, &op_21, &op_22, &op_23, &op_24, &op_25, &op_26, &op_27,   /* 20 */
  &op_28, &op_29, &op_2a, &op_2b, &op_2c, &op_2d, &op_2e, &op_2f,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   /* 30 */
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL
};


void p405_set_opcodes (p405_t *c)
{
  unsigned i;

  for (i = 0; i < 64; i++) {
    if (p405_opcodes[i] != NULL) {
      c->opcodes.op[i] = p405_opcodes[i];
    }
    else {
      c->opcodes.op[i] = &p405_op_undefined;
    }
  }

  p405_set_opcode13 (c);
  p405_set_opcode1f (c);
}
