/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/e6845.h                                          *
 * Created:     2017-08-07 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2017 Hampa Hug <hampa@hampa.ch>                          *
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


/* MC6850 CRTC */


#ifndef PCE_CHIPSET_E6845_H
#define PCE_CHIPSET_E6845_H 1


#define E6845_REG_CNT 18

#define E6845_REG_HT  0
#define E6845_REG_HD  1
#define E6845_REG_HS  2
#define E6845_REG_SW  3
#define E6845_REG_VT  4
#define E6845_REG_VA  5
#define E6845_REG_VD  6
#define E6845_REG_VS  7
#define E6845_REG_IL  8
#define E6845_REG_ML  9
#define E6845_REG_CS  10
#define E6845_REG_CE  11
#define E6845_REG_AH  12
#define E6845_REG_AL  13
#define E6845_REG_CH  14
#define E6845_REG_CL  15
#define E6845_REG_LH  16
#define E6845_REG_LL  17


typedef struct {
	unsigned      ccol;
	unsigned      crow;
	unsigned      frame;
	unsigned      ma;
	unsigned char ra;

	unsigned char hsync_cnt;
	unsigned char vsync_cnt;

	unsigned char index;
	unsigned char reg[E6845_REG_CNT];

	void          *hsync_ext;
	void          (*hsync_fct) (void *ext);

	void          *vsync_ext;
	void          (*vsync_fct) (void *ext);
} e6845_t;


#define e6845_get_ht(crt) ((crt)->reg[E6845_REG_HT])
#define e6845_get_hd(crt) ((crt)->reg[E6845_REG_HD])
#define e6845_get_hs(crt) ((crt)->reg[E6845_REG_HS])
#define e6845_get_sw(crt) ((crt)->reg[E6845_REG_SW])
#define e6845_get_vt(crt) ((crt)->reg[E6845_REG_VT])
#define e6845_get_va(crt) ((crt)->reg[E6845_REG_VA])
#define e6845_get_vd(crt) ((crt)->reg[E6845_REG_VD])
#define e6845_get_vs(crt) ((crt)->reg[E6845_REG_VS])
#define e6845_get_il(crt) ((crt)->reg[E6845_REG_IL])
#define e6845_get_ml(crt) ((crt)->reg[E6845_REG_ML])
#define e6845_get_cs(crt) ((crt)->reg[E6845_REG_CS])
#define e6845_get_ce(crt) ((crt)->reg[E6845_REG_CE])
#define e6845_get_ah(crt) ((crt)->reg[E6845_REG_AH])
#define e6845_get_al(crt) ((crt)->reg[E6845_REG_AL])
#define e6845_get_ch(crt) ((crt)->reg[E6845_REG_CH])
#define e6845_get_cl(crt) ((crt)->reg[E6845_REG_CL])
#define e6845_get_lh(crt) ((crt)->reg[E6845_REG_LH])
#define e6845_get_ll(crt) ((crt)->reg[E6845_REG_LL])


void e6845_init (e6845_t *crt);
void e6845_free (e6845_t *crt);

void e6845_set_hsync_fct (e6845_t *crt, void *ext, void *fct);
void e6845_set_vsync_fct (e6845_t *crt, void *ext, void *fct);

unsigned e6845_get_start_address (const e6845_t *crt);
unsigned e6845_get_cursor_address (const e6845_t *crt);
unsigned e6845_get_cursor_mask (e6845_t *crt, int blink);
unsigned e6845_get_vdl (const e6845_t *crt);
unsigned e6845_get_vtl (const e6845_t *crt);
int e6845_get_hde (const e6845_t *crt);
int e6845_get_vde (const e6845_t *crt);
int e6845_get_de (const e6845_t *crt);

void e6845_set_pen (e6845_t *crt);

unsigned char e6845_get_index (const e6845_t *crt);
unsigned char e6845_get_data (e6845_t *crt);
unsigned char e6845_get_uint8 (e6845_t *crt, unsigned long addr);
unsigned short e6845_get_uint16 (e6845_t *crt, unsigned long addr);

void e6845_set_index (e6845_t *crt, unsigned char val);
void e6845_set_data (e6845_t *crt, unsigned char val);
void e6845_set_uint8 (e6845_t *crt, unsigned long addr, unsigned char val);
void e6850_set_uint16 (e6845_t *crt, unsigned long addr, unsigned short val);

void e6845_reset (e6845_t *crt);

void e6845_clock (e6845_t *crt, unsigned cnt);


#endif
