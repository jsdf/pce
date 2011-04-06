/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/adb_mouse.h                                 *
 * Created:     2010-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_ADB_MOUSE_H
#define PCE_MACPLUS_ADB_MOUSE_H 1


#include "adb.h"


typedef struct {
	adb_dev_t dev;

	char      talking;

	int       change;

	unsigned  button;
	int       dx;
	int       dy;

	unsigned  talk_button;
	int       talk_dx;
	int       talk_dy;
} adb_mouse_t;


void adb_mouse_move (adb_mouse_t *am, unsigned button, int dx, int dy);

void adb_mouse_init (adb_mouse_t *am);

adb_mouse_t *adb_mouse_new (void);


#endif
