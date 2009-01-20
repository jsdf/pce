/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/clock/ds1743.h                                   *
 * Created:     2006-12-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_CHIPSET_CLOCK_DS1743_H
#define PCE_CHIPSET_CLOCK_DS1743_H 1


#define DS1743_NVMAX 8192


typedef struct {
	/* This is called just before the RTC bytes are read. */
	void          *cbrext;
	void          (*cbread) (void *ext, unsigned char val);

	/* This is called just after the RTC bytes were written. */
	void          *cbwext;
	void          (*cbwrite) (void *ext, unsigned char val);

	unsigned      cnt;
	unsigned char data[DS1743_NVMAX];

	unsigned      year;
	unsigned      month;
	unsigned      day;
	unsigned      dow;
	unsigned      hour;
	unsigned      minute;
	unsigned      second;
} ds1743_t;


void ds1743_init (ds1743_t *rtc, unsigned size);
void ds1743_free (ds1743_t *rtc);

ds1743_t *ds1743_new (unsigned size);
void ds1743_del (ds1743_t *rtc);

void ds1743_set_cbread (ds1743_t *rtc, void *ext, void *fct);
void ds1743_set_cbwrite (ds1743_t *rtc, void *ext, void *fct);

void ds1743_set_date (ds1743_t *rtc, unsigned y, unsigned m, unsigned d);
void ds1743_set_time (ds1743_t *rtc, unsigned h, unsigned m, unsigned s);
void ds1743_set_day (ds1743_t *rtc, unsigned dow);

unsigned char ds1743_get_uint8 (ds1743_t *rtc, unsigned long addr);
unsigned short ds1743_get_uint16 (ds1743_t *rtc, unsigned long addr);
unsigned long ds1743_get_uint32 (ds1743_t *rtc, unsigned long addr);

void ds1743_set_uint8 (ds1743_t *rtc, unsigned long addr, unsigned char val);
void ds1743_set_uint16 (ds1743_t *rtc, unsigned long addr, unsigned short val);
void ds1743_set_uint32 (ds1743_t *rtc, unsigned long addr, unsigned long val);


#endif
