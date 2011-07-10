/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/video/terminal.c                                 *
 * Created:     2003-04-18 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/video/terminal.h>


#define TRM_ESC_ESC  1
#define TRM_ESC_OK   2
#define TRM_ESC_ALT  4
#define TRM_ESC_CTRL 8
#define TRM_ESC_KEYS (TRM_ESC_ALT | TRM_ESC_CTRL)


void trm_init (terminal_t *trm, void *ext)
{
	trm->ext = ext;

	trm->set_msg_emu_ext = NULL;
	trm->set_msg_emu = NULL;

	trm->set_key_ext = NULL;
	trm->set_key = NULL;

	trm->set_mouse_ext = NULL;
	trm->set_mouse = NULL;

	trm->del = NULL;
	trm->set_msg_trm = NULL;
	trm->update = NULL;
	trm->check = NULL;

	trm->is_open = 0;

	trm->escape_key = PCE_KEY_ESC;
	trm->escape = 0;

	trm->w = 0;
	trm->h = 0;

	trm->buf_cnt = 0;
	trm->buf = NULL;

	trm->scale_x = 1;
	trm->scale_y = 1;

	trm->mouse_scale_x[0] = 1;
	trm->mouse_scale_x[1] = 1;
	trm->mouse_scale_x[2] = 0;
	trm->mouse_scale_y[0] = 1;
	trm->mouse_scale_y[1] = 1;
	trm->mouse_scale_y[2] = 0;

	trm->scale_buf_cnt = 0;
	trm->scale_buf = NULL;

	trm->update_x = 0;
	trm->update_y = 0;
	trm->update_w = 0;
	trm->update_h = 0;

	trm->pict_index = 0;
}

void trm_free (terminal_t *trm)
{
	trm_close (trm);

	free (trm->buf);
	free (trm->scale_buf);
}

void trm_del (terminal_t *trm)
{
	if (trm == NULL) {
		return;
	}

	trm_free (trm);

	if (trm->del != NULL) {
		trm->del (trm->ext);
	}
}

int trm_open (terminal_t *trm, unsigned w, unsigned h)
{
	if (trm->is_open) {
		return (0);
	}

	if (trm->open != NULL) {
		if (trm->open (trm->ext, w, h)) {
			return (1);
		}
	}

	trm->is_open = 1;

	return (0);
}

int trm_close (terminal_t *trm)
{
	if (trm->is_open == 0) {
		return (0);
	}

	if (trm->close != NULL) {
		if (trm->close (trm->ext)) {
			return (1);
		}
	}

	trm->is_open = 0;

	return (0);
}

void trm_set_msg_fct (terminal_t *trm, void *ext, void *fct)
{
	trm->set_msg_emu_ext = ext;
	trm->set_msg_emu = fct;
}

void trm_set_key_fct (terminal_t *trm, void *ext, void *fct)
{
	trm->set_key_ext = ext;
	trm->set_key = fct;
}

void trm_set_mouse_fct (terminal_t *trm, void *ext, void *fct)
{
	trm->set_mouse_ext = ext;
	trm->set_mouse = fct;
}

int trm_screenshot (terminal_t *trm, const char *fname)
{
	FILE          *fp;
	char          str[256];
	unsigned long cnt;

	if ((fname == NULL) || (fname[0] == 0)) {
		sprintf (str, "pce%04u.ppm", trm->pict_index);
		trm->pict_index += 1;
		fname = str;
	}

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	cnt = 3 * (unsigned long) trm->w * (unsigned long) trm->h;

	fprintf (fp, "P6\n%u %u\n%u\x0a", trm->w, trm->h, 255);

	if (fwrite (trm->buf, 1, cnt, fp) != cnt) {
		fclose (fp);
		return (1);
	}

	fclose (fp);

	return (0);
}

int trm_set_msg_trm (terminal_t *trm, const char *msg, const char *val)
{
	if (strcmp (msg, "term.screenshot") == 0) {
		trm_screenshot (trm, val);
		return (0);
	}

	if (trm->set_msg_trm != NULL) {
		return (trm->set_msg_trm (trm->ext, msg, val));
	}

	return (-1);
}

