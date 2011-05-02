/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8272.c                                     *
 * Created:     2005-03-06 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2005-2011 Hampa Hug <hampa@hampa.ch>                     *
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


/*
 * FDC 8272 emulator
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "e8272.h"


#ifndef E8272_DEBUG
#define E8272_DEBUG 0
#endif


#define E8272_CMD_NONE               0
#define E8272_CMD_SPECIFY            1
#define E8272_CMD_SENSE_DRIVE_STATUS 2
#define E8272_CMD_WRITE              3
#define E8272_CMD_READ               4
#define E8272_CMD_READ_TRACK         5
#define E8272_CMD_RECALIBRATE        6
#define E8272_CMD_SENSE_INT_STATUS   7
#define E8272_CMD_READ_ID            8
#define E8272_CMD_FORMAT             9
#define E8272_CMD_SEEK               10

#define E8272_CMD0_MT  0x80
#define E8272_CMD0_MFM 0x40
#define E8272_CMD0_SK  0x20

#define E8272_MSR_D0B 0x01
#define E8272_MSR_D1B 0x02
#define E8272_MSR_D2B 0x04
#define E8272_MSR_D3B 0x08
#define E8272_MSR_CB  0x10
#define E8272_MSR_NDM 0x20
#define E8272_MSR_DIO 0x40
#define E8272_MSR_RQM 0x80

#define E8272_DOR_DSEL   0x01
#define E8272_DOR_RESET  0x04
#define E8272_DOR_DMAEN  0x08
#define E8272_DOR_MOTEN0 0x10
#define E8272_DOR_MOTEN1 0x20

#define E8272_ST0_IC 0xc0	/* interrupt code */
#define E8272_ST0_SE 0x20	/* seek end */

#define E8272_ST1_EN 0x80	/* end of cylinder */
#define E8272_ST1_DE 0x20	/* data error */
#define E8272_ST1_OR 0x10	/* overrun */
#define E8272_ST1_ND 0x04	/* no data */
#define E8272_ST1_NW 0x02	/* not writeable */
#define E8272_ST1_MA 0x01	/* missing address mark */

#define E8272_ST2_DD 0x20	/* data field crc error */
#define E8272_ST2_WC 0x10	/* wrong cylinder */
#define E8272_ST2_BC 0x02	/* bad cylinder (ff) */
#define E8272_ST2_MD 0x01	/* missing data address mark */

#define E8272_ST3_FT  0x80	/* fault signal */
#define E8272_ST3_WP  0x40	/* write protect */
#define E8272_ST3_RDY 0x20	/* ready */
#define E8272_ST3_T0  0x10	/* track 0 */
#define E8272_ST3_TS  0x08	/* two side */
#define E8272_ST3_HD  0x04	/* head address */
#define E8272_ST3_US1 0x02	/* unit select 1 */
#define E8272_ST3_US0 0x02	/* unit select 2 */


#define E8272_RATE 250000
#define E8272_RPS  5


static void e8272_write_cmd (e8272_t *fdc, unsigned char val);


static
void e8272_drive_init (e8272_drive_t *drv, unsigned pd)
{
	drv->d = pd;
	drv->c = 0;
	drv->h = 0;

	drv->ok = 0;
	drv->sct_cnt = 0;
}

void e8272_init (e8272_t *fdc)
{
	unsigned i;

	fdc->dor = E8272_DOR_RESET;
	fdc->msr = E8272_MSR_RQM;

	fdc->st[0] = 0xc0;
	fdc->st[1] = 0x00;
	fdc->st[2] = 0x00;
	fdc->st[3] = 0x00;

	for (i = 0; i < 4; i++) {
		e8272_drive_init (&fdc->drv[i], i);
	}

	fdc->curdrv = &fdc->drv[0];

	fdc->cmd_i = 0;
	fdc->cmd_n = 0;

	fdc->res_i = 0;
	fdc->res_n = 0;

	fdc->buf_i = 0;
	fdc->buf_n = 0;

	fdc->dma = 1;

	fdc->step_rate = 1;

	fdc->accurate = 0;

	fdc->delay_clock = 0;

	fdc->input_clock = 1000000;

	fdc->track_pos = 0;
	fdc->track_clk = 0;
	fdc->track_size = E8272_RATE / E8272_RPS;

	fdc->index_cnt = 0;

	fdc->set_data = NULL;
	fdc->get_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;

	fdc->blk_rd_ext = NULL;
	fdc->blk_rd = NULL;

	fdc->blk_wr_ext = NULL;
	fdc->blk_wr = NULL;

	fdc->blk_fmt_ext = NULL;
	fdc->blk_fmt = NULL;

	fdc->blk_rdid_ext = NULL;
	fdc->blk_rdid = NULL;

	fdc->irq_ext = NULL;
	fdc->irq_val = 0;
	fdc->irq = NULL;

	fdc->dreq_ext = NULL;
	fdc->dreq_val = 0;
	fdc->dreq = NULL;
}

e8272_t *e8272_new (void)
{
	e8272_t *fdc;

	fdc = malloc (sizeof (e8272_t));

	if (fdc == NULL) {
		return (NULL);
	}

	e8272_init (fdc);

	return (fdc);
}

void e8272_free (e8272_t *fdc)
{
}

void e8272_del (e8272_t *fdc)
{
	if (fdc != NULL) {
		e8272_free (fdc);
		free (fdc);
	}
}


void e8272_set_irq_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->irq = fct;
	fdc->irq_ext = ext;
}

void e8272_set_dreq_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->dreq = fct;
	fdc->dreq_ext = ext;
}


void e8272_set_block_read_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->blk_rd_ext = ext;
	fdc->blk_rd = fct;
}

