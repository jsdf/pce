/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/arm/opcodes.c                                      *
 * Created:       2004-11-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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


#include "arm.h"
#include "internal.h"


int arm_is_privileged (arm_t *c)
{
  if (arm_get_cpsr_m (c) == ARM_MODE_USR) {
    return (0);
  }

  return (1);
}

unsigned arm_bitcnt32 (unsigned long v)
{
  unsigned n;

  n = 0;
  while (v != 0) {
    n += 1;
    v &= v - 1;
  }

  return (n);
}

void arm_op_undefined (arm_t *c)
{
  arm_undefined (c);
  arm_set_clk (c, 0, 1);

  arm_exception_undefined (c);
}

int arm_write_cpsr (arm_t *c, uint32_t val)
{
  if (arm_get_cpsr_m (c) == ARM_MODE_USR) {
    return (1);
  }

  arm_set_reg_map (c, val & 0x1f);
  arm_set_cpsr (c, val);

  return (0);
}

void arm_set_cc_log (arm_t *c, uint32_t val)
{
  uint32_t cc;

  val &= 0xffffffffUL;
  cc = 0;

  if (val == 0) {
    cc |= ARM_PSR_Z;
  }
  else if (val & 0x80000000UL) {
    cc |= ARM_PSR_N;
  }

  c->cpsr &= ~(ARM_PSR_Z | ARM_PSR_N);
  c->cpsr |= cc;
}

void arm_set_cc_add (arm_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
  arm_set_cc_log (c, d);
  arm_set_cc_c (c, (d < s1) || ((d == s1) && (s2 != 0)));
  arm_set_cc_v (c, ~(s1 ^ s2) & (s1 ^ d) & 0x80000000UL);
}

void arm_set_cc_sub (arm_t *c, uint32_t d, uint32_t s1, uint32_t s2)
{
  arm_set_cc_log (c, d);
  arm_set_cc_c (c, !((d > s1) || ((d == s1) && (s2 != 0))));
  arm_set_cc_v (c, (s1 ^ s2) & (s1 ^ d) & 0x80000000UL);
}

uint32_t arm_set_psr_field (uint32_t psr, uint32_t val, unsigned fld)
{
  if (fld & 0x01) {
    psr &= 0xffffff00UL;
    psr |= val & 0x000000ffUL;
  }

  if (fld & 0x02) {
    psr &= 0xffff00ffUL;
    psr |= val & 0x0000ff00UL;
  }

  if (fld & 0x04) {
    psr &= 0xff00ffffUL;
    psr |= val & 0x00ff0000UL;
  }

  if (fld & 0x08) {
    psr &= 0x00ffffffUL;
    psr |= val & 0xff000000UL;
  }

  return (psr);
}

int arm_check_cond (arm_t *c, unsigned cond)
{
  switch (cond & 0x0f) {
  case 0x00: /* eq */
    return (arm_get_cc_z (c));

  case 0x01: /* ne */
    return (!arm_get_cc_z (c));

  case 0x02: /* cs */
    return (arm_get_cc_c (c));

  case 0x03: /* cc */
    return (!arm_get_cc_c (c));

  case 0x04: /* mi */
    return (arm_get_cc_n (c));

  case 0x05: /* pl */
    return (!arm_get_cc_n (c));

  case 0x06: /* vs */
    return (arm_get_cc_v (c));

  case 0x07: /* vc */
    return (!arm_get_cc_v (c));

  case 0x08: /* hi */
    return (arm_get_cc_c (c) && !arm_get_cc_z (c));

  case 0x09: /* ls */
    return (!arm_get_cc_c (c) || arm_get_cc_z (c));

  case 0x0a: /* ge */
    return (arm_get_cc_n (c) == arm_get_cc_v (c));

  case 0x0b: /* lt */
    return (arm_get_cc_n (c) != arm_get_cc_v (c));

  case 0x0c: /* gt */
    return (!arm_get_cc_z (c) && (arm_get_cc_n (c) == arm_get_cc_v (c)));

  case 0x0d: /* le */
    return (arm_get_cc_z (c) || (arm_get_cc_n (c) != arm_get_cc_v (c)));

  case 0x0e: /* al */
    return (1);

  case 0x0f: /* nv */
    return (0);
  }

  return (0);
}