void trm_set_mouse_scale (terminal_t *trm, int mul_x, int div_x, int mul_y, int div_y)
{
	trm->mouse_scale_x[0] = mul_x;
	trm->mouse_scale_x[1] = div_x;
	trm->mouse_scale_x[2] = 0;

	trm->mouse_scale_y[0] = mul_y;
	trm->mouse_scale_y[1] = div_y;
	trm->mouse_scale_y[2] = 0;
}

void trm_set_size (terminal_t *trm, unsigned w, unsigned h)
{
	unsigned long cnt;

	if ((w == 0) || (h == 0)) {
		free (trm->buf);

		trm->buf_cnt = 0;
		trm->buf = NULL;

		trm->w = 0;
		trm->h = 0;

		return;
	}

	if ((trm->w == w) && (trm->h == h)) {
		return;
	}

	cnt = 3 * (unsigned long) w * (unsigned long) h;

	if (trm->buf_cnt != cnt) {
		unsigned char *tmp;

		tmp = realloc (trm->buf, cnt);
		if (tmp == NULL) {
			return;
		}

		trm->buf = tmp;
		trm->buf_cnt = cnt;
	}

	trm->w = w;
	trm->h = h;

	trm->update_x = 0;
	trm->update_y = 0;
	trm->update_w = w;
	trm->update_h = h;
}

void trm_set_scale (terminal_t *trm, unsigned fx, unsigned fy)
{
	if ((fx < 1) || (fy < 1)) {
		fx = 1;
		fy = 1;
	}

	trm->scale_x = fx;
	trm->scale_y = fy;

	trm->update_x = 0;
	trm->update_y = 0;
	trm->update_w = trm->w;
	trm->update_h = trm->h;
}

void trm_set_pixel (terminal_t *trm, unsigned x, unsigned y, const unsigned char *col)
{
	unsigned char *buf;

	buf = trm->buf + 3 * (trm->w * y + x);

	buf[0] = col[0];
	buf[1] = col[1];
	buf[2] = col[2];

	if (trm->update_w == 0) {
		trm->update_x = x;
		trm->update_w = 1;
	}
	else {
		if (x < trm->update_x) {
			trm->update_w += trm->update_x - x;
			trm->update_x = x;
		}

		if (x >= (trm->update_x + trm->update_w)) {
			trm->update_w = x - trm->update_x + 1;
		}
	}

	if (trm->update_h == 0) {
		trm->update_y = y;
		trm->update_h = 1;
	}
	else {
		if (y < trm->update_y) {
			trm->update_h += trm->update_y - y;
			trm->update_y = y;
		}

		if (y >= (trm->update_y + trm->update_h)) {
			trm->update_h = y - trm->update_y + 1;
		}
	}
}

void trm_set_lines (terminal_t *trm, const void *buf, unsigned y, unsigned cnt)
{
	unsigned long       w3, tmp;
	const unsigned char *src;
	unsigned char       *dst;

	w3 = 3UL * trm->w;

	src = buf;
	dst = trm->buf + w3 * y;

	while (cnt > 0) {
		if (memcmp (dst, src, w3) != 0) {
			break;
		}

		src += w3;
		dst += w3;
		y += 1;
		cnt -= 1;
	}

	tmp = cnt * w3;

	while (cnt > 0) {
		tmp -= w3;

		if (memcmp (dst + tmp, src + tmp, w3) != 0) {
			break;
		}

		cnt -= 1;
	}

	if (cnt == 0) {
		return;
	}

	memcpy (dst, src, w3 * cnt);

	trm->update_x = 0;
	trm->update_w = trm->w;

	if (trm->update_h == 0) {
		trm->update_y = y;
		trm->update_h = cnt;
	}
	else {
		if (y < trm->update_y) {
			trm->update_h += trm->update_y - y;
			trm->update_y = y;
		}

		if ((y + cnt) > (trm->update_y + trm->update_h)) {
			trm->update_h = (y + cnt) - trm->update_y;
		}
	}
}

void trm_update (terminal_t *trm)
{
	if ((trm->update_w == 0) || (trm->update_h == 0)) {
		return;
	}

	if (trm->update != NULL) {
		trm->update (trm->ext);
	}

	trm->update_x = 0;
	trm->update_y = 0;
	trm->update_w = 0;
	trm->update_h = 0;
}

void trm_check (terminal_t *trm)
{
	if (trm->check != NULL) {
		trm->check (trm->ext);
	}
}


