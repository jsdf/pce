/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/ibmpc/ibmpc.c                                     *
 * Created:       1999-04-16 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-01-09 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1999-2004 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id$ */


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


static
int pc_load_ram (mem_blk_t *blk, const char *fname)
{
  FILE *fp;

  fp = fopen (fname, "rb");
  if (fp == NULL) {
    return (1);
  }

  fread (blk->data, 1, blk->size, fp);

  fclose (fp);

  return (0);
}

static
void pc_setup_ram (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t     *sct;
  mem_blk_t     *ram;
  char          *fname;
  unsigned long base, size;

  pc->ram = NULL;

  sct = ini_sct_find_sct (ini, "ram");

  while (sct != NULL) {
    ini_get_string (sct, "file", &fname, NULL);
    ini_get_ulng (sct, "base", &base, 0);

    if (ini_get_ulng (sct, "sizek", &size, 640)) {
      ini_get_ulng (sct, "size", &size, 640 * 1024);
    }
    else {
      size *= 1024;
    }

    pce_log (MSG_INF, "RAM:\tbase=0x%05x size=%lu file=%s\n",
      base, size, (fname == NULL) ? "<>" : fname
    );

    ram = mem_blk_new (base, size, 1);
    mem_blk_clear (ram, 0x00);
    mem_blk_set_ro (ram, 0);
    mem_add_blk (pc->mem, ram, 1);

    if (base == 0) {
      pc->ram = ram;
    }

    if (fname != NULL) {
      if (pc_load_ram (ram, fname)) {
        pce_log (MSG_ERR, "*** loading ram failed (%s)\n", fname);
      }
    }

    sct = ini_sct_find_next (sct, "ram");
  }
}

static
void pc_setup_rom (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t     *sct;
  mem_blk_t     *rom;
  char          *fname;
  unsigned long base, size;

  sct = ini_sct_find_sct (ini, "rom");

  while (sct != NULL) {
    ini_get_string (sct, "file", &fname, "default.rom");
    ini_get_ulng (sct, "base", &base, 0);
    ini_get_ulng (sct, "size", &size, 64UL * 1024);

    pce_log (MSG_INF, "ROM:\tbase=0x%05x size=%lu file=%s\n", base, size, fname);

    rom = mem_blk_new (base, size, 1);
    mem_blk_clear (rom, 0x00);
    mem_blk_set_ro (rom, 1);
    mem_add_blk (pc->mem, rom, 1);

    if (pc_load_ram (rom, fname)) {
      pce_log (MSG_ERR, "*** loading rom failed (%s)\n", fname);
    }

    sct = ini_sct_find_next (sct, "rom");
  }
}

void pc_setup_nvram (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t     *sct;
  char          *fname;
  unsigned long base, size;

  pc->nvr = NULL;

  sct = ini_sct_find_sct (ini, "nvram");
  if (sct == NULL) {
    return;
  }

  ini_get_string (sct, "file", &fname, NULL);
  ini_get_ulng (sct, "base", &base, 0);
  ini_get_ulng (sct, "size", &size, 65536UL);

  pce_log (MSG_INF, "NVRAM:\tbase=0x%08x size=%lu file=%s\n",
    base, size, (fname == NULL) ? "<>" : fname
  );

  pc->nvr = nvr_new (base, size);
  if (pc->nvr == NULL) {
    pce_log (MSG_ERR, "*** creating nvram failed\n");
    return;
  }

  nvr_set_endian (pc->nvr, 0);

  mem_add_blk (pc->mem, nvr_get_mem (pc->nvr), 0);

  if (fname != NULL) {
    if (nvr_set_fname (pc->nvr, fname)) {
      pce_log (MSG_ERR, "*** loading nvram failed (%s)\n", fname);
    }
  }
}

