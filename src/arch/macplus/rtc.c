/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/rtc.c                                       *
 * Created:     2007-11-16 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2011 Hampa Hug <hampa@hampa.ch>                     *
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
#include <time.h>


void mac_rtc_init (mac_rtc_t *rtc)
{
	rtc->set_data_ext = NULL;
	rtc->set_data = NULL;
	rtc->set_data_val = 0;

	rtc->set_osi_ext = NULL;
	rtc->set_osi = NULL;
	rtc->set_osi_val = 0;

	rtc->data_out = 0;
	rtc->state = 0;
	rtc->bitcnt = 0;
	rtc->sigval = 0;

	rtc->realtime = 0;
	rtc->clkcnt = 0;

	rtc->reg_wp = 0;
	rtc->reg_test = 0;

	rtc->clock = 0x00000000;

	mac_rtc_set_defaults (rtc);
}

void mac_rtc_free (mac_rtc_t *rtc)
{
}

void mac_rtc_set_data_fct (mac_rtc_t *rtc, void *ext, void *fct)
{
	rtc->set_data_ext = ext;
	rtc->set_data = fct;
}

void mac_rtc_set_osi_fct (mac_rtc_t *rtc, void *ext, void *fct)
{
	rtc->set_osi_ext = ext;
	rtc->set_osi = fct;
}

void mac_rtc_set_realtime (mac_rtc_t *rtc, int realtime)
{
	rtc->realtime = (realtime != 0);
}

void mac_rtc_set_defaults (mac_rtc_t *rtc)
{
	unsigned i;

	for (i = 0; i < 256; i++) {
		rtc->data[i] = 0;
	}

	rtc->data[0x10] = 0xa8;
	rtc->data[0x13] = 0x22;	/* serial port */
	rtc->data[0x1e] = 0x64; /* key repeat delay / rate */
	rtc->data[0x08] = 0x18; /* mouse speed / speaker volume */
	rtc->data[0x09] = 0x88; /* double click / caret flash */
	rtc->data[0x0b] = 0x20; /* mouse tracking / startup drive / menu blink */
}

int mac_rtc_load_file (mac_rtc_t *rtc, const char *fname)
{
	FILE *fp;

	fp = fopen (fname, "rb");

	if (fp == NULL) {
		return (1);
	}

	if (fread (rtc->data, 1, 256, fp) != 256) {
		fclose (fp);
		return (1);
	}

	fclose (fp);

	rtc->reg_wp = 0x80;
	rtc->reg_test = 0x00;

	return (0);
}

int mac_rtc_save_file (mac_rtc_t *rtc, const char *fname)
{
	FILE *fp;

	fp = fopen (fname, "wb");

	if (fp == NULL) {
		return (1);
	}

	if (fwrite (rtc->data, 1, 256, fp) != 256) {
		fclose (fp);
		return (1);
	}

	fclose (fp);

	return (0);
}

static
unsigned long mac_rtc_get_current_time (mac_rtc_t *rtc)
{
	time_t        ut;
	unsigned long mt;

	ut = time (NULL);

	mt = (unsigned long) ut;

	/* 1970-01-01 00:00:00 */
	mt += 2082844800;

	return (mt);
}

void mac_rtc_set_current_time (mac_rtc_t *rtc)
{
	rtc->clock = mac_rtc_get_current_time (rtc);
}

static
unsigned long mac_rtc_get_timezone (mac_rtc_t *rtc)
{
	unsigned long tz;

	tz = rtc->data[0xed];
	tz = (tz << 8) | rtc->data[0xee];
	tz = (tz << 8) | rtc->data[0xef];

	if (tz & 0x800000) {
		tz |= 0xff000000;
	}

	return (tz);
}

static
void mac_rtc_set_data (mac_rtc_t *rtc, unsigned char val)
{
	/* data must always be set because it switches from input to output */

	rtc->set_data_val = (val != 0);

	if (rtc->set_data != NULL) {
		rtc->set_data (rtc->set_data_ext, rtc->set_data_val);
	}
}

static
void mac_rtc_set_osi (mac_rtc_t *rtc, unsigned char val)
{
	val = (val != 0);

	if (rtc->set_osi_val == val) {
		return;
	}

	rtc->set_osi_val = val;

	if (rtc->set_osi != NULL) {
		rtc->set_osi (rtc->set_osi_ext, val);
	}
}

static
void mac_rtc_cmd1_read (mac_rtc_t *rtc)
{
	unsigned char reg;

	reg = (rtc->cmd1 >> 2) & 0x1f;

	if ((rtc->cmd1 & 0xe3) == 0x81) {
		rtc->shift = (rtc->clock >> (8 * (reg & 3))) & 0xff;
	}
	else if ((rtc->cmd1 & 0xf3) == 0xa1) {
		rtc->shift = rtc->data[8 + ((rtc->cmd1 >> 2) & 3)];
	}
	else if ((rtc->cmd1 & 0xc3) == 0xc1) {
		rtc->shift = rtc->data[16 + ((rtc->cmd1 >> 2) & 15)];
	}
	else {
		rtc->shift = 0x00;
	}

#ifdef DEBUG_RTC
	mac_log_deb ("rtc: read command 1: %02X (%02X)\n",
		rtc->cmd1, rtc->shift
	);
#endif
}