void e8272_set_block_write_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->blk_wr_ext = ext;
	fdc->blk_wr = fct;
}

void e8272_set_block_fmt_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->blk_fmt_ext = ext;
	fdc->blk_fmt = fct;
}

void e8272_set_block_rdid_fct (e8272_t *fdc, void *ext, void *fct)
{
	fdc->blk_rdid_ext = ext;
	fdc->blk_rdid = fct;
}


void e8272_set_input_clock (e8272_t *fdc, unsigned long clk)
{
	fdc->input_clock = clk;
}

void e8272_set_accuracy (e8272_t *fdc, int accurate)
{
	fdc->accurate = (accurate != 0);
}

/*
 * Set the IRQ output
 */
static
void e8272_set_irq (e8272_t *fdc, unsigned char val)
{
	if (fdc->irq_val != val) {
#if E8272_DEBUG >= 3
		fprintf (stderr, "E8272: irq = %d\n", val);
#endif
		fdc->irq_val = val;
		if (fdc->irq != NULL) {
			fdc->irq (fdc->irq_ext, val);
		}
	}
}

/*
 * Set the DREQ output
 */
static
void e8272_set_dreq (e8272_t *fdc, unsigned char val)
{
	if (fdc->dreq_val != val) {
		fdc->dreq_val = val;

		if (fdc->dreq != NULL) {
			fdc->dreq (fdc->dreq_ext, val);
		}
	}
}


static
unsigned e8272_block_read (e8272_t *fdc, void *buf, unsigned *cnt, unsigned d,
	unsigned pc, unsigned ph, unsigned ps, unsigned s)
{
	unsigned r;

	if (fdc->blk_rd == NULL) {
		return (E8272_ERR_NO_ID);
	}

	if (*cnt > 8192) {
		*cnt = 0;
		return (E8272_ERR_OTHER);
	}

	r = fdc->blk_rd (fdc->blk_rd_ext, buf, cnt, d, pc, ph, ps, s);

	return (r);
}

static
unsigned e8272_block_write (e8272_t *fdc, const void *buf, unsigned *cnt, unsigned d,
	unsigned pc, unsigned ph, unsigned ps, unsigned s)
{
	unsigned r;

	if (fdc->blk_wr == NULL) {
		return (E8272_ERR_NO_ID);
	}

	if (*cnt > 8192) {
		*cnt = 0;
		return (E8272_ERR_OTHER);
	}

	r = fdc->blk_wr (fdc->blk_rd_ext, buf, cnt, d, pc, ph, ps, s);

	return (r);
}

static
int e8272_block_format (e8272_t *fdc, unsigned d,
	unsigned pc, unsigned ph, unsigned ps,
	unsigned lc, unsigned lh, unsigned ls, unsigned ln, unsigned fill)
{
	int r;

	if (fdc->blk_fmt == NULL) {
		return (1);
	}

	r = fdc->blk_fmt (fdc->blk_rd_ext, d, pc, ph, ps, lc, lh, ls, ln, fill);

	return (r);
}

static
int e8272_block_rdid (e8272_t *fdc, unsigned d,
	unsigned pc, unsigned ph, unsigned ps,
	unsigned *lc, unsigned *lh, unsigned *ls, unsigned *ln)
{
	int r;

	if (fdc->blk_rdid == NULL) {
		return (1);
	}

	r = fdc->blk_rdid (fdc->blk_rdid_ext, d, pc, ph, ps, lc, lh, ls, ln);

	return (r);
}

/*
 * Read all the IDs on a track into fdc->curdrv->sct.
 */
static
void e8272_read_track (e8272_t *fdc)
{
	unsigned      i;
	unsigned      lc, lh, ls, ln;
	unsigned long ofs, cnt;
	e8272_drive_t *drv;

	drv = fdc->curdrv;

	if (drv->ok) {
		return;
	}

	drv->sct_cnt = 0;

	for (i = 0; i < E8272_MAX_SCT; i++) {
		if (e8272_block_rdid (fdc, drv->d, drv->c, drv->h, i, &lc, &lh, &ls, &ln)) {
			break;
		}

		drv->sct[i].c = lc;
		drv->sct[i].h = lh;
		drv->sct[i].s = ls;
		drv->sct[i].n = ln;

		drv->sct_cnt += 1;
	}

	ofs = 8 * 150;
	cnt = fdc->track_size - ofs;

	for (i = 0; i < drv->sct_cnt; i++) {
		drv->sct[i].ofs = ofs + (2UL * i * cnt) / (2UL * drv->sct_cnt + 1);
	}

	drv->ok = 1;
}

static
void e8272_select_cylinder (e8272_t *fdc, unsigned pd, unsigned pc)
{
	e8272_drive_t *drv;

	drv = &fdc->drv[pd & 3];

	drv->ok = 0;
	drv->c = pc;
	drv->sct_cnt = 0;

	fdc->curdrv = drv;
}

static
void e8272_select_head (e8272_t *fdc, unsigned pd, unsigned ph)
{
	e8272_drive_t *drv;

	drv = &fdc->drv[pd & 3];

	drv->ok = 0;
	drv->h = ph;
	drv->sct_cnt = 0;

	fdc->curdrv = drv;
}

/*
 * Get the last ID before the current track position.
 */
static
unsigned e8272_get_current_id (e8272_t *fdc)
{
	unsigned i;

	if (fdc->curdrv->sct_cnt == 0) {
		return (0);
	}

	for (i = 0; i < fdc->curdrv->sct_cnt; i++) {
		if (fdc->curdrv->sct[i].ofs > fdc->track_pos) {
			if (i == 0) {
				return (fdc->curdrv->sct_cnt - 1);
			}
			else {
				return (i - 1);
			}
		}
	}

	return (fdc->curdrv->sct_cnt - 1);
}

