/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/wd179x.h                                         *
 * Created:     2012-07-05 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2013 Hampa Hug <hampa@hampa.ch>                     *
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


/* WD 179X FDC */


#ifndef PCE_CHIPSET_WD179X_H
#define PCE_CHIPSET_WD179X_H 1


#define WD179X_TRKBUF_SIZE 32768


typedef struct {
	unsigned char ready;
	unsigned char wprot;
	unsigned char motor;

	unsigned      d;
	unsigned      c;
	unsigned      h;

	unsigned long motor_clock;
	unsigned      index_cnt;

	unsigned char trkbuf_mod;
	unsigned long trkbuf_idx;
	unsigned long trkbuf_cnt;
	unsigned char trkbuf[WD179X_TRKBUF_SIZE];
} wd179x_drive_t;


typedef struct wd179x_t {
	unsigned char  check;
	unsigned char  auto_motor;

	unsigned char  cmd;
	unsigned char  status;
	unsigned char  track;
	unsigned char  sector;
	unsigned char  data;

	unsigned char  step_dir;

	char           is_data_bit;
	unsigned char  val;
	unsigned short crc;

	char           interrupt_enable;
	unsigned char  interrupt_request;

	unsigned       scan_cnt;
	unsigned       scan_max;
	unsigned short scan_buf;
	unsigned char  scan_mark;
	unsigned char  scan_val[4];
	unsigned short scan_crc[2];

	unsigned       read_cnt;
	unsigned short read_crc[2];

	unsigned       write_idx;
	unsigned       write_cnt;
	unsigned short write_buf;
	unsigned char  write_val[2];
	unsigned short write_crc;

	unsigned char  last_mark_a1;
	unsigned char  last_mark_c2;

	unsigned long  input_clock;
	unsigned long  bit_clock;

	unsigned       sel_drv;
	wd179x_drive_t drive[2];
	wd179x_drive_t *drv;

	unsigned       head;

	unsigned long  delay;
	void           (*cont) (struct wd179x_t *fdc);
	void           (*clock) (struct wd179x_t *fdc);

	void           *read_track_ext;
	int            (*read_track) (void *ext, wd179x_drive_t *drv);

	void           *write_track_ext;
	int            (*write_track) (void *ext, wd179x_drive_t *drv);

	unsigned char  irq_val;
	void           *irq_ext;
	void           (*irq) (void *ext, unsigned char val);

	unsigned char  drq_val;
	void           *drq_ext;
	void           (*drq) (void *ext, unsigned char val);
} wd179x_t;


void wd179x_init (wd179x_t *fdc);
void wd179x_free (wd179x_t *fdc);

void wd179x_set_irq_fct (wd179x_t *fdc, void *ext, void *fct);
void wd179x_set_drq_fct (wd179x_t *fdc, void *ext, void *fct);

void wd179x_set_read_track_fct (wd179x_t *fdc, void *ext, void *fct);
void wd179x_set_write_track_fct (wd179x_t *fdc, void *ext, void *fct);

void wd179x_set_input_clock (wd179x_t *fdc, unsigned long clk);
void wd179x_set_bit_clock (wd179x_t *fdc, unsigned long clk);

/*****************************************************************************
 * @short Enable automatic motor control
 *
 * By default, the motor is controlled externally. If automatic motor control
 * is enabled, the fdc takes over motor control as the 177x does.
 *****************************************************************************/
void wd179x_set_auto_motor (wd179x_t *fdc, int val);

void wd179x_reset (wd179x_t *fdc);

void wd179x_set_ready (wd179x_t *fdc, unsigned drive, int val);
void wd179x_set_wprot (wd179x_t *fdc, unsigned drive, int val);
void wd179x_set_motor (wd179x_t *fdc, unsigned drive, int val);

unsigned char wd179x_get_status (wd179x_t *fdc);

unsigned char wd179x_get_track (const wd179x_t *fdc);
void wd179x_set_track (wd179x_t *fdc, unsigned char val);

unsigned char wd179x_get_sector (const wd179x_t *fdc);
void wd179x_set_sector (wd179x_t *fdc, unsigned char val);

unsigned char wd179x_get_data (wd179x_t *fdc);
void wd179x_set_data (wd179x_t *fdc, unsigned char val);

void wd179x_select_drive (wd179x_t *fdc, unsigned drive);

/*****************************************************************************
 * @short Select the head for subsequent I/O operations
 *
 * If internal is true then the head is specified in the command byte and
 * val is ignored. If internal is false then val will be used as head.
 *****************************************************************************/
void wd179x_select_head (wd179x_t *fdc, unsigned val, int internal);

int wd179x_flush (wd179x_t *fdc, unsigned d);

void wd179x_set_cmd (wd179x_t *fdc, unsigned char val);

void wd179x_clock2 (wd179x_t *fdc, unsigned cnt);

static inline
void wd179x_clock (wd179x_t *fdc, unsigned cnt)
{
	if (fdc->check) {
		wd179x_clock2 (fdc, cnt);
	}
}


#endif
