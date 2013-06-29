/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/iwm.c                                       *
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


#include "main.h"
#include "iwm.h"
#include "iwm-io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/block/block.h>
#include <lib/console.h>


#define MAC_IWM_CA0    0x01
#define MAC_IWM_CA1    0x02
#define MAC_IWM_CA2    0x04
#define MAC_IWM_LSTRB  0x08
#define MAC_IWM_ENABLE 0x10
#define MAC_IWM_SELECT 0x20
#define MAC_IWM_Q6     0x40
#define MAC_IWM_Q7     0x80


#ifndef DEBUG_IWM
#define DEBUG_IWM 0
#endif


static unsigned char pwm_tab[64] = {
	0x00, 0x2b, 0x30, 0x2a, 0x2f, 0x04, 0x35, 0x29,
	0x2e, 0x0c, 0x3a, 0x03, 0x34, 0x19, 0x09, 0x28,
	0x2d, 0x37, 0x0e, 0x0b, 0x39, 0x20, 0x1e, 0x02,
	0x33, 0x1c, 0x11, 0x18, 0x08, 0x15, 0x3f, 0x27,
	0x2c, 0x31, 0x05, 0x36, 0x0d, 0x3b, 0x1a, 0x0a,
	0x38, 0x0f, 0x21, 0x1f, 0x1d, 0x12, 0x16, 0x01,
	0x32, 0x06, 0x3c, 0x1b, 0x10, 0x22, 0x13, 0x17,
	0x07, 0x3d, 0x23, 0x14, 0x3e, 0x24, 0x25, 0x26
};


static
int iwm_drv_init (mac_iwm_drive_t *drv, unsigned drive)
{
	drv->drive = drive;

	drv->fname = NULL;

	drv->dsks = NULL;
	drv->diskid = drive;

	drv->img = NULL;

	drv->use_fname = 0;
	drv->locked = 0;
	drv->auto_rotate = 0;

	drv->cylinders = MAC_IWM_CYLINDERS;
	drv->heads = 1;

	drv->step_direction = 0;
	drv->stepping = 0;
	drv->disk_inserted = 0;
	drv->disk_switched = 0;
	drv->motor_on = 0;

	drv->cur_cyl = 0;
	drv->cur_head = 0;

	drv->cur_track = NULL;

	drv->cur_track_pos = 0;
	drv->cur_track_len = 0;

	drv->pwm_pos = 0;
	drv->pwm_len = 65000;

	drv->read_pos = 0;
	drv->write_pos = 0;

	drv->write_cnt = 0;

	drv->input_clock = MAC_CPU_CLOCK / 10;
	drv->input_clock_cnt = 0;

	drv->dirty = 0;

	return (0);
}

static
void iwm_drv_free (mac_iwm_drive_t *drv)
{
	if (drv->dirty) {
		iwm_drv_save (drv);
	}

	pri_img_del (drv->img);
}

/*
 * CLK = (0x4000000/x)*8
 * BIT = CLK/ICLK*500000
 * BIT = 8*(0x4000000/x)/ICLK*500000
 * tracks  0-15: [0x11e9 0x1135]  [74734 77788]  76261
 * tracks 16-31: [0x138a 0x12c6]  [68505 71298]  69902
 * tracks 32-47: [0x157f 0x14a7]  [62265 64813]  63539
 * tracks 48-63: [0x17e2 0x16f2]  [56046 58333]  57190
 * tracks 64-79: [0x1ade 0x19d0]  [49821 51854]  50838
*/
static
unsigned long iwm_drv_get_track_length (unsigned cyl)
{
	static unsigned long length_tab[5] = {
		76262, 69902, 63540, 57190, 50838
	};

	if (cyl > 79) {
		cyl = 79;
	}

	return (length_tab[cyl / 16]);
}

static
void iwm_drv_write_end (mac_iwm_drive_t *drv)
{
	if (drv->write_cnt > drv->cur_track_len) {
		if (drv->auto_rotate) {
			pri_trk_rotate (drv->cur_track, drv->cur_track_pos);
			drv->cur_track_pos = 0;
		}
	}

	drv->write_cnt = 0;
}

