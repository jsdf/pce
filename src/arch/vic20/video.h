/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/video.h                                       *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_VIC20_VIDEO_H
#define PCE_VIC20_VIDEO_H 1


#include "video.h"

#include <chipset/e6560.h>

#include <devices/memory.h>

#include <drivers/sound/sound.h>

#include <drivers/video/terminal.h>


#define VIC20_VIDEO_BUF (284 * 312)


typedef struct {
	e6560_t       vic;

	terminal_t    *trm;
	sound_drv_t   *snd;

	unsigned      x;
	unsigned      y;
	unsigned      w;
	unsigned      h;

	char          pal;

	char          update_palette;
	double        hue;
	double        saturation;
	double        brightness;

	unsigned      framedrop;
	unsigned      framedrop_init;

	unsigned      vsync_cnt;

	unsigned long clock;
	unsigned long srate;

	unsigned char palette[16 * 3];

	unsigned char colram[1024];

	unsigned char buf[3 * VIC20_VIDEO_BUF];
} vic20_video_t;


void v20_video_init (vic20_video_t *vid);
void v20_video_free (vic20_video_t *vid);

void v20_video_set_clock (vic20_video_t *vid, unsigned long val);
int v20_video_set_srate (vic20_video_t *vid, unsigned long val);

void v20_video_set_pal (vic20_video_t *vid, int val, int full);

void v20_video_set_term (vic20_video_t *vid, terminal_t *trm);

int v20_video_set_sound_driver (vic20_video_t *vid, const char *drv);

void v20_video_set_memmap (vic20_video_t *vid, memory_t *mem);

void v20_video_set_hue (vic20_video_t *vid, double val);
void v20_video_set_saturation (vic20_video_t *vid, double val);
void v20_video_set_brightness (vic20_video_t *vid, double val);

void v20_video_set_framedrop (vic20_video_t *vid, unsigned cnt);

void v20_video_reset (vic20_video_t *vid);

void v20_video_print_state (vic20_video_t *vid);


#endif
