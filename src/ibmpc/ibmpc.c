/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/ibmpc.c                                          *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-25 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ibmpc.c,v 1.5 2003/04/25 02:30:18 hampa Exp $ */


#include <stdio.h>
#include <time.h>

#include "pce.h"


void pc_load_bios (ibmpc_t *pc, char *fname);
void pc_e86_hook (void *ext, unsigned char op1, unsigned char op2);

unsigned char pc_ppi_get_port_a (ibmpc_t *pc);
unsigned char pc_ppi_get_port_c (ibmpc_t *pc);
void pc_ppi_set_port_b (ibmpc_t *pc, unsigned char val);

void pc_break (ibmpc_t *pc, unsigned char val);
void pc_set_keycode (ibmpc_t *pc, unsigned char val);


void pc_setup_ram (ibmpc_t *pc, ini_sct_t *ini)
{
  unsigned ram;

  ram = ini_get_def_long (ini, "ram", 640);

  pce_log (0, "RAM: %uKB\n", ram);

  pc->ram = mem_blk_new (0, 1024 * ram, 1);
  mem_blk_init (pc->ram, 0x00);
  mem_add_blk (pc->mem, pc->ram, 1);
}

void pc_setup_rom (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t     *sct;
  mem_blk_t     *rom;
  FILE          *fp;
  char          *fname;
  unsigned long base, size;

  sct = ini_sct_find_sct (ini, "rom");

  while (sct != NULL) {
    fname = ini_get_def_string (sct, "file", "default.rom");
    base = ini_get_def_long (sct, "base", 0);
    size = ini_get_def_long (sct, "size", 64 * 1024);

    pce_log (0, "ROM: %05X %04X %s\n", base, size, fname);

    fp = fopen (fname, "rb");
    if (fp == NULL) {
      pce_log (0, "loading rom failed (%s)\n", fname);
    }
    else {
      rom = mem_blk_new (base, size, 1);
      mem_blk_init (rom, 0x00);
      mem_blk_set_ro (rom, 1);
      mem_add_blk (pc->mem, rom, 1);

      if (fread (rom->data, 1, size, fp) != size) {
        pce_log (0, "loading rom data failed (%s)\n", fname);
      }

      fclose (fp);
    }

    sct = ini_sct_find_next (sct, "rom");
  }
}

void pc_setup_cpu (ibmpc_t *pc, ini_sct_t *ini)
{
  pc->cpu = e86_new ();

  e86_set_mem (pc->cpu, pc->mem,
    (e86_get_uint8_f) &mem_get_uint8,
    (e86_set_uint8_f) &mem_set_uint8,
    (e86_get_uint16_f) &mem_get_uint16_le,
    (e86_set_uint16_f) &mem_set_uint16_le
  );

  e86_set_prt (pc->cpu, pc->prt,
    (e86_get_uint8_f) &mem_get_uint8,
    (e86_set_uint8_f) &mem_set_uint8,
    (e86_get_uint16_f) &mem_get_uint16_le,
    (e86_set_uint16_f) &mem_set_uint16_le
  );

  e86_set_ram (pc->cpu, pc->ram->data, pc->ram->size);

  pc->cpu->op_ext = pc;
  pc->cpu->op_hook = &pc_e86_hook;
}

