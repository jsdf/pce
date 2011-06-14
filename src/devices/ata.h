/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/ata.h                                            *
 * Created:     2004-12-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2011 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_DEVICES_ATA_H
#define PCE_DEVICES_ATA_H 1


#include <devices/memory.h>

#include <drivers/block/block.h>


struct ata_dev_s;
struct ata_chn_s;


#define ATA_BUF_MAX 4096


typedef struct ata_dev_s {
	struct ata_chn_s *chn;

	unsigned char    reg_cmd;
	unsigned char    reg_status;
	unsigned char    reg_error;
	unsigned char    reg_features;
	unsigned char    reg_cyl_lo;
	unsigned char    reg_cyl_hi;
	unsigned char    reg_head;
	unsigned char    reg_sec;
	unsigned char    reg_sec_cnt;
	unsigned char    reg_dev_ctl;

	uint16_t         c;
	uint16_t         h;
	uint16_t         s;

	uint16_t         default_c;
	uint16_t         default_h;
	uint16_t         default_s;

	unsigned         multi_block_max;
	unsigned         multi_block_size;

	unsigned         buf_i;
	unsigned         buf_n;
	unsigned         buf_m;
	unsigned         buf_mode;
	uint32_t         buf_blk_i;
	uint32_t         buf_blk_n;
	unsigned         buf_mult_i;
	unsigned         buf_mult_n;
	unsigned char    buf[ATA_BUF_MAX];
	void             (*callback) (struct ata_dev_s *dev);

	char             model[64];
	char             firmware[16];
	char             serial[32];

	disk_t           *blk;
} ata_dev_t;


typedef struct ata_chn_s {
	mem_blk_t     reg_cmd;
	mem_blk_t     reg_ctl;

	void          *irq_ext;
	unsigned char irq_val;
	void          (*irq) (void *ext, unsigned char val);

	ata_dev_t     dev[2];
	ata_dev_t     *sel;
} ata_chn_t;


void ata_init (ata_chn_t *ata, unsigned long addr1, unsigned long addr2);
void ata_free (ata_chn_t *ata);

ata_chn_t *ata_new (unsigned long addr1, unsigned long addr2);
void ata_del (ata_chn_t *ata);

void ata_set_irq_f (ata_chn_t *ata, void *irq, void *ext);

void ata_set_model (ata_chn_t *ata, unsigned devi, const char *name);

void ata_set_multi_mode (ata_chn_t *ata, unsigned devi, unsigned max);

void ata_set_block (ata_chn_t *ata, disk_t *blk, unsigned devi);

unsigned char ata_cmd_get_uint8 (ata_chn_t *ata, unsigned long addr);
unsigned short ata_cmd_get_uint16 (ata_chn_t *ata, unsigned long addr);
unsigned long ata_cmd_get_uint32 (ata_chn_t *ata, unsigned long addr);

void ata_cmd_set_uint8 (ata_chn_t *ata, unsigned long addr, unsigned char val);
void ata_cmd_set_uint16 (ata_chn_t *ata, unsigned long addr, unsigned short val);
void ata_cmd_set_uint32 (ata_chn_t *ata, unsigned long addr, unsigned long val);


#endif
