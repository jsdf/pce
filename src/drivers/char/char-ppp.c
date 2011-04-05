/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/char/char-ppp.c                                  *
 * Created:     2009-10-22 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2009-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include <drivers/char/char-ppp.h>


#define PPP_STATE_INITIAL  0
#define PPP_STATE_STARTING 1
#define PPP_STATE_CLOSED   2
#define PPP_STATE_STOPPED  3
#define PPP_STATE_CLOSING  4
#define PPP_STATE_STOPPING 5
#define PPP_STATE_REQSENT  6
#define PPP_STATE_ACKRECV  7
#define PPP_STATE_ACKSENT  8
#define PPP_STATE_OPENED   9

#define PPP_PROTO_IP   0x0021
#define PPP_PROTO_IPCP 0x8021
#define PPP_PROTO_LCP  0xc021

#define LCP_OPT_MRU   1
#define LCP_OPT_ACCM  2
#define LCP_OPT_MAGIC 5

#define IPCP_OPT_ADDR 3

/* #define DEBUG_PPP 1 */


static void cp_send_config_request (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt);


static
int chr_ppp_get_option_ip (const char *str, const char *name, unsigned char *val)
{
	unsigned i, j;
	char     *s;

	s = drv_get_option (str, name);

	if (s == NULL) {
		return (1);
	}

	val[0] = 0;
	val[1] = 0;
	val[2] = 0;
	val[3] = 0;

	i = 0;
	j = 0;

	while (s[j] != 0) {
		if ((s[j] >= '0') && (s[j] <= '9')) {
			val[i] = 10 * val[i] + (s[j] - '0');
		}
		else if (s[j] == '.') {
			i += 1;

			if (i >= 4) {
				break;
			}
		}
		else {
			break;
		}

		j += 1;
	}

	if (s[j] != 0) {
		free (s);
		return (1);
	}

	free (s);

	return (0);
}

static
unsigned long ppp_get_uint32_be (const void *buf, unsigned i)
{
	unsigned long       v;
	const unsigned char *tmp;

	tmp = (const unsigned char *) buf + i;

	v = tmp[0] & 0xff;
	v = (v << 8) | (tmp[1] & 0xff);
	v = (v << 8) | (tmp[2] & 0xff);
	v = (v << 8) | (tmp[3] & 0xff);

	return (v);
}

static
void ppp_set_uint32_be (void *buf, unsigned i, unsigned long v)
{
	unsigned char *tmp;

	tmp = (unsigned char *) buf + i;

	tmp[0] = (v >> 24) & 0xff;
	tmp[1] = (v >> 16) & 0xff;
	tmp[2] = (v >> 8) & 0xff;
	tmp[3] = v & 0xff;
}

static
const unsigned short *ppp_crc16_get_tab (char_ppp_t *drv)
{
	unsigned       i, j;
	unsigned short v;

	if (drv->crc16_ok) {
		return (drv->crc16);
	}

	for (i = 0; i < 256; i++) {
		v = i;

		for (j = 0; j < 8; j++) {
			if (v & 1) {
				v = (v >> 1) ^ 0x8408;
			}
			else {
				v = (v >> 1);
			}
		}

		drv->crc16[i] = v;
	}

	drv->crc16_ok = 1;

	return (drv->crc16);
}

static
unsigned short ppp_crc16 (char_ppp_t *drv, const unsigned char *buf, unsigned cnt)
{
	unsigned short       crc;
	const unsigned short *tab;

	tab = ppp_crc16_get_tab (drv);

	crc = 0xffff;

	while (cnt > 0) {
		crc = (crc >> 8) ^ tab[(crc ^ *buf) & 0xff];

		cnt -= 1;
		buf += 1;
	}

	return (~crc & 0xffff);
}

static
ppp_packet_t *ppp_packet_alloc (char_ppp_t *drv, unsigned size)
{
	ppp_packet_t *pk;

	pk = malloc (sizeof (ppp_packet_t));

	if (pk == NULL) {
		return (NULL);
	}

	pk->next = NULL;

	pk->started = 0;
	pk->nocfgopt = 0;

	pk->idx = 4;
	pk->cnt = size + 4;
	pk->max = size + 6;

	pk->data = malloc (pk->max);

	if (pk->data == NULL) {
		free (pk);
		return (NULL);
	}

	return (pk);
}

static
void ppp_packet_free (char_ppp_t *drv, ppp_packet_t *pk)
{
	free (pk->data);
	free (pk);
}

/*
 * Add the PPP header and CRC to a packet and add it to the output queue.
 */
