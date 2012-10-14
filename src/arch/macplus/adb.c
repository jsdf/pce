/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/adb.c                                       *
 * Created:     2010-11-02 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010-2012 Hampa Hug <hampa@hampa.ch>                     *
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

#include <stdlib.h>


#ifndef DEBUG_ADB
#define DEBUG_ADB 0
#endif


/* ADB bit cell time (100 us) */
#define ADB_BIT_CLK (MAC_CPU_CLOCK / 10000)


void adb_dev_reset (adb_dev_t *dev)
{
#if DEBUG_ADB >= 1
	mac_log_deb ("adb: reset device %u\n", dev->default_addr);
#endif

	dev->current_addr = dev->default_addr;

	dev->service_request = 0;

	dev->reg[0] = 0;
	dev->reg[1] = 0;
	dev->reg[2] = 0;
	dev->reg[3] = 0x6000;
	dev->reg[3] |= (dev->default_addr & 0x0f) << 8;
	dev->reg[3] |= dev->default_handler & 0xff;
}

void adb_dev_flush (adb_dev_t *dev)
{
	dev->service_request = 0;
}

unsigned adb_dev_talk (adb_dev_t *dev, unsigned reg, void *buf)
{
	unsigned char *tmp;

	if (reg != 3) {
		return (0);
	}

	tmp = buf;

	tmp[1] = dev->reg[3] & 0xff;
	tmp[0] = (dev->reg[3] >> 8) & 0xff;

	return (2);
}

void adb_dev_talk_done (adb_dev_t *dev, unsigned reg)
{
}

void adb_dev_listen (adb_dev_t *dev, unsigned reg, const void *buf, unsigned cnt)
{
	unsigned            addr;
	const unsigned char *src;

	if ((reg != 3) || (cnt < 2)) {
		return;
	}

	src = buf;

	if (src[1] == 0xfe) {
		addr = src[0] & 0x0f;

#if DEBUG_ADB >= 1
		mac_log_deb ("adb: device %u set address %u\n",
			dev->current_addr, addr
		);
#endif

		dev->current_addr = addr;
		dev->reg[3] &= 0xf0ff;
		dev->reg[3] |= addr << 8;
	}
#if DEBUG_ADB >= 1
	else {
		mac_log_deb ("adb: device %u set reg 3 0x%04x\n",
			dev->current_addr, (src[0] << 8) | src[1]
		);
	}
#endif
}

void adb_dev_del (adb_dev_t *dev)
{
	if (dev->del != NULL) {
		dev->del (dev);
	}
	else {
		free (dev);
	}
}

void adb_dev_init (adb_dev_t *dev, unsigned addr, unsigned handler)
{
	dev->ext = NULL;

	dev->current_addr = addr;
	dev->default_addr = addr;
	dev->default_handler = handler;

	dev->service_request = 0;

	dev->reg[0] = 0;
	dev->reg[1] = 0;
	dev->reg[2] = 0;
	dev->reg[3] = 0x6000;
	dev->reg[3] |= (dev->default_addr & 0x0f) << 8;
	dev->reg[3] |= dev->default_handler & 0xff;

	dev->del = NULL;

	dev->reset = adb_dev_reset;
	dev->flush = adb_dev_flush;
	dev->talk = adb_dev_talk;
	dev->talk_done = adb_dev_talk_done;
	dev->listen = adb_dev_listen;
}


void adb_init (mac_adb_t *adb)
{
	adb->state = 3;

	adb->writing = 0;

	adb->cmd = 0;

	adb->last_talk = 0;

	adb->buf_idx = 0;
	adb->buf_cnt = 0;

	adb->bit_cnt = 0;
	adb->bit_val = 0;

	adb->clock = 0;
	adb->scan_clock = 0;

	adb->dev_cnt = 0;

	adb->shift_in_ext = NULL;
	adb->shift_in = NULL;

	adb->shift_out_ext = NULL;
	adb->shift_out = NULL;

	adb->set_int_val = 0;
	adb->set_int_ext = NULL;
	adb->set_int = NULL;
}

void adb_free (mac_adb_t *adb)
{
	unsigned i;

	if (adb == NULL) {
		return;
	}

	for (i = 0; i < adb->dev_cnt; i++) {
		adb_dev_del (adb->dev[i]);
	}
}

mac_adb_t *mac_adb_new (void)
{
	mac_adb_t *adb;

	adb = malloc (sizeof (mac_adb_t));

	if (adb == NULL) {
		return (NULL);
	}

	adb_init (adb);

	return (adb);
}

void mac_adb_del (mac_adb_t *adb)
{
	if (adb != NULL) {
		adb_free (adb);
		free (adb);
	}
}

void adb_set_shift_in_fct (mac_adb_t *adb, void *ext, void *fct)
{
	adb->shift_in_ext = ext;
	adb->shift_in = fct;
}

