/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/chipset/e8253.h                                        *
 * Created:       2001-05-04 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-02-15 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2001-2004 Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id$ */


/* PIT 8253 / 8254 */


#ifndef PCE_E8253_H
#define PCE_E8253_H 1


typedef void (*e8253_set_out_f) (void *ext, unsigned char val);


typedef struct {
  /* couner register */
  unsigned char   cr[2];
  unsigned char   cr_wr;

  /* output latch */
  unsigned char   ol[2];
  unsigned char   ol_rd;
  unsigned char   cnt_rd;

  unsigned char   sr;
  unsigned short  rw;
  unsigned short  mode;
  unsigned short  bcd;

  short           counting;

  unsigned char   gate;

  void            *out_ext;
  e8253_set_out_f out;
  unsigned char   out_val;

  unsigned short val;
} e8253_counter_t;


typedef struct {
  e8253_counter_t counter[3];
} e8253_t;


unsigned char e8253_cnt_get_uint8 (e8253_counter_t *cnt);
void e8253_cnt_set_uint8 (e8253_counter_t *cnt, unsigned char val);
unsigned char e8253_cnt_get_out (e8253_counter_t *cnt);
void e8253_cnt_set_gate (e8253_counter_t *cnt, unsigned char val);
void e8253_counter_clock (e8253_counter_t *cnt, unsigned n);
void e8253_counter_init (e8253_counter_t *cnt);


void e8253_init (e8253_t *pit);
e8253_t *e8253_new (void);
void e8253_free (e8253_t *pit);
void e8253_del (e8253_t *pit);

void e8253_set_out (e8253_t *pit, unsigned cntr, void *ext, e8253_set_out_f set);

void e8253_set_gate (e8253_t *pit, unsigned cntr, unsigned char val);
void e8253_set_gate0 (e8253_t *pit, unsigned char val);
void e8253_set_gate1 (e8253_t *pit, unsigned char val);
void e8253_set_gate2 (e8253_t *pit, unsigned char val);

unsigned char e8253_get_uint8 (e8253_t *pit, unsigned long addr);
unsigned short e8253_get_uint16 (e8253_t *pit, unsigned long addr);
unsigned long e8253_get_uint32 (e8253_t *pit, unsigned long addr);

void e8253_set_uint8 (e8253_t *pit, unsigned long addr, unsigned char val);
void e8253_set_uint16 (e8253_t *pit, unsigned long addr, unsigned short val);
void e8253_set_uint32 (e8253_t *pit, unsigned long addr, unsigned long val);

void e8253_clock (e8253_t *pit, unsigned n);



#endif
