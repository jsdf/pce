/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/memory.h                                         *
 * Created:     2000-04-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2000-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MEMORY_H
#define PCE_MEMORY_H 1


#include <stdio.h>


#define MEM_LAST_CNT 4


typedef unsigned char (*mem_get_uint8_f) (void *blk, unsigned long addr);
typedef unsigned short (*mem_get_uint16_f) (void *blk, unsigned long addr);
typedef unsigned long (*mem_get_uint32_f) (void *blk, unsigned long addr);

typedef void (*mem_set_uint8_f) (void *blk, unsigned long addr, unsigned char val);
typedef void (*mem_set_uint16_f) (void *blk, unsigned long addr, unsigned short val);
typedef void (*mem_set_uint32_f) (void *blk, unsigned long addr, unsigned long val);


/*!***************************************************************************
 * @short The memory block structure
 *****************************************************************************/
typedef struct {
	/* These access functions are used if data is NULL. */
	mem_get_uint8_f  get_uint8;
	mem_get_uint16_f get_uint16;
	mem_get_uint32_f get_uint32;

	mem_set_uint8_f  set_uint8;
	mem_set_uint16_f set_uint16;
	mem_set_uint32_f set_uint32;

	/* The transparant parameter for get_*() and set_*(). */
	void             *ext;

	unsigned char    active;
	unsigned char    readonly;

	/* Delete data when the memory block is deleted */
	unsigned char    data_del;

	/* Memory block base address */
	unsigned long    addr1;

	/* The last address inside the memory block. addr2 == addr1 + size - 1 */
	unsigned long    addr2;

	/* The memory block size */
	unsigned long    size;

	/* The actual memory or NULL if get_* and set_* are used */
	unsigned char    *data;
} mem_blk_t;


typedef struct {
	mem_blk_t     *blk;
	int           del;
} mem_lst_t;


typedef struct {
	unsigned         cnt;
	mem_lst_t        *lst;

	mem_lst_t        *last[MEM_LAST_CNT];

	/* these functions are used if no block is found */
	void             *ext;
	mem_get_uint8_f  get_uint8;
	mem_get_uint16_f get_uint16;
	mem_get_uint32_f get_uint32;
	mem_set_uint8_f  set_uint8;
	mem_set_uint16_f set_uint16;
	mem_set_uint32_f set_uint32;

	unsigned long    defval;
} memory_t;



/*!***************************************************************************
 * @short  Initialize a static memory block structure
 * @param  base  The linear base address
 * @param  size  The block size in bytes
 * @param  alloc If true then backing store is allocated
 * @return Zero if successful, nonzero otherwise
 *****************************************************************************/
int mem_blk_init (mem_blk_t *blk, unsigned long base, unsigned long size, int alloc);

/*!***************************************************************************
 * @short  Create a new memory block
 * @param  base  The linear base address
 * @param  size  The block size in bytes
 * @param  alloc If true then backing store is allocated
 * @return The memory block or NULL on error
 *****************************************************************************/
mem_blk_t *mem_blk_new (unsigned long base, unsigned long size, int alloc);

/*!***************************************************************************
 * @short Free the resources used by a static memory block structure
 * @param blk The memory block
 *****************************************************************************/
void mem_blk_free (mem_blk_t *blk);

/*!***************************************************************************
 * @short Delete a memory block
 * @param blk The memory block
 *
 * Backing store is freed if it was allocated in mem_blk_new().
 *****************************************************************************/
void mem_blk_del (mem_blk_t *blk);

mem_blk_t *mem_blk_clone (const mem_blk_t *blk);

void mem_blk_fix_fct (mem_blk_t *blk);

void mem_blk_set_fget (mem_blk_t *blk, void *ext, void *g8, void *g16, void *g32);
void mem_blk_set_fset (mem_blk_t *blk, void *ext, void *s8, void *s16, void *s32);
void mem_blk_set_fct (mem_blk_t *blk, void *ext,
	void *g8, void *g16, void *g32, void *s8, void *s16, void *s32
);
void mem_blk_set_ext (mem_blk_t *blk, void *ext);

