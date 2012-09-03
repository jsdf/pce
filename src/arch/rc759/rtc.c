/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/rtc.c                                         *
 * Created:     2012-07-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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
#include "rtc.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif


#ifndef DEBUG_RTC
#define DEBUG_RTC 0
#endif


void rc759_rtc_init (rc759_rtc_t *rtc)
{
	rtc->input_clock = 6000000;
	rtc->clock_div = rtc->input_clock / 10000;

	rtc->irq_val = 0;
	rtc->irq_ext = NULL;
	rtc->irq = NULL;
}

void rc759_rtc_free (rc759_rtc_t *rtc)
{
}

void rc759_rtc_reset (rc759_rtc_t *rtc)
{
	rtc->read_pulse = 0;
	rtc->read_addr = 0;
	rtc->read_data = 0;

	rtc->write_pulse = 0;
	rtc->write_addr = 0;
	rtc->write_data = 0;
}

void rc759_rtc_set_irq_fct (rc759_rtc_t *rtc, void *ext, void *fct)
{
	rtc->irq_ext = ext;
	rtc->irq = fct;
}

void rc759_rtc_set_input_clock (rc759_rtc_t *rtc, unsigned long clk)
{
	rtc->input_clock = clk;
	rtc->clock_div = clk / 10000;
}

void rc759_rtc_set_time_now (rc759_rtc_t *rtc)
{
	time_t    tm;
	struct tm *tval;

	tm = time (NULL);
	tval = localtime (&tm);

	rtc->msec[0] = 0;
	rtc->csec[0] = 0;
	rtc->sec[0] = tval->tm_sec;
	rtc->min[0] = tval->tm_min;
	rtc->hrs[0] = tval->tm_hour;
	rtc->wday[0] = tval->tm_wday;
	rtc->mday[0] = tval->tm_mday - 1;
	rtc->month[0] = tval->tm_mon;
}

static
void rc759_rtc_set_irq (rc759_rtc_t *rtc, int val)
{
	val = (val != 0);

	if (rtc->irq_val == val) {
		return;
	}

	rtc->irq_val = val;

	if (rtc->irq != NULL) {
		rtc->irq (rtc->irq_ext, val);
	}
}

static
unsigned char get_bcd (unsigned char val)
{
	return (16 * (val / 10) + (val % 10));
}

static
unsigned char get_bin (unsigned char val)
{
	return (10 * (val / 16) + (val % 16));
}

static
unsigned char rc759_rtc_read (rc759_rtc_t *rtc, unsigned addr)
{
	unsigned char val;
	unsigned      idx;

	if (addr == 0x10) {
		val = rtc->int_status;
		rtc->int_status = 0;

		rc759_rtc_set_irq (rtc, 0);

		return (val);
	}
	else if (addr == 0x14) {
		return (0);
	}

	idx = (addr >> 3) & 1;

	val = 0x00;

	switch (addr & 0xf7) {
	case 0:
		val = get_bcd (rtc->msec[idx]);
		break;

	case 1:
		val = get_bcd (rtc->csec[idx]);
		break;

	case 2:
		val = get_bcd (rtc->sec[idx]);
		break;

	case 3:
		val = get_bcd (rtc->min[idx]);
		break;

	case 4:
		val = get_bcd (rtc->hrs[idx]);
		break;

	case 5:
		val = get_bcd (rtc->wday[idx]);
		break;

	case 6:
		val = get_bcd (rtc->mday[idx] + 1);
		break;

	case 7:
		val = get_bcd (rtc->month[idx] + 1);
		break;

	default:
#if DEBUG_RTC >= 1
		sim_log_deb ("RTC: read %04X\n", addr);
#endif
		break;
	}

	return (val);
}

static
void rc759_rtc_write (rc759_rtc_t *rtc, unsigned addr, unsigned char val)
{
	unsigned idx;

	if (addr == 0x11) {
#if DEBUG_RTC >= 1
		sim_log_deb ("RTC: set interrupt control: %02X\n", val);
#endif
		rtc->int_ctrl = val;
		return;
	}

	idx = (addr >> 3) & 1;

	switch (addr & 0xf7) {
	case 0:
		rtc->msec[idx] = get_bin (val);
		break;

	case 1:
		rtc->csec[idx] = get_bin (val);
		break;

	case 2:
		rtc->sec[idx] = get_bin (val);
		break;

	case 3:
		rtc->min[idx] = get_bin (val);
		break;

	case 4:
		rtc->hrs[idx] = get_bin (val);
		break;

	case 5:
		rtc->wday[idx] = get_bin (val);
		break;

	case 6:
		rtc->mday[idx] = get_bin (val) - 1;
		break;

	case 7:
		rtc->month[idx] = get_bin (val) - 1;
		break;

	default:
#if DEBUG_RTC >= 1
		sim_log_deb ("RTC: write %04X <- %02X\n", addr, val);
#endif
		break;
	}
}

unsigned char rc759_rtc_get_addr (rc759_rtc_t *rtc)
{
	return (rtc->read_data);
}

void rc759_rtc_set_addr (rc759_rtc_t *rtc, unsigned char val)
{
	switch (val & 0xe0) {
	case 0x80:
		rtc->read_pulse = 0;
		rtc->read_addr = val & 0x1f;
		break;

	case 0xa0:
		rtc->read_pulse = 1;
		rtc->read_data = rc759_rtc_read (rtc, rtc->read_addr);
		break;

	case 0x00:
		rtc->write_pulse = 0;
		rtc->write_addr = val & 0x1f;
		break;

	case 0x40:
		rtc->write_pulse = 1;
		rc759_rtc_write (rtc, rtc->write_addr, rtc->write_data);
		break;

	default:
#if DEBUG_RTC >= 1
		sim_log_deb ("RTC: set addr %02X\n", val);
#endif
		break;
	}
}

void rc759_rtc_set_data (rc759_rtc_t *rtc, unsigned char val)
{
	rtc->write_data = val;
}

void rc759_rtc_clock (rc759_rtc_t *rtc, unsigned cnt)
{
	rtc->clk += cnt;

	if (rtc->clk < rtc->clock_div) {
		return;
	}

	rtc->clk -= rtc->clock_div;
	rtc->msec[0] += 1;

	if (rtc->msec[0] < 100) {
		return;
	}

	rtc->msec[0] -= 100;
	rtc->csec[0] += 1;

	if (rtc->csec[0] < 100) {
		return;
	}

	rtc->csec[0] -= 100;
	rtc->sec[0] += 1;

	if (rtc->int_ctrl & 0x04) {
		rtc->int_status |= 0x04;
		rc759_rtc_set_irq (rtc, 1);
	}

	if (rtc->sec[0] < 60) {
		return;
	}

	rtc->sec[0] -= 60;
	rtc->min[0] += 1;

	if (rtc->int_ctrl & 0x08) {
		rtc->int_status |= 0x08;
		rc759_rtc_set_irq (rtc, 1);
	}

	if (rtc->min[0] < 60) {
		return;
	}

	rtc->min[0] -= 60;
	rtc->hrs[0] += 1;

	if (rtc->int_ctrl & 0x10) {
		rtc->int_status |= 0x10;
		rc759_rtc_set_irq (rtc, 1);
	}

	if (rtc->hrs[0] < 24) {
		return;
	}

	rtc->hrs[0] -= 24;
	rtc->wday[0] += 1;
	rtc->mday[0] += 1;
}
