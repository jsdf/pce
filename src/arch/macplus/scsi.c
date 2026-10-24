/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/scsi.c                                      *
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


#include "main.h"
#include "scsi.h"

#include <stdlib.h>
#include <string.h>

#include <devices/memory.h>

#include <drivers/block/block.h>


/* ICR 1 */
#define E5380_ICR_RST  0x80
#define E5380_ICR_AIP  0x40
#define E5380_ICR_LA   0x20
#define E5380_ICR_ACK  0x10
#define E5380_ICR_BSY  0x08
#define E5380_ICR_SEL  0x04
#define E5380_ICR_ATN  0x02
#define E5380_ICR_DBUS 0x01

/* MR2 2 */
#define E5380_MR2_BLK  0x80
#define E5380_MR2_TARG 0x40
#define E5380_MR2_PCHK 0x20
#define E5380_MR2_PINT 0x10
#define E5380_MR2_EOP  0x08
#define E5380_MR2_BSY  0x04
#define E5380_MR2_DMA  0x02
#define E5380_MR2_ARB  0x01

/* TCR 3 */
#define E5380_TCR_REQ  0x08
#define E5380_TCR_MSG  0x04
#define E5380_TCR_CD   0x02
#define E5380_TCR_IO   0x01

/* CSB 4 */
#define E5380_CSB_RST  0x80
#define E5380_CSB_BSY  0x40
#define E5380_CSB_REQ  0x20
#define E5380_CSB_MSG  0x10
#define E5380_CSB_CD   0x08
#define E5380_CSB_IO   0x04
#define E5380_CSB_SEL  0x02
#define E5380_CSB_DBP  0x01

/* BSR 5 */
#define E5380_BSR_EDMA 0x80
#define E5380_BSR_DRQ  0x40
#define E5380_BSR_SPER 0x20
#define E5380_BSR_INT  0x10
#define E5380_BSR_PHSM 0x08
#define E5380_BSR_BSY  0x04
#define E5380_BSR_ATN  0x02
#define E5380_BSR_ACK  0x01

#define E5380_PHASE_FREE     0
#define E5380_PHASE_ARB      1
#define E5380_PHASE_SEL      2
#define E5380_PHASE_MSG_OUT  3
#define E5380_PHASE_CMD      4
#define E5380_PHASE_DATA_IN  5
#define E5380_PHASE_DATA_OUT 6
#define E5380_PHASE_STATUS   7
#define E5380_PHASE_MSG_IN   8


void mac_scsi_init (mac_scsi_t *scsi)
{
	unsigned i;

	scsi->phase = E5380_PHASE_FREE;

	scsi->odr = 0x00;
	scsi->csd = 0x00;
	scsi->icr = 0x00;
	scsi->mr2 = 0x00;
	scsi->csb = 0x00;
	scsi->ser = 0x00;
	scsi->bsr = 0x00;

	scsi->cmd_i = 0;
	scsi->cmd_n = 0;

	scsi->buf_i = 0;
	scsi->buf_n = 0;
	scsi->buf_max = 4096;
	scsi->buf = malloc (scsi->buf_max);

	scsi->addr_mask = 0xff0;
	scsi->addr_shift = 4;

	scsi->cmd_start = NULL;
	scsi->cmd_finish = NULL;

	scsi->set_int_val = 0;
	scsi->set_int_ext = NULL;
	scsi->set_int = NULL;

	for (i = 0; i < 8; i++) {
		scsi->dev[i].valid = 0;
	}

	scsi->dsks = NULL;
}

void mac_scsi_free (mac_scsi_t *scsi)
{
	free (scsi->buf);
}

void mac_scsi_set_int_fct (mac_scsi_t *scsi, void *ext, void *fct)
{
	scsi->set_int_ext = ext;
	scsi->set_int = fct;
}

void mac_scsi_set_disks (mac_scsi_t *scsi, disks_t *dsks)
{
	scsi->dsks = dsks;
}

void mac_scsi_set_drive (mac_scsi_t *scsi, unsigned id, unsigned drive)
{
	id &= 7;

	scsi->dev[id].valid = 1;
	scsi->dev[id].drive = drive;

	memcpy (scsi->dev[id].vendor, "PCE     ", 8);
	memcpy (scsi->dev[id].product, "PCEDISK         ", 16);
}

void mac_scsi_set_drive_vendor (mac_scsi_t *scsi, unsigned id, const char *vendor)
{
	unsigned      i;
	unsigned char *dst;

	dst = scsi->dev[id & 7].vendor;

	for (i = 0; i < 8; i++) {
		if (*vendor == 0) {
			dst[i] = ' ';
		}
		else {
			dst[i] = *(vendor++);
		}
	}
}

