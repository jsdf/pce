/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     ea.c                                                       *
 * Created:       1996-04-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-15 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: ea.c,v 1.2 2003/04/16 02:26:17 hampa Exp $ */


#include <pce.h>

#include "e8086.h"
#include "internal.h"


/*************************************************************************
 * EA functions
 *************************************************************************/

/* EA 00: [BX + SI] */
static
void ea_get00 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX] + c->dreg[E86_REG_SI];
  c->ea.cnt = 1;
  c->ea.delay = 7;
}

/* EA 01: [BX + DI] */
static
void ea_get01 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX] + c->dreg[E86_REG_DI];
  c->ea.cnt = 1;
  c->ea.delay = 8;
}

/* EA 02: [BP + SI] */
static
void ea_get02 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP] + c->dreg[E86_REG_SI];
  c->ea.cnt = 1;
  c->ea.delay = 8;
}

/* EA 03: [BP + DI] */
static
void ea_get03 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP] + c->dreg[E86_REG_DI];
  c->ea.cnt = 1;
  c->ea.delay = 7;
}

/* EA 04: [SI] */
static
void ea_get04 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_SI];
  c->ea.cnt = 1;
  c->ea.delay = 5;
}

/* EA 05: [DI] */
static
void ea_get05 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_DI];
  c->ea.cnt = 1;
  c->ea.delay = 5;
}

/* EA 06: [XXXX] */
static
void ea_get06 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 6;
}

/* EA 07: [BX] */
static
void ea_get07 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX];
  c->ea.cnt = 1;
  c->ea.delay = 5;
}

/* EA 08: [BX + SI + XX] */
static
void ea_get08 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX] + c->dreg[E86_REG_SI] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 11;
}

/* EA 09: [BX + DI + XX] */
static
void ea_get09 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX] + c->dreg[E86_REG_DI] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 12;
}

/* EA 0A: [BP + SI + XX] */
static
void ea_get0a (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP] + c->dreg[E86_REG_SI] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 12;
}

/* EA 0B: [BP + DI + XX] */
static
void ea_get0b (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP] + c->dreg[E86_REG_DI] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 11;
}

/* EA 0C: [SI + XX] */
static
void ea_get0c (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_SI] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 5;
}

/* EA 0D: [DI + XX] */
static
void ea_get0d (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_DI] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 5;
}

/* EA 0E: [BP + XX] */
static
void ea_get0e (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 5;
}

/* EA 0F: [BX + XX] */
static
void ea_get0f (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX] + e86_mk_sint16 (c->ea.data[1]);
  c->ea.cnt = 2;
  c->ea.delay = 5;
}

/* EA 10: [BX + SI + XXXX] */
static
void ea_get10 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX] + c->dreg[E86_REG_SI];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 11;
}

/* EA 11: [BX + DI + XXXX] */
static
void ea_get11 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX] + c->dreg[E86_REG_DI];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 12;
}

/* EA 12: [BP + SI + XXXX] */
static
void ea_get12 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP] + c->dreg[E86_REG_SI];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 12;
}

/* EA 13: [BP + DI + XXXX] */
static
void ea_get13 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP] + c->dreg[E86_REG_DI];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 11;
}

/* EA 14: [SI + XXXX] */
static
void ea_get14 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_SI];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 5;
}

/* EA 15: [DI + XXXX] */
static
void ea_get15 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_DI];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 5;
}

/* EA 16: [BP + XXXX] */
static
void ea_get16 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_SS];
  c->ea.ofs = c->dreg[E86_REG_BP];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 5;
}

/* EA 17: [BX + XXXX] */
static
void ea_get17 (e8086_t *c)
{
  c->ea.is_mem = 1;
  c->ea.seg = c->sreg[E86_REG_DS];
  c->ea.ofs = c->dreg[E86_REG_BX];
  c->ea.ofs += e86_mk_uint16 (c->ea.data[1], c->ea.data[2]);
  c->ea.cnt = 3;
  c->ea.delay = 5;
}

/* EA 18-1F: REG */
static
void ea_get18 (e8086_t *c)
{
  c->ea.is_mem = 0;
  c->ea.seg = 0;
  c->ea.ofs = c->ea.data[0] & 7;
  c->ea.cnt = 1;
  c->ea.delay = 0;
}

e86_ea_f e86_ea[32] = {
  &ea_get00, &ea_get01, &ea_get02, &ea_get03,
  &ea_get04, &ea_get05, &ea_get06, &ea_get07,
  &ea_get08, &ea_get09, &ea_get0a, &ea_get0b,
  &ea_get0c, &ea_get0d, &ea_get0e, &ea_get0f,
  &ea_get10, &ea_get11, &ea_get12, &ea_get13,
  &ea_get14, &ea_get15, &ea_get16, &ea_get17,
  &ea_get18, &ea_get18, &ea_get18, &ea_get18,
  &ea_get18, &ea_get18, &ea_get18, &ea_get18
};

void e86_get_ea_ptr (e8086_t *c, unsigned char *ea)
{
  unsigned fea;

  c->ea.data = ea;

  fea = (ea[0] & 7) | ((ea[0] & 0xc0) >> 3);

  e86_ea[fea] (c);

  c->ea.ofs &= 0xffff;

  if (c->prefix & E86_PREFIX_SEG) {
    c->ea.seg = c->seg_override;
  }
}

unsigned char e86_get_ea8 (e8086_t *c)
{
  if (c->ea.is_mem) {
    return (e86_get_mem8 (c, c->ea.seg, c->ea.ofs));
  }

  return (e86_get_reg8 (c, c->ea.ofs));
}

unsigned short e86_get_ea16 (e8086_t *c)
{
  if (c->ea.is_mem) {
    return (e86_get_mem16 (c, c->ea.seg, c->ea.ofs));
  }

  return (e86_get_reg16 (c, c->ea.ofs));
}

void e86_set_ea8 (e8086_t *c, unsigned char val)
{
  if (c->ea.is_mem) {
    e86_set_mem8 (c, c->ea.seg, c->ea.ofs, val);
  }
  else {
    e86_set_reg8 (c, c->ea.ofs, val);
  }
}

void e86_set_ea16 (e8086_t *c, unsigned short val)
{
  if (c->ea.is_mem) {
    e86_set_mem16 (c, c->ea.seg, c->ea.ofs, val);
  }
  else {
    e86_set_reg16 (c, c->ea.ofs, val);
  }
}
