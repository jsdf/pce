/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/slip.c                                     *
 * Created:       2004-12-15 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-15 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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

/* $Id$ */


#include "main.h"

#ifdef PCE_ENABLE_TUN
#include <lib/tun.h>
#endif

#include "slip.h"


/* #define SLIP_DEBUG 1 */


static void slip_uart_check_out (slip_t *slip, unsigned char val);
static void slip_uart_check_inp (slip_t *slip, unsigned char val);


void slip_init (slip_t *slip)
{
  slip->ser = NULL;

  slip->buf_inp_i = 0;
  slip->buf_inp_n = 0;
  slip->buf_out_n = 0;

  slip->check_out = 0;
  slip->check_inp = 0;

  slip->tun_fd = -1;
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

void slip_set_serport (slip_t *slip, serport_t *ser)
{
  slip->ser = ser;

  e8250_set_send_f (&ser->uart, slip_uart_check_out, slip);
  e8250_set_recv_f (&ser->uart, slip_uart_check_inp, slip);
  e8250_set_setup_f (&ser->uart, NULL, NULL);
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
void slip_send_packet (slip_t *slip)
{
#ifdef PCE_ENABLE_TUN
  if (slip->tun_fd >= 0) {
#ifdef SLIP_DEBUG
    fprintf (stderr, "slip: send %u\n", slip->buf_out_n);
    fflush (stderr);
#endif

    tun_set_packet (slip->tun_fd, slip->buf_out, slip->buf_out_n);
  }
#endif
}

static
void slip_receive_packet (slip_t *slip)
{
#ifdef PCE_ENABLE_TUN
  unsigned      i, j, n;
  unsigned char buf[PCE_SLIP_BUF_MAX];

  slip->buf_inp_i = 0;
  slip->buf_inp_n = 0;

  if (tun_check_packet (slip->tun_fd) == 0) {
    return;
  }

  n = PCE_SLIP_BUF_MAX;

  if (tun_get_packet (slip->tun_fd, buf, &n)) {
    return;
  }

  j = 0;
  slip->buf_inp[j++] = 192;

  for (i = 0; i < n; i++) {
    if (buf[i] == 192) {
      if (j < PCE_SLIP_BUF_MAX) {
        slip->buf_inp[j++] = 219;
      }
      if (j < PCE_SLIP_BUF_MAX) {
        slip->buf_inp[j++] = 220;
      }
    }
    else if (buf[i] == 219) {
      if (j < PCE_SLIP_BUF_MAX) {
        slip->buf_inp[j++] = 219;
      }
      if (j < PCE_SLIP_BUF_MAX) {
        slip->buf_inp[j++] = 221;
      }
    }
    else {
      if (j < PCE_SLIP_BUF_MAX) {
        slip->buf_inp[j++] = buf[i];
      }
    }
  }

  if (j < PCE_SLIP_BUF_MAX) {
    slip->buf_inp[j++] = 192;
  }

#ifdef SLIP_DEBUG
  fprintf (stderr, "slip: recv %u / %u\n", n, j);
#endif

  slip->buf_inp_n = j;

  slip->check_inp = 1;
#else
  slip->buf_inp_i = 0;
  slip->buf_inp_n = 0;
#endif
}

static
void slip_set_out (slip_t *slip, unsigned char c)
{
  if (c == 192) {
    if (slip->buf_out_n > 0) {
      slip_send_packet (slip);
    }

    slip->buf_out_n = 0;
    slip->buf_out_esc = 0;

    return;
  }

  if (c == 219) {
    slip->buf_out_esc = 1;
    return;
  }

  if (slip->buf_out_esc) {
    slip->buf_out_esc = 0;

    if (c == 220) {
      c = 192;
    }
    else if (c == 221) {
      c = 219;
    }
    else {
      return;
    }
  }

  if (slip->buf_out_n >= PCE_SLIP_BUF_MAX) {
    return;
  }

  slip->buf_out[slip->buf_out_n++] = c;
}

static
void slip_uart_check_out (slip_t *slip, unsigned char val)
{
  slip->check_out = 1;
}

static
void slip_uart_check_inp (slip_t *slip, unsigned char val)
{
  slip->check_inp = 1;
}

void slip_clock (slip_t *slip, unsigned n)
{
  unsigned char c;

  if (slip->check_out) {
    slip->check_out = 0;

    while (1) {
      if (e8250_get_out (&slip->ser->uart, &c)) {
        break;
      }

      slip_set_out (slip, c);
    }
  }

  if (slip->check_inp) {
    slip->check_inp = 0;

    while (slip->buf_inp_i < slip->buf_inp_n) {
      if (e8250_receive (&slip->ser->uart, slip->buf_inp[slip->buf_inp_i])) {
        break;
      }

      slip->buf_inp_i += 1;
    }
  }

  if (slip->buf_inp_i >= slip->buf_inp_n) {
    slip_receive_packet (slip);
  }
}
