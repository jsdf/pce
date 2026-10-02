/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/vic20/video.c                                       *
 * Created:     2020-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2020-2021 Hampa Hug <hampa@hampa.ch>                     *
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

#include <math.h>
#include <string.h>

#include <devices/memory.h>

#include <drivers/sound/sound.h>

#include <lib/console.h>


#define VIDEO_BUF_W 284
#define VIDEO_BUF_H 312


static
double vic_pal_ypbpr[16 * 3] = {
	0.00,  0.0000000,  0.0000000,
	1.00,  0.0000000,  0.0000000,
	0.25, -0.3826834,  0.9238795,
	0.75,  0.3826834, -0.9238795,
	0.50,  0.7071068,  0.7071068,
	0.50, -0.7071068, -0.7071068,
	0.25,  1.0000000,  0.0000000,
	0.75, -1.0000000,  0.0000000,
	0.50, -0.7071068,  0.7071068,
	0.75, -0.7071068,  0.7071068,
	0.50, -0.3826834,  0.9238795,
	1.00,  0.3826834, -0.9238795,
	0.75,  0.7071068,  0.7071068,
	0.75, -0.7071068, -0.7071068,
	0.50,  1.0000000,  0.0000000,
	1.00, -1.0000000,  0.0000000
};


static
void v20_video_update_palette (vic20_video_t *vid)
{
	unsigned      i, j;
	unsigned      v;
	double        ts[4], td[3];
	double        ch, sh;
	double        *s;
	unsigned char *d;

	s = vic_pal_ypbpr;
	d = vid->palette;

	sh = sin (vid->hue * (M_PI / 180.0));
	ch = cos (vid->hue * (M_PI / 180.0));

	for (i = 0; i < 16; i++) {
		ts[0] = vid->brightness * *(s++);
		ts[1] = 0.5 * vid->saturation * *(s++);
		ts[2] = 0.5 * vid->saturation * *(s++);

		ts[3] = ch * ts[1] + sh * ts[2];
		ts[2] = ch * ts[2] - sh * ts[1];
		ts[1] = ts[3];

		td[0] = ts[0] + 1.402 * ts[2];
		td[1] = ts[0] - 0.43469847 * ts[1] - 0.71413629 * ts[2];
		td[2] = ts[0] + 1.772 * ts[1];

		for (j = 0; j < 3; j++) {
			if (td[j] < 0.0) {
				v = 0;
			}
			else if (td[j] >= 1.0) {
				v = 255;
			}
			else {
				v = (unsigned) (256.0 * td[j]);
			}

			*(d++) = v;
		}
	}

	vid->update_palette = 0;
}

static
void v20_video_hsync (vic20_video_t *vid, unsigned y, unsigned w, const unsigned char *buf)
{
	unsigned            i;
	unsigned char       *p;
	const unsigned char *s;

	if (vid->framedrop > 0) {
		return;
	}

	if ((y < vid->y) || (y >= (vid->y + vid->h))) {
		return;
	}

	if (w <= vid->x) {
		return;
	}

	w -= vid->x;
	buf += vid->x;

	if (w > vid->w) {
		w = vid->w;
	}

	if (vid->update_palette) {
		v20_video_update_palette (vid);
	}

	p = vid->buf + (3 * vid->w * (y - vid->y));

	for (i = 0; i < w; i++) {
		s = vid->palette + 3 * (buf[i] & 0x0f);

		*(p++) = *(s++);
		*(p++) = *(s++);
		*(p++) = *(s++);
	}
}

static
void v20_video_vsync (vic20_video_t *vid)
{
	if (vid->trm == NULL) {
		return;
	}

	if (vid->framedrop > 0) {
		vid->framedrop -= 1;
		return;
	}

	vid->framedrop = vid->framedrop_init;

	vid->vsync_cnt += 1;

	trm_set_size (vid->trm, vid->w, vid->h);
	trm_set_lines (vid->trm, vid->buf, 0, vid->h);
	trm_update (vid->trm);
}

static
void v20_video_sound (vic20_video_t *vid, uint16_t *buf, unsigned cnt, int done)
{
	if (vid->snd == NULL) {
		return;
	}

	snd_write (vid->snd, buf, cnt);
}

void v20_video_init (vic20_video_t *vid)
{
	unsigned i;

	vid->trm = NULL;
	vid->snd = NULL;

	vid->x = 0;
	vid->y = 0;
	vid->w = 260;
	vid->h = 261;

	vid->pal = 0;

	vid->update_palette = 1;
	vid->hue = 0.0;
	vid->saturation = 1.0;
	vid->brightness = 1.0;

	vid->framedrop = 0;
	vid->framedrop_init = 0;

	vid->vsync_cnt = 0;

	vid->clock = 0;
	vid->srate = 0;

	for (i = 0; i < 1024; i++) {
		vid->colram[i] = 0;
	}

	for (i = 0; i < (3 * VIC20_VIDEO_BUF); i++) {
		vid->buf[i] = 0;
	}

	e6560_init (&vid->vic);
	e6560_set_hsync_fct (&vid->vic, vid, v20_video_hsync);
	e6560_set_vsync_fct (&vid->vic, vid, v20_video_vsync);
	e6560_set_sound_fct (&vid->vic, vid, v20_video_sound);
	e6560_set_colram (&vid->vic, vid->colram);
}

