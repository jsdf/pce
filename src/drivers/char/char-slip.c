/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-slip.c                                 *
 * Created:     2009-03-10 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2010 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef PCE_ENABLE_TUN
#include <lib/tun.h>
#endif

#include <drivers/options.h>
#include <drivers/char/char.h>
#include <drivers/char/char-slip.h>


#ifdef PCE_ENABLE_TUN

static
void slip_send_packet (char_slip_t *drv)
{
	if (drv->tun_fd < 0) {
		return;
	}

	if (tun_set_packet (drv->tun_fd, drv->out, drv->out_cnt)) {
		fprintf (stderr, "slip: packet drop\n");
	}
}

static
int slip_receive_packet (char_slip_t *drv)
{
	unsigned      i, j, n;
	unsigned char tmp[SLIP_BUF_MAX];
	unsigned char *buf;

	if (drv->tun_fd < 0) {
		return (1);
	}

	if (tun_check_packet (drv->tun_fd) == 0) {
		return (1);
	}

	n = SLIP_BUF_MAX;

	if (tun_get_packet (drv->tun_fd, tmp, &n)) {
		return (1);
	}

	buf = drv->inp;

	j = 0;
	buf[j++] = 192;

	for (i = 0; i < n; i++) {
		if (tmp[i] == 192) {
			if (j < SLIP_BUF_MAX) {
				buf[j++] = 219;
			}
			if (j < SLIP_BUF_MAX) {
				buf[j++] = 220;
			}
		}
		else if (tmp[i] == 219) {
			if (j < SLIP_BUF_MAX) {
				buf[j++] = 219;
			}
			if (j < SLIP_BUF_MAX) {
				buf[j++] = 221;
			}
		}
		else {
			if (j < SLIP_BUF_MAX) {
				buf[j++] = tmp[i];
			}
		}
	}

	if (j < SLIP_BUF_MAX) {
		buf[j++] = 192;
	}

	drv->inp_idx = 0;
	drv->inp_cnt = j;

	return (0);
}

#else

static
void slip_send_packet (char_slip_t *drv)
{
}

static
int slip_receive_packet (char_slip_t *drv)
{
	return (1);
}

#endif


static
void chr_slip_close (char_drv_t *cdrv)
{
	char_slip_t *drv;

	drv = cdrv->ext;

	if (drv->tun_fd >= 0) {
		tun_close (drv->tun_fd);
	}

	if (drv->tun_name != NULL) {
		free (drv->tun_name);
	}

	free (drv);
}

static
unsigned chr_slip_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	unsigned    n;
	char_slip_t *drv;

	drv = cdrv->ext;

	if (drv->inp_idx >= drv->inp_cnt) {
		if (slip_receive_packet (drv)) {
			return (0);
		}
	}

	n = drv->inp_cnt - drv->inp_idx;

	if (n > cnt) {
		n = cnt;
	}

	if (n > 0) {
		memcpy (buf, drv->inp + drv->inp_idx, n);
	}

	drv->inp_idx += n;

	return (n);
}

static
void slip_write_char (char_slip_t *drv, unsigned char c)
{
	if (c == 192) {
		if (drv->out_cnt > 0) {
			slip_send_packet (drv);
		}

		drv->out_cnt = 0;
		drv->out_esc = 0;

		return;
	}

	if (drv->out_esc) {
		drv->out_esc = 0;

		if (c == 220) {
			c = 192;
		}
		else if (c == 221) {
			c = 219;
		}
		else {
			fprintf (stderr, "slip: unknown escape (%02X)\n",
				(unsigned) c
			);
			return;
		}
	}
	else {
		if (c == 219) {
			drv->out_esc = 1;
			return;
		}
	}

	if (drv->out_cnt >= SLIP_BUF_MAX) {
		fprintf (stderr, "slip: send buffer overrun\n");
		return;
	}

	drv->out[drv->out_cnt++] = c;
}

static
unsigned chr_slip_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tmp;
	char_slip_t         *drv;

	drv = cdrv->ext;

	tmp = buf;

	for (i = 0; i < cnt; i++) {
		slip_write_char (drv, tmp[i]);
	}

	return (cnt);
}

static
int chr_slip_init (char_slip_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_slip_close;
	drv->cdrv.read = chr_slip_read;
	drv->cdrv.write = chr_slip_write;

	drv->out_cnt = 0;
	drv->out_esc = 0;

	drv->inp_idx = 0;
	drv->inp_cnt = 0;

	drv->tun_name = NULL;
	drv->tun_fd = -1;

	drv->tun_name = drv_get_option (name, "if");

	if (drv->tun_name == NULL) {
		return (1);
	}

	drv->tun_fd = tun_open (drv->tun_name);

	if (drv->tun_fd < 0) {
		return (1);
	}

	return (0);
}

char_drv_t *chr_slip_open (const char *name)
{
	char_slip_t *drv;

	drv = malloc (sizeof (char_slip_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_slip_init (drv, name)) {
		free (drv);
		return (NULL);
	}

	return (&drv->cdrv);
}
