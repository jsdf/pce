/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/viking.h                                    *
 * Created:     2015-09-30 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_ATARIST_VIKING_H
#define PCE_ATARIST_VIKING_H 1


#include <devices/memory.h>
#include <drivers/video/terminal.h>


typedef struct {
	memory_t            *mem;

	mem_blk_t           ram;
	unsigned char       *ptr;

	const unsigned char *src;
	unsigned char       *rgb;

	char                mod;
	unsigned            mod_y1;
	unsigned            mod_y2;

	unsigned long       input_clock;
	unsigned long       clock;

	terminal_t          *trm;
} st_viking_t;


int st_viking_init (st_viking_t *vid, unsigned long addr);
void st_viking_free (st_viking_t *vid);

st_viking_t *st_viking_new (unsigned long addr);
void st_viking_del (st_viking_t *vid);

void st_viking_set_input_clock (st_viking_t *vik, unsigned long clock);

void st_viking_set_memory (st_viking_t *vid, memory_t *mem);

void st_viking_set_terminal (st_viking_t *vid, terminal_t *trm);

void st_viking_redraw (st_viking_t *vid);

void st_viking_reset (st_viking_t *vid);

void st_viking_clock (st_viking_t *vid, unsigned cnt);


#endif
