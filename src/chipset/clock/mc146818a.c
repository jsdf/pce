/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/clock/mc146818a.c                                *
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


/*
 * MC146818A RTC
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mc146818a.h"


#ifndef DEBUG_MC146818A
#define DEBUG_MC146818A 0
#endif


#define MC146818A_REGA_UIP  0x80
#define MC146818A_REGA_DV   0x70
#define MC146818A_REGA_RS   0x0f

#define MC146818A_REGB_SET  0x80
#define MC146818A_REGB_PIE  0x40
#define MC146818A_REGB_AIE  0x20
#define MC146818A_REGB_UIE  0x10
#define MC146818A_REGB_SQWE 0x08
#define MC146818A_REGB_DM   0x04
#define MC146818A_REGB_24   0x02
#define MC146818A_REGB_DSE  0x01

#define MC146818A_REGC_IRQF 0x80
#define MC146818A_REGC_PF   0x40
#define MC146818A_REGC_AF   0x20
#define MC146818A_REGC_UF   0x10

#define MC146818A_REGD_VRT  0x80


void mc146818a_init (mc146818a_t *rtc)
{
	unsigned i;

	rtc->cnt = 64;

	for (i = 0; i < rtc->cnt; i++) {
		rtc->data[i] = 0;
	}

	rtc->century_offset = 0;

	rtc->clock_input = 0;
	rtc->clock = 0;
	rtc->clock_uip = 0;

	rtc->irq_ext = NULL;
	rtc->irq_fct = NULL;

	rtc->fname = NULL;
}

void mc146818a_free (mc146818a_t *rtc)
{
	if (rtc->fname != NULL) {
		free (rtc->fname);
		rtc->fname = NULL;
	}
}

mc146818a_t *mc146818a_new (void)
{
	mc146818a_t *rtc;

	if ((rtc = malloc (sizeof (mc146818a_t))) == NULL) {
		return (NULL);
	}

	mc146818a_init (rtc);

	return (rtc);
}

void mc146818a_del (mc146818a_t *rtc)
{
	if (rtc != NULL) {
		mc146818a_free (rtc);
		free (rtc);
	}
}

void mc146818a_set_irq_fct (mc146818a_t *rtc, void *ext, void *fct)
{
	rtc->irq_ext = ext;
	rtc->irq_fct = fct;
}

void mc146818a_set_input_clock (mc146818a_t *rtc, unsigned long val)
{
	rtc->clock_input = val;
	rtc->clock = 0;
	rtc->clock_uip = val - (val / 512);
}

int mc146818a_set_fname (mc146818a_t *rtc, const char *fname)
{
	unsigned n;

	if (rtc->fname != NULL) {
		free (rtc->fname);
		rtc->fname = NULL;
	}

	if (fname == NULL) {
		return (0);
	}

	n = strlen (fname);

	if ((rtc->fname = malloc (n + 1)) == NULL) {
		return (1);
	}

	memcpy (rtc->fname, fname, n + 1);

	return (0);
}

int mc146818a_load (mc146818a_t *rtc)
{
	int  r;
	FILE *fp;

	if (rtc->fname == NULL) {
		return (1);
	}

	if ((fp = fopen (rtc->fname, "rb")) == NULL) {
		return (1);
	}

	r = 0;

	if (fread (rtc->data, 1, 64, fp) != 64) {
		r = 1;
	}

	fclose (fp);

	return (r);
}

int mc146818a_save (mc146818a_t *rtc)
{
	int  r;
	FILE *fp;

	if (rtc->fname == NULL) {
		return (1);
	}

	if ((fp = fopen (rtc->fname, "wb")) == NULL) {
		return (1);
	}

	r = 0;

	if (fwrite (rtc->data, 1, 64, fp) != 64) {
		r = 1;
	}

	fclose (fp);

	return (r);
}

void mc146818a_set_century_offset (mc146818a_t *rtc, unsigned ofs)
{
	rtc->century_offset = ofs;
}

static
void rtc_update_irq (mc146818a_t *rtc)
{
	unsigned char val;

	val = rtc->data[0x0c];

	if (rtc->data[0x0b] & rtc->data[0x0c] & 0x70) {
		val |= MC146818A_REGC_IRQF;
	}
	else {
		val &= ~MC146818A_REGC_IRQF;
	}

	if (rtc->data[0x0c] != val) {
		rtc->data[0x0c] = val;

		if (rtc->irq_fct != NULL) {
			rtc->irq_fct (rtc->irq_ext, (val & MC146818A_REGC_IRQF) != 0);
		}
	}
}

static
void rtc_set_val (mc146818a_t *rtc, unsigned addr, unsigned char val)
{
	if (rtc->data[0x0b] & MC146818A_REGB_DM) {
		rtc->data[addr] = val;
	}
	else {
		rtc->data[addr] = ((val / 10) << 4) | (val % 10);
	}
}

static
unsigned rtc_get_val (mc146818a_t *rtc, unsigned addr)
{
	unsigned val;

	if (rtc->data[0x0b] & MC146818A_REGB_DM) {
		val = rtc->data[addr];
	}
	else {
		val = 10 * (rtc->data[addr] >> 4) + (rtc->data[addr] & 15);
	}

	return (val);
}

static
void rtc_update_alarm (mc146818a_t *rtc)
{
	int           val;
	unsigned char *reg;

	reg = rtc->data;

	val = 1;

	if ((reg[0] != reg[1]) && ((reg[1] & 0xc0) != 0xc0)) {
		val = 0;
	}

	if ((reg[2] != reg[3]) && ((reg[3] & 0xc0) != 0xc0)) {
		val = 0;
	}

	if ((reg[4] != reg[5]) && ((reg[5] & 0xc0) != 0xc0)) {
		val = 0;
	}

	if (val) {
		reg[0x0c] |= MC146818A_REGC_AF;
		rtc_update_irq (rtc);
	}
}

static
void rtc_update_clock (mc146818a_t *rtc)
{
	unsigned yrs, mon, day, wdy, len;
	unsigned hrs, min, sec;

	static unsigned char month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	sec = rtc_get_val (rtc, 0) + 1;

	if (sec < 60) {
		rtc_set_val (rtc, 0, sec);
		return;
	}
	else {
		rtc_set_val (rtc, 0, 0);
	}

	min = rtc_get_val (rtc, 2) + 1;

	if (min < 60) {
		rtc_set_val (rtc, 2, min);
		return;
	}
	else {
		rtc_set_val (rtc, 2, 0);
	}

	hrs = rtc_get_val (rtc, 4) + 1;

	if (hrs < 24) {
		rtc_set_val (rtc, 4, hrs);
		return;
	}
	else {
		rtc_set_val (rtc, 4, 0);
	}

	wdy = rtc_get_val (rtc, 6) + 1;

	if (wdy > 7) {
		wdy = 1;
	}

	rtc_set_val (rtc, 6, wdy);

	mon = rtc_get_val (rtc, 8);
	yrs = rtc_get_val (rtc, 9);

	if ((mon >= 1) && (mon <= 12)) {
		len = month[mon - 1];
	}
	else {
		len = 0;
	}

	if ((mon == 2) && ((yrs & 3) == 0)) {
		len += 1;
	}

	day = rtc_get_val (rtc, 7) + 1;

	if (day <= len) {
		rtc_set_val (rtc, 7, day);
		return;
	}
	else {
		rtc_set_val (rtc, 7, 1);
	}

	mon += 1;

	if (mon < 12) {
		rtc_set_val (rtc, 8, mon);
		return;
	}
	else {
		rtc_set_val (rtc, 8, 1);
	}

	yrs += 1;
	rtc_set_val (rtc, 9, yrs % 100);
}

#if DEBUG_MC146818A >= 1
static
void rtc_print_clock (mc146818a_t *rtc)
{
	int      irq, af;
	unsigned yrs, mon, day, wdy;
	unsigned hrs, min, sec;

	sec = rtc_get_val (rtc, 0);
	min = rtc_get_val (rtc, 2);
	hrs = rtc_get_val (rtc, 4);
	wdy = rtc_get_val (rtc, 6);
	day = rtc_get_val (rtc, 7);
	mon = rtc_get_val (rtc, 8);
	yrs = rtc_get_val (rtc, 9);

	irq = (rtc->data[0x0c] & MC146818A_REGC_IRQF) != 0;
	af = (rtc->data[0x0c] & MC146818A_REGC_AF) != 0;

	fprintf (stderr,
		"MC146818A: IRQ=%d AF=%d %02u-%02u-%02u %02u:%02u:%02u WD=%u\n",
		irq, af,
		yrs, mon, day, hrs, min, sec, wdy
	);
}
#endif

void mc146818a_set_date (mc146818a_t *rtc, unsigned yrs, unsigned mon, unsigned day)
{
	rtc_set_val (rtc, 6, 0);
	rtc_set_val (rtc, 7, day);
	rtc_set_val (rtc, 8, mon);
	rtc_set_val (rtc, 9, yrs % 100);

	if (rtc->century_offset > 0) {
		rtc_set_val (rtc, rtc->century_offset, yrs / 100);
	}
}

void mc146818a_set_time (mc146818a_t *rtc, unsigned hrs, unsigned min, unsigned sec)
{
	rtc_set_val (rtc, 0, sec);
	rtc_set_val (rtc, 2, min);
	rtc_set_val (rtc, 4, hrs);
}

void mc146818a_set_time_now (mc146818a_t *rtc)
{
	time_t    t;
	struct tm *tm;

	t = time (NULL);
	tm = localtime (&t);

	rtc_set_val (rtc, 0, tm->tm_sec);
	rtc_set_val (rtc, 2, tm->tm_min);
	rtc_set_val (rtc, 4, tm->tm_hour);
	rtc_set_val (rtc, 6, tm->tm_wday + 1);
	rtc_set_val (rtc, 7, tm->tm_mday);
	rtc_set_val (rtc, 8, tm->tm_mon + 1);
	rtc_set_val (rtc, 9, tm->tm_year % 100);

	if (rtc->century_offset > 0) {
		rtc_set_val (rtc, rtc->century_offset, (1900 + tm->tm_year) / 100);
	}
}

static
void rtc_get_uint (const char **str, unsigned *val)
{
	const char *s;
	unsigned   v;

	s = *str;
	v = 0;

	while ((*s < '0') || (*s > '9')) {
		if (*s == 0) {
			*str = s;
			*val = 0;
			return;
		}

		s += 1;
	}

	while ((*s >= '0') && (*s <= '9')) {
		v = 10 * v + (*s - '0');
		s += 1;
	}

	*str = s;
	*val = v;
}

void mc146818a_set_time_string (mc146818a_t *rtc, const char *str)
{
	unsigned i;
	unsigned val[6];

	if (str == NULL) {
		mc146818a_set_time_now (rtc);
		return;
	}

	for (i = 0; i < 6; i++) {
		rtc_get_uint (&str, &val[i]);
	}

	mc146818a_set_date (rtc, val[0], val[1], val[2]);
	mc146818a_set_time (rtc, val[3], val[4], val[5]);
}

unsigned char mc146818a_get_uint8 (mc146818a_t *rtc, unsigned long addr)
{
	unsigned char val;

	if (addr == 0x0a) {
		val = rtc->data[0x0a];
	}
	else if (addr == 0x0c) {
		val = rtc->data[0x0c];

		/* clear interrupt flags */
		rtc->data[0x0c] &= 0x8f;

		rtc_update_irq (rtc);
	}
	else if (addr == 0x0d) {
		rtc->data[0x0d] = 0x80;
		val = 0x80;
	}
	else if (addr < rtc->cnt) {
		val = rtc->data[addr & 0x3f];
	}
	else {
		val = 0;
	}

