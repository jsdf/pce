/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/decode-bits.h                                *
 * Created:     2013-12-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013-2017 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFI_DECODE_BITS_H
#define PFI_DECODE_BITS_H 1


#include <drivers/pfi/track.h>


typedef struct {
	unsigned long cnt;
	unsigned long max;

	unsigned long index;

	unsigned char *buf;
	unsigned char *weak;
	unsigned long *clk;
} pfi_dec_t;


int pfi_dec_init (pfi_dec_t *buf, unsigned long max);
void pfi_dec_free (pfi_dec_t *buf);
int pfi_dec_alloc_bits (pfi_dec_t *buf, unsigned long cnt);
int pfi_dec_add_bit (pfi_dec_t *buf, int val, int weak, unsigned long clk);

void pfi_dec_clock_average (pfi_dec_t *bit);
void pfi_dec_clock_median (pfi_dec_t *bit);

int pfi_trk_decode_bits (pfi_trk_t *trk, pfi_dec_t *dst, unsigned long rate, unsigned rev);


#endif
