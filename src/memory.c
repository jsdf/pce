/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     memory.c                                                   *
 * Created:       2000-04-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: memory.c,v 1.4 2003/04/18 20:04:58 hampa Exp $ */


#include <stdlib.h>

#include <pce.h>


mem_blk_t *mem_blk_new (unsigned long base, unsigned long size, int alloc)
{
  mem_blk_t *blk;

  blk = (mem_blk_t *) malloc (sizeof (mem_blk_t));
  if (blk == NULL) {
    return (NULL);
  }

  if (alloc) {
    blk->data = (unsigned char *) malloc (size);
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
  memset (blk->data, val, blk->size);
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


unsigned char mem_get_uint8 (void *obj, unsigned long addr)
{
  unsigned  i;
  memory_t  *mem;
  mem_blk_t *blk;

  mem = (memory_t *) obj;

  for (i = 0; i < mem->blk_cnt; i++) {
    blk = mem->blk[i];

    if ((addr >= blk->base) && (addr <= blk->end)) {
      if (blk->get_uint8 != NULL) {
        return (blk->get_uint8 (blk->ext, addr - blk->base));
      }
      else {
        return (blk->data[addr - blk->base]);
      }
    }
  }

  return (mem->def_val);
}

unsigned short mem_get_uint16_le (void *obj, unsigned long addr)
{
  unsigned  i;
  memory_t  *mem;
  mem_blk_t *blk;

  mem = (memory_t *) obj;

  for (i = 0; i < mem->blk_cnt; i++) {
    blk = mem->blk[i];

    if ((addr >= blk->base) && (addr <= blk->end)) {
      if (blk->get_uint16 != NULL) {
        return (blk->get_uint16 (blk->ext, addr - blk->base));
      }
      else {
        if (addr < blk->end) {
          addr -= blk->base;
          return (blk->data[addr] + (blk->data[addr + 1] << 8));
        }
        else {
          return (blk->data[addr - blk->base] | mem->def_val);
        }
      }
    }
  }

  return ((mem->def_val << 8) | mem->def_val);
}

void mem_set_uint8 (void *obj, unsigned long addr, unsigned char val)
{
  unsigned  i;
  memory_t  *mem;
  mem_blk_t *blk;

  mem = (memory_t *) obj;

  for (i = 0; i < mem->blk_cnt; i++) {
    blk = mem->blk[i];

    if ((addr >= blk->base) && (addr <= blk->end)) {
      if (blk->flags & MEM_FLAG_RO) {
        return;
      }
      else if (blk->set_uint8 != NULL) {
        blk->set_uint8 (blk->ext, addr - blk->base, val);
      }
      else {
        blk->data[addr - blk->base] = val;
      }
    }
  }
}

void mem_set_uint16_le (void *obj, unsigned long addr, unsigned short val)
{
  unsigned  i;
  memory_t  *mem;
  mem_blk_t *blk;

  mem = (memory_t *) obj;

  for (i = 0; i < mem->blk_cnt; i++) {
    blk = mem->blk[i];

    if ((addr >= blk->base) && (addr <= blk->end)) {
      if (blk->flags & MEM_FLAG_RO) {
        return;
      }
      else if (blk->set_uint16 != NULL) {
        blk->set_uint16 (blk->ext, addr - blk->base, val);
      }
      else {
        blk->data[addr - blk->base] = val & 0xff;

        if (addr < blk->end) {
          blk->data[addr - blk->base + 1] = (val >> 8) & 0xff;
        }
      }
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

  mem->blk_cnt = 0;
  mem->blk = NULL;

  mem->def_val = 0xaa;

  return (mem);
}

void mem_del (memory_t *mem)
{
  if (mem != NULL) {
    free (mem->blk);
    free (mem);
  }
}

void mem_add_blk (memory_t *mem, mem_blk_t *blk)
{
  mem->blk = (mem_blk_t **) realloc (mem->blk, mem->blk_cnt + 1);
  if (mem->blk == NULL) {
    return;
  }

  mem->blk[mem->blk_cnt] = blk;

  mem->blk_cnt += 1;
}