static
void ppp_send (char_ppp_t *drv, ppp_packet_t *pk, unsigned proto)
{
	unsigned short crc;

	pk->started = 0;

	pk->data[0] = 0xff;
	pk->data[1] = 0x03;
	pk->data[2] = (proto >> 8) & 0xff;
	pk->data[3] = proto & 0xff;

	pk->idx = 0;

	if ((pk->cnt - 4) > drv->mru_send) {
		fprintf (stderr, "ppp: truncate packet (%u -> %u)\n",
			pk->cnt - 4, drv->mru_send
		);

		pk->cnt = drv->mru_send + 4;
	}

	if ((pk->cnt + 2) > pk->max) {
		return;
	}

	crc = ppp_crc16 (drv, pk->data, pk->cnt);

	pk->data[pk->cnt + 0] = crc & 0xff;
	pk->data[pk->cnt + 1] = (crc >> 8) & 0xff;

	pk->cnt += 2;

	if (drv->ser_pk_tl == NULL) {
		drv->ser_pk_hd = pk;
	}
	else {
		drv->ser_pk_tl->next = pk;
	}

	drv->ser_pk_tl = pk;

#ifdef DEBUG_PPP
	fprintf (stderr, "ppp: send packet (n=%u, addr=%02X, ctrl=%02X, proto=%04X, crc=%04X)\n",
		pk->cnt, 0xff, 0x03, proto, crc
	);
#endif
}


#ifdef PCE_ENABLE_TUN

static
void tun_send (char_ppp_t *drv, const unsigned char *buf, unsigned cnt)
{
	if (drv->tun_fd < 0) {
		return;
	}

	tun_set_packet (drv->tun_fd, buf, cnt);
}

static
void tun_receive (char_ppp_t *drv)
{
	unsigned cnt;

	ppp_packet_t  *pk;

	if (drv->tun_fd < 0) {
		return;
	}

	if (tun_check_packet (drv->tun_fd) == 0) {
		return;
	}

	pk = ppp_packet_alloc (drv, PPP_MAX_MTU);

	if (pk == NULL) {
		return;
	}

	cnt = pk->cnt - pk->idx;

	if (tun_get_packet (drv->tun_fd, pk->data + pk->idx, &cnt)) {
		ppp_packet_free (drv, pk);
		return;
	}

	pk->cnt = pk->idx + cnt;

	if (drv->ipcp.state != PPP_STATE_OPENED) {
		ppp_packet_free (drv, pk);
		return;
	}

	ppp_send (drv, pk, PPP_PROTO_IP);
}

#else

static
void tun_send (char_ppp_t *drv, const unsigned char *buf, unsigned cnt)
{
}

static
void tun_receive (char_ppp_t *drv)
{
}

#endif


static
void ipcp_reset (ppp_cp_t *cp)
{
	unsigned i;

	cp->state = PPP_STATE_STOPPED;

	for (i = 0; i < 32; i++) {
		cp->opt_state[i] = 0;
	}
}

static
void ipcp_send_config_request (ppp_cp_t *cp)
{
	unsigned      i;
	unsigned char buf[64];

	i = 0;

	if (cp->opt_state[IPCP_OPT_ADDR] == 0) {
		buf[i++] = IPCP_OPT_ADDR;
		buf[i++] = 6;
		buf[i++] = cp->drv->ip_local[0];
		buf[i++] = cp->drv->ip_local[1];
		buf[i++] = cp->drv->ip_local[2];
		buf[i++] = cp->drv->ip_local[3];
	}

	cp_send_config_request (cp, buf, i);

	cp->counter = 4;
}

static
int ipcp_compare_ip (const unsigned char *ip1, const unsigned char *ip2)
{
	unsigned i;

	for (i = 0; i < 4; i++) {
		if (ip1[i] != ip2[i]) {
			return (1);
		}
	}

	return (0);
}

static
unsigned ipcp_options_rej (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i, j, k;
	unsigned char type, length;

	i = 0;
	j = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		if ((type == IPCP_OPT_ADDR) && (length >= 6)) {
			;
		}
		else {
			for (k = 0; k < length; k++) {
				buf[j++] = buf[i + k];
			}
		}

		i += length;
	}

	return (j);
}

static
unsigned ipcp_options_nak (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i, j, k;
	unsigned char type, length;
	int           nak;

	i = 0;
	j = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		nak = 0;

		if (type == IPCP_OPT_ADDR) {
			if (length > 6) {
				nak = 1;
			}
			else if (ipcp_compare_ip (cp->drv->ip_remote, buf + i + 2) != 0) {
				nak = 1;
			}

			if (nak) {
				buf[j++] = type;
				buf[j++] = 6;
				buf[j++] = cp->drv->ip_remote[0];
				buf[j++] = cp->drv->ip_remote[1];
				buf[j++] = cp->drv->ip_remote[2];
				buf[j++] = cp->drv->ip_remote[3];
			}
		}
		else {
			for (k = 0; k < length; k++) {
				buf[j++] = buf[i + k];
			}
		}

		i += length;
	}

	return (j);
}

static
unsigned ipcp_options_ack (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	return (cnt);
}

static
void ipcp_config_rej (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i;
	unsigned char type, length;

	i = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		if ((length < 2) || ((i + length) > cnt)) {
			return;
		}

		if ((type == IPCP_OPT_ADDR) && (length >= 6)) {
			cp->drv->ip_local[0] = 0;
			cp->drv->ip_local[1] = 0;
			cp->drv->ip_local[2] = 0;
			cp->drv->ip_local[3] = 0;
			cp->opt_state[IPCP_OPT_ADDR] = 1;
		}

		i += length;
	}
}

