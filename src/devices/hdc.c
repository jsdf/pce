/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/hdc.c                                            *
 * Created:     2011-09-11 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2011-2013 Hampa Hug <hampa@hampa.ch>                     *
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

/* IBM XT hard disk controller */


#include <stdlib.h>
#include <string.h>

#include <devices/hdc.h>
#include <devices/memory.h>

#include <lib/log.h>


#ifndef DEBUG_HDC
#define DEBUG_HDC 0
#endif


#define HDC_STATUS_REQ 0x01
#define HDC_STATUS_INP 0x02
#define HDC_STATUS_CMD 0x04
#define HDC_STATUS_BSY 0x08
#define HDC_STATUS_DRQ 0x10
#define HDC_STATUS_INT 0x20

#define HDC_MASK_DRQ 0x01
#define HDC_MASK_IRQ 0x02


#if DEBUG_HDC >= 1
static
void hdc_print_hex (FILE *fp, const char *tag, const unsigned char *buf, unsigned cnt)
{
	unsigned i;

	if (tag != NULL) {
		fputs (tag, fp);
	}

	for (i = 0; i < cnt; i++) {
		if ((i & 7) == 0) {
			fputs ("\n\t", fp);
		}

		fprintf (fp, " %02X", buf[i]);
	}

	fputc ('\n', fp);
}
#endif

static
void hdc_set_irq (hdc_t *hdc, unsigned char val)
{
	if ((hdc->mask & HDC_MASK_IRQ) == 0) {
		val = 0;
	}
	else {
		val = (val != 0);
	}

	if (val) {
		hdc->status |= HDC_STATUS_INT;
	}
	else {
		hdc->status &= ~HDC_STATUS_INT;
	}

	if (hdc->irq_val != val) {
		hdc->irq_val = val;

		if (hdc->irq != NULL) {
			hdc->irq (hdc->irq_ext, val);
		}
	}
}

static
void hdc_set_dreq (hdc_t *hdc, unsigned char val)
{
	val = (val != 0);

	if (val) {
		hdc->status |= HDC_STATUS_DRQ;
	}
	else {
		hdc->status &= ~HDC_STATUS_DRQ;
	}

	if (hdc->dreq_val != val) {
		hdc->dreq_val = val;

		if ((hdc->mask & HDC_MASK_DRQ) == 0) {
			return;
		}

		if (hdc->dreq != NULL) {
			hdc->dreq (hdc->dreq_ext, val);
		}
	}
}

static
void hdc_request_command (hdc_t *hdc)
{
	hdc->status |= HDC_STATUS_REQ | HDC_STATUS_BSY | HDC_STATUS_CMD;
	hdc->status &= ~HDC_STATUS_INP;
}

static
void hdc_request_data (hdc_t *hdc, int inp)
{
	hdc->status |= HDC_STATUS_REQ | HDC_STATUS_BSY;
	hdc->status &= ~(HDC_STATUS_INP | HDC_STATUS_CMD);

	if (inp) {
		hdc->status |= HDC_STATUS_INP;
	}

	hdc_set_dreq (hdc, 1);
}

static
void hdc_request_status (hdc_t *hdc)
{
	hdc->status |= HDC_STATUS_REQ | HDC_STATUS_BSY;
	hdc->status |= HDC_STATUS_INP | HDC_STATUS_CMD;

	hdc->delay = 0;
	hdc->cont = NULL;

	hdc_set_dreq (hdc, 0);
	hdc_set_irq (hdc, 1);
}

static
void hdc_next_id (hdc_t *hdc)
{
	hdc_drive_t *d;

	d = &hdc->drv[hdc->id.d & 1];

	hdc->id.s += 1;

	if (hdc->id.s >= d->max_s) {
		hdc->id.s = 0;
		hdc->id.h += 1;

		if (hdc->id.h >= d->max_h) {
			hdc->id.h = 0;
			hdc->id.c += 1;
		}
	}
}

static
disk_t *hdc_get_disk (hdc_t *hdc, unsigned drv)
{
	unsigned d;
	disk_t   *dsk;

	d = hdc->drv[drv & 1].drive;

	if (d == 0xffff) {
		return (NULL);
	}

	dsk = dsks_get_disk (hdc->dsks, d);

	return (dsk);
}

static
void hdc_set_result (hdc_t *hdc, int err, unsigned code)
{
	unsigned d;

	d = hdc->id.d & 1;

	hdc->result = d << 5;

	if (err) {
		hdc->result |= 0x02;
	}

	hdc->drv[d].sense[0] = code;
	hdc->drv[d].sense[1] = (d << 5) | (hdc->id.h & 0x1f);
	hdc->drv[d].sense[2] = (hdc->id.c >> 2) & 0xc0;
	hdc->drv[d].sense[3] = hdc->id.c & 0xff;
}

