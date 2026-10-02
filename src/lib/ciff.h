/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/lib/ciff.h                                               *
 * Created:     2022-02-12 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2022 Hampa Hug <hampa@hampa.ch>                          *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef PCE_LIB_CIFF_H
#define PCE_LIB_CIFF_H 1


#include <stdint.h>
#include <stdio.h>


typedef struct {
	FILE     *fp;

	uint32_t crc;

	uint32_t ckid;
	uint32_t cksize;

	uint32_t size;

	char     in_chunk;
	char     use_crc;
	char     crc_error;
} ciff_t;


void ciff_init (ciff_t *ciff, FILE *fp, int use_crc);
void ciff_free (ciff_t *ciff);

int ciff_read (ciff_t *ciff, void *buf, uint32_t cnt);
int ciff_read_id (ciff_t *ciff);
int ciff_read_crc (ciff_t *ciff);
int ciff_read_chunk (ciff_t *ciff, void *buf, uint32_t size);

int ciff_write (ciff_t *ciff, const void *buf, uint32_t cnt);
int ciff_write_id (ciff_t *ciff, uint32_t ckid, uint32_t size);
int ciff_write_crc (ciff_t *ciff);
int ciff_write_chunk (ciff_t *ciff, uint32_t ckid, const void *buf, uint32_t size);



#endif
