/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/arch/sim405/slip.h                                     *
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


#ifndef PCE_DEVICES_SLIP_H
#define PCE_DEVICES_SLIP_H 1


#include <devices/serport.h>


#define PCE_SLIP_BUF_MAX 4096


typedef struct {
  serport_t     *ser;

  unsigned      buf_out_n;
  unsigned char buf_out[PCE_SLIP_BUF_MAX];
  char          buf_out_esc;

  unsigned      buf_inp_i;
  unsigned      buf_inp_n;
  unsigned char buf_inp[PCE_SLIP_BUF_MAX];

  int           check_out;
  int           check_inp;

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
