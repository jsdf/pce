/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/track.h                                      *
 * Created:     2012-01-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2017 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PFI_TRACK_H
#define PFI_TRACK_H 1


typedef struct {
	unsigned long clock;

	unsigned long size;
	unsigned long max;
	unsigned char *data;

	unsigned      idx_cnt;
	unsigned long *idx;

	unsigned long cur_pos;
	unsigned long cur_clk;
	unsigned      cur_idx;
} pfi_trk_t;


pfi_trk_t *pfi_trk_new (void);
void pfi_trk_del (pfi_trk_t *trk);
pfi_trk_t *pfi_trk_clone (const pfi_trk_t *trk);
void pfi_trk_reset (pfi_trk_t *trk);

void pfi_trk_set_clock (pfi_trk_t *trk, unsigned long clock);
unsigned long pfi_trk_get_clock (const pfi_trk_t *trk);

int pfi_trk_set_size (pfi_trk_t *trk, unsigned long cnt);

int pfi_trk_add_data (pfi_trk_t *trk, const void *buf, unsigned long cnt);
int pfi_trk_add_index (pfi_trk_t *trk, unsigned long clk);
int pfi_trk_add_pulse (pfi_trk_t *trk, unsigned long cnt);

void pfi_trk_rewind (pfi_trk_t *trk);
int pfi_trk_get_pulse (pfi_trk_t *trk, unsigned long *val, unsigned long *idx);

pfi_trk_t *pfi_trk_scale (pfi_trk_t *trk, unsigned long mul, unsigned long div);
void pfi_trk_shift_index (pfi_trk_t *trk, long ofs);


#endif
