/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/video.c                                          *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: video.c,v 1.1 2003/08/30 03:08:53 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


void pce_video_init (video_t *vid)
{
  vid->ext = NULL;

  vid->del = NULL;
  vid->get_mem = NULL;
  vid->get_reg = NULL;
  vid->prt_state = NULL;
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