static
void iwm_drv_select_track (mac_iwm_drive_t *drv, unsigned c, unsigned h)
{
	pri_trk_t *trk;

	if ((c >= drv->cylinders) || (h >= drv->heads)) {
		return;
	}

	if (drv->img == NULL) {
		drv->img = pri_img_new();
	}

	trk = pri_img_get_track (drv->img, c, h, 1);

	if (trk == NULL) {
		return;
	}

	if (trk->size == 0) {
		if (pri_trk_set_size (trk, iwm_drv_get_track_length (c))) {
			return;
		}
	}

	if (trk->clock == 0) {
		pri_trk_set_clock (trk, 500000);
	}

	drv->cur_cyl = c;
	drv->cur_head = h;
	drv->cur_track = trk;
	drv->cur_track_len = trk->size;

	if (drv->cur_track_pos >= drv->cur_track_len) {
		drv->cur_track_pos = 0;
	}
}

static
void iwm_drv_select_head (mac_iwm_drive_t *drv, unsigned head)
{
	if (drv->cur_head == head) {
		return;
	}

	iwm_drv_select_track (drv, drv->cur_cyl, head);
}

static
int iwm_drv_get_step_direction (mac_iwm_drive_t *drv)
{
	int val;

	val = (drv->step_direction != 0);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: step direction == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_stepping (mac_iwm_drive_t *drv)
{
	int val;

	val = (drv->stepping != 0);

	drv->stepping = 0;

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: stepping == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_motor_on (mac_iwm_drive_t *drv)
{
	int val;

	val = (drv->motor_on != 0);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: motor on == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_disk_switched (mac_iwm_drive_t *drv)
{
	int val;

	val = (drv->disk_switched != 0);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: disk switched == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_head (mac_iwm_drive_t *drv, unsigned head)
{
	int val;

	val = 1;

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: head %u == %d\n",
		drv->drive + 1, head, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_superdrive (mac_iwm_drive_t *drv)
{
	int val;

	val = 0;

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: superdrive == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_sides (mac_iwm_drive_t *drv)
{
	int val;

	val = (drv->heads > 1);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: sides == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_installed (mac_iwm_drive_t *drv)
{
	int val;

	val = 1;

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: drive installed == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_disk_inserted (mac_iwm_drive_t *drv)
{
	int val;

	val = (drv->disk_inserted != 0);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: disk inserted == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_locked (mac_iwm_drive_t *drv)
{
	int    val;
	disk_t *dsk;

	val = drv->locked;

	if (drv->use_fname == 0) {
		dsk = dsks_get_disk (drv->dsks, drv->diskid);

		if ((dsk != NULL) && dsk_get_readonly (dsk)) {
			val = 1;
		}
	}

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: locked == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_track0 (mac_iwm_drive_t *drv)
{
	int val;

	val = (drv->cur_cyl == 0);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: track0 == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_tacho (mac_iwm_drive_t *drv)
{
	int           val;
	unsigned long pwm;

	if (drv->heads == 1) {
		pwm = 65536 - drv->pwm_val;

		val = ((((120 * pwm) / 32768) * drv->pwm_pos) / drv->pwm_len) & 1;
	}
	else {
		val = ((120 * drv->cur_track_pos) / drv->cur_track_len) & 1;
	}

#if DEBUG_IWM >= 4
	mac_log_deb ("iwm: get sense: d=%u: tacho == %d (%lu / %lu)\n",
		drv->drive + 1, val, drv->cur_track_pos, drv->cur_track_len
	);
#endif

	return (val);
}

static
int iwm_drv_get_ready (mac_iwm_drive_t *drv)
{
	int val;

	val = 1;

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: ready == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
int iwm_drv_get_new_if (mac_iwm_drive_t *drv)
{
	int val;

	val = 1;

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: get sense: d=%u: new interface == %d\n",
		drv->drive + 1, val
	);
#endif

	return (val);
}

static
void iwm_drv_set_step_direction (mac_iwm_drive_t *drv, int val)
{
	drv->step_direction = (val != 0);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: set cntrl: d=%u: step direction = %d\n",
		drv->drive + 1, val
	);
#endif
}

static
void iwm_drv_set_step (mac_iwm_drive_t *drv)
{
	if (drv->step_direction == 0) {
		if (drv->cur_cyl > 0) {
			drv->cur_cyl -= 1;
		}
	}
	else {
		if ((drv->cur_cyl + 1) < drv->cylinders) {
			drv->cur_cyl += 1;
		}
	}

	drv->stepping = 1;

	iwm_drv_select_track(drv, drv->cur_cyl, drv->cur_head);

	pce_printf ("IWM: D%u Track %u    \r",
		drv->drive + 1, drv->cur_cyl
	);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: drive %u step to track %u\n",
		drv->drive + 1, drv->cur_cyl
	);
#endif
}

static
void iwm_drv_set_motor_on (mac_iwm_drive_t *drv, int val)
{
	if (drv->motor_on == (val != 0)) {
		return;
	}

	drv->motor_on = (val != 0);

#if DEBUG_IWM >= 1
	mac_log_deb ("iwm: drive %u motor %s\n",
		drv->drive + 1, val ? "on" : "off"
	);
#elif DEBUG_IWM >= 2
	mac_log_deb ("iwm: set cntrl: d=%u: motor %s\n",
		drv->drive + 1, val ? "on" : "off"
	);
#endif
}

static
void iwm_drv_set_disk_switched (mac_iwm_drive_t *drv)
{
	drv->disk_switched = 0;

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: set cntrl: d=%u: reset disk switched\n",
		drv->drive + 1
	);
#endif
}

static
void iwm_drv_set_eject (mac_iwm_drive_t *drv)
{
	drv->disk_inserted = 0;

#if DEBUG_IWM >= 0
	mac_log_deb ("iwm: drive %u eject\n", drv->drive + 1);
#elif DEBUG_IWM >= 2
	mac_log_deb ("iwm: set cntrl: d=%u: eject\n", drv->drive + 1);
#endif

	if (drv->dirty) {
		iwm_drv_save (drv);
	}
}

void mac_iwm_init (mac_iwm_t *iwm)
{
	unsigned i;

	iwm->lines = 0;
	iwm->head_sel = 0;

	iwm->status = 0;
	iwm->mode = 0;
	iwm->handshake = 0x7f;

	iwm->writing = 0;

	iwm->shift_cnt = 0;
	iwm->shift = 0;

	iwm->read_buf = 0;
	iwm->write_buf = 0;

	iwm->read_zero_cnt = 0;

	iwm->pwm_val = 0;

	for (i = 0; i < MAC_IWM_DRIVES; i++) {
		iwm_drv_init (&iwm->drv[i], i);
	}

	iwm->curdrv = &iwm->drv[0];

	iwm->set_motor_val = 0;
	iwm->set_motor_ext = NULL;
	iwm->set_motor = NULL;
}

void mac_iwm_free (mac_iwm_t *iwm)
{
	unsigned i;

	for (i = 0; i < MAC_IWM_DRIVES; i++) {
		iwm_drv_free (&iwm->drv[i]);
	}
}

void mac_iwm_set_motor_fct (mac_iwm_t *iwm, void *ext, void *fct)
{
	iwm->set_motor_ext = ext;
	iwm->set_motor = fct;
}

int mac_iwm_set_heads (mac_iwm_t *iwm, unsigned drive, unsigned heads)
{
	mac_iwm_drive_t *drv;

	if (drive >= MAC_IWM_DRIVES) {
		return (1);
	}

	if (heads > MAC_IWM_HEADS) {
		return (1);
	}

	drv = &iwm->drv[drive];

	drv->heads = heads;

	if (drv->cur_head >= drv->heads) {
		iwm_drv_select_track (drv, drv->cur_cyl, 0);
	}

	return (0);
}

void mac_iwm_set_disks (mac_iwm_t *iwm, disks_t *dsks)
{
	unsigned i;

	for (i = 0; i < MAC_IWM_DRIVES; i++) {
		iwm->drv[i].dsks = dsks;
	}
}

void mac_iwm_set_disk_id (mac_iwm_t *iwm, unsigned drive, unsigned id)
{
	if (drive < MAC_IWM_DRIVES) {
		iwm->drv[drive].diskid = id;
	}
}

void mac_iwm_set_fname (mac_iwm_t *iwm, unsigned drive, const char *fname)
{
	unsigned n;
	char     *str;

	if (drive >= MAC_IWM_DRIVES) {
		return;
	}

	free (iwm->drv[drive].fname);
	iwm->drv[drive].fname = NULL;
	iwm->drv[drive].use_fname = 0;

	if (fname == NULL) {
		return;
	}

	n = strlen (fname);

	str = malloc (n + 1);

	if (str == NULL) {
		return;
	}

	memcpy (str, fname, n + 1);

	iwm->drv[drive].fname = str;
}

int mac_iwm_get_locked (const mac_iwm_t *iwm, unsigned drive)
{
	if (drive >= MAC_IWM_DRIVES) {
		return (1);
	}

	return (iwm->drv[drive].locked != 0);
}

void mac_iwm_set_locked (mac_iwm_t *iwm, unsigned drive, int locked)
{
	if (drive >= MAC_IWM_DRIVES) {
		return;
	}

	iwm->drv[drive].locked = (locked != 0);
}

void mac_iwm_set_auto_rotate (mac_iwm_t *iwm, unsigned drive, int val)
{
	if (drive >= MAC_IWM_DRIVES) {
		return;
	}

	iwm->drv[drive].auto_rotate = (val != 0);
}

static
void mac_iwm_select_drive (mac_iwm_t *iwm, unsigned drive)
{
	if (drive >= MAC_IWM_DRIVES) {
		drive = 0;
	}

	iwm->curdrv = &iwm->drv[drive];

	iwm_drv_select_track (iwm->curdrv, iwm->curdrv->cur_cyl, iwm->head_sel);
}

void mac_iwm_set_drive_sel (mac_iwm_t *iwm, unsigned char val)
{
	val = (val != 0);

	if (iwm->drive_sel == val) {
		return;
	}

	iwm->drive_sel = val;

	if ((iwm->lines & MAC_IWM_SELECT) == 0) {
		mac_iwm_select_drive (iwm, iwm->drive_sel ? 2 : 0);
	}

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: drive sel = %d\n", iwm->drive_sel);
#endif
}

void mac_iwm_set_head_sel (mac_iwm_t *iwm, unsigned char val)
{
	val = (val != 0);

	if (iwm->head_sel == val) {
		return;
	}

	iwm->head_sel = val;

	iwm_drv_select_head (iwm->curdrv, val);

#if DEBUG_IWM >= 2
	mac_log_deb ("iwm: head sel = %d\n", iwm->head_sel);
#endif
}

void mac_iwm_set_pwm (mac_iwm_t *iwm, const unsigned char *buf, unsigned cnt)
{
	unsigned long pwm, val;

	pwm = iwm->pwm_val;

	while (cnt > 0) {
		val = 1024UL * pwm_tab[*(buf++) & 0x3f];
		pwm = (31 * pwm + val) / 32;
		cnt -= 1;
	}

#if DEBUG_IWM >= 2
	if (pwm != iwm->pwm_val) {
		mac_log_deb ("iwm: drive %u set pwm to %lu\n",
			iwm->curdrv->drive + 1, pwm
		);
	}
#endif

	iwm->pwm_val = pwm;
	iwm->curdrv->pwm_val = pwm;
}

static
void mac_iwm_set_motor (mac_iwm_t *iwm)
{
	unsigned      i;
	unsigned char v;

	v = 0;

	for (i = 0; i < MAC_IWM_DRIVES; i++) {
		if (iwm->drv[i].motor_on) {
			v = 1;
			break;
		}
	}

	if (iwm->set_motor_val == v) {
		return;
	}

	iwm->set_motor_val = v;

	if (iwm->set_motor != NULL) {
		iwm->set_motor (iwm->set_motor_ext, v);
	}
}

void mac_iwm_insert (mac_iwm_t *iwm, unsigned drive)
{
	mac_iwm_drive_t *drv;

	if (drive >= MAC_IWM_DRIVES) {
		return;
	}

	drv = &iwm->drv[drive];

	if (drv->disk_inserted) {
		return;
	}

#if DEBUG_IWM >= 1
	mac_log_deb ("iwm: drive %u insert\n", drive + 1);
#endif

	if (iwm_drv_load (drv) == 0) {
		drv->disk_inserted = 1;
		iwm_drv_select_track (drv, drv->cur_cyl, drv->cur_head);
	}
}

static
int mac_iwm_get_drive_status (mac_iwm_t *iwm)
{
	unsigned        reg;
	mac_iwm_drive_t *drv;

	reg = (iwm->lines & 7) | (iwm->head_sel ? 8 : 0);
	drv = iwm->curdrv;

	switch (reg) {
	case 0:
		return (iwm_drv_get_step_direction (drv) == 0);

	case 1:
		return (iwm_drv_get_stepping (drv) == 0);

	case 2:
		return (iwm_drv_get_motor_on (drv) == 0);

	case 3:
		return (iwm_drv_get_disk_switched (drv) != 0);

	case 4:
		return (iwm_drv_get_head (drv, 0) == 0);

	case 5:
		return (iwm_drv_get_superdrive (drv));

	case 6:
		return (iwm_drv_get_sides (drv));

	case 7:
		return (iwm_drv_get_installed (drv) == 0);

	case 8:
		return (iwm_drv_get_disk_inserted (drv) == 0);

	case 9:
		return (iwm_drv_get_locked (drv) == 0);

	case 10:
		return (iwm_drv_get_track0 (drv) == 0);

	case 11:
		return (iwm_drv_get_tacho (drv) == 0);

	case 12:
		return (iwm_drv_get_head (drv, 1) == 0);

	case 13:
		return (1);

	case 14:
		return (iwm_drv_get_ready (drv) == 0);

	case 15:
		return (iwm_drv_get_new_if (drv));

	default:
#if DEBUG_IWM >= 1
		mac_log_deb ("iwm: get sense: d=%u: reg 0x%02x == %d\n",
			drv->drive + 1, reg, 0
		);
#endif
		break;
	}

	return (1);
}

static
void mac_iwm_set_drive_control (mac_iwm_t *iwm)
{
	int             val;
	unsigned        reg;
	mac_iwm_drive_t *drv;

	reg = (iwm->lines & 3) | (iwm->head_sel ? 4 : 0);
	val = ((iwm->lines & MAC_IWM_CA2) != 0);
	drv = iwm->curdrv;

	switch (reg) {
	case 0:
		iwm_drv_set_step_direction (drv, val == 0);
		break;

	case 1:
		if (val == 0) {
			iwm_drv_set_step (drv);
		}
		break;

	case 2:
		iwm_drv_set_motor_on (drv, val == 0);
		mac_iwm_set_motor (iwm);
		break;

	case 3:
		if (val) {
			iwm_drv_set_eject (drv);
		}
		break;

	case 4:
		if (val) {
			iwm_drv_set_disk_switched (drv);
		}
		break;

	default:
#if DEBUG_IWM >= 1
		mac_log_deb ("iwm: set cntrl: d=%u: reg 0x%02x = %d\n",
			drv->drive + 1, reg, val
		);
#endif
		break;
	}
}

static
void mac_iwm_access_uint8 (mac_iwm_t *iwm, unsigned reg)
{
	switch (reg & 0x0f) {
	case 0x00: /* ca0 low */
		iwm->lines &= ~MAC_IWM_CA0;
		break;

	case 0x01: /* ca0 high */
		iwm->lines |= MAC_IWM_CA0;
		break;

	case 0x02: /* ca1 low */
		iwm->lines &= ~MAC_IWM_CA1;
		break;

	case 0x03: /* ca1 high */
		iwm->lines |= MAC_IWM_CA1;
		break;

	case 0x04: /* ca2 low */
		iwm->lines &= ~MAC_IWM_CA2;
		break;

	case 0x05: /* ca2 high */
		iwm->lines |= MAC_IWM_CA2;
		break;

	case 0x06: /* lstrb low */
		iwm->lines &= ~MAC_IWM_LSTRB;
		break;

	case 0x07: /* lstrb high */
		iwm->lines |= MAC_IWM_LSTRB;
		break;

	case 0x08: /* enable low */
		iwm->lines &= ~MAC_IWM_ENABLE;
		iwm->status &= ~0x20;
		break;

	case 0x09: /* enable high */
		iwm->lines |= MAC_IWM_ENABLE;
		iwm->status |= 0x20;
		break;

	case 0x0a: /* select low */
		iwm->lines &= ~MAC_IWM_SELECT;
		mac_iwm_select_drive (iwm, iwm->drive_sel ? 2 : 0);
		break;

	case 0x0b: /* select high */
		iwm->lines |= MAC_IWM_SELECT;
		mac_iwm_select_drive (iwm, 1);
		break;

	case 0x0c: /* q6 low */
		iwm->lines &= ~MAC_IWM_Q6;
		break;

	case 0x0d: /* q6 high */
		iwm->lines |= MAC_IWM_Q6;
		break;

	case 0x0e: /* q7 low */
		iwm->lines &= ~MAC_IWM_Q7;
		break;

	case 0x0f: /* q7 high */
		iwm->lines |= MAC_IWM_Q7;
		break;
	}

	if (iwm->lines & MAC_IWM_LSTRB) {
		mac_iwm_set_drive_control (iwm);
	}

	if ((iwm->lines & MAC_IWM_Q7) == 0) {
		iwm->handshake |= 0x40;

		if (iwm->writing) {
			iwm->writing = 0;
			iwm_drv_write_end (iwm->curdrv);
		}
	}
}

unsigned char mac_iwm_get_uint8 (mac_iwm_t *iwm, unsigned long addr)
{
	unsigned char val;

	if ((addr & 1) == 0) {
		return (0);
	}

	addr = (addr >> 9) & 0x0f;

	mac_iwm_access_uint8 (iwm, addr);

	val = 0;

	switch (iwm->lines & (MAC_IWM_Q6 | MAC_IWM_Q7)) {
	case 0x00: /* read data */
		if (iwm->lines & MAC_IWM_ENABLE) {
			val = iwm->read_buf;
			iwm->read_buf = 0;
		}
		else {
			val = 0xff;
		}
		break;

	case MAC_IWM_Q6: /* read status */
		val = (iwm->status & 0x60) | (iwm->mode & 0x1f);

		if (mac_iwm_get_drive_status (iwm)) {
			val |= 0x80;
		}
#if DEBUG_IWM >= 4
		mac_log_deb ("iwm: read status (0x%02x)\n", val);
#endif
		break;

	case MAC_IWM_Q7: /* read handshake */
		val = iwm->handshake & 0x7f;

		if ((iwm->write_buf & 0xff00) == 0) {
			val |= 0x80;
		}

#if DEBUG_IWM >= 3
		mac_log_deb ("iwm: read handshake (0x%02x)\n", val);
#endif
		break;

	case (MAC_IWM_Q6 | MAC_IWM_Q7): /* ? */
#if DEBUG_IWM >= 2
		mac_log_deb ("iwm: get  8: %06lX -> 0x%02x\n", addr, val);
#endif
		break;
	}

	return (val);
}

void mac_iwm_set_uint8 (mac_iwm_t *iwm, unsigned long addr, unsigned char val)
{
	if ((addr & 1) == 0) {
		return;
	}

	addr = (addr >> 9) & 0x0f;

	mac_iwm_access_uint8 (iwm, addr);

	switch (iwm->lines & (MAC_IWM_Q6 | MAC_IWM_Q7)) {
	case (MAC_IWM_Q6 | MAC_IWM_Q7): /* mode write */
		if (iwm->lines & MAC_IWM_ENABLE) {
			/* write data */
			if (iwm->writing == 0) {
				iwm->writing = 1;
				iwm->shift_cnt = 0;
				iwm->curdrv->write_cnt = 0;
#if DEBUG_IWM >= 1
				mac_log_deb (
					"iwm: drive %u writing track %u head %u\n",
					iwm->curdrv->drive + 1,
					iwm->curdrv->cur_cyl,
					iwm->curdrv->cur_head
				);
#endif
			}

			iwm->write_buf = val | 0xff00;
#if DEBUG_IWM >= 3
			mac_log_deb ("iwm: write data (0x%02x)\n", val);
#endif
		}
		else {
			/* write mode */
			iwm->mode = val;
#if DEBUG_IWM >= 2
			mac_log_deb ("iwm: write mode (0x%02x)\n", val);
#endif
		}
		break;

	default:
#if DEBUG_IWM >= 2
		mac_log_deb ("iwm: set  8: %04lX <- %02X\n", addr, val);
#endif
		break;
	}
}

static
void mac_iwm_read (mac_iwm_t *iwm, mac_iwm_drive_t *drv)
{
	unsigned long p;
	unsigned char m;
	unsigned char *data;

	if ((drv->cur_track == NULL) || (drv->cur_track_len == 0)) {
		return;
	}

	p = drv->read_pos / 8;
	m = 0x80 >> (drv->read_pos & 7);

	data = drv->cur_track->data;

	while (drv->read_pos != drv->cur_track_pos) {
		iwm->shift = (iwm->shift << 1) | ((data[p] & m) != 0);

		if (iwm->shift & 1) {
			iwm->read_zero_cnt = 0;
		}
		else {
			iwm->read_zero_cnt += 1;

			if (iwm->read_zero_cnt >= 8) {
				iwm->read_zero_cnt = 0;
				iwm->shift |= 1;
			}
		}

		drv->read_pos += 1;

		if (drv->read_pos >= drv->cur_track_len) {
			drv->read_pos = 0;
			p = 0;
			m = 0x80;
		}
		else if (m == 1) {
			m = 0x80;
			p += 1;
		}
		else {
			m >>= 1;
		}

		if (iwm->shift & 0x80) {
			iwm->read_buf = iwm->shift;
			iwm->shift = 0;
		}
	}
}

static
void mac_iwm_write (mac_iwm_t *iwm, mac_iwm_drive_t *drv)
{
	unsigned long p;
	unsigned char m;
	unsigned char *data;

	if ((drv->cur_track == NULL) || (drv->cur_track_len == 0)) {
		return;
	}

	p = drv->write_pos / 8;
	m = 0x80 >> (drv->write_pos & 7);

	data = drv->cur_track->data;

	while (drv->write_pos != drv->cur_track_pos) {
		if (iwm->shift_cnt == 0) {
			if ((iwm->write_buf & 0xff00) == 0) {
#if DEBUG_IWM >= 2
				mac_log_deb ("iwm: drive %u underrun\n",
					drv->drive + 1
				);
#endif
				iwm->handshake &= ~0x40;
				iwm->shift_cnt = 0;
				iwm->writing = 0;

				iwm_drv_write_end (drv);

				return;
			}

			iwm->shift = iwm->write_buf & 0xff;
			iwm->shift_cnt = 8;
			iwm->write_buf = 0;
		}

		if (iwm->shift & 0x80) {
			data[p] |= m;
		}
		else {
			data[p] &= ~m;
		}

		drv->dirty = 1;

		iwm->shift = (iwm->shift << 1) & 0xff;
		iwm->shift_cnt -= 1;

		drv->write_pos += 1;

		drv->write_cnt += 1;

		if (drv->write_pos >= drv->cur_track_len) {
			drv->write_pos = 0;
			p = 0;
			m = 0x80;
		}
		else if (m == 1) {
			m = 0x80;
			p += 1;
		}
		else {
			m >>= 1;
		}
	}
}

void mac_iwm_clock (mac_iwm_t *iwm, unsigned cnt)
{
	unsigned long   clk, bit;
	mac_iwm_drive_t *drv;

	drv = iwm->curdrv;

	if (drv->motor_on == 0) {
		return;
	}

	clk = drv->input_clock_cnt + 500000UL * cnt;
	bit = clk / drv->input_clock;
	clk = clk % drv->input_clock;
	drv->input_clock_cnt = clk;

	if (drv->cur_track_len > 0) {
		drv->cur_track_pos += bit;

		while (drv->cur_track_pos >= drv->cur_track_len) {
			drv->cur_track_pos -= drv->cur_track_len;
		}
	}

	if (drv->pwm_len > 0) {
		drv->pwm_pos += bit;

		if (drv->pwm_pos >= drv->pwm_len) {
			drv->pwm_pos -= drv->pwm_len;
		}
	}

	if (iwm->writing) {
		mac_iwm_write (iwm, drv);
	}
	else if ((iwm->lines & (MAC_IWM_Q6 | MAC_IWM_Q7)) == 0) {
		mac_iwm_read (iwm, drv);
	}

	drv->read_pos = drv->cur_track_pos;
	drv->write_pos = drv->cur_track_pos;
}