void pc_setup_cpu (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t *sct;
  char      *model;

  sct = ini_sct_find_sct (ini, "cpu");

  if (par_cpu == NULL) {
    ini_get_string (sct, "model", &model, "8086");
  }
  else {
    model = par_cpu;
  }

  pce_log (MSG_INF, "CPU:\tmodel=%s\n", model);

  pc->cpu = e86_new ();
  pc->cpu_model = PCE_CPU_8086;

  if (strcmp (model, "8086") == 0) {
    pc_set_cpu_model (pc, PCE_CPU_8086);
  }
  else if (strcmp (model, "8088") == 0) {
    pc_set_cpu_model (pc, PCE_CPU_8088);
  }
  else if ((strcmp (model, "v20") == 0) || (strcmp (model, "V20") == 0)) {
    pc_set_cpu_model (pc, PCE_CPU_V20);
  }
  else if ((strcmp (model, "v30") == 0) || (strcmp (model, "V30") == 0)) {
    pc_set_cpu_model (pc, PCE_CPU_V30);
  }
  else if ((strcmp (model, "80186") == 0) || (strcmp (model, "186") == 0)) {
    pc_set_cpu_model (pc, PCE_CPU_80186);
  }
  else if ((strcmp (model, "80188") == 0) || (strcmp (model, "188") == 0)) {
    pc_set_cpu_model (pc, PCE_CPU_80188);
  }
  else if ((strcmp (model, "80286") == 0) || (strcmp (model, "286") == 0)) {
    pc_set_cpu_model (pc, PCE_CPU_80286);
  }
  else {
    pce_log (MSG_ERR, "*** unknown cpu model (%s)\n", model);
  }

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

  if (pc->ram != NULL) {
    e86_set_ram (pc->cpu, pc->ram->data, pc->ram->size);
  }
  else {
    e86_set_ram (pc->cpu, NULL, 0);
  }

  pc->cpu->op_ext = pc;
  pc->cpu->op_hook = &pc_e86_hook;
}

void pc_setup_pic (ibmpc_t *pc)
{
  mem_blk_t *blk;

  pc->pic = e8259_new();

  blk = mem_blk_new (0x20, 2, 0);
  blk->ext = pc->pic;
  blk->set_uint8 = (seta_uint8_f) &e8259_set_uint8;
  blk->get_uint8 = (geta_uint8_f) &e8259_get_uint8;
  blk->set_uint16 = (seta_uint16_f) &e8259_set_uint16;
  blk->get_uint16 = (geta_uint16_f) &e8259_get_uint16;
  mem_add_blk (pc->prt, blk, 1);

  e8259_set_irq (pc->pic, pc->cpu, (e8259_irq_f) &e86_irq);

  pc->cpu->inta_ext = pc->pic;
  pc->cpu->inta = (get_uint8_f) &e8259_inta;
}

void pc_setup_pit (ibmpc_t *pc)
{
  mem_blk_t *blk;

  pc->pit = e8253_new();

  blk = mem_blk_new (0x40, 4, 0);
  blk->ext = pc->pit;
  blk->set_uint8 = (seta_uint8_f) &e8253_set_uint8;
  blk->get_uint8 = (geta_uint8_f) &e8253_get_uint8;
  blk->set_uint16 = (seta_uint16_f) &e8253_set_uint16;
  blk->get_uint16 = (geta_uint16_f) &e8253_get_uint16;
  mem_add_blk (pc->prt, blk, 1);

  e8253_set_gate (pc->pit, 0, 1);
  e8253_set_gate (pc->pit, 1, 1);
  e8253_set_gate (pc->pit, 1, 1);

  e8253_set_out (pc->pit, 0, pc->pic, (e8253_set_out_f) &e8259_set_irq0);
}

