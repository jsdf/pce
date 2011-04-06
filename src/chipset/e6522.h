/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6522.h                                          *
 * Created:     2007-11-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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


/* VIA 6522 */


#ifndef PCE_CHIPSET_E6522_H
#define PCE_CHIPSET_E6522_H 1


typedef struct {
	unsigned       addr_shift;

	unsigned char  ora;
	unsigned char  orb;

	unsigned char  ira;
	unsigned char  irb;

	unsigned char  ddra;
	unsigned char  ddrb;

	unsigned char  shift_val;
	unsigned char  shift_cnt;

	unsigned char  acr;
	unsigned char  pcr;

	unsigned char  ifr;
	unsigned char  ier;

	unsigned short t1_latch;
	unsigned short t1_val;
	int            t1_hot;

	unsigned short t2_latch;
	unsigned short t2_val;
	int            t2_hot;

	unsigned char  ca1_inp;
	unsigned char  ca2_inp;

	unsigned char  cb1_inp;
	unsigned char  cb2_inp;

	void           *set_ora_ext;
	void           (*set_ora) (void *ext, unsigned char val);
	unsigned char  set_ora_val;

	void           *set_orb_ext;
	void           (*set_orb) (void *ext, unsigned char val);
	unsigned char  set_orb_val;

	void           *set_cb2_ext;
	void           (*set_cb2) (void *ext, unsigned char val);
	unsigned char  set_cb2_val;

	void           *set_shift_out_ext;
	void           (*set_shift_out) (void *ext, unsigned char val);

	void           *irq_ext;
	void           (*irq) (void *ext, unsigned char val);
	unsigned char  irq_val;
} e6522_t;


void e6522_init (e6522_t *via, unsigned addr_shift);
void e6522_free (e6522_t *via);

void e6522_set_ora_fct (e6522_t *via, void *ext, void *fct);
void e6522_set_orb_fct (e6522_t *via, void *ext, void *fct);
void e6522_set_cb2_fct (e6522_t *via, void *ext, void *fct);
void e6522_set_shift_out_fct (e6522_t *via, void *ext, void *fct);
void e6522_set_irq_fct (e6522_t *via, void *ext, void *fct);

unsigned char e6522_shift_out (e6522_t *via);
void e6522_shift_in (e6522_t *via, unsigned char val);

void e6522_set_ca1_inp (e6522_t *via, unsigned char val);
void e6522_set_ca2_inp (e6522_t *via, unsigned char val);

void e6522_set_cb1_inp (e6522_t *via, unsigned char val);
void e6522_set_cb2_inp (e6522_t *via, unsigned char val);

void e6522_set_ira_inp (e6522_t *via, unsigned char val);
void e6522_set_irb_inp (e6522_t *via, unsigned char val);

void e6522_set_shift_inp (e6522_t *via, unsigned char val);

void e6522_set_ora (e6522_t *via, unsigned char val);
void e6522_set_ddra (e6522_t *via, unsigned char val);
void e6522_set_orb (e6522_t *via, unsigned char val);
void e6522_set_ddrb (e6522_t *via, unsigned char val);

unsigned char e6522_get_uint8 (e6522_t *via, unsigned long addr);
unsigned short e6522_get_uint16 (e6522_t *via, unsigned long addr);
unsigned long e6522_get_uint32 (e6522_t *via, unsigned long addr);

void e6522_set_uint8 (e6522_t *via, unsigned long addr, unsigned char val);
void e6522_set_uint16 (e6522_t *via, unsigned long addr, unsigned short val);
void e6522_set_uint32 (e6522_t *via, unsigned long addr, unsigned long val);

void e6522_reset (e6522_t *via);

void e6522_clock (e6522_t *via, unsigned long n);


#endif
