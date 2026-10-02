/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/aym/main.h                                         *
 * Created:     2015-05-21 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015-2016 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef AYM_MAIN_H
#define AYM_MAIN_H 1


#include <config.h>

#include <drivers/sound/sound.h>


#define AYM_MAGIC 0x41594d1a


extern const char    *arg0;

extern char          par_verbose;

extern unsigned long par_srate;


unsigned long aym_get_uint32_be (const void *buf, unsigned i);
void aym_set_uint32_be (void *buf, unsigned i, unsigned long val);

int aym_decode (const char *inp, const char *out, unsigned long th, unsigned long mark);

int aym_encode (const char *inp, const char *out, unsigned long th);

int aym_play (const char *fname, const char *snddrv, unsigned long lp, int hp);


#endif
