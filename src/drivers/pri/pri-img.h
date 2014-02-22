/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri-io.h                                     *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_PRI_IMG_H
#define PCE_PRI_IMG_H 1


#include <drivers/pri/pri.h>


#define PRI_FORMAT_NONE 0
#define PRI_FORMAT_PBIT 1
#define PRI_FORMAT_PRI  2
#define PRI_FORMAT_TC   3


unsigned pri_get_uint16_be (const void *buf, unsigned idx);
unsigned pri_get_uint16_le (const void *buf, unsigned idx);

unsigned long pri_get_uint32_be (const void *buf, unsigned idx);
unsigned long pri_get_uint32_le (const void *buf, unsigned idx);

void pri_set_uint16_be (void *buf, unsigned idx, unsigned val);
void pri_set_uint16_le (void *buf, unsigned idx, unsigned val);

void pri_set_uint32_be (void *buf, unsigned idx, unsigned long val);
void pri_set_uint32_le (void *buf, unsigned idx, unsigned long val);


int pri_read (FILE *fp, void *buf, unsigned long cnt);
int pri_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt);
int pri_write (FILE *fp, const void *buf, unsigned long cnt);
int pri_write_ofs (FILE *fp, unsigned long ofs, const void *buf, unsigned long cnt);
int pri_skip (FILE *fp, unsigned long cnt);


pri_img_t *pri_img_load_fp (FILE *fp, unsigned type);
pri_img_t *pri_img_load (const char *fname, unsigned type);

int pri_img_save_fp (FILE *fp, const pri_img_t *img, unsigned type);
int pri_img_save (const char *fname, const pri_img_t *img, unsigned type);


#endif