void mac_scsi_set_drive_product (mac_scsi_t *scsi, unsigned id, const char *product)
{
	unsigned      i;
	unsigned char *dst;

	dst = scsi->dev[id & 7].product;

	for (i = 0; i < 16; i++) {
		if (*product == 0) {
			dst[i] = ' ';
		}
		else {
			dst[i] = *(product++);
		}
	}
}

static
int mac_scsi_set_buf_max (mac_scsi_t *scsi, unsigned long max)
{
	unsigned char *tmp;

	if (max <= scsi->buf_max) {
		return (0);
	}

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: buffer size = %lu\n", max);
#endif

	tmp = realloc (scsi->buf, max);

	if (tmp == NULL) {
		return (1);
	}

	scsi->buf = tmp;
	scsi->buf_max = max;

	return (0);
}

static
void mac_scsi_set_int (mac_scsi_t *scsi, int val)
{
	val = (val != 0);

	if (scsi->set_int_val == val) {
		return;
	}

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: interrupt = %d\n", val != 0);
#endif

	scsi->set_int_val = val;

	if (scsi->set_int != NULL) {
		scsi->set_int (scsi->set_int_ext, val);
	}
}

static
void mac_scsi_check_phase (mac_scsi_t *scsi)
{
	unsigned char dif;

	dif = ((scsi->csb >> 2) ^ scsi->tcr) & 0x07;

	if (dif) {
#ifdef DEBUG_SCSI
		mac_log_deb ("scsi: phase mismatch (%02X)\n", dif);
#endif
		scsi->bsr &= ~E5380_BSR_PHSM;
	}
	else {
		scsi->bsr |= E5380_BSR_PHSM;
	}
}

static
void mac_scsi_set_phase_free (mac_scsi_t *scsi)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: free\n");
#endif

	scsi->phase = E5380_PHASE_FREE;

	scsi->csb &= ~E5380_CSB_BSY;
	scsi->csb &= ~E5380_CSB_CD;
	scsi->csb &= ~E5380_CSB_MSG;
	scsi->csb &= ~E5380_CSB_IO;
	scsi->csb &= ~E5380_CSB_REQ;
}

static
void mac_scsi_set_phase_arb (mac_scsi_t *scsi)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: arbitration (id=%02X)\n", scsi->odr);
#endif

	scsi->phase = E5380_PHASE_ARB;

	scsi->csd = scsi->odr;
	scsi->icr |= E5380_ICR_AIP;
}

static
void mac_scsi_set_phase_sel (mac_scsi_t *scsi)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: selection\n");
#endif

	scsi->phase = E5380_PHASE_SEL;
}

static
void mac_scsi_set_phase_cmd (mac_scsi_t *scsi)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: command\n");
#endif

	scsi->phase = E5380_PHASE_CMD;

	scsi->csb |= E5380_CSB_BSY;
	scsi->csb |= E5380_CSB_CD;
	scsi->csb &= ~E5380_CSB_IO;
	scsi->csb &= ~E5380_CSB_MSG;
	scsi->csb |= E5380_CSB_REQ;

	scsi->cmd_i = 0;
	scsi->cmd_n = 16;
}

static
void mac_scsi_set_phase_data_in (mac_scsi_t *scsi)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: data in\n");
#endif

	scsi->phase = E5380_PHASE_DATA_IN;

	scsi->csb |= E5380_CSB_BSY;
	scsi->csb &= ~E5380_CSB_CD;
	scsi->csb |= E5380_CSB_IO;
	scsi->csb &= ~E5380_CSB_MSG;
	scsi->csb |= E5380_CSB_REQ;

	scsi->csd = scsi->buf[scsi->buf_i];
}

static
void mac_scsi_set_phase_data_out (mac_scsi_t *scsi)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: data out\n");
#endif

	scsi->phase = E5380_PHASE_DATA_OUT;

	scsi->csb |= E5380_CSB_BSY;
	scsi->csb &= ~E5380_CSB_CD;
	scsi->csb &= ~E5380_CSB_IO;
	scsi->csb &= ~E5380_CSB_MSG;
	scsi->csb |= E5380_CSB_REQ;
}