static
void ipcp_config_nak (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i;
	unsigned char type, length;

	i = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		if ((length < 2) || ((i + length) > cnt)) {
			return;
		}

		if ((type == IPCP_OPT_ADDR) && (length >= 6)) {
			cp->drv->ip_local[0] = buf[i + 2];
			cp->drv->ip_local[1] = buf[i + 3];
			cp->drv->ip_local[2] = buf[i + 4];
			cp->drv->ip_local[3] = buf[i + 5];
			cp->opt_state[IPCP_OPT_ADDR] = 0;
		}

		i += length;
	}
}

static
void ipcp_config_ack (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "ipcp: local ip = %u.%u.%u.%u\n",
		cp->drv->ip_local[0],
		cp->drv->ip_local[1],
		cp->drv->ip_local[2],
		cp->drv->ip_local[3]
	);

	fprintf (stderr, "ipcp: remote ip = %u.%u.%u.%u\n",
		cp->drv->ip_remote[0],
		cp->drv->ip_remote[1],
		cp->drv->ip_remote[2],
		cp->drv->ip_remote[3]
	);
#endif
}

static
void ipcp_init (char_ppp_t *drv, ppp_cp_t *cp)
{
	cp->drv = drv;

	cp->state = PPP_STATE_STOPPED;
	cp->protocol = PPP_PROTO_IPCP;
	cp->name = "ipcp";

	cp->counter = 0;
	cp->current_id = 0;

	cp->reset = ipcp_reset;

	cp->send_config_request = ipcp_send_config_request;

	cp->options_rej = ipcp_options_rej;
	cp->options_nak = ipcp_options_nak;
	cp->options_ack = ipcp_options_ack;

	cp->config_rej = ipcp_config_rej;
	cp->config_nak = ipcp_config_nak;
	cp->config_ack = ipcp_config_ack;
}


static
void lcp_reset (ppp_cp_t *cp)
{
	unsigned i;

	cp->state = PPP_STATE_STOPPED;

	cp->drv->accm_send = 0xffffffff;
	cp->drv->accm_recv = 0x00000000;

	for (i = 0; i < 32; i++) {
		cp->opt_state[i] = 0;
	}

	cp->drv->mru_send = 1500;
	cp->drv->mru_recv = 1500;

	cp->drv->magic_send = 0;
	cp->drv->magic_recv = 0;

	ipcp_reset (&cp->drv->ipcp);
}

static
void lcp_send_config_request (ppp_cp_t *cp)
{
	unsigned      i;
	unsigned char buf[64];

	i = 0;

	if (cp->opt_state[LCP_OPT_MRU] == 0) {
		buf[i++] = LCP_OPT_MRU;
		buf[i++] = 4;
		buf[i++] = (cp->drv->mru_recv >> 8) & 0xff;
		buf[i++] = cp->drv->mru_recv & 0xff;
	}

	if (cp->opt_state[LCP_OPT_ACCM] == 0) {
		buf[i++] = LCP_OPT_ACCM;
		buf[i++] = 6;

		ppp_set_uint32_be (buf, i, cp->drv->accm_recv);

		i += 4;
	}

	cp_send_config_request (cp, buf, i);

	cp->counter = 4;
}

/*
 * Filter rejected options received in a config request from the peer.
 *
 * Options in buf are examined and the ones that are not to be rejected
 * are removed. The new size of buf is returned.
 *
 * If no options in buf are to be rejected buf is not touched and 0
 * is returned.
 */
static
unsigned lcp_options_rej (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i, j, k;
	unsigned char type, length;

	i = 0;
	j = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		if ((type == LCP_OPT_MRU) && (length >= 4)) {
			;
		}
		else if ((type == LCP_OPT_ACCM) && (length >= 6)) {
			;
		}
		else if ((type == LCP_OPT_MAGIC) && (length >= 6)) {
			;
		}
		else {
#ifdef DEBUG_PPP
			fprintf (stderr, "lcp: send option reject (type=%u, n=%u)\n",
				type, length
			);
#endif
			for (k = 0; k < length; k++) {
				buf[j++] = buf[i + k];
			}
		}

		i += length;
	}

	return (j);
}

/*
 * Filter refused options received in a config request from the peer.
 *
 * This function is called after rejected options have already been
 * filtered out.
 */