/*
 * Convert bits to input clock.
 */
static
unsigned long e8272_bits_to_clock (e8272_t *fdc, unsigned long bits)
{
	unsigned long clk;

	clk = ((unsigned long long) bits * fdc->input_clock) / E8272_RATE;

	return (clk);
}

static
void e8272_delay_bits (e8272_t *fdc, unsigned long bits, int accurate)
{
	if ((fdc->accurate == 0) && (accurate == 0)) {
		fdc->track_pos += bits;

		while (fdc->track_pos >= fdc->track_size) {
			fdc->track_pos -= fdc->track_size;
			fdc->index_cnt += 1;
		}

		fdc->delay_clock = 0;
	}
	else {
		fdc->delay_clock = e8272_bits_to_clock (fdc, bits);
	}
}

static
void e8272_delay_index (e8272_t *fdc, int accurate)
{
	e8272_delay_bits (fdc, fdc->track_size - fdc->track_pos, accurate);
}

static
void e8272_delay_id (e8272_t *fdc, unsigned id, int accurate)
{
	unsigned long bits, pos;

	if (id >= fdc->curdrv->sct_cnt) {
		e8272_delay_index (fdc, accurate);
		return;
	}

	pos = fdc->curdrv->sct[id].ofs;

	if (pos < fdc->track_pos) {
		bits = pos - fdc->track_pos + fdc->track_size;
	}
	else {
		bits = pos - fdc->track_pos;
	}

	bits += 128;

	e8272_delay_bits (fdc, bits, accurate);
}

static
void e8272_delay_next_id (e8272_t *fdc, int accurate)
{
	unsigned id;

	id = e8272_get_current_id (fdc);

	id += 1;

	if (id >= fdc->curdrv->sct_cnt) {
		id = 0;
	}

	e8272_delay_id (fdc, id, accurate);
}


/*
 * Move to the next sector ID for read and write commands
 */
static
void e8272_next_id (e8272_t *fdc)
{
	if (fdc->cmd[4] != fdc->cmd[6]) {
		fdc->cmd[4] += 1;
		return;
	}

	fdc->cmd[4] = 1;

	if (fdc->cmd[0] & E8272_CMD0_MT) {
		fdc->cmd[1] ^= 0x04;
		fdc->cmd[3] ^= 0x01;

		if ((fdc->cmd[1] & 0x04) == 0) {
			fdc->cmd[2] += 1;
		}
	}
	else {
		fdc->cmd[2] += 1;
	}
}

/*
 * Request a data transfer byte from (rd == 0) / to (rd != 0) the CPU.
 */
static
void e8272_request_data (e8272_t *fdc, int rd)
{
	if (rd) {
		fdc->msr |= E8272_MSR_DIO;
	}
	else {
		fdc->msr &= ~E8272_MSR_DIO;
	}

	if (fdc->dma) {
		fdc->msr &= ~(E8272_MSR_RQM | E8272_MSR_NDM);

		e8272_set_dreq (fdc, 1);
	}
	else {
		fdc->msr |= E8272_MSR_RQM | E8272_MSR_NDM;

		e8272_set_irq (fdc, 1);
	}

	fdc->msr |= E8272_MSR_CB;
}

/*
 * Request a control byte transfer from (rd == 0) / to (rd != 0) the CPU.
 */
static
void e8272_request_control (e8272_t *fdc, int rd)
{
	if (rd) {
		fdc->msr |= (E8272_MSR_DIO | E8272_MSR_CB);
	}
	else {
		fdc->msr &= ~(E8272_MSR_DIO | E8272_MSR_CB);
	}

	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}

	fdc->msr |= E8272_MSR_RQM;
	fdc->msr &= ~E8272_MSR_NDM;
}


/*
 * Ready for next command
 */
static
void cmd_done (e8272_t *fdc)
{
	fdc->set_data = e8272_write_cmd;
	fdc->get_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;

	e8272_request_control (fdc, 0);
}

/*
 * Write a command byte
 */
static
void cmd_set_command (e8272_t *fdc, unsigned char val)
{
	fdc->cmd[fdc->cmd_i++] = val;

	if (fdc->cmd_i < fdc->cmd_n) {
		return;
	}

	fdc->set_data = NULL;

	fdc->msr &= ~E8272_MSR_RQM;
	fdc->msr |= E8272_MSR_CB;

	if (fdc->start_cmd != NULL) {
		fdc->start_cmd (fdc);
	}
}

/*
 * Get a result byte
 */
static
unsigned char cmd_get_result (e8272_t *fdc)
{
	unsigned char val;

	val = fdc->res[fdc->res_i++];

	if (fdc->res_i >= fdc->res_n) {
		cmd_done (fdc);
	}

#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: get result (%02X)\n", val);
#endif

	return (val);
}

/*
 * Enter the result phase
 */
static
void cmd_result (e8272_t *fdc, unsigned cnt)
{
	fdc->res_i = 0;
	fdc->res_n = cnt;

	fdc->set_data = NULL;
	fdc->get_data = cmd_get_result;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;

	e8272_request_control (fdc, 1);
}


/*****************************************************************************
 * read
 *****************************************************************************/

static void cmd_read_clock (e8272_t *fdc, unsigned long cnt);

static
void cmd_read_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  READ TC"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif

	/* head and unit */
	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);

	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->st[1];
	fdc->res[2] = fdc->st[2];
	fdc->res[3] = fdc->cmd[2];
	fdc->res[4] = fdc->cmd[3];
	fdc->res[5] = fdc->cmd[4];
	fdc->res[6] = fdc->cmd[5];

	cmd_result (fdc, 7);

	e8272_set_irq (fdc, 1);
}

