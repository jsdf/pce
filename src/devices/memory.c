/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/memory.c                                       *
 * Created:       2000-04-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-08-02 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2004 Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdlib.h>
#include <string.h>

#include "memory.h"


int mem_blk_init (mem_blk_t *blk, unsigned long base, unsigned long size, int alloc)
{
  if (alloc) {
    blk->data = (unsigned char *) malloc (size + 16);
    if (blk->data == NULL) {
      return (1);
    }
  }
  else {
    blk->data = NULL;
  }

  blk->get_uint8 = NULL;
  blk->get_uint16 = NULL;
  blk->get_uint32 = NULL;
  blk->set_uint8 = NULL;
  blk->set_uint16 = NULL;
  blk->set_uint32 = NULL;

  blk->ext = blk;

  blk->flags = 0;
  blk->addr1 = base;
  blk->addr2 = base + size - 1;
  blk->size = size;

  return (0);
}

mem_blk_t *mem_blk_new (unsigned long base, unsigned long size, int alloc)
{
  mem_blk_t *blk;

  blk = (mem_blk_t *) malloc (sizeof (mem_blk_t));
  if (blk == NULL) {
    return (NULL);
  }

  if (mem_blk_init (blk, base, size, alloc)) {
    free (blk);
    return (NULL);
  }

  return (blk);
}

void mem_blk_free (mem_blk_t *blk)
{
  if (blk != NULL) {
    free (blk->data);
  }
}

void mem_blk_del (mem_blk_t *blk)
{
  if (blk != NULL) {
    mem_blk_free (blk);
    free (blk);
  }
}

void mem_blk_clear (mem_blk_t *blk, unsigned char val)
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

unsigned long mem_blk_get_addr (const mem_blk_t *blk)
{
  return (blk->addr1);
}

unsigned long mem_blk_get_size (const mem_blk_t *blk)
{
  return (blk->size);
}

unsigned char *mem_blk_get_data (mem_blk_t *blk)
{
  return (blk->data);
}


void buf_set_uint8 (void *buf, unsigned long addr, unsigned char val)
{
  unsigned char *tmp = (unsigned char *) buf;

  tmp[addr] = val;
}

void buf_set_uint16_be (void *buf, unsigned long addr, unsigned short val)
{
  unsigned char *tmp = (unsigned char *) buf;

  tmp[addr] = (val >> 8) & 0xff;
  tmp[addr + 1] = val & 0xff;
}

void buf_set_uint16_le (void *buf, unsigned long addr, unsigned short val)
{
  unsigned char *tmp = (unsigned char *) buf;

  tmp[addr] = val & 0xff;
  tmp[addr + 1] = (val >> 8) & 0xff;
}

void buf_set_uint32_be (void *buf, unsigned long addr, unsigned long val)
{
  unsigned char *tmp = (unsigned char *) buf;

  tmp[addr] = (val >> 24) & 0xff;
  tmp[addr + 1] = (val >> 16) & 0xff;
  tmp[addr + 2] = (val >> 8) & 0xff;
  tmp[addr + 3] = val & 0xff;
}

void buf_set_uint32_le (void *buf, unsigned long addr, unsigned long val)
{
  unsigned char *tmp = (unsigned char *) buf;

  tmp[addr] = val & 0xff;
  tmp[addr + 1] = (val >> 8) & 0xff;
  tmp[addr + 2] = (val >> 16) & 0xff;
  tmp[addr + 3] = (val >> 24) & 0xff;
}

unsigned char buf_get_uint8 (const void *buf, unsigned long addr)
{
  const unsigned char *tmp = (const unsigned char *) buf;

  return (tmp[addr]);
}

unsigned short buf_get_uint16_be (const void *buf, unsigned long addr)
{
  const unsigned char *tmp = (const unsigned char *) buf;
  unsigned short      ret;

  ret = tmp[addr + 1];
  ret = (ret << 8) | tmp[addr];

  return (ret);
}

unsigned short buf_get_uint16_le (const void *buf, unsigned long addr)
{
  const unsigned char *tmp = (const unsigned char *) buf;
  unsigned short      ret;

  ret = tmp[addr];
  ret = (ret << 8) | tmp[addr + 1];

  return (ret);
}

