/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e68901.h                                         *
 * Created:     2011-06-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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


/* MFP 68901 */


#ifndef PCE_CHIPSET_E68901_H
#define PCE_CHIPSET_E68901_H 1


typedef struct {
	unsigned short int_mask;
	unsigned char  cr;
	unsigned char  dr[2];

	char           inp;
	char           out;

	unsigned       clk_div_inp;
	unsigned       clk_div_set;

	unsigned       clk_div;
	unsigned       clk_val;
} e68901_timer_t;


typedef struct {
	unsigned       addr_shift;

	unsigned char  gpip_xor;
	unsigned char  gpip_inp;
	unsigned char  gpip_val;
	unsigned char  gpip_aer;
	unsigned char  gpip_ddr;

	unsigned short irr1;
	unsigned short irr2;
	unsigned short ier;
	unsigned short ipr;
	unsigned short isr;
	unsigned short imr;
	unsigned char  ivr;
	unsigned char  vec;

	unsigned char  ucr;
	unsigned char  rsr[2];
	unsigned char  tsr[2];
	unsigned char  rdr[2];
	unsigned char  tdr[2];
	unsigned       recv_clk_cnt;
	unsigned       recv_clk_max;
	unsigned       send_clk_cnt;
	unsigned       send_clk_max;
	unsigned char  usart_timer;

	e68901_timer_t timer[4];

	void           *recv_ext;
	int            (*recv_fct) (void *ext, unsigned char *val);

	void           *send_ext;
	int            (*send_fct) (void *ext, unsigned char val);

	unsigned char  irq_val;
	void           *irq_ext;
	void           (*irq_fct) (void *ext, unsigned char val);
} e68901_t;


void e68901_init (e68901_t *mfp, unsigned addr_shift);
void e68901_free (e68901_t *mfp);

void e68901_set_irq_fct (e68901_t *mfp, void *ext, void *fct);
void e68901_set_recv_fct (e68901_t *mfp, void *ext, void *fct);
void e68901_set_send_fct (e68901_t *mfp, void *ext, void *fct);

void e68901_set_usart_timer (e68901_t *mfp, unsigned idx);

void e68901_set_clk_div (e68901_t *mfp, unsigned div);

unsigned e68901_inta (e68901_t *mfp);

void e68901_set_inp (e68901_t *mfp, unsigned char val);
void e68901_set_inp_n (e68901_t *mfp, unsigned idx, unsigned char val);
void e68901_set_inp_4 (e68901_t *mfp, unsigned char val);
void e68901_set_inp_5 (e68901_t *mfp, unsigned char val);

void e68901_set_tbi (e68901_t *mfp, unsigned char val);

unsigned char e68901_get_uint8 (e68901_t *mfp, unsigned long addr);
unsigned short e68901_get_uint16 (e68901_t *mfp, unsigned long addr);
unsigned long e68901_get_uint32 (e68901_t *mfp, unsigned long addr);

void e68901_set_uint8 (e68901_t *mfp, unsigned long addr, unsigned char val);
void e68901_set_uint16 (e68901_t *mfp, unsigned long addr, unsigned short val);
void e68901_set_uint32 (e68901_t *mfp, unsigned long addr, unsigned long val);

int e68901_receive (e68901_t *mfp, unsigned char val);

void e68901_reset (e68901_t *mfp);

void e68901_clock_usart (e68901_t *mfp, unsigned n);

void e68901_clock (e68901_t *mfp, unsigned n);


#endif