static
void cmd_read_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ ERROR"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u eot=%u)\n",
		fdc->cmd[0], fdc->cmd[1] & 3,
		fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif

	fdc->read_error = 1;

	/* abnormal termination */
	fdc->st[0] = (fdc->st[0] & 0x3f) | 0x40;

	if (err & E8272_ERR_CRC_DATA) {
		fdc->st[1] |= E8272_ST1_DE;
		fdc->st[2] |= E8272_ST2_DD;
	}
	else if (err & E8272_ERR_NO_ID) {
		fdc->st[1] |= E8272_ST1_MA | E8272_ST1_ND;
	}
	else if (err & E8272_ERR_NO_DATA) {
		fdc->st[1] |= E8272_ST1_MA | E8272_ST1_ND;
		fdc->st[2] |= E8272_ST2_MD;
	}
	else {
		fdc->st[1] |= E8272_ST1_EN | E8272_ST1_ND;
	}
}

static
unsigned char cmd_read_get_data (e8272_t *fdc)
{
	unsigned char val;

	val = fdc->buf[fdc->buf_i++];

	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}

	if (fdc->buf_i < fdc->buf_n) {
		e8272_request_data (fdc, 1);
		return (val);
	}

	fdc->msr &= ~(E8272_MSR_RQM | E8272_MSR_DIO);

	fdc->get_data = NULL;
	fdc->set_clock = cmd_read_clock;

	if (fdc->read_error) {
		cmd_read_tc (fdc);
	}
	else {
		e8272_next_id (fdc);
	}

	return (val);
}

static
void cmd_read_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	unsigned    c, h, s, n;
	unsigned    cnt1, cnt2;
	unsigned    err;
	e8272_sct_t *sct;

	if (fdc->index_cnt >= 2) {
		cmd_read_error (fdc, E8272_ERR_NO_ID);
		cmd_read_tc (fdc);
		return;
	}

	if (fdc->delay_clock > 0) {
		return;
	}

	id = e8272_get_current_id (fdc);

	e8272_delay_next_id (fdc, 0);

	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}

	c = fdc->cmd[2];
	h = fdc->cmd[3];
	s = fdc->cmd[4];
	n = fdc->cmd[5];

	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}

	sct = &fdc->curdrv->sct[id];

	if ((sct->c != c) || (sct->h != h) || (sct->s != s)) {
		/* wrong cylinder */
		return;
	}

	fdc->index_cnt = 0;

	if (n > 6) {
		cmd_read_error (fdc, E8272_ERR_OTHER);
		cmd_read_tc (fdc);
		return;
	}

	if (n == 0) {
		cnt1 = fdc->cmd[8];
	}
	else {
		cnt1 = 128 << n;
	}

	cnt2 = cnt1;

	err = e8272_block_read (fdc, fdc->buf, &cnt2,
		fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h, id, s
	);

	fdc->buf_i = 0;
	fdc->buf_n = cnt2;

	if (err) {
		cmd_read_error (fdc, err);
	}

	if (fdc->buf_n > 0) {
		e8272_request_data (fdc, 1);

		fdc->set_clock = NULL;
		fdc->get_data = cmd_read_get_data;
	}
	else {
		cmd_read_tc (fdc);
	}
}

static
void cmd_read (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);

#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u"
		"  READ (pc=%u, ph=%u, c=%u, h=%u, s=%u, n=%u, eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif

	fdc->st[0] = 0;
	fdc->st[1] = 0;
	fdc->st[2] = 0;

	fdc->read_error = 0;
	fdc->index_cnt = 0;

	e8272_delay_next_id (fdc, 0);

	fdc->set_clock = cmd_read_clock;
	fdc->set_tc = cmd_read_tc;
}


/*****************************************************************************
 * read track
 *****************************************************************************/

static void cmd_read_track_clock (e8272_t *fdc, unsigned long cnt);

static
void cmd_read_track_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  READ TRACK TC\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif

	/* head and unit */
	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);

	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->st[1];
	fdc->res[2] = fdc->st[2];
	fdc->res[3] = fdc->cmd[2];
	fdc->res[4] = fdc->cmd[3];
	fdc->res[5] = fdc->cmd[4];
	fdc->res[6] = fdc->cmd[5];

	cmd_result (fdc, 7);

	e8272_set_irq (fdc, 1);
}

static
void cmd_read_track_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: read track error (%04X)\n", err);
#endif

	/* abnormal termination */
	fdc->st[0] = (fdc->st[0] & 0x3f) | 0x40;

	if (err & E8272_ERR_NO_ID) {
		fdc->st[1] |= E8272_ST1_MA;
	}
	else {
		fdc->st[1] |= E8272_ST1_EN | E8272_ST1_ND;
	}

	cmd_read_track_tc (fdc);
}

static
unsigned char cmd_read_track_get_data (e8272_t *fdc)
{
	unsigned char val;

	val = fdc->buf[fdc->buf_i++];

	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}

	if (fdc->buf_i < fdc->buf_n) {
		e8272_request_data (fdc, 1);
		return (val);
	}

	fdc->msr &= ~(E8272_MSR_RQM | E8272_MSR_DIO);

	fdc->read_track_cnt += 1;

	if (fdc->read_track_cnt >= fdc->cmd[6]) {
		cmd_read_track_tc (fdc);
		return (val);
	}

	fdc->get_data = NULL;
	fdc->set_clock = cmd_read_track_clock;

	return (val);
}

