/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     memory.h                                                   *
 * Created:       2000-04-23 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-11-11 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: memory.h,v 1.4 2003/11/11 16:33:57 hampa Exp $ */


#ifndef PCE_MEMORY_H
#define PCE_MEMORY_H 1


#define MEM_FLAG_RO  1


typedef unsigned char (*mem_get_uint8_f) (void *blk, unsigned long addr);
typedef unsigned short (*mem_get_uint16_f) (void *blk, unsigned long addr);
typedef unsigned long (*mem_get_uint32_f) (void *blk, unsigned long addr);

typedef void (*mem_set_uint8_f) (void *blk, unsigned long addr, unsigned char val);
typedef void (*mem_set_uint16_f) (void *blk, unsigned long addr, unsigned short val);
typedef void (*mem_set_uint32_f) (void *blk, unsigned long addr, unsigned long val);


typedef struct {
  mem_get_uint8_f  get_uint8;
  mem_get_uint16_f get_uint16;
  mem_get_uint32_f get_uint32;

  mem_set_uint8_f  set_uint8;
  mem_set_uint16_f set_uint16;
  mem_set_uint32_f set_uint32;

  void             *ext;

  unsigned short   flags;
  unsigned long    base;
  unsigned long    size;
  unsigned long    end;
  unsigned char    *data;
} mem_blk_t;


typedef struct {
  mem_blk_t *blk;
  int       del;
} mem_lst_t;

typedef struct {
  unsigned      cnt;
  mem_lst_t     *lst;

  unsigned char  def_val8;
  unsigned short def_val16;
  unsigned long  def_val32;
} memory_t;



/*!***************************************************************************
 * @short  Create a new memory block
 * @param  base  The linear base address
 * @param  size  The block size in bytes
 * @param  alloc If true then backing store is allocated
 * @return The memory block or NULL on error
 *****************************************************************************/
mem_blk_t *mem_blk_new (unsigned long base, unsigned long size, int alloc);

/*!***************************************************************************
 * @short Delete a memory block
 * @param blk The memory block
 *
 * Backing store is freed if it was allocated in mem_blk_new().
 *****************************************************************************/
void mem_blk_del (mem_blk_t *blk);

/*!***************************************************************************
 * @short Initialize a memory block
 * @param blk The memory block
 * @param val The byte value with which the block is initialized
 *****************************************************************************/
void mem_blk_init (mem_blk_t *blk, unsigned char val);

void mem_blk_set_ext (mem_blk_t *blk, void *ext);
void mem_blk_set_ro (mem_blk_t *blk, int ro);
unsigned long mem_blk_get_size (mem_blk_t *blk);

unsigned char mem_get_uint8 (memory_t *mem, unsigned long addr);
unsigned short mem_get_uint16_be (memory_t *mem, unsigned long addr);
unsigned short mem_get_uint16_le (memory_t *mem, unsigned long addr);
unsigned long mem_get_uint32_be (memory_t *mem, unsigned long addr);

void mem_set_uint8 (memory_t *mem, unsigned long addr, unsigned char val);
void mem_set_uint16_be (memory_t *mem, unsigned long addr, unsigned short val);
void mem_set_uint16_le (memory_t *mem, unsigned long addr, unsigned short val);
void mem_set_uint32_be (memory_t *mem, unsigned long addr, unsigned long val);

memory_t *mem_new (void);
void mem_del (memory_t *mem);

/*!***************************************************************************
 * @short Set the default value
 * @param mem The memory structure
 * @param val The default byte value
 *****************************************************************************/
void mem_set_default (memory_t *mem, unsigned char val);

void mem_add_blk (memory_t *mem, mem_blk_t *blk, int del);


#endif
