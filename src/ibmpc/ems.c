/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/ems.c                                            *
 * Created:       2003-10-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-10-18 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: ems.c,v 1.1 2003/10/18 03:28:11 hampa Exp $ */


#include "pce.h"


ems_block_t *ems_blk_new (unsigned handle, unsigned pages)
{
  ems_block_t *blk;

  blk = (ems_block_t *) malloc (sizeof (ems_block_t));
  if (blk == NULL) {
    return (NULL);
  }

  blk->handle = handle;
  blk->pages = pages;

  blk->data = (unsigned char *) malloc (16384UL * pages);
  if (blk->data == NULL) {
    free (blk);
    return (NULL);
  }

  blk->map_saved = 0;

  return (blk);
}

void ems_blk_del (ems_block_t *blk)
{
  if (blk != NULL) {
    free (blk->data);
    free (blk);
  }
}


ems_t *ems_new (ini_sct_t *sct)
{
  unsigned      i;
  unsigned long pages;
  unsigned      segm;
  ems_t         *ems;

  ini_get_ulng (sct, "size", &pages, 0);
  pages = (pages + 15) / 16;

  ini_get_uint (sct, "segment", &segm, 0xe000);

  ems = (ems_t *) malloc (sizeof (ems_t));
  if (ems == NULL) {
    return (NULL);
  }

  for (i = 0; i < 256; i++) {
    ems->blk[i] = NULL;
  }

  ems->blk[0] = ems_blk_new (0, 4);

  ems->pages_max = pages;
  ems->pages_used = 0;

  for (i = 0; i < 4; i++) {
    ems->map_blk[i] = NULL;
    ems->map_page[i] = 0;
  }

  ems->mem = mem_blk_new (16 * segm, 65536, 0);
  ems->mem->ext = ems;
  ems->mem->get_uint8 = (geta_uint8_f) &ems_get_uint8;
  ems->mem->set_uint8 = (seta_uint8_f) &ems_set_uint8;
  ems->mem->get_uint16 = (geta_uint16_f) &ems_get_uint16;
  ems->mem->set_uint16 = (seta_uint16_f) &ems_set_uint16;

  pce_log (MSG_INF, "ems:\tsize=%luK (%lu pages) segment=0x%04x\n",
    16UL * pages, ems->pages_max, segm
  );

  return (ems);
}

void ems_del (ems_t *ems)
{
  unsigned i;

  if (ems != NULL) {
    for (i = 0; i < 256; i++) {
      ems_blk_del (ems->blk[i]);
    }

    mem_blk_del (ems->mem);

    free (ems);
  }
}

mem_blk_t *ems_get_mem (ems_t *ems)
{
  return (ems->mem);
}

void ems_prt_state (ems_t *ems, FILE *fp)
{
  unsigned i, n;

  n = 0;
  for (i = 1; i < 255; i++) {
    if (ems->blk[i] != NULL) {
      n += 1;
    }
  }

  fprintf (fp,
    "EMS: %u/%u pages  %luK/%luK  %u handles\n",
    ems->pages_used, ems->pages_max,
    16UL * ems->pages_used, 16UL * ems->pages_max,
    n
  );

  fprintf (fp,
    "  mapping: [%u/%u %u/%u %u/%u %u/%u]\n",
    (ems->map_blk[0] != NULL) ? ems->map_blk[0]->handle : 0xfff,
    ems->map_page[0],
    (ems->map_blk[1] != NULL) ? ems->map_blk[1]->handle : 0xfff,
    ems->map_page[1],
    (ems->map_blk[2] != NULL) ? ems->map_blk[2]->handle : 0xfff,
    ems->map_page[2],
    (ems->map_blk[3] != NULL) ? ems->map_blk[3]->handle : 0xfff,
    ems->map_page[3]
  );

  for (i = 0; i < 256; i++) {
    if (ems->blk[i] != NULL) {
      fprintf (fp, "  %u: %u pages\n", i, ems->blk[i]->pages);
    }
  }

  fflush (fp);
}

void ems_info (ems_t *ems, e8086_t *cpu)
{
  if (ems != NULL) {
    e86_set_ax (cpu, 0x0001);
    e86_set_dx (cpu, 16 * ems->pages_max);
  }
  else {
    e86_set_ax (cpu, 0x0000);
    e86_set_dx (cpu, 0x0000);
  }
}

