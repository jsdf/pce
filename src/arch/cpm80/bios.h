/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/cpm80/bios.h                                        *
 * Created:     2012-11-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2020 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_CPM80_BIOS_H
#define PCE_CPM80_BIOS_H 1


#include "cpm80.h"


int con_ready (cpm80_t *sim);
int con_getc (cpm80_t *sim, unsigned char *c);
int con_putc (cpm80_t *sim, unsigned char c);
int con_puts (cpm80_t *sim, const char *str);
int aux_ready (cpm80_t *sim);
int aux_getc (cpm80_t *sim, unsigned char *c);
int aux_putc (cpm80_t *sim, unsigned char c);
int lst_putc (cpm80_t *sim, unsigned char c);

void c80_bios (cpm80_t *sim, unsigned fct);
void c80_bios_init (cpm80_t *sim);


#endif
