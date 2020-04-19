/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/wd179x.c                                         *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wd179x.h"


#define WD179X_ST_NOT_READY   0x80
#define WD179X_ST_MOTOR       0x80
#define WD179X_ST_WPROT       0x40
#define WD179X_ST_RECORD_TYPE 0x20
#define WD179X_ST_RNF         0x10
#define WD179X_ST_SEEK_ERROR  0x10
#define WD179X_ST_CRC_ERROR   0x08
#define WD179X_ST_LOST_DATA   0x04
#define WD179X_ST_TRACK0      0x04
#define WD179X_ST_DRQ         0x02
#define WD179X_ST_BUSY        0x01

#define WD179X_CRC 0x1021

#define WD179X_MARK_A1 0x4489
#define WD179X_MARK_C2 0x5224
#define WD179X_MARK_FE 0x5554
#define WD179X_MARK_FB 0x5545
#define WD179X_MARK_F8 0x554a


#ifndef DEBUG_WD179X
#define DEBUG_WD179X 0
#endif


static void cmd_force_interrupt (wd179x_t *fdc, unsigned char cmd);


static
void wd179x_drv_init (wd179x_drive_t *drv, unsigned idx)
{
	drv->ready = 1;
	drv->wprot = 0;

	drv->d = idx & 1;
	drv->c = 0;
	drv->h = 0;

	drv->motor_clock = 0;

	drv->trkbuf_mod = 0;
	drv->trkbuf_idx = 0;
	drv->trkbuf_cnt = 0;
}

void wd179x_drv_free (wd179x_drive_t *drv)
{
}

void wd179x_drv_reset (wd179x_drive_t *drv)
{
	drv->motor_clock = 0;
	drv->index_cnt = 0;
}

void wd179x_init (wd179x_t *fdc)
{
	wd179x_drv_init (&fdc->drive[0], 0);
	wd179x_drv_init (&fdc->drive[1], 1);

	fdc->check = 0;
	fdc->auto_motor = 0;

	fdc->input_clock = 1000000;
	fdc->bit_clock = 1000000;

	fdc->drv = &fdc->drive[0];

	fdc->delay = 0;
	fdc->cont = NULL;
	fdc->clock = NULL;

	fdc->read_track_ext = NULL;
	fdc->read_track = NULL;

	fdc->write_track_ext = NULL;
	fdc->write_track = NULL;

	fdc->irq_val = 0;
	fdc->irq_ext = NULL;
	fdc->irq = NULL;

	fdc->drq_val = 0;
	fdc->drq_ext = NULL;
	fdc->drq = NULL;
}

void wd179x_free (wd179x_t *fdc)
{
	wd179x_drv_free (&fdc->drive[1]);
	wd179x_drv_free (&fdc->drive[0]);
}

void wd179x_set_irq_fct (wd179x_t *fdc, void *ext, void *fct)
{
	fdc->irq_ext = ext;
	fdc->irq = fct;
}

void wd179x_set_drq_fct (wd179x_t *fdc, void *ext, void *fct)
{
	fdc->drq_ext = ext;
	fdc->drq = fct;
}

void wd179x_set_read_track_fct (wd179x_t *fdc, void *ext, void *fct)
{
	fdc->read_track_ext = ext;
	fdc->read_track = fct;
}

void wd179x_set_write_track_fct (wd179x_t *fdc, void *ext, void *fct)
{
	fdc->write_track_ext = ext;
	fdc->write_track = fct;
}

void wd179x_set_input_clock (wd179x_t *fdc, unsigned long clk)
{
	fdc->input_clock = clk;
}

void wd179x_set_bit_clock (wd179x_t *fdc, unsigned long clk)
{
	fdc->bit_clock = clk;
}

void wd179x_set_auto_motor (wd179x_t *fdc, int val)
{
	fdc->auto_motor = (val != 0);
}

void wd179x_reset (wd179x_t *fdc)
{
	fdc->cmd = 0;
	fdc->status = 0;
	fdc->track = 0;
	fdc->sector = 1;

	fdc->step_dir = 1;

	fdc->is_data_bit = 0;
	fdc->val = 0;
	fdc->crc = 0;

	fdc->interrupt_enable = 1;
	fdc->interrupt_request = 0;

	fdc->scan_cnt = 0;
	fdc->scan_max = 0;
	fdc->scan_buf = 0;
	fdc->scan_mark = 0;
	fdc->scan_crc[0] = 0;
	fdc->scan_crc[1] = 1;

	fdc->read_cnt = 0;
	fdc->read_crc[0] = 0;
	fdc->read_crc[1] = 1;

	fdc->write_idx = 0;
	fdc->write_cnt = 0;
	fdc->write_buf = 0;
	fdc->write_val[0] = 0;
	fdc->write_val[1] = 0;
	fdc->write_crc = 0;

	wd179x_drv_reset (&fdc->drive[0]);
	wd179x_drv_reset (&fdc->drive[1]);

	fdc->head = 0;

	fdc->cont = NULL;
	fdc->clock = NULL;
}

static
void wd179x_set_irq (wd179x_t *fdc, int val)
{
	val = (val != 0);

	if (fdc->irq_val == val) {
		return;
	}

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: IRQ=%d\n", val);
#endif

	fdc->irq_val = val;

	if (fdc->irq != NULL) {
		fdc->irq (fdc->irq_ext, val);
	}
}

static
void wd179x_set_drq (wd179x_t *fdc, int val)
{
	val = (val != 0);

	if (val) {
		fdc->status |= WD179X_ST_DRQ;
	}
	else {
		fdc->status &= ~WD179X_ST_DRQ;
	}

	if (fdc->drq_val == val) {
		return;
	}

	fdc->drq_val = val;

	if (fdc->drq != NULL) {
		fdc->drq (fdc->drq_ext, val);
	}
}

