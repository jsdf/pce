/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/block/block.h                                  *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-03 by Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_DEVICES_BLOCK_BLOCK_H
#define PCE_DEVICES_BLOCK_BLOCK_H 1


#include <config.h>

#include <stdio.h>
#include <stdint.h>


struct disk_s;


typedef void (*dsk_del_f) (struct disk_s *dsk);

typedef int (*dsk_read_f) (struct disk_s *dsk, void *buf, uint32_t i, uint32_t n);

typedef int (*dsk_write_f) (struct disk_s *dsk, const void *buf, uint32_t i, uint32_t n);

typedef int (*dsk_commit_f) (struct disk_s *dsk);


/*!***************************************************************************
 * @short The disk structure
 *****************************************************************************/
typedef struct disk_s {
  dsk_del_f     del;
  dsk_read_f    read;
  dsk_write_f   write;
  dsk_commit_f  commit;

  unsigned      drive;
  uint32_t      c;
  uint32_t      h;
  uint32_t      s;
  uint32_t      blocks;

  uint32_t      visible_c;
  uint32_t      visible_h;
  uint32_t      visible_s;

  char          readonly;

  void          *ext;
} disk_t;


/*!***************************************************************************
 * @short The disk set structure
 *****************************************************************************/
typedef struct {
  unsigned cnt;
  disk_t   **dsk;
} disks_t;


uint16_t dsk_get_uint16_be (const void *buf, unsigned i);
uint32_t dsk_get_uint32_be (const void *buf, unsigned i);
uint64_t dsk_get_uint64_be (const void *buf, unsigned i);

void dsk_set_uint16_be (void *buf, unsigned i, uint16_t v);
void dsk_set_uint32_be (void *buf, unsigned i, uint32_t v);
void dsk_set_uint64_be (void *buf, unsigned i, uint64_t v);

uint32_t dsk_get_uint32_le (const void *buf, unsigned i);

void dsk_set_uint32_le (void *buf, unsigned i, uint32_t v);


int dsk_read (FILE *fp, void *buf, uint64_t ofs, uint64_t cnt);
int dsk_write (FILE *fp, const void *buf, uint64_t ofs, uint64_t cnt);


/*!***************************************************************************
 * @short Initialize a disk structure
 *****************************************************************************/
void dsk_init (disk_t *dsk, void *ext, uint32_t c, uint32_t h, uint32_t s);

/*!***************************************************************************
 * @short Delete a disk
 *****************************************************************************/
void dsk_del (disk_t *dsk);


/*!***************************************************************************
 * @short Set the drive number
 *****************************************************************************/
void dsk_set_drive (disk_t *dsk, unsigned d);

/*!***************************************************************************
 * @short Set the read-only flag
 *****************************************************************************/
void dsk_set_readonly (disk_t *dsk, int v);

/*!***************************************************************************
 * @short Set the visible geometry
 *****************************************************************************/
void dsk_set_visible_chs (disk_t *dsk, uint32_t c, uint32_t h, uint32_t s);

uint32_t dsk_get_block_cnt (disk_t *dsk);


/*!***************************************************************************
 * @short Create a new disk
 *****************************************************************************/
disk_t *dsk_auto_open (const char *fname, int ro);


/*!***************************************************************************
 * @short  Convert CHS to LBA
 * @return Nonzero if the CHS address is illegal
 *****************************************************************************/
int dsk_get_lba (disk_t *dsk, uint32_t c, uint32_t h, uint32_t s, uint32_t *v);

/*!***************************************************************************
 * @short  Read blocks using LBA addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_read_lba (disk_t *dsk, void *buf, uint32_t i, uint32_t n);

/*!***************************************************************************
 * @short  Read blocks using CHS addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_read_chs (disk_t *dsk, void *buf,
  uint32_t c, uint32_t h, uint32_t s, uint32_t blk_n
);

/*!***************************************************************************
 * @short  Write blocks using LBA addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_write_lba (disk_t *dsk, const void *buf, uint32_t i, uint32_t n);

/*!***************************************************************************
 * @short  Write blocks using LBA addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_write_chs (disk_t *dsk, const void *buf,
  uint32_t c, uint32_t h, uint32_t s, uint32_t n
);

/*!***************************************************************************
 * @short  Commit changes
 * @return Zero if successful
 *****************************************************************************/
int dsk_commit (disk_t *dsk);


/*!***************************************************************************
 * @short  Create a new disk set
 * @return The new disk set or NULL on error
 *****************************************************************************/
disks_t *dsks_new (void);

/*!***************************************************************************
 * @short Delete a disk set and all included disks
 *****************************************************************************/
void dsks_del (disks_t *dsks);

/*!***************************************************************************
 * @short  Add a disk to a disk set
 * @return Zero if successful
 *****************************************************************************/
int dsks_add_disk (disks_t *dsks, disk_t *dsk);

/*!***************************************************************************
 * @short  Remove a disk from a disk set
 * @return Zero if the disk was not in the set
 *****************************************************************************/
int dsks_rmv_disk (disks_t *dsks, disk_t *dsk);

/*!***************************************************************************
 * @short  Get a disk from a disk set
 * @param  drive The drive number
 * @return The disk or NULL on error
 *****************************************************************************/
disk_t *dsks_get_disk (disks_t *dsks, unsigned drive);

/*!***************************************************************************
 * @short  Commit all disks in a disk set
 * @return Zero if successful
 *****************************************************************************/
int dsks_commit (disks_t *dsks);


#endif
