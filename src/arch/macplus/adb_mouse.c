/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/adb_mouse.c                                 *
 * Created:     2010-11-04 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include "adb.h"
#include "adb_mouse.h"

#include <stdlib.h>


static
void adb_mouse_del (adb_dev_t *dev)
{
	free (dev->ext);
}

static
void adb_mouse_reset (adb_dev_t *dev)
{
	adb_mouse_t *mse;

	mse = dev->ext;

	adb_dev_reset (dev);

	mse->change = 0;

	mse->button = 0;
	mse->dx = 0;
	mse->dy = 0;

	mse->talk_button = 0;
	mse->talk_dx = 0;
	mse->talk_dy = 0;

	dev->reg[0] = 0x8080;
}

static
void adb_mouse_flush (adb_dev_t *dev)
{
	adb_mouse_t *mse;

	mse = dev->ext;

	adb_dev_flush (dev);

	mse->change = 0;

	mse->button = 0;
	mse->dx = 0;
	mse->dy = 0;

	mse->talk_button = 0;
	mse->talk_dx = 0;
	mse->talk_dy = 0;
}

static
unsigned adb_mouse_get_val (int val)
{
	unsigned r;

	if (val < 0) {
		if (val < -63) {
			val = -63;
		}

		r = -val;
		r = (~r + 1) & 0x7f;
	}
	else {
		if (val > 63) {
			val = 63;
		}

		r = val;
	}

	return (r);
}

static
unsigned adb_mouse_talk_0 (adb_dev_t *dev, void *buf)
{
	unsigned      v;
	adb_mouse_t   *mse;
	unsigned char *dst;

	mse = dev->ext;
	dst = buf;

	if (mse->change == 0) {
		return (0);
	}

	mse->talking = 1;

	mse->talk_button = mse->button;
	mse->talk_dx = mse->dx;
	mse->talk_dy = mse->dy;

	dev->reg[0] = 0x8080;

	if (mse->talk_button & 1) {
		dev->reg[0] &= 0x7fff;
	}

	v = adb_mouse_get_val (mse->talk_dx);
	dev->reg[0] = (dev->reg[0] & 0xff80) | v;

	v = adb_mouse_get_val (mse->talk_dy);
	dev->reg[0] = (dev->reg[0] & 0x80ff) | (v << 8);

	dst[0] = (dev->reg[0] >> 8) & 0xff;
	dst[1] = dev->reg[0] & 0xff;

	return (2);
}

static
unsigned adb_mouse_talk (adb_dev_t *dev, unsigned reg, void *buf)
{
	if (reg == 0) {
		return (adb_mouse_talk_0 (dev, buf));
	}

	return (adb_dev_talk (dev, reg, buf));
}

static
void adb_mouse_talk_done (adb_dev_t *dev, unsigned reg)
{
	adb_mouse_t *mse;

	mse = dev->ext;

	if (reg == 0) {
		mse->talking = 0;

		mse->change = 0;

		if (mse->talk_button != mse->button) {
			mse->change = 1;
		}

		mse->dx -= mse->talk_dx;
		mse->dy -= mse->talk_dy;

		if ((mse->dx != 0) || (mse->dy != 0)) {
			mse->change = 1;
		}

		dev->service_request = mse->change;
	}
}

void adb_mouse_move (adb_mouse_t *mse, unsigned button, int dx, int dy)
{
	if ((mse->button ^ button) & 1) {
		mse->change = 1;
	}

	mse->button = button;

	mse->dx += dx;
	mse->dy += dy;

	if ((mse->dx != 0) || (mse->dy != 0)) {
		mse->change = 1;
	}

	mse->dev.service_request = mse->change;
}

void adb_mouse_init (adb_mouse_t *mse)
{
	adb_dev_init (&mse->dev, 3, 1);

	mse->dev.ext = mse;

	mse->dev.del = adb_mouse_del;

	mse->dev.reset = adb_mouse_reset;
	mse->dev.flush = adb_mouse_flush;
	mse->dev.talk = adb_mouse_talk;
	mse->dev.talk_done = adb_mouse_talk_done;

	mse->change = 0;

	mse->button = 0;
	mse->dx = 0;
	mse->dy = 0;

	mse->talk_button = 0;
	mse->talk_dx = 0;
	mse->talk_dy = 0;
}

adb_mouse_t *adb_mouse_new (void)
{
	adb_mouse_t *mse;

	mse = malloc (sizeof (adb_mouse_t));

	if (mse == NULL) {
		return (NULL);
	}

	adb_mouse_init (mse);

	return (mse);
}
