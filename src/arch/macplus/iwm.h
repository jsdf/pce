/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/iwm.h                                       *
 * Created:     2007-11-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2013 Hampa Hug <hampa@hampa.ch>                     *
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


#ifndef PCE_MACPLUS_IWM_H
#define PCE_MACPLUS_IWM_H 1


#include <drivers/block/block.h>
#include <drivers/pri/pri.h>


#define MAC_IWM_DRIVES    3
#define MAC_IWM_CYLINDERS 80
#define MAC_IWM_HEADS     2


typedef struct {
	unsigned        drive;

	char            *fname;

	disks_t         *dsks;
	unsigned        diskid;

	pri_img_t       *img;

	char            use_fname;
	char            locked;
	char            auto_rotate;

	unsigned        cylinders;
	unsigned        heads;

	char            step_direction;
	char            stepping;
	char            disk_inserted;
	char            disk_switched;
	char            motor_on;

	unsigned        cur_cyl;
	unsigned        cur_head;

	pri_trk_t       *cur_track;
	unsigned long   cur_track_pos;
	unsigned long   cur_track_len;

	unsigned long   pwm_pos;
	unsigned long   pwm_len;

	unsigned long   read_pos;
	unsigned long   write_pos;

	unsigned long   write_cnt;

	unsigned long   input_clock;
	unsigned long   input_clock_cnt;

	unsigned        pwm_val;

	char            dirty;
} mac_iwm_drive_t;


typedef struct {
	unsigned char   lines;
	unsigned char   head_sel;
	unsigned char   drive_sel;

	unsigned char   status;
	unsigned char   mode;
	unsigned char   handshake;

	char            writing;

	unsigned        shift_cnt;
	unsigned char   shift;

	unsigned char   read_buf;
	unsigned short  write_buf;

	unsigned        read_zero_cnt;

	unsigned long   pwm_val;

	mac_iwm_drive_t drv[MAC_IWM_DRIVES];
	mac_iwm_drive_t *curdrv;

	unsigned char   set_motor_val;
	void            *set_motor_ext;
	void            (*set_motor) (void *ext, unsigned char val);
} mac_iwm_t;


void mac_iwm_init (mac_iwm_t *iwm);
void mac_iwm_free (mac_iwm_t *iwm);

void mac_iwm_set_motor_fct (mac_iwm_t *iwm, void *ext, void *fct);

int mac_iwm_set_heads (mac_iwm_t *iwm, unsigned drive, unsigned heads);
void mac_iwm_set_disks (mac_iwm_t *iwm, disks_t *dsks);
void mac_iwm_set_disk_id (mac_iwm_t *iwm, unsigned drive, unsigned id);
void mac_iwm_set_fname (mac_iwm_t *iwm, unsigned drive, const char *fname);
int mac_iwm_get_locked (const mac_iwm_t *iwm, unsigned drive);
void mac_iwm_set_locked (mac_iwm_t *iwm, unsigned drive, int locked);
void mac_iwm_set_auto_rotate (mac_iwm_t *iwm, unsigned drive, int val);

void mac_iwm_set_head_sel (mac_iwm_t *iwm, unsigned char val);
void mac_iwm_set_drive_sel (mac_iwm_t *iwm, unsigned char val);

void mac_iwm_set_pwm (mac_iwm_t *iwm, const unsigned char *buf, unsigned cnt);

void mac_iwm_insert (mac_iwm_t *iwm, unsigned drive);

unsigned char mac_iwm_get_uint8 (mac_iwm_t *iwm, unsigned long addr);

void mac_iwm_set_uint8 (mac_iwm_t *iwm, unsigned long addr, unsigned char val);

void mac_iwm_clock (mac_iwm_t *iwm, unsigned cnt);


#endif