void adb_set_shift_out_fct (mac_adb_t *adb, void *ext, void *fct)
{
	adb->shift_out_ext = ext;
	adb->shift_out = fct;
}

void adb_set_int_fct (mac_adb_t *adb, void *ext, void *fct)
{
	adb->set_int_ext = ext;
	adb->set_int = fct;
}

int adb_add_device (mac_adb_t *adb, adb_dev_t *dev)
{
	unsigned i;

	for (i = 0; i < adb->dev_cnt; i++) {
		if (adb->dev[i] == dev) {
			return (1);
		}
	}

	if (adb->dev_cnt >= 16) {
		return (1);
	}

	adb->dev[adb->dev_cnt++] = dev;

	return (0);
}

static
adb_dev_t *adb_get_device (mac_adb_t *adb, unsigned addr)
{
	unsigned i;

	for (i = 0; i < adb->dev_cnt; i++) {
		if (adb->dev[i]->current_addr == addr) {
			return (adb->dev[i]);
		}
	}

	return (NULL);
}

static
void adb_set_int (mac_adb_t *adb, int val)
{
	val = (val != 0);

	if (adb->set_int_val == val) {
		return;
	}

	adb->set_int_val = val;

#if DEBUG_ADB >= 2
	mac_log_deb ("adb: interrupt = %d\n", val != 0);
#endif

	if (adb->set_int != NULL) {
		adb->set_int (adb->set_int_ext, val);
	}
}

static
void adb_set_service_request (mac_adb_t *adb, unsigned addr)
{
	unsigned i;

	for (i = 0; i < adb->dev_cnt; i++) {
		if (adb->dev[i]->service_request == 0) {
			continue;
		}

		if (adb->dev[i]->current_addr == addr) {
			continue;
		}

		if ((adb->dev[i]->reg[3] & 0x2000) == 0) {
			continue;
		}

#if DEBUG_ADB >= 1
		mac_log_deb ("adb: service request dev %u\n",
			adb->dev[i]->current_addr
		);
#endif

		adb_set_int (adb, 1);
	}
}

void adb_reset (mac_adb_t *adb)
{
	unsigned i;

#if DEBUG_ADB >= 1
	mac_log_deb ("adb: reset\n");
#endif

	for (i = 0; i < adb->dev_cnt; i++) {
		adb->dev[i]->reset (adb->dev[i]);
	}

	adb->state = 3;

	adb->writing = 1;

	adb->cmd = 0;

	adb->last_talk = 0;

	adb->buf_idx = 0;
	adb->buf_cnt = 0;

	adb->bit_cnt = 0;
	adb->bit_val = 0;

	adb->clock = 0;
	adb->scan_clock = 0;

	adb_set_int (adb, 0);
}

static
void adb_cmd_reset (mac_adb_t *adb, unsigned char cmd)
{
#if DEBUG_ADB >= 1
	mac_log_deb ("adb: cmd reset\n");
#endif

	adb_reset (adb);

	adb->writing = 1;
}

static
void adb_flush (mac_adb_t *adb, unsigned char cmd)
{
	unsigned  addr;
	adb_dev_t *dev;

	adb->writing = 1;

	addr = cmd >> 4;

#if DEBUG_ADB >= 1
	mac_log_deb ("adb: cmd flush dev %u\n", addr);
#endif

	dev = adb_get_device (adb, addr);

	if (dev != NULL) {
		dev->flush (dev);
	}
}

static
void adb_talk (mac_adb_t *adb, unsigned char cmd)
{
	unsigned  addr, reg;
	adb_dev_t *dev;

	adb->writing = 1;

	addr = (cmd >> 4) & 0x0f;
	reg = cmd & 3;

#if DEBUG_ADB >= 2
	if (adb->state != 3) {
		mac_log_deb ("adb: cmd talk dev %u reg %u\n", addr, reg);
	}
#endif

	dev = adb_get_device (adb, addr);

	if (dev == NULL) {
		return;
	}

	adb->buf_cnt = dev->talk (dev, reg, adb->buf);

	adb->last_talk = cmd;
}

static
void adb_talk_done (mac_adb_t *adb, unsigned char cmd)
{
	adb_dev_t *dev;

	dev = adb_get_device (adb, (cmd >> 4) & 0x0f);

	if (dev != NULL) {
		dev->talk_done (dev, cmd & 3);
	}
}

static
void adb_listen (mac_adb_t *adb, unsigned char cmd)
{
	unsigned  addr, reg;
	adb_dev_t *dev;

	adb->writing = 0;

	addr = (cmd >> 4) & 0x0f;
	reg = cmd & 3;

#if DEBUG_ADB >= 2
	mac_log_deb ("adb: cmd listen dev %u reg %u cnt %u\n",
		addr, reg, adb->buf_cnt
	);
#endif

	dev = adb_get_device (adb, addr);

	if (dev == NULL) {
		return;
	}

	dev->listen (dev, reg, adb->buf, adb->buf_cnt);
}

