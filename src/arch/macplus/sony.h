/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/sony.h                                      *
 * Created:     2007-11-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_SONY_H
#define PCE_MACPLUS_SONY_H 1


typedef struct {
	char               open;
	char               patched;
	unsigned long      delay;

	unsigned long      check;
	unsigned long      icon[2];

	unsigned long      pcex_addr;
	unsigned long      sony_addr;
	unsigned char      patch_buf[64];
} mac_sony_t;


void mac_sony_init (mac_sony_t *sony);
void mac_sony_free (mac_sony_t *sony);

void mac_sony_patch (macplus_t *sim);

void mac_sony_insert (macplus_t *sim, unsigned drive);

void mac_sony_check (macplus_t *sim);

int mac_sony_hook (macplus_t *sim, unsigned val);

void mac_sony_reset (macplus_t *sim);


#endif
