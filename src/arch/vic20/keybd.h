/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/keybd.h                                       *
 * Created:     2020-04-19 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_VIC20_KEYBD_H
#define PCE_VIC20_KEYBD_H 1


#include "vic20.h"

#include <drivers/video/keys.h>


void v20_keybd_matrix (vic20_t *sim, unsigned char *row, unsigned char *col);

void v20_keybd_reset (vic20_t *sim);

void v20_keybd_set_key (vic20_t *sim, unsigned event, pce_key_t key);


#endif