static
unsigned lcp_options_nak (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i, j, k;
	unsigned char type, length;
	unsigned      val;
	int           nak;

	i = 0;
	j = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		nak = 0;

		if (type == LCP_OPT_MRU) {
			val = (buf[i + 2] << 8) | buf[i + 3];

			if (val < 580) {
#ifdef DEBUG_PPP
				fprintf (stderr,
					"lcp: send option nak: peer mru=%u, suggest=%u\n",
					val, 580
				);
#endif
				buf[j++] = type;
				buf[j++] = 4;
				buf[j++] = (580 >> 8) & 0xff;
				buf[j++] = 580 & 0xff;
			}
		}
		else if (type == LCP_OPT_ACCM) {
			;
		}
		else if (type == LCP_OPT_MAGIC) {
			if (ppp_get_uint32_be (buf, i + 2) == 0) {
				/* negotiated magic must not be 0 */
				nak = 1;
			}
		}

		if (nak) {
#ifdef DEBUG_PPP
			fprintf (stderr, "lcp: send option nak (type=%u, n=%u)\n",
				type, length
			);
#endif
			for (k = 0; k < length; k++) {
				buf[j++] = buf[i + k];
			}
		}

		i += length;
	}

	return (j);
}

/*
 * Filter ack options received in a config request from the peer.
 *
 * By the time this function is called rejected and refused
 * options have already been filtered out, so no options are actually
 * filtered here.
 */
static
unsigned lcp_options_ack (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i;
	unsigned char type, length;

	i = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		if (type == LCP_OPT_MRU) {
			cp->drv->mru_send = (buf[i + 2] << 8) | buf[i + 3];

			if (cp->drv->mru_send < 580) {
				cp->drv->mru_send = 580;
			}
		}
		else if (type == LCP_OPT_ACCM) {
			cp->drv->accm_send = ppp_get_uint32_be (buf, i + 2);
		}
		else if (type == LCP_OPT_MAGIC) {
			cp->drv->magic_recv = ppp_get_uint32_be (buf, i + 2);
		}

		i += length;
	}

#ifdef DEBUG_PPP
	fprintf (stderr, "lcp: peer mru:   %u\n", cp->drv->mru_send);
	fprintf (stderr, "lcp: peer accm:  0x%08lx\n", cp->drv->accm_send);
	fprintf (stderr, "lcp: peer magic: 0x%08lx\n", cp->drv->magic_recv);
#endif

	return (cnt);
}

/*
 * Handle options in a config reject received from the peer.
 */
static
void lcp_config_rej (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i;
	unsigned char type, length;

	i = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		if ((length < 2) || ((i + length) > cnt)) {
			return;
		}

#ifdef DEBUG_PPP
		fprintf (stderr,
			"lcp: recv option rej: type=%u, n=%u\n",
			type, length
		);
#endif

		if ((type == LCP_OPT_MRU) && (length >= 4)) {
			cp->drv->mru_recv = 1500;
			cp->opt_state[LCP_OPT_MRU] = 1;
		}
		else if ((type == LCP_OPT_ACCM) && (length >= 6)) {
			cp->drv->accm_recv = 0xffffffff;
			cp->opt_state[LCP_OPT_ACCM] = 1;
		}

		i += length;
	}
}

/*
 * Handle options in a config nak received from the peer.
 */
static
void lcp_config_nak (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned      i;
	unsigned char type, length;
	unsigned      val;

	i = 0;

	while (i < cnt) {
		type = buf[i];
		length = buf[i + 1];

		if ((length < 2) || ((i + length) > cnt)) {
			return;
		}

		if ((type == LCP_OPT_MRU) && (length >= 4)) {
			val = (buf[i + 2] << 8) | buf[i + 3];

#ifdef DEBUG_PPP
			fprintf (stderr,
				"lcp: recv option nak: host mru=%u, suggest=%u\n",
				cp->drv->mru_recv, val
			);
#endif

			if ((val >= 580) && (val <= PPP_MAX_MTU)) {
				cp->drv->mru_recv = val;
				cp->opt_state[LCP_OPT_MRU] = 0;
			}
			else {
				cp->drv->mru_recv = 1500;
				cp->opt_state[LCP_OPT_MRU] = 1;
			}
		}
		else if ((type == LCP_OPT_ACCM) && (length >= 6)) {
#ifdef DEBUG_PPP
			fprintf (stderr,
				"lcp: recv option nak: host accm=0x%08lx, suggest=0x%08lx\n",
				cp->drv->accm_recv, ppp_get_uint32_be (buf, i + 2)
			);
#endif
			cp->drv->accm_recv = ppp_get_uint32_be (buf, i + 2);
			cp->opt_state[LCP_OPT_ACCM] = 0;
		}

		i += length;
	}
}

/*
 * Handle options in a config ack received from the peer.
 */
static
void lcp_config_ack (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "lcp: host mru:   %u\n", cp->drv->mru_recv);
	fprintf (stderr, "lcp: host accm:  0x%08lx\n", cp->drv->accm_recv);
	fprintf (stderr, "lcp: host magic: 0x%08lx\n", 0UL);
#endif
}