/* normal command termination */
static
void hdc_cmd_done (hdc_t *hdc)
{
	hdc_request_status (hdc);
}


/* Command 00: test drive ready */

static
void hdc_cmd_tst_rdy (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  test drive ready\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	if (hdc_get_disk (hdc, hdc->id.d) == NULL) {
		hdc_set_result (hdc, 1, 0x20);
		hdc->delay = 256;
	}
	else {
		hdc_set_result (hdc, 0, 0x00);
		hdc->delay = 4096;
	}

	hdc->cont = hdc_cmd_done;
}


/* Command 01: recalibrate */

static
void hdc_cmd_recal (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  recalibrate\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	if (hdc_get_disk (hdc, hdc->id.d) == NULL) {
		hdc_set_result (hdc, 1, 0x20);
		hdc->delay = 512;
	}
	else {
		hdc_set_result (hdc, 0, 0x00);
		hdc->delay = 65536;
	}

	hdc->cont = hdc_cmd_done;
}


/* Command 03: sense */

static
void hdc_cmd_sense_cont (hdc_t *hdc)
{
	unsigned d;

	d = (hdc->cmd[1] >> 5) & 1;

	hdc->buf[0] = hdc->drv[d].sense[0];
	hdc->buf[1] = hdc->drv[d].sense[1];
	hdc->buf[2] = hdc->drv[d].sense[2];
	hdc->buf[3] = hdc->drv[d].sense[3];

	hdc->buf_idx = 0;
	hdc->buf_cnt = 4;

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 0;
	hdc->cont = hdc_cmd_done;

	hdc_request_data (hdc, 1);
}

static
void hdc_cmd_sense (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  sense\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc->delay = 256;
	hdc->cont = hdc_cmd_sense_cont;
}


/* Command 04: format drive */

static
void hdc_cmd_fmtdrv_error (hdc_t *hdc, unsigned code)
{
	fprintf (stderr, "hdc: format error (d=%u)\n", hdc->id.d);

	hdc_set_result (hdc, hdc->id.d, code | 0x80);

	hdc_cmd_done (hdc);
}

static
void hdc_cmd_fmtdrv_cont (hdc_t *hdc)
{
	unsigned      i, d, c, h, cn, hn, sn;
	unsigned char buf[512];
	disk_t        *dsk;

	d = hdc->id.d & 1;
	c = hdc->id.c;
	h = hdc->id.h;

	cn = hdc->drv[d].max_c;
	hn = hdc->drv[d].max_h;
	sn = hdc->drv[d].max_s;

	dsk = hdc_get_disk (hdc, d);

	if (dsk == NULL) {
		hdc_cmd_fmtdrv_error (hdc, 0x04);
		return;
	}

	if (hdc->cmd[5] & 0x40) {
		for (i = 0; i < 512; i++) {
			buf[i] = hdc->buf[i];
		}
	}
	else {
		for (i = 0; i < 512; i++) {
			buf[i] = 0x6c;
		}
	}

#if DEBUG_HDC >= 2
	fprintf (stderr, "HDC: CMD=%02X D=%u    format track (c=%u h=%u n=%u)\n",
		hdc->cmd[0], d, c, h, sn
	);
#endif

	for (i = 0; i < sn; i++) {
		if (dsk_write_chs (dsk, buf, c, h, i + 1, 1)) {
			hdc_cmd_fmtdrv_error (hdc, 0x03);
			return;
		}
	}

	hdc->delay = 8192;
	hdc->cont = hdc_cmd_fmtdrv_cont;

	h += 1;

	if (h >= hn) {
		h = 0;

		c += 1;

		if (c >= cn) {
			hdc->cont = hdc_cmd_done;
		}
	}

	hdc->id.c = c;
	hdc->id.h = h;
}

static
void hdc_cmd_fmtdrv (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;
	hdc->id.c = ((hdc->cmd[2] << 2) & 0x300) | hdc->cmd[3];
	hdc->id.h = hdc->cmd[1] & 0x1f;
	hdc->id.n = hdc->cmd[4] & 0x1f;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  format drive (c=%u h=%u il=%u)\n",
		hdc->cmd[0], hdc->id.d, hdc->id.c, hdc->id.h, hdc->id.n
	);
#endif

	hdc->result = 0;

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_fmtdrv_cont;
}


/* Command 05: verify */

static
void hdc_cmd_chk_trk (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;
	hdc->id.c = ((hdc->cmd[2] << 2) & 0x300) | hdc->cmd[3];
	hdc->id.h = hdc->cmd[1] & 0x1f;
	hdc->id.s = hdc->cmd[2] & 0x3f;
	hdc->id.n = hdc->cmd[4];

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  verify (chs=%u/%u/%u n=%u)\n",
		hdc->cmd[0], hdc->id.d,
		hdc->id.c, hdc->id.h, hdc->id.s + 1, hdc->id.n
	);
