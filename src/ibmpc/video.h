/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/video.h                                          *
 * Created:       2003-08-30 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-23 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: video.h,v 1.5 2003/09/23 00:39:16 hampa Exp $ */


#ifndef PCE_VIDEO_H
#define PCE_VIDEO_H 1


#include <stdio.h>


#define PCE_VIDEO_NONE 0
#define PCE_VIDEO_MDA  1
#define PCE_VIDEO_CGA  2
#define PCE_VIDEO_HGC  3
#define PCE_VIDEO_EGA  4


typedef void (*pce_video_del_f) (void *ext);
typedef mem_blk_t *(*pce_video_get_mem_f) (void *ext);
typedef mem_blk_t *(*pce_video_get_reg_f) (void *ext);
typedef void (*pce_video_prt_state_f) (void *ext, FILE *fp);
typedef int (*pce_video_dump_f) (void *ext, FILE *fp);
typedef int (*pce_video_screenshot_f) (void *ext, FILE *fp, unsigned mode);


typedef struct {
  unsigned x;
  unsigned y;
  unsigned w;
  unsigned h;
} scrmap_pixel_t;

typedef struct {
  unsigned       w;
  unsigned       h;
  unsigned       sw;
  unsigned       sh;
  scrmap_pixel_t *pxl;
} scrmap_t;


typedef struct {
  pce_video_del_f        del;
  pce_video_get_mem_f    get_mem;
  pce_video_get_reg_f    get_reg;
  pce_video_prt_state_f  prt_state;
  pce_video_dump_f       dump;
  pce_video_screenshot_f screenshot;

  void     *ext;

  unsigned type;
} video_t;


void pce_video_init (video_t *vid);
void pce_video_del (video_t *vid);

mem_blk_t *pce_video_get_mem (video_t *vid);
mem_blk_t *pce_video_get_reg (video_t *vid);

void pce_video_prt_state (video_t *vid, FILE *fp);

int pce_video_dump (video_t *vid, FILE *fp);

int pce_video_screenshot (video_t *vid, FILE *fp, unsigned mode);


void pce_smap_init (scrmap_t *smap,
  unsigned w, unsigned h, unsigned sw, unsigned sh
);

void pce_smap_free (scrmap_t *smap);

void pce_smap_get_pixel (scrmap_t *smap, unsigned x, unsigned y,
  unsigned *sx, unsigned *sy, unsigned *sw, unsigned *sh
);


#endif
