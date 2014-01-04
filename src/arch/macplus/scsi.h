/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/scsi.h                                      *
 * Created:     2007-11-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2014 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_SCSI_H
#define PCE_MACPLUS_SCSI_H 1


#include <drivers/block/block.h>


typedef struct {
	int           valid;

	/* the PCE drive number */
	unsigned      drive;

	unsigned char vendor[8];
	unsigned char product[16];
} mac_scsi_dev_t;


typedef struct mac_scsi_s {
	unsigned      phase;

	unsigned char odr;
	unsigned char csd;
	unsigned char icr;
	unsigned char mr2;
	unsigned char tcr;
	unsigned char csb;
	unsigned char ser;
	unsigned char bsr;

	unsigned char status;
	unsigned char message;

	unsigned      cmd_i;
	unsigned      cmd_n;
	unsigned char cmd[16];

	unsigned long buf_i;
	unsigned long buf_n;
	unsigned long buf_max;
	unsigned char *buf;

	unsigned      sel_drv;

	unsigned long addr_mask;
	unsigned      addr_shift;

	void          (*cmd_start) (struct mac_scsi_s *scsi);
	void          (*cmd_finish) (struct mac_scsi_s *scsi);

	unsigned char  set_int_val;
	void           *set_int_ext;
	void           (*set_int) (void *ext, unsigned char val);

	mac_scsi_dev_t dev[8];

	disks_t        *dsks;
} mac_scsi_t;


void mac_scsi_init (mac_scsi_t *scsi);
void mac_scsi_free (mac_scsi_t *scsi);

void mac_scsi_set_int_fct (mac_scsi_t *scsi, void *ext, void *fct);

void mac_scsi_set_disks (mac_scsi_t *scsi, disks_t *dsks);
void mac_scsi_set_drive (mac_scsi_t *scsi, unsigned id, unsigned drive);
void mac_scsi_set_drive_vendor (mac_scsi_t *scsi, unsigned id, const char *vendor);
void mac_scsi_set_drive_product (mac_scsi_t *scsi, unsigned id, const char *product);

unsigned char mac_scsi_get_uint8 (void *ext, unsigned long addr);
unsigned short mac_scsi_get_uint16 (void *ext, unsigned long addr);

void mac_scsi_set_uint8 (void *ext, unsigned long addr, unsigned char val);
void mac_scsi_set_uint16 (void *ext, unsigned long addr, unsigned short val);

void mac_scsi_reset (mac_scsi_t *scsi);


#endif
