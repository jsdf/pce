/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc.c                                                *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-21 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1999-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: ibmpc.c,v 1.15 2003/04/21 19:14:18 hampa Exp $ */


#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include <pce.h>


void pc_load_bios (ibmpc_t *pc, char *fname);
void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2);

unsigned char pc_ppi_get_port_a (ibmpc_t *pc);
unsigned char pc_ppi_get_port_c (ibmpc_t *pc);
void pc_ppi_set_port_b (ibmpc_t *pc, unsigned char val);

void pc_break (ibmpc_t *pc, unsigned char val);


void pc_log (ibmpc_t *pc, const char *str, ...)
{
  va_list     va;

  if (str != NULL) {
    va_start (va, str);
    vfprintf (stderr, str, va);
    va_end (va);
  }

  fflush (stderr);
}

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

  pc->ppi_port_a = 0x40 | 0x30 | 0x0c | 0x01;
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

void pc_setup_pic (ibmpc_t *pc)
{
  pc->pic = e8259_new();

  pc->pic_prt = mem_blk_new (0x20, 2, 0);
  pc->pic_prt->ext = pc->pic;
  pc->pic_prt->set_uint8 = (seta_uint8_f) &e8259_set_uint8;
  pc->pic_prt->get_uint8 = (geta_uint8_f) &e8259_get_uint8;
  mem_add_blk (pc->prt, pc->pic_prt);

  pc->pic->irq_ext = pc->cpu;
  pc->pic->irq = (set_uint8_f) &e86_irq;

  pc->cpu->inta_ext = pc->pic;
  pc->cpu->inta = (get_uint8_f) &e8259_inta;
}

void pc_setup_keyboard (ibmpc_t *pc)
{
  pc->key = key_new();
  pc->key->mem = pc->mem;

  pc->key->brk_ext = pc;
  pc->key->brk = (set_uint8_f) &pc_break;
}

void pc_setup_mda (ibmpc_t *pc)
{
  pc->mda = mda_new (stdout);
  mem_add_blk (pc->mem, pc->mda->mem);
  mem_add_blk (pc->prt, pc->mda->crtc);

  pc->ppi_port_a &= ~0x30;
  pc->ppi_port_a |= 0x30;
}

void pc_setup_cga (ibmpc_t *pc)
{
  pc->cga = cga_new (stdout);
  mem_add_blk (pc->mem, pc->cga->mem);
  mem_add_blk (pc->prt, pc->cga->crtc);

  pc->ppi_port_a &= ~0x30;
  pc->ppi_port_a |= 0x20;
}

void pc_setup_disks (ibmpc_t *pc)
{
  disk_t  *dsk;

  pc->dsk = dsks_new();

  dsk = dsk_new (0);
  if (dsk_set_mem (dsk, 80, 2, 18, "drive_a.img", 0)) {
    pc_log (pc, "loading drive a failed\n");
    dsk_del (dsk);
  }
  else {
    dsks_add_disk (pc->dsk, dsk);
  }

  dsk = dsk_new (0x80);
  if (dsk_set_hdimage (dsk, "drive_c.img", 0)) {
    pc_log (pc, "loading drive c failed\n");
    dsk_del (dsk);
  }
  else {
    dsks_add_disk (pc->dsk, dsk);
  }
}

