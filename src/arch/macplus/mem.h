/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/mem.h                                       *
 * Created:     2007-11-13 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_MEM_H
#define PCE_MACPLUS_MEM_H 1


#include "macplus.h"


void mac_set_overlay (macplus_t *sim, int overlay);


unsigned char mac_mem_get_uint8 (void *ext, unsigned long addr);
unsigned short mac_mem_get_uint16 (void *ext, unsigned long addr);
unsigned long mac_mem_get_uint32 (void *ext, unsigned long addr);

void mac_mem_set_uint8 (void *ext, unsigned long addr, unsigned char val);
void mac_mem_set_uint16 (void *ext, unsigned long addr, unsigned short val);
void mac_mem_set_uint32 (void *ext, unsigned long addr, unsigned long val);


#endif