static
void adb_finish_transaction (mac_adb_t *adb)
{
	if (adb->buf_idx >= adb->buf_cnt) {
		return;
	}

	if ((adb->cmd & 0x0c) == 0x08) {
		adb_listen (adb, adb->cmd);
	}
}

static
void adb_start_transaction (mac_adb_t *adb, unsigned char cmd, int poll)
{
	adb_set_int (adb, 0);

	adb->cmd = cmd;

	adb->buf_idx = 0;
	adb->buf_cnt = 0;

	if ((cmd & 0x0f) == 0) {
		adb_cmd_reset (adb, cmd);
	}
	else if ((cmd & 0x0c) == 0x0c) {
		adb_talk (adb, cmd);
	}
	else if ((cmd & 0x0c) == 0x08) {
		; /* listen */
	}
	else if ((cmd & 0x0f) == 0x01) {
		adb_flush (adb, cmd);
	}
	else {
		mac_log_deb ("adb: unknown cmd (%02X)\n", cmd);
	}

	if (poll) {
		adb_set_service_request (adb, (cmd >> 4) & 0x0f);
	}
}

void mac_adb_set_state (mac_adb_t *adb, unsigned char val)
{
	val &= 3;

	if (adb->state == val) {
		return;
	}

	adb->clock += ADB_BIT_CLK;

	if ((val == 0) || (val == 3)) {
		adb_finish_transaction (adb);
	}

#if DEBUG_ADB >= 2
	mac_log_deb ("adb: state = %u\n", val);
#endif

	adb->state = val;

	adb_set_int (adb, 0);

	if (val == 0) {
		adb->writing = 0;

		adb->buf_idx = 0;
		adb->buf_cnt = 0;

		adb->bit_cnt = 8;
		adb->bit_val = 0;
	}
	else if ((val == 1) || (val == 2)) {
		if (adb->writing) {
			if (adb->buf_idx < adb->buf_cnt) {
				adb->bit_cnt = 8;
				adb->bit_val = adb->buf[adb->buf_idx++];

				if (adb->buf_idx >= adb->buf_cnt) {
					adb_talk_done (adb, adb->cmd);
				}
			}
			else {
				adb->bit_cnt = 8;
				adb->bit_val = 0xaa;
				adb_set_int (adb, 1);
			}
		}
		else {
			adb->bit_cnt = 8;
			adb->bit_val = 0;
		}
	}
	else if (val == 3) {
		adb->writing = 1;

		adb->buf_idx = 0;
		adb->buf_cnt = 0;

		adb->bit_cnt = 0;
		adb->bit_val = 0;

		adb->scan_clock = 0;
	}
}

static
void adb_clock_idle (mac_adb_t *adb, unsigned cnt)
{
	adb->scan_clock += cnt;

	if (adb->scan_clock < 86170) {
		return;
	}

	adb->scan_clock = 0;

	if (adb->last_talk == 0) {
		return;
	}

	adb_start_transaction (adb, adb->last_talk, 1);

	if (adb->buf_idx < adb->buf_cnt) {
		adb->bit_cnt = 8;
		adb->bit_val = 0xaa;
	}
	else if (adb->set_int_val) {
		adb->bit_cnt = 8;
		adb->bit_val = 0xaa;
	}
}

void mac_adb_clock (mac_adb_t *adb, unsigned cnt)
{
	if (adb->bit_cnt == 0) {
		if (adb->state == 3) {
			adb_clock_idle (adb, cnt);
		}

		return;
	}

	adb->scan_clock = 0;

	if (cnt < adb->clock) {
		adb->clock -= cnt;
		return;
	}

	adb->clock = 0;

	if (adb->writing) {
		if (adb->shift_in != NULL) {
			adb->shift_in (adb->shift_in_ext, (adb->bit_val >> 7) & 1);
		}

		adb->bit_val <<= 1;
		adb->bit_cnt -= 1;
	}
	else {
		adb->bit_val <<= 1;
		adb->bit_cnt -= 1;

		if (adb->shift_out != NULL) {
			if (adb->shift_out (adb->shift_out_ext)) {
				adb->bit_val |= 0x01;
			}
		}

		if (adb->bit_cnt == 0) {
			if (adb->state == 0) {
				adb_start_transaction (adb, adb->bit_val, 0);
			}
			else if (adb->buf_cnt < 8) {
				adb->buf[adb->buf_cnt++] = adb->bit_val;
			}
		}
	}

	adb->clock += ADB_BIT_CLK;
}
