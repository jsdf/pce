/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/ibmpc.h                                     *
 * Created:       2001-05-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-12-23 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ibmpc.h,v 1.2 2003/12/23 03:48:30 hampa Exp $ */


#ifndef PCE_IBMPC_H
#define PCE_IBMPC_H 1


#define PCE_CPU_8086  0
#define PCE_CPU_8088  1
#define PCE_CPU_V30   2
#define PCE_CPU_V20   3
#define PCE_CPU_80186 4
#define PCE_CPU_80188 5
#define PCE_CPU_80286 6


typedef struct ibmpc_t {
  e8086_t       *cpu;
  video_t       *video;
  disks_t       *dsk;
  mouse_t       *mse;

  memory_t      *mem;
  mem_blk_t     *ram;

  memory_t      *prt;

  nvram_t       *nvr;

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

  ems_t         *ems;
  xms_t         *xms;

  terminal_t    *trm;

  serial_t      *serport[4];
  parport_t     *parport[4];

  unsigned           cpu_model;

  unsigned long long clk_cnt;
  unsigned long      clk_div[4];

  unsigned           brk;
} ibmpc_t;


ibmpc_t *pc_new (ini_sct_t *ini);

void pc_del (ibmpc_t *pc);

void pc_clock (ibmpc_t *pc);

int pc_set_cpu_model (ibmpc_t *pc, unsigned model);

void pc_break (ibmpc_t *pc, unsigned char val);


#endif