/*!***************************************************************************
 * @short Clear a memory block
 * @param blk The memory block
 * @param val The byte value with which the block is initialized
 *****************************************************************************/
void mem_blk_clear (mem_blk_t *blk, unsigned char val);

unsigned char *mem_blk_get_data (mem_blk_t *blk);

void mem_blk_set_data (mem_blk_t *blk, void *data, int del);

int mem_blk_get_active (mem_blk_t *blk);

void mem_blk_set_active (mem_blk_t *blk, int val);

/*!***************************************************************************
 * @short  Get the read-only flag
 * @param  blk The memory block
 * @return True if the block is read-only
 *****************************************************************************/
int mem_blk_get_readonly (mem_blk_t *blk);

/*!***************************************************************************
 * @short Set the read-only flag
 * @param blk The memory block
 * @param ro  Make block-read only if true, read-write otherwise
 *****************************************************************************/
void mem_blk_set_readonly (mem_blk_t *blk, int val);

/*!***************************************************************************
 * @short  Get the base address
 * @param  blk The memory block
 * @return The memory block's base address
 *****************************************************************************/
unsigned long mem_blk_get_addr (const mem_blk_t *blk);

/*!***************************************************************************
 * @short Set the base address
 * @param blk The memory block
 * @param addr The memory block's new base address
 *****************************************************************************/
void mem_blk_set_addr (mem_blk_t *blk, unsigned long addr);

/*!***************************************************************************
 * @short  Get the memory block size in bytes
 * @param  blk The memory block
 * @return The memory block size in bytes
 *****************************************************************************/
unsigned long mem_blk_get_size (const mem_blk_t *blk);

void mem_blk_set_size (mem_blk_t *blk, unsigned long size);


void buf_set_uint8 (void *buf, unsigned long addr, unsigned char val);
void buf_set_uint16_be (void *buf, unsigned long addr, unsigned short val);
void buf_set_uint16_le (void *buf, unsigned long addr, unsigned short val);
void buf_set_uint32_be (void *buf, unsigned long addr, unsigned long val);
void buf_set_uint32_le (void *buf, unsigned long addr, unsigned long val);

unsigned char buf_get_uint8 (const void *buf, unsigned long addr);
unsigned short buf_get_uint16_be (const void *buf, unsigned long addr);
unsigned short buf_get_uint16_le (const void *buf, unsigned long addr);
unsigned long buf_get_uint32_be (const void *buf, unsigned long addr);
unsigned long buf_get_uint32_le (const void *buf, unsigned long addr);


void mem_blk_set_uint8 (mem_blk_t *blk, unsigned long addr, unsigned char val);
void mem_blk_set_uint8_null (void *ext, unsigned long addr, unsigned char val);
void mem_blk_set_uint16_be (mem_blk_t *blk, unsigned long addr, unsigned short val);
void mem_blk_set_uint16_le (mem_blk_t *blk, unsigned long addr, unsigned short val);
void mem_blk_set_uint16_null (void *ext, unsigned long addr, unsigned short val);
void mem_blk_set_uint32_be (mem_blk_t *blk, unsigned long addr, unsigned long val);
void mem_blk_set_uint32_le (mem_blk_t *blk, unsigned long addr, unsigned long val);
void mem_blk_set_uint32_null (void *ext, unsigned long addr, unsigned long val);

unsigned char mem_blk_get_uint8 (const mem_blk_t *blk, unsigned long addr);
unsigned char mem_blk_get_uint8_null (const void *ext, unsigned long addr);
unsigned short mem_blk_get_uint16_be (const mem_blk_t *blk, unsigned long addr);
unsigned short mem_blk_get_uint16_le (const mem_blk_t *blk, unsigned long addr);
unsigned short mem_blk_get_uint16_null (const void *ext, unsigned long addr);
unsigned long mem_blk_get_uint32_be (const mem_blk_t *blk, unsigned long addr);
unsigned long mem_blk_get_uint32_le (const mem_blk_t *blk, unsigned long addr);
unsigned long mem_blk_get_uint32_null (const void *ext, unsigned long addr);