#endif

	hdc->result = 0;

	hdc->delay = 65536;
	hdc->cont = hdc_cmd_done;
}


/* Command 06: format track */

static
void hdc_cmd_fmttrk_error (hdc_t *hdc, unsigned code)
{
	fprintf (stderr, "hdc: format track error (d=%u)\n", hdc->id.d);

	hdc_set_result (hdc, hdc->id.d, code | 0x80);

	hdc_cmd_done (hdc);
}

static
void hdc_cmd_fmttrk (hdc_t *hdc)
{
	unsigned      i, d;
	unsigned char buf[512];
	disk_t        *dsk;

	hdc->id.d = (hdc->cmd[1] >> 5) & 1;
	hdc->id.c = ((hdc->cmd[2] << 2) & 0x300) | hdc->cmd[3];
	hdc->id.h = hdc->cmd[1] & 0x1f;
	hdc->id.n = hdc->cmd[4] & 0x1f;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  format track (c=%u h=%u il=%u)\n",
		hdc->cmd[0], hdc->id.d, hdc->id.c, hdc->id.h, hdc->id.n
	);
#endif

	d = hdc->id.d & 1;

	dsk = hdc_get_disk (hdc, d);

	if (dsk == NULL) {
		hdc_cmd_fmttrk_error (hdc, 0x04);
		return;
	}

	if (hdc->cmd[5] & 0x40) {
		for (i = 0; i < 512; i++) {
			buf[i] = hdc->buf[i];
		}
	}
	else {
		for (i = 0; i < 512; i++) {
			buf[i] = 0x6c;
		}
	}

	for (i = 0; i < hdc->drv[d].max_s; i++) {
		if (dsk_write_chs (dsk, buf, hdc->id.c, hdc->id.h, i + 1, 1)) {
			hdc_cmd_fmttrk_error (hdc, 0x03);
			return;
		}
	}

	hdc->result = 0;

	hdc->delay = 65536;
	hdc->cont = hdc_cmd_done;
}


/* Command 08: read */

static void hdc_cmd_read_next (hdc_t *hdc);

static
void hdc_cmd_read_error (hdc_t *hdc, unsigned code)
{
	fprintf (stderr, "hdc: read error (d=%u)\n", hdc->id.d);

	hdc_set_result (hdc, 1, code | 0x80);

	hdc_cmd_done (hdc);
}

static
void hdc_cmd_read_delay (hdc_t *hdc)
{
	hdc_set_dreq (hdc, 0);

	if (hdc->id.n == 0) {
		hdc_cmd_done (hdc);
		return;
	}

	hdc->delay = 2048;
	hdc->cont = hdc_cmd_read_next;
}

static
void hdc_cmd_read_next (hdc_t *hdc)
{
	unsigned d, c, h, s;
	disk_t   *dsk;

	d = hdc->id.d & 1;
	c = hdc->id.c;
	h = hdc->id.h;
	s = hdc->id.s;

	dsk = hdc_get_disk (hdc, d);

	if (dsk == NULL) {
		hdc_cmd_read_error (hdc, 0x04);
		return;
	}

#if DEBUG_HDC >= 2
	fprintf (stderr, "HDC: CMD=%02X D=%u    read sector (chs=%u/%u/%u)\n",
		hdc->cmd[0], d, c, h, s + 1
	);
#endif

	if (dsk_read_chs (dsk, hdc->buf, c, h, s + 1, 1)) {
		hdc_cmd_read_error (hdc, 0x12);
		return;
	}

	hdc->buf_idx = 0;
	hdc->buf_cnt = 512;

	hdc_next_id (hdc);

	hdc->id.n -= 1;

	hdc->delay = 0;
	hdc->cont = hdc_cmd_read_delay;

	hdc_request_data (hdc, 1);
}

static
void hdc_cmd_read (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;
	hdc->id.c = ((hdc->cmd[2] << 2) & 0x300) | hdc->cmd[3];
	hdc->id.h = hdc->cmd[1] & 0x1f;
	hdc->id.s = hdc->cmd[2] & 0x3f;
	hdc->id.n = hdc->cmd[4];

	if (hdc->id.n == 0) {
		hdc->id.n = 256;
	}

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  read (chs=%u/%u/%u n=%u)\n",
		hdc->cmd[0], hdc->id.d,
		hdc->id.c, hdc->id.h, hdc->id.s + 1, hdc->id.n
	);
#endif

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_read_next;
}


/* Command 0A: write */

static void hdc_cmd_write_next (hdc_t *hdc);

static
void hdc_cmd_write_error (hdc_t *hdc, unsigned code)
{
	fprintf (stderr, "hdc: write error (d=%u)\n", hdc->id.d);

	hdc_set_result (hdc, 1, code | 0x80);

	hdc_cmd_done (hdc);
}