void wd179x_set_ready (wd179x_t *fdc, unsigned drive, int val)
{
	fdc->drive[drive & 1].ready = (val != 0);
	fdc->check = 1;
}

void wd179x_set_wprot (wd179x_t *fdc, unsigned drive, int val)
{
	fdc->drive[drive & 1].wprot = (val != 0);

	if (val) {
		fdc->status |= WD179X_ST_WPROT;
	}
	else {
		fdc->status &= ~WD179X_ST_WPROT;
	}
}

void wd179x_set_motor (wd179x_t *fdc, unsigned drive, int val)
{
	if (drive >= 2) {
		return;
	}

	val = (val != 0);

	if (fdc->drive[drive].motor == val) {
		return;
	}

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: motor %u %s\n", drive, val ? "on" : "off");
#endif

	fdc->drive[drive].motor = val;

	if (val == 0) {
		fdc->drive[drive].motor_clock = 0;
	}

	fdc->check = 1;
}

unsigned char wd179x_get_status (wd179x_t *fdc)
{
	wd179x_set_irq (fdc, 0);

	if (fdc->auto_motor) {
		if ((fdc->drive[0].motor == 0) && (fdc->drive[1].motor == 0)) {
			fdc->status &= ~WD179X_ST_MOTOR;
		}
		else {
			fdc->status |= WD179X_ST_MOTOR;
		}
	}
	else {
		if (fdc->drv->ready) {
			fdc->status &= ~WD179X_ST_NOT_READY;
		}
		else {
			fdc->status |= WD179X_ST_NOT_READY;
		}
	}

	return (fdc->status);
}

unsigned char wd179x_get_track (const wd179x_t *fdc)
{
	return (fdc->track);
}

void wd179x_set_track (wd179x_t *fdc, unsigned char val)
{
	fdc->track = val;
}

unsigned char wd179x_get_sector (const wd179x_t *fdc)
{
	return (fdc->sector);
}

void wd179x_set_sector (wd179x_t *fdc, unsigned char val)
{
	fdc->sector = val;
}

unsigned char wd179x_get_data (wd179x_t *fdc)
{
	wd179x_set_drq (fdc, 0);

	return (fdc->data);
}

void wd179x_set_data (wd179x_t *fdc, unsigned char val)
{
	wd179x_set_drq (fdc, 0);

	fdc->data = val;
}

void wd179x_select_drive (wd179x_t *fdc, unsigned drive)
{
	fdc->sel_drv = drive & 1;
	fdc->drv = &fdc->drive[fdc->sel_drv];
}

void wd179x_select_head (wd179x_t *fdc, unsigned val, int internal)
{
	if (internal) {
		fdc->head = 0;
	}
	else {
		fdc->head = val | 0x80;
	}
}

static
int wd179x_write_track (wd179x_t *fdc, wd179x_drive_t *drv)
{
	if (drv->trkbuf_mod == 0) {
		return (0);
	}

	if (fdc->write_track == NULL) {
		return (1);
	}

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u SAVE TRACK (c=%u h=%u)\n",
		fdc->drv->d, drv->c, drv->h
	);
#endif

	if (fdc->write_track (fdc->write_track_ext, drv)) {
		fprintf (stderr, "wd179x: save track failed\n");
		return (1);
	}

	drv->trkbuf_mod = 0;

	return (0);
}

static
int wd179x_read_track (wd179x_t *fdc, wd179x_drive_t *drv, unsigned h)
{
	if (drv->trkbuf_mod) {
		if (wd179x_write_track (fdc, drv)) {
			return (1);
		}
	}

	if ((drv->trkbuf_cnt > 0) && (drv->h == h)) {
		return (0);
	}

	if (fdc->read_track == NULL) {
		return (1);
	}

	drv->h = h;

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u SELECT TRACK (c=%u h=%u)\n",
		fdc->drv->d, drv->c, drv->h
	);
#endif

	if (fdc->read_track (fdc->read_track_ext, drv)) {
		fprintf (stderr, "WD179X: D=%u SELECT TRACK ERROR (c=%u h=%u)\n",
			fdc->drv->d, drv->c, drv->h
		);

		drv->trkbuf_idx = 0;
		drv->trkbuf_cnt = 1000000 / 6;
		memset (drv->trkbuf, 0, (drv->trkbuf_cnt + 7) / 8);
		return (1);
	}

	drv->trkbuf_mod = 0;

	if (drv->trkbuf_idx >= drv->trkbuf_cnt) {
		drv->trkbuf_idx = 0;
	}

	return (0);
}

int wd179x_flush (wd179x_t *fdc, unsigned d)
{
	wd179x_drive_t *drv;

	drv = &fdc->drive[d & 1];

	if (drv->trkbuf_mod) {
		if (wd179x_write_track (fdc, drv)) {
			return (1);
		}
	}

	drv->trkbuf_idx = 0;
	drv->trkbuf_cnt = 0;

	return (0);
}

static
unsigned wd179x_crc (unsigned short crc, unsigned char val)
{
	unsigned i;

	crc ^= (unsigned) (val & 0xff) << 8;

	for (i = 0; i < 8; i++) {
		if (crc & 0x8000) {
			crc = (crc << 1) ^ WD179X_CRC;
		}
		else {
			crc = crc << 1;
		}
	}

	return (crc & 0xffff);
}

static
void wd179x_move_bit (wd179x_t *fdc, wd179x_drive_t *drv)
{
	if (drv->motor == 0) {
		fprintf (stderr, "WD179X: motor is off!\n");
		return;
	}

	drv->trkbuf_idx += 1;

	if (drv->trkbuf_idx >= drv->trkbuf_cnt) {
		if (drv->index_cnt > 0) {
			drv->index_cnt -= 1;
		}

		drv->trkbuf_idx = 0;
	}
}