static
void cmd_read_track_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	unsigned    bcnt, err;
	e8272_sct_t *sct;

	if (fdc->index_cnt >= 2) {
		cmd_read_track_error (fdc, E8272_ERR_NO_ID);
		return;
	}

	if (fdc->delay_clock > 0) {
		return;
	}

	e8272_delay_next_id (fdc, 0);

	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}

	id = e8272_get_current_id (fdc);

	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}

	fdc->index_cnt = 0;

	sct = &fdc->curdrv->sct[id];

	if ((sct->c == fdc->cmd[2]) && (sct->h == fdc->cmd[3])) {
		if (sct->s == fdc->cmd[4]) {
			fdc->st[1] &= ~E8272_ST1_ND;
		}
	}

#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ TRACK CONT"
		" (pc=%u, ph=%u, c=%u, h=%u, s=%u, n=%u eot=%u rs=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6], sct->s
	);
#endif

	bcnt = 8192;

	err = e8272_block_read (fdc, fdc->buf, &bcnt,
		fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		id, sct->s
	);

	fdc->buf_i = 0;
	fdc->buf_n = bcnt;

	err &= ~(E8272_ERR_DATALEN | E8272_ERR_CRC_DATA);

	if (err) {
		cmd_read_track_error (fdc, err);
		return;
	}

	if (fdc->buf_n > 0) {
		e8272_request_data (fdc, 1);

		fdc->set_clock = NULL;
		fdc->get_data = cmd_read_track_get_data;
	}
}

static
void cmd_read_track (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);

#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ TRACK"
		" (pc=%u, ph=%u, c=%u, h=%u s=%u, n=%u, eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif

	fdc->st[0] = 0;
	fdc->st[1] = E8272_ST1_ND;
	fdc->st[2] = 0;

	fdc->read_track_cnt = 0;
	fdc->index_cnt = 0;

	e8272_delay_id (fdc, 0, 0);

	fdc->set_clock = cmd_read_track_clock;
	fdc->set_tc = cmd_read_track_tc;
}


/*****************************************************************************
 * read id
 *****************************************************************************/

static
void cmd_read_id_error (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ ID ERROR (pc=%u ph=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h
	);
#endif

	fdc->st[0] = 0x40 | (fdc->cmd[1] & 7);
	fdc->st[1] |= E8272_ST1_MA | E8272_ST1_ND;

	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->st[1];
	fdc->res[2] = fdc->st[2];
	fdc->res[3] = 0;
	fdc->res[4] = 0;
	fdc->res[5] = 0;
	fdc->res[6] = 0;

	cmd_result (fdc, 7);

	e8272_set_irq (fdc, 1);
}

static
void cmd_read_id_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	e8272_sct_t *sct;

	if (fdc->index_cnt >= 2) {
		cmd_read_id_error (fdc);
		return;
	}

	if (fdc->delay_clock > 0) {
		return;
	}

	e8272_delay_next_id (fdc, 1);

	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}

	id = e8272_get_current_id (fdc);

	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}

	sct = &fdc->curdrv->sct[id];

#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  READ ID (pc=%u ph=%u id=[%02x %02x %02x %02x])\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		sct->c, sct->h, sct->s, sct->n
	);
#endif

	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);

	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->st[1];
	fdc->res[2] = fdc->st[2];
	fdc->res[3] = sct->c;
	fdc->res[4] = sct->h;
	fdc->res[5] = sct->s;
	fdc->res[6] = sct->n;

	cmd_result (fdc, 7);

	e8272_set_irq (fdc, 1);
}

static
void cmd_read_id (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);

#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  READ ID (pc=%u, ph=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h
	);
#endif

	fdc->index_cnt = 0;

	fdc->st[0] = 0;
	fdc->st[1] = 0;
	fdc->st[2] = 0;

	e8272_delay_next_id (fdc, 1);

	fdc->set_clock = cmd_read_id_clock;
}


/*****************************************************************************
 * write
 *****************************************************************************/

static void cmd_write_clock (e8272_t *fdc, unsigned long cnt);

static
void cmd_write_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u WRITE TC\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif

	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);

	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->st[1];
	fdc->res[2] = fdc->st[2];
	fdc->res[3] = fdc->cmd[2];
	fdc->res[4] = fdc->cmd[3];
	fdc->res[5] = fdc->cmd[4];
	fdc->res[6] = fdc->cmd[5];

	cmd_result (fdc, 7);

	e8272_set_irq (fdc, 1);
}

static
void cmd_write_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  WRITE ERROR"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u eot=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5], fdc->cmd[6]
	);
#endif

	fdc->st[0] = (fdc->st[0] & 0x3f) | 0x40;

	if (err & E8272_ERR_WPROT) {
		fdc->st[1] |= E8272_ST1_NW;
	}

	if (err & E8272_ERR_NO_ID) {
		fdc->st[1] |= E8272_ST1_MA;
	}

	cmd_write_tc (fdc);
}

static
void cmd_write_set_data (e8272_t *fdc, unsigned char val)
{
	unsigned err, cnt;

	fdc->buf[fdc->buf_i++] = val;

	if (fdc->buf_i < fdc->buf_n) {
		return;
	}

	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}

	cnt = fdc->buf_n;

	err = e8272_block_write (fdc, fdc->buf, &cnt,
		fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->write_id, fdc->cmd[4]
	);

	if (err) {
		cmd_write_error (fdc, err);
	}
	else {
		e8272_next_id (fdc);

		fdc->set_data = NULL;
		fdc->set_clock = cmd_write_clock;
	}
}