#if DEBUG_MC146818A >= 2
	fprintf (stderr, "RTC: %02lX -> %02X\n", addr, val);
#endif

	return (val);
}

unsigned short mc146818a_get_uint16 (mc146818a_t *rtc, unsigned long addr)
{
	return (0);
}

unsigned long mc146818a_get_uint32 (mc146818a_t *rtc, unsigned long addr)
{
	return (0);
}

void mc146818a_set_uint8 (mc146818a_t *rtc, unsigned long addr, unsigned char val)
{
#if DEBUG_MC146818A >= 2
	fprintf (stderr, "RTC: %02lX <- %02X\n", addr, val);
#endif

	switch (addr) {
	case 0x0a:
		rtc->data[0x0a] &= MC146818A_REGA_UIP;
		rtc->data[0x0a] |= val & ~MC146818A_REGA_UIP;
		break;

	case 0x0b:
		rtc->data[0x0b] = val;

		if (val & MC146818A_REGB_SET) {
			rtc->data[0x0a] &= ~MC146818A_REGA_UIP;
		}
		break;

	case 0x0c:
	case 0x0d:
		break;

	default:
		if (addr < rtc->cnt) {
			rtc->data[addr] = val;
		}
		break;
	}
}

void mc146818a_set_uint16 (mc146818a_t *rtc, unsigned long addr, unsigned short val)
{
}

