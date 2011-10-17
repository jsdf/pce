/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-mouse.c                                *
 * Created:     2011-10-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011 Hampa Hug <hampa@hampa.ch>                          *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-mouse.h>


#define MOUSE_MAX 8


static unsigned     mouse_cnt = 0;
static char_mouse_t *mouse[MOUSE_MAX];


static
void chr_mouse_close (char_drv_t *cdrv)
{
	unsigned     i, n;
	char_mouse_t *drv;

	drv = cdrv->ext;

	n = 0;

	for (i = 0; i < mouse_cnt; i++) {
		if (mouse[i] != drv) {
			mouse[n++] = mouse[i];
		}
	}

	mouse_cnt = n;

	free (drv);
}

static
void chr_mouse_add_packet (char_mouse_t *drv)
{
	unsigned char val;
	int           dx, dy;
	unsigned char x, y;

	val = (drv->buf_hd - drv->buf_tl + CHAR_MOUSE_BUF) % CHAR_MOUSE_BUF;

	if ((val + 3) >= CHAR_MOUSE_BUF) {
		return;
	}

	dx = drv->dx;

	if (dx < -127) {
		dx = -127;
	}
	else if (dx > 127) {
		dx = 127;
	}

	dy = drv->dy;

	if (dy < -127) {
		dy = -127;
	}
	else if (dy > 127) {
		dy = 127;
	}

	if (dx < 0) {
		x = -dx;
		x = (~x + 1) & 0xff;
	}
	else {
		x = dx;
	}

	if (dy < 0) {
		y = -dy;
		y = (~y + 1) & 0xff;
	}
	else {
		y = dy;
	}

	val = 0x40;
	val |= (drv->button & 0x01) ? 0x20 : 0x00;
	val |= (drv->button & 0x02) ? 0x10 : 0x00;
	val |= (y >> 4) & 0x0c;
	val |= (x >> 6) & 0x03;

	drv->buf[drv->buf_hd] = val;
	drv->buf_hd = (drv->buf_hd + 1) % CHAR_MOUSE_BUF;

	drv->buf[drv->buf_hd] = x & 0x3f;
	drv->buf_hd = (drv->buf_hd + 1) % CHAR_MOUSE_BUF;

	drv->buf[drv->buf_hd] = y & 0x3f;
	drv->buf_hd = (drv->buf_hd + 1) % CHAR_MOUSE_BUF;

	drv->dx -= dx;
	drv->dy -= dy;
}

static
void chr_mouse_set_drv (char_mouse_t *drv, int dx, int dy, unsigned button)
{
	int      tx, ty;
	unsigned tb;

	tb = drv->button;

	dx = drv->scale_x[0] * dx + drv->scale_x[2];
	tx = dx;
	dx = dx / drv->scale_x[1];
	drv->scale_x[2] = tx - drv->scale_x[1] * dx;

	dy = drv->scale_y[0] * dy + drv->scale_y[2];
	ty = dy;
	dy = dy / drv->scale_y[1];
	drv->scale_y[2] = ty - drv->scale_y[1] * dy;

	drv->dx += dx;
	drv->dy += dy;
	drv->button = button;

	if ((tb ^ button) & 3) {
		chr_mouse_add_packet (drv);
	}
}

static
unsigned chr_mouse_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	unsigned      i, n;
	unsigned char *tmp;
	char_mouse_t  *drv;

	drv = cdrv->ext;
	tmp = buf;

	if (drv->buf_hd == drv->buf_tl) {
		if ((drv->dx != 0) || (drv->dy != 0)) {
			chr_mouse_add_packet (drv);
		}
	}

	if (drv->buf_hd == drv->buf_tl) {
		return (0);
	}

	n = (drv->buf_hd - drv->buf_tl + CHAR_MOUSE_BUF) % CHAR_MOUSE_BUF;

	if (n > cnt) {
		n = cnt;
	}

	for (i = 0; i < n; i++) {
		tmp[i] = drv->buf[drv->buf_tl];
		drv->buf_tl = (drv->buf_tl + 1) % CHAR_MOUSE_BUF;
	}

	return (n);
}

static
unsigned chr_mouse_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	return (cnt);
}

static
int chr_mouse_get_ctl (char_drv_t *cdrv, unsigned *ctl)
{
	*ctl = PCE_CHAR_DSR | PCE_CHAR_CTS | PCE_CHAR_CD;

	return (0);
}

static
int chr_mouse_set_ctl (char_drv_t *cdrv, unsigned ctl)
{
	int          dtr, rts;
	char_mouse_t *drv;

	drv = cdrv->ext;

	dtr = (ctl & PCE_CHAR_DTR) != 0;
	rts = (ctl & PCE_CHAR_RTS) != 0;

	if (rts && dtr) {
		if ((drv->dtr == 0) || (drv->rts == 0)) {
			drv->dx = 0;
			drv->dy = 0;
			drv->scale_x[2] = 0;
			drv->scale_y[2] = 0;

			drv->buf_hd = 1;
			drv->buf_tl = 0;
			drv->buf[0] = 'M';
		}
	}

	drv->dtr = dtr;
	drv->rts = rts;

	return (0);
}

static
int chr_mouse_set_params (char_drv_t *cdrv, unsigned long bps, unsigned bpc, unsigned parity, unsigned stop)
{
	return (0);
}

static
int chr_mouse_init (char_mouse_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_mouse_close;
	drv->cdrv.read = chr_mouse_read;
	drv->cdrv.write = chr_mouse_write;
	drv->cdrv.get_ctl = chr_mouse_get_ctl;
	drv->cdrv.set_ctl = chr_mouse_set_ctl;
	drv->cdrv.set_params = chr_mouse_set_params;

	drv->buf_hd = 0;
	drv->buf_tl = 0;

	drv->dtr = 0;
	drv->rts = 0;

	drv->dx = 0;
	drv->dy = 0;
	drv->button = 0;

	drv->scale_x[0] = drv_get_option_sint (name, "xmul", 1);
	drv->scale_x[1] = drv_get_option_sint (name, "xdiv", 1);
	drv->scale_x[2] = 0;

	if (drv->scale_x[1] == 0) {
		drv->scale_x[1] = 1;
	}

	drv->scale_y[0] = drv_get_option_sint (name, "ymul", 1);
	drv->scale_y[1] = drv_get_option_sint (name, "ydiv", 1);
	drv->scale_y[2] = 0;

	if (drv->scale_y[1] == 0) {
		drv->scale_y[1] = 1;
	}

	if ((mouse_cnt + 1) < MOUSE_MAX) {
		mouse[mouse_cnt++] = drv;
	}

	return (0);
}

void chr_mouse_set (int dx, int dy, unsigned button)
{
	unsigned i;

	for (i = 0; i < mouse_cnt; i++) {
		chr_mouse_set_drv (mouse[i], dx, dy, button);
	}
}

char_drv_t *chr_mouse_open (const char *name)
{
	char_mouse_t *drv;

	drv = malloc (sizeof (char_mouse_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_mouse_init (drv, name)) {
		free (drv);
		return (NULL);
	}

	return (&drv->cdrv);
}