static
void mac_scsi_set_phase_status (mac_scsi_t *scsi, unsigned char val)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: status (%02X) cnt=%u\n", val, scsi->buf_i);
#endif

	scsi->phase = E5380_PHASE_STATUS;

	scsi->csb |= E5380_CSB_BSY;
	scsi->csb |= E5380_CSB_CD;
	scsi->csb |= E5380_CSB_IO;
	scsi->csb &= ~E5380_CSB_MSG;
	scsi->csb |= E5380_CSB_REQ;

	scsi->csd = val;
}

static
void mac_scsi_set_phase_msg_in (mac_scsi_t *scsi, unsigned char val)
{
#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: phase: msg in (%02X)\n", val);
#endif

	scsi->phase = E5380_PHASE_MSG_IN;

	scsi->csb |= E5380_CSB_BSY;
	scsi->csb |= E5380_CSB_CD;
	scsi->csb |= E5380_CSB_IO;
	scsi->csb |= E5380_CSB_MSG;
	scsi->csb |= E5380_CSB_REQ;

	scsi->csd = val;
}


static
void mac_scsi_select (mac_scsi_t *scsi, unsigned char msk)
{
	unsigned i;

	msk &= 0x7f;

	i = 0;
	while (msk != 0) {
		if (msk & 1) {
			break;
		}

		i += 1;
		msk >>= 1;
	}

	scsi->sel_drv = i;

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: select (id=%02X/%u)\n", scsi->odr, scsi->sel_drv);
#endif
}

static
mac_scsi_dev_t *mac_scsi_get_device (mac_scsi_t *scsi)
{
	unsigned id;

	id = scsi->sel_drv & 7;

	if (scsi->dev[id].valid) {
		return (&scsi->dev[id]);
	}

	return (NULL);
}

static
disk_t *mac_scsi_get_disk (mac_scsi_t *scsi)
{
	mac_scsi_dev_t *dev;
	disk_t         *dsk;

	dev = &scsi->dev[scsi->sel_drv & 7];

	if ((dev->valid == 0) || (dev->drive == 0xffff)) {
		return (NULL);
	}

	dsk = dsks_get_disk (scsi->dsks, dev->drive);

	return (dsk);
}


static
void mac_scsi_cmd_test_unit_ready (mac_scsi_t *scsi)
{
	mac_scsi_set_phase_status (scsi, 0x00);
}

static
void mac_scsi_cmd_sense (mac_scsi_t *scsi)
{
	memset (scsi->buf, 0, 13);
	scsi->buf[0] = 0xf0;

	scsi->buf_i = 0;
	scsi->buf_n = 13;

	mac_scsi_set_phase_data_in (scsi);
}

static
void mac_scsi_cmd_format_unit (mac_scsi_t *scsi)
{
	mac_scsi_set_phase_status (scsi, 0x00);
}

