/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/acsi.c                                      *
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


#include "main.h"
#include "acsi.h"

#include <string.h>

#include <devices/memory.h>
#include <lib/log.h>

#ifndef DEBUG_ACSI
#define DEBUG_ACSI 0
#endif


void st_acsi_init (st_acsi_t *acsi)
{
	unsigned i;

	acsi->cmd_cnt = 0;
	acsi->result = 0;

	for (i = 0; i < 8; i++) {
		acsi->disk_id[i] = 0xffff;
	}

	acsi->dsks = NULL;

	acsi->drq_val = 0;
	acsi->drq_ext = NULL;
	acsi->set_drq = NULL;

	acsi->irq_val = 0;
	acsi->irq_ext = NULL;
	acsi->set_irq = NULL;
}

void st_acsi_free (st_acsi_t *acsi)
{
}

void st_acsi_set_drq_fct (st_acsi_t *acsi, void *ext, void *fct)
{
	acsi->drq_ext = ext;
	acsi->set_drq = fct;
}

void st_acsi_set_irq_fct (st_acsi_t *acsi, void *ext, void *fct)
{
	acsi->irq_ext = ext;
	acsi->set_irq = fct;
}

void st_acsi_set_disks (st_acsi_t *acsi, disks_t *dsks)
{
	acsi->dsks = dsks;
}

void st_acsi_set_disk_id (st_acsi_t *acsi, unsigned idx, unsigned val)
{
	acsi->disk_id[idx & 7] = val;
}

static
void st_acsi_set_drq (st_acsi_t *acsi, unsigned char val)
{
	if (acsi->drq_val != val) {
		acsi->drq_val = val;

		if (acsi->set_drq != NULL) {
			acsi->set_drq (acsi->drq_ext, acsi->drq_val);
		}
	}
}

static
void st_acsi_set_irq (st_acsi_t *acsi, unsigned char val)
{
	if (acsi->irq_val != val) {
		acsi->irq_val = val;

		if (acsi->set_irq != NULL) {
			acsi->set_irq (acsi->irq_ext, acsi->irq_val);
		}
	}
}

static
void st_acsi_set_result (st_acsi_t *acsi, unsigned val)
{
	acsi->result = val;
	acsi->sense = 0;

	st_acsi_set_drq (acsi, 0);
	st_acsi_set_irq (acsi, 1);
}

unsigned char st_acsi_get_result (st_acsi_t *acsi)
{
	st_acsi_set_irq (acsi, 0);

	return (acsi->result);
}

static
disk_t *st_acsi_get_disk (st_acsi_t *acsi)
{
	unsigned drv;
	disk_t   *dsk;

	drv = (acsi->cmd[0] >> 5) & 7;
	dsk = dsks_get_disk (acsi->dsks, acsi->disk_id[drv]);

	return (dsk);
}

/*
 * CMD 00: TEST UNIT READY
 */
static
void st_acsi_cmd_00 (st_acsi_t *acsi)
{
#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%X/%02X] TEST UNIT READY\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f, acsi->cmd[4]
	);
#endif

	if ((st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	st_acsi_set_result (acsi, 0);
}

/*
 * CMD 03: REQUEST SENSE
 */
static
void st_acsi_cmd_03 (st_acsi_t *acsi)
{
#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%X/%02X] REQUEST SENSE (N=%02X)\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f, acsi->cmd[4]
	);
#endif

	memset (acsi->buf, 0, 256);

	acsi->buf[0] = acsi->sense;

	acsi->buf_idx = 0;
	acsi->buf_cnt = acsi->cmd[4];

	acsi->result = 0;
}

/*
 * CMD 04: FORMAT
 */
static
void st_acsi_cmd_04 (st_acsi_t *acsi)
{
#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%02X] FORMAT (IL=%02X)\n",
		acsi->cmd[0], acsi->cmd[4]
	);
#endif

	st_acsi_set_result (acsi, 0);
}

/*
 * CMD 08: READ
 */
static
void st_acsi_cmd_08 (st_acsi_t *acsi)
{
	disk_t *dsk;

#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%X/%02X] READ (%lu + %u)\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f, acsi->blk, acsi->cnt
	);
