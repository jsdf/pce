/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     flags.c                                                    *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: flags.c,v 1.1 2003/04/18 20:15:24 hampa Exp $ */


#include "e8086.h"
#include "internal.h"


static
unsigned char parity[32] = {
  0x96, 0x69, 0x69, 0x96, 0x69, 0x96, 0x96, 0x69,
  0x69, 0x96, 0x96, 0x69, 0x96, 0x69, 0x69, 0x96,
  0x69, 0x96, 0x96, 0x69, 0x96, 0x69, 0x69, 0x96,
  0x96, 0x69, 0x69, 0x96, 0x69, 0x96, 0x96, 0x69
};


/*************************************************************************
 * Flags functions
 *************************************************************************/

void e86_set_flg_szp_8 (e8086_t *c, unsigned char val)
{
  unsigned short set;

  set = 0;

  if ((val & 0xff) == 0) {
    set |= E86_FLG_Z;
  }

  if (val & 0x80) {
    set |= E86_FLG_S;
  }

  val &= 0xff;

  if (parity[val >> 3] & (0x80 >> (val & 7))) {
    set |= E86_FLG_P;
  }

  c->flg &= ~(E86_FLG_S | E86_FLG_Z | E86_FLG_P);
  c->flg |= set;
}

void e86_set_flg_szp_16 (e8086_t *c, unsigned short val)
{
  unsigned short set;

  set = 0;

  if ((val & 0xffff) == 0) {
    set |= E86_FLG_Z;
  }

  if (val & 0x8000) {
    set |= E86_FLG_S;
  }

  val &= 0xff;

  if (parity[val >> 3] & (0x80 >> (val & 7))) {
    set |= E86_FLG_P;
  }

  c->flg &= ~(E86_FLG_S | E86_FLG_Z | E86_FLG_P);
  c->flg |= set;
}

void e86_set_flg_log_8 (e8086_t *c, unsigned char val)
{
  e86_set_flg_szp_8 (c, val);

  c->flg &= ~(E86_FLG_C | E86_FLG_O);
}

void e86_set_flg_log_16 (e8086_t *c, unsigned short val)
{
  e86_set_flg_szp_16 (c, val);

  c->flg &= ~(E86_FLG_C | E86_FLG_O);
}

void e86_set_flg_adc_8 (e8086_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
  unsigned short set;
  unsigned short udst, sdst, adst;

  e86_set_flg_szp_8 (c, s1 + s2 + s3);

  set = 0;

  udst = s1 + s2 + s3;
  sdst = (e86_mk_sint16 (s1) + e86_mk_sint16 (s2) + s3) & 0xffff;
  adst = (s1 & 0x0f) + (s2 & 0x0f) + (s3 & 0x0f);

  if (udst & 0xff00) {
    set |= E86_FLG_C;
  }

  if (((sdst >> 1) ^ sdst) & 0x80) {
    set |= E86_FLG_O;
  }

  if (adst & 0xfff0) {
    set |= E86_FLG_A;
  }

  c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
  c->flg |= set;
}

void e86_set_flg_adc_16 (e8086_t *c, unsigned short s1, unsigned short s2, unsigned short s3)
{
  unsigned short set;
  unsigned long  t1, t2, t3;
  unsigned long  udst, sdst, adst;

  e86_set_flg_szp_16 (c, s1 + s2 + s3);

  set = 0;

  t1 = (s1 & 0x8000) ? (0xffff0000UL | s1) : s1;
  t2 = (s2 & 0x8000) ? (0xffff0000UL | s2) : s2;
  t3 = s3;

  udst = (unsigned long) s1 + (unsigned long) s2 + (unsigned long) s3;
  sdst = (t1 + t2 + t3) & 0xffffffffUL;
  adst = (s1 & 0x0f) + (s2 & 0x0f) + (s3 & 0x0f);

  if (udst & 0xffff0000) {
    set |= E86_FLG_C;
  }

  if (((sdst >> 1) ^ sdst) & 0x8000) {
    set |= E86_FLG_O;
  }

  if (adst & 0xfffffff0) {
    set |= E86_FLG_A;
  }

  c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
  c->flg |= set;
}

void e86_set_flg_sbb_8 (e8086_t *c, unsigned char s1, unsigned char s2, unsigned char s3)
{
  unsigned short set;
  unsigned short udst, sdst, adst;

  e86_set_flg_szp_8 (c, s1 - s2 - s3);

  set = 0;

  udst = s1 - s2 - s3;
  sdst = (e86_mk_sint16 (s1) - e86_mk_sint16 (s2) - s3) & 0xffff;
  adst = (s1 & 0x0f) - (s2 & 0x0f) - (s3 & 0x0f);

  if (udst & 0xff00) {
    set |= E86_FLG_C;
  }

  if (((sdst >> 1) ^ sdst) & 0x80) {
    set |= E86_FLG_O;
  }

  if (adst & 0xfff0) {
    set |= E86_FLG_A;
  }

  c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
  c->flg |= set;
}

void e86_set_flg_sbb_16 (e8086_t *c, unsigned short s1, unsigned short s2, unsigned short s3)
{
  unsigned short set;
  unsigned long  t1, t2, t3;
  unsigned long  udst, sdst, adst;

  e86_set_flg_szp_16 (c, s1 - s2 - s3);

  set = 0;

  t1 = (s1 & 0x8000) ? (0xffff0000UL | s1) : s1;
  t2 = (s2 & 0x8000) ? (0xffff0000UL | s2) : s2;
  t3 = s3;

  udst = (unsigned long) s1 - (unsigned long) s2 - (unsigned long) s3;
  sdst = (t1 - t2 - t3) & 0xffffffffUL;
  adst = (s1 & 0x0f) - (s2 & 0x0f) - (s3 & 0x0f);

  if (udst & 0xffff0000) {
    set |= E86_FLG_C;
  }

  if (((sdst >> 1) ^ sdst) & 0x8000) {
    set |= E86_FLG_O;
  }

  if (adst & 0xfffffff0) {
    set |= E86_FLG_A;
  }

  c->flg &= ~(E86_FLG_C | E86_FLG_O | E86_FLG_A);
  c->flg |= set;
}
