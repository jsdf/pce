/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/arm/copr15.c                                       *
 * Created:       2004-11-09 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-16 by Hampa Hug <hampa@hampa.ch>                   *
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


int p15_exec (arm_t *c, arm_copr_t *p);


void p15_init (arm_copr15_t *c)
{
  arm_copr_init (&c->copr);

  c->copr.ext = c;
  c->copr.exec = p15_exec;

  c->reg[0] = ARM_C15_ID;
  c->reg[1] = ARM_C15_CR_P | ARM_C15_CR_D | ARM_C15_CR_L | ARM_C15_CR_B;
}

arm_copr_t *p15_new (void)
{
  arm_copr15_t *c;

  c = malloc (sizeof (arm_copr15_t));
  if (c == NULL) {
    return (NULL);
  }

  p15_init (c);

  return (&c->copr);
}

void p15_free (arm_copr15_t *p)
{
}

void p15_del (arm_copr15_t *p)
{
  if (p != NULL) {
    p15_free (p);
  }

  free (p);
}

/* cache functions */
int p15_set_reg7 (arm_t *c, arm_copr15_t *p)
{
  unsigned rm, op2;

  rm = arm_ir_rm (c->ir);
  op2 = arm_get_bits (c->ir, 5, 3);

  if ((rm == 7) && (op2 == 0)) {
    /* invalidate all caches */
    return (0);
  }
  else if ((rm == 2) && (op2 == 5)) {
    /* ??? */
    return (0);
  }
  else if (rm == 5) {
    switch (op2) {
    case 0x00:
      /* invalidate entire instruction cache */
      return (0);

    case 0x01:
      /* invalidate instruction cache line */
      return (0);

    case 0x02:
      /* invalidate instruction cache line */
      return (0);

    case 0x04:
      /* flush prefetch buffer */
      return (0);

    case 0x06:
      /* flush entire branch target cache */
      return (0);

    case 0x07:
      /* flush branch target cache entry */
      return (0);
    }

    return (1);
  }
  else if (rm == 6) {
    switch (op2) {
    case 0x00:
      /* invalidate entire data cache */
      return (0);

    case 0x01:
      /* invalidate data cache line */
      return (0);

    case 0x02:
      /* invalidate data cache line */
      return (0);
    }
  }
  else if (rm == 10) {
    switch (op2) {
    case 0x01:
      /* clean data cache line */
      return (0);

    case 0x02:
      /* clean data cache line */
      return (0);

    case 0x04:
      /* drain write buffer */
      return (0);
    }

    return (1);
  }

  return (1);
}

/* TLB functions */
int p15_set_reg8 (arm_t *c, arm_copr15_t *p)
{
  unsigned rm, op2;

  rm = arm_ir_rm (c->ir);
  op2 = arm_get_bits (c->ir, 5, 3);

  if (rm == 5) {
    switch (op2) {
    case 0x00:
      /* invalidate entire instruction tlb */
      return (0);

    case 0x01:
      /* invalidate instruction tlb single entry */
      return (0);
    }
  }
  else if (rm == 6) {
    switch (op2) {
    case 0x00:
      /* invalidate entire data tlb */
      return (0);

    case 0x01:
      /* invalidate data tlb single entry */
      return (0);
    }
  }
  else if (rm == 7) {
    switch (op2) {
    case 0x00:
      /* invalidate entire unified tlb */
      return (0);

    case 0x01:
      /* invalidate unified tlb single entry */
      return (0);
    }
  }

  return (1);
}

int p15_op_mrc (arm_t *c, arm_copr_t *p)
{
  arm_copr15_t *p15;
  unsigned     op2;
  uint32_t     val;

  p15 = p->ext;

  op2 = arm_get_bits (c->ir, 5, 3);

  switch (arm_ir_rn (c->ir)) {
  case 0x00: /* ID register */
    val = p15->reg[0];
    break;

  case 0x01: /* control register */
    val = p15->reg[1];
    break;

  case 0x02: /* translation table base */
    val = p15->reg[2];
    break;

  case 0x03: /* domain access control */
    val = p15->reg[2];
    break;

  case 0x05: /* fault status */
    val = p15->reg[5];
    break;

  case 0x06: /* fault address */
    val = p15->reg[6];
    break;

  default:
    return (1);
  }

  if (arm_rd_is_pc (c->ir)) {
    arm_set_cpsr (c, (arm_get_cpsr (c) & ~ARM_PSR_CC) | (val & ARM_PSR_CC));
  }
  else {
    arm_set_rd (c, c->ir, val & 0xffffffffUL);
  }

  return (0);
}

int p15_op_mcr (arm_t *c, arm_copr_t *p)
{
  arm_copr15_t *p15;
  unsigned     op2;
  uint32_t     val;

  p15 = p->ext;

  op2 = arm_get_bits (c->ir, 5, 3);

  val = arm_get_rd (c, c->ir);

  /* conservative flushing of translation buffer */
  arm_tbuf_flush (c);

  switch (arm_ir_rn (c->ir)) {
  case 0x00: /* id register */
    return (1);

  case 0x01: /* control register */
    val &= ~ARM_C15_CR_C;
    val &= ~ARM_C15_CR_W;
    val &= ~ARM_C15_CR_B;
    val |= ARM_C15_CR_P;
    val |= ARM_C15_CR_D;
    val |= ARM_C15_CR_L;
    p15->reg[1] = val & 0xffffffffUL;

    c->exception_base = (val & ARM_C15_CR_V) ? 0xffff0000UL : 0x00000000UL;
    break;

  case 0x02: /* translation table base */
    p15->reg[2] = val & 0xffffc000UL;
    break;

  case 0x03: /* domain access control */
    p15->reg[3] = val & 0xffffffffUL;
    break;

  case 0x07:
    return (p15_set_reg7 (c, p15));

  case 0x08:
    return (p15_set_reg8 (c, p15));

  case 0x0f: /* implementation defined */
    return (0);

  default:
    return (1);
  }

  return (0);
}

int p15_exec (arm_t *c, arm_copr_t *p)
{
  int           r;
  unsigned long pc;
  char          *op;

  pc = arm_get_pc (c);

  if (arm_is_privileged (c) == 0) {
    return (1);
  }

  op = "?";

  switch (c->ir & 0x00f00010UL) {
  case 0x00000010UL: /* mcr */
    op = "W";
    r = p15_op_mcr (c, p);
    break;

  case 0x00100010UL: /* mrc */
    op = "R";
    r = p15_op_mrc (c, p);
    break;

  default:
    r = 1;
    break;
  }

  if (r == 0) {
    arm_set_clk (c, 4, 1);
  }

  if (r) {
    fprintf (stderr, "%08lX C15: %s Rd=%u Rn=%u Rm=%u op2=%u\n",
      pc, op,
      (unsigned) arm_ir_rd (c->ir),
      (unsigned) arm_ir_rn (c->ir),
      (unsigned) arm_ir_rm (c->ir),
      (unsigned) arm_get_bits (c->ir, 5, 3)
    ); fflush (stderr);
  }

  return (r);
}
