/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-ppp.h                                  *
 * Created:     2009-10-22 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DRIVERS_CHAR_PPP_H
#define PCE_DRIVERS_CHAR_PPP_H 1


#include <stdio.h>

#include <drivers/char/char.h>

#define PPP_MAX_MTU 4096


typedef struct ppp_packet_s {
	struct ppp_packet_s *next;

	int                 started;
	int                 nocfgopt;

	unsigned            idx;
	unsigned            cnt;
	unsigned            max;
	unsigned char       *data;
} ppp_packet_t;


typedef struct ppp_cp_s {
	struct char_ppp_s *drv;

	unsigned          state;
	unsigned short    protocol;
	const char        *name;

	unsigned          counter;
	unsigned char     current_id;

	unsigned char     opt_state[32];

	void     (*reset) (struct ppp_cp_s *cp);

	void     (*send_config_request) (struct ppp_cp_s *cp);

	unsigned (*options_rej) (struct ppp_cp_s *cp, unsigned char *buf, unsigned cnt);
	unsigned (*options_nak) (struct ppp_cp_s *cp, unsigned char *buf, unsigned cnt);
	unsigned (*options_ack) (struct ppp_cp_s *cp, unsigned char *buf, unsigned cnt);

	void     (*config_rej) (struct ppp_cp_s *cp, unsigned char *buf, unsigned cnt);
	void     (*config_nak) (struct ppp_cp_s *cp, unsigned char *buf, unsigned cnt);
	void     (*config_ack) (struct ppp_cp_s *cp, unsigned char *buf, unsigned cnt);
} ppp_cp_t;


typedef struct char_ppp_s {
	char_drv_t cdrv;

	unsigned       ser_out_idx;
	unsigned       ser_out_cnt;
	unsigned       ser_out_max;
	unsigned char  ser_out[PPP_MAX_MTU + 6];

	unsigned       ser_inp_cnt;
	unsigned       ser_inp_max;
	int            ser_inp_esc;
	unsigned char  ser_inp[PPP_MAX_MTU + 6];

	ppp_packet_t   *ser_pk_hd;
	ppp_packet_t   *ser_pk_tl;

	int            crc16_ok;
	unsigned short crc16[256];

	ppp_cp_t       lcp;
	ppp_cp_t       ipcp;

	unsigned long  accm_send;
	unsigned long  accm_recv;

	unsigned       mru_send;
	unsigned       mru_recv;

	unsigned char  ip_local[4];
	unsigned char  ip_remote[4];

	unsigned long  magic_send;
	unsigned long  magic_recv;

	unsigned char  lcp_send_id;
	unsigned       lcp_counter;

	unsigned char  ipcp_send_id;
	unsigned       ipcp_counter;

	char           *tun_name;
	int            tun_fd;
} char_ppp_t;


#endif