static
void cmd_write_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned    id;
	unsigned    c, h, s, n;
	e8272_sct_t *sct;

	if (fdc->index_cnt >= 2) {
		cmd_write_error (fdc, E8272_ERR_NO_ID);
		return;
	}

	if (fdc->delay_clock > 0) {
		return;
	}

	id = e8272_get_current_id (fdc);

	e8272_delay_next_id (fdc, 0);

	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		return;
	}

	c = fdc->cmd[2];
	h = fdc->cmd[3];
	s = fdc->cmd[4];
	n = fdc->cmd[5];

	if (id >= fdc->curdrv->sct_cnt) {
		return;
	}

	sct = &fdc->curdrv->sct[id];

	if ((sct->c != c) || (sct->h != h) || (sct->s != s)) {
		return;
	}

	fdc->index_cnt = 0;

	if (n > 6) {
		cmd_write_error (fdc, E8272_ERR_OTHER);
		return;
	}

	fdc->buf_i = 0;

	if (n == 0) {
		fdc->buf_n = fdc->cmd[8];
	}
	else {
		fdc->buf_n = 128 << n;
	}

	fdc->write_id = id;

	fdc->set_data = cmd_write_set_data;
	fdc->set_clock = NULL;

	e8272_request_data (fdc, 0);
}

static
void cmd_write (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);
	e8272_read_track (fdc);

#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  WRITE"
		" (pc=%u, ph=%u c=%u, h=%u, s=%u, n=%u)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5]
	);
#endif

	fdc->st[0] = 0;
	fdc->st[1] = 0;
	fdc->st[2] = 0;

	fdc->index_cnt = 0;

	e8272_delay_next_id (fdc, 0);

	fdc->set_tc = cmd_write_tc;
	fdc->set_clock = cmd_write_clock;
}


/*****************************************************************************
 * format
 *****************************************************************************/

static void cmd_format_clock (e8272_t *fdc, unsigned long cnt);

static
void cmd_format_tc (e8272_t *fdc)
{
#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: CMD=%02X D=%u  FORMAT TC\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif

	fdc->st[0] = (fdc->st[0] & ~0x07) | (fdc->cmd[1] & 0x07);

	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->st[1];
	fdc->res[2] = fdc->st[2];
	fdc->res[3] = fdc->curdrv->c;
	fdc->res[4] = (fdc->cmd[1] >> 2) & 0x01;
	fdc->res[5] = fdc->cmd[3] + 1;
	fdc->res[6] = fdc->cmd[2];

	cmd_result (fdc, 7);

	e8272_set_irq (fdc, 1);
}

static
void cmd_format_error (e8272_t *fdc, unsigned err)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: format error\n");
#endif

	fdc->st[0] = 0x40;

	if (err & E8272_ERR_WPROT) {
		fdc->st[1] |= E8272_ST1_NW;
	}

	cmd_format_tc (fdc);
}

static
void cmd_format_set_data (e8272_t *fdc, unsigned char val)
{
	unsigned d, c, h, n;
	unsigned lc, lh, ls, ln;
	unsigned gpl, fill;

	fdc->buf[fdc->buf_i++] = val;

	if (fdc->buf_i < 4) {
		return;
	}

	if (fdc->dma) {
		e8272_set_dreq (fdc, 0);
	}

	d = fdc->curdrv->d;
	c = fdc->curdrv->c;
	h = fdc->curdrv->h;
	n = fdc->cmd[2];
	gpl = fdc->cmd[4];
	fill = fdc->cmd[5];

	lc = fdc->buf[0];
	lh = fdc->buf[1];
	ls = fdc->buf[2];
	ln = fdc->buf[3];

#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  FORMAT SECTOR "
		"(c=%u, h=%u, n=%u, g=%u, f=0x%02x, id=[%02x %02x %02x %02x])\n",
		fdc->cmd[0], d, c, h, n, gpl, fill, lc, lh, ls, ln
	);
#else
	(void) n;
#endif

	fdc->curdrv->ok = 0;

	if (e8272_block_format (fdc, d, c, h, fdc->format_cnt, lc, lh, ls, ln, fill)) {
		cmd_format_error (fdc, E8272_ERR_WPROT);
		return;
	}

	fdc->res[3] = lc;
	fdc->res[4] = lh;
	fdc->res[5] = ls + 1;
	fdc->res[6] = ln;

	fdc->format_cnt += 1;

	if (fdc->format_cnt >= fdc->cmd[3]) {
		e8272_delay_index (fdc, 0);
	}
	else {
		e8272_delay_bits (fdc, 8UL * (62 + (128 << ln) + gpl), 0);
	}

	fdc->set_data = NULL;
	fdc->set_clock = cmd_format_clock;
}

static
void cmd_format_clock (e8272_t *fdc, unsigned long cnt)
{
	if (fdc->delay_clock > 0) {
		return;
	}

	if (fdc->format_cnt >= fdc->cmd[3]) {
		cmd_format_tc (fdc);
		return;
	}

	fdc->buf_i = 0;
	fdc->buf_n = 4;

	fdc->set_data = cmd_format_set_data;
	fdc->set_clock = NULL;

	e8272_request_data (fdc, 0);
}

static
void cmd_format (e8272_t *fdc)
{
	e8272_select_head (fdc, fdc->cmd[1] & 3, (fdc->cmd[1] >> 2) & 1);

#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=%u  FORMAT (pc=%u, ph=%u, n=%u sc=%u gpl=%u d=%02x)\n",
		fdc->cmd[0], fdc->curdrv->d, fdc->curdrv->c, fdc->curdrv->h,
		fdc->cmd[2], fdc->cmd[3], fdc->cmd[4], fdc->cmd[5]
	);
#endif

	fdc->st[0] = 0;
	fdc->st[1] = 0;
	fdc->st[2] = 0;

	if ((fdc->cmd[0] & E8272_CMD0_MFM) == 0) {
		cmd_format_error (fdc, 0);
		return;
	}

	fdc->format_cnt = 0;

	e8272_delay_index (fdc, 0);

	fdc->set_tc = cmd_format_tc;
	fdc->set_clock = cmd_format_clock;
}


/*****************************************************************************
 * recalibrate
 *****************************************************************************/