int trm_set_msg_emu (terminal_t *trm, const char *msg, const char *val)
{
	if (trm->set_msg_emu != NULL) {
		return (trm->set_msg_emu (trm->set_msg_emu_ext, msg, val));
	}

	return (1);
}

int trm_set_escape_str (terminal_t *trm, const char *str)
{
	pce_key_t key;

	key = pce_key_from_string (str);

	if (key == PCE_KEY_NONE) {
		return (1);
	}

	trm->escape_key = key;

	return (0);
}

void trm_set_escape_key (terminal_t *trm, pce_key_t key)
{
	trm->escape_key = key;
}

static
int trm_set_key_magic (terminal_t *trm, pce_key_t key)
{
	switch (key) {
	case PCE_KEY_0:
		trm_set_msg_emu (trm, "emu.cpu.speed", "0");
		return (0);

	case PCE_KEY_1:
		trm_set_msg_emu (trm, "emu.cpu.speed", "1");
		return (0);

	case PCE_KEY_2:
		trm_set_msg_emu (trm, "emu.cpu.speed", "2");
		return (0);

	case PCE_KEY_3:
		trm_set_msg_emu (trm, "emu.cpu.speed", "3");
		return (0);

	case PCE_KEY_4:
		trm_set_msg_emu (trm, "emu.cpu.speed", "4");
		return (0);

	case PCE_KEY_5:
		trm_set_msg_emu (trm, "emu.cpu.speed", "5");
		return (0);

	case PCE_KEY_6:
		trm_set_msg_emu (trm, "emu.cpu.speed", "6");
		return (0);

	case PCE_KEY_7:
		trm_set_msg_emu (trm, "emu.cpu.speed", "7");
		return (0);

	case PCE_KEY_8:
		trm_set_msg_emu (trm, "emu.cpu.speed", "8");
		return (0);


	case PCE_KEY_F:
		trm_set_msg_trm (trm, "term.fullscreen.toggle", "");
		return (0);

	case PCE_KEY_G:
		trm_set_msg_trm (trm, "term.grab", "");
		return (0);

	case PCE_KEY_M:
		trm_set_msg_trm (trm, "term.release", "");
		trm_set_msg_trm (trm, "term.fullscreen", "0");
		trm_set_msg_emu (trm, "emu.stop", "1");
		return (0);

	case PCE_KEY_P:
		trm_set_msg_emu (trm, "emu.pause.toggle", "");
		return (0);

	case PCE_KEY_Q:
		trm_set_msg_trm (trm, "term.release", "");
		trm_set_msg_trm (trm, "term.fullscreen", "0");
		trm_set_msg_emu (trm, "emu.exit", "1");
		return (0);

	case PCE_KEY_R:
		trm_set_msg_emu (trm, "emu.reset", "1");
		return (0);

	case PCE_KEY_S:
		trm_screenshot (trm, NULL);
		return (0);

	case PCE_KEY_UP:
	case PCE_KEY_KP_8:
		trm_set_scale (trm, trm->scale_x + 1, trm->scale_y + 1);
		return (0);

	case PCE_KEY_DOWN:
	case PCE_KEY_KP_2:
		if ((trm->scale_x > 1) && (trm->scale_y > 1)) {
			trm_set_scale (trm, trm->scale_x - 1, trm->scale_y - 1);
		}
		return (0);

	case PCE_KEY_LEFT:
	case PCE_KEY_KP_4:
		trm_set_msg_emu (trm, "emu.cpu.speed.step", "-1");
		return (0);

	case PCE_KEY_RIGHT:
	case PCE_KEY_KP_6:
		trm_set_msg_emu (trm, "emu.cpu.speed.step", "+1");
		return (0);

	default:
		return (1);
	}

	return (1);
}

static
void trm_set_key_emu (terminal_t *trm, unsigned event, pce_key_t key)
{
	if (event == PCE_KEY_EVENT_MAGIC) {
		if (trm_set_key_magic (trm, key) == 0) {
			return;
		}
	}

	if (trm->set_key != NULL) {
		trm->set_key (trm->set_key_ext, event, key);
	}
}