static
void hdc_cmd_write_delay (hdc_t *hdc)
{
	hdc->buf_idx = 0;
	hdc->buf_cnt = 512;

	hdc->delay = 0;
	hdc->cont = hdc_cmd_write_next;

	hdc_request_data (hdc, 0);
}

static
void hdc_cmd_write_next (hdc_t *hdc)
{
	unsigned d, c, h, s;
	disk_t   *dsk;

	d = hdc->id.d & 1;
	c = hdc->id.c;
	h = hdc->id.h;
	s = hdc->id.s;

	dsk = hdc_get_disk (hdc, d);

	if (dsk == NULL) {
		hdc_cmd_write_error (hdc, 0x04);
		return;
	}

#if DEBUG_HDC >= 2
	fprintf (stderr, "HDC: CMD=%02X D=%u    write sector (chs=%u/%u/%u)\n",
		hdc->cmd[0], d, c, h, s + 1
	);
#endif

	if (dsk_write_chs (dsk, hdc->buf, c, h, s + 1, 1)) {
		hdc_cmd_write_error (hdc, 0x12);
		return;
	}

	hdc_next_id (hdc);

	hdc->id.n -= 1;

	if (hdc->id.n == 0) {
		hdc_cmd_done (hdc);
		return;
	}

	hdc->delay = 2048;
	hdc->cont = hdc_cmd_write_delay;

	hdc_set_dreq (hdc, 0);
}

static
void hdc_cmd_write (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;
	hdc->id.c = ((hdc->cmd[2] << 2) & 0x300) | hdc->cmd[3];
	hdc->id.h = hdc->cmd[1] & 0x1f;
	hdc->id.s = hdc->cmd[2] & 0x3f;
	hdc->id.n = hdc->cmd[4];

	if (hdc->id.n == 0) {
		hdc->id.n = 256;
	}

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  write (chs=%u/%u/%u n=%u)\n",
		hdc->cmd[0], hdc->id.d,
		hdc->id.c, hdc->id.h, hdc->id.s + 1, hdc->id.n
	);
#endif

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_write_delay;
}


/* Command 0B: seek */

static
void hdc_cmd_seek (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;
	hdc->id.c = ((hdc->cmd[2] << 2) & 0x300) | hdc->cmd[3];
	hdc->id.h = hdc->cmd[1] & 0x1f;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  seek (c=%u h=%u)\n",
		hdc->cmd[0], hdc->id.d, hdc->id.c, hdc->id.h
	);
#endif

	if (hdc_get_disk (hdc, hdc->id.d) == NULL) {
		hdc_set_result (hdc, 1, 0x04);
		hdc->delay = 512;
	}
	else {
		hdc_set_result (hdc, 0, 0x00);
		hdc->delay = 65536;
	}

	hdc->cont = hdc_cmd_done;
}


/* Command 0C: init drive */

static
void hdc_cmd_init_cont2 (hdc_t *hdc)
{
	unsigned      d, c, h, s, rc, wp, ec;
	unsigned long size;

	d = (hdc->cmd[1] >> 5) & 1;
	c = (hdc->buf[0] << 8) | hdc->buf[1];
	h = hdc->buf[2];
	s = hdc->drv[d].max_s;
	rc = (hdc->buf[3] << 8) | hdc->buf[4];
	wp = (hdc->buf[5] << 8) | hdc->buf[6];
	ec = hdc->buf[7];

	size = (unsigned long) c * (unsigned long) h * s;

	pce_log_tag (MSG_INF,
		"HDC:",
		"init drive %u (c=%u h=%u s=%u rc=%u wp=%u ecc=%u size=%lu.%uK)\n",
		d, c, h, s, rc, wp, ec, size / 2, (size & 1) ? 5 : 0
	);

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  init (c=%u h=%u s=%u rc=%u wp=%u ecc=%u)\n",
		hdc->cmd[0], d, c, h, s, rc, wp, ec
	);
#endif

	hdc->drv[d].max_c = c;
	hdc->drv[d].max_h = h;

	hdc_set_result (hdc, 0, 0x00);

	hdc_cmd_done (hdc);
}

static
void hdc_cmd_init_cont1 (hdc_t *hdc)
{
	hdc->delay = 0;
	hdc->cont = hdc_cmd_init_cont2;

	hdc_request_data (hdc, 0);
}

static
void hdc_cmd_init (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

	hdc->buf_idx = 0;
	hdc->buf_cnt = 8;

	hdc->delay = 512;
	hdc->cont = hdc_cmd_init_cont1;
}


/* Command 0E: read buffer */

static
void hdc_cmd_rd_buf (hdc_t *hdc)
{
#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=*  read buffer\n",
		hdc->cmd[0]
	);
#endif

	hdc->buf_idx = 0;
	hdc->buf_cnt = 512;

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 0;
	hdc->cont = hdc_cmd_done;

	hdc_request_data (hdc, 1);
}


/* Command 0F: write buffer */

