/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/clock/mc146818a.h                                *
 * Created:     2018-09-01 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2018 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_CHIPSET_MC146818A_H
#define PCE_CHIPSET_MC146818A_H 1


typedef struct {
	unsigned      cnt;
	unsigned char data[64];

	unsigned      century_offset;

	unsigned long clock_input;
	unsigned long clock;
	unsigned long clock_uip;

	void          *irq_ext;
	void          (*irq_fct) (void *ext, unsigned char val);

	char          *fname;
} mc146818a_t;


void mc146818a_init (mc146818a_t *rtc);
void mc146818a_free (mc146818a_t *rtc);

mc146818a_t *mc146818a_new (void);
void mc146818a_del (mc146818a_t *rtc);

void mc146818a_set_irq_fct (mc146818a_t *rtc, void *ext, void *fct);
void mc146818a_set_input_clock (mc146818a_t *rtc, unsigned long val);

int mc146818a_set_fname (mc146818a_t *rtc, const char *fname);

int mc146818a_load (mc146818a_t *rtc);
int mc146818a_save (mc146818a_t *rtc);

void mc146818a_set_century_offset (mc146818a_t *rtc, unsigned ofs);

void mc146818a_set_date (mc146818a_t *rtc, unsigned yrs, unsigned mon, unsigned day);
void mc146818a_set_time (mc146818a_t *rtc, unsigned hrs, unsigned min, unsigned sec);
void mc146818a_set_time_now (mc146818a_t *rtc);
void mc146818a_set_time_string (mc146818a_t *rtc, const char *str);

unsigned char mc146818a_get_uint8 (mc146818a_t *rtc, unsigned long addr);
unsigned short mc146818a_get_uint16 (mc146818a_t *rtc, unsigned long addr);
unsigned long mc146818a_get_uint32 (mc146818a_t *rtc, unsigned long addr);

void mc146818a_set_uint8 (mc146818a_t *rtc, unsigned long addr, unsigned char val);
void mc146818a_set_uint16 (mc146818a_t *rtc, unsigned long addr, unsigned short val);
void mc146818a_set_uint32 (mc146818a_t *rtc, unsigned long addr, unsigned long val);

void mc146818a_reset (mc146818a_t *rtc);

void mc146818a_clock (mc146818a_t *rtc, unsigned long cnt);


#endif