void trm_set_key (terminal_t *trm, unsigned event, pce_key_t key)
{
	if (event == PCE_KEY_EVENT_DOWN) {
		if (key == trm->escape_key) {
			trm->escape ^= TRM_ESC_ESC;
		}

		if (trm->escape & TRM_ESC_ESC) {
			if (key != trm->escape_key) {
				trm_set_key_emu (trm, PCE_KEY_EVENT_MAGIC, key);
			}
			return;
		}

		if (key == PCE_KEY_LALT) {
			trm->escape |= TRM_ESC_ALT;
		}
		else if (key == PCE_KEY_LCTRL) {
			trm->escape |= TRM_ESC_CTRL;
		}
		else if (key == PCE_KEY_PAUSE) {
			trm_set_msg_trm (trm, "term.release", "");
			trm_set_msg_trm (trm, "term.fullscreen", "0");
			trm_set_msg_emu (trm, "emu.exit", "1");
			return;
		}

		if ((trm->escape & TRM_ESC_KEYS) == TRM_ESC_KEYS) {
			trm_set_msg_trm (trm, "term.release", "");
			trm->escape &= ~TRM_ESC_KEYS;
		}
	}
	else if (event == PCE_KEY_EVENT_UP) {
		if (trm->escape & TRM_ESC_ESC) {
			if (key != trm->escape_key) {
				trm->escape &= ~TRM_ESC_ESC;
			}
			return;
		}

		if (key == PCE_KEY_LALT) {
			trm->escape &= ~TRM_ESC_ALT;
		}
		else if (key == PCE_KEY_LCTRL) {
			trm->escape &= ~TRM_ESC_CTRL;
		}
		else if (key == PCE_KEY_PAUSE) {
			return;
		}
	}

	trm_set_key_emu (trm, event, key);
}

void trm_set_mouse (terminal_t *trm, int dx, int dy, unsigned but)
{
	int tx, ty;

	dx = trm->mouse_scale_x[0] * dx + trm->mouse_scale_x[2];
	tx = dx;
	dx = dx / trm->mouse_scale_x[1];
	trm->mouse_scale_x[2] = tx - trm->mouse_scale_x[1] * dx;

	dy = trm->mouse_scale_y[0] * dy + trm->mouse_scale_y[2];
	ty = dy;
	dy = dy / trm->mouse_scale_y[1];
	trm->mouse_scale_y[2] = ty - trm->mouse_scale_y[1] * dy;

	if (trm->set_mouse != NULL) {
		trm->set_mouse (trm->set_mouse_ext, dx, dy, but);
	}
}

void trm_get_scale (terminal_t *trm, unsigned w, unsigned h, unsigned *fx, unsigned *fy)
{
	unsigned x, y;
	unsigned w2, h2;

	x = 1;
	y = 1;

	w2 = w;
	h2 = h;

	while ((8 * h2) < (3 * w2)) {
		y += 1;
		h2 += h;
	}

	while (w2 < h2) {
		x += 1;
		w2 += w;
	}

	if ((w2 <= 320) && (h2 <= 240)) {
		x *= 2;
		y *= 2;
	}

	*fx = x * trm->scale_x;
	*fy = y * trm->scale_y;
}

static
unsigned char *trm_scale_get_buf (terminal_t *trm, unsigned w, unsigned h)
{
	unsigned long cnt;
	unsigned char *tmp;

	cnt = 3 * (unsigned long) w * (unsigned long) h;

	if (cnt > trm->scale_buf_cnt) {
		tmp = realloc (trm->scale_buf, cnt);
		if (tmp == NULL) {
			return (NULL);
		}

		trm->scale_buf = tmp;
		trm->scale_buf_cnt = cnt;
	}

	return (trm->scale_buf);
}

const unsigned char *trm_scale (terminal_t *trm,
	const unsigned char *src, unsigned w, unsigned h,
	unsigned fx, unsigned fy)
{
	unsigned      i, j;
	unsigned      x, y;
	unsigned      dw, dh;
	unsigned char *dst, *ret;

	if ((fx == 1) && (fy == 1)) {
		return (src);
	}

	dw = fx * w;
	dh = fy * h;

	ret = trm_scale_get_buf (trm, dw, dh);
	if (ret == NULL) {
		return (NULL);
	}

	dst = ret;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			for (i = 0; i < fx; i++) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];

				dst += 3;
			}
			src += 3;
		}

		for (j = 1; j < fy; j++) {
			memcpy (dst, dst - 3 * dw, 3 * dw);
			dst += 3 * dw;
		}
	}

	return (ret);
}