static
void hdc_cmd_wr_buf_delay (hdc_t *hdc)
{
	hdc->buf_idx = 0;
	hdc->buf_cnt = 512;

	hdc->delay = 0;
	hdc->cont = hdc_cmd_done;

	hdc_set_result (hdc, 0, 0x00);

	hdc_request_data (hdc, 0);
}

static
void hdc_cmd_wr_buf (hdc_t *hdc)
{
#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=*  write buffer\n",
		hdc->cmd[0]
	);
#endif

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_wr_buf_delay;
}


/* Command 11: unknown (Adaptec) */

static
void hdc_cmd_11 (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  cmd_11\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_done;
}


/* Command 15: set mode (Adaptec) */

static
void hdc_cmd_set_mode_cont (hdc_t *hdc)
{
	unsigned      d, c, h, s, rc, wp, ec, sr;
	unsigned long size;

#if DEBUG_HDC >= 1
	hdc_print_hex (stderr, "mode:", hdc->buf, 12);
#endif

	d = hdc->id.d;
	c = buf_get_uint16_be (hdc->buf, 1);
	h = buf_get_uint8 (hdc->buf, 3);
	s = hdc->drv[d].max_s;
	rc = buf_get_uint16_be (hdc->buf, 4);
	wp = buf_get_uint16_be (hdc->buf, 6);
	ec = buf_get_uint8 (hdc->buf, 8);
	sr = buf_get_uint8 (hdc->buf, 10);

	size = (unsigned long) c * (unsigned long) h * s;

	pce_log_tag (MSG_INF,
		"HDC:",
		"set mode drive %u (c=%u h=%u s=%u rc=%u wp=%u ecc=%u sr=%u size=%lu.%uK)\n",
		d, c, h, s, rc, wp, ec, sr, size / 2, (size & 1) ? 5 : 0
	);

	hdc_set_result (hdc, 0, 0x00);

	hdc_cmd_done (hdc);
}

static
void hdc_cmd_set_mode_delay (hdc_t *hdc)
{
	hdc->buf_idx = 0;
	hdc->buf_cnt = 12;

	hdc->delay = 0;
	hdc->cont = hdc_cmd_set_mode_cont;

	hdc_request_data (hdc, 0);
}

static
void hdc_cmd_set_mode (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  set mode\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc->delay = 2048;
	hdc->cont = hdc_cmd_set_mode_delay;

	hdc_set_dreq (hdc, 0);
}


/* Command 1A: mode sense (Adaptec) */

static
void hdc_cmd_mode_sense_cont (hdc_t *hdc)
{
	unsigned c, h, s;
	disk_t   *dsk;

	dsk = dsks_get_disk (hdc->dsks, hdc->drv[hdc->id.d].drive);

	if (dsk == NULL) {
		c = 0;
		h = 0;
		s = hdc->sectors;
	}
	else {
		c = dsk->c;
		h = dsk->h;
		s = dsk->s;
	}

	hdc->drv[hdc->id.d].max_c = c;
	hdc->drv[hdc->id.d].max_h = h;
	hdc->drv[hdc->id.d].max_s = s;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  mode sense (chs=%u/%u/%u ecc=8)\n",
		hdc->cmd[0], hdc->id.d, c, h, s
	);
#endif

	buf_set_uint8 (hdc->buf, 0, 0x02);
	buf_set_uint16_be (hdc->buf, 1, c);
	buf_set_uint8 (hdc->buf, 3, h);
	buf_set_uint16_be (hdc->buf, 4, c);
	buf_set_uint16_be (hdc->buf, 6, c);
	buf_set_uint8 (hdc->buf, 8, 0x08);
	buf_set_uint8 (hdc->buf, 9, 0x00);
	buf_set_uint8 (hdc->buf, 10, 0x03);
	buf_set_uint8 (hdc->buf, 11, 0x01);

	hdc->buf_idx = 0;
	hdc->buf_cnt = 12;

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 0;
	hdc->cont = hdc_cmd_done;

	hdc_request_data (hdc, 1);
}

static
void hdc_cmd_mode_sense (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 2
	fprintf (stderr, "HDC: CMD=%02X D=%u  mode sense\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_mode_sense_cont;
}


/* Command E0: ram diagnostics */

static
void hdc_cmd_ram_diag (hdc_t *hdc)
{
#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=*  ram diagnostics\n",
		hdc->cmd[0]
	);
#endif

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_done;
}


/* Command E4: controller diagnostics */

static
void hdc_cmd_cntlr_diag (hdc_t *hdc)
{
#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=*  controller diagnostics\n",
		hdc->cmd[0]
	);
#endif

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_done;
}


/* Command FB: get drive geometry */

