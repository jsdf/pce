/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/memory.h                                               *
 * Created:       2000-04-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-22 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: memory.h,v 1.4 2003/04/22 17:58:18 hampa Exp $ */


#ifndef PCE_MEMORY_H
#define PCE_MEMORY_H 1

#include <pce.h>


#define MEM_FLAG_RO  1


typedef struct {
  unsigned char (*get_uint8) (void *blk, unsigned long addr);
  unsigned short (*get_uint16) (void *blk, unsigned long addr);

  void (*set_uint8) (void *blk, unsigned long addr, unsigned char val);
  void (*set_uint16) (void *blk, unsigned long addr, unsigned short val);

  void           *ext;

  unsigned short flags;
  unsigned long  base;
  unsigned long  size;
  unsigned long  end;
  unsigned char  *data;
} mem_blk_t;


typedef struct {
  mem_blk_t *blk;
  int       del;
} mem_lst_t;

typedef struct {
  unsigned      cnt;
  mem_lst_t     *lst;

  unsigned char def_val;
} memory_t;



mem_blk_t *mem_blk_new (unsigned long base, unsigned long size, int alloc);
void mem_blk_del (mem_blk_t *blk);
void mem_blk_init (mem_blk_t *blk, unsigned char val);
void mem_blk_set_ro (mem_blk_t *blk, int ro);

unsigned char mem_get_uint8 (memory_t *mem, unsigned long addr);
unsigned short mem_get_uint16_le (memory_t *mem, unsigned long addr);
void mem_set_uint8 (memory_t *mem, unsigned long addr, unsigned char val);
void mem_set_uint16_le (memory_t *mem, unsigned long addr, unsigned short val);

memory_t *mem_new (void);
void mem_del (memory_t *mem);
void mem_add_blk (memory_t *mem, mem_blk_t *blk, int del);


#endif