static
int wd179x_get_bit (wd179x_t *fdc)
{
	int            val;
	wd179x_drive_t *drv;

	fdc->is_data_bit = !fdc->is_data_bit;

	drv = fdc->drv;
	val = (drv->trkbuf[drv->trkbuf_idx >> 3] & (0x80 >> (drv->trkbuf_idx & 7))) != 0;

	if (fdc->is_data_bit) {
		fdc->val = ((fdc->val << 1) | val) & 0xff;

		if (val) {
			fdc->crc ^= 0x8000;
		}

		if (fdc->crc & 0x8000) {
			fdc->crc = (fdc->crc << 1) ^ WD179X_CRC;
		}
		else {
			fdc->crc <<= 1;
		}

		fdc->crc &= 0xffff;
	}

	wd179x_move_bit (fdc, drv);

	return (val);
}

static
void wd179x_set_bit (wd179x_t *fdc, int val, int clk)
{
	wd179x_drive_t *drv;

	drv = fdc->drv;

	if (val) {
		drv->trkbuf[drv->trkbuf_idx >> 3] |= 0x80 >> (drv->trkbuf_idx & 7);
	}
	else {
		drv->trkbuf[drv->trkbuf_idx >> 3] &= ~(0x80 >> (drv->trkbuf_idx & 7));
	}

	drv->trkbuf_mod = 1;

	if (clk == 0) {
		if (val) {
			fdc->crc ^= 0x8000;
		}

		if (fdc->crc & 0x8000) {
			fdc->crc = (fdc->crc << 1) ^ WD179X_CRC;
		}
		else {
			fdc->crc <<= 1;
		}

		fdc->crc &= 0xffff;
	}

	wd179x_move_bit (fdc, drv);
}

static
void wd179x_write_bit (wd179x_t *fdc, int val, int clk)
{
	fdc->write_buf <<= 1;

	if (val) {
		fdc->write_buf |= 1;
	}

	if (clk == 0) {
		if ((fdc->write_buf & 5) != 0) {
			fdc->write_buf &= ~2U;
		}

		wd179x_set_bit (fdc, fdc->write_buf & 2, 1);
		wd179x_set_bit (fdc, fdc->write_buf & 1, 0);
	}
}

static
void wd179x_scan_mark (wd179x_t *fdc)
{
	while (fdc->drv->motor_clock >= fdc->input_clock) {
		fdc->drv->motor_clock -= fdc->input_clock;

		fdc->scan_buf = ((fdc->scan_buf << 1) | wd179x_get_bit (fdc)) & 0xffff;

		if (fdc->scan_max > 0) {
			fdc->scan_max -= 1;

			if (fdc->scan_max == 0) {
				fdc->clock = NULL;
				return;
			}
		}

		if (fdc->drv->index_cnt == 0) {
			fdc->clock = NULL;
			return;
		}

		if (fdc->scan_cnt == 0) {
			if (fdc->scan_buf == WD179X_MARK_A1) {
				fdc->scan_cnt = 16;

				fdc->crc = wd179x_crc (0xffff, 0xa1);
				fdc->is_data_bit = 1;
			}

			continue;
		}

		fdc->scan_cnt += 1;

		if (fdc->scan_cnt & 15) {
			continue;
		}

		if ((fdc->scan_cnt == 32) || (fdc->scan_cnt == 48)) {
			if (fdc->scan_buf != WD179X_MARK_A1) {
				fdc->scan_cnt = 0;
			}
		}
		else if (fdc->scan_cnt == 64) {
			fdc->scan_mark = fdc->val;

			if (fdc->scan_buf == WD179X_MARK_FE) {
				;
			}
			else if ((fdc->scan_buf == WD179X_MARK_FB) || (fdc->scan_buf == WD179X_MARK_F8)) {
				fdc->clock = NULL;
				fdc->drv->motor_clock = 0;
				return;
			}
			else {
				fdc->scan_cnt = 0;
			}
		}
		else if (fdc->scan_cnt == 80) {
			fdc->scan_val[0] = fdc->val;
		}
		else if (fdc->scan_cnt == 96) {
			fdc->scan_val[1] = fdc->val;
		}
		else if (fdc->scan_cnt == 112) {
			fdc->scan_val[2] = fdc->val;
		}
		else if (fdc->scan_cnt == 128) {
			fdc->scan_val[3] = fdc->val;
			fdc->scan_crc[0] = fdc->crc;
		}
		else if (fdc->scan_cnt == 144) {
			fdc->scan_crc[1] = fdc->val << 8;
		}
		else if (fdc->scan_cnt == 160) {
			fdc->scan_crc[1] |= fdc->val;

			if (fdc->scan_crc[0] == fdc->scan_crc[1]) {
				fdc->clock = NULL;
				fdc->drv->motor_clock = 0;
				return;
			}

			fdc->scan_cnt = 0;
		}
	}
}

static
void wd179x_scan_mark_start (wd179x_t *fdc)
{
	fdc->scan_cnt = 0;
	fdc->scan_buf = 0;
	fdc->scan_mark = 0;
	fdc->scan_crc[0] = 0;
	fdc->scan_crc[1] = 1;

	fdc->clock = wd179x_scan_mark;
}


static
void cmd_auto_motor_off (wd179x_t *fdc)
{
	if (fdc->drive[0].motor) {
		wd179x_set_motor (fdc, 0, 0);
	}

	if (fdc->drive[1].motor) {
		wd179x_set_motor (fdc, 1, 0);
	}
}

static
void cmd_done (wd179x_t *fdc, int irq)
{
	fdc->cont = NULL;
	fdc->clock = NULL;

	if (fdc->auto_motor) {
		fdc->delay = 2 * fdc->input_clock;
		fdc->cont = cmd_auto_motor_off;
	}

	fdc->interrupt_enable = 1;
	fdc->interrupt_request = 0;

	fdc->status &= ~WD179X_ST_BUSY;

	if (irq) {
		wd179x_set_irq (fdc, 1);
	}

	wd179x_write_track (fdc, fdc->drv);
}

