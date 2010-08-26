/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/sound/sound-null.h                               *
 * Created:     2009-10-17 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_DRIVERS_SOUND_NULL_H
#define PCE_DRIVERS_SOUND_NULL_H 1


#include <drivers/sound/sound.h>


typedef struct sound_null_t {
	sound_drv_t sdrv;
} sound_null_t;


#endif
