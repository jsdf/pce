/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/dosmem.h                                        *
 * Created:     2012-12-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2015 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DOS_DOSMEM_H
#define PCE_DOS_DOSMEM_H 1


#include "dos.h"


unsigned short sim_mem_alloc (dos_t *sim, unsigned para_min, unsigned para_max);
unsigned short sim_mem_resize (dos_t *sim, unsigned short blk, unsigned short para);
int sim_mem_free (dos_t *sim, unsigned short blk);
unsigned short sim_mem_get_max (dos_t *sim);
unsigned short sim_mem_get_size (dos_t *sim, unsigned short blk);


#endif