static
void hdc_cmd_getgeo_cont (hdc_t *hdc)
{
	unsigned c, h, s;
	disk_t   *dsk;

	dsk = dsks_get_disk (hdc->dsks, hdc->drv[hdc->id.d].drive);

	if (dsk == NULL) {
		c = 0;
		h = 0;
		s = hdc->sectors;
	}
	else {
		c = dsk->c - 1;
		h = dsk->h - 1;
		s = dsk->s;
	}

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  get drive geometry (c=%u h=%u s=%u)\n",
		hdc->cmd[0], hdc->id.d, c + 1, h + 1, s
	);
#endif

	hdc->drv[hdc->id.d].max_s = s;

	hdc->buf[0] = c & 0xff;
	hdc->buf[1] = ((c >> 2) & 0xc0) | (s & 0x3f);
	hdc->buf[2] = h;
	hdc->buf[3] = 0x00;

	hdc->buf_idx = 0;
	hdc->buf_cnt = 4;

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 0;
	hdc->cont = hdc_cmd_done;

	hdc_request_data (hdc, 1);
}

static
void hdc_cmd_getgeo (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_getgeo_cont;
}


/* Command FC: unknown */

static
void hdc_cmd_fc (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  cmd_fc\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_done;
}


/* Command FD: unknown */

static
void hdc_cmd_fd (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  cmd_fd\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_done;
}


/* Command FE: set config parameters */

static
void hdc_cmd_fe_cont (hdc_t *hdc)
{
#if DEBUG_HDC >= 1
	hdc_print_hex (stderr, "config parameters:", hdc->buf, 64);
#endif

	hdc_cmd_done (hdc);
}

static
void hdc_cmd_fe (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  set config parameters\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc->buf_idx = 0;
	hdc->buf_cnt = 64;

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 0;
	hdc->cont = hdc_cmd_fe_cont;

	hdc_request_data (hdc, 0);
}


/* Command FF: get config parameters */

static
void hdc_cmd_ff_cont (hdc_t *hdc)
{
	unsigned c, h, s;
	disk_t   *dsk;

	dsk = dsks_get_disk (hdc->dsks, hdc->drv[hdc->id.d].drive);

	if (dsk == NULL) {
		c = 0;
		h = 0;
		s = hdc->sectors;
	}
	else {
		c = dsk->c;
		h = dsk->h;
		s = dsk->s;
	}

	hdc->drv[hdc->id.d].max_s = s;

	memcpy (hdc->buf, hdc->config_params, 64);

	buf_set_uint16_le (hdc->buf, 8, c);
	buf_set_uint8 (hdc->buf, 10, h);
	buf_set_uint16_le (hdc->buf, 11, c);
	buf_set_uint16_le (hdc->buf, 13, c);
	buf_set_uint8 (hdc->buf, 15, 5);
	buf_set_uint8 (hdc->buf, 22, s);

	buf_set_uint8 (hdc->buf, 24, 0x80);
	buf_set_uint8 (hdc->buf, 25, 0x00);
	buf_set_uint8 (hdc->buf, 26, 0xff);
	buf_set_uint8 (hdc->buf, 27, 0xff);
	buf_set_uint8 (hdc->buf, 28, 0xff);
	buf_set_uint8 (hdc->buf, 29, 0xff);
	buf_set_uint8 (hdc->buf, 30, 0xff);
	buf_set_uint8 (hdc->buf, 31, 0xff);

	hdc->buf_idx = 0;
	hdc->buf_cnt = 64;

	hdc_set_result (hdc, 0, 0x00);

	hdc->delay = 0;
	hdc->cont = hdc_cmd_done;

	hdc_request_data (hdc, 1);
}

static
void hdc_cmd_ff (hdc_t *hdc)
{
	hdc->id.d = (hdc->cmd[1] >> 5) & 1;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: CMD=%02X D=%u  get config parameters\n",
		hdc->cmd[0], hdc->id.d
	);
#endif

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_ff_cont;
}


/* Command XX: unknown command */

static
void hdc_cmd_unknown (hdc_t *hdc)
{
	fprintf (stderr,
		"HDC: CMD=%02X D=*  unknown command [%02X %02X %02X %02X %02X %02X]\n",
		hdc->cmd[0],
		hdc->cmd[0], hdc->cmd[1], hdc->cmd[2],
		hdc->cmd[3], hdc->cmd[4], hdc->cmd[5]
	);

	hdc_set_result (hdc, 1, 0x20);

	hdc->delay = 4096;
	hdc->cont = hdc_cmd_done;
}


static
unsigned char hdc_get_status (hdc_t *hdc)
{
	return (hdc->status);
}

static
unsigned char hdc_get_config (hdc_t *hdc)
{
#if DEBUG_HDC >= 3
	fprintf (stderr, "hdc: get config: %02X\n", hdc->config);
#endif

	return (hdc->config);
}