static
void lcp_init (char_ppp_t *drv, ppp_cp_t *cp)
{
	cp->drv = drv;

	cp->state = PPP_STATE_STOPPED;
	cp->protocol = PPP_PROTO_LCP;
	cp->name = "lcp";

	cp->counter = 0;
	cp->current_id = 0;

	cp->reset = lcp_reset;

	cp->send_config_request = lcp_send_config_request;

	cp->options_rej = lcp_options_rej;
	cp->options_nak = lcp_options_nak;
	cp->options_ack = lcp_options_ack;

	cp->config_rej = lcp_config_rej;
	cp->config_nak = lcp_config_nak;
	cp->config_ack = lcp_config_ack;
}


static
void cp_set_state (ppp_cp_t *cp, unsigned state)
{
	if (cp->state != state) {
#ifdef DEBUG_PPP
		fprintf (stderr, "%s: state = %u\n", cp->name, state);
#endif
		cp->state = state;
	}
}

static
void cp_send (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt, unsigned code, unsigned id)
{
	unsigned     length;
	ppp_packet_t *pk;

	length = cnt + 4;

	pk = ppp_packet_alloc (cp->drv, length);

	if (pk == NULL) {
		return;
	}

	if ((code >= 1) && (code <= 7)) {
		pk->nocfgopt = 1;
	}

	pk->data[pk->idx + 0] = code;
	pk->data[pk->idx + 1] = id;
	pk->data[pk->idx + 2] = (length >> 8) & 0xff;
	pk->data[pk->idx + 3] = length & 0xff;

	if (cnt > 0) {
		memcpy (pk->data + pk->idx + 4, buf, cnt);
	}

	ppp_send (cp->drv, pk, cp->protocol);
}

static
void cp_send_config_request (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending config request (n=%u)\n",
		cp->name, cnt
	);
#endif

	cp_send (cp, buf, cnt, 1, cp->current_id++);
}

static
void cp_send_config_ack (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt, unsigned id)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending config ack (n=%u, id=%u)\n",
		cp->name, cnt, id
	);
#endif

	cp_send (cp, buf, cnt, 2, id);
}

static
void cp_send_config_nak (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt, unsigned id)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending config nak (n=%u, id=%u)\n",
		cp->name, cnt, id
	);
#endif

	cp_send (cp, buf, cnt, 3, id);
}

static
void cp_send_config_reject (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt, unsigned id)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending config reject (n=%u, id=%u)\n",
		cp->name, cnt, id
	);
#endif

	cp_send (cp, buf, cnt, 4, id);
}

static
void cp_send_term_ack (ppp_cp_t *cp)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending terminate ack\n",
		cp->name
	);
#endif

	cp_send (cp, NULL, 0, 6, cp->current_id++);
}

static
void cp_send_code_reject (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending code reject\n", cp->name);
#endif

	cp_send (cp, buf, cnt, 7, cp->current_id++);
}

static
void cp_send_protocol_reject (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt, unsigned proto)
{
	unsigned char tmp[4];

#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending protocol reject (0x%04x)\n",
		cp->name, proto
	);
#endif

	tmp[0] = (proto >> 8) & 0xff;
	tmp[1] = proto & 0xff;

	cp_send (cp, tmp, 2, 8, cp->current_id++);
}

static
void cp_send_echo_reply (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt, unsigned id)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: sending echo reply (n=%u, id=%u)\n",
		cp->name, cnt, id
	);
#endif

	cp_send (cp, buf, cnt, 10, id);
}

static
int cp_options_invalid (ppp_cp_t *cp, const unsigned char *buf, unsigned cnt)
{
	unsigned i, n;

	i = 0;

	while (i < cnt) {
		if ((i + 2) > cnt) {
			return (1);
		}

		n = buf[i + 1];

		if ((n < 2) || ((i + n) > cnt)) {
			return (1);
		}

		i += n;
	}

	return (0);
}

static
int cp_cfgreq_handle (ppp_cp_t *cp, unsigned char *buf, unsigned cnt, unsigned id)
{
	unsigned n;

	n = cp->options_rej (cp, buf, cnt);

	if (n > 0) {
		cp_send_config_reject (cp, buf, n, id);
		return (1);
	}

	n = cp->options_nak (cp, buf, cnt);

	if (n > 0) {
		cp_send_config_nak (cp, buf, n, id);
		return (1);
	}

	n = cp->options_ack (cp, buf, cnt);

	cp_send_config_ack (cp, buf, n, id);

	return (0);
}

static
void cp_cfgreq (ppp_cp_t *cp, unsigned char *buf, unsigned cnt, unsigned id)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: recv config request (n=%u, id=%u)\n",
		cp->name, cnt, id
	);
