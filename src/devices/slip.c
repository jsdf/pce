/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/slip.c                                           *
 * Created:     2004-12-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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

#ifdef PCE_ENABLE_TUN
#include <lib/tun.h>
#endif

#include "slip.h"


/* #define SLIP_DEBUG 1 */

/* receive packet buffer */
#define SLIP_PACKET_BUFFER 8


void slip_init (slip_t *slip)
{
	slip->out_cnt = 0;
	slip->out_esc = 0;

	slip->inp_hd = NULL;
	slip->inp_tl = NULL;
	slip->inp_cnt = 0;

	slip->tun_fd = -1;

	slip->checking = 0;

	slip->get_uint8 = NULL;
	slip->get_uint8_ext = NULL;

	slip->set_uint8 = NULL;
	slip->set_uint8_ext = NULL;
}

void slip_free (slip_t *slip)
{
#ifdef PCE_ENABLE_TUN
	if (slip->tun_fd >= 0) {
		tun_close (slip->tun_fd);
	}
#endif
}

slip_t *slip_new (void)
{
	slip_t *slip;

	slip = malloc (sizeof (slip_t));
	if (slip == NULL) {
		return (NULL);
	}

	slip_init (slip);

	return (slip);
}

void slip_del (slip_t *slip)
{
	if (slip != NULL) {
		slip_free (slip);
		free (slip);
	}
}

void slip_set_set_uint8_fct (slip_t *slip, void *ext, void *fct)
{
	slip->set_uint8 = fct;
	slip->set_uint8_ext = ext;
}

void slip_set_get_uint8_fct (slip_t *slip, void *ext, void *fct)
{
	slip->get_uint8 = fct;
	slip->get_uint8_ext = ext;
}

int slip_set_tun (slip_t *slip, const char *name)
{
#ifdef PCE_ENABLE_TUN
	if (slip->tun_fd >= 0) {
		tun_close (slip->tun_fd);
	}

	slip->tun_fd = tun_open (name);
	if (slip->tun_fd < 0) {
		return (1);
	}

	return (0);
#else
	return (1);
#endif
}


static
int slip_get_uint8 (slip_t *slip, unsigned char *val)
{
	if (slip->get_uint8 != NULL) {
		return (slip->get_uint8 (slip->get_uint8_ext, val));
	}

	return (1);
}

static
int slip_set_uint8 (slip_t *slip, unsigned char val)
{
	if (slip->set_uint8 != NULL) {
		return (slip->set_uint8 (slip->set_uint8_ext, val));
	}

	return (1);
}


static
slip_buf_t *slip_buf_alloc (slip_t *slip)
{
	slip_buf_t *buf;

	buf = malloc (sizeof (slip_buf_t));
	if (buf == NULL) {
		return (0);
	}

	buf->next = NULL;
	buf->i = 0;
	buf->n = 0;

	return (buf);
}

static
void slip_buf_free (slip_t *slip, slip_buf_t *buf)
{
	free (buf);
}

static
void slip_send_packet (slip_t *slip)
{
#ifdef PCE_ENABLE_TUN
	if (slip->tun_fd >= 0) {
#ifdef SLIP_DEBUG
		fprintf (stderr, "slip: send tun S=%u\n", slip->out_cnt);
		fflush (stderr);
#endif

		if (tun_set_packet (slip->tun_fd, slip->out, slip->out_cnt)) {
			fprintf (stderr, "slip: packet drop\n");
		}
	}
#endif
}

static
int slip_receive_packet (slip_t *slip)
{
#ifdef PCE_ENABLE_TUN
	unsigned      i, j, n;
	unsigned char tmp[PCE_SLIP_BUF_MAX];
	slip_buf_t    *buf;

	if (tun_check_packet (slip->tun_fd) == 0) {
		return (1);
	}

	n = PCE_SLIP_BUF_MAX;

	if (tun_get_packet (slip->tun_fd, tmp, &n)) {
		return (1);
	}

	buf = slip_buf_alloc (slip);
	if (buf == NULL) {
		return (1);
	}

	j = 0;
	buf->buf[j++] = 192;

	for (i = 0; i < n; i++) {
		if (tmp[i] == 192) {
			if (j < PCE_SLIP_BUF_MAX) {
				buf->buf[j++] = 219;
			}
			if (j < PCE_SLIP_BUF_MAX) {
				buf->buf[j++] = 220;
			}
		}
		else if (tmp[i] == 219) {
			if (j < PCE_SLIP_BUF_MAX) {
				buf->buf[j++] = 219;
			}
			if (j < PCE_SLIP_BUF_MAX) {
				buf->buf[j++] = 221;
			}
		}
		else {
			if (j < PCE_SLIP_BUF_MAX) {
				buf->buf[j++] = tmp[i];
			}
		}
	}

	if (j < PCE_SLIP_BUF_MAX) {
		buf->buf[j++] = 192;
	}

#ifdef SLIP_DEBUG
	fprintf (stderr, "slip: recv tun S=%u+%u Q=%u\n",
		n, j - n, slip->inp_cnt + 1
	);
#endif

	buf->n = j;

	if (slip->inp_hd == NULL) {
		slip->inp_hd = buf;
	}
	else {
		slip->inp_tl->next = buf;
	}

	slip->inp_tl = buf;
	slip->inp_cnt += 1;

	return (0);
#else
	return (1);
#endif
}

static
void slip_set_out (slip_t *slip, unsigned char c)
{
	if (c == 192) {
		if (slip->out_cnt > 0) {
			slip_send_packet (slip);
		}

		slip->out_cnt = 0;
		slip->out_esc = 0;

		return;
	}

	if (slip->out_esc) {
		slip->out_esc = 0;

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
			slip->out_esc = 1;
			return;
		}
	}

	if (slip->out_cnt >= PCE_SLIP_BUF_MAX) {
		fprintf (stderr, "slip: send buffer overrun\n");
		return;
	}

	slip->out[slip->out_cnt++] = c;
}

void slip_uart_check_out (slip_t *slip, unsigned char val)
{
	unsigned char c;

	if (slip->checking) {
		return;
	}

	slip->checking = 1;

	while (1) {
		if (slip_get_uint8 (slip, &c)) {
			break;
		}

		slip_set_out (slip, c);
	}

	slip->checking = 0;
}

void slip_uart_check_inp (slip_t *slip, unsigned char val)
{
	if (slip->checking) {
		return;
	}

	slip->checking = 1;

	while (slip->inp_hd != NULL) {
		slip_buf_t *buf;

		buf = slip->inp_hd;

		while (buf->i < buf->n) {
			if (slip_set_uint8 (slip, buf->buf[buf->i])) {
				break;
			}

			buf->i += 1;
		}

		if (buf->i < buf->n) {
			break;
		}

#ifdef SLIP_DEBUG
		fprintf (stderr, "slip: recv uart S=%u Q=%u\n",
			buf->n, slip->inp_cnt - 1
		);
#endif

		if (buf->next == NULL) {
			slip->inp_hd = NULL;
			slip->inp_tl = NULL;
		}
		else {
			slip->inp_hd = buf->next;
		}

		slip_buf_free (slip, buf);

		slip->inp_cnt -= 1;
	}

	while (slip->inp_cnt < SLIP_PACKET_BUFFER) {
		if (slip_receive_packet (slip)) {
			break;
		}
	}

	slip->checking = 0;
}

void slip_clock (slip_t *slip, unsigned n)
{
	slip_uart_check_inp (slip, 1);
	slip_uart_check_out (slip, 1);
}