static
void cmd_set_type1_status (wd179x_t *fdc)
{
	fdc->status &= ~(WD179X_ST_WPROT | WD179X_ST_SEEK_ERROR | WD179X_ST_TRACK0);

	if (fdc->drv->c == 0) {
		fdc->status |= WD179X_ST_TRACK0;
	}

	if (fdc->drv->wprot) {
		fdc->status |= WD179X_ST_WPROT;
	}
}

/*****************************************************************************
 * restore
 *****************************************************************************/
static
void cmd_restore_done (wd179x_t *fdc)
{
#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] RESTORE DONE\n",
		fdc->drv->d, fdc->cmd
	);
#endif

	fdc->track = 0;

	cmd_set_type1_status (fdc);

	cmd_done (fdc, 1);
}

static
void cmd_restore_cont (wd179x_t *fdc)
{
	if (fdc->drv->c == 0) {
		cmd_restore_done (fdc);
		return;
	}

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] RESTORE CONT (%u -> 0)\n",
		fdc->drv->d, fdc->cmd, fdc->drv->c
	);
#endif

	fdc->drv->trkbuf_cnt = 0;

	fdc->drv->c -= 1;
	fdc->delay = 1000;
}

static
void cmd_restore (wd179x_t *fdc)
{
#if DEBUG_WD179X >= 1
	if (fdc->drv->c != 0) {
		fprintf (stderr, "WD179X: D=%u CMD[%02X] RESTORE (%u -> 0)\n",
			fdc->drv->d, fdc->cmd, fdc->drv->c
		);
	}
#endif

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	fdc->delay = 1000;
	fdc->cont = cmd_restore_cont;
}


/*****************************************************************************
 * seek
 *****************************************************************************/
static
void cmd_seek_done (wd179x_t *fdc)
{
#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] SEEK DONE\n",
		fdc->drv->d, fdc->cmd
	);
#endif

	cmd_set_type1_status (fdc);

	if (fdc->cmd & 4) {
		if (fdc->drv->c != fdc->track) {
			fdc->status |= WD179X_ST_SEEK_ERROR;
		}
	}

	cmd_done (fdc, 1);
}

static
void cmd_seek_cont (wd179x_t *fdc)
{
#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] SEEK CONT (%u -> %u)\n",
		fdc->drv->d, fdc->cmd, fdc->track, fdc->data
	);
#endif

	if (fdc->data < fdc->track) {
		fdc->track -= 1;

		if (fdc->drv->c > 0) {
			fdc->drv->c -= 1;
		}
	}
	else if (fdc->data > fdc->track) {
		fdc->track += 1;

		if (fdc->drv->c < 83) {
			fdc->drv->c += 1;
		}
	}
	else {
		cmd_seek_done (fdc);
		return;
	}

	fdc->drv->trkbuf_cnt = 0;

	if (fdc->drv->c == 0) {
		fdc->status |= WD179X_ST_TRACK0;
	}
	else {
		fdc->status &= ~WD179X_ST_TRACK0;
	}

	fdc->delay = 1000;
}

static
void cmd_seek (wd179x_t *fdc)
{
#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] SEEK (%u -> %u)\n",
		fdc->drv->d, fdc->cmd, fdc->track, fdc->data
	);
#endif

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	fdc->delay = 1000;
	fdc->cont = cmd_seek_cont;
}


/*****************************************************************************
 * step
 *****************************************************************************/
static
void cmd_step_cont (wd179x_t *fdc)
{
	int update;

	update = (fdc->cmd & 0x10) != 0;

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] STEP CONT (dir=%u upd=%d)\n",
		fdc->drv->d, fdc->cmd, fdc->step_dir, update
	);
#endif

	if (fdc->step_dir) {
		if (update) {
			fdc->track += 1;
		}

		if (fdc->drv->c < 83) {
			fdc->drv->c += 1;
		}
	}
	else {
		if (update) {
			fdc->track -= 1;
		}

		if (fdc->drv->c > 0) {
			fdc->drv->c -= 1;
		}
	}

	fdc->drv->trkbuf_cnt = 0;

	cmd_set_type1_status (fdc);

	cmd_done (fdc, 1);
}

static
void cmd_step (wd179x_t *fdc, unsigned dir)
{
#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] STEP (dir=%u upd=%d)\n",
		fdc->drv->d, fdc->cmd, dir,
		(fdc->cmd & 0x10) != 0
	);
#endif

	if (dir > 0) {
		fdc->step_dir = dir;
	}

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	fdc->delay = 1000;
	fdc->cont = cmd_step_cont;
}


/*****************************************************************************
 * read sector
 *****************************************************************************/

static void cmd_read_sector_idam (wd179x_t *fdc);