ibmpc_t *pc_new (unsigned ramsize, unsigned dsp)
{
  unsigned i;
  ibmpc_t  *pc;

  ramsize = (ramsize + 31) & ~31;

  pc = (ibmpc_t *) malloc (sizeof (ibmpc_t));
  if (pc == NULL) {
    return (NULL);
  }

  pc->brk = 0;

  pc->clk_cnt = 0;

  for (i = 0; i < 4; i++) {
    pc->clk_div[i] = 0;
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
  pc->prt->def_val = 0xff;

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

  pc_setup_ppi (pc, ramsize);

  pc_setup_pic (pc);

  pc_setup_keyboard (pc);

  pc->mda = NULL;
  pc->cga = NULL;

  if (dsp == 0) {
    pc_setup_mda (pc);
  }
  else {
    pc_setup_cga (pc);
  }

  pc_setup_disks (pc);

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
  cga_del (pc->cga);

  e86_del (pc->cpu);

  e8259_del (pc->pic);
  mem_blk_del (pc->pic_prt);

  e8255_del (pc->ppi);
  mem_blk_del (pc->ppi_prt);

  mem_del (pc->mem);
  mem_del (pc->prt);

  mem_blk_del (pc->ram);
  mem_blk_del (pc->bios);

  free (pc);
}

void pc_clock (ibmpc_t *pc)
{
  unsigned long n;

  n = e86_get_delay (pc->cpu);

  e86_clock (pc->cpu, n);

  if (pc->clk_div[0] >= (4 * 65536)) {
    e8259_set_irq0 (pc->pic, 1);
    pc->clk_div[0] -= (4 * 65536);
  }

  if (pc->clk_div[1] >= 16) {
    e8259_clock (pc->pic);
    pc->clk_div[1] -= 16;
  }

  if (pc->clk_div[2] >= 64) {
    cga_clock (pc->cga);
    key_clock (pc->key);
    pc->clk_div[2] -= 64;
  }

  pc->clk_cnt += n;
  pc->clk_div[0] += n;
  pc->clk_div[1] += n;
  pc->clk_div[2] += n;
}

void pc_break (ibmpc_t *pc, unsigned char val)
{
  pc->brk = 1;
}

unsigned get_bcd_8 (unsigned n)
{
  return ((n % 10) + 16 * ((n / 10) % 10));
}

void pc_int_1a (ibmpc_t *pc)
{
  time_t    tm;
  struct tm *tval;

  switch (e86_get_ah (pc->cpu)) {
    case 0x00:
      e86_set_dx (pc->cpu, e86_get_mem16 (pc->cpu, 0x40, 0x6c));
      e86_set_cx (pc->cpu, e86_get_mem16 (pc->cpu, 0x40, 0x6e));
      e86_set_al (pc->cpu, e86_get_mem8 (pc->cpu, 0x40, 0x70));
      e86_set_mem8 (pc->cpu, 0x40, 0x70, 0);
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x01:
      e86_set_mem16 (pc->cpu, 0x40, 0x6c, e86_get_dx (pc->cpu));
      e86_set_mem16 (pc->cpu, 0x40, 0x6e, e86_get_cx (pc->cpu));
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x02:
      tm = time (NULL);
      tval = localtime (&tm);
      e86_set_ch (pc->cpu, get_bcd_8 (tval->tm_hour));
      e86_set_cl (pc->cpu, get_bcd_8 (tval->tm_min));
      e86_set_dh (pc->cpu, get_bcd_8 (tval->tm_sec));
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x03:
      break;

    case 0x04:
      tm = time (NULL);
      tval = localtime (&tm);
      e86_set_ch (pc->cpu, get_bcd_8 ((1900 + tval->tm_year) / 100));
      e86_set_cl (pc->cpu, get_bcd_8 (1900 + tval->tm_year));
      e86_set_dh (pc->cpu, get_bcd_8 (tval->tm_mon + 1));
      e86_set_dl (pc->cpu, get_bcd_8 (tval->tm_mday));
      e86_set_cf (pc->cpu, 0);
      break;

    case 0x05:
      break;

    default:
      e86_set_cf (pc->cpu, 1);
      break;
  }
}

void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2)
{
  ibmpc_t *pc;

  pc = (ibmpc_t *) ext;

  if ((op1 == 0xcd) && (op2 == 0x13)) {
    dsk_int13 (pc->dsk, pc->cpu);
  }
  else if ((op1 == 0xcd) && (op2 == 0x1a)) {
    pc_int_1a (pc);
  }
  else if ((op1 == 0x00) && (op2 == 0x00)) {
    pc->brk = 1;
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

  /* key code */
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
    fprintf (stderr, "loading bios failed (fopen)\n");
    return;
  }

  if (fread (pc->bios->data, 1, 64 * 1024, fp) != (64 * 1024)) {
    fprintf (stderr, "loading bios failed\n");
  }
}