uint32_t arm_get_sh (arm_t *c, uint32_t ir, uint32_t *cry)
{
  unsigned n;
  uint32_t v, tmp;

  if (cry == NULL) {
    cry = &tmp;
  }

  if (arm_get_bit (ir, 25)) {
    /* 32 bit immediate */

    v = (ir & 0xff);
    n = (ir >> 7) & 0x1e;

    if (n == 0) {
      *cry = arm_get_cc_c (c);
    }
    else {
      v = ((v >> n) | (v << (32 - n))) & 0xffffffffUL;
      *cry = ((v & 0x80000000UL) != 0);
    }

    return (v);
  }

  v = arm_get_rm (c, c->ir);

  if (arm_get_bit (ir, 4)) {
    /* register shifts */

    if (arm_get_bit (ir, 7)) {
      /* extension */
      *cry = 0;
      return (0);
    }

    n = arm_get_rs (c, c->ir);

    switch (arm_get_bits (ir, 5, 2)) {
    case 0x00: /* lsl */
      if (n == 0) {
        *cry = arm_get_cc_c (c);
      }
      else {
        *cry = (v >> (32 - n)) & 0x01;
        v = (v << n) & 0xffffffffUL;
      }
      return (v);

    case 0x01: /* lsr */
      if (n == 0) {
        *cry = (v >> 31) & 0x01;
        v = 0;
      }
      else {
        *cry = (v >> (n - 1)) & 0x01;
        v = v >> n;
      }
      return (v);

    case 0x02: /* asr */
      if (n == 0) {
        *cry = (v >> 31) & 0x01;
        v = *cry ? 0xffffffffUL : 0x00000000UL;
      }
      else {
        *cry = (v >> (n - 1)) & 0x01;
        v = arm_asr32 (v, n);
      }
      return (v);

    case 0x03: /* ror */
      if (n == 0) {
        *cry = v & 0x01;
        v = (v >> 1) | (arm_get_cc_c (c) << 31);
      }
      else {
        *cry = (v >> (n - 1)) & 0x01;
        v = (v >> n) | (v << (32 - n));
      }
      return (v);
    }
  }
  else {
    /* immediate shifts */

    n = arm_get_bits (ir, 7, 5);

    switch (arm_get_bits (ir, 5, 2)) {
    case 0x00: /* lsl */
      if (n == 0) {
        *cry = arm_get_cc_c (c);
      }
      else if (n <= 32) {
        *cry = (v >> (32 - n)) & 0x01;
        v = (n < 32) ? (v << n) : 0;
      }
      else {
        *cry = 0;
        v = 0;
      }
      return (v);

    case 0x01: /* lsr */
      if (n == 0) {
        *cry = arm_get_cc_c (c);
      }
      else if (n <= 32) {
        *cry = (v >> (n - 1)) & 0x01;
        v = (n < 32) ? (v >> n) : 0;
      }
      else {
        *cry = 0;
        v = 0;
      }
      return (v);

    case 0x02: /* asr */
      if (n == 0) {
        *cry = arm_get_cc_c (c);
      }
      else if (n < 32) {
        *cry = (v >> (n - 1)) & 0x01;
        v = v >> n;
        if (v & 0x80000000UL) {
          v |= (0xffffffffUL << (32 - n)) & 0xffffffffUL;
        }
      }
      else {
        *cry = (v >> 31) & 0x01;
        v = *cry ? 0xffffffffUL : 0x00000000UL;
      }
      return (v);

    case 0x03: /* ror */
      if (n == 0) {
        *cry = arm_get_cc_c (c);
      }
      else if ((n & 0x1f) == 0) {
        *cry = (v >> 31) & 0x01;
      }
      else {
        n &= 0x1f;
        *cry = (v >> (n - 1)) & 0x01;
        v = (v >> n) | (v << (32 - n));
      }
      return (v);
    }
  }

  *cry = 0;

  return (0);
}