void pc_setup_ppi (ibmpc_t *pc, ini_sct_t *ini)
{
  e8255_t   *ppi;
  mem_blk_t *blk;
  unsigned  ram;

  ram = ini_get_def_long (ini, "ram", 640);
  ram = ram / 32;

  ppi = e8255_new();
  ppi->port[0].read_ext = pc;
  ppi->port[0].read = (get_uint8_f) &pc_ppi_get_port_a;
  ppi->port[1].write_ext = pc;
  ppi->port[1].write = (set_uint8_f) &pc_ppi_set_port_b;
  ppi->port[2].read_ext = pc;
  ppi->port[2].read = (get_uint8_f) &pc_ppi_get_port_c;
  pc->ppi = ppi;

  pc->ppi_port_a[0] = 0x40 | 0x30 | 0x0c | 0x01;
  pc->ppi_port_a[1] = 0;
  pc->ppi_port_b = 0;
  pc->ppi_port_c[0] = (ram & 0x0f);
  pc->ppi_port_c[1] = (ram >> 4) & 0x01;

  blk = mem_blk_new (0x60, 4, 0);
  blk->ext = ppi;
  blk->set_uint8 = (seta_uint8_f) &e8255_set_port8;
  blk->get_uint8 = (geta_uint8_f) &e8255_get_port8;
  mem_add_blk (pc->prt, blk, 1);
}