void pc_setup_ppi (ibmpc_t *pc, ini_sct_t *ini)
{
  e8255_t   *ppi;
  mem_blk_t *blk;
  unsigned  ram;

  ini_get_uint (ini, "ram", &ram, 640);
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

void pc_setup_terminal (ibmpc_t *pc, ini_sct_t *ini)
{
  char      *driver;
  ini_sct_t *sct;

  pc->trm = NULL;

  sct = ini_sct_find_sct (ini, "terminal");
  ini_get_string (sct, "driver", &driver, "vt100");

  if (par_terminal != NULL) {
    while ((sct != NULL) && (strcmp (par_terminal, driver) != 0)) {
      sct = ini_sct_find_next (sct, "terminal");
      ini_get_string (sct, "driver", &driver, "vt100");
    }

    if (sct == NULL) {
      driver = par_terminal;
    }
  }

  pce_log (MSG_INF, "term:\tdriver=%s\n", driver);

  if (strcmp (driver, "x11") == 0) {
#ifdef PCE_X11_USE
    pc->trm = xt_new (sct);
    if (pc->trm == NULL) {
      pce_log (MSG_ERR, "setting up x11 terminal failed\n");
    }
#else
    pce_log (MSG_ERR, "terminal driver 'x11' not supported\n");
#endif
  }
  else if (strcmp (driver, "sdl") == 0) {
#ifdef PCE_SDL_USE
    pc->trm = sdl_new (sct);
    if (pc->trm == NULL) {
      pce_log (MSG_ERR, "setting up sdl terminal failed\n");
    }
#else
    pce_log (MSG_ERR, "terminal driver 'sdl' not supported\n");
#endif
  }
  else if (strcmp (driver, "null") == 0) {
    pc->trm = null_new (sct);
    if (pc->trm == NULL) {
      pce_log (MSG_ERR, "setting up null terminal failed\n");
    }
  }
  else if (strcmp (driver, "vt100") == 0) {
    pc->trm = vt100_new (sct, 0, 1);
    if (pc->trm == NULL) {
      pce_log (MSG_ERR, "setting up vt100 terminal failed\n");
    }
  }
  else {
    pce_log (MSG_ERR, "unknown terminal driver: %s\n", driver);
  }

  if (pc->trm == NULL) {
    pce_log (MSG_ERR, "no terminal found\n");
    return;
  }

  pc->trm->key_ext = pc;
  pc->trm->set_key = (set_uint8_f) &pc_set_keycode;
  pc->trm->set_brk = (set_uint8_f) &pc_break;
}

void pc_setup_mda (ibmpc_t *pc, ini_sct_t *sct)
{
  pc->video = mda_new (pc->trm, sct);
  mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
  mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);

  pc->ppi_port_a[0] &= ~0x30;
  pc->ppi_port_a[0] |= 0x30;
}

void pc_setup_hgc (ibmpc_t *pc, ini_sct_t *sct)
{
  pc->video = hgc_new (pc->trm, sct);
  mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
  mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);

  pc->ppi_port_a[0] &= ~0x30;
  pc->ppi_port_a[0] |= 0x30;
}

void pc_setup_cga (ibmpc_t *pc, ini_sct_t *sct)
{
  pc->video = cga_new (pc->trm, sct);
  mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
  mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);

  pc->ppi_port_a[0] &= ~0x30;
  pc->ppi_port_a[0] |= 0x20;
}

void pc_setup_ega (ibmpc_t *pc, ini_sct_t *sct)
{
  pc->video = ega_new (pc->trm, sct);
  mem_add_blk (pc->mem, pce_video_get_mem (pc->video), 0);
  mem_add_blk (pc->prt, pce_video_get_reg (pc->video), 0);

  pc->ppi_port_a[0] &= ~0x30;
  pc->ppi_port_a[0] |= 0x00;
}

void pc_setup_video (ibmpc_t *pc, ini_sct_t *ini)
{
  char      *dev;
  ini_sct_t *sct;

  pc->video = NULL;

  sct = ini_sct_find_sct (ini, "video");
  ini_get_string (sct, "device", &dev, "mda");

  if (par_video != NULL) {
    while ((sct != NULL) && (strcmp (par_video, dev) != 0)) {
      sct = ini_sct_find_next (sct, "video");
      ini_get_string (sct, "device", &dev, "cga");
    }

    if (sct == NULL) {
      dev = par_video;
    }
  }

  pce_log (MSG_INF, "video:\tdevice=%s\n", dev);

  if (strcmp (dev, "ega") == 0) {
    pc_setup_ega (pc, sct);
  }
  else if (strcmp (dev, "cga") == 0) {
    pc_setup_cga (pc, sct);
  }
  else if (strcmp (dev, "hgc") == 0) {
    pc_setup_hgc (pc, sct);
  }
  else if (strcmp (dev, "mda") == 0) {
    pc_setup_mda (pc, sct);
  }
  else {
    pce_log (MSG_ERR, "unknown video device (%s)\n", dev);
  }
}

