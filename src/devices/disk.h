/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/disk.h                                         *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-01-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2004 by Hampa Hug <hampa@hampa.ch>                *
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


#ifndef PCE_DEVICES_DISK_H
#define PCE_DEVICES_DISK_H 1


struct disk_s;


typedef void (*dsk_del_f) (struct disk_s *dsk);

typedef int (*dsk_read_f) (struct disk_s *dsk, void *buf,
  unsigned long i, unsigned long n
);

typedef int (*dsk_write_f) (struct disk_s *dsk, const void *buf,
  unsigned long i, unsigned long n
);


/*!***************************************************************************
 * @short The disk structure
 *****************************************************************************/
typedef struct disk_s {
  dsk_del_f     del;
  dsk_read_f    read;
  dsk_write_f   write;

  unsigned      drive;
  unsigned      c;
  unsigned      h;
  unsigned      s;
  unsigned long blocks;

  int           readonly;

  void          *ext;
} disk_t;


/*!***************************************************************************
 * @short The ram disk structure
 *****************************************************************************/
typedef struct {
  disk_t        dsk;

  unsigned char *data;
} disk_ram_t;


/*!***************************************************************************
 * @short The image file disk structure
 *****************************************************************************/
typedef struct {
  disk_t        dsk;

  unsigned long start;
  FILE          *fp;
} disk_img_t;


/*!***************************************************************************
 * @short The disk set structure
 *****************************************************************************/
typedef struct {
  unsigned cnt;
  disk_t   **dsk;
} disks_t;


/*!***************************************************************************
 * @short Create a new ram disk
 *****************************************************************************/
disk_t *dsk_ram_new (unsigned d, unsigned c, unsigned h, unsigned s,
  const char *fname, int ro
);

/*!***************************************************************************
 * @short Create a new image file disk
 *****************************************************************************/
disk_t *dsk_img_new (unsigned d, unsigned c, unsigned h, unsigned s,
  unsigned long start, const char *fname, int ro
);

/*!***************************************************************************
 * @short Create a new dosemu disk
 *****************************************************************************/
disk_t *dsk_dosemu_new (unsigned d, const char *fname, int ro);

/*!***************************************************************************
 * @short Create a new disk
 *****************************************************************************/
disk_t *dsk_auto_new (unsigned d, const char *fname, int ro);

/*!***************************************************************************
 * @short Delete a disk
 *****************************************************************************/
void dsk_del (disk_t *dsk);


/*!***************************************************************************
 * @short  Convert CHS to LBA
 * @return Nonzero if the CHS address is illegal
 *****************************************************************************/
int dsk_get_lba (disk_t *dsk, unsigned c, unsigned h, unsigned s,
  unsigned long *lba
);

/*!***************************************************************************
 * @short  Read blocks using LBA addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_read_lba (disk_t *dsk, void *buf,
  unsigned long blk_i, unsigned long blk_n
);

/*!***************************************************************************
 * @short  Read blocks using CHS addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_read_chs (disk_t *dsk, void *buf,
  unsigned c, unsigned h, unsigned s, unsigned long blk_n
);

/*!***************************************************************************
 * @short  Write blocks using LBA addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_write_lba (disk_t *dsk, const void *buf,
  unsigned long blk_i, unsigned long blk_n
);

/*!***************************************************************************
 * @short  Write blocks using LBA addressing
 * @return Zero if successful
 *****************************************************************************/
int dsk_write_chs (disk_t *dsk, const void *buf,
  unsigned c, unsigned h, unsigned s, unsigned long blk_n
);


disks_t *dsks_new (void);
void dsks_del (disks_t *dsks);
int dsks_add_disk (disks_t *dsks, disk_t *dsk);
void dsks_rmv_disk (disks_t *dsks, disk_t *dsk);
disk_t *dsks_get_disk (disks_t *dsks, unsigned drive);


#endif