unsigned long buf_get_uint32_be (const void *buf, unsigned long addr)
{
  const unsigned char *tmp = (const unsigned char *) buf;
  unsigned long       ret;

  ret = tmp[addr + 3];
  ret = (ret << 8) | tmp[addr + 2];
  ret = (ret << 8) | tmp[addr + 1];
  ret = (ret << 8) | tmp[addr];

  return (ret);
}

unsigned long buf_get_uint32_le (const void *buf, unsigned long addr)
{
  const unsigned char *tmp = (const unsigned char *) buf;
  unsigned long       ret;

  ret = tmp[addr];
  ret = (ret << 8) | tmp[addr + 1];
  ret = (ret << 8) | tmp[addr + 2];
  ret = (ret << 8) | tmp[addr + 3];

  return (ret);
}


void mem_blk_set_uint8 (mem_blk_t *blk, unsigned long addr, unsigned char val)
{
  blk->data[addr] = val;
}

void mem_blk_set_uint16_be (mem_blk_t *blk, unsigned long addr, unsigned short val)
{
  blk->data[addr] = (val >> 8) & 0xff;
  blk->data[addr + 1] = val & 0xff;
}

void mem_blk_set_uint16_le (mem_blk_t *blk, unsigned long addr, unsigned short val)
{
  blk->data[addr] = val & 0xff;
  blk->data[addr + 1] = (val >> 8) & 0xff;
}

void mem_blk_set_uint32_be (mem_blk_t *blk, unsigned long addr, unsigned long val)
{
  blk->data[addr] = (val >> 24) & 0xff;
  blk->data[addr + 1] = (val >> 16) & 0xff;
  blk->data[addr + 2] = (val >> 8) & 0xff;
  blk->data[addr + 3] = val & 0xff;
}

void mem_blk_set_uint32_le (mem_blk_t *blk, unsigned long addr, unsigned long val)
{
  blk->data[addr] = val & 0xff;
  blk->data[addr + 1] = (val >> 8) & 0xff;
  blk->data[addr + 2] = (val >> 16) & 0xff;
  blk->data[addr + 3] = (val >> 24) & 0xff;
}

unsigned char mem_blk_get_uint8 (const mem_blk_t *blk, unsigned long addr)
{
  return (blk->data[addr]);
}

unsigned short mem_blk_get_uint16_be (const mem_blk_t *blk, unsigned long addr)
{
  unsigned short ret;

  ret = blk->data[addr];
  ret = (ret << 8) | blk->data[addr + 1];

  return (ret);
}

unsigned short mem_blk_get_uint16_le (const mem_blk_t *blk, unsigned long addr)
{
  unsigned short ret;

  ret = blk->data[addr + 1];
  ret = (ret << 8) | blk->data[addr];

  return (ret);
}

unsigned long mem_blk_get_uint32_be (const mem_blk_t *blk, unsigned long addr)
{
  unsigned long ret;

  ret = blk->data[addr];
  ret = (ret << 8) | blk->data[addr + 1];
  ret = (ret << 8) | blk->data[addr + 2];
  ret = (ret << 8) | blk->data[addr + 3];

  return (ret);
}

unsigned long mem_blk_get_uint32_le (const mem_blk_t *blk, unsigned long addr)
{
  unsigned long ret;

  ret = blk->data[addr + 3];
  ret = (ret << 8) | blk->data[addr + 2];
  ret = (ret << 8) | blk->data[addr + 1];
  ret = (ret << 8) | blk->data[addr];

  return (ret);
}


static
mem_blk_t *mem_get_blk (memory_t *mem, unsigned long addr)
{
  unsigned  i;
  mem_lst_t *lst;

  if (mem->last != NULL) {
    lst = mem->last;
    if ((addr >= lst->addr1) && (addr <= lst->addr2)) {
      return (lst->blk);
    }
  }

  lst = mem->lst;

  for (i = 0; i < mem->cnt; i++) {
    if ((addr >= lst->addr1) && (addr <= lst->addr2)) {
      mem->last = lst;
      return (lst->blk);
    }

    lst += 1;
  }

  return (NULL);
}

unsigned char mem_get_uint8 (memory_t *mem, unsigned long addr)
{
  mem_blk_t *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    addr -= blk->addr1;

    if (blk->get_uint8 != NULL) {
      return (blk->get_uint8 (blk->ext, addr));
    }
    else {
      return (blk->data[addr]);
    }
  }

  return (mem->def_val8);
}