static
void cmd_read_sector_clock (wd179x_t *fdc)
{
	while (fdc->drv->motor_clock >= fdc->input_clock) {
		fdc->drv->motor_clock -= fdc->input_clock;

		wd179x_get_bit (fdc);

		if (fdc->is_data_bit == 0) {
			continue;
		}

		fdc->read_cnt -= 1;

		if (fdc->read_cnt & 7) {
			continue;
		}

		if (fdc->read_cnt >= 16) {
			fdc->interrupt_enable = 0;

			if (fdc->status & WD179X_ST_DRQ) {
				fprintf (stderr, "WD179X: READ LOST DATA\n");
				fdc->status |= WD179X_ST_LOST_DATA;
				cmd_done (fdc, 1);
				return;
			}

			fdc->data = fdc->val & 0xff;

			wd179x_set_drq (fdc, 1);

			if (fdc->read_cnt == 16) {
				fdc->read_crc[0] = fdc->crc;
			}
		}
		else if (fdc->read_cnt == 8) {
			fdc->read_crc[1] = (unsigned) (fdc->val & 0xff) << 8;
		}
		else {
			fdc->read_crc[1] |= fdc->val & 0xff;
			fdc->interrupt_enable = 1;

			if (fdc->read_crc[0] != fdc->read_crc[1]) {
				fprintf (stderr, "WD179X: READ CRC ERROR (%02X %04X)\n",
					fdc->read_crc[0], fdc->read_crc[1]
				);

				fdc->status |= WD179X_ST_CRC_ERROR;
			}

			if (fdc->interrupt_request) {
				cmd_force_interrupt (fdc, fdc->interrupt_request);
			}
			else if ((fdc->status & WD179X_ST_CRC_ERROR) || ((fdc->cmd & 0x10) == 0)) {
				cmd_done (fdc, 1);
			}
			else {
				fdc->sector += 1;

				fdc->drv->index_cnt = 5;
				wd179x_scan_mark_start (fdc);

				fdc->delay = 0;
				fdc->cont = cmd_read_sector_idam;
			}

			return;
		}
	}
}

static
void cmd_read_sector_dam (wd179x_t *fdc)
{
	if ((fdc->scan_mark != 0xfb) && (fdc->scan_mark != 0xf8)) {
#if DEBUG_WD179X >= 1
		fprintf (stderr, "WD179X: MISSING DAM\n");
#endif
		fdc->scan_max = 0;
		wd179x_scan_mark_start (fdc);
		fdc->cont = cmd_read_sector_idam;
		return;
	}

	if (fdc->scan_mark == 0xf8) {
		fdc->status |= WD179X_ST_RECORD_TYPE;
	}
	else {
		fdc->status &= ~WD179X_ST_RECORD_TYPE;
	}

	fdc->read_cnt = 128U << fdc->scan_val[3];
	fdc->read_cnt = 8 * fdc->read_cnt + 16;
	fdc->clock = cmd_read_sector_clock;
	fdc->cont = NULL;

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X:   READ [%02X %02X %02X %02X]\n",
		fdc->scan_val[0], fdc->scan_val[1], fdc->scan_val[2], fdc->scan_val[3]
	);
#endif

	cmd_read_sector_clock (fdc);
}

static
void cmd_read_sector_idam (wd179x_t *fdc)
{
	if (fdc->drv->index_cnt == 0) {
		fdc->status |= WD179X_ST_RNF;
		cmd_done (fdc, 1);
		return;
	}

	if (fdc->scan_mark != 0xfe) {
		wd179x_scan_mark_start (fdc);
		return;
	}

#if DEBUG_WD179X >= 3
	fprintf (stderr, "WD179X:   IDAM [%02X %02X %02X %02X]\n",
		fdc->scan_val[0], fdc->scan_val[1], fdc->scan_val[2], fdc->scan_val[3]
	);
#endif

	if (fdc->scan_crc[0] != fdc->scan_crc[1]) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	if (fdc->track != fdc->scan_val[0]) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	if (fdc->cmd & 0x02) {
		/* side compare enabled */

		if (fdc->drv->h != fdc->scan_val[1]) {
			wd179x_scan_mark_start (fdc);
			return;
		}
	}

	if (fdc->sector != fdc->scan_val[2]) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	if (fdc->scan_val[3] > 3) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	fdc->scan_max = 60 * 16;

	wd179x_scan_mark_start (fdc);

	fdc->delay = 0;
	fdc->cont = cmd_read_sector_dam;
}

static
void cmd_read_sector (wd179x_t *fdc)
{
	unsigned h;

	h = (fdc->head & 0x80) ? (fdc->head & 0x7f) : ((fdc->cmd >> 1) & 1);

#if DEBUG_WD179X >= 1
	fprintf (stderr, "WD179X: D=%u CMD[%02X] READ SECTOR (pc=%u c=%u h=%u s=%u)\n",
		fdc->drv->d, fdc->cmd, fdc->drv->c, fdc->track, h, fdc->sector
	);
#endif

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	wd179x_read_track (fdc, fdc->drv, h);

	fdc->scan_max = 0;
	fdc->drv->index_cnt = 5;

	wd179x_scan_mark_start (fdc);

	fdc->delay = 0;
	fdc->cont = cmd_read_sector_idam;
}


/*****************************************************************************
 * write sector
 *****************************************************************************/

static void cmd_write_sector_idam (wd179x_t *fdc);

