/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/intc.h                                       *
 * Created:     2004-11-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2006 Lukas Ruf <ruf@lpr.ch>                         *
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

/*****************************************************************************
 * This software was developed at the Computer Engineering and Networks      *
 * Laboratory (TIK), Swiss Federal Institute of Technology (ETH) Zurich.     *
 *****************************************************************************/


/* IXP2400 interrupt controller */

#ifndef PCE_SIMARM_INTC_H
#define PCE_SIMARM_INTC_H 1


#include <stdio.h>

#include <devices/memory.h>


/*!***************************************************************************
 * @short The IXP interrupt controller context
 *****************************************************************************/
typedef struct ixp_intc_s {
	mem_blk_t     io;

	unsigned long base;

	void          (*fiq) (void *ext, unsigned char val);
	void          *fiq_ext;
	unsigned char fiq_val;

	void          (*irq) (void *ext, unsigned char val);
	void          *irq_ext;
	unsigned char irq_val;

	unsigned long status_raw;
	unsigned long status_fiq;
	unsigned long status_irq;
	unsigned long enable_fiq;
	unsigned long enable_irq;
} ixp_intc_t;


void ict_init (ixp_intc_t *ict, unsigned long base);
ixp_intc_t *ict_new (unsigned long base);
void ict_free (ixp_intc_t *ict);
void ict_del (ixp_intc_t *ict);

mem_blk_t *ict_get_io (ixp_intc_t *ict, unsigned i);

void ict_set_fiq_f (ixp_intc_t *ict, void *f, void *ext);
void ict_set_irq_f (ixp_intc_t *ict, void *f, void *ext);

void *ict_get_irq_f (ixp_intc_t *ict, unsigned irq);

void ict_set_irq (ixp_intc_t *ict, unsigned i, unsigned char val);

void ict_set_irq0 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq1 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq2 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq3 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq4 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq5 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq6 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq7 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq8 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq9 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq10 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq11 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq12 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq13 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq14 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq15 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq16 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq17 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq18 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq19 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq20 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq21 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq22 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq23 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq24 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq25 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq26 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq27 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq28 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq29 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq30 (ixp_intc_t *ict, unsigned char val);
void ict_set_irq31 (ixp_intc_t *ict, unsigned char val);


#endif
