/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/video.c                                        *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2007 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


#include <stdio.h>
#include <stdlib.h>

#include "video.h"


void pce_video_init (video_t *vid)
{
	vid->ext = NULL;

	vid->type = PCE_VIDEO_NONE;

	vid->del = NULL;
	vid->set_msg = NULL;
	vid->set_terminal = NULL;
	vid->get_mem = NULL;
	vid->get_reg = NULL;
	vid->print_info = NULL;
	vid->screenshot = NULL;
	vid->redraw = NULL;
	vid->clock = NULL;
}

void pce_video_del (video_t *vid)
{
	if (vid->del != NULL) {
		vid->del (vid->ext);
	}
}

void pce_video_set_msg (video_t *vid, const char *msg, const char *val)
{
	if (vid->set_msg != NULL) {
		vid->set_msg (vid->ext, msg, val);
	}
}

void pce_video_set_terminal (video_t *vid, void *trm)
{
	if (vid->set_terminal != NULL) {
		vid->set_terminal (vid->ext, trm);
	}
}

mem_blk_t *pce_video_get_mem (video_t *vid)
{
	if (vid->get_mem != NULL) {
		return (vid->get_mem (vid->ext));
	}

	return (NULL);
}

mem_blk_t *pce_video_get_reg (video_t *vid)
{
	if (vid->get_reg != NULL) {
		return (vid->get_reg (vid->ext));
	}

	return (NULL);
}

void pce_video_print_info (video_t *vid, FILE *fp)
{
	if (vid->print_info != NULL) {
		vid->print_info (vid->ext, fp);
	}
}

int pce_video_screenshot (video_t *vid, FILE *fp, unsigned mode)
{
	if (vid->screenshot != NULL) {
		return (vid->screenshot (vid->ext, fp, mode));
	}

	return (1);
}

void pce_video_redraw (video_t *vid)
{
	if (vid->redraw != NULL) {
		vid->redraw (vid->ext);
	}
}

void pce_video_clock (video_t *vid, unsigned long cnt)
{
	if (vid->clock != NULL) {
		vid->clock (vid->ext, cnt);
	}
}
