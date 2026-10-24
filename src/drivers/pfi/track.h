/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pfi/track.h                                      *
 * Created:     2012-01-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2021 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdint.h>


typedef struct {
	unsigned long clock;

	unsigned long pulse_cnt;
	unsigned long pulse_max;
	uint32_t      *pulse;

	unsigned      index_cnt;
	unsigned      index_max;
	uint32_t      *index;

	unsigned long cur_pos;
	unsigned long cur_clk;
	unsigned      cur_idx;
} pfi_trk_t;


void pfi_trk_init (pfi_trk_t *trk);
void pfi_trk_free (pfi_trk_t *trk);

pfi_trk_t *pfi_trk_new (void);
void pfi_trk_del (pfi_trk_t *trk);

void pfi_trk_clean (pfi_trk_t *trk);

pfi_trk_t *pfi_trk_clone (const pfi_trk_t *trk, int data, int index);
void pfi_trk_reset (pfi_trk_t *trk);

void pfi_trk_set_clock (pfi_trk_t *trk, unsigned long clock);
unsigned long pfi_trk_get_clock (const pfi_trk_t *trk);

int pfi_trk_get_pos (const pfi_trk_t *trk, uint32_t clk, unsigned long *pos, uint32_t *ofs);
int pfi_trk_get_index (const pfi_trk_t *trk, unsigned idx, unsigned long *pos, uint32_t *ofs);
unsigned long pfi_trk_get_clk (const pfi_trk_t *trk, unsigned long pos);

int pfi_trk_add_index (pfi_trk_t *trk, uint32_t clk);
int pfi_trk_add_pulses (pfi_trk_t *trk, const uint32_t *buf, unsigned long cnt);
int pfi_trk_add_pulse (pfi_trk_t *trk, uint32_t val);
int pfi_trk_inc_pulse (pfi_trk_t *trk, uint32_t val);

void pfi_trk_rewind (pfi_trk_t *trk);
int pfi_trk_get_pulse (pfi_trk_t *trk, uint32_t *val, uint32_t *idx);

pfi_trk_t *pfi_trk_scale (pfi_trk_t *trk, unsigned long mul, unsigned long div);
void pfi_trk_shift_index (pfi_trk_t *trk, long ofs);


#endif