static
void cmd_write_sector_clock (wd179x_t *fdc)
{
	while (fdc->drv->motor_clock >= fdc->input_clock) {
		fdc->drv->motor_clock -= fdc->input_clock;

		if ((fdc->write_idx & 15) == 0) {
			fdc->write_val[1] = 0xff;

			if (fdc->write_idx < (16 * 22)) {
				;
			}
			else if (fdc->write_idx < (16 * 34)) {
				if (fdc->status & WD179X_ST_DRQ) {
					fprintf (stderr, "WD179X: WRITE LOST DATA\n");
					fdc->status |= WD179X_ST_LOST_DATA;
				}

				fdc->write_val[0] = 0x00;
			}
			else if (fdc->write_idx < (16 * 37)) {
				if (fdc->write_idx == (16 * 34)) {
					fdc->crc = 0xffff;
				}

				fdc->write_val[0] = 0xa1;
				fdc->write_val[1] = 0xfb;
			}
			else if (fdc->write_idx < (16 * 38)) {
				fdc->write_val[0] = (fdc->cmd & 1) ? 0xf8 : 0xfb;
			}
			else if (fdc->write_idx < (fdc->write_cnt - 32)) {
				fdc->interrupt_enable = 0;

				if (fdc->status & WD179X_ST_DRQ) {
					fprintf (stderr, "WD179X: WRITE LOST DATA\n");
					fdc->status |= WD179X_ST_LOST_DATA;
					fdc->data = 0;
				}

				fdc->write_val[0] = fdc->data;

				if ((fdc->write_idx + 16) < (fdc->write_cnt - 32)) {
					wd179x_set_drq (fdc, 1);
				}
			}
			else if (fdc->write_idx < (fdc->write_cnt - 16)) {
				fdc->write_crc = fdc->crc;
				fdc->write_val[0] = (fdc->write_crc >> 8) & 0xff;
			}
			else if (fdc->write_idx < fdc->write_cnt) {
				fdc->write_val[0] = fdc->write_crc & 0xff;
			}
			else {
#if DEBUG_WD179X >= 2
				fprintf (stderr, "WD179X:   DONE [%02X %02X %02X %02X]\n",
					fdc->scan_val[0], fdc->scan_val[1], fdc->scan_val[2], fdc->scan_val[3]
				);
#endif

				fdc->interrupt_enable = 1;

				if (fdc->interrupt_request) {
					cmd_force_interrupt (fdc, fdc->interrupt_request);
				}
				else if (fdc->cmd & 0x10) {
					fdc->sector += 1;

					fdc->drv->index_cnt = 5;
					wd179x_scan_mark_start (fdc);

					fdc->delay = 0;
					fdc->cont = cmd_write_sector_idam;
				}
				else {
					cmd_done (fdc, 1);
				}

				return;
			}
		}

		if (fdc->write_idx >= (16 * 22)) {
			if (fdc->write_idx & 1) {
				wd179x_write_bit (fdc, (fdc->write_val[0] >> 7) & 1, 0);
				fdc->write_val[0] <<= 1;
			}
			else {
				wd179x_write_bit (fdc, (fdc->write_val[1] >> 7) & 1, 1);
				fdc->write_val[1] <<= 1;
			}
		}
		else {
			wd179x_move_bit (fdc, fdc->drv);
		}

		fdc->write_idx += 1;
	}
}

static
void cmd_write_sector_idam (wd179x_t *fdc)
{
	if (fdc->drv->index_cnt == 0) {
		fdc->status |= WD179X_ST_RNF;
		cmd_done (fdc, 1);
		return;
	}

	if (fdc->scan_mark != 0xfe) {
		wd179x_scan_mark_start (fdc);
		return;
	}

#if DEBUG_WD179X >= 3
	fprintf (stderr, "WD179X:   IDAM [%02X %02X %02X %02X]\n",
		fdc->scan_val[0], fdc->scan_val[1], fdc->scan_val[2], fdc->scan_val[3]
	);
#endif

	if (fdc->scan_crc[0] != fdc->scan_crc[1]) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	if (fdc->track != fdc->scan_val[0]) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	if (fdc->cmd & 0x02) {
		/* side compare enabled */

		if (fdc->drv->h != fdc->scan_val[1]) {
			wd179x_scan_mark_start (fdc);
			return;
		}
	}

	if (fdc->sector != fdc->scan_val[2]) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	if (fdc->scan_val[3] > 3) {
		wd179x_scan_mark_start (fdc);
		return;
	}

	wd179x_set_drq (fdc, 1);

	fdc->write_idx = 0;
	fdc->write_cnt = 16 * (22 + 12 + 4 + (128U << fdc->scan_val[3]) + 2);

	fdc->clock = cmd_write_sector_clock;

	cmd_write_sector_clock (fdc);
}

static
void cmd_write_sector (wd179x_t *fdc)
{
	unsigned h;

	h = (fdc->head & 0x80) ? (fdc->head & 0x7f) : ((fdc->cmd >> 1) & 1);

#if DEBUG_WD179X >= 1
	fprintf (stderr, "WD179X: D=%u CMD[%02X] WRITE SECTOR (c=%u h=%u s=%u)\n",
		fdc->drv->d, fdc->cmd, fdc->track, h, fdc->sector
	);
#endif

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	if (wd179x_read_track (fdc, fdc->drv, h)) {
		fdc->status |= WD179X_ST_RNF;
		cmd_done (fdc, 1);
		return;
	}

	fdc->drv->index_cnt = 5;

	wd179x_scan_mark_start (fdc);

	fdc->delay = 0;
	fdc->cont = cmd_write_sector_idam;
}


/*****************************************************************************
 * write track
 *****************************************************************************/

static
void cmd_write_track_clock (wd179x_t *fdc)
{
	while (fdc->drv->motor_clock >= fdc->input_clock) {
		fdc->drv->motor_clock -= fdc->input_clock;

		if (fdc->drv->index_cnt > 1) {
			wd179x_move_bit (fdc, fdc->drv);
			continue;
		}

		if (fdc->drv->index_cnt < 1) {
			wd179x_set_drq (fdc, 0);
			cmd_done (fdc, 1);
			return;
		}

		if ((fdc->write_idx & 15) == 0) {
			if (fdc->status & WD179X_ST_DRQ) {
				fprintf (stderr, "WD179X: FORMAT LOST DATA\n");
				fdc->data = 0;
			}

			if (fdc->write_cnt > 0) {
				fdc->write_val[0] = fdc->write_crc & 0xff;
				fdc->write_val[1] = 0xff;
				fdc->write_cnt = 0;
			}
			else if (fdc->data == 0xf5) {
				fdc->write_val[0] = 0xa1;
				fdc->write_val[1] = 0xfb;

				fdc->crc = wd179x_crc (0xffff, 0xa1);
				fdc->crc = wd179x_crc (fdc->crc, 0xa1);
			}
			else if (fdc->data == 0xf6) {
				fdc->write_val[0] = 0xc2;
				fdc->write_val[1] = 0xf7;
			}
			else if (fdc->data == 0xf7) {
				fdc->write_crc = fdc->crc;
				fdc->write_val[0] = (fdc->write_crc >> 8) & 0xff;
				fdc->write_val[1] = 0xff;
				fdc->write_cnt = 1;
			}
			else {
				fdc->write_val[0] = fdc->data;
				fdc->write_val[1] = 0xff;
			}

			if (fdc->write_cnt == 0) {
				wd179x_set_drq (fdc, 1);
			}
		}

		if (fdc->write_idx & 1) {
			wd179x_write_bit (fdc, (fdc->write_val[0] >> 7) & 1, 0);
			fdc->write_val[0] <<= 1;
		}
		else {
			wd179x_write_bit (fdc, (fdc->write_val[1] >> 7) & 1, 1);
			fdc->write_val[1] <<= 1;
		}

		fdc->write_idx += 1;
	}
}

