/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/rtc.h                                         *
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


#ifndef PCE_RC759_RTC_H
#define PCE_RC759_RTC_H 1


typedef struct {
	unsigned char  read_pulse;
	unsigned char  read_data;
	unsigned short read_addr;

	unsigned char  write_pulse;
	unsigned char  write_data;
	unsigned short write_addr;

	unsigned       clk;
	unsigned char  msec[2];
	unsigned char  csec[2];
	unsigned char  sec[2];
	unsigned char  min[2];
	unsigned char  hrs[2];
	unsigned char  wday[2];
	unsigned char  mday[2];
	unsigned char  month[2];

	unsigned char  int_status;
	unsigned char  int_ctrl;

	unsigned long  input_clock;
	unsigned long  clock_div;

	unsigned char  irq_val;
	void           *irq_ext;
	void           (*irq) (void *ext, unsigned char val);
} rc759_rtc_t;


void rc759_rtc_init (rc759_rtc_t *rtc);
void rc759_rtc_free (rc759_rtc_t *rtc);

void rc759_rtc_reset (rc759_rtc_t *rtc);

void rc759_rtc_set_irq_fct (rc759_rtc_t *rtc, void *ext, void *fct);

void rc759_rtc_set_input_clock (rc759_rtc_t *rtc, unsigned long clk);

void rc759_rtc_set_time_now (rc759_rtc_t *rtc);

unsigned char rc759_rtc_get_addr (rc759_rtc_t *rtc);
void rc759_rtc_set_addr (rc759_rtc_t *rtc, unsigned char val);

void rc759_rtc_set_data (rc759_rtc_t *rtc, unsigned char val);

void rc759_rtc_clock (rc759_rtc_t *rtc, unsigned cnt);


#endif
