/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/ibmpc.h                                          *
 * Created:       2001-05-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-19 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: ibmpc.h,v 1.8 2003/08/19 17:07:14 hampa Exp $ */


#ifndef PCE_IBMPC_H
#define PCE_IBMPC_H 1


typedef struct {
  e8086_t       *cpu;
  mda_t         *mda;
  hgc_t         *hgc;
  cga_t         *cga;
  disks_t       *dsk;

  memory_t      *mem;
  mem_blk_t     *ram;

  memory_t      *prt;

  e8253_t       *pit;
  mem_blk_t     *pit_prt;

  e8255_t       *ppi;
  mem_blk_t     *ppi_prt;
  unsigned char ppi_port_a[2];
  unsigned char ppi_port_b;
  unsigned char ppi_port_c[2];

  unsigned      key_i;
  unsigned      key_j;
  unsigned long key_clk;
  unsigned char key_buf[256];

  e8259_t       *pic;
  mem_blk_t     *pic_prt;

  terminal_t    *trm;

  parport_t     *parport[4];

  unsigned long long clk_cnt;
  unsigned long      clk_div[4];

  unsigned           brk;
} ibmpc_t;


ibmpc_t *pc_new (ini_sct_t *ini);

void pc_del (ibmpc_t *pc);

void pc_clock (ibmpc_t *pc);


#endif
