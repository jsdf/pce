/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/sim6502/console.h                                   *
 * Created:     2004-05-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_SIM6502_CONSOLE_H
#define PCE_SIM6502_CONSOLE_H 1


#define CON_BUF_CNT 16

typedef void (*con_set_uint8_f) (void *ext, unsigned char val);


typedef struct {
	mem_blk_t       *io;

	void            *irq_ext;
	con_set_uint8_f irq;

	void            *msg_ext;

	unsigned char   status;
	unsigned char   data_inp;
	unsigned char   data_out;

	unsigned        buf_i;
	unsigned        buf_j;
	unsigned char   buf[CON_BUF_CNT];
} console_t;


void con_init (console_t *con, ini_sct_t *sct);
void con_free (console_t *con);

unsigned char con_get_uint8 (console_t *con, unsigned long addr);
void con_set_uint8 (console_t *con, unsigned long addr, unsigned char val);

void con_check (console_t *con);


#endif
