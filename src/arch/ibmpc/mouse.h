/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/mouse.h                                       *
 * Created:     2003-08-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2010 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MOUSE_H
#define PCE_MOUSE_H 1


#include <chipset/82xx/e8250.h>
#include <devices/memory.h>
#include <libini/libini.h>


typedef struct {
	mem_blk_t     port;
	e8250_t       uart;

	int           dtr;
	int           rts;

	int           dx;
	int           dy;
	unsigned      button;

	int           scale_x[3];
	int           scale_y[3];

	unsigned long reset_cntr;
} mouse_t;


void mse_init (mouse_t *mse, unsigned long base, ini_sct_t *sct);
mouse_t *mse_new (unsigned long base, ini_sct_t *sct);
void mse_free (mouse_t *mse);
void mse_del (mouse_t *mse);

mem_blk_t *mse_get_reg (mouse_t *mse);

void mse_set (mouse_t *mse, int dx, int dy, unsigned but);

void mse_uart_setup (mouse_t *mse, unsigned char val);
void mse_uart_out (mouse_t *mse, unsigned char val);
void mse_uart_inp (mouse_t *mse, unsigned char val);

void mse_clock (mouse_t *mse, unsigned long cnt);


#endif