unsigned short mem_get_uint16_be (memory_t *mem, unsigned long addr)
{
  unsigned short val;
  mem_blk_t      *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if ((addr + 1) > blk->addr2) {
      val = (unsigned short) mem_get_uint8 (mem, addr) << 8;
      val |= mem_get_uint8 (mem, addr + 1);
      return (val);
    }

    addr -= blk->addr1;

    if (blk->get_uint16 != NULL) {
      return (blk->get_uint16 (blk->ext, addr));
    }
    else {
      val = (unsigned short) blk->data[addr] << 8;
      val |= blk->data[addr + 1];
      return (val);
    }
  }

  return (mem->def_val16);
}

unsigned short mem_get_uint16_le (memory_t *mem, unsigned long addr)
{
  unsigned short val;
  mem_blk_t      *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if ((addr + 1) > blk->addr2) {
      val = mem_get_uint8 (mem, addr);
      val |= (unsigned short) mem_get_uint8 (mem, addr + 1) << 8;
      return (val);
    }

    addr -= blk->addr1;

    if (blk->get_uint16 != NULL) {
      return (blk->get_uint16 (blk->ext, addr));
    }
    else {
      val = blk->data[addr];
      val |= (unsigned short) blk->data[addr + 1] << 8;
      return (val);
    }
  }

  return (mem->def_val16);
}

unsigned long mem_get_uint32_be (memory_t *mem, unsigned long addr)
{
  unsigned long val;
  mem_blk_t     *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if ((addr + 3) > blk->addr2) {
      val = (unsigned long) mem_get_uint8 (mem, addr) << 24;
      val |= (unsigned long) mem_get_uint8 (mem, addr + 1) << 16;
      val |= (unsigned long) mem_get_uint8 (mem, addr + 2) << 8;
      val |= mem_get_uint8 (mem, addr + 3);
      return (val);
    }

    addr -= blk->addr1;

    if (blk->get_uint32 != NULL) {
      return (blk->get_uint32 (blk->ext, addr));
    }
    else {
      val = (unsigned long) blk->data[addr] << 24;
      val |= (unsigned long) blk->data[addr + 1] << 16;
      val |= (unsigned long) blk->data[addr + 2] << 8;
      val |= blk->data[addr + 3];
      return (val);
    }
  }

  return (mem->def_val32);
}

void mem_set_uint8_rw (memory_t *mem, unsigned long addr, unsigned char val)
{
  mem_blk_t *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    addr -= blk->addr1;

    if (blk->set_uint8 != NULL) {
      blk->set_uint8 (blk->ext, addr, val);
    }
    else {
      blk->data[addr] = val;
    }
  }
}

void mem_set_uint8 (memory_t *mem, unsigned long addr, unsigned char val)
{
  mem_blk_t *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if (blk->flags & MEM_FLAG_RO) {
      return;
    }

    addr -= blk->addr1;

    if (blk->set_uint8 != NULL) {
      blk->set_uint8 (blk->ext, addr, val);
    }
    else {
      blk->data[addr] = val;
    }
  }
}

void mem_set_uint16_be (memory_t *mem, unsigned long addr, unsigned short val)
{
  mem_blk_t *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if ((addr + 1) > blk->addr2) {
      mem_set_uint8 (mem, addr, (val >> 8) & 0xff);
      mem_set_uint8 (mem, addr + 1, val & 0xff);
      return;
    }

    if (blk->flags & MEM_FLAG_RO) {
      return;
    }

    addr -= blk->addr1;

    if (blk->set_uint16 != NULL) {
      blk->set_uint16 (blk->ext, addr, val);
    }
    else {
      blk->data[addr] = (val >> 8) & 0xff;
      blk->data[addr + 1] = val & 0xff;
    }
  }
}

void mem_set_uint16_le (memory_t *mem, unsigned long addr, unsigned short val)
{
  mem_blk_t *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if ((addr + 1) > blk->addr2) {
      mem_set_uint8 (mem, addr, val & 0xff);
      mem_set_uint8 (mem, addr + 1, (val >> 8) & 0xff);
      return;
    }

    if (blk->flags & MEM_FLAG_RO) {
      return;
    }

    addr -= blk->addr1;

    if (blk->set_uint16 != NULL) {
      blk->set_uint16 (blk->ext, addr, val);
    }
    else {
      blk->data[addr] = val & 0xff;
      blk->data[addr + 1] = (val >> 8) & 0xff;
    }
  }
}