void pc_setup_disks (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t *sct;
  disk_t    *dsk;
  unsigned  drive;
  unsigned  c, h, s;
  int       ro;
  char      *fname, *type;

  pc->dsk = dsks_new();

  sct = ini_sct_find_sct (ini, "disk");

  while (sct != NULL) {
    ini_get_uint (sct, "drive", &drive, 0);
    ini_get_string (sct, "type", &type, "auto");
    ini_get_string (sct, "file", &fname, NULL);

    ini_get_uint (sct, "c", &c, 80);
    ini_get_uint (sct, "h", &h, 2);
    ini_get_uint (sct, "s", &s, 18);

    ini_get_sint (sct, "readonly", &ro, 0);

    if (strcmp (type, "ram") == 0) {
      dsk = dsk_ram_new (drive, c, h, s, fname, ro);
    }
    else if (strcmp (type, "image") == 0) {
      dsk = dsk_img_new (drive, c, h, s, 0, fname, ro);
    }
    else if (strcmp (type, "dosemu") == 0) {
      dsk = dsk_dosemu_new (drive, fname, ro);
    }
    else if (strcmp (type, "auto") == 0) {
      dsk = dsk_auto_new (drive, fname, ro);
    }
    else {
      dsk = NULL;
    }

    if (dsk == NULL) {
      pce_log (MSG_ERR, "loading drive %02X failed\n", drive);
    }
    else {
      pce_log (MSG_INF, "disk:\tdrive=%u type=%s chs=%u/%u/%u ro=%d file=%s\n",
        drive, type,
        dsk->c, dsk->h, dsk->s, ro,
        (fname != NULL) ? fname : "<>"
      );

      dsks_add_disk (pc->dsk, dsk);
    }

    sct = ini_sct_find_next (sct, "disk");
  }
}

void pc_setup_mouse (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t *sct;
  unsigned  base, irq;

  sct = ini_sct_find_sct (ini, "mouse");
  if (sct == NULL) {
    return;
  }

  ini_get_uint (sct, "io", &base, 0x03f8);
  ini_get_uint (sct, "irq", &irq, 4);

  pce_log (MSG_INF, "mouse:\tio=0x%04x irq=%u\n", base, irq);

  pc->mse = mse_new (base, sct);
  pc->mse->intr_ext = pc->pic;
  pc->mse->intr = (mse_intr_f) e8259_get_irq (pc->pic, irq);

  mem_add_blk (pc->prt, pc->mse->reg, 0);

  pc->trm->mse_ext = pc->mse;
  pc->trm->set_mse = (trm_set_mse_f) &mse_set;
}

void pc_setup_parport (ibmpc_t *pc, ini_sct_t *ini)
{
  unsigned        i;
  unsigned        base;
  char            *fname;
  ini_sct_t       *sct;
  static unsigned defbase[4] = { 0x378, 0x278, 0x3bc, 0x2bc };

  for (i = 0; i < 4; i++) {
    pc->parport[i] = NULL;
  }

  i = 0;
  sct = ini_sct_find_sct (ini, "parport");

  while ((i < 4) && (sct != NULL)) {
    ini_get_uint (sct, "io", &base, defbase[i]);
    ini_get_string (sct, "file", &fname, NULL);

    pce_log (MSG_INF, "LPT%u:\tio=0x%04x file=%s\n",
      i + 1, base, (fname == NULL) ? "<none>" : fname
    );

    pc->parport[i] = parport_new (base);
    if (pc->parport[i] == NULL) {
      pce_log (MSG_ERR, "parport setup failed [%04X -> %s]\n",
        base, (fname == NULL) ? "<none>" : fname
      );
    }
    else {
      if (fname != NULL) {
        parport_set_fname (pc->parport[i], fname);
      }

      mem_add_blk (pc->prt, pc->parport[i]->prt, 0);

      i += 1;
    }

    sct = ini_sct_find_next (sct, "parport");
  }
}