void pc_setup_pic (ibmpc_t *pc)
{
  mem_blk_t *blk;

  pc->pic = e8259_new();

  blk = mem_blk_new (0x20, 2, 0);
  blk->ext = pc->pic;
  blk->set_uint8 = (seta_uint8_f) &e8259_set_uint8;
  blk->get_uint8 = (geta_uint8_f) &e8259_get_uint8;
  mem_add_blk (pc->prt, blk, 1);

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

void pc_setup_terminal (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t *sct;

  pc->trm = NULL;

#ifdef PCE_X11_USE
  sct = ini_sct_find_sct (ini, "term_x11");
  if (sct != NULL) {
    pce_log (MSG_INF, "setting up term_x11\n");

    pc->trm = xt_new();
    if (pc->trm == NULL) {
      pce_log (MSG_ERR, "setting up term_x11 failed\n");
    }
  }
#endif

  sct = ini_sct_find_sct (ini, "term_vt100");
  if (pc->trm == NULL) {
    pce_log (MSG_INF, "setting up term_vt100\n");

    pc->trm = vt100_new (0, 1);
    if (pc->trm == NULL) {
      pce_log (MSG_ERR, "setting up term_vt100 failed\n");
    }
  }

  if (pc->trm == NULL) {
    pce_log (MSG_ERR, "no terminal found\n");
    return;
  }

  pc->trm->key_ext = pc;
  pc->trm->set_key = (set_uint8_f) &pc_set_keycode;
  pc->trm->set_brk = (set_uint8_f) &pc_break;
}

void pc_setup_mda (ibmpc_t *pc)
{
  pc->mda = mda_new (pc->trm);
  mem_add_blk (pc->mem, pc->mda->mem, 0);
  mem_add_blk (pc->prt, pc->mda->crtc, 0);

  pc->ppi_port_a[0] &= ~0x30;
  pc->ppi_port_a[0] |= 0x30;
}

void pc_setup_cga (ibmpc_t *pc)
{
  pc->cga = cga_new (pc->trm);
  mem_add_blk (pc->mem, pc->cga->mem, 0);
  mem_add_blk (pc->prt, pc->cga->crtc, 0);

  pc->ppi_port_a[0] &= ~0x30;
  pc->ppi_port_a[0] |= 0x20;
}

void pc_setup_video (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t * sct;

  pc->mda = NULL;
  pc->cga = NULL;

  sct = ini_sct_find_sct (ini, "cga");
  if (sct != NULL) {
    pc_setup_cga (pc);
    return;
  }

  sct = ini_sct_find_sct (ini, "mda");
  if (sct != NULL) {
    pc_setup_mda (pc);
    return;
  }
}

void pc_setup_disks (ibmpc_t *pc, ini_sct_t *ini)
{
  int       r;
  ini_sct_t *sct;
  disk_t    *dsk;
  unsigned  drive;
  unsigned  c, h, s;
  int       ro;
  char      *fname, *type;

  pc->dsk = dsks_new();

  sct = ini_sct_find_sct (ini, "disk");

  while (sct != NULL) {
    drive = ini_get_def_long (sct, "drive", 0);
    type = ini_get_def_string (sct, "type", "image");
    fname = ini_get_def_string (sct, "file", NULL);

    c = ini_get_def_long (sct, "c", 80);
    h = ini_get_def_long (sct, "h", 2);
    s = ini_get_def_long (sct, "s", 18);

    ro = ini_get_def_long (sct, "readonly", 0);

    dsk = dsk_new (drive);

    pce_log (0, "disk: %s:%s %u/%u/%u ro=%d\n",
      (fname != NULL) ? fname : "", type, c, h, s, ro
    );

    if (strcmp (type, "ram") == 0) {
      r = dsk_set_mem (dsk, c, h, s, fname, ro);
    }
    else if (strcmp (type, "image") == 0) {
      r = dsk_set_image (dsk, c, h, s, fname, ro);
    }
    else if (strcmp (type, "dosemu") == 0) {
      r = dsk_set_hdimage (dsk, fname, ro);
    }
    else {
      r = 1;
    }

    if (r) {
      pce_log (0, "loading drive %02X failed\n", drive);
      dsk_del (dsk);
      dsk = NULL;
    }
    else {
      dsks_add_disk (pc->dsk, dsk);
    }

    sct = ini_sct_find_next (sct, "disk");
  }
}

ibmpc_t *pc_new (ini_sct_t *ini)
{
  unsigned  i;
  ibmpc_t   *pc;

  pc = (ibmpc_t *) malloc (sizeof (ibmpc_t));
  if (pc == NULL) {
    return (NULL);
  }

  pc->key_i = 0;
  pc->key_j = 0;
  pc->key_clk = 0;

  pc->brk = 0;
  pc->clk_cnt = 0;

  for (i = 0; i < 4; i++) {
    pc->clk_div[i] = 0;
  }

  pc->mem = mem_new();

  pc->prt = mem_new();
  pc->prt->def_val = 0xff;

  pc_setup_ram (pc, ini);
  pc_setup_rom (pc, ini);
  pc_setup_cpu (pc, ini);
  pc_setup_ppi (pc, ini);
  pc_setup_pic (pc);
  pc_setup_terminal (pc, ini);
  pc_setup_keyboard (pc);
  pc_setup_video (pc, ini);
  pc_setup_disks (pc, ini);

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

  trm_del (pc->trm);

  e86_del (pc->cpu);

  e8259_del (pc->pic);

  e8255_del (pc->ppi);

  mem_del (pc->mem);
  mem_del (pc->prt);

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

  if (pc->key_clk > 0) {
    pc->key_clk = (n < pc->key_clk) ? (pc->key_clk - n) : 0;

    if ((pc->key_clk == 0) && (pc->key_i < pc->key_j)) {
      pc->ppi_port_a[1] = pc->key_buf[pc->key_i];
      e8259_set_irq1 (pc->pic, 1);
      pc->key_i += 1;

      if (pc->key_i < pc->key_j) {
        pc->key_clk = 10000;
      }
      else {
        pc->key_i = 0;
        pc->key_j = 0;
      }
    }
  }

  if (pc->clk_div[1] >= 16) {
    e8259_clock (pc->pic);
    pc->clk_div[1] -= 16;
  }

  if (pc->clk_div[2] >= 256) {
    cga_clock (pc->cga);
//    key_clock (pc->key);
    trm_check (pc->trm);
    pc->clk_div[2] -= 256;
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
    return (pc->ppi_port_a[0]);
  }
  else {
    return (pc->ppi_port_a[1]);
  }
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

void pc_set_keycode (ibmpc_t *pc, unsigned char val)
{
  pce_log (MSG_DEB, "keycode: %02X\n", val);

  if (pc->key_clk > 0) {
    if (pc->key_j < 256) {
      pc->key_buf[pc->key_j] = val;
      pc->key_j += 1;
    }
    return;
  }

  pc->ppi_port_a[1] = val;
  e8259_set_irq1 (pc->pic, 1);
  pc->key_clk = 10000;
}
