/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/sony.h                                      *
 * Created:     2007-11-15 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_SONY_H
#define PCE_MACPLUS_SONY_H 1


#include <devices/memory.h>
#include <drivers/block/block.h>


#define SONY_DRIVES 8


typedef struct {
	char          open;
	char          patched;
	char          enable;

	memory_t      *mem;
	disks_t       *dsks;

	unsigned      delay_val[SONY_DRIVES];
	unsigned      delay_cnt[SONY_DRIVES];

	unsigned long check_addr;
	unsigned long icon_addr[2];

	unsigned long tag_buf;

	char          format_hd_as_dd;

	unsigned      format_cnt;
	unsigned long format_list[16];

	unsigned long pcex_addr;
	unsigned long sony_addr;
	unsigned char patch_buf[64];

	unsigned long d0;
	unsigned long a0;
	unsigned long a1;
	unsigned long pc;
} mac_sony_t;


void mac_sony_init (mac_sony_t *sony, int enable);
void mac_sony_free (mac_sony_t *sony);

void mac_sony_set_mem (mac_sony_t *sony, memory_t *mem);

void mac_sony_set_disks (mac_sony_t *sony, disks_t *dsks);

void mac_sony_patch (mac_sony_t *sony);

void mac_sony_set_delay (mac_sony_t *sony, unsigned drive, unsigned delay);

void mac_sony_insert (mac_sony_t *sony, unsigned drive);

int mac_sony_check (mac_sony_t *sony);

int mac_sony_hook (mac_sony_t *sony, unsigned val);

void mac_sony_reset (mac_sony_t *sony);


#endif