unsigned char ems_get_uint8 (ems_t *ems, unsigned long addr)
{
  unsigned page, offs;

  if (addr > 65535) {
    return (0xff);
  }

  page = addr / 16384;
  offs = addr % 16384;

  if (ems->map_blk[page] == NULL) {
    return (0xff);
  }

  addr = 16384UL * ems->map_page[page] + offs;

  return (ems->map_blk[page]->data[addr]);
}

void ems_set_uint8 (ems_t *ems, unsigned long addr, unsigned char val)
{
  unsigned page, offs;

  if (addr > 65535) {
    return;
  }

  page = addr / 16384;
  offs = addr % 16384;

  if (ems->map_blk[page] == NULL) {
    return;
  }

  addr = 16384UL * ems->map_page[page] + offs;

  ems->map_blk[page]->data[addr] = val;
}

unsigned short ems_get_uint16 (ems_t *ems, unsigned long addr)
{
  unsigned short ret;

  ret = ems_get_uint8 (ems, addr) + (ems_get_uint8 (ems, addr + 1) << 8);

  return (ret);
}

void ems_set_uint16 (ems_t *ems, unsigned long addr, unsigned short val)
{
  ems_set_uint8 (ems, addr, val & 0xff);
  ems_set_uint8 (ems, addr + 1, (val >> 8) & 0xff);
}


/* 40: get status */
void ems_40 (ems_t *ems, e8086_t *cpu)
{
  e86_set_ah (cpu, 0x00);
}

/* 41: get page frame address */
void ems_41 (ems_t *ems, e8086_t *cpu)
{
  e86_set_ah (cpu, 0x00);
  e86_set_bx (cpu, ems->mem->base / 16);
}

/* 42: get page count */
void ems_42 (ems_t *ems, e8086_t *cpu)
{
  e86_set_ah (cpu, 0x00);
  e86_set_bx (cpu, ems->pages_max - ems->pages_used);
  e86_set_dx (cpu, ems->pages_max);
}

/* 43: allocate pages */
void ems_43 (ems_t *ems, e8086_t *cpu)
{
  unsigned i;
  unsigned cnt;

  cnt = e86_get_bx (cpu);

  if (cnt == 0) {
    e86_set_ah (cpu, 0x89);
    return;
  }

  if (cnt > ems->pages_max) {
    e86_set_ah (cpu, 0x87);
    return;
  }

  if ((ems->pages_used + cnt) > ems->pages_max) {
    e86_set_ah (cpu, 0x88);
    return;
  }

  for (i = 1; i < 255; i++) {
    if (ems->blk[i] == NULL) {
      ems->blk[i] = ems_blk_new (i, cnt);
      ems->pages_used += cnt;
      e86_set_ah (cpu, 0x00);
      e86_set_dx (cpu, i);
      return;
    }
  }

  e86_set_ah (cpu, 0x85);
}

/* 44: map a page */
void ems_44 (ems_t *ems, e8086_t *cpu)
{
  unsigned fpage, spage, handle;

  fpage = e86_get_al (cpu);
  spage = e86_get_bx (cpu);
  handle = e86_get_dx (cpu);

  if (fpage > 3) {
    e86_set_ah (cpu, 0x8b);
    return;
  }

  if ((handle > 255) || (ems->blk[handle] == NULL)) {
    e86_set_ah (cpu, 0x83);
    return;
  }

  if (spage == 0xffff) {
    ems->map_blk[fpage] = NULL;
    ems->map_page[fpage] = 0;
    e86_set_ah (cpu, 0x00);
    return;
  }

  if (spage >= ems->blk[handle]->pages) {
    e86_set_ah (cpu, 0xa8);
    return;
  }

  ems->map_blk[fpage] = ems->blk[handle];
  ems->map_page[fpage] = spage;

  e86_set_ah (cpu, 0x00);
}

/* 45: deallocate pages */
void ems_45 (ems_t *ems, e8086_t *cpu)
{
  unsigned    i;
  unsigned    handle;
  ems_block_t *blk;

  handle = e86_get_dx (cpu);

  if ((handle > 255) || (ems->blk[handle] == NULL)) {
    e86_set_ah (cpu, 0x83);
    return;
  }

  blk = ems->blk[handle];

  if (blk->map_saved) {
    e86_set_ah (cpu, 0x86);
    return;
  }

  for (i = 0; i < 4; i++) {
    if (ems->map_blk[i] == blk) {
      ems->map_blk[i] = NULL;
      ems->map_page[i] = 0;
    }
  }

  ems->pages_used -= blk->pages;

  ems->blk[handle] = NULL;

  ems_blk_del (blk);

  e86_set_ah (cpu, 0x00);
}