void mc146818a_set_uint32 (mc146818a_t *rtc, unsigned long addr, unsigned long val)
{
}

void mc146818a_reset (mc146818a_t *rtc)
{
	rtc->data[0x0b] &= 0x8f;

	rtc_update_irq (rtc);
}

void mc146818a_clock (mc146818a_t *rtc, unsigned long cnt)
{
	unsigned div;

	if (rtc->clock_input == 0) {
		return;
	}

	div = (rtc->data[0x0a] >> 4) & 7;

	if (div > 2) {
		return;
	}

	if (rtc->data[0x0b] & MC146818A_REGB_SET) {
		return;
	}

	rtc->clock = (rtc->clock + cnt) % rtc->clock_input;

	if (rtc->clock < rtc->clock_uip) {
		if (rtc->data[0x0a] & MC146818A_REGA_UIP) {
			rtc_update_clock (rtc);
			rtc_update_alarm (rtc);

#if DEBUG_MC146818A >= 1
			rtc_print_clock (rtc);
#endif

			rtc->data[0x0a] &= ~MC146818A_REGA_UIP;
			rtc->data[0x0c] |= MC146818A_REGC_UF;

			rtc_update_irq (rtc);
		}
		return;
	}
	else {
		if ((rtc->data[0x0a] & MC146818A_REGA_UIP) == 0) {
			rtc->data[0x0a] |= MC146818A_REGA_UIP;
		}
	}
}
