/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     internal.h                                                 *
 * Created:       2003-04-10 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-15 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: internal.h,v 1.2 2003/04/16 02:26:17 hampa Exp $ */


#ifndef PCE_E8086_INTERNAL_H
#define PCE_E8086_INTERNAL_H 1


#include "e8086.h"


#define FLG_STD (E86_FLG_C | E86_FLG_O | E86_FLG_Z | E86_FLG_S | E86_FLG_P)


#define e86_mk_uint16(lo, hi) \
  (((hi) << 8) | (lo))

#define e86_mk_sint16(lo) \
  (((lo) & 0x80) ? ((lo) | 0xff00) : ((lo) & 0xff))

#define e86_add_sint8(w, b) \
  ((((b) & 0x80) ? ((w) + ((b) | 0xff00)) : ((w) + ((b) & 0xff))) & 0xffff)

#define e86_get_prt8(cpu, ofs) \
  (cpu)->prt_get_uint8 ((cpu)->prt, ofs)

#define e86_get_prt16(cpu, ofs) \
  (cpu)->prt_get_uint16 ((cpu)->prt, ofs)

#define e86_set_prt8(cpu, ofs, val) \
  (cpu)->prt_set_uint8 ((cpu)->prt, ofs, val)

#define e86_set_prt16(cpu, ofs, val) \
  (cpu)->prt_set_uint16 ((cpu)->prt, ofs, val)

#define e86_get_flg(cpu, f) \
  (((cpu)->flg & (f)) != 0)

#define e86_set_flg(cpu, f, v) \
  if (v) (c)->flg |= (f); else (c)->flg &= ~(f);

#define e86_get_cf(cpu) \
  (((cpu)->flg & E86_FLG_C) != 0)

#define e86_set_cf(cpu, v) \
  if (v) (c)->flg |= E86_FLG_C; else (c)->flg &= ~E86_FLG_C;

#define e86_get_of(cpu) \
  (((cpu)->flg & E86_FLG_O) != 0)

#define e86_set_of(cpu, v) \
  if (v) (c)->flg |= E86_FLG_O; else (c)->flg &= ~E86_FLG_O;

#define e86_get_seg(cpu, seg) \
  ((cpu)->prefix & E86_PREFIX_SEG) ? (cpu)->seg_override : (cpu)->sreg[(seg)];

#define e86_add_uint4(s1, s2) \
  (((s1) & 0x0f) + ((s2) & 0x0f))

#define e86_sub_uint4(s1, s2) \
  (((s1) & 0x0f) - ((s2) & 0x0f))


typedef unsigned (*e86_opcode_f) (e8086_t *c);

typedef void (*e86_ea_f) (e8086_t *c);


extern e86_opcode_f e86_opcodes[256];

extern e86_ea_f e86_ea[32];


void e86_get_ea_ptr (e8086_t *c, unsigned char *ea);

unsigned char e86_get_ea8 (e8086_t *c);
unsigned short e86_get_ea16 (e8086_t *c);
void e86_set_ea8 (e8086_t *c, unsigned char val);
void e86_set_ea16 (e8086_t *c, unsigned short val);

void e86_pq_init (e8086_t *c);
void e86_pq_fill (e8086_t *c);
void e86_pq_adjust (e8086_t *c, unsigned cnt);

void e86_set_flags8 (e8086_t *c, unsigned short mask, unsigned short dst, unsigned short src);
void e86_set_flags16 (e8086_t *c, unsigned short mask, unsigned long dst, unsigned long src);
void e86_set_flags_af (e8086_t *c, unsigned d);


#endif