void v20_video_free (vic20_video_t *vid)
{
	e6560_free (&vid->vic);

	if (vid->snd != NULL) {
		snd_close (vid->snd);
	}
}

void v20_video_set_clock (vic20_video_t *vid, unsigned long val)
{
	vid->clock = val;

	e6560_set_clock (&vid->vic, val);
}

int v20_video_set_srate (vic20_video_t *vid, unsigned long val)
{
	if (vid->snd != NULL) {
		if (snd_set_params (vid->snd, 1, val, 0)) {
			return (1);
		}
	}

	vid->srate = val;

	e6560_set_srate (&vid->vic, val);

	return (0);
}

void v20_video_set_pal (vic20_video_t *vid, int val, int full)
{
	if (val) {
		if (full) {
			vid->x = 0;
			vid->y = 0;
			vid->w = 284;
			vid->h = 312;
		}
		else {
			vid->x = 24;
			vid->y = 26;
			vid->w = 224;
			vid->h = 284;
		}
	}
	else {
		if (full) {
			vid->x = 0;
			vid->y = 0;
			vid->w = 260;
			vid->h = 262;
		}
		else {
			vid->x = 0;
			vid->y = 25;
			vid->w = 216;
			vid->h = 234;
		}
	}

	vid->pal = (val != 0);

	e6560_set_pal (&vid->vic, val);
}

void v20_video_set_term (vic20_video_t *vid, terminal_t *trm)
{
	vid->trm = trm;

	if (trm != NULL) {
		trm_open (trm, vid->w, vid->h);
		trm_set_aspect_ratio (trm, 4, 3);
	}
}

int v20_video_set_sound_driver (vic20_video_t *vid, const char *drv)
{
	if (vid->snd != NULL) {
		snd_close (vid->snd);
		vid->snd = NULL;
	}

	if ((drv == NULL) || (*drv == 0)) {
		return (0);
	}

	vid->snd = snd_open (drv);

	if (vid->snd == NULL) {
		return (1);
	}

	if (snd_set_params (vid->snd, 1, vid->srate, 0)) {
		snd_close (vid->snd);
		vid->snd = NULL;
		return (1);
	}

	return (0);
}

void v20_video_set_memmap (vic20_video_t *vid, memory_t *mem)
{
	e6560_set_memmap (&vid->vic, 0x00, 64, NULL);
	e6560_set_memmap (&vid->vic, 0x00, 16, mem_get_ptr (mem, 0x8000, 4096));
	e6560_set_memmap (&vid->vic, 0x20, 4, mem_get_ptr (mem, 0x0000, 1024));
	e6560_set_memmap (&vid->vic, 0x30, 16, mem_get_ptr (mem, 0x1000, 4096));
}

void v20_video_set_hue (vic20_video_t *vid, double val)
{
	vid->hue = val;
	vid->update_palette = 1;
}

void v20_video_set_saturation (vic20_video_t *vid, double val)
{
	vid->saturation = val;
	vid->update_palette = 1;
}

void v20_video_set_brightness (vic20_video_t *vid, double val)
{
	vid->brightness = val;
	vid->update_palette = 1;
}

void v20_video_set_framedrop (vic20_video_t *vid, unsigned cnt)
{
	vid->framedrop = 0;
	vid->framedrop_init = cnt;
}

void v20_video_reset (vic20_video_t *vid)
{
	unsigned i;

	e6560_reset (&vid->vic);

	for (i = 0; i < 1024; i++) {
		vid->colram[i] = 0;
	}
}

void v20_video_print_state (vic20_video_t *vid)
{
	unsigned i;
	e6560_t  *vic;

	pce_prt_sep ("VIC");

	vic = &vid->vic;

	for (i = 0; i < 4; i++) {
		pce_printf ("R[%X]=%02X  R[%X]=%02X  R[%X]=%02X  R[%X]=%02X\n",
			i, vic->reg[i],
			i + 4, vic->reg[i + 4],
			i + 8, vic->reg[i + 8],
			i + 12, vic->reg[i + 12]
		);
	}

	pce_printf ("VBASE=%04X -> %04X   ADDR1=%04X\n",
		vic->vbase,
		(vic->vbase & 8191) | ((~vic->vbase & 8192) << 2),
		vic->addr1
	);

	pce_printf ("CBASE=%04X -> %04X   ADDR2=%04X\n",
		vic->cbase,
		(vic->cbase & 8191) | ((~vic->cbase & 8192) << 2),
		vic->addr2
	);

	pce_printf ("X=%3u/%3u  COL=%2u/%2u\n",
		vic->x, vic->w, vic->col,
		vic->reg[2] & 0x7f
	);

	pce_printf ("Y=%3u/%3u  ROW=%2u/%2u  L=%u/%u\n",
		vic->y, vic->h, vic->row,
		(vic->reg[3] >> 1) & 0x3f,
		vic->line, vic->line_cnt
	);

	pce_printf ("F=%lu\n", vic->frame);
}
