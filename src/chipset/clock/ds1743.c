/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/clock/ds1743.c                                   *
 * Created:     2006-12-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2006-2015 Hampa Hug <hampa@hampa.ch>                     *
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


/*
 * DS1743/DS1743P RTC
 */


#include <stdio.h>
#include <stdlib.h>

#include "ds1743.h"


#define DS1743_YEAR    7  /* year (00 - 99) */
#define DS1743_MONTH   6  /* month (01 - 12) */
#define DS1743_DAY     5  /* day (01 - 31) */
#define DS1743_DOW     4  /* day of week (01 - 07) */
#define DS1743_HOUR    3  /* hour (00 - 23) */
#define DS1743_MINUTE  2  /* minute (00 - 59) */
#define DS1743_SECOND  1  /* second (00 - 59) */
#define DS1743_CENTURY 0  /* century (00 - 39) */

/* century */
#define DS1743_W 0x80
#define DS1743_R 0x40

/* dow */
#define DS1743_BF 0x80
#define DS1743_FT 0x40

/* seconds */
#define DS1743_NOSC 0x80


static void ds1743_clk_to_ram (ds1743_t *rtc);


void ds1743_init (ds1743_t *rtc, unsigned size)
{
	unsigned i;

	rtc->cbrext = NULL;
	rtc->cbread = NULL;

	rtc->cbwext = NULL;
	rtc->cbwrite = NULL;

	if (size < 8) {
		size = 8;
	}

	rtc->cnt = size;

	for (i = 0; i < rtc->cnt; i++) {
		rtc->data[i] = 0;
	}

	rtc->year = 2000;
	rtc->month = 0;
	rtc->day = 0;
	rtc->dow = 0;
	rtc->hour = 0;
	rtc->minute = 0;
	rtc->second = 0;

	ds1743_clk_to_ram (rtc);
}

void ds1743_free (ds1743_t *rtc)
{
}

ds1743_t *ds1743_new (unsigned size)
{
	ds1743_t *rtc;

	rtc = malloc (sizeof (ds1743_t));
	if (rtc == NULL) {
		return (NULL);
	}

	ds1743_init (rtc, size);

	return (rtc);
}

void ds1743_del (ds1743_t *rtc)
{
	if (rtc != NULL) {
		ds1743_free (rtc);
		free (rtc);
	}
}

void ds1743_set_cbread (ds1743_t *rtc, void *ext, void *fct)
{
	rtc->cbrext = ext;
	rtc->cbread = fct;
}

void ds1743_set_cbwrite (ds1743_t *rtc, void *ext, void *fct)
{
	rtc->cbwext = ext;
	rtc->cbwrite = fct;
}

static
unsigned ds1743_int_to_bcd (unsigned val)
{
	if (val >= 100) {
		val %= 100;
	}

	return ((val % 10) + 16 * (val / 10));
}

static
unsigned ds1743_bcd_to_int (unsigned val)
{
	if ((val & 0x0f) > 0x09) {
		val += 0x06;
	}

	if ((val & 0xf0) > 0x90) {
		val += 0x60;
	}

	return ((val & 0x0f) + 10 * ((val >> 4) & 0xf0));
}

static
void ds1743_clk_to_ram (ds1743_t *rtc)
{
	unsigned      v;
	unsigned char *p;

	p = rtc->data + rtc->cnt - 8;

	v = ds1743_int_to_bcd (rtc->year / 100);
	p[DS1743_CENTURY] &= 0xc0;
	p[DS1743_CENTURY] |= (v & 0x3f);

	v = ds1743_int_to_bcd (rtc->year % 100);
	p[DS1743_YEAR] = v;

	v = ds1743_int_to_bcd (rtc->month + 1);
	p[DS1743_MONTH] &= 0xe0;
	p[DS1743_MONTH] |= (v & 0x1f);

	v = ds1743_int_to_bcd (rtc->day + 1);
	p[DS1743_DAY] &= 0xc0;
	p[DS1743_DAY] |= (v & 0x3f);

	v = ds1743_int_to_bcd (rtc->dow + 1);
	p[DS1743_DOW] &= 0xf8;
	p[DS1743_DOW] |= (v & 0x07);

	v = ds1743_int_to_bcd (rtc->hour);
	p[DS1743_HOUR] &= 0xc0;
	p[DS1743_HOUR] |= (v & 0x3f);

	v = ds1743_int_to_bcd (rtc->minute);
	p[DS1743_MINUTE] &= 0x80;
	p[DS1743_MINUTE] |= (v & 0x7f);

	v = ds1743_int_to_bcd (rtc->second);
	p[DS1743_SECOND] &= 0x80;
	p[DS1743_SECOND] |= (v & 0x7f);
}

