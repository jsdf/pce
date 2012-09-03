/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pbit/pbit-io.h                                   *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_PBIT_IO_H
#define PCE_PBIT_IO_H 1


#include <drivers/pbit/pbit.h>


#define PBIT_FORMAT_NONE 0
#define PBIT_FORMAT_PBIT 1
#define PBIT_FORMAT_TC   2


unsigned pbit_get_uint16_be (const void *buf, unsigned idx);
unsigned pbit_get_uint16_le (const void *buf, unsigned idx);

unsigned long pbit_get_uint32_be (const void *buf, unsigned idx);
unsigned long pbit_get_uint32_le (const void *buf, unsigned idx);

void pbit_set_uint16_be (void *buf, unsigned idx, unsigned val);
void pbit_set_uint16_le (void *buf, unsigned idx, unsigned val);

void pbit_set_uint32_be (void *buf, unsigned idx, unsigned long val);
void pbit_set_uint32_le (void *buf, unsigned idx, unsigned long val);


int pbit_read (FILE *fp, void *buf, unsigned long cnt);
int pbit_read_ofs (FILE *fp, unsigned long ofs, void *buf, unsigned long cnt);
int pbit_write (FILE *fp, const void *buf, unsigned long cnt);
int pbit_skip (FILE *fp, unsigned long cnt);


pbit_img_t *pbit_img_load_fp (FILE *fp, unsigned type);
pbit_img_t *pbit_img_load (const char *fname, unsigned type);

int pbit_img_save_fp (FILE *fp, const pbit_img_t *img, unsigned type);
int pbit_img_save (const char *fname, const pbit_img_t *img, unsigned type);


#endif
