/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/rp5c15.h                                    *
 * Created:     2013-06-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_ATARIST_RP5C15_H
#define PCE_ATARIST_RP5C15_H 1


typedef struct {
	unsigned char mode;
	unsigned char bank0[16];
	unsigned char bank1[16];
} rp5c15_t;


void rp5c15_init (rp5c15_t *rtc);
void rp5c15_free (rp5c15_t *rtc);

unsigned char rp5c15_get_uint8 (rp5c15_t *rtc, unsigned long addr);
void rp5c15_set_uint8 (rp5c15_t *rtc, unsigned long addr, unsigned char val);


#endif