#endif

	if (cp_options_invalid (cp, buf, cnt)) {
		return;
	}

	switch (cp->state) {
	case PPP_STATE_INITIAL:
		return;

	case PPP_STATE_STOPPED:
		cp->send_config_request (cp);

		if (cp_cfgreq_handle (cp, buf, cnt, id)) {
			cp_set_state (cp, PPP_STATE_REQSENT);
		}
		else {
			cp_set_state (cp, PPP_STATE_ACKSENT);
		}
		break;

	case PPP_STATE_REQSENT:
		if (cp_cfgreq_handle (cp, buf, cnt, id)) {
			cp_set_state (cp, PPP_STATE_REQSENT);
		}
		else {
			cp_set_state (cp, PPP_STATE_ACKSENT);
		}
		break;

	case PPP_STATE_ACKSENT:
		if (cp_cfgreq_handle (cp, buf, cnt, id)) {
			cp_set_state (cp, PPP_STATE_REQSENT);
		}
		else {
			cp_set_state (cp, PPP_STATE_ACKSENT);
		}
		break;

	case PPP_STATE_ACKRECV:
		if (cp_cfgreq_handle (cp, buf, cnt, id)) {
			cp_set_state (cp, PPP_STATE_ACKRECV);
		}
		else {
			cp_set_state (cp, PPP_STATE_OPENED);
		}
		break;

	case PPP_STATE_OPENED:
		cp->send_config_request (cp);

		if (cp_cfgreq_handle (cp, buf, cnt, id)) {
			cp_set_state (cp, PPP_STATE_REQSENT);
		}
		else {
			cp_set_state (cp, PPP_STATE_ACKSENT);
		}
		break;
	}
}

static
void cp_cfgack (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: recv config ack (n=%u)\n", cp->name, cnt);
#endif

	switch (cp->state) {
	case PPP_STATE_STOPPED:
		cp_send_term_ack (cp);
		break;

	case PPP_STATE_REQSENT:
		cp->config_ack (cp, buf, cnt);
		cp_set_state (cp, PPP_STATE_ACKRECV);
		break;

	case PPP_STATE_ACKRECV:
		cp->send_config_request (cp);
		cp_set_state (cp, PPP_STATE_REQSENT);
		break;

	case PPP_STATE_ACKSENT:
		cp->config_ack (cp, buf, cnt);
		cp_set_state (cp, PPP_STATE_OPENED);
		break;
	}
}

static
void cp_cfgnak (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: recv config nak (n=%u)\n", cp->name, cnt);
#endif

	switch (cp->state) {
	case PPP_STATE_STOPPED:
		cp_send_term_ack (cp);
		break;

	case PPP_STATE_REQSENT:
		cp->config_nak (cp, buf, cnt);
		cp->send_config_request (cp);
		cp_set_state (cp, PPP_STATE_REQSENT);
		break;

	case PPP_STATE_ACKRECV:
		cp->config_nak (cp, buf, cnt);
		cp->send_config_request (cp);
		cp_set_state (cp, PPP_STATE_REQSENT);
		break;

	case PPP_STATE_ACKSENT:
		cp->config_nak (cp, buf, cnt);
		cp->send_config_request (cp);
		cp_set_state (cp, PPP_STATE_ACKSENT);
		break;
	}
}

static
void cp_cfgrej (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: recv config rej (n=%u)\n", cp->name, cnt);
#endif

	switch (cp->state) {
	case PPP_STATE_STOPPED:
		cp_send_term_ack (cp);
		break;

	case PPP_STATE_REQSENT:
		cp->config_rej (cp, buf, cnt);
		cp->send_config_request (cp);
		cp_set_state (cp, PPP_STATE_REQSENT);
		break;

	case PPP_STATE_ACKRECV:
		cp->config_rej (cp, buf, cnt);
		cp->send_config_request (cp);
		cp_set_state (cp, PPP_STATE_REQSENT);
		break;

	case PPP_STATE_ACKSENT:
		cp->config_rej (cp, buf, cnt);
		cp->send_config_request (cp);
		cp_set_state (cp, PPP_STATE_ACKSENT);
		break;
	}
}

static
void cp_termreq (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
#ifdef DEBUG_PPP
	fprintf (stderr, "%s: recv terminate request (n=%u)\n", cp->name, cnt);
#endif

	switch (cp->state) {
	case PPP_STATE_STOPPED:
		cp_send_term_ack (cp);
		break;

	case PPP_STATE_REQSENT:
		cp_send_term_ack (cp);
		cp_set_state (cp, PPP_STATE_STOPPED);
		break;

	case PPP_STATE_ACKRECV:
		cp_send_term_ack (cp);
		cp_set_state (cp, PPP_STATE_STOPPED);
		break;

	case PPP_STATE_ACKSENT:
		cp_send_term_ack (cp);
		cp_set_state (cp, PPP_STATE_STOPPED);
		break;

	case PPP_STATE_OPENED:
		cp_send_term_ack (cp);
		cp_set_state (cp, PPP_STATE_STOPPED);
		break;
	}

	cp->reset (cp);
}

static
void cp_timeout (ppp_cp_t *cp)
{
	switch (cp->state) {
	case PPP_STATE_REQSENT:
	case PPP_STATE_ACKSENT:
		cp->send_config_request (cp);
		break;
	}
}

static
void cp_echoreq (ppp_cp_t *cp, unsigned char *buf, unsigned cnt, unsigned id)
{
	if (cp->state != PPP_STATE_OPENED) {
		return;
	}

	if (cnt < 4) {
		return;
	}

	/* magic */
	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;

	cp_send_echo_reply (cp, buf, cnt, id);
}

