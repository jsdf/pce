/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     ibmpc.h                                                    *
 * Created:       2001-05-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-17 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ibmpc.h,v 1.3 2003/04/17 11:47:57 hampa Exp $ */


#ifndef PCE_IBMPC_H
#define PCE_IBMPC_H 1


#include <pce.h>


typedef struct {
  e8086_t       *cpu;
  mda_t         *mda;
  keyboard_t    *key;
  disks_t       *dsk;

  memory_t      *mem;
  mem_blk_t     *bios;
  mem_blk_t     *ram;

  memory_t      *prt;

  e8255_t       *ppi;
  mem_blk_t     *ppi_prt;
  unsigned char ppi_port_a;
  unsigned char ppi_port_b;
  unsigned char ppi_port_c[2];

  unsigned long clk_cnt;
} ibmpc_t;


ibmpc_t *pc_new (unsigned ramsize);

void pc_del (ibmpc_t *pc);

void pc_clock (ibmpc_t *pc);

void pc_prt_state (ibmpc_t *pc, FILE *fp);


#endif
