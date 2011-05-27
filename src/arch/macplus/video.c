/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/video.c                                     *
 * Created:     2007-04-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include "main.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>


#define MAC_VIDEO_PFREQ 15667200
#define MAC_VIDEO_HFREQ (MAC_VIDEO_PFREQ / (512 + 192))
#define MAC_VIDEO_VFREQ (MAC_VIDEO_HFREQ / (342 + 28))

/* #define MAC_VIDEO_VB1 ((342 * (512 + 192) * 7833600) / MAC_VIDEO_PFREQ) */
#define MAC_VIDEO_VB1 120384

/* #define MAC_VIDEO_VB2 (((342 + 28) * (512 + 192) * 7833600) / MAC_VIDEO_PFREQ) */
#define MAC_VIDEO_VB2 130240


int mac_video_init (mac_video_t *mv, unsigned w, unsigned h)
{
	mv->vbuf = NULL;
	mv->trm = NULL;

	mv->w = w;
	mv->h = h;

	mv->cmp_cnt = 8;

	mv->vcmp = malloc ((unsigned long) ((mv->w + 7) / 8) * mv->h);

	if (mv->vcmp == NULL) {
		return (1);
	}

	mv->rgb = malloc (3UL * (unsigned long) w * mv->cmp_cnt);

	if (mv->rgb == NULL) {
		return (1);
	}

	mv->clk = 0;

	mv->vbi_val = 0;
	mv->vbi_ext = NULL;
	mv->set_vbi = NULL;

	return (0);
}

mac_video_t *mac_video_new (unsigned w, unsigned h)
{
	mac_video_t *mv;

	mv = malloc (sizeof (mac_video_t));

	if (mv == NULL) {
		return (NULL);
	}

	if (mac_video_init (mv, w, h)) {
		free (mv);
		return (NULL);
	}

	return (mv);
}

void mac_video_free (mac_video_t *mv)
{
}

void mac_video_del (mac_video_t *mv)
{
	if (mv != NULL) {
		mac_video_free (mv);
		free (mv);
	}
}

void mac_video_set_vbi_fct (mac_video_t *mv, void *ext, void *fct)
{
	mv->vbi_ext = ext;
	mv->set_vbi = fct;
}

void mac_video_set_vbuf (mac_video_t *mv, const unsigned char *vbuf)
{
	mv->vbuf = vbuf;
}

void mac_video_set_terminal (mac_video_t *mv, terminal_t *trm)
{
	mv->trm = trm;

	if (mv->trm != NULL) {
		trm_open (mv->trm, mv->w, mv->h);
	}
}

static
void mac_video_set_vbi (mac_video_t *mv, unsigned char val)
{
	if (mv->vbi_val == val) {
		return;
	}

	mv->vbi_val = val;

	if (mv->set_vbi != NULL) {
		mv->set_vbi (mv->vbi_ext, val);
	}
}

static
void mac_video_update (mac_video_t *mv)
{
	unsigned            y;
	unsigned            i, j;
	unsigned            k, n;
	const unsigned char *src;
	unsigned char       *dst, *rgb;

	if (mv->trm == NULL) {
		return;
	}

	if (mv->vbuf == NULL) {
		return;
	}

	trm_set_size (mv->trm, mv->w, mv->h);

	src = mv->vbuf;
	dst = mv->vcmp;
	rgb = mv->rgb;

	y = 0;
	while (y < mv->h) {
		n = mv->h - y;

		if (n > mv->cmp_cnt) {
			n = mv->cmp_cnt;
		}

		k = n * ((mv->w + 7) / 8);

		if (memcmp (dst, src, k) != 0) {
			memcpy (dst, src, k);

			j = 0;
			for (i = 0; i < (8 * k); i++) {
				if (dst[i >> 3] & (0x80 >> (i & 7))) {
					rgb[j + 0] = 0;
					rgb[j + 1] = 0;
					rgb[j + 2] = 0;
				}
				else {
					rgb[j + 0] = 0xff;
					rgb[j + 1] = 0xff;
					rgb[j + 2] = 0xff;
				}

				j += 3;
			}

			trm_set_lines (mv->trm, rgb, y, n);
		}

		src += k;
		dst += k;

		y += n;
	}

	trm_update (mv->trm);
}

void mac_video_redraw (mac_video_t *mv)
{
	mac_video_update (mv);
}

void mac_video_clock (mac_video_t *mv, unsigned long n)
{
	unsigned long old;

	old = mv->clk;

	mv->clk += n;

	if (mv->clk < MAC_VIDEO_VB1) {
		return;
	}

	if (old < MAC_VIDEO_VB1) {
		/* vbl start */
		mac_video_update (mv);
		mac_video_set_vbi (mv, 1);
	}

	if (mv->clk >= MAC_VIDEO_VB2) {
		mac_video_set_vbi (mv, 0);

		mv->clk -= MAC_VIDEO_VB2;
	}
}