static
void mac_scsi_cmd_read (mac_scsi_t *scsi, unsigned long lba, unsigned long cnt)
{
	disk_t *dsk;

	dsk = mac_scsi_get_disk (scsi);

	if (dsk == NULL) {
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: read %u blocks at %lu\n", cnt, lba);
#endif

	if (mac_scsi_set_buf_max (scsi, 512UL * cnt)) {
		mac_log_deb ("scsi: too many blocks (%u)\n", cnt);
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	if (dsk_read_lba (dsk, scsi->buf, lba, cnt)) {
		mac_log_deb ("scsi: read error at %lu + %lu\n", lba, cnt);
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	scsi->buf_i = 0;
	scsi->buf_n = 512 * cnt;

	mac_scsi_set_phase_data_in (scsi);
}

static
void mac_scsi_cmd_read6 (mac_scsi_t *scsi)
{
	unsigned long lba;
	unsigned long cnt;

	/* lun = (scsi->cmd[1] >> 5) & 0x07; */

	lba = scsi->cmd[1] & 0x1f;
	lba = (lba << 8) | scsi->cmd[2];
	lba = (lba << 8) | scsi->cmd[3];

	cnt = scsi->cmd[4];
	if (cnt == 0) {
		cnt = 256;
	}

	mac_scsi_cmd_read (scsi, lba, cnt);
}

static
void mac_scsi_cmd_read10 (mac_scsi_t *scsi)
{
	unsigned long lba;
	unsigned long cnt;

	/* lun = (scsi->cmd[1] >> 5) & 0x07; */

	lba = scsi->cmd[2];
	lba = (lba << 8) | scsi->cmd[3];
	lba = (lba << 8) | scsi->cmd[4];
	lba = (lba << 8) | scsi->cmd[5];

	cnt = scsi->cmd[7];
	cnt = (cnt << 8) | scsi->cmd[8];

	mac_scsi_cmd_read (scsi, lba, cnt);
}

static
void mac_scsi_cmd_write_finish (mac_scsi_t *scsi, unsigned long lba, unsigned long cnt)
{
	disk_t *dsk;

	dsk = mac_scsi_get_disk (scsi);

	if (dsk == NULL) {
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	if ((512 * cnt) != scsi->buf_i) {
		mac_log_deb ("scsi: write size mismatch (%u / %u)\n",
			512 * cnt, scsi->buf_i
		);
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: write %u blocks at %lu\n", cnt, lba);
#endif

	if (dsk_write_lba (dsk, scsi->buf, lba, cnt)) {
		mac_log_deb ("scsi: write error\n");
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	scsi->buf_i = 0;
	scsi->buf_n = 0;

	scsi->cmd_finish = NULL;

	mac_scsi_set_phase_status (scsi, 0x00);
}

static
void mac_scsi_cmd_write6_finish (mac_scsi_t *scsi)
{
	unsigned long lba;
	unsigned long cnt;

	/* lun = (scsi->cmd[1] >> 5) & 0x07; */

	lba = scsi->cmd[1] & 0x1f;
	lba = (lba << 8) | scsi->cmd[2];
	lba = (lba << 8) | scsi->cmd[3];

	cnt = scsi->cmd[4];
	if (cnt == 0) {
		cnt = 256;
	}

	mac_scsi_cmd_write_finish (scsi, lba, cnt);
}

static
void mac_scsi_cmd_write6 (mac_scsi_t *scsi)
{
	unsigned cnt;

	cnt = scsi->cmd[4];

	if (cnt == 0) {
		cnt = 256;
	}

	if (mac_scsi_set_buf_max (scsi, 512UL * cnt)) {
		mac_log_deb ("scsi: write block count %u\n", cnt);
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	scsi->buf_i = 0;
	scsi->buf_n = 512 * cnt;

	scsi->cmd_finish = mac_scsi_cmd_write6_finish;

	mac_scsi_set_phase_data_out (scsi);
}

static
void mac_scsi_cmd_write10_finish (mac_scsi_t *scsi)
{
	unsigned long lba;
	unsigned      cnt;

	/* lun = (scsi->cmd[1] >> 5) & 0x07; */

	lba = scsi->cmd[2];
	lba = (lba << 8) | scsi->cmd[3];
	lba = (lba << 8) | scsi->cmd[4];
	lba = (lba << 8) | scsi->cmd[5];

	cnt = (scsi->cmd[7] << 8) | scsi->cmd[8];

	mac_scsi_cmd_write_finish (scsi, lba, cnt);
}

static
void mac_scsi_cmd_write10 (mac_scsi_t *scsi)
{
	unsigned cnt;

	cnt = (scsi->cmd[7] << 8) | scsi->cmd[8];

	if (mac_scsi_set_buf_max (scsi, 512UL * cnt)) {
		mac_log_deb ("scsi: write block count %u\n", cnt);
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	scsi->buf_i = 0;
	scsi->buf_n = 512 * cnt;

	scsi->cmd_finish = mac_scsi_cmd_write10_finish;

	mac_scsi_set_phase_data_out (scsi);
}

static
void mac_scsi_cmd_verify10 (mac_scsi_t *scsi)
{
	unsigned long lba;
	unsigned      cnt;
	disk_t        *dsk;

	dsk = mac_scsi_get_disk (scsi);

	if (dsk == NULL) {
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	/* lun = (scsi->cmd[1] >> 5) & 0x07; */

	lba = scsi->cmd[2];
	lba = (lba << 8) | scsi->cmd[3];
	lba = (lba << 8) | scsi->cmd[4];
	lba = (lba << 8) | scsi->cmd[5];

	cnt = scsi->cmd[7];
	cnt = (cnt << 8) | scsi->cmd[8];

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: verify %u blocks at %lu\n", cnt, lba);
#endif

	scsi->buf_i = 0;
	scsi->buf_n = 0;

	mac_scsi_set_phase_status (scsi, 0x00);
}

static
void mac_scsi_cmd_inquiry (mac_scsi_t *scsi)
{
	mac_scsi_dev_t *dev;

	dev = mac_scsi_get_device (scsi);

	memset (scsi->buf, 0, 256);

	if (dev != NULL) {
		memcpy (scsi->buf + 8, dev->vendor, 8);
		memcpy (scsi->buf + 16, dev->product, 16);
	}

	scsi->buf[4] = 32;

	scsi->buf_i = 0;
	scsi->buf_n = (scsi->cmd[4] < 36) ? scsi->cmd[4] : 36;

	mac_scsi_set_phase_data_in (scsi);
}

static
void mac_scsi_cmd_mode_select (mac_scsi_t *scsi)
{
	scsi->buf_i = 0;
	scsi->buf_n = 0;

	mac_scsi_set_phase_data_out (scsi);
}

static
void mac_scsi_cmd_mode_sense (mac_scsi_t *scsi)
{
	disk_t *dsk;

	dsk = mac_scsi_get_disk (scsi);

	if (dsk == NULL) {
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	memset (scsi->buf, 0, 512);

	scsi->buf_i = 0;
	scsi->buf_n = 0;

	switch (scsi->cmd[2] & 0x3f) {
	case 0x01: /* read-write error recovery page */
		scsi->buf[0] = 0x01;
		scsi->buf[1] = 10;
		scsi->buf_n = 12;
		break;

	case 0x03: /* format device page */
		scsi->buf[0] = 0x03;
		scsi->buf[1] = 22;
		scsi->buf_n = 24;
		break;

	case 0x04: /* rigid disk drive geometry page */
		scsi->buf[0] = 0x04;
		scsi->buf[1] = 22;
		scsi->buf[2] = 0;
		buf_set_uint16_be (scsi->buf, 3, dsk->c);
		scsi->buf[5] = dsk->h;
		buf_set_uint16_be (scsi->buf, 20, 3600);
		scsi->buf_n = 32;
		break;

	case 0x30: /* vendor specific */
		scsi->buf[0] = 0x30;
		scsi->buf[1] = 33;
		strcpy ((char *) scsi->buf + 14, "APPLE COMPUTER, INC");
		scsi->buf_n = 34;
		break;

	default:
		mac_log_deb ("scsi: mode sense: unknown mode page (%02X)\n",
			scsi->cmd[2]
		);
		break;
	}

	if (scsi->buf_n > 0) {
		mac_scsi_set_phase_data_in (scsi);
	}
	else {
		mac_scsi_set_phase_status (scsi, 0x02);
	}
}

static
void mac_scsi_cmd_start_stop (mac_scsi_t *scsi)
{
	const char *str;

	switch (scsi->cmd[4] & 3) {
	case 0:
		str = "stop motor";
		break;
	case 1:
		str = "start motor";
		break;
	case 2:
		str = "eject media";
		break;
	case 3:
		str = "load media";
		break;
	}

	mac_log_deb ("scsi: start/stop unit %u (%s)\n", scsi->sel_drv, str);

	mac_scsi_set_phase_status (scsi, 0x00);
}

static
void mac_scsi_cmd_read_capacity (mac_scsi_t *scsi)
{
	unsigned long cnt;
	disk_t        *dsk;

	dsk = mac_scsi_get_disk (scsi);

	if (dsk == NULL) {
		mac_scsi_set_phase_status (scsi, 0x02);
		return;
	}

	cnt = dsk_get_block_cnt (dsk);
	buf_set_uint32_be (scsi->buf, 0, cnt - 1);
	buf_set_uint32_be (scsi->buf, 4, 512);

	scsi->buf_i = 0;
	scsi->buf_n = 8;

	mac_scsi_set_phase_data_in (scsi);
}

static
void mac_scsi_cmd_read_buffer (mac_scsi_t *scsi)
{
	memset (scsi->buf, 0, 512);

	scsi->buf_i = 0;
	scsi->buf_n = 4;

	mac_scsi_set_phase_data_in (scsi);
}


static
void mac_scsi_set_cmd (mac_scsi_t *scsi, unsigned cnt, void (*cmd) (mac_scsi_t *scsi))
{
	scsi->cmd_n = cnt;
	scsi->cmd_start = cmd;
}

static
void mac_scsi_cmd_init (mac_scsi_t *scsi, unsigned char cmd)
{
	scsi->cmd_start = NULL;
	scsi->cmd_finish = NULL;

	switch (cmd) {
	case 0x00:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_test_unit_ready);
		break;

	case 0x03:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_sense);
		break;

	case 0x04:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_format_unit);
		break;

	case 0x08:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_read6);
		break;

	case 0x0a:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_write6);
		break;

	case 0x12:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_inquiry);
		break;

	case 0x15:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_mode_select);
		break;

	case 0x1a:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_mode_sense);
		break;

	case 0x1b:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_start_stop);
		break;

	case 0x25:
		mac_scsi_set_cmd (scsi, 10, mac_scsi_cmd_read_capacity);
		break;

	case 0x28:
		mac_scsi_set_cmd (scsi, 10, mac_scsi_cmd_read10);
		break;

	case 0x2a:
		mac_scsi_set_cmd (scsi, 10, mac_scsi_cmd_write10);
		break;

	case 0x2f:
		mac_scsi_set_cmd (scsi, 10, mac_scsi_cmd_verify10);
		break;

	case 0x3c:
		mac_scsi_set_cmd (scsi, 6, mac_scsi_cmd_read_buffer);
		break;

	default:
		mac_log_deb ("scsi: unknown command (%02X)\n", cmd);
		mac_scsi_set_phase_status (scsi, 0x02);
		break;
	}
}


static
unsigned char mac_scsi_get_csd_dma (mac_scsi_t *scsi)
{
	unsigned char val;

	if (scsi->phase != E5380_PHASE_DATA_IN) {
		return (0);
	}

	if (scsi->buf_i >= scsi->buf_n) {
		return (0);
	}

	val = scsi->buf[scsi->buf_i];
	scsi->buf_i += 1;

	if (scsi->buf_i >= scsi->buf_n) {
		mac_scsi_set_phase_status (scsi, 0x00);
	}

	return (val);
}

static
unsigned char mac_scsi_get_icr (mac_scsi_t *scsi)
{
	return (scsi->icr);
}

static
unsigned char mac_scsi_get_mr2 (mac_scsi_t *scsi)
{
	return (scsi->mr2);
}

static
unsigned char mac_scsi_get_tcr (mac_scsi_t *scsi)
{
	return (scsi->tcr & 0x0f);
}

static
unsigned char mac_scsi_get_csb (mac_scsi_t *scsi)
{
	return (scsi->csb);
}

static
unsigned char mac_scsi_get_bsr (mac_scsi_t *scsi)
{
	return (scsi->bsr);
}

unsigned char mac_scsi_get_uint8 (void *ext, unsigned long addr)
{
	unsigned char val;
	mac_scsi_t    *scsi = ext;

	addr = (addr & scsi->addr_mask) >> scsi->addr_shift;

	switch (addr) {
	case 0x00: /* CSD */
		val = scsi->csd;
		break;

	case 0x01: /* ICR */
		val = mac_scsi_get_icr (scsi);
		break;

	case 0x02: /* MR2 */
		val = mac_scsi_get_mr2 (scsi);
		break;

	case 0x03: /* TCR */
		val = mac_scsi_get_tcr (scsi);
		break;

	case 0x04: /* CSB */
		val = mac_scsi_get_csb (scsi);
		break;

	case 0x05: /* BSR */
		mac_scsi_check_phase (scsi);
		val = mac_scsi_get_bsr (scsi);
		break;

	case 0x06: /* IDR */
	case 0x26:
		val = mac_scsi_get_csd_dma (scsi);
		break;

	case 0x07: /* RPI */
		mac_scsi_set_int (scsi, 0);
		val = 0xff;
		break;

	case 0x20: /* CSD with DACK */
		val = mac_scsi_get_csd_dma (scsi);
		break;

	default:
		val = 0xff;
		mac_log_deb ("scsi: get  8: %04lX -> %02X\n", addr, val);
		break;
	}

	return (val);
}

unsigned short mac_scsi_get_uint16 (void *ext, unsigned long addr)
{
	mac_scsi_t *scsi = ext;

	addr = (addr & scsi->addr_mask) >> scsi->addr_shift;

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: set 16: %04lX -> %02X\n", addr, 0x00);
#endif

	return (0);
}


static
void mac_scsi_set_odr (mac_scsi_t *scsi, unsigned char val)
{
	scsi->odr = val;
}

static
void mac_scsi_set_odr_dma (mac_scsi_t *scsi, unsigned char val)
{
	if (scsi->phase != E5380_PHASE_DATA_OUT) {
		return;
	}

	if (scsi->buf_i >= scsi->buf_n) {
		return;
	}

	scsi->buf[scsi->buf_i] = val;
	scsi->buf_i += 1;

	if (scsi->buf_i >= scsi->buf_n) {
		if (scsi->cmd_finish != NULL) {
			scsi->cmd_finish (scsi);
		}
		else {
			mac_scsi_set_phase_status (scsi, 0x02);
		}
	}
}

static
void mac_scsi_set_icr (mac_scsi_t *scsi, unsigned char val)
{
	unsigned char dif;

	val &= ~0x60;

	dif = scsi->icr ^ val;
	scsi->icr = val;

	switch (scsi->phase) {
	case E5380_PHASE_ARB:
		if (dif & val & E5380_ICR_SEL) {
			mac_scsi_set_phase_sel (scsi);
		}
		break;

	case E5380_PHASE_CMD:
		if (dif & val & E5380_ICR_ACK) {
			if (scsi->cmd_i < scsi->cmd_n) {
				scsi->cmd[scsi->cmd_i] = scsi->odr;
				scsi->cmd_i += 1;
			}

			if (scsi->cmd_i == 1) {
				mac_scsi_cmd_init (scsi, scsi->cmd[0]);
			}

			scsi->csb &= ~E5380_CSB_REQ;

#ifdef DEBUG_SCSI
			mac_log_deb ("scsi: command byte: %02X (%u/%u)\n",
				scsi->odr, scsi->cmd_i, scsi->cmd_n
			);
#endif
		}

		if (dif & ~val & E5380_ICR_ACK) {
			if (scsi->cmd_i >= scsi->cmd_n) {
				if (scsi->cmd_start != NULL) {
#ifdef DEBUG_SCSI
					mac_log_deb (
						"scsi: command exec (%u) %02X %02X %02X %02X %02X %02X\n",
						scsi->cmd_i,
						scsi->cmd[0], scsi->cmd[1],
						scsi->cmd[2], scsi->cmd[3],
						scsi->cmd[4], scsi->cmd[5]
					);
#endif

					scsi->cmd_start (scsi);
				}
				else {
					mac_scsi_set_phase_status (scsi, 0x02);
				}
			}
			else {
				scsi->csb |= E5380_CSB_REQ;
			}
		}
		break;

	case E5380_PHASE_DATA_IN:
		if (dif & val & E5380_ICR_ACK) {
			scsi->csb &= ~E5380_CSB_REQ;
#ifdef DEBUG_SCSI
			mac_log_deb ("scsi: data ack (%02X)\n", scsi->csd);
#endif
		}

		if (dif & ~val & E5380_ICR_ACK) {
			scsi->buf_i += 1;
			if (scsi->buf_i < scsi->buf_n) {
				scsi->csd = scsi->buf[scsi->buf_i];
				scsi->csb |= E5380_CSB_REQ;
			}
			else {
				mac_scsi_set_phase_status (scsi, 0x00);
			}
		}
		break;

	case E5380_PHASE_DATA_OUT:
		if (dif & val & E5380_ICR_ACK) {
			if (scsi->buf_i < scsi->buf_n) {
				scsi->buf[scsi->buf_i] = scsi->odr;
				scsi->buf_i += 1;
			}
			scsi->csb &= ~E5380_CSB_REQ;
#ifdef DEBUG_SCSI
			mac_log_deb ("scsi: data out ack (%02X)\n", scsi->odr);
#endif
		}

		if (dif & ~val & E5380_ICR_ACK) {
			if (scsi->buf_i >= scsi->buf_n) {
				/* command */
				mac_scsi_set_phase_status (scsi, 0x02);
			}
			else {
				scsi->csb |= E5380_CSB_REQ;
			}
		}
		break;

	case E5380_PHASE_STATUS:
		if (dif & val & E5380_ICR_ACK) {
			scsi->csb &= ~E5380_CSB_REQ;
		}

		if (dif & ~val & E5380_ICR_ACK) {
#ifdef DEBUG_SCSI
			mac_log_deb ("scsi: status ack\n");
#endif
			mac_scsi_set_phase_msg_in (scsi, 0x00);
		}
		break;

	case E5380_PHASE_MSG_IN:
		if (dif & val & E5380_ICR_ACK) {
			scsi->csb &= ~E5380_CSB_REQ;
		}

		if (dif & ~val & E5380_ICR_ACK) {
			mac_scsi_set_phase_free (scsi);
		}
		break;
	}
}

static
void mac_scsi_set_mr2 (mac_scsi_t *scsi, unsigned char val)
{
	unsigned char dif;

	dif = scsi->mr2 ^ val;
	scsi->mr2 = val;

	switch (scsi->phase) {
	case E5380_PHASE_FREE:
		if (dif & val & E5380_MR2_ARB) {
			mac_scsi_set_phase_arb (scsi);
		}
		break;

	case E5380_PHASE_SEL:
		if (dif & ~val & E5380_MR2_ARB) {
			mac_scsi_select (scsi, scsi->odr);

			scsi->icr &= ~E5380_ICR_BSY;

			if (mac_scsi_get_disk (scsi) != NULL) {
				mac_scsi_set_phase_cmd (scsi);
				mac_scsi_set_int (scsi, 1);
			}
			else {
				mac_scsi_set_phase_free (scsi);
			}
		}
		break;

	case E5380_PHASE_DATA_IN:
		if (dif & val & E5380_MR2_DMA) {
			if (scsi->buf_i < scsi->buf_n) {
				scsi->bsr |= E5380_BSR_DRQ;
			}
		}
		break;

	case E5380_PHASE_DATA_OUT:
		if (dif & val & E5380_MR2_DMA) {
			if (scsi->buf_i < scsi->buf_n) {
				scsi->bsr |= E5380_BSR_DRQ;
			}
		}
		break;
	}

	if (dif & ~val & E5380_MR2_DMA) {
		scsi->bsr &= ~E5380_BSR_DRQ;
	}
}

static
void mac_scsi_set_tcr (mac_scsi_t *scsi, unsigned char val)
{
	unsigned char dif;

	dif = scsi->tcr ^ val;
	scsi->tcr = val;

	if (dif & val & E5380_TCR_CD) {
#ifdef DEBUG_SCSI
		mac_log_deb ("scsi: set c/d\n");
#endif
		if (scsi->phase == E5380_PHASE_DATA_IN) {
			mac_scsi_set_phase_status (scsi, 0x00);
		}
		if (scsi->phase == E5380_PHASE_DATA_OUT) {
			mac_scsi_set_phase_status (scsi, 0x00);
		}
	}

#ifdef DEBUG_SCSI
	if (dif & ~val & E5380_TCR_CD) {
		mac_log_deb ("scsi: clear c/d\n");
	}
#endif

#ifdef DEBUG_SCSI
	if (dif & val & E5380_TCR_IO) {
		mac_log_deb ("scsi: set i/o\n");
	}
#endif

#ifdef DEBUG_SCSI
	if (dif & ~val & E5380_TCR_IO) {
		mac_log_deb ("scsi: clear i/o\n");
	}
#endif

#ifdef DEBUG_SCSI
	if (dif & val & E5380_TCR_MSG) {
		mac_log_deb ("scsi: set msg\n");
	}
#endif

#ifdef DEBUG_SCSI
	if (dif & ~val & E5380_TCR_MSG) {
		mac_log_deb ("scsi: clear msg\n");
	}
#endif
}

static
void mac_scsi_set_ser (mac_scsi_t *scsi, unsigned char val)
{
	scsi->ser = val;
}

void mac_scsi_set_uint8 (void *ext, unsigned long addr, unsigned char val)
{
	mac_scsi_t *scsi = ext;

	addr = (addr & scsi->addr_mask) >> scsi->addr_shift;

	switch (addr) {
	case 0x00: /* ODR */
		mac_scsi_set_odr (scsi, val);
		break;

	case 0x01: /* ICR */
		mac_scsi_set_icr (scsi, val);
		break;

	case 0x02: /* MR2 */
		mac_scsi_set_mr2 (scsi, val);
		break;

	case 0x03: /* TCR */
		mac_scsi_set_tcr (scsi, val);
		break;

	case 0x04: /* SER */
		mac_scsi_set_ser (scsi, val);
		break;

	case 0x05: /* start dma send */
		scsi->bsr |= E5380_BSR_DRQ;
		break;

	case 0x06: /* start dma target receive */
		break;

	case 0x07: /* start dma initiator receive */
		break;

	case 0x20: /* ODR with dma */
		mac_scsi_set_odr_dma (scsi, val);
		break;

	default:
		mac_log_deb ("scsi: set  8: %04lX <- %02X\n", addr, val);
		break;
	}
}

void mac_scsi_set_uint16 (void *ext, unsigned long addr, unsigned short val)
{
	mac_scsi_t *scsi = ext;

	addr = (addr & scsi->addr_mask) >> scsi->addr_shift;

#ifdef DEBUG_SCSI
	mac_log_deb ("scsi: set 16: %04lX <- %02X\n", addr, val);
#endif
}

void mac_scsi_reset (mac_scsi_t *scsi)
{
#ifdef DEBUG_SCSI
	pce_log_deb ("scsi: reset\n");
#endif

	scsi->phase = E5380_PHASE_FREE;

	scsi->odr = 0x00;
	scsi->csd = 0x00;
	scsi->icr = 0x00;
	scsi->mr2 = 0x00;
	scsi->csb = 0x00;
	scsi->ser = 0x00;
	scsi->bsr = E5380_BSR_PHSM;

	scsi->status = 0x00;

	scsi->cmd_i = 0;
	scsi->cmd_n = 0;

	scsi->buf_i = 0;
	scsi->buf_n = 0;

	scsi->cmd_start = NULL;
	scsi->cmd_finish = NULL;
}
