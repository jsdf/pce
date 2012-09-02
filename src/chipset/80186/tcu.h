/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/80186/tcu.h                                      *
 * Created:     2012-06-30 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_CHIPSET_80186_TCU_H
#define PCE_CHIPSET_80186_TCU_H 1


typedef struct {
	unsigned short control;
	unsigned short count;
	unsigned short max_count_a;
	unsigned short max_count_b;
	unsigned char  current_input;
	unsigned char  last_input;
	unsigned char  mc;

	unsigned char  intr_val;
	void           *intr_ext;
	void           (*intr) (void *ext, unsigned char val);

	unsigned char  out_val;
	void           *out_ext;
	void           (*out) (void *ext, unsigned char val);
} e80186_tcu_cntr_t;

typedef struct {
	e80186_tcu_cntr_t cntr[3];
	unsigned          clock;
} e80186_tcu_t;


void e80186_tcu_init (e80186_tcu_t *tcu);
void e80186_tcu_free (e80186_tcu_t *tcu);

void e80186_tcu_set_int_fct (e80186_tcu_t *tcu, unsigned idx, void *ext, void *fct);
void e80186_tcu_set_out_fct (e80186_tcu_t *tcu, unsigned idx, void *ext, void *fct);

unsigned short e80186_tcu_get_control (const e80186_tcu_t *tcu, unsigned idx);
void e80186_tcu_set_control (e80186_tcu_t *tcu, unsigned idx, unsigned short val);

unsigned short e80186_tcu_get_count (const e80186_tcu_t *tcu, unsigned idx);
void e80186_tcu_set_count (e80186_tcu_t *tcu, unsigned idx, unsigned short val);

unsigned short e80186_tcu_get_max_count_a (const e80186_tcu_t *tcu, unsigned idx);
void e80186_tcu_set_max_count_a (e80186_tcu_t *tcu, unsigned idx, unsigned short val);

unsigned short e80186_tcu_get_max_count_b (const e80186_tcu_t *tcu, unsigned idx);
void e80186_tcu_set_max_count_b (e80186_tcu_t *tcu, unsigned idx, unsigned short val);

void e80186_tcu_set_input (e80186_tcu_t *tcu, unsigned idx, unsigned char val);

void e80186_tcu_reset (e80186_tcu_t *tcu);

void e80186_tcu_clock (e80186_tcu_t *tcu, unsigned cnt);


#endif
