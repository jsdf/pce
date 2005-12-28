/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/video.c                                        *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-08-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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
	vid->get_mem = NULL;
	vid->get_reg = NULL;
	vid->prt_state = NULL;
	vid->update = NULL;
	vid->dump = NULL;
	vid->screenshot = NULL;
	vid->clock = NULL;
}

void pce_video_del (video_t *vid)
{
	if (vid->del != NULL) {
		vid->del (vid->ext);
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

void pce_video_prt_state (video_t *vid, FILE *fp)
{
	if (vid->prt_state != NULL) {
		vid->prt_state (vid->ext, fp);
	}
}

void pce_video_update (video_t *vid)
{
	if (vid->update != NULL) {
		vid->update (vid->ext);
	}
}

int pce_video_dump (video_t *vid, FILE *fp)
{
	if (vid->dump != NULL) {
		return (vid->dump (vid->ext, fp));
	}

	return (1);
}

int pce_video_screenshot (video_t *vid, FILE *fp, unsigned mode)
{
	if (vid->screenshot != NULL) {
		return (vid->screenshot (vid->ext, fp, mode));
	}

	return (1);
}

void pce_video_clock (video_t *vid, unsigned long cnt)
{
	if (vid->clock != NULL) {
		vid->clock (vid->ext, cnt);
	}
}