static
void cp_recv (ppp_cp_t *cp, unsigned char *buf, unsigned cnt)
{
	unsigned char  code;
	unsigned char  id;
	unsigned short length;

	if (cnt < 4) {
		return;
	}

	code = buf[0];
	id = buf[1];
	length = (buf[2] << 8) | buf[3];

	if ((length < 4) || (length > cnt)) {
		return;
	}

	switch (code) {
	case 1: /* configure request */
		cp_cfgreq (cp, buf + 4, cnt - 4, id);
		break;

	case 2: /* configure ack */
		cp_cfgack (cp, buf + 4, cnt - 4);
		break;

	case 3: /* configure nak */
		cp_cfgnak (cp, buf + 4, cnt - 4);
		break;

	case 4: /* configure reject */
		cp_cfgrej (cp, buf + 4, cnt - 4);
		break;

	case 5: /* terminate request */
		cp_termreq (cp, buf + 4, cnt - 4);
		break;

	case 9: /* echo request */
		cp_echoreq (cp, buf + 4, cnt - 4, id);
		break;

	case 11: /* discard request */
		break;

	case 6: /* terminate ack */

	case 7: /* code reject */

	case 8: /* protocol reject */

	case 10: /* echo reply */

	default:
		fprintf (stderr,
			"%s: recv size=%u code=%u type=%u length=%u\n",
			cp->name, cnt, code, id, length
		);
		cp_send_code_reject (cp, buf, cnt);
		break;
	}
}


static
void ppp_recv (char_ppp_t *drv, unsigned char *buf, unsigned cnt)
{
	unsigned char  addr;
	unsigned char  ctrl;
	unsigned short prot;
	unsigned short crc1, crc2;

	if (drv->lcp.counter > 0) {
		drv->lcp.counter -= 1;

		if (drv->lcp.counter == 0) {
			cp_timeout (&drv->lcp);
		}
	}

	if (drv->ipcp.counter > 0) {
		drv->ipcp.counter -= 1;

		if (drv->ipcp.counter == 0) {
			cp_timeout (&drv->ipcp);
		}
	}

	if (cnt < 6) {
#ifdef DEBUG_PPP
		fprintf (stderr, "ppp: recv short packet (n=%u)\n", cnt);
#endif
		return;
	}

	addr = buf[0];
	ctrl = buf[1];
	prot = (buf[2] << 8) | buf[3];
	crc1 = (buf[cnt - 1] << 8) | buf[cnt - 2];

#ifdef DEBUG_PPP
	fprintf (stderr,
		"ppp: recv packet (n=%u, addr=%02X, ctrl=%02X, proto=%04X, crc=%04X)\n",
		cnt, addr, ctrl, prot, crc1
	);
#endif

	if ((addr != 0xff) || (ctrl != 0x03)) {
#ifdef DEBUG_PPP
		fprintf (stderr, "ppp: recv unknown addr/ctrl (%02X / %02X)\n",
			addr, ctrl
		);
#endif
		return;
	}

	crc2 = ppp_crc16 (drv, buf, cnt - 2);

	if (crc1 != crc2) {
#ifdef DEBUG_PPP
		fprintf (stderr, "ppp: recv bad crc (%04X / %04X)\n", crc1, crc2);
#endif
		return;
	}

	switch (prot) {
	case PPP_PROTO_LCP:
		cp_recv (&drv->lcp, buf + 4, cnt - 6);
		break;

	case PPP_PROTO_IPCP:
		if (drv->lcp.state != PPP_STATE_OPENED) {
			return;
		}
		cp_recv (&drv->ipcp, buf + 4, cnt - 6);
		break;

	case PPP_PROTO_IP:
		if (drv->ipcp.state != PPP_STATE_OPENED) {
			return;
		}
		tun_send (drv, buf + 4, cnt - 6);
		break;

	default:
#ifdef DEBUG_PPP
		fprintf (stderr, "ppp: unknown protocol (%04X)\n", prot);
#endif
		cp_send_protocol_reject (&drv->lcp, buf + 4, cnt - 4, prot);
		break;
	}
}