/*!***************************************************************************
 * @short Initialize a static memory structure
 * @param mem The memory structure
 *****************************************************************************/
void mem_init (memory_t *mem);

/*!***************************************************************************
 * @short  Create a new memory structure
 * @return The new memory structure or NULL on error
 *****************************************************************************/
memory_t *mem_new (void);

/*!***************************************************************************
 * @short Free the resources used by a static memory structure
 *****************************************************************************/
void mem_free (memory_t *mem);

/*!***************************************************************************
 * @short Delete a memory structure
 *****************************************************************************/
void mem_del (memory_t *mem);

/*!***************************************************************************
 * @short Set the default access functions
 *****************************************************************************/
void mem_set_fct (memory_t *mem, void *ext,
	void *g8, void *g16, void *g32, void *s8, void *s16, void *s32
);

/*!***************************************************************************
 * @short Set the default value
 * @param mem The memory structure
 * @param val The default value
 *****************************************************************************/
void mem_set_default (memory_t *mem, unsigned char val);

/*!***************************************************************************
 * @short Print memory state information
 *****************************************************************************/
void mem_prt_state (memory_t *mem, FILE *fp);

/*!***************************************************************************
 * @short Add a memory block to a memory structure
 * @param mem The memory structure
 * @param blk The memory block
 * @param del If true then the block will be deleted when the memory
 *            structure is deleted.
 *****************************************************************************/
void mem_add_blk (memory_t *mem, mem_blk_t *blk, int del);

/*!***************************************************************************
 * @short Remove a memory block without deleting it
 * @param mem The memory structure
 * @param blk The memory block
 *****************************************************************************/
void mem_rmv_blk (memory_t *mem, const mem_blk_t *blk);

/*!***************************************************************************
 * @short Remove all memory blocks and delete marked blocks
 * @param mem The memory structure
 *****************************************************************************/
void mem_rmv_all (memory_t *mem);

/*!***************************************************************************
 * @short Move a memory block to the front of the list
 * @param mem   The memory structure
 * @param addr  The memory block containing this address is moved
 *****************************************************************************/
void mem_move_to_front (memory_t *mem, unsigned long addr);

/*!***************************************************************************
 * @short Get a memory block containing an address
 * @param  mem The memory structure
 * @return blk The memory block or NULL
 *****************************************************************************/
mem_blk_t *mem_get_blk (memory_t *mem, unsigned long addr);

/*!***************************************************************************
 * @short Get a pointer to memory
 * @param  mem  The memory structure
 * @param  addr The pointer address
 * @param  size The requested block size
 * @return A pointer to a block of memory at address addr and of size size
 *****************************************************************************/
void *mem_get_ptr (memory_t *mem, unsigned long addr, unsigned long size);

unsigned char mem_get_uint8 (memory_t *mem, unsigned long addr);
unsigned short mem_get_uint16_be (memory_t *mem, unsigned long addr);
unsigned short mem_get_uint16_le (memory_t *mem, unsigned long addr);
unsigned long mem_get_uint32_be (memory_t *mem, unsigned long addr);
unsigned long mem_get_uint32_le (memory_t *mem, unsigned long addr);

void mem_set_uint8_rw (memory_t *mem, unsigned long addr, unsigned char val);
void mem_set_uint8 (memory_t *mem, unsigned long addr, unsigned char val);
void mem_set_uint16_be (memory_t *mem, unsigned long addr, unsigned short val);
void mem_set_uint16_le (memory_t *mem, unsigned long addr, unsigned short val);
void mem_set_uint32_be (memory_t *mem, unsigned long addr, unsigned long val);
void mem_set_uint32_le (memory_t *mem, unsigned long addr, unsigned long val);


#endif