static
void cmd_write_track (wd179x_t *fdc)
{
	unsigned h;

	h = (fdc->head & 0x80) ? (fdc->head & 0x7f) : ((fdc->cmd >> 1) & 1);

#if DEBUG_WD179X >= 1
	fprintf (stderr, "WD179X: D=%u CMD[%02X] WRITE TRACK (c=%u h=%u)\n",
		fdc->drv->d, fdc->cmd, fdc->track, h
	);
#endif

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	wd179x_read_track (fdc, fdc->drv, h);

	fdc->drv->index_cnt = 2;

	fdc->write_idx = 0;
	fdc->write_cnt = 0;
	fdc->write_buf = 0;

	fdc->clock = cmd_write_track_clock;

	fdc->delay = 0;
	fdc->cont = NULL;

	wd179x_set_drq (fdc, 1);

	cmd_write_track_clock (fdc);
}


/*****************************************************************************
 * read address
 *****************************************************************************/

static void cmd_read_address_idam (wd179x_t *fdc);

static
void cmd_read_address_clock (wd179x_t *fdc)
{
	while (fdc->drv->motor_clock >= fdc->input_clock) {
		fdc->drv->motor_clock -= fdc->input_clock;

		wd179x_move_bit (fdc, fdc->drv);

		if (fdc->is_data_bit == 0) {
			continue;
		}

		fdc->read_cnt -= 1;

		if (fdc->read_cnt & 7) {
			continue;
		}

		if (fdc->status & WD179X_ST_DRQ) {
			fprintf (stderr, "WD179X: READ ADDRESS LOST DATA\n");
			fdc->status |= WD179X_ST_LOST_DATA;
		}

		if (fdc->read_cnt >= 40) {
			fdc->data = fdc->scan_val[1];
		}
		else if (fdc->read_cnt >= 32) {
			fdc->data = fdc->scan_val[2];
		}
		else if (fdc->read_cnt >= 24) {
			fdc->data = fdc->scan_val[3];
		}
		else if (fdc->read_cnt >= 16) {
			fdc->data = (fdc->scan_crc[1] >> 8) & 0xff;
		}
		else if (fdc->read_cnt >= 8) {
			fdc->data = fdc->scan_crc[1] & 0xff;
		}
		else {
			cmd_done (fdc, 1);
			return;
		}

		wd179x_set_drq (fdc, 1);
	}
}

static
void cmd_read_address_idam (wd179x_t *fdc)
{
	if (fdc->drv->index_cnt == 0) {
		fdc->status |= WD179X_ST_RNF;
		cmd_done (fdc, 1);
		return;
	}

	if (fdc->scan_mark != 0xfe) {
		wd179x_scan_mark_start (fdc);
		return;
	}

#if DEBUG_WD179X >= 1
	fprintf (stderr, "WD179X:   READ ADDRESS [%02X %02X %02X %02X]\n",
		fdc->scan_val[0], fdc->scan_val[1], fdc->scan_val[2], fdc->scan_val[3]
	);
#endif

	if (fdc->scan_crc[0] != fdc->scan_crc[1]) {
		fdc->status |= WD179X_ST_CRC_ERROR;
	}

	fdc->sector = fdc->scan_val[0];

	fdc->read_cnt = 8 * 6;
	fdc->clock = cmd_read_address_clock;
	fdc->cont = NULL;

	fdc->data = fdc->scan_val[0];

	wd179x_set_drq (fdc, 1);
}

static
void cmd_read_address (wd179x_t *fdc)
{
	unsigned h;

	h = (fdc->head & 0x80) ? (fdc->head & 0x7f) : ((fdc->cmd >> 1) & 1);

#if DEBUG_WD179X >= 1
	fprintf (stderr, "WD179X: D=%u CMD[%02X] READ ADDRESS (c=%u h=%u)\n",
		fdc->drv->d, fdc->cmd, fdc->track, h
	);
#endif

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	wd179x_read_track (fdc, fdc->drv, h);

	fdc->scan_max = 0;
	fdc->drv->index_cnt = 5;

	wd179x_scan_mark_start (fdc);

	fdc->delay = 0;
	fdc->cont = cmd_read_address_idam;
}


/*****************************************************************************
 * read track
 *****************************************************************************/