/* 46: get version */
void ems_46 (ems_t *ems, e8086_t *cpu)
{
  e86_set_ah (cpu, 0x00);
  e86_set_al (cpu, 0x32);
}

/* 47: save page map */
void ems_47 (ems_t *ems, e8086_t *cpu)
{
  unsigned    i;
  unsigned    handle;
  ems_block_t *blk;

  handle = e86_get_dx (cpu);

  if ((handle > 255) || (ems->blk[handle] == NULL)) {
    e86_set_ah (cpu, 0x83);
    return;
  }

  blk = ems->blk[handle];

  if (blk->map_saved) {
    e86_set_ah (cpu, 0x8d);
    return;
  }

  for (i = 0; i < 4; i++) {
    blk->map_blk[i] = ems->map_blk[i];
    blk->map_page[i] = ems->map_page[i];
  }

  blk->map_saved = 1;

  e86_set_ah (cpu, 0x00);
}

/* 48: restore page map */
void ems_48 (ems_t *ems, e8086_t *cpu)
{
  unsigned    i;
  unsigned    handle;
  ems_block_t *blk;

  handle = e86_get_dx (cpu);

  if ((handle > 255) || (ems->blk[handle] == NULL)) {
    e86_set_ah (cpu, 0x83);
    return;
  }

  blk = ems->blk[handle];

  if (blk->map_saved == 0) {
    e86_set_ah (cpu, 0x8e);
    return;
  }

  for (i = 0; i < 4; i++) {
    ems->map_blk[i] = blk->map_blk[i];
    ems->map_page[i] = blk->map_page[i];
  }

  blk->map_saved = 0;

  e86_set_ah (cpu, 0x00);
}

/* 4b: get handle count */
void ems_4b (ems_t *ems, e8086_t *cpu)
{
  unsigned i, n;

  n = 0;

  for (i = 0; i < 255; i++) {
    if (ems->blk[i] != NULL) {
      n += 1;
    }
  }

  e86_set_ah (cpu, 0x00);
  e86_set_bx (cpu, n);
}

/* 4c: get handle page count */
void ems_4c (ems_t *ems, e8086_t *cpu)
{
  unsigned handle;

  handle = e86_get_dx (cpu);

  if ((handle > 255) || (ems->blk[handle] == NULL)) {
    e86_set_ah (cpu, 0x83);
    return;
  }

  e86_set_ah (cpu, 0x00);
  e86_set_bx (cpu, ems->blk[handle]->pages);
}

/* 4d: get all handles page count */
void ems_4d (ems_t *ems, e8086_t *cpu)
{
  unsigned       i, n;
  unsigned short segm, offs;

  segm = e86_get_es (cpu);
  offs = e86_get_di (cpu);

  n = 0;

  for (i = 0; i < 255; i++) {
    if (ems->blk[i] != NULL) {
      n += 1;
      e86_set_mem16 (cpu, segm, offs + 4 * i, i);
      e86_set_mem16 (cpu, segm, offs + 4 * i + 2, ems->blk[i]->pages);
    }
  }

  e86_set_ah (cpu, 0x00);
  e86_set_bx (cpu, n);
}

void ems_handler (ems_t *ems, e8086_t *cpu)
{
  if (ems == NULL) {
    return;
  }

  switch (e86_get_ah (cpu)) {
    case 0x40:
      ems_40 (ems, cpu);
      break;

    case 0x41:
      ems_41 (ems, cpu);
      break;

    case 0x42:
      ems_42 (ems, cpu);
      break;

    case 0x43:
      ems_43 (ems, cpu);
      break;

    case 0x44:
      ems_44 (ems, cpu);
      break;

    case 0x45:
      ems_45 (ems, cpu);
      break;

    case 0x46:
      ems_46 (ems, cpu);
      break;

    case 0x47:
      ems_47 (ems, cpu);
      break;

    case 0x48:
      ems_48 (ems, cpu);
      break;

    case 0x4b:
      ems_4b (ems, cpu);
      break;

    case 0x4c:
      ems_4c (ems, cpu);
      break;

    case 0x4d:
      ems_4d (ems, cpu);
      break;

    default:
      pce_log (MSG_MSG, "ems:\tunknown function: AH=%02X\n", e86_get_ah (cpu));
      e86_set_ah (cpu, 0x84);
      break;
  }
}