static
void mac_rtc_cmd1_write (mac_rtc_t *rtc)
{
#ifdef DEBUG_RTC
	mac_log_deb ("rtc: write command 1: %02X (%02X)\n",
		rtc->cmd1, rtc->shift
	);
#endif

	if (rtc->cmd1 == 0x35) {
		rtc->reg_wp = rtc->shift & 0x80;
		return;
	}

	if (rtc->reg_wp & 0x80) {
		return;
	}

	if ((rtc->cmd1 & 0xe3) == 0x01) {
		unsigned      bit;
		unsigned long val;

		bit = 8 * ((rtc->cmd1 >> 2) & 3);
		val = rtc->shift & 0xff;

		rtc->clock &= ~(0x000000ffUL << bit);
		rtc->clock |= val << bit;
	}
	else if ((rtc->cmd1 & 0xf3) == 0x21) {
		rtc->data[8 + ((rtc->cmd1 >> 2) & 3)] = rtc->shift;
	}
	else if (rtc->cmd1 == 0x31) {
		rtc->reg_test = rtc->shift;
	}
	else if ((rtc->cmd1 & 0xc3) == 0x41) {
		rtc->data[16 + ((rtc->cmd1 >> 2) & 15)] = rtc->shift;
	}
}

static
void mac_rtc_cmd2_read (mac_rtc_t *rtc)
{
	unsigned addr;

	addr = ((rtc->cmd1 & 7) << 5) | ((rtc->cmd2 >> 2) & 0x1f);

	if (addr < 256) {
		rtc->shift = rtc->data[addr];
	}
	else {
		rtc->shift = 0;
	}

#ifdef DEBUG_RTC
	mac_log_deb ("rtc: read command 2: %02X %02X S=%X A=%02X (%02X)\n",
		rtc->cmd1, rtc->cmd2, rtc->cmd1 & 7, (rtc->cmd2 >> 2) & 0x1f,
		rtc->shift
	);
#endif
}

static
void mac_rtc_cmd2_write (mac_rtc_t *rtc)
{
	unsigned addr;

#ifdef DEBUG_RTC
	mac_log_deb ("rtc: write command 2: %02X %02X S=%X A=%02X (%02X)\n",
		rtc->cmd1, rtc->cmd2, rtc->cmd1 & 7, (rtc->cmd2 >> 2) & 0x1f,
		rtc->shift
	);
#endif

	if (rtc->reg_wp & 0x80) {
		return;
	}

	addr = ((rtc->cmd1 & 7) << 5) | ((rtc->cmd2 >> 2) & 0x1f);

	if (addr < 256) {
		rtc->data[addr] = rtc->shift;
	}
}

void mac_rtc_set_uint8 (mac_rtc_t *rtc, unsigned char val)
{
	unsigned char dif;

	dif = rtc->sigval ^ val;
	rtc->sigval = val;

	if (val & 0x04) {
		/* serial disabled */
		rtc->state = 0;
		rtc->data_out = 0;
		rtc->bitcnt = 0;
		return;
	}

	if ((dif & ~val & 0x02) == 0) {
		return;
	}

	/* clock went low */

	if (rtc->data_out) {
		/* send a byte to the cpu */
		mac_rtc_set_data (rtc, rtc->shift & 0x80);

		rtc->shift = (rtc->shift << 1) | ((rtc->shift >> 7) & 0x01);

		rtc->bitcnt += 1;

		if (rtc->bitcnt >= 8) {
			rtc->bitcnt = 0;
			rtc->data_out = 0;
			rtc->state = 0;
		}
	}
	else {
		/* receive a byte from the cpu */
		rtc->shift = (rtc->shift << 1) | (val & 0x01);

		rtc->bitcnt += 1;

		if (rtc->bitcnt >= 8) {
			if (rtc->state == 0) {
				/* cmd1 */
				rtc->cmd1 = rtc->shift;

				if ((rtc->cmd1 & 0x78) == 0x38) {
					/* extended command */
					rtc->state = 2;
				}
				else if (rtc->cmd1 & 0x80) {
					/* read command */
					mac_rtc_cmd1_read (rtc);
					rtc->state = 0;
					rtc->data_out = 1;
				}
				else {
					rtc->state = 1;
				}
			}
			else if (rtc->state == 1) {
				/* data byte for cmd1 */
				mac_rtc_cmd1_write (rtc);
				rtc->state = 0;
			}
			else if (rtc->state == 2) {
				/* cmd2 */
				rtc->cmd2 = rtc->shift;
				if (rtc->cmd1 & 0x80) {
					mac_rtc_cmd2_read (rtc);
					rtc->state = 0;
					rtc->data_out = 1;
				}
				else {
					rtc->state = 3;
				}
			}
			else if (rtc->state == 3) {
				/* data byte for cmd2 */
				mac_rtc_cmd2_write (rtc);
				rtc->state = 0;
			}

			rtc->bitcnt = 0;
		}
	}
}

void mac_rtc_clock (mac_rtc_t *rtc, unsigned long n)
{
	unsigned long old;

	old = rtc->clock;

	if (rtc->realtime) {
		rtc->clock = mac_rtc_get_current_time (rtc);
		rtc->clock += mac_rtc_get_timezone (rtc);
		rtc->clock &= 0xffffffff;
	}
	else {
		rtc->clkcnt += n;

		if (rtc->clkcnt > MAC_CPU_CLOCK) {
			rtc->clkcnt -= MAC_CPU_CLOCK;

			rtc->clock += 1;
		}
	}

	if (rtc->clock != old) {
#ifdef DEBUG_RTC
			mac_log_deb ("rtc: osi (%lu)\n", rtc->clkcnt);
#endif

			mac_rtc_set_osi (rtc, 1);
			mac_rtc_set_osi (rtc, 0);
	}
}