void mem_set_uint32_be (memory_t *mem, unsigned long addr, unsigned long val)
{
  mem_blk_t *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if ((addr + 3) > blk->addr2) {
      mem_set_uint8 (mem, addr, (val >> 24) & 0xff);
      mem_set_uint8 (mem, addr + 1, (val >> 16) & 0xff);
      mem_set_uint8 (mem, addr + 2, (val >> 8) & 0xff);
      mem_set_uint8 (mem, addr + 3, val & 0xff);
      return;
    }

    if (blk->flags & MEM_FLAG_RO) {
      return;
    }

    addr -= blk->addr1;

    if (blk->set_uint32 != NULL) {
      blk->set_uint32 (blk->ext, addr, val);
    }
    else {
      blk->data[addr] = (val >> 24) & 0xff;
      blk->data[addr + 1] = (val >> 16) & 0xff;
      blk->data[addr + 2] = (val >> 8) & 0xff;
      blk->data[addr + 3] = val & 0xff;
    }
  }
}

void mem_set_uint32_le (memory_t *mem, unsigned long addr, unsigned long val)
{
  mem_blk_t *blk;

  blk = mem_get_blk (mem, addr);

  if (blk != NULL) {
    if ((addr + 3) > blk->addr2) {
      mem_set_uint8 (mem, addr, val & 0xff);
      mem_set_uint8 (mem, addr + 1, (val >> 8) & 0xff);
      mem_set_uint8 (mem, addr + 2, (val >> 16) & 0xff);
      mem_set_uint8 (mem, addr + 3, (val >> 24) & 0xff);
      return;
    }

    if (blk->flags & MEM_FLAG_RO) {
      return;
    }

    addr -= blk->addr1;

    if (blk->set_uint32 != NULL) {
      blk->set_uint32 (blk->ext, addr, val);
    }
    else {
      blk->data[addr] = val & 0xff;
      blk->data[addr + 1] = (val >> 8) & 0xff;
      blk->data[addr + 2] = (val >> 16) & 0xff;
      blk->data[addr + 3] = (val >> 24) & 0xff;
    }
  }
}

void mem_init (memory_t *mem)
{
  mem->cnt = 0;
  mem->lst = NULL;

  mem->last = NULL;

  mem->def_val8 = 0xaa;
  mem->def_val16 = 0xaaaaU;
  mem->def_val32 = 0xaaaaaaaaUL;
}

memory_t *mem_new (void)
{
  memory_t *mem;

  mem = (memory_t *) malloc (sizeof (memory_t));
  if (mem == NULL) {
    return (NULL);
  }

  mem_init (mem);

  return (mem);
}

void mem_free (memory_t *mem)
{
  unsigned i;

  if (mem != NULL) {
    for (i = 0; i < mem->cnt; i++) {
      if (mem->lst[i].del) {
        mem_blk_del (mem->lst[i].blk);
      }
    }

    free (mem->lst);
  }
}

void mem_del (memory_t *mem)
{
  if (mem != NULL) {
    mem_free (mem);
    free (mem);
  }
}

void mem_set_default (memory_t *mem, unsigned char val)
{
  mem->def_val8 = val;
  mem->def_val16 = ((unsigned) val << 8) | val;
  mem->def_val32 = ((unsigned long) val << 8) | val;
  mem->def_val32 = (mem->def_val32 << 16) | mem->def_val32;
}

void mem_add_blk (memory_t *mem, mem_blk_t *blk, int del)
{
  mem_lst_t *lst;

  if (blk == NULL) {
    return;
  }

  lst = (mem_lst_t *) realloc (mem->lst, (mem->cnt + 1) * sizeof (mem_lst_t));
  if (lst == NULL) {
    return;
  }

  mem->lst = lst;

  lst += mem->cnt;
  mem->cnt += 1;

  lst->blk = blk;
  lst->addr1 = blk->addr1;
  lst->addr2 = blk->addr1 + blk->size - 1;
  lst->del = (del != 0);
}