static
void cmd_recalibrate_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned drv;

	if (fdc->delay_clock > 0) {
		return;
	}

	drv = fdc->cmd[1] & 0x03;

	fdc->msr &= ~(0x01 << drv);

	fdc->st[0] = (fdc->cmd[1] & 0x07) | E8272_ST0_SE;

	e8272_set_irq (fdc, 1);

	cmd_done (fdc);
}

static
void cmd_recalibrate (e8272_t *fdc)
{
	unsigned pd, steps;

	pd = fdc->cmd[1] & 0x03;

#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u  RECALIBRATE\n",
		fdc->cmd[0], pd
	);
#endif

	steps = fdc->drv[pd].c;

	e8272_select_cylinder (fdc, pd, 0);

	fdc->msr |= 0x01 << pd;

	if (fdc->accurate) {
		fdc->delay_clock = steps * ((fdc->step_rate * fdc->input_clock) / 1000);
	}
	else {
		fdc->delay_clock = 0;
	}

	fdc->set_clock = cmd_recalibrate_clock;
}


/*****************************************************************************
 * seek
 *****************************************************************************/
static
void cmd_seek_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned drv;

	if (fdc->delay_clock > 0) {
		return;
	}

	drv = fdc->cmd[1] & 0x03;

	fdc->msr &= ~(0x01 << drv);

	fdc->st[0] = (fdc->cmd[1] & 0x07) | E8272_ST0_SE;

	e8272_set_irq (fdc, 1);

	cmd_done (fdc);
}

static
void cmd_seek (e8272_t *fdc)
{
	unsigned pd, pc, steps;

	pd = fdc->cmd[1] & 3;
	pc = fdc->cmd[2];

#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u  SEEK (pc=%u)\n",
		fdc->cmd[0], pd, pc
	);
#endif

	if (fdc->drv[pd].c < pc) {
		steps = pc - fdc->drv[pd].c;
	}
	else {
		steps = fdc->drv[pd].c - pc;
	}

	e8272_select_cylinder (fdc, pd, pc);

	fdc->msr |= 0x01 << pd;

	if (fdc->accurate) {
		fdc->delay_clock = steps * ((fdc->step_rate * fdc->input_clock) / 1000);
	}
	else {
		fdc->delay_clock = 0;
	}

	fdc->set_clock = cmd_seek_clock;
}


/*****************************************************************************
 * sense interrupt status
 *****************************************************************************/

static
void cmd_sense_int_status_clock (e8272_t *fdc, unsigned long cnt)
{
	if (fdc->delay_clock > 0) {
		return;
	}

	fdc->res[0] = fdc->st[0];
	fdc->res[1] = fdc->curdrv->c;

	/* reset interrupt condition */
	fdc->st[0] &= 0x3f;

	fdc->st[0] &= ~(E8272_ST0_SE);

	e8272_set_irq (fdc, 0);

	cmd_result (fdc, 2);
}

static
void cmd_sense_int_status (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=*  SENSE INTERRUPT STATUS\n",
		fdc->cmd[0]
	);
#endif

	fdc->delay_clock = fdc->accurate ? (fdc->input_clock / 10000) : 0;

	fdc->set_clock = cmd_sense_int_status_clock;
}


/*****************************************************************************
 * sense drive status
 *****************************************************************************/

static
void cmd_sense_drive_status_clock (e8272_t *fdc, unsigned long cnt)
{
	unsigned d;

	if (fdc->delay_clock > 0) {
		return;
	}

	d = fdc->cmd[1] & 0x03;

	fdc->st[3] &= ~E8272_ST3_T0;
	fdc->st[3] |= E8272_ST3_RDY | E8272_ST3_TS;

	if (fdc->drv[d].c == 0) {
		fdc->st[3] |= E8272_ST3_T0;
	}

	fdc->res[0] = fdc->st[3];

	cmd_result (fdc, 1);
}

static
void cmd_sense_drive_status (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: CMD=%02X D=%u  SENSE DRIVE STATUS\n",
		fdc->cmd[0], fdc->cmd[1] & 3
	);
#endif

	fdc->delay_clock = fdc->accurate ? (fdc->input_clock / 10000) : 0;

	fdc->set_clock = cmd_sense_drive_status_clock;
}


/*****************************************************************************
 * specify
 *****************************************************************************/

static
void cmd_specify (e8272_t *fdc)
{
	unsigned char srt, hut, hlt, nd;

	srt = 16 - ((fdc->cmd[1] >> 4) & 0x0f);
	hut = 16 * (fdc->cmd[1] & 0x0f);
	hlt = 2 * ((fdc->cmd[2] >> 1) & 0x7f);
	nd = (fdc->cmd[2] & 0x01);

#if E8272_DEBUG >= 1
	fprintf (stderr,
		"E8272: CMD=%02X D=*  SPECIFY (srt=%ums, hut=%ums, hlt=%ums, dma=%d)\n",
		fdc->cmd[0], srt, hut, hlt, nd == 0
	);
#else
	(void) hlt;
	(void) hut;
#endif

	fdc->dma = (nd == 0);
	fdc->step_rate = srt;

	cmd_done (fdc);
}


/*****************************************************************************
 * invalid
 *****************************************************************************/

static
void cmd_invalid (e8272_t *fdc)
{
#if E8272_DEBUG >= 0
	fprintf (stderr, "E8272: CMD=%02X D=? INVALID\n", fdc->cmd[0]);
#endif

	fdc->res[0] = 0x80;

	cmd_result (fdc, 1);
}