static
void ds1743_ram_to_clk (ds1743_t *rtc)
{
	unsigned      v;
	unsigned char *p;

	p = rtc->data + rtc->cnt - 8;

	v = ds1743_bcd_to_int (p[DS1743_CENTURY] & 0x3f);
	rtc->year = 100 * v;

	v = ds1743_bcd_to_int (p[DS1743_YEAR]);
	rtc->year += v;

	v = ds1743_bcd_to_int (p[DS1743_MONTH] & 0x1f);
	rtc->month = v - 1;

	v = ds1743_bcd_to_int (p[DS1743_DAY] & 0x3f);
	rtc->day = v - 1;

	v = ds1743_bcd_to_int (p[DS1743_DOW] & 0x07);
	rtc->dow = v - 1;

	v = ds1743_bcd_to_int (p[DS1743_HOUR] & 0x3f);
	rtc->hour = v;

	v = ds1743_bcd_to_int (p[DS1743_MINUTE] & 0x7f);
	rtc->minute = v;

	v = ds1743_bcd_to_int (p[DS1743_SECOND] & 0x7f);
	rtc->second = v;
}

void ds1743_set_date (ds1743_t *rtc, unsigned y, unsigned m, unsigned d)
{
	rtc->year = y;
	rtc->month = m;
	rtc->day = d;

	ds1743_clk_to_ram (rtc);
}

void ds1743_set_time (ds1743_t *rtc, unsigned h, unsigned m, unsigned s)
{
	rtc->hour = h;
	rtc->minute = m;
	rtc->second = s;

	ds1743_clk_to_ram (rtc);
}

void ds1743_set_day (ds1743_t *rtc, unsigned dow)
{
	rtc->dow = dow;

	ds1743_clk_to_ram (rtc);
}

unsigned char ds1743_get_uint8 (ds1743_t *rtc, unsigned long addr)
{
	unsigned char val;
	unsigned char *p;

	if (addr >= rtc->cnt) {
		return (0);
	}

	if (addr < (rtc->cnt - 8)) {
		return (rtc->data[addr]);
	}

	if (rtc->cbread != NULL) {
		rtc->cbread (rtc->cbrext, 1);
	}

	p = rtc->data + (rtc->cnt - 8);

	if ((p[DS1743_CENTURY] & DS1743_R & DS1743_W) == 0) {
		ds1743_clk_to_ram (rtc);
	}

	val = rtc->data[addr];

	if (addr == (rtc->cnt - 8 + DS1743_DOW)) {
		val |= DS1743_BF;
	}

	return (val);
}

unsigned short ds1743_get_uint16 (ds1743_t *rtc, unsigned long addr)
{
	return (0);
}

unsigned long ds1743_get_uint32 (ds1743_t *rtc, unsigned long addr)
{
	return (0);
}

void ds1743_set_uint8 (ds1743_t *rtc, unsigned long addr, unsigned char val)
{
	unsigned char old;

	if (addr >= rtc->cnt) {
		return;
	}

	if (addr < (rtc->cnt - 8)) {
		rtc->data[addr] = val;
		return;
	}

	if (addr == (rtc->cnt - 8 + DS1743_CENTURY)) {
		old = rtc->data[addr];
		rtc->data[addr] = val;

		if ((old ^ val) & 0x40) {
			/* R changed */
			ds1743_clk_to_ram (rtc);
		}
		else if ((old ^ val) & ~val & 0x80) {
			/* W: 1 -> 0 */
			ds1743_ram_to_clk (rtc);
		}
	}
	else {
		rtc->data[addr] = val;
	}

	if (rtc->cbwrite != NULL) {
		rtc->cbwrite (rtc->cbwext, 1);
	}
}

void ds1743_set_uint16 (ds1743_t *rtc, unsigned long addr, unsigned short val)
{
}

void ds1743_set_uint32 (ds1743_t *rtc, unsigned long addr, unsigned long val)
{
}
