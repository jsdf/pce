/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/acsi.h                                      *
 * Created:     2013-06-04 by Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_ATARIST_ACSI_H
#define PCE_ATARIST_ACSI_H 1


#include <chipset/wd179x.h>
#include <devices/memory.h>
#include <drivers/block/block.h>


typedef struct {
	unsigned       cmd_cnt;
	unsigned char  cmd[8];

	unsigned char  result;
	unsigned char  sense;

	unsigned long  blk;
	unsigned short cnt;

	unsigned long  buf_idx;
	unsigned long  buf_cnt;
	unsigned char  buf[128 * 1024UL];

	unsigned short disk_id[8];

	disks_t        *dsks;

	unsigned char  drq_val;
	void           *drq_ext;
	void           (*set_drq) (void *ext, unsigned char val);

	unsigned char  irq_val;
	void           *irq_ext;
	void           (*set_irq) (void *ext, unsigned char val);
} st_acsi_t;


void st_acsi_init (st_acsi_t *acsi);
void st_acsi_free (st_acsi_t *acsi);

void st_acsi_set_drq_fct (st_acsi_t *acsi, void *ext, void *fct);
void st_acsi_set_irq_fct (st_acsi_t *acsi, void *ext, void *fct);

void st_acsi_set_disks (st_acsi_t *acsi, disks_t *dsks);
void st_acsi_set_disk_id (st_acsi_t *acsi, unsigned idx, unsigned val);

unsigned char st_acsi_get_result (st_acsi_t *acsi);

unsigned char st_acsi_get_data (st_acsi_t *acsi);
void st_acsi_set_data (st_acsi_t *acsi, unsigned char val);

void st_acsi_set_cmd (st_acsi_t *acsi, unsigned char val, int a0);

void st_acsi_reset (st_acsi_t *acsi);

void st_acsi_clock (st_acsi_t *acsi);


#endif
