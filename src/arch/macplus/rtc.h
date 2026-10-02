/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/rtc.h                                       *
 * Created:     2007-11-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_RTC_H
#define PCE_MACPLUS_RTC_H 1


typedef struct {
	unsigned char data[256];

	unsigned char reg_wp;
	unsigned char reg_test;

	unsigned long clock;
	unsigned long rtime;

	int           data_out;
	unsigned      state;
	unsigned      bitcnt;
	unsigned char cmd1;
	unsigned char cmd2;
	unsigned char shift;
	unsigned char sigval;

	int           realtime;

	unsigned long clkcnt;

	void          *set_data_ext;
	void          (*set_data) (void *ext, unsigned char val);
	unsigned char set_data_val;

	void          *set_osi_ext;
	void          (*set_osi) (void *ext, unsigned char val);
	unsigned char set_osi_val;
} mac_rtc_t;


void mac_rtc_init (mac_rtc_t *rtc);
void mac_rtc_free (mac_rtc_t *rtc);

void mac_rtc_set_data_fct (mac_rtc_t *rtc, void *ext, void *fct);
void mac_rtc_set_osi_fct (mac_rtc_t *rtc, void *ext, void *fct);

void mac_rtc_set_realtime (mac_rtc_t *rtc, int realtime);

void mac_rtc_set_defaults (mac_rtc_t *rtc);

int mac_rtc_load_file (mac_rtc_t *rtc, const char *fname);
int mac_rtc_save_file (mac_rtc_t *rtc, const char *fname);

void mac_rtc_set_time (mac_rtc_t *rtc, unsigned long time, int utc);
void mac_rtc_set_time_now (mac_rtc_t *rtc);
void mac_rtc_set_time_str (mac_rtc_t *rtc, const char *str);

void mac_rtc_set_uint8 (mac_rtc_t *rtc, unsigned char val);

void mac_rtc_clock (mac_rtc_t *rtc, unsigned long n);


#endif
