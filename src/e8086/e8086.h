/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     e8086.h                                                    *
 * Created:       1996-04-28 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e8086.h,v 1.1 2003/04/15 04:03:58 hampa Exp $ */


#ifndef PCE_E8086_H
#define PCE_E8086_H 1


#include <stdio.h>


/* CPU flags */
#define E86_FLG_C 0x0001
#define E86_FLG_P 0x0004
#define E86_FLG_A 0x0010
#define E86_FLG_Z 0x0040
#define E86_FLG_S 0x0080
#define E86_FLG_T 0x0100
#define E86_FLG_I 0x0200
#define E86_FLG_D 0x0400
#define E86_FLG_O 0x0800

/* 16 bit register values */
#define E86_REG_AX 0
#define E86_REG_CX 1
#define E86_REG_DX 2
#define E86_REG_BX 3
#define E86_REG_SP 4
#define E86_REG_BP 5
#define E86_REG_SI 6
#define E86_REG_DI 7

/* 8 bit register values */
#define E86_REG_AL 0
#define E86_REG_CL 1
#define E86_REG_DL 2
#define E86_REG_BL 3
#define E86_REG_AH 4
#define E86_REG_CH 5
#define E86_REG_DH 6
#define E86_REG_BH 7

/* Segment register values */
#define E86_REG_ES 0
#define E86_REG_CS 1
#define E86_REG_SS 2
#define E86_REG_DS 3

#define E86_PREFIX_NEW  0x0001
#define E86_PREFIX_SEG  0x0002
#define E86_PREFIX_REP  0x0004
#define E86_PREFIX_REPN 0x0008
#define E86_PREFIX_LOCK 0x0010

#define E86_PQ_SIZE 6


typedef struct {
  unsigned char (*mem_get_uint8) (void *mem, unsigned long addr);
  unsigned short (*mem_get_uint16) (void *mem, unsigned long addr);

  void (*mem_set_uint8) (void *mem, unsigned long addr, unsigned char val);
  void (*mem_set_uint16) (void *mem, unsigned long addr, unsigned short val);

  unsigned char (*prt_get_uint8) (void *mem, unsigned long addr);
  unsigned short (*prt_get_uint16) (void *mem, unsigned long addr);

  void (*prt_set_uint8) (void *mem, unsigned long addr, unsigned char val);
  void (*prt_set_uint16) (void *mem, unsigned long addr, unsigned short val);

  void (*hook) (void *ext, unsigned char op1, unsigned char op2);

  void *mem;
  void *prt;
  void *hook_ext;

  unsigned short dreg[8];
  unsigned short sreg[4];
  unsigned short ip;
  unsigned short flg;

  unsigned       pq_cnt;
  unsigned char  pq[E86_PQ_SIZE];

  unsigned       prefix;

  unsigned short seg_override;

  struct {
    int            is_mem;
    unsigned char  *data;
    unsigned short seg;
    unsigned short ofs;
    unsigned short cnt;
    unsigned long  delay;
  } ea;

  unsigned long  delay;

  unsigned long  clocks;
  unsigned long  instructions;
} e8086_t;



void e86_prt_state (e8086_t *c, FILE *fp);

void e86_execute (e8086_t *c);
void e86_clock (e8086_t *c);

void e86_reset (e8086_t *c);

e8086_t *e86_new (void);

void e86_del (e8086_t *c);


typedef struct {
  unsigned short ip;

  unsigned       dat_n;
  unsigned char  dat[16];

  char           op[64];

  unsigned       arg_n;
  char           arg1[64];
  char           arg2[64];
} e86_disasm_t;


void e86_disasm (e86_disasm_t *op, unsigned char *src, unsigned short ip);
void e86_disasm_mem (e8086_t *c, e86_disasm_t *op, unsigned short, unsigned short ip);
void e86_disasm_cur (e8086_t *c, e86_disasm_t *op);


#endif