static
void wd179x_read_track_clock (wd179x_t *fdc)
{
	while (fdc->drv->motor_clock >= fdc->input_clock) {
		fdc->drv->motor_clock -= fdc->input_clock;

		if (fdc->drv->index_cnt > 1) {
			wd179x_move_bit (fdc, fdc->drv);
			continue;
		}

		if (fdc->drv->index_cnt == 0) {
			cmd_done (fdc, 1);
			return;
		}

		fdc->scan_buf = ((fdc->scan_buf << 1) | wd179x_get_bit (fdc)) & 0xffff;
		fdc->scan_cnt += 1;

		if (fdc->last_mark_a1 > 0) {
			fdc->last_mark_a1 -= 1;
		}

		if (fdc->last_mark_c2 > 0) {
			fdc->last_mark_c2 -= 1;
		}

		if (fdc->scan_buf == WD179X_MARK_A1) {
			if (fdc->last_mark_a1 == 0) {
				fdc->is_data_bit = 1;
				fdc->last_mark_a1 = 16;

				if (fdc->last_mark_c2 > 0) {
					fdc->scan_cnt = 0;
				}
				else {
					fdc->scan_cnt = 16;
				}
			}
		}
		else if (fdc->scan_buf == WD179X_MARK_C2) {
			fdc->is_data_bit = 1;
			fdc->last_mark_c2 = 16;

			if (fdc->scan_cnt < 4) {
				fdc->scan_cnt = 0;
			}
			else {
				fdc->scan_cnt = 16;
			}
		}

		if (fdc->scan_cnt < 16) {
			continue;
		}

		fdc->scan_cnt = 0;

		if (fdc->status & WD179X_ST_DRQ) {
			fprintf (stderr, "WD179X: READ TRACK LOST DATA\n");
			fdc->status |= WD179X_ST_LOST_DATA;
			wd179x_set_drq (fdc, 0);
		}

		fdc->data = fdc->val & 0xff;

		wd179x_set_drq (fdc, 1);
	}
}

static
void cmd_read_track (wd179x_t *fdc)
{
	unsigned h;

	h = (fdc->head & 0x80) ? (fdc->head & 0x7f) : ((fdc->cmd >> 1) & 1);

#if DEBUG_WD179X >= 1
	fprintf (stderr, "WD179X: D=%u CMD[%02X] READ TRACK (c=%u h=%u)\n",
		fdc->drv->d, fdc->cmd, fdc->track, h
	);
#endif

	fdc->status = WD179X_ST_BUSY;

	if (fdc->auto_motor) {
		wd179x_set_motor (fdc, fdc->sel_drv, 1);
	}

	wd179x_read_track (fdc, fdc->drv, h);

	fdc->drv->index_cnt = 2;

	fdc->scan_cnt = 0;
	fdc->scan_buf = 0;
	fdc->last_mark_a1 = 0;
	fdc->last_mark_c2 = 0;
	fdc->is_data_bit = 1;
	fdc->val = 0;

	fdc->clock = wd179x_read_track_clock;
}


/*****************************************************************************
 * force interrupt
 *****************************************************************************/

static
void cmd_force_interrupt (wd179x_t *fdc, unsigned char cmd)
{
	int busy;

	if (fdc->interrupt_enable == 0) {
		fdc->interrupt_request = cmd;
		return;
	}

	fdc->cmd = cmd;

	busy = fdc->status & WD179X_ST_BUSY;

#if DEBUG_WD179X >= 2
	fprintf (stderr, "WD179X: D=%u CMD[%02X] FORCE INTERRUPT\n",
		fdc->drv->d, fdc->cmd
	);
#endif

	wd179x_set_irq (fdc, 0);

	fdc->status = WD179X_ST_BUSY;

	if (busy == 0) {
		cmd_set_type1_status (fdc);
	}

	cmd_done (fdc, fdc->cmd & 0x08);
}


void wd179x_set_cmd (wd179x_t *fdc, unsigned char val)
{
	fdc->check = 1;

	if ((val & 0xf0) == 0xd0) {
		cmd_force_interrupt (fdc, val);
		return;
	}

	fdc->clock = NULL;
	fdc->cont = NULL;

	if (fdc->status & WD179X_ST_BUSY) {
		fprintf (stderr, "WD179X: CMD[%02X] SKIP COMMAND (%02X/%02X)\n",
			val, fdc->cmd, fdc->status
		);

		return;
	}

	fdc->cmd = val;

	wd179x_set_irq (fdc, 0);
	wd179x_set_drq (fdc, 0);

	fdc->interrupt_enable = 1;
	fdc->interrupt_request = 0;

	if ((val & 0xf0) == 0) {
		cmd_restore (fdc);
	}
	else if ((val & 0xf0) == 0x10) {
		cmd_seek (fdc);
	}
	else if ((val & 0xe0) == 0x40) {
		cmd_step (fdc, 1);
	}
	else if ((val & 0xe1) == 0x80) {
		cmd_read_sector (fdc);
	}
	else if ((val & 0xe0) == 0xa0) {
		cmd_write_sector (fdc);
	}
	else if ((val & 0xe0) == 0xc0) {
		cmd_read_address (fdc);
	}
	else if ((val & 0xf0) == 0xe0) {
		cmd_read_track (fdc);
	}
	else if ((val & 0xf1) == 0xf0) {
		cmd_write_track (fdc);
	}
	else {
		fprintf (stderr, "WD179X: CMD[%02X] UNKNOWN COMMAND\n", val);
	}
}

void wd179x_clock2 (wd179x_t *fdc, unsigned cnt)
{
	fdc->check = 0;

	if (fdc->drive[0].motor) {
		fdc->drive[0].motor_clock += cnt * (fdc->bit_clock / 2);
		fdc->check = 1;
	}

	if (fdc->drive[1].motor) {
		fdc->drive[1].motor_clock += cnt * (fdc->bit_clock / 2);
		fdc->check = 1;
	}

	if (fdc->clock != NULL) {
		fdc->clock (fdc);
		fdc->check = 1;
	}
	else if (fdc->cont != NULL) {
		if (cnt < fdc->delay) {
			fdc->delay -= cnt;
		}
		else {
			fdc->delay = 0;
			fdc->cont (fdc);
		}

		fdc->check = 1;
	}

	while (fdc->drive[0].motor_clock >= fdc->input_clock) {
		fdc->drive[0].motor_clock -= fdc->input_clock;
		wd179x_move_bit (fdc, &fdc->drive[0]);
	}

	while (fdc->drive[1].motor_clock >= fdc->input_clock) {
		fdc->drive[1].motor_clock -= fdc->input_clock;
		wd179x_move_bit (fdc, &fdc->drive[1]);
	}
}
