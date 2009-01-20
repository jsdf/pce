/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/slip.h                                           *
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


#ifndef PCE_DEVICES_SLIP_H
#define PCE_DEVICES_SLIP_H 1


#define PCE_SLIP_BUF_MAX 4096


/* a packet buffer */
typedef struct slip_buf_s {
	struct slip_buf_s *next;
	unsigned          i;
	unsigned          n;
	unsigned char     buf[PCE_SLIP_BUF_MAX];
} slip_buf_t;


typedef struct {
	unsigned      out_cnt;
	unsigned char out[PCE_SLIP_BUF_MAX];
	char          out_esc;

	/* input packet queue */
	slip_buf_t    *inp_hd;
	slip_buf_t    *inp_tl;
	unsigned      inp_cnt;

	int           tun_fd;

	char          checking;

	void *get_uint8_ext;
	int  (*get_uint8) (void *ext, unsigned char *val);

	void *set_uint8_ext;
	int  (*set_uint8) (void *ext, unsigned char val);
} slip_t;


void slip_init (slip_t *slip);
void slip_free (slip_t *slip);

slip_t *slip_new (void);
void slip_del (slip_t *slip);

void slip_set_set_uint8_fct (slip_t *slip, void *ext, void *fct);
void slip_set_get_uint8_fct (slip_t *slip, void *ext, void *fct);

int slip_set_tun (slip_t *slip, const char *name);

void slip_uart_check_out (slip_t *slip, unsigned char val);
void slip_uart_check_inp (slip_t *slip, unsigned char val);

void slip_clock (slip_t *slip, unsigned n);


#endif