unsigned char hdc_read_data (hdc_t *hdc)
{
	unsigned char val;

	if ((hdc->status & HDC_STATUS_REQ) == 0) {
		return (0);
	}

	val = 0;

	if (hdc->status & HDC_STATUS_CMD) {
		val = hdc->result;

		hdc_set_irq (hdc, 0);

#if DEBUG_HDC >= 3
		fprintf (stderr, "hdc: %02X\n", val);
#endif

		hdc->cmd_idx = 0;
		hdc->cmd_cnt = 0;

		hdc->status &= ~(HDC_STATUS_BSY | HDC_STATUS_INP | HDC_STATUS_REQ);
	}
	else {
		if (hdc->buf_idx < hdc->buf_cnt) {
			val = hdc->buf[hdc->buf_idx++];

			if (hdc->buf_idx >= hdc->buf_cnt) {
				hdc_set_dreq (hdc, 0);

				if (hdc->cont != NULL) {
					hdc->cont (hdc);
				}
			}
		}
	}

	return (val);
}


static
void hdc_select (hdc_t *hdc)
{
#if DEBUG_HDC >= 3
	fprintf (stderr, "hdc: select\n");
#endif

	hdc->delay = 0;
	hdc->cont = NULL;

	hdc_set_dreq (hdc, 0);
	hdc_set_irq (hdc, 0);

	hdc->cmd_idx = 0;

	hdc_request_command (hdc);
}

static
void hdc_set_mask (hdc_t *hdc, unsigned char val)
{
	hdc->mask = val;
}

static
void hdc_set_command (hdc_t *hdc, unsigned char val)
{
	if (hdc->cmd_idx == 0) {
		hdc->cmd_cnt = 6;
	}

	hdc->cmd[hdc->cmd_idx++] = val;

	if (hdc->cmd_idx < hdc->cmd_cnt) {
		return;
	}

	hdc->status &= ~HDC_STATUS_REQ;

	switch (hdc->cmd[0]) {
	case 0x00:
		hdc_cmd_tst_rdy (hdc);
		break;

	case 0x01:
		hdc_cmd_recal (hdc);
		break;

	case 0x03:
		hdc_cmd_sense (hdc);
		break;

	case 0x04:
		hdc_cmd_fmtdrv (hdc);
		break;

	case 0x05:
		hdc_cmd_chk_trk (hdc);
		break;

	case 0x06:
		hdc_cmd_fmttrk (hdc);
		break;

	case 0x08:
		hdc_cmd_read (hdc);
		break;

	case 0x0a:
		hdc_cmd_write (hdc);
		break;

	case 0x0b:
		hdc_cmd_seek (hdc);
		break;

	case 0x0c:
		hdc_cmd_init (hdc);
		break;

	case 0x0e:
		hdc_cmd_rd_buf (hdc);
		break;

	case 0x0f:
		hdc_cmd_wr_buf (hdc);
		break;

	case 0x11:
		hdc_cmd_11 (hdc);
		break;

	case 0x15:
		hdc_cmd_set_mode (hdc);
		break;

	case 0x1a:
		hdc_cmd_mode_sense (hdc);
		break;

	case 0xe0:
		hdc_cmd_ram_diag (hdc);
		break;

	case 0xe4:
		hdc_cmd_cntlr_diag (hdc);
		break;

	case 0xfb:
		hdc_cmd_getgeo (hdc);
		break;

	case 0xfc:
		hdc_cmd_fc (hdc);
		break;

	case 0xfd:
		hdc_cmd_fd (hdc);
		break;

	case 0xfe:
		hdc_cmd_fe (hdc);
		break;

	case 0xff:
		hdc_cmd_ff (hdc);
		break;

	default:
		hdc_cmd_unknown (hdc);
		break;
	}
}

void hdc_write_data (hdc_t *hdc, unsigned char val)
{
	if ((hdc->status & HDC_STATUS_REQ) == 0) {
		return;
	}

	if (hdc->status & HDC_STATUS_CMD) {
		hdc_set_command (hdc, val);
		return;
	}

	if (hdc->buf_idx < hdc->buf_cnt) {
		hdc->buf[hdc->buf_idx++] = val;
	}

	if (hdc->buf_idx >= hdc->buf_cnt) {
		hdc_set_dreq (hdc, 0);

		if (hdc->cont != NULL) {
			hdc->cont (hdc);
		}
	}
}

static
unsigned char hdc_get_uint8 (hdc_t *hdc, unsigned long addr)
{
	unsigned char val;

	val = 0;

	switch (addr) {
	case 0:
		val = hdc_read_data (hdc);
		break;

	case 1:
		val = hdc_get_status (hdc);
		break;

	case 2:
		val = hdc_get_config (hdc);
		break;

	default:
		fprintf (stderr, "HDC: unknown read: %04lX\n", addr);
		break;
	}

#if DEBUG_HDC >= 4
	fprintf (stderr, "hdc: get %04lX -> %02X\n", addr, val);
#endif

	return (val);
}

static
unsigned char hdc_get_uint16 (hdc_t *hdc, unsigned long addr)
{
	return (0);
}

