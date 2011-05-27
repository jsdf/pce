/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/adb.h                                       *
 * Created:     2010-11-02 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_ADB_H
#define PCE_MACPLUS_ADB_H 1


typedef struct adb_dev_t {
	void          *ext;

	unsigned char current_addr;
	unsigned char default_addr;
	unsigned char default_handler;

	char          service_request;

	unsigned long reg[4];

	void     (*del) (struct adb_dev_t *dev);

	void     (*reset) (struct adb_dev_t *dev);
	void     (*flush) (struct adb_dev_t *dev);
	unsigned (*talk) (struct adb_dev_t *dev, unsigned reg, void *buf);
	void     (*talk_done) (struct adb_dev_t *dev, unsigned reg);
	void     (*listen) (struct adb_dev_t *dev, unsigned reg, const void *buf, unsigned cnt);
} adb_dev_t;


typedef struct {
	unsigned char state;

	char          writing;

	unsigned char cmd;

	unsigned char last_talk;

	unsigned char buf_idx;
	unsigned char buf_cnt;
	unsigned char buf[8];

	unsigned char bit_cnt;
	unsigned char bit_val;

	unsigned      clock;

	unsigned long scan_clock;

	unsigned      dev_cnt;
	adb_dev_t     *dev[16];

	void          *shift_in_ext;
	void          (*shift_in) (void *ext, unsigned char val);

	void          *shift_out_ext;
	unsigned char (*shift_out) (void *ext);

	unsigned char set_int_val;
	void          *set_int_ext;
	void          (*set_int) (void *ext, unsigned char val);
} mac_adb_t;


void adb_dev_del (adb_dev_t *dev);
void adb_dev_init (adb_dev_t *dev, unsigned addr, unsigned handler);

void adb_dev_reset (adb_dev_t *dev);
void adb_dev_flush (adb_dev_t *dev);
unsigned adb_dev_talk (adb_dev_t *dev, unsigned reg, void *buf);
void adb_dev_listen (adb_dev_t *dev, unsigned reg, const void *buf, unsigned cnt);

void adb_init (mac_adb_t *adb);
void adb_free (mac_adb_t *adb);

mac_adb_t *mac_adb_new (void);
void mac_adb_del (mac_adb_t *adb);

void adb_reset (mac_adb_t *adb);

void adb_set_shift_in_fct (mac_adb_t *adb, void *ext, void *fct);

void adb_set_shift_out_fct (mac_adb_t *adb, void *ext, void *fct);

void adb_set_int_fct (mac_adb_t *adb, void *ext, void *fct);

int adb_add_device (mac_adb_t *adb, adb_dev_t *dev);

void mac_adb_set_state (mac_adb_t *adb, unsigned char val);

void mac_adb_clock (mac_adb_t *adb, unsigned cnt);


#endif