void pc_setup_serport (ibmpc_t *pc, ini_sct_t *ini)
{
  unsigned        i;
  unsigned        base, irq;
  char            *fname;
  ini_sct_t       *sct;
  static unsigned defbase[4] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };
  static unsigned defirq[4] = { 4, 3, 4, 3 };

  for (i = 0; i < 4; i++) {
    pc->serport[i] = NULL;
  }

  i = 0;
  sct = ini_sct_find_sct (ini, "serial");

  while ((i < 4) && (sct != NULL)) {
    ini_get_uint (sct, "io", &base, defbase[i]);
    ini_get_uint (sct, "irq", &irq, defirq[i]);
    ini_get_string (sct, "file", &fname, NULL);

    pce_log (MSG_INF, "COM%u:\tio=0x%04x irq=%u file=%s\n",
      i + 1, base, irq, (fname == NULL) ? "<none>" : fname
    );

    pc->serport[i] = ser_new (base);
    if (pc->serport[i] == NULL) {
      pce_log (MSG_ERR, "serial port setup failed [%04X/%u -> %s]\n",
        base, irq, (fname == NULL) ? "<none>" : fname
      );
    }
    else {
      if (fname != NULL) {
        if (ser_set_fname (pc->serport[i], fname)) {
          pce_log (MSG_ERR, "can't open file (%s)\n", fname);
        }
      }

      pc->serport[i]->uart.irq_ext = pc->pic;
      pc->serport[i]->uart.irq = (e8250_irq_f) e8259_get_irq (pc->pic, irq);

      mem_add_blk (pc->prt, pc->serport[i]->prt, 0);

      i += 1;
    }

    sct = ini_sct_find_next (sct, "serial");
  }
}

void pc_setup_ems (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t *sct;
  mem_blk_t *mem;

  pc->ems = NULL;

  sct = ini_sct_find_sct (ini, "ems");
  if (sct == NULL) {
    return;
  }

  pc->ems = ems_new (sct);
  if (pc->ems == NULL) {
    return;
  }

  mem = ems_get_mem (pc->ems);
  if (mem != NULL) {
    mem_add_blk (pc->mem, mem, 0);
  }
}