static struct {
	unsigned char mask;
	unsigned char val;
	unsigned      cnt;
	void          (*start_cmd) (e8272_t *fdc);
} cmd_tab[] = {
	{ 0x1f, 0x06, 9, cmd_read },
	{ 0x1f, 0x02, 9, cmd_read_track },
	{ 0xbf, 0x0a, 2, cmd_read_id },
	{ 0x3f, 0x05, 9, cmd_write },
	{ 0xbf, 0x0d, 6, cmd_format },
	{ 0xff, 0x07, 2, cmd_recalibrate },
	{ 0xff, 0x0f, 3, cmd_seek },
	{ 0xff, 0x08, 1, cmd_sense_int_status },
	{ 0xff, 0x04, 2, cmd_sense_drive_status },
	{ 0xff, 0x03, 3, cmd_specify },
	{ 0x00, 0x00, 1, cmd_invalid }
};


/*
 * Write the first command byte.
 */
static
void e8272_write_cmd (e8272_t *fdc, unsigned char val)
{
	unsigned i, n;

	n = sizeof (cmd_tab) / sizeof (cmd_tab[0]);

	fdc->get_data = NULL;
	fdc->set_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;
	fdc->start_cmd = NULL;

	for (i = 0; i < n; i++) {
		if ((val & cmd_tab[i].mask) == cmd_tab[i].val) {
			fdc->cmd_i = 0;
			fdc->cmd_n = cmd_tab[i].cnt;
			fdc->set_data = cmd_set_command;
			fdc->start_cmd = cmd_tab[i].start_cmd;

			fdc->set_data (fdc, val);

			return;
		}
	}
}


void e8272_reset (e8272_t *fdc)
{
#if E8272_DEBUG >= 1
	fprintf (stderr, "E8272: reset\n");
#endif

	fdc->msr = E8272_MSR_RQM;

	fdc->st[0] = 0xc0;
	fdc->st[1] = 0x00;
	fdc->st[2] = 0x00;
	fdc->st[3] = 0x00;

	fdc->cmd_i = 0;
	fdc->cmd_n = 0;

	fdc->res_i = 0;
	fdc->res_n = 0;

	fdc->buf_i = 0;
	fdc->buf_n = 0;

	fdc->delay_clock = 0;

	fdc->set_data = e8272_write_cmd;
	fdc->get_data = NULL;
	fdc->set_tc = NULL;
	fdc->set_clock = NULL;

	e8272_set_irq (fdc, 0);
	e8272_set_dreq (fdc, 0);
}

static
void e8272_write_dor (e8272_t *fdc, unsigned char val)
{
	if (((fdc->dor) ^ val) & E8272_DOR_RESET) {
		if (val & E8272_DOR_RESET) {
			e8272_set_irq (fdc, 1);
		}
		else {
			e8272_reset (fdc);
		}
	}

	fdc->dor = val;
}

void e8272_write_data (e8272_t *fdc, unsigned char val)
{
	e8272_set_irq (fdc, 0);

	if ((fdc->msr & E8272_MSR_RQM) == 0) {
		if (fdc->dreq_val == 0) {
			return;
		}
	}

	if (fdc->msr & E8272_MSR_DIO) {
		return;
	}

	if (fdc->set_data != NULL) {
		fdc->set_data (fdc, val);
	}
}

static
unsigned char e8272_read_dor (e8272_t *fdc)
{
	return (fdc->dor);
}

static
unsigned char e8272_read_msr (e8272_t *fdc)
{
	return (fdc->msr);
}

unsigned char e8272_read_data (e8272_t *fdc)
{
	unsigned char val;

	e8272_set_irq (fdc, 0);

	val = 0;

	if (fdc->get_data != NULL) {
		val = fdc->get_data (fdc);
	}

#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: read data: %02X\n", val);
#endif

	return (val);

	return (0);
}

unsigned char e8272_get_uint8 (e8272_t *fdc, unsigned long addr)
{
	unsigned char ret;

	switch (addr) {
	case 0x02:
		ret = e8272_read_dor (fdc);
		break;

	case 0x04:
		ret = e8272_read_msr (fdc);
		break;

	case 0x05:
		ret = e8272_read_data (fdc);
		break;

	default:
		ret = 0xff;
#if E8272_DEBUG >= 1
		fprintf (stderr, "E8272: get %04lx -> %02x\n", addr, ret);
#endif
		break;
	}

#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: get %04lx -> %02x\n", addr, ret);
#endif

	return (ret);
}

void e8272_set_uint8 (e8272_t *fdc, unsigned long addr, unsigned char val)
{
#if E8272_DEBUG >= 3
	fprintf (stderr, "E8272: set %04lx <- %02x\n", addr, val);
#endif

	switch (addr) {
	case 0x02:
		e8272_write_dor (fdc, val);
		break;

	case 0x05:
		e8272_write_data (fdc, val);
		break;

	default:
#if E8272_DEBUG >= 1
		fprintf (stderr, "E8272: set %04lx <- %02x\n", addr, val);
#endif
		break;
	}
}

void e8272_set_tc (e8272_t *fdc, unsigned char val)
{
	if (val == 0) {
		return;
	}

#if E8272_DEBUG >= 2
	fprintf (stderr, "E8272: TC\n");
#endif

	if (fdc->set_tc != NULL) {
		fdc->set_tc (fdc);
	}
}

void e8272_clock (e8272_t *fdc, unsigned long n)
{
	fdc->track_clk += E8272_RATE * n;

	fdc->track_pos += fdc->track_clk / fdc->input_clock;
	fdc->track_clk %= fdc->input_clock;

	if (fdc->track_pos >= fdc->track_size) {
		fdc->track_pos -= fdc->track_size;
		fdc->index_cnt += 1;
	}

	if (n < fdc->delay_clock) {
		fdc->delay_clock -= n;
	}
	else {
		fdc->delay_clock = 0;
	}

	if (fdc->set_clock != NULL) {
		fdc->set_clock (fdc, n);
	}
}
