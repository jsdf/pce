/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     ibmpc.c                                                    *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ibmpc.c,v 1.5 2003/04/17 11:47:57 hampa Exp $ */


#include <stdio.h>

#include <pce.h>


void pc_load_bios (ibmpc_t *pc, char *fname);
void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2);

unsigned char pc_ppi_get_port_a (ibmpc_t *pc);
unsigned char pc_ppi_get_port_c (ibmpc_t *pc);
void pc_ppi_set_port_b (ibmpc_t *pc, unsigned char val);


void pc_setup_ppi (ibmpc_t *pc, unsigned ramsize)
{
  e8255_t   *ppi;
  mem_blk_t *blk;

  ramsize = ramsize / 32;

  ppi = e8255_new();
  ppi->port[0].read_ext = pc;
  ppi->port[0].read = (get_uint8_f) &pc_ppi_get_port_a;
  ppi->port[1].write_ext = pc;
  ppi->port[1].write = (set_uint8_f) &pc_ppi_set_port_b;
  ppi->port[2].read_ext = pc;
  ppi->port[2].read = (get_uint8_f) &pc_ppi_get_port_c;
  pc->ppi = ppi;

  pc->ppi_port_a = 0x7d;
  pc->ppi_port_b = 0;
  pc->ppi_port_c[0] = (ramsize & 0x0f);
  pc->ppi_port_c[1] = (ramsize >> 4) & 0x01;

  blk = mem_blk_new (0x60, 4, 0);
  blk->ext = ppi;
  blk->set_uint8 = (seta_uint8_f) &e8255_set_port8;
  blk->get_uint8 = (geta_uint8_f) &e8255_get_port8;
  mem_add_blk (pc->prt, blk);
  pc->ppi_prt = blk;
}

ibmpc_t *pc_new (unsigned ramsize)
{
  ibmpc_t *pc;
  disk_t  *dsk;

  ramsize = (ramsize + 31) & ~31;

  pc = (ibmpc_t *) malloc (sizeof (ibmpc_t));
  if (pc == NULL) {
    return (NULL);
  }

  pc->mem = mem_new();
  pc->ram = mem_blk_new (0, 1024 * ramsize, 1);
  mem_blk_init (pc->ram, 0x00);
  mem_add_blk (pc->mem, pc->ram);

  pc->bios = mem_blk_new (0xf0000, 64 * 1024, 1);
  mem_blk_init (pc->bios, 0x00);
  mem_blk_set_ro (pc->bios, 1);
  mem_add_blk (pc->mem, pc->bios);
  pc_load_bios (pc, "ibmpc.rom");

  pc->prt = mem_new();

  pc->cpu = e86_new ();
  pc->cpu->hook = &pc_e86_hook;
  pc->cpu->mem = pc->mem;
  pc->cpu->prt = pc->prt;
  pc->cpu->ext = pc;
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

  pc_setup_ppi (pc, ramsize);

  pc->key = key_new();
  pc->key->mem = pc->mem;

  pc->dsk = dsks_new();
  dsk = dsk_new (0, 80, 2, 18);
  dsk_alloc (dsk);
  dsk_load_img (dsk, "drive_a.img");
  dsks_add_disk (pc->dsk, dsk);

  dsk = dsk_new (1, 80, 2, 18);
  dsk_alloc (dsk);
  dsk_load_img (dsk, "drive_b.img");
  dsks_add_disk (pc->dsk, dsk);

  dsk = dsk_new (0x80, 255, 4, 17);
  dsk_set_file (dsk, "drive_c.img");
  dsks_add_disk (pc->dsk, dsk);

  return (pc);
}

void pc_del (ibmpc_t *pc)
{
  if (pc == NULL) {
    return;
  }

  dsks_del (pc->dsk);
  key_del (pc->key);

  mda_del (pc->mda);
  e86_del (pc->cpu);

  e8255_del (pc->ppi);
  mem_blk_del (pc->ppi_prt);

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
    dsk_int13 (pc->dsk, pc->cpu);
  }
  else {
    fprintf (stderr, "hook: %02X %02X\n", op1, op2);
  }
}

unsigned char pc_ppi_get_port_a (ibmpc_t *pc)
{
  if (pc->ppi_port_b & 0x80) {
    return (pc->ppi_port_a);
  }

  return (0);
}

unsigned char pc_ppi_get_port_c (ibmpc_t *pc)
{
  if (pc->ppi_port_b & 0x04) {
    return (pc->ppi_port_c[1]);
  }
  else {
    return (pc->ppi_port_c[0]);
  }
}

void pc_ppi_set_port_b (ibmpc_t *pc, unsigned char val)
{
  pc->ppi_port_b = val;
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