#endif

	if ((dsk = st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	if (dsk_read_lba (dsk, acsi->buf, acsi->blk, acsi->cnt)) {
		st_acsi_set_result (acsi, 0x14);
		acsi->sense = 0x05;
		return;
	}

	acsi->buf_idx = 0;
	acsi->buf_cnt = 512 * acsi->cnt;

	acsi->result = 0;
}

static
void st_acsi_cmd_0a_cont (st_acsi_t *acsi)
{
	disk_t *dsk;

#if DEBUG_ACSI >= 2
	st_log_deb ("ACSI: CMD[%X/%02X] WRITE CONT (%lu + %u)\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f, acsi->blk, acsi->cnt
	);
#endif

	if ((dsk = st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	if (dsk_write_lba (dsk, acsi->buf, acsi->blk, acsi->cnt)) {
		st_acsi_set_result (acsi, 0x14);
		acsi->sense = 0x05;
		return;
	}

	st_acsi_set_result (acsi, 0);
}

/*
 * CMD 0A: WRITE
 */
static
void st_acsi_cmd_0a (st_acsi_t *acsi)
{
	disk_t *dsk;

#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%X/%02X] WRITE (%lu + %u)\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f, acsi->blk, acsi->cnt
	);
#endif

	if ((dsk = st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	if (dsk_read_lba (dsk, acsi->buf, acsi->blk, acsi->cnt)) {
		st_acsi_set_result (acsi, 0x14);
		return;
	}

	acsi->buf_idx = 0;
	acsi->buf_cnt = 512UL * acsi->cnt;

	acsi->result = 0;
}

/*
 * CMD 12: INQUIRY
 */
static
void st_acsi_cmd_12 (st_acsi_t *acsi)
{
	unsigned cnt;
	disk_t   *dsk;

#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%X/%02X] INQUIRY (n=%u)\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f, acsi->cmd[4]
	);
#endif

	if ((dsk = st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	cnt = acsi->cmd[4];

	memset (acsi->buf, 0, 256);
	memcpy (acsi->buf + 8, "PCE     ", 8);
	memcpy (acsi->buf + 16, "PCEDISK         ", 16);

	acsi->buf[2] = 1;
	acsi->buf[4] = 32;

	acsi->buf_idx = 0;
	acsi->buf_cnt = (cnt < 36) ? cnt : 36;

	acsi->result = 0;
}

static
void st_acsi_cmd_15_cont (st_acsi_t *acsi)
{
	unsigned i;
	disk_t   *dsk;

#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%X/%02X] MODE SELECT CONT (%u)\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f, acsi->cnt
	);
#endif

	if ((dsk = st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	fputs ("mode select:\n", stderr);

	for (i = 0; i < acsi->cnt; i++) {
		fprintf (stderr, "%02X", acsi->buf[i]);

		if ((i + 1) & 15) {
			fputs (" ", stderr);
		}
		else {
			fputs ("\n", stderr);
		}
	}

	fputs ("\n", stderr);

	st_acsi_set_result (acsi, 0);
}

/*
 * CMD 15: MODE SELECT
 */
static
void st_acsi_cmd_15 (st_acsi_t *acsi)
{
	disk_t *dsk;

#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%X/%02X] MODE SELECT (PF=%d N=%02X)\n",
		acsi->cmd[0] >> 5, acsi->cmd[0] & 0x1f,
		(acsi->cmd[1] >> 4) & 1, acsi->cmd[4]
	);
#endif

	if ((dsk = st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	acsi->buf_idx = 0;
	acsi->buf_cnt = acsi->cmd[4];

	acsi->result = 0;
}

/*
 * CMD 1A: MODE SENSE
 */
static
void st_acsi_cmd_1a (st_acsi_t *acsi)
{
	disk_t *dsk;

#if DEBUG_ACSI >= 1
	st_log_deb ("ACSI: CMD[%02X] MODE SENSE (PAGE=%02X N=%02X)\n",
		acsi->cmd[0], acsi->cmd[2], acsi->cmd[4]
	);
#endif

	if ((dsk = st_acsi_get_disk (acsi)) == NULL) {
		st_acsi_set_result (acsi, 0x04);
		return;
	}

	memset (acsi->buf, 0, 256);

	acsi->buf_idx = 0;
	acsi->buf_cnt = 0;

	switch (acsi->cmd[2] & 0x3f) {
	case 0x00: /* vendor specific */
		acsi->buf[3] = 8;
		buf_set_uint32_be (acsi->buf, 4, dsk->blocks);
		buf_set_uint32_be (acsi->buf, 8, 512);
		acsi->buf_cnt = 16;
		break;

	case 0x01: /* read-write error recovery page */
		acsi->buf[0] = 0x01;
		acsi->buf[1] = 10;
		acsi->buf_cnt = 12;
		break;

	case 0x03: /* format device page */
		acsi->buf[0] = 0x03;
		acsi->buf[1] = 22;
		acsi->buf_cnt = 24;
		break;

	case 0x04: /* rigid disk drive geometry page */
		acsi->buf[0] = 0x04;
		acsi->buf[1] = 22;
		acsi->buf[2] = 0;
		buf_set_uint16_be (acsi->buf, 3, dsk->c);
		acsi->buf[5] = dsk->h;
		buf_set_uint16_be (acsi->buf, 20, 3600);
		acsi->buf_cnt = 32;
		break;

	default:
		st_log_deb ("acsi: mode sense: unknown mode page (%02X)\n",
			acsi->cmd[2]
		);
		break;
	}

	if (acsi->buf_cnt > 0) {
		acsi->result = 0;
	}
	else {
		st_acsi_set_result (acsi, 0x02);
	}
}

static
void st_acsi_cmd (st_acsi_t *acsi)
{
	switch (acsi->cmd[0] & 0x1f) {
	case 0x00:
		st_acsi_cmd_00 (acsi);
		break;

	case 0x03:
		st_acsi_cmd_03 (acsi);
		break;

	case 0x04:
		st_acsi_cmd_04 (acsi);
		break;

	case 0x08:
		st_acsi_cmd_08 (acsi);
		break;

	case 0x0a:
		st_acsi_cmd_0a (acsi);
		break;

	case 0x12:
		st_acsi_cmd_12 (acsi);
		break;

	case 0x15:
		st_acsi_cmd_15 (acsi);
		break;

	case 0x1a:
		st_acsi_cmd_1a (acsi);
		break;

	default:
		st_log_deb ("ACSI: CMD[%02X] UNKNOWN [%02X %02X %02X %02X %02X %02X]\n",
			acsi->cmd[0],
			acsi->cmd[0], acsi->cmd[1], acsi->cmd[2],
			acsi->cmd[3], acsi->cmd[4], acsi->cmd[5]
		);

		st_acsi_set_result (acsi, 0x20);
	}

	acsi->cmd_cnt = 0;
}

unsigned char st_acsi_get_data (st_acsi_t *acsi)
{
	unsigned char val;

	st_acsi_set_drq (acsi, 0);

	if (acsi->buf_idx >= acsi->buf_cnt) {
		return (0);
	}

	val = acsi->buf[acsi->buf_idx++];

	if (acsi->buf_idx >= acsi->buf_cnt) {
		st_acsi_set_irq (acsi, 1);
	}

	return (val);
}

void st_acsi_set_data (st_acsi_t *acsi, unsigned char val)
{
	st_acsi_set_drq (acsi, 0);

	if (acsi->buf_idx >= acsi->buf_cnt) {
		return;
	}

	acsi->buf[acsi->buf_idx++] = val;

	if (acsi->buf_idx >= acsi->buf_cnt) {
		switch (acsi->cmd[0] & 0x1f) {
		case 0x0a:
			st_acsi_cmd_0a_cont (acsi);
			break;

		case 0x15:
			st_acsi_cmd_15_cont (acsi);
			break;

		default:
			st_log_deb (
				"ACSI: data out for unknown command (%02X)\n",
				acsi->cmd[0]
			);
			break;
		}
	}
}

void st_acsi_set_cmd (st_acsi_t *acsi, unsigned char val, int a0)
{
	st_acsi_set_drq (acsi, 0);
	st_acsi_set_irq (acsi, 0);

#if DEBUG_ACSI >= 3
	st_log_deb ("ACSI: command byte %02X\n", val);
#endif

	if (acsi->cmd_cnt >= 6) {
		acsi->cmd_cnt = 0;
		return;
	}

	acsi->cmd[acsi->cmd_cnt++] = val;

	if (acsi->cmd_cnt == 1) {
		if (st_acsi_get_disk (acsi) == NULL) {
#if DEBUG_ACSI >= 1
			st_log_deb ("ACSI: ignoring command %02X\n", val);
#endif
			acsi->cmd_cnt = 0;
			return;
		}
	}

	if (acsi->cmd_cnt >= 6) {
		acsi->blk = acsi->cmd[1];
		acsi->blk = (acsi->blk << 8) | acsi->cmd[2];
		acsi->blk = (acsi->blk << 8) | acsi->cmd[3];
		acsi->cnt = acsi->cmd[4];
		acsi->buf_idx = 0;
		acsi->buf_cnt = 0;

		st_acsi_cmd (acsi);
	}
	else {
		st_acsi_set_irq (acsi, 1);
	}
}

void st_acsi_reset (st_acsi_t *acsi)
{
	acsi->cmd_cnt = 0;

	acsi->buf_idx = 0;
	acsi->buf_cnt = 0;
	acsi->result = 0;

	st_acsi_set_drq (acsi, 0);
	st_acsi_set_irq (acsi, 0);
}

void st_acsi_clock (st_acsi_t *acsi)
{
	unsigned i;

	if (acsi->buf_idx >= acsi->buf_cnt) {
		return;
	}

	for (i = acsi->buf_idx; i < acsi->buf_cnt; i++) {
		st_acsi_set_drq (acsi, 1);
	}
}
