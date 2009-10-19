/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/video.c                                     *
 * Created:     2007-04-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#define MAC_VIDEO_PFREQ 15667200
#define MAC_VIDEO_HFREQ (MAC_VIDEO_PFREQ / (512 + 192))
#define MAC_VIDEO_VFREQ (MAC_VIDEO_HFREQ / (342 + 28))

/* #define MAC_VIDEO_VB1 ((342 * (512 + 192) * 7833600) / MAC_VIDEO_PFREQ) */
#define MAC_VIDEO_VB1 120384

/* #define MAC_VIDEO_VB1 (((342 + 28) * (512 + 192) * 7833600) / MAC_VIDEO_PFREQ) */
#define MAC_VIDEO_VB2 130240


void mac_video_init (mac_video_t *mv)
{
	mv->vbuf = NULL;
	mv->trm = NULL;

	mv->clk = 0;
	mv->clkacc = 0;
	mv->realclk = 0;

	mv->realtime = 0;

	mv->vbi_val = 0;
	mv->vbi_ext = NULL;
	mv->set_vbi = NULL;
}

mac_video_t *mac_video_new (void)
{
	mac_video_t *mv;

	mv = malloc (sizeof (mac_video_t));
	if (mv == NULL) {
		return (NULL);
	}

	mac_video_init (mv);

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

void mac_video_set_realtime (mac_video_t *mv, int realtime)
{
	mv->realtime = (realtime != 0);
}

void mac_video_set_vbuf (mac_video_t *mv, const unsigned char *vbuf)
{
	mv->vbuf = vbuf;
}

void mac_video_set_terminal (mac_video_t *mv, terminal_t *trm)
{
	mv->trm = trm;

	if (mv->trm != NULL) {
		trm_open (mv->trm, 512, 342);
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
	unsigned char       *dst;
	unsigned char       rgb[512 * 8 * 3];

	if (mv->trm == NULL) {
		return;
	}

	if (mv->vbuf == NULL) {
		return;
	}

	trm_set_size (mv->trm, MAC_VIDEO_W, MAC_VIDEO_H);

	src = mv->vbuf;
	dst = mv->vcmp;

	y = 0;
	while (y < MAC_VIDEO_H) {
		n = MAC_VIDEO_H - y;
		if (n > 8) {
			n = 8;
		}

		k = n * (MAC_VIDEO_W / 8);

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
	unsigned long us;

	if (mv->realtime) {
		old = mv->realclk;

		us = pce_get_interval_us (&mv->realclk);

		if (us < 100) {
			mv->realclk = old;
			return;
		}

		if (us > (4 * 16625)) {
			/* discontinuity */
#ifdef DEBUG_VIDEO
			mac_log_deb ("video: discontinuity\n");
#endif
			us = 4 * 16625;
		}

		/* n = 7.8336 * us */
		n = (32086 * us) / 4096;
	}

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

		mv->clkacc += (mv->clk - MAC_VIDEO_VB2);

		if (mv->clkacc < (MAC_VIDEO_VB2 / 4)) {
			mv->clk = mv->clkacc;
		}
		else {
			mv->clk = MAC_VIDEO_VB2 / 4;
		}

		mv->clkacc -= mv->clk;
	}
}
