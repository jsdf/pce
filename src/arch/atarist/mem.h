/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/mem.h                                       *
 * Created:     2011-03-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_ATARIST_MEM_H
#define PCE_ATARIST_MEM_H 1


unsigned char st_mem_get_uint8 (void *ext, unsigned long addr);
unsigned short st_mem_get_uint16 (void *ext, unsigned long addr);
unsigned long st_mem_get_uint32 (void *ext, unsigned long addr);

void st_mem_set_uint8 (void *ext, unsigned long addr, unsigned char val);
void st_mem_set_uint16 (void *ext, unsigned long addr, unsigned short val);
void st_mem_set_uint32 (void *ext, unsigned long addr, unsigned long val);


#endif
