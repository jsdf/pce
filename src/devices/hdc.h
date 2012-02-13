/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/hdc.h                                            *
 * Created:     2011-09-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2012 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_HDC_H
#define PCE_DEVICES_HDC_H 1


#include <devices/device.h>
#include <devices/memory.h>

#include <drivers/block/block.h>


typedef struct {
	unsigned       drive;

	unsigned char  sense[4];

	unsigned short max_c;
	unsigned short max_h;
	unsigned short max_s;
} hdc_drive_t;


typedef struct hdc_t {
	mem_blk_t      blk;

	unsigned char  status;
	unsigned char  config;
	unsigned char  mask;

	unsigned char  result;

	unsigned short cmd_idx;
	unsigned short cmd_cnt;
	unsigned char  cmd[6];

	unsigned short buf_idx;
	unsigned short buf_cnt;
	unsigned char  buf[516];

	unsigned char  config_params[64];

	unsigned       sectors;

	hdc_drive_t    drv[2];

	struct {
		unsigned short d;
		unsigned short c;
		unsigned short h;
		unsigned short s;
		unsigned short n;
	} id;

	disks_t        *dsks;

	unsigned long  delay;

	void           (*cont) (struct hdc_t *hdc);

	unsigned char  irq_val;
	void           *irq_ext;
	void           (*irq) (void *ext, unsigned char val);

	unsigned char  dreq_val;
	void           *dreq_ext;
	void           (*dreq) (void *ext, unsigned char val);
} hdc_t;


unsigned char hdc_read_data (hdc_t *hdc);
void hdc_write_data (hdc_t *hdc, unsigned char val);
void hdc_set_tc (hdc_t *hdc, unsigned char val);

hdc_t *hdc_new (unsigned long addr);
void hdc_del (hdc_t *hdc);

void hdc_mem_add_io (hdc_t *hdc, memory_t *io);
void hdc_mem_rmv_io (hdc_t *hdc, memory_t *io);

void hdc_set_irq_fct (hdc_t *hdc, void *ext, void *fct);
void hdc_set_dreq_fct (hdc_t *hdc, void *ext, void *fct);

void hdc_set_config (hdc_t *hdc, unsigned val);

void hdc_set_sectors (hdc_t *hdc, unsigned val);

void hdc_set_disks (hdc_t *hdc, disks_t *dsks);

void hdc_set_drive (hdc_t *hdc, unsigned hdcdrv, unsigned drive);

unsigned hdc_get_drive (hdc_t *hdc, unsigned hdcdrv);

/*!***************************************************************************
 * @short Set the configuration parameters ID
 * @param id   An array of 8 bytes that will be returned as the first 8 bytes
 *             by the get configuration parameters command.
 * @param cnt  The number of bytes in id. If cnt < 8 then the remaining bytes
 *             will be set to 0.
 *****************************************************************************/
void hdc_set_config_id (hdc_t *hdc, const unsigned char *id, unsigned cnt);

void hdc_reset (hdc_t *hdc);

void hdc_clock (hdc_t *hdc, unsigned long cnt);


#endif
