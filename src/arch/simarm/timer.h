/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/simarm/timer.h                                      *
 * Created:     2004-11-14 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_SIMARM_TIMER_H
#define PCE_SIMARM_TIMER_H 1


typedef struct ixp_timer_counter_s {
	unsigned long ctrl;
	unsigned long load;
	unsigned long status;
	unsigned long clear;

	/* clock divisor, remaining clocks */
	unsigned long clkdiv;

	/* set to the clocking function if the counter is active */
	void          (*clock) (struct ixp_timer_counter_s *cntr, unsigned long n);

	void          (*irq) (void *ext, unsigned char val);
	void          *irq_ext;
	unsigned char irq_val;
} ixp_timer_counter_t;


/*!***************************************************************************
 * @short The IXP timer context
 *****************************************************************************/
typedef struct ixp_timer_s {
	mem_blk_t           io;

	unsigned long       base;

	ixp_timer_counter_t cntr[4];

	unsigned long       twde;
} ixp_timer_t;


void tmr_init (ixp_timer_t *tmr, unsigned long base);
ixp_timer_t *tmr_new (unsigned long base);
void tmr_free (ixp_timer_t *tmr);
void tmr_del (ixp_timer_t *tmr);

mem_blk_t *tmr_get_io (ixp_timer_t *tmr, unsigned i);

int tmr_get_active (ixp_timer_t *tmr, unsigned i);

void tmr_set_irq_f (ixp_timer_t *tmr, unsigned i, void *f, void *ext);

void tmr_clock (ixp_timer_t *tmr, unsigned long n);


#endif
