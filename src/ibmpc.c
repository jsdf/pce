/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     ibmpc.c                                                    *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ibmpc.c,v 1.1 2003/04/15 04:03:56 hampa Exp $ */


#include <stdio.h>

#include <pce.h>


void pc_load_bios (ibmpc_t *pc, char *fname);
void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2);


ibmpc_t *pc_new (void)
{
  ibmpc_t *pc;

  pc = (ibmpc_t *) malloc (sizeof (ibmpc_t));
  if (pc == NULL) {
    return (NULL);
  }

  pc->cpu = e86_new ();
  pc->cpu->hook = &pc_e86_hook;
  pc->cpu->hook_ext = pc;

  pc->mem = mem_new();
  pc->prt = mem_new();

  pc->cpu->mem = pc->mem;
  pc->cpu->prt = pc->prt;

  pc->ram = mem_blk_new (0, 512 * 1024, 1);
  mem_add_blk (pc->mem, pc->ram);

  pc->bios = mem_blk_new (0xf0000, 64 * 1024, 1);
  mem_blk_set_ro (pc->bios, 1);
  mem_add_blk (pc->mem, pc->bios);
  pc_load_bios (pc, "bios.rom");

  pc->cpu->mem_get_uint8 = &mem_get_uint8;
  pc->cpu->mem_get_uint16 = &mem_get_uint16_le;
  pc->cpu->mem_set_uint8 = &mem_set_uint8;
  pc->cpu->mem_set_uint16 = &mem_set_uint16_le;

  pc->cpu->prt_get_uint8 = &mem_get_uint8;
  pc->cpu->prt_get_uint16 = &mem_get_uint16_le;
  pc->cpu->prt_set_uint8 = &mem_set_uint8;
  pc->cpu->prt_set_uint16 = &mem_set_uint16_le;

  pc->mda = mda_new();
  pc->mda->fp = stdout;
  mem_add_blk (pc->mem, pc->mda->mem);
  mem_add_blk (pc->prt, pc->mda->crtc);

  pc->key = key_new();
  pc->key->mem = pc->mem;

  pc->flp = flp_new (80, 2, 18);
  flp_load_img (pc->flp, "floppy.img");

  return (pc);
}

void pc_del (ibmpc_t *pc)
{
  if (pc == NULL) {
    return;
  }

  flp_del (pc->flp);
  key_del (pc->key);

  mda_del (pc->mda);
  e86_del (pc->cpu);

  mem_del (pc->mem);
  mem_del (pc->mem);

  mem_blk_del (pc->ram);
  mem_blk_del (pc->bios);

  free (pc);
}

void pc_clock (ibmpc_t *pc)
{
  e86_clock (pc->cpu);
  mda_clock (pc->mda);
  key_clock (pc->key);

  pc->clk_cnt += 1;
}

void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2)
{
  ibmpc_t *pc;

  pc = (ibmpc_t *) ext;

  if ((op1 == 0xcd) && (op2 == 0x13)) {
    flp_int_13 (pc->flp, pc->cpu);
  }
}

void pc_load_bios (ibmpc_t *pc, char *fname)
{
  FILE *fp;

  fp = fopen (fname, "rb");
  if (fp == NULL) {
    return;
  }

  fread (pc->bios->data, 1, 64 * 1024, fp);
}

void pc_prt_state (ibmpc_t *pc, FILE *fp)
{
  e86_prt_state (pc->cpu, fp);
}
