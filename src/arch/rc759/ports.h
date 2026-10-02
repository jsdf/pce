/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/ports.h                                       *
 * Created:     2021-08-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2021 Hampa Hug <hampa@hampa.ch>                          *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General *
 * Public License for more details.                                          *
 *****************************************************************************/


#ifndef PCE_RC759_PORTS_H
#define PCE_RC759_PORTS_H 1


#include "rc759.h"


unsigned char rc759_get_port8 (rc759_t *sim, unsigned long addr);
unsigned short rc759_get_port16 (rc759_t *sim, unsigned long addr);

void rc759_set_port8 (rc759_t *sim, unsigned long addr, unsigned char val);
void rc759_set_port16 (rc759_t *sim, unsigned long addr, unsigned short val);


#endif