/*****************************************************************************/

/* 00 09: mul[cond] rn, rm, rs */
static
void op00_09 (arm_t *c)
{
  uint32_t d, s1, s2;

  s1 = arm_get_rm (c, c->ir);
  s2 = arm_get_rs (c, c->ir);

  d = (s1 * s2) & 0xffffffffUL;

  arm_set_rn (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 00 ext */
static
void op00_ext (arm_t *c)
{
  switch (arm_get_shext (c->ir)) {
  case 0x09:
    op00_09 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 00: and[cond] rd, rn, shifter_operand */
static
void op00 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    op00_ext (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = s1 & s2;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 01 09: mul[cond]s rn, rm, rs */
static
void op01_09 (arm_t *c)
{
  uint32_t d, s1, s2;

  s1 = arm_get_rm (c, c->ir);
  s2 = arm_get_rs (c, c->ir);

  d = (s1 * s2) & 0xffffffffUL;

  arm_set_rn (c, c->ir, d);
  arm_set_cc_log (c, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 01 ext */
static
void op01_ext (arm_t *c)
{
  switch (arm_get_shext (c->ir)) {
  case 0x09:
    op01_09 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 01: and[cond]s rd, rn, shifter_operand */
static
void op01 (arm_t *c)
{
  uint32_t s1, s2, d, shc;

  if (arm_is_shext (c->ir)) {
    op01_ext (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, &shc);

  d = s1 & s2;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_log (c, d);
    arm_set_cc_c (c, shc);
    arm_set_clk (c, 4, 1);
  }
}


/* 02 09: mla[cond] rn, rm, rs, rd */
static
void op02_09 (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  s1 = arm_get_rm (c, c->ir);
  s2 = arm_get_rs (c, c->ir);
  s3 = arm_get_rd (c, c->ir);

  d = (s1 * s2 + s3) & 0xffffffffUL;

  arm_set_rn (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 02 ext */
static
void op02_ext (arm_t *c)
{
  switch (arm_get_shext (c->ir)) {
  case 0x09:
    op02_09 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 02: eor[cond] rd, rn, shifter_operand */
static
void op02 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    op02_ext (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = s1 ^ s2;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 03 09: mla[cond]s rn, rm, rs, rd */
static
void op03_09 (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  s1 = arm_get_rm (c, c->ir);
  s2 = arm_get_rs (c, c->ir);
  s3 = arm_get_rd (c, c->ir);

  d = (s1 * s2 + s3) & 0xffffffffUL;

  arm_set_rn (c, c->ir, d);
  arm_set_cc_log (c, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 03 ext */
static
void op03_ext (arm_t *c)
{
  switch (arm_get_shext (c->ir)) {
  case 0x09:
    op03_09 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 03: eor[cond]s rd, rn, shifter_operand */
static
void op03 (arm_t *c)
{
  uint32_t s1, s2, d, shc;

  if (arm_is_shext (c->ir)) {
    op03_ext (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, &shc);

  d = s1 ^ s2;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_log (c, d);
    arm_set_cc_c (c, shc);
    arm_set_clk (c, 4, 1);
  }
}

/* 04: sub[cond] rd, rn, shifter_operand */
static
void op04 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = (s1 - s2) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 05: sub[cond]s rd, rn, shifter_operand */
static
void op05 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = (s1 - s2) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_sub (c, d, s1, s2);
    arm_set_clk (c, 4, 1);
  }
}

/* 06: rsb[cond] rd, rn, shifter_operand */
static
void op06 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s2 = arm_get_rn (c, c->ir);
  s1 = arm_get_sh (c, c->ir, NULL);

  d = (s1 - s2) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 07: rsb[cond]s rd, rn, shifter_operand */
static
void op07 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s2 = arm_get_rn (c, c->ir);
  s1 = arm_get_sh (c, c->ir, NULL);

  d = (s1 - s2) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_sub (c, d, s1, s2);
    arm_set_clk (c, 4, 1);
  }
}

/* 08: add[cond] rd, rn, shifter_operand */
static
void op08 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = (s1 + s2) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 09: add[cond]s rd, rn, shifter_operand */
static
void op09 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = (s1 + s2) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_add (c, d, s1, s2);
    arm_set_clk (c, 4, 1);
  }
}

/* 0A: adc[cond] rd, rn, shifter_operand */
static
void op0a (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);
  s3 = arm_get_cc_c (c);

  d = (s1 + s2 + s3) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 0B: adc[cond]s rd, rn, shifter_operand */
static
void op0b (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);
  s3 = arm_get_cc_c (c);

  d = (s1 + s2 + s3) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_add (c, d, s1, s2);
    arm_set_clk (c, 4, 1);
  }
}

/* 0C: sbc[cond] rd, rn, shifter_operand */
static
void op0c (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);
  s3 = !arm_get_cc_c (c);

  d = (s1 - s2 - s3) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 0D: sbc[cond]s rd, rn, shifter_operand */
static
void op0d (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);
  s3 = !arm_get_cc_c (c);

  d = (s1 - s2 - s3) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_sub (c, d, s1, s2);
    arm_set_clk (c, 4, 1);
  }
}

/* 0E: rsc[cond] rd, rn, shifter_operand */
static
void op0e (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s2 = arm_get_rn (c, c->ir);
  s1 = arm_get_sh (c, c->ir, NULL);
  s3 = !arm_get_cc_c (c);

  d = (s1 - s2 - s3) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 0F: rsc[cond]s rd, rn, shifter_operand */
static
void op0f (arm_t *c)
{
  uint32_t s1, s2, s3, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s2 = arm_get_rn (c, c->ir);
  s1 = arm_get_sh (c, c->ir, NULL);
  s3 = !arm_get_cc_c (c);

  d = (s1 - s2 - s3) & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_sub (c, d, s1, s2);
    arm_set_clk (c, 4, 1);
  }
}

/* 10 00: mrs[cond] rd, cpsr */
static
void op10_00 (arm_t *c)
{
  arm_set_rd (c, c->ir, arm_get_cpsr (c));
  arm_set_clk (c, 4, 1);
}

/* 10 09: swp[cond] rd, rm, rn */
static
void op10_09 (arm_t *c)
{
  uint32_t addr, d, s;

  addr = arm_get_rn (c, c->ir);
  s = arm_get_rm (c, c->ir);

  if (arm_dload32 (c, addr & 0xfffffffcUL, &d)) {
    arm_exception_data_abort (c);
    return;
  }

  if (addr & 0x03) {
    d = arm_ror32 (d, (addr & 0x03) << 3);
  }

  if (arm_dstore32 (c, addr & 0xfffffffcUL, s)) {
    arm_exception_data_abort (c);
    return;
  }

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 10 */
static
void op10 (arm_t *c)
{
  switch (arm_get_bits (c->ir, 4, 4)) {
  case 0x00:
    op10_00 (c);
    break;

  case 0x09:
    op10_09 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 11: tst[cond] rn, shifter_operand */
static
void op11 (arm_t *c)
{
  uint32_t s1, s2, d, shc;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, &shc);

  d = s1 & s2;

  arm_set_cc_log (c, d);
  arm_set_cc_c (c, shc);
  arm_set_clk (c, 4, 1);
}

/* 12 00: msr[cond] fields, rm */
static
void op12_00 (arm_t *c)
{
  unsigned fld;
  uint32_t val;

  fld = arm_get_bits (c->ir, 16, 4);

  if (arm_is_privileged (c) == 0) {
    fld &= 0x08;
  }

  val = arm_get_rm (c, c->ir);

  if (arm_get_bit (c->ir, 22)) {
    arm_set_spsr (c, arm_set_psr_field (arm_get_spsr (c), val, fld));
  }
  else {
    arm_write_cpsr (c, arm_set_psr_field (arm_get_cpsr (c), val, fld));
  }

  arm_set_clk (c, 4, 1);
}

/* 12 07: bkpt uimm16 */
static
void op12_07 (arm_t *c)
{
  arm_set_clk (c, 0, 1);
  arm_exception_prefetch_abort (c);
}

/* 12 */
static
void op12 (arm_t *c)
{
  switch (c->ir & 0x0ff000f0UL) {
  case 0x01200000UL:
    op12_00 (c);
    break;

  case 0x01200070UL:
    op12_07 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 13: teq[cond] rn, shifter_operand */
static
void op13 (arm_t *c)
{
  uint32_t s1, s2, d, shc;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, &shc);

  d = s1 ^ s2;

  arm_set_cc_log (c, d);
  arm_set_cc_c (c, shc);
  arm_set_clk (c, 4, 1);
}

/* 14 00: mrs[cond] rd, spsr */
static
void op14_00 (arm_t *c)
{
  arm_set_rd (c, c->ir, arm_get_spsr (c));
  arm_set_clk (c, 4, 1);
}

/* 14 09: swp[cond]b rd, rm, rn */
static
void op14_09 (arm_t *c)
{
  uint32_t addr, s;
  uint8_t  d;

  addr = arm_get_rn (c, c->ir);
  s = arm_get_rm (c, c->ir);

  if (arm_dload8 (c, addr, &d)) {
    arm_exception_data_abort (c);
    return;
  }

  if (arm_dstore8 (c, addr, s & 0xff)) {
    arm_exception_data_abort (c);
    return;
  }

  arm_set_rd (c, c->ir, d & 0xff);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 14 */
static
void op14 (arm_t *c)
{
  switch (arm_get_bits (c->ir, 4, 4)) {
  case 0x00:
    op14_00 (c);
    break;

  case 0x09:
    op14_09 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 15: cmp[cond] rn, shifter_operand */
static
void op15 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = (s1 - s2) & 0xffffffffUL;

  arm_set_cc_sub (c, d, s1, s2);
  arm_set_clk (c, 4, 1);
}

/* 16 01: clz rd, rm */
static
void op16_01 (arm_t *c)
{
  if (arm_get_bits (c->ir, 4, 4) == 0x01) {
    uint32_t d, s;

    s = arm_get_rm (c, c->ir);

    if (s == 0) {
      d = 32;
    }
    else {
      d = 0;
      while ((s & 0x80000000UL) == 0) {
        d += 1;
        s <<= 1;
      }
    }

    arm_set_rd (c, c->ir, d);
    arm_set_clk (c, 4, 1);
  }
  else {
    arm_op_undefined (c);
  }
}

/* 16 */
static
void op16 (arm_t *c)
{
  switch (c->ir & 0x0ff000f0UL) {
  case 0x01600000UL:
    op12_00 (c);
    break;

  case 0x01600010UL:
    op16_01 (c);
    break;

  default:
    arm_op_undefined (c);
    break;
  }
}

/* 17: cmn[cond] rn, shifter_operand */
static
void op17 (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = (s1 + s2) & 0xffffffffUL;

  arm_set_cc_add (c, d, s1, s2);
  arm_set_clk (c, 4, 1);
}

/* 18: orr[cond] rd, rn, shifter_operand */
static
void op18 (arm_t *c)
{
  uint32_t d, s1, s2;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = s1 | s2;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 19: orr[cond]s rd, rn, shifter_operand */
static
void op19 (arm_t *c)
{
  uint32_t s1, s2, d, shc;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, &shc);

  d = s1 | s2;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_log (c, d);
    arm_set_cc_c (c, shc);
    arm_set_clk (c, 4, 1);
  }
}

/* 1A: mov[cond] rd, shifter_operand */
static
void op1a (arm_t *c)
{
  uint32_t d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  d = arm_get_sh (c, c->ir, NULL);

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 1B: mov[cond]s rd, shifter_operand */
static
void op1b (arm_t *c)
{
  uint32_t d, shc;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  d = arm_get_sh (c, c->ir, &shc);

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_log (c, d);
    arm_set_cc_c (c, shc);
    arm_set_clk (c, 4, 1);
  }
}

/* 1c: bic[cond] rd, rn, shifter_operand */
static
void op1c (arm_t *c)
{
  uint32_t s1, s2, d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, NULL);

  d = s1 & ~s2;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 1d: bic[cond]s rd, rn, shifter_operand */
static
void op1d (arm_t *c)
{
  uint32_t s1, s2, d, shc;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  s1 = arm_get_rn (c, c->ir);
  s2 = arm_get_sh (c, c->ir, &shc);

  d = s1 & ~s2;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_log (c, d);
    arm_set_cc_c (c, shc);
    arm_set_clk (c, 4, 1);
  }
}

/* 1e: mvn[cond] rd, shifter_operand */
static
void op1e (arm_t *c)
{
  uint32_t d;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  d = arm_get_sh (c, c->ir, NULL);

  d = ~d & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);
  arm_set_clk (c, arm_rd_is_pc (c->ir) ? 0 : 4, 1);
}

/* 1f: mvn[cond]s rd, shifter_operand */
static
void op1f (arm_t *c)
{
  uint32_t d, shc;

  if (arm_is_shext (c->ir)) {
    arm_op_undefined (c);
    return;
  }

  d = arm_get_sh (c, c->ir, &shc);

  d = ~d & 0xffffffffUL;

  arm_set_rd (c, c->ir, d);

  if (arm_rd_is_pc (c->ir)) {
    arm_write_cpsr (c, arm_get_spsr (c));
    arm_set_clk (c, 0, 1);
  }
  else {
    arm_set_cc_log (c, d);
    arm_set_cc_c (c, shc);
    arm_set_clk (c, 4, 1);
  }
}

/* 32: msr[cond] fields, #imm */
static
void op32 (arm_t *c)
{
  unsigned fld, rot;
  uint32_t val;

  fld = arm_get_bits (c->ir, 16, 4);
  rot = 2 * arm_get_bits (c->ir, 8, 4);

  if (arm_is_privileged (c) == 0) {
    fld &= 0x08;
  }

  val = arm_get_bits (c->ir, 0, 8);
  val = arm_ror32 (val, rot);

  if (arm_get_bit (c->ir, 22)) {
    arm_set_spsr (c, arm_set_psr_field (arm_get_spsr (c), val, fld));
  }
  else {
    arm_write_cpsr (c, arm_set_psr_field (arm_get_cpsr (c), val, fld));
  }

  arm_set_clk (c, 4, 1);
}

/* 40: ldr/str[cond][b][t] rd, [rn], address */
static
void op40 (arm_t *c)
{
  int      p, u, b, w, l, t;
  uint32_t val, base, index;

  p = arm_get_bit (c->ir, 24);
  u = arm_get_bit (c->ir, 23);
  b = arm_get_bit (c->ir, 22);
  w = arm_get_bit (c->ir, 21);
  l = arm_get_bit (c->ir, 20);
  t = (p == 0) && (w == 1);

  base = arm_get_rn (c, c->ir);

  /* get index */
  if (arm_get_bit (c->ir, 25)) {
    unsigned n;

    index = arm_get_rm (c, c->ir);
    n = arm_get_bits (c->ir, 7, 5);

    switch (arm_get_bits (c->ir, 5, 2)) {
    case 0x00: /* lsl */
      index = index << n;
      break;

    case 0x01: /* lsr */
      index = (n == 0) ? 0 : (index >> n);
      break;

    case 0x02: /* asr */
      if (n == 0) {
        index = (index & 0x80000000UL) ? 0xffffffffUL : 0x00000000UL;
      }
      else {
        index = arm_asr32 (index, n);
      }
      break;

    case 0x03: /* ror / rrx */
      if (n == 0) {
        index = (index >> 1) | (arm_get_cc_c (c) << 31);
      }
      else {
        index = (index >> n) | (index << (32 - n));
      }
    }
  }
  else {
    index = arm_extu (c->ir, 12);
  }

  /* pre-index */
  if (p) {
    base = (base + ((u) ? (index) : (~index + 1))) & 0xffffffffUL;
  }

  if (l) {
    if (b) {
      uint8_t tmp;

      if (arm_dload8 (c, base, &tmp)) {
        arm_exception_data_abort (c);
        return;
      }

      val = tmp & 0xff;
    }
    else {
      if (arm_dload32 (c, base & 0xfffffffcUL, &val)) {
        arm_exception_data_abort (c);
        return;
      }

      if (base & 0x03) {
        val = arm_ror32 (val, (base & 0x03) << 3);
      }
    }

    arm_set_rd (c, c->ir, val);
  }
  else {
    val = arm_get_rd (c, c->ir);

    if (b) {
      if (arm_dstore8 (c, base, val)) {
        arm_exception_data_abort (c);
        return;
      }
    }
    else {
      if (arm_dstore32 (c, base, val)) {
        arm_exception_data_abort (c);
        return;
      }
    }
  }

  /* post-index */
  if (p == 0) {
    base = (base + ((u) ? (index) : (~index + 1))) & 0xffffffffUL;
  }

  /* base register writeback */
  if ((p == 0) || (w == 1)) {
    arm_set_rn (c, c->ir, base);
  }

  arm_set_clk (c, (l && arm_rd_is_pc (c->ir)) ? 0 : 4, 1);
}

/* 80: ldm/stm[cond][mode] rn[!], registers[^] */
static
void op80 (arm_t *c)
{
  int      p, u, s, w, l;
  unsigned i;
  unsigned regs, regn;
  unsigned mode;
  uint32_t addr1, addr2;
  uint32_t val;

  p = arm_get_bit (c->ir, 24);
  u = arm_get_bit (c->ir, 23);
  s = arm_get_bit (c->ir, 22);
  w = arm_get_bit (c->ir, 21);
  l = arm_get_bit (c->ir, 20);

  regs = arm_get_bits (c->ir, 0, 16);
  regn = arm_bitcnt32 (regs);
  mode = arm_get_cpsr_m (c);

  addr1 = arm_get_rn (c, c->ir);
  addr2 = u ? (addr1 + 4 * regn) : (addr1 - 4 * regn);

  if (u) {
    addr1 = addr1 + (p ? 4 : 0);
  }
  else {
    addr1 = addr2 + (p ? 0 : 4);
  }

  if (s && !(regs & 0x8000U)) {
    /* access user mode registers */
    arm_set_reg_map (c, ARM_MODE_USR);
  }

  for (i = 0; i < 16; i++) {
    if (regs & (1U << i)) {
      if (l) {
        if (arm_dload32 (c, addr1 & 0xfffffffcUL, &val)) {
          arm_exception_data_abort (c);
          return;
        }

        arm_set_reg (c, i, val);
      }
      else {
        val = arm_get_reg_pc (c, i, 8);

        if (arm_dstore32 (c, addr1 & 0xfffffffcUL, val)) {
          arm_exception_data_abort (c);
          return;
        }
      }

      addr1 += 4;
    }
  }

  if (w) {
    arm_set_rn (c, c->ir, addr2);
  }

  if (s) {
    if (regs & 0x8000U) {
      arm_write_cpsr (c, arm_get_spsr (c));
    }
    else {
      arm_set_reg_map (c, mode);
    }
  }

  arm_set_clk (c, (l && (regs & 0x8000U)) ? 0 : 4, 1);
}

/* A0: b[cond] target */
static
void opa0 (arm_t *c)
{
  uint32_t d;

  d = arm_exts (c->ir, 24);
  d = (arm_get_pc (c) + (d << 2) + 8) & 0xffffffffUL;

  arm_set_pc (c, d);

  arm_set_clk (c, 0, 1);
}

/* B0: bl[cond] target */
static
void opb0 (arm_t *c)
{
  uint32_t d;

  d = arm_exts (c->ir, 24);
  d = (arm_get_pc (c) + (d << 2) + 8) & 0xffffffffUL;

  arm_set_lr (c, (arm_get_pc (c) + 4) & 0xffffffffUL);
  arm_set_pc (c, d);

  arm_set_clk (c, 0, 1);
}

/* F0: swi[cond] immediate */
static
void opf0 (arm_t *c)
{
  arm_set_clk (c, 0, 1);

  arm_exception_swi (c);
}


static
arm_opcode_f arm_opcodes[256] = {
  op00, op01, op02, op03, op04, op05, op06, op07,  /* 00 */
  op08, op09, op0a, op0b, op0c, op0d, op0e, op0f,
  op10, op11, op12, op13, op14, op15, op16, op17,  /* 10 */
  op18, op19, op1a, op1b, op1c, op1d, op1e, op1f,
  op00, op01, op02, op03, op04, op05, op06, op07,  /* 20 */
  op08, op09, op0a, op0b, op0c, op0d, op0e, op0f,
  NULL, op11, op32, op13, NULL, op15, NULL, op17,  /* 30 */
  op18, op19, op1a, op1b, op1c, op1d, op1e, op1f,
  op40, op40, op40, op40, op40, op40, op40, op40,  /* 40 */
  op40, op40, op40, op40, op40, op40, op40, op40,
  op40, op40, op40, op40, op40, op40, op40, op40,  /* 50 */
  op40, op40, op40, op40, op40, op40, op40, op40,
  op40, op40, op40, op40, op40, op40, op40, op40,  /* 60 */
  op40, op40, op40, op40, op40, op40, op40, op40,
  op40, op40, op40, op40, op40, op40, op40, op40,  /* 70 */
  op40, op40, op40, op40, op40, op40, op40, op40,
  op80, op80, op80, op80, op80, op80, op80, op80,  /* 80 */
  op80, op80, op80, op80, op80, op80, op80, op80,
  op80, op80, op80, op80, op80, op80, op80, op80,  /* 90 */
  op80, op80, op80, op80, op80, op80, op80, op80,
  opa0, opa0, opa0, opa0, opa0, opa0, opa0, opa0,  /* a0 */
  opa0, opa0, opa0, opa0, opa0, opa0, opa0, opa0,
  opb0, opb0, opb0, opb0, opb0, opb0, opb0, opb0,  /* b0 */
  opb0, opb0, opb0, opb0, opb0, opb0, opb0, opb0,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  /* c0 */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  /* d0 */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,  /* e0 */
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  opf0, opf0, opf0, opf0, opf0, opf0, opf0, opf0,  /* f0 */
  opf0, opf0, opf0, opf0, opf0, opf0, opf0, opf0
};


void arm_set_opcodes (arm_t *c)
{
  unsigned i;

  for (i = 0; i < 256; i++) {
    if (arm_opcodes[i] != NULL) {
      c->opcodes[i] = arm_opcodes[i];
    }
    else {
      c->opcodes[i] = arm_op_undefined;
    }
  }
}
