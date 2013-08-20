/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/rp5c15.c                                    *
 * Created:     2013-06-20 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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


#include "rp5c15.h"

#include <time.h>


void rp5c15_init (rp5c15_t *rtc)
{
	unsigned i;

	for (i = 0; i < 16; i++) {
		rtc->bank0[i] = 0;
		rtc->bank1[i] = 0;
	}

	rtc->bank1[10] = 1;
}

void rp5c15_free (rp5c15_t *rtc)
{
}

static
void rp5c15_update (rp5c15_t *rtc)
{
	time_t    tm;
	struct tm *tval;

	tm = time (NULL);
	tval = localtime (&tm);

	rtc->bank0[0] = tval->tm_sec % 10;
	rtc->bank0[1] = tval->tm_sec / 10;
	rtc->bank0[2] = tval->tm_min % 10;
	rtc->bank0[3] = tval->tm_min / 10;
	rtc->bank0[4] = tval->tm_hour % 10;
	rtc->bank0[5] = tval->tm_hour / 10;
	rtc->bank0[6] = tval->tm_wday;
	rtc->bank0[7] = tval->tm_mday % 10;
	rtc->bank0[8] = tval->tm_mday / 10;
	rtc->bank0[9] = (tval->tm_mon + 1) % 10;
	rtc->bank0[10] = (tval->tm_mon + 1) / 10;
	rtc->bank0[11] = (tval->tm_year - 80) % 10;
	rtc->bank0[12] = (tval->tm_year - 80) / 10;

	rtc->bank1[11] = (tval->tm_year - 80) % 4;
}

unsigned char rp5c15_get_uint8 (rp5c15_t *rtc, unsigned long addr)
{
	unsigned char val;

	if (addr > 15) {
		return (0);
	}

	if (addr == 13) {
		return (rtc->mode);
	}

	rp5c15_update (rtc);

	if (rtc->mode & 1) {
		val = rtc->bank1[addr];
	}
	else {
		val = rtc->bank0[addr];
	}

	return (val);
}

void rp5c15_set_uint8 (rp5c15_t *rtc, unsigned long addr, unsigned char val)
{
	if (addr > 15) {
		return;
	}

	if (rtc->mode & 1) {
		rtc->bank1[addr] = val;
	}
	else {
		rtc->bank0[addr] = val;
	}

	if (addr == 13) {
		rtc->mode = val;
	}
}