void pc_setup_xms (ibmpc_t *pc, ini_sct_t *ini)
{
  ini_sct_t *sct;
  mem_blk_t *mem;

  pc->xms = NULL;

  sct = ini_sct_find_sct (ini, "xms");
  if (sct == NULL) {
    return;
  }

  pc->xms = xms_new (sct);
  if (pc->xms == NULL) {
    return;
  }

  mem = xms_get_umb_mem (pc->xms);
  if (mem != NULL) {
    mem_add_blk (pc->mem, mem, 0);
  }

  mem = xms_get_hma_mem (pc->xms);
  if (mem != NULL) {
    mem_add_blk (pc->mem, mem, 0);
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
  mem_set_default (pc->mem, 0xff);

  pc->prt = mem_new();
  mem_set_default (pc->prt, 0xff);

  pc_setup_ram (pc, ini);
  pc_setup_rom (pc, ini);
  pc_setup_nvram (pc, ini);
  pc_setup_cpu (pc, ini);
  pc_setup_pic (pc);
  pc_setup_pit (pc);
  pc_setup_ppi (pc, ini);

  pc_setup_terminal (pc, ini);

  if (pc->trm != NULL) {
    pc_setup_video (pc, ini);
  }

  pc_setup_disks (pc, ini);
  pc_setup_mouse (pc, ini);
  pc_setup_serport (pc, ini);
  pc_setup_parport (pc, ini);
  pc_setup_ems (pc, ini);
  pc_setup_xms (pc, ini);

  return (pc);
}

void pc_del_xms (ibmpc_t *pc)
{
  xms_del (pc->xms);
  pc->xms = NULL;
}

void pc_del_ems (ibmpc_t *pc)
{
  ems_del (pc->ems);
  pc->ems = NULL;
}

void pc_del_mouse (ibmpc_t *pc)
{
  if (pc->mse != NULL) {
    mse_del (pc->mse);
    pc->mse = NULL;
  }
}

void pc_del_parport (ibmpc_t *pc)
{
  unsigned i;

  for (i = 0; i < 4; i++) {
    if (pc->parport[i] != NULL) {
      parport_del (pc->parport[i]);
    }
  }
}

void pc_del_serport (ibmpc_t *pc)
{
  unsigned i;

  for (i = 0; i < 4; i++) {
    if (pc->serport[i] != NULL) {
      ser_del (pc->serport[i]);
    }
  }
}

void pc_del (ibmpc_t *pc)
{
  if (pc == NULL) {
    return;
  }

  pc_del_xms (pc);
  pc_del_ems (pc);
  pc_del_parport (pc);
  pc_del_serport (pc);
  pc_del_mouse (pc);

  dsks_del (pc->dsk);

  pce_video_del (pc->video);

  trm_del (pc->trm);

  e8255_del (pc->ppi);
  e8253_del (pc->pit);
  e8259_del (pc->pic);
  e86_del (pc->cpu);

  nvr_del (pc->nvr);
  mem_del (pc->mem);
  mem_del (pc->prt);

  free (pc);
}

void pc_clock (ibmpc_t *pc)
{
  unsigned long n;

  n = e86_get_delay (pc->cpu);

  if (pc->clk_div[0] >= 16) {
    e8259_clock (pc->pic);
    e8253_clock (pc->pit, pc->clk_div[0] >> 2);
    pc->clk_div[0] &= 3;
  }

  if (pc->clk_div[2] >= 4096) {
    unsigned      i;
    unsigned long clk;

    clk = pc->clk_div[2] & ~4095;

    pce_video_clock (pc->video, clk);

    trm_check (pc->trm);

    for (i = 0; i < 4; i++) {
      if (pc->serport[i] != NULL) {
        ser_clock (pc->serport[i], clk);
      }
    }

    pc->clk_div[2] &= 4095;
  }

  if (pc->key_clk > 0) {
    pc->key_clk = (n < pc->key_clk) ? (pc->key_clk - n) : 0;

    if ((pc->key_clk == 0) && (pc->key_i < pc->key_j)) {
      pc->ppi_port_a[1] = pc->key_buf[pc->key_i];
      e8259_set_irq1 (pc->pic, 1);
      pc->key_i += 1;

      if (pc->key_i < pc->key_j) {
        pc->key_clk = 4096;
      }
      else {
        pc->key_i = 0;
        pc->key_j = 0;
      }
    }
  }

  e86_clock (pc->cpu, n);

  pc->clk_cnt += n;
  pc->clk_div[0] += n;
//  pc->clk_div[1] += n;
  pc->clk_div[2] += n;
//  pc->clk_div[3] += n;
}

void pc_screenshot (ibmpc_t *pc)
{
  static unsigned i = 0;
  char            fname[256];
  FILE            *fp;

  sprintf (fname, "snap%04u.dat", i++);
  fp = fopen (fname, "wb");
  if (fp == NULL) {
    return;
  }

  pce_video_screenshot (pc->video, fp, 0);

  fclose (fp);
}

int pc_set_cpu_model (ibmpc_t *pc, unsigned model)
{
  switch (model) {
    case PCE_CPU_8086:
    case PCE_CPU_8088:
      e86_enable_86 (pc->cpu);
      break;

    case PCE_CPU_V20:
    case PCE_CPU_V30:
      e86_enable_v30 (pc->cpu);
      break;

    case PCE_CPU_80186:
    case PCE_CPU_80188:
      e86_enable_186 (pc->cpu);
      break;

    case PCE_CPU_80286:
      e86_enable_286 (pc->cpu);
      break;

    default:
      return (1);
  }

  pc->cpu_model = model;

  return (0);
}

void pc_break (ibmpc_t *pc, unsigned char val)
{
  if ((val == PCE_BRK_STOP) || (val == PCE_BRK_ABORT)) {
    pc->brk = val;
  }
  else if (val == PCE_BRK_SNAP) {
    pc_screenshot (pc);
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

  e8253_cnt_set_gate (&pc->pit->counter[2], val & 0x01);
}

void pc_set_keycode (ibmpc_t *pc, unsigned char val)
{
//  pce_log (MSG_DEB, "keycode: %02X\n", val);

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
