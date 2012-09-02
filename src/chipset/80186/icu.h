/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/80186/icu.h                                      *
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


#ifndef PCE_CHIPSET_80186_ICU_H
#define PCE_CHIPSET_80186_ICU_H 1


typedef struct {
	unsigned short valid_irqs;
	unsigned short inp;
	unsigned short irr;
	unsigned short imr;
	unsigned short isr;
	unsigned short pmr;
	unsigned short intsts;

	unsigned short icon[8];
	unsigned short icon_mask[8];

	unsigned long  int_cnt[8];

	void           *intr_ext;
	unsigned char  intr_val;
	void           (*intr) (void *ext, unsigned char val);

	void           *inta0_ext;
	unsigned char  (*inta0) (void *ext);

	void           *inta1_ext;
	unsigned char  (*inta1) (void *ext);
} e80186_icu_t;


void e80186_icu_init (e80186_icu_t *icu);
void e80186_icu_free (e80186_icu_t *icu);

void e80186_icu_set_intr_fct (e80186_icu_t *icu, void *ext, void *fct);
void e80186_icu_set_inta0_fct (e80186_icu_t *icu, void *ext, void *fct);
void e80186_icu_set_inta1_fct (e80186_icu_t *icu, void *ext, void *fct);

void e80186_icu_reset (e80186_icu_t *icu);

void e80186_icu_set_irq (e80186_icu_t *icu, unsigned idx, int val);
void e80186_icu_set_irq_tmr0 (e80186_icu_t *icu, unsigned char val);
void e80186_icu_set_irq_tmr1 (e80186_icu_t *icu, unsigned char val);
void e80186_icu_set_irq_tmr2 (e80186_icu_t *icu, unsigned char val);
void e80186_icu_set_irq_dma0 (e80186_icu_t *icu, unsigned char val);
void e80186_icu_set_irq_dma1 (e80186_icu_t *icu, unsigned char val);
void e80186_icu_set_irq_int0 (e80186_icu_t *icu, unsigned char val);
void e80186_icu_set_irq_int1 (e80186_icu_t *icu, unsigned char val);

unsigned short e80186_icu_get_icon (const e80186_icu_t *icu, unsigned idx);
void e80186_icu_set_icon (e80186_icu_t *icu, unsigned idx, unsigned short val);

unsigned short e80186_icu_get_imr (const e80186_icu_t *icu);
void e80186_icu_set_imr (e80186_icu_t *icu, unsigned short val);

unsigned short e80186_icu_get_pmr (const e80186_icu_t *icu);
void e80186_icu_set_pmr (e80186_icu_t *icu, unsigned short val);

unsigned short e80186_icu_get_isr (const e80186_icu_t *icu);
void e80186_icu_set_isr (e80186_icu_t *icu, unsigned short val);

unsigned short e80186_icu_get_irr (const e80186_icu_t *icu);
void e80186_icu_set_irr (e80186_icu_t *icu, unsigned short val);

void e80186_icu_set_eoi (e80186_icu_t *icu, unsigned short val);

unsigned short e80186_icu_get_intsts (const e80186_icu_t *icu);
void e80186_icu_set_intsts (e80186_icu_t *icu, unsigned short val);

unsigned short e80186_icu_get_pollst (e80186_icu_t *icu);
unsigned short e80186_icu_get_poll (e80186_icu_t *icu);

unsigned char e80186_icu_inta (e80186_icu_t *icu);


#endif