static
void chr_ppp_close (char_drv_t *cdrv)
{
	char_ppp_t *drv;

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
unsigned chr_ppp_read (char_drv_t *cdrv, void *buf, unsigned cnt)
{
	unsigned      i, n;
	unsigned char v;
	int           esc;
	ppp_packet_t  *pk;
	char_ppp_t    *drv;

	drv = cdrv->ext;

	if (drv->ser_pk_hd == NULL) {
		tun_receive (drv);

		if (drv->ser_out_idx >= drv->ser_out_cnt) {
			return (0);
		}
	}

	if (drv->ser_out_idx >= drv->ser_out_cnt) {
		if (drv->ser_pk_hd == NULL) {
			return (0);
		}

		i = 0;

		pk = drv->ser_pk_hd;

		if (pk->started == 0) {
			drv->ser_out[i++] = 0x7e;
			pk->started = 1;
		}

		while (((i + 2) < drv->ser_out_max) && (pk->idx < pk->cnt)) {
			v = pk->data[pk->idx++];

			esc = 0;

			if ((v == 0x7d) || (v == 0x7e)) {
				esc = 1;
			}
			else if (pk->nocfgopt) {
				if (v < 0x20) {
					esc = 1;
				}
			}
			else if ((v < 0x20) && (drv->accm_send & (1UL << v))) {
				esc = 1;
			}

			if (esc) {
				drv->ser_out[i++] = 0x7d;
				drv->ser_out[i++] = v ^ 0x20;
			}
			else {
				drv->ser_out[i++] = v;
			}
		}

		if (pk->idx >= pk->cnt) {
			drv->ser_out[i++] = 0x7e;

			if (pk->next == NULL) {
				drv->ser_pk_hd = NULL;
				drv->ser_pk_tl = NULL;
			}
			else {
				drv->ser_pk_hd = pk->next;
			}

			ppp_packet_free (drv, pk);
		}

		drv->ser_out_idx = 0;
		drv->ser_out_cnt = i;
	}

	n = drv->ser_out_cnt - drv->ser_out_idx;

	if (n > cnt) {
		n = cnt;
	}

	if (n > 0) {
		memcpy (buf, drv->ser_out + drv->ser_out_idx, n);
		drv->ser_out_idx += n;
	}

	return (n);
}

static
unsigned chr_ppp_write (char_drv_t *cdrv, const void *buf, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tmp;
	unsigned char       val;
	char_ppp_t          *drv;

	drv = cdrv->ext;
	tmp = buf;

	for (i = 0; i < cnt; i++) {
		val = tmp[i];

		if (val == 0x7e) {
			if (drv->ser_inp_esc) {
#ifdef DEBUG_PPP
				fprintf (stderr, "ppp: escape at end of packet\n");
#endif
				drv->ser_inp_cnt = 0;
				drv->ser_inp_esc = 0;
				continue;
			}

			if (drv->ser_inp_cnt > 0) {
				ppp_recv (drv, drv->ser_inp, drv->ser_inp_cnt);
			}

			drv->ser_inp_cnt = 0;
			drv->ser_inp_esc = 0;

			continue;
		}

		if ((val == 0x7d) && (drv->ser_inp_esc == 0)) {
			drv->ser_inp_esc = 1;
			continue;
		}

		if (drv->ser_inp_esc) {
			drv->ser_inp_esc = 0;
			val ^= 0x20;
		}
		else if ((val < 0x20) && (drv->accm_recv & (1UL << val))) {
#ifdef DEBUG_PPP
			fprintf (stderr, "ppp: missing escape (%02X)\n", val);
#endif
			continue;
		}

		if (drv->ser_inp_cnt >= drv->ser_inp_max) {
#ifdef DEBUG_PPP
			fprintf (stderr, "ppp: buffer overflow\n");
#endif
			drv->ser_inp_cnt = 0;
		}

		drv->ser_inp[drv->ser_inp_cnt++] = val;
	}

	return (cnt);
}

static
int chr_ppp_init (char_ppp_t *drv, const char *name)
{
	chr_init (&drv->cdrv, drv);

	drv->cdrv.close = chr_ppp_close;
	drv->cdrv.read = chr_ppp_read;
	drv->cdrv.write = chr_ppp_write;

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

	if (chr_ppp_get_option_ip (name, "host-ip", drv->ip_local)) {
		drv->ip_local[0] = 192;
		drv->ip_local[0] = 168;
		drv->ip_local[0] = 0;
		drv->ip_local[0] = 1;
	}

	if (chr_ppp_get_option_ip (name, "guest-ip", drv->ip_remote)) {
		drv->ip_remote[0] = 192;
		drv->ip_remote[1] = 168;
		drv->ip_remote[2] = 0;
		drv->ip_remote[3] = 2;
	}

	drv->magic_send = 0;
	drv->magic_recv = 0;

	drv->ser_out_idx = 0;
	drv->ser_out_cnt = 0;
	drv->ser_out_max = PPP_MAX_MTU + 6;

	drv->ser_inp_cnt = 0;
	drv->ser_inp_esc = 0;
	drv->ser_inp_max = PPP_MAX_MTU + 6;

	drv->ser_pk_hd = NULL;
	drv->ser_pk_tl = NULL;

	drv->crc16_ok = 0;

	lcp_init (drv, &drv->lcp);
	ipcp_init (drv, &drv->ipcp);

	lcp_reset (&drv->lcp);

	return (0);
}

char_drv_t *chr_ppp_open (const char *name)
{
	char_ppp_t *drv;

	drv = malloc (sizeof (char_ppp_t));

	if (drv == NULL) {
		return (NULL);
	}

	if (chr_ppp_init (drv, name)) {
		chr_ppp_close (&drv->cdrv);
		return (NULL);
	}

	return (&drv->cdrv);
}