static
void hdc_set_uint8 (hdc_t *hdc, unsigned long addr, unsigned char val)
{
#if DEBUG_HDC >= 4
	fprintf (stderr, "hdc: set %04lX <- %02X\n", addr, val);
#endif

	switch (addr) {
	case 0:
		hdc_write_data (hdc, val);
		break;

	case 1:
		hdc_reset (hdc);
		break;

	case 2:
		hdc_select (hdc);
		break;

	case 3:
		hdc_set_mask (hdc, val);
		break;
	}
}

static
void hdc_set_uint16 (hdc_t *hdc, unsigned long addr, unsigned val)
{
}

hdc_t *hdc_new (unsigned long addr)
{
	hdc_t *hdc;

	hdc = malloc (sizeof (hdc_t));

	if (hdc == NULL) {
		return (NULL);
	}

	mem_blk_init (&hdc->blk, addr, 4, 0);

	mem_blk_set_fct (&hdc->blk, hdc,
		hdc_get_uint8, hdc_get_uint16, NULL,
		hdc_set_uint8, hdc_set_uint16, NULL
	);

	hdc->config = 0;

	memset (hdc->config_params, 0, 64);

	hdc->sectors = 17;

	hdc->drv[0].drive = 0xffff;
	hdc->drv[1].drive = 0xffff;

	hdc->dsks = NULL;

	hdc->delay = 0;

	hdc->cont = NULL;

	hdc->irq_val = 0;
	hdc->irq_ext = NULL;
	hdc->irq = NULL;

	hdc->dreq_val = 0;
	hdc->dreq_ext = NULL;
	hdc->dreq = NULL;

	return (hdc);
}

void hdc_del (hdc_t *hdc)
{
	if (hdc != NULL) {
		mem_blk_free (&hdc->blk);
		free (hdc);
	}
}

void hdc_mem_add_io (hdc_t *hdc, memory_t *io)
{
	mem_add_blk (io, &hdc->blk, 0);
}

void hdc_mem_rmv_io (hdc_t *hdc, memory_t *io)
{
	mem_rmv_blk (io, &hdc->blk);
}

void hdc_set_irq_fct (hdc_t *hdc, void *ext, void *fct)
{
	hdc->irq = fct;
	hdc->irq_ext = ext;
}

void hdc_set_dreq_fct (hdc_t *hdc, void *ext, void *fct)
{
	hdc->dreq = fct;
	hdc->dreq_ext = ext;
}

void hdc_set_config (hdc_t *hdc, unsigned val)
{
	hdc->config = val;
}

void hdc_set_sectors (hdc_t *hdc, unsigned val)
{
	hdc->sectors = val;
}

void hdc_set_disks (hdc_t *hdc, disks_t *dsks)
{
	hdc->dsks = dsks;
}

void hdc_set_drive (hdc_t *hdc, unsigned hdcdrv, unsigned drive)
{
	if (hdcdrv < 2) {
		hdc->drv[hdcdrv].drive = drive;
	}
}

unsigned hdc_get_drive (hdc_t *hdc, unsigned hdcdrv)
{
	if (hdcdrv < 2) {
		return (hdc->drv[hdcdrv].drive);
	}

	return (0xffff);
}

void hdc_set_config_id (hdc_t *hdc, const unsigned char *id, unsigned cnt)
{
	unsigned i;

	memset (hdc->config_params, 0, 64);

	if (cnt > 8) {
		cnt = 8;
	}

	for (i = 0; i < cnt; i++) {
		hdc->config_params[i] = id[i];
	}
}

void hdc_reset (hdc_t *hdc)
{
	unsigned i;

#if DEBUG_HDC >= 1
	fprintf (stderr, "HDC: reset\n");
#endif

	hdc->status = 0;
	hdc->mask = 0;

	hdc_set_result (hdc, 0, 0x00);

	hdc->cmd_idx = 0;
	hdc->cmd_cnt = 0;

	hdc->buf_idx = 0;
	hdc->buf_cnt = 0;

	for (i = 0; i < 2; i++) {
		hdc->drv[i].max_c = 306;
		hdc->drv[i].max_h = 4;
		hdc->drv[i].max_s = hdc->sectors;

		hdc->drv[i].sense[0] = 0;
		hdc->drv[i].sense[1] = 0;
		hdc->drv[i].sense[2] = 0;
		hdc->drv[i].sense[3] = 0;
	}

	hdc->delay = 0;
	hdc->cont = NULL;

	hdc_set_irq (hdc, 0);
}

void hdc_clock (hdc_t *hdc, unsigned long cnt)
{
	if (hdc->delay == 0) {
		return;
	}

	if (cnt < hdc->delay) {
		hdc->delay -= cnt;
		return;
	}

	hdc->delay = 0;

	if (hdc->cont != NULL) {
		hdc->cont (hdc);
	}
}
