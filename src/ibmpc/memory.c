/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/memory.c                                         *
 * Created:       2000-04-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-06 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: memory.c,v 1.6 2003/09/08 17:16:06 hampa Exp $ */


#include <stdlib.h>
#include <string.h>

#include "memory.h"


mem_blk_t *mem_blk_new (unsigned long base, unsigned long size, int alloc)
{
  mem_blk_t *blk;

  blk = (mem_blk_t *) malloc (sizeof (mem_blk_t));
  if (blk == NULL) {
    return (NULL);
  }

  if (alloc) {
    blk->data = (unsigned char *) malloc (size + 16);
    if (blk->data == NULL) {
      free (blk);
      return (NULL);
    }
  }
  else {
    blk->data = NULL;
  }

  blk->get_uint8 = NULL;
  blk->get_uint16 = NULL;
  blk->set_uint8 = NULL;
  blk->set_uint16 = NULL;

  blk->ext = blk;

  blk->flags = 0;
  blk->base = base;
  blk->size = size;
  blk->end = base + size - 1;

  return (blk);
}

void mem_blk_del (mem_blk_t *blk)
{
  if (blk != NULL) {
    free (blk->data);
    free (blk);
  }
}

void mem_blk_init (mem_blk_t *blk, unsigned char val)
{
  if (blk->data != NULL) {
    memset (blk->data, val, blk->size);
  }
}

void mem_blk_set_ext (mem_blk_t *blk, void *ext)
{
  blk->ext = ext;
}

void mem_blk_set_ro (mem_blk_t *blk, int ro)
{
  if (ro) {
    blk->flags |= MEM_FLAG_RO;
  }
  else {
    blk->flags &= ~MEM_FLAG_RO;
  }
}

unsigned long mem_blk_get_size (mem_blk_t *blk)
{
  return (blk->size);
}


unsigned char mem_get_uint8 (memory_t *mem, unsigned long addr)
{
  unsigned  i;
  mem_blk_t *blk;

  for (i = 0; i < mem->cnt; i++) {
    blk = mem->lst[i].blk;

    if ((addr >= blk->base) && (addr <= blk->end)) {
      addr -= blk->base;

      if (blk->get_uint8 != NULL) {
        return (blk->get_uint8 (blk->ext, addr));
      }
      else {
        return (blk->data[addr]);
      }
    }
  }

  return (mem->def_val);
}

unsigned short mem_get_uint16_le (memory_t *mem, unsigned long addr)
{
  unsigned       i;
  unsigned short val;
  mem_blk_t      *blk;

  for (i = 0; i < mem->cnt; i++) {
    blk = mem->lst[i].blk;

    if ((addr >= blk->base) && (addr <= blk->end)) {
      if ((addr + 1) > blk->end) {
        val = mem_get_uint8 (mem, addr);
        val += (unsigned short) mem_get_uint8 (mem, addr + 1) << 8;
        return (val);
      }

      addr -= blk->base;

      if (blk->get_uint16 != NULL) {
        return (blk->get_uint16 (blk->ext, addr));
      }
      else {
        val = blk->data[addr];
        val += (unsigned short) blk->data[addr + 1] << 8;
        return (val);
      }
    }
  }

  return (((unsigned short) mem->def_val << 8) | mem->def_val);
}

void mem_set_uint8 (memory_t *mem, unsigned long addr, unsigned char val)
{
  unsigned  i;
  mem_blk_t *blk;

  for (i = 0; i < mem->cnt; i++) {
    blk = mem->lst[i].blk;

    if ((addr >= blk->base) && (addr <= blk->end)) {
      addr -= blk->base;

      if (blk->flags & MEM_FLAG_RO) {
        return;
      }

      if (blk->set_uint8 != NULL) {
        blk->set_uint8 (blk->ext, addr, val);
      }
      else {
        blk->data[addr] = val;
      }

      return;
    }
  }
}

void mem_set_uint16_le (memory_t *mem, unsigned long addr, unsigned short val)
{
  unsigned  i;
  mem_blk_t *blk;

  for (i = 0; i < mem->cnt; i++) {
    blk = mem->lst[i].blk;

    if ((addr >= blk->base) && (addr <= blk->end)) {
      if ((addr + 1) > blk->end) {
        mem_set_uint8 (mem, addr, val & 0xff);
        mem_set_uint8 (mem, addr + 1, (val >> 8) & 0xff);
        return;
      }

      addr -= blk->base;

      if (blk->flags & MEM_FLAG_RO) {
        return;
      }

      if (blk->set_uint16 != NULL) {
        blk->set_uint16 (blk->ext, addr, val);
      }
      else {
        blk->data[addr] = val & 0xff;
        blk->data[addr + 1] = (val >> 8) & 0xff;
      }

      return;
    }
  }
}

memory_t *mem_new (void)
{
  memory_t *mem;

  mem = (memory_t *) malloc (sizeof (memory_t));
  if (mem == NULL) {
    return (NULL);
  }

  mem->cnt = 0;
  mem->lst = NULL;

  mem->def_val = 0xaa;

  return (mem);
}

void mem_del (memory_t *mem)
{
  unsigned i;

  if (mem != NULL) {
    for (i = 0; i < mem->cnt; i++) {
      if (mem->lst[i].del) {
        mem_blk_del (mem->lst[i].blk);
      }
    }
    free (mem->lst);
    free (mem);
  }
}

void mem_add_blk (memory_t *mem, mem_blk_t *blk, int del)
{
  mem->lst = (mem_lst_t *) realloc (mem->lst, (mem->cnt + 1) * sizeof (mem_lst_t));
  if (mem->lst == NULL) {
    return;
  }

  mem->lst[mem->cnt].blk = blk;
  mem->lst[mem->cnt].del = del;

  mem->cnt += 1;
}
