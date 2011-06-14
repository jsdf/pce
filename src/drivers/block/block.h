/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/block/block.h                                    *
 * Created:     2003-04-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_BLOCK_BLOCK_H
#define PCE_DEVICES_BLOCK_BLOCK_H 1


#include <config.h>

#include <stdio.h>
#include <stdint.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#define PCE_DISK_NONE   0
#define PCE_DISK_RAW    1
#define PCE_DISK_RAM    2
#define PCE_DISK_PCE    3
#define PCE_DISK_DOSEMU 4
#define PCE_DISK_COW    5
#define PCE_DISK_FDC    6
#define PCE_DISK_QED    7


struct disk_s;


typedef int (*dsk_read_f) (struct disk_s *dsk, void *buf, uint32_t i, uint32_t n);

typedef int (*dsk_write_f) (struct disk_s *dsk, const void *buf, uint32_t i, uint32_t n);

typedef int (*dsk_get_msg_f) (struct disk_s *dsk, const char *msg, char *val, unsigned max);
typedef int (*dsk_set_msg_f) (struct disk_s *dsk, const char *msg, const char *val);


/*!***************************************************************************
 * @short The disk structure
 *****************************************************************************/
typedef struct disk_s {
	unsigned      type;

	void          (*del) (struct disk_s *dsk);
	dsk_read_f    read;
	dsk_write_f   write;
	dsk_get_msg_f get_msg;
	dsk_set_msg_f set_msg;

	unsigned      drive;

	uint32_t      blocks;

	uint32_t      c;
	uint32_t      h;
	uint32_t      s;

	uint32_t      visible_c;
	uint32_t      visible_h;
	uint32_t      visible_s;

	char          readonly;

	char          *fname;

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

uint16_t dsk_get_uint16_le (const void *buf, unsigned i);
uint32_t dsk_get_uint32_le (const void *buf, unsigned i);
uint64_t dsk_get_uint64_le (const void *buf, unsigned i);

void dsk_set_uint16_le (void *buf, unsigned i, uint16_t v);
void dsk_set_uint32_le (void *buf, unsigned i, uint32_t v);
void dsk_set_uint64_le (void *buf, unsigned i, uint64_t v);


int dsk_set_pos (FILE *fp, uint64_t ofs);
int dsk_read (FILE *fp, void *buf, uint64_t ofs, uint64_t cnt);
int dsk_write (FILE *fp, const void *buf, uint64_t ofs, uint64_t cnt);
int dsk_get_filesize (FILE *fp, uint64_t *cnt);
int dsk_set_filesize (FILE *fp, uint64_t cnt);

int dsk_adjust_chs (uint32_t *n, uint32_t *c, uint32_t *h, uint32_t *s);

/*!***************************************************************************
 * @short Initialize a disk structure
 *****************************************************************************/
void dsk_init (disk_t *dsk, void *ext, uint32_t n, uint32_t c, uint32_t h, uint32_t s);

/*!***************************************************************************
 * @short Delete a disk
 *****************************************************************************/
void dsk_del (disk_t *dsk);


/*!***************************************************************************
 * @short Set the drive number
 *****************************************************************************/
void dsk_set_drive (disk_t *dsk, unsigned d);

/*!***************************************************************************
 * @short Get the disk type
 *****************************************************************************/
unsigned dsk_get_type (const disk_t *dsk);

/*!***************************************************************************
 * @short Set the disk type
 *****************************************************************************/
void dsk_set_type (disk_t *dsk, unsigned type);

/*!***************************************************************************
 * @short Get the read-only flag
 *****************************************************************************/
int dsk_get_readonly (disk_t *dsk);

/*!***************************************************************************
 * @short Set the read-only flag
 *****************************************************************************/
void dsk_set_readonly (disk_t *dsk, int v);

/*!***************************************************************************
 * @short Set the disk file name
 *****************************************************************************/
void dsk_set_fname (disk_t *dsk, const char *fname);

/*!***************************************************************************
 * @short Get the disk file name
 *****************************************************************************/
const char *dsk_get_fname (const disk_t *dsk);

/*!***************************************************************************
 * @short Set the disk geometry
 *****************************************************************************/
int dsk_set_geometry (disk_t *dsk, uint32_t n, uint32_t c, uint32_t h, uint32_t s);

/*!***************************************************************************
 * @short Set the visible geometry
 *****************************************************************************/
void dsk_set_visible_chs (disk_t *dsk, uint32_t c, uint32_t h, uint32_t s);

/*!***************************************************************************
 * @short Get the drive number
 *****************************************************************************/
unsigned dsk_get_drive (const disk_t *dsk);

/*!***************************************************************************
 * @short Get the number of blocks in a disk
 *****************************************************************************/
uint32_t dsk_get_block_cnt (const disk_t *dsk);

/*!***************************************************************************
 * @short Guess and set the disk geometry
 *****************************************************************************/
int dsk_guess_geometry (disk_t *dsk);


/*!***************************************************************************
 * @short  Open a disk image
 * @param  fname The disk image file name
 * @param  ofs   For raw images, the image data start offset. Not used for
 *               other image formats.
 * @param  ro    Open read-only if true
 * @return A new disk image structure
 *****************************************************************************/
disk_t *dsk_auto_open (const char *fname, uint64_t ofs, int ro);


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

int dsk_commit (disk_t *dsk);

/*!***************************************************************************
 * @short  Get a message from a disk
 * @return Zero if successful
 *****************************************************************************/
int dsk_get_msg (disk_t *dsk, const char *msg, char *val, unsigned max);

/*!***************************************************************************
 * @short  Send a message to a disk
 * @return Zero if successful
 *****************************************************************************/
int dsk_set_msg (disk_t *dsk, const char *msg, const char *val);


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

/*!***************************************************************************
 * @short  Get a message from a disk
 * @param  drv The drive number
 * @param  msg The message
 * @param  val The return value
 * @param  max The size of val
 * @return Zero if successful
 *****************************************************************************/
int dsks_get_msg (disks_t *dsks, unsigned drv, const char *msg, char *val, unsigned max);

/*!***************************************************************************
 * @short  Send a message to a disk
 * @param  drv The drive number
 * @param  msg The message
 * @param  val The message parameters or NULL
 * @return Zero if successful
 *****************************************************************************/
int dsks_set_msg (disks_t *dsks, unsigned drv, const char *msg, const char *val);


#endif
