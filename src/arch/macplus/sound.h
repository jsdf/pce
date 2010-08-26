/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/sound.h                                     *
 * Created:     2008-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2008-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_SOUND_H
#define PCE_MACPLUS_SOUND_H 1


#include <drivers/sound/sound.h>


typedef struct {
	sound_drv_t         *drv;

	const unsigned char *sbuf;

	unsigned            idx;
	unsigned            cnt;
	uint16_t            buf[370];
	unsigned long       clk;

	int                 enable;
	unsigned            volume;

	unsigned long       lowpass_freq;
	sound_iir2_t        iir;

	uint16_t            last_val;
	int                 changed;
	unsigned            silence_cnt;
} mac_sound_t;


void mac_sound_init (mac_sound_t *ms);
mac_sound_t *mac_sound_new (void);

void mac_sound_free (mac_sound_t *ms);
void mac_sound_del (mac_sound_t *ms);

void mac_sound_set_sbuf (mac_sound_t *ms, const unsigned char *sbuf);

void mac_sound_set_lowpass (mac_sound_t *ms, unsigned freq);

void mac_sound_set_volume (mac_sound_t *ms, unsigned vol);

void mac_sound_set_enable (mac_sound_t *ms, int on);

int mac_sound_set_driver (mac_sound_t *ms, const char *driver);

void mac_sound_vbl (mac_sound_t *ms);

void mac_sound_clock (mac_sound_t *ms, unsigned long cnt);


#endif
