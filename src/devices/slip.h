/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/slip.h                                         *
 * Created:       2004-12-15 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2005-12-08 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004-2005 Hampa Hug <hampa@hampa.ch>                   *
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


#ifndef PCE_DEVICES_SLIP_H
#define PCE_DEVICES_SLIP_H 1


#include <devices/serport.h>


#define PCE_SLIP_BUF_MAX 4096


/* a packet buffer */
typedef struct slip_buf_s {
  struct slip_buf_s *next;
  unsigned          i;
  unsigned          n;
  unsigned char     buf[PCE_SLIP_BUF_MAX];
} slip_buf_t;


typedef struct {
  serport_t     *ser;

  unsigned      out_cnt;
  unsigned char out[PCE_SLIP_BUF_MAX];
  char          out_esc;

  /* input packet queue */
  slip_buf_t    *inp_hd;
  slip_buf_t    *inp_tl;
  unsigned      inp_cnt;

  int           tun_fd;
} slip_t;


void slip_init (slip_t *slip);
void slip_free (slip_t *slip);

slip_t *slip_new (void);
void slip_del (slip_t *slip);

void slip_set_serport (slip_t *slip, serport_t *ser);
int slip_set_tun (slip_t *slip, const char *name);

void slip_clock (slip_t *slip, unsigned n);


#endif
