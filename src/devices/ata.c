/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/ata.c                                            *
 * Created:     2004-12-03 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2004-2018 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ata.h"


#ifndef DEBUG_ATA
#define DEBUG_ATA 0
#endif


#define ATA_REG_COMMAND       0x07
#define ATA_REG_STATUS        0x07
#define ATA_REG_DEVICE_HEAD   0x06
#define ATA_REG_CYLINDER_HIGH 0x05
#define ATA_REG_CYLINDER_LOW  0x04
#define ATA_REG_SECTOR_NUMBER 0x03
#define ATA_REG_SECTOR_COUNT  0x02
#define ATA_REG_ERROR         0x01
#define ATA_REG_FEATURES      0x01
#define ATA_REG_DATA          0x00

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DF   0x20
#define ATA_STATUS_DSC  0x10
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_CORR 0x04
#define ATA_STATUS_IDX  0x02
#define ATA_STATUS_ERR  0x01

#define ATA_ERROR_UNC   0x40
#define ATA_ERROR_MC    0x20
#define ATA_ERROR_IDNF  0x10
#define ATA_ERROR_MCR   0x08
#define ATA_ERROR_ABRT  0x04
#define ATA_ERROR_TK0NF 0x02
#define ATA_ERROR_AMNF  0x01

#define ATA_BUF_MODE_NONE  0
#define ATA_BUF_MODE_READ  1
#define ATA_BUF_MODE_WRITE 2

#define ATA_CMD_RECALIBRATE        0x10
#define ATA_CMD_READ               0x20
#define ATA_CMD_READ_RETRY         0x21
#define ATA_CMD_WRITE              0x30
#define ATA_CMD_WRITE_RETRY        0x31
#define ATA_CMD_DIAGNOSTIC         0x90
#define ATA_CMD_SET_GEOMETRY       0x91
#define ATA_CMD_STANDBY_IMMEDIATE1 0x94
#define ATA_CMD_READ_MULTIPLE      0xc4
#define ATA_CMD_WRITE_MULTIPLE     0xc5
#define ATA_CMD_SET_MULTIPLE_MODE  0xc6
#define ATA_CMD_STANDBY_IMMEDIATE2 0xe0
#define ATA_CMD_FLUSH_CACHE        0xe7
#define ATA_CMD_IDENTIFY           0xec


unsigned char ata_ctl_get_uint8 (ata_chn_t *ata, unsigned long addr);
unsigned short ata_ctl_get_uint16 (ata_chn_t *ata, unsigned long addr);
unsigned long ata_ctl_get_uint32 (ata_chn_t *ata, unsigned long addr);

void ata_ctl_set_uint8 (ata_chn_t *ata, unsigned long addr, unsigned char val);
void ata_ctl_set_uint16 (ata_chn_t *ata, unsigned long addr, unsigned short val);
void ata_ctl_set_uint32 (ata_chn_t *ata, unsigned long addr, unsigned long val);


void ata_dev_init (ata_dev_t *dev)
{
	static unsigned serial = 0;

	dev->chn = NULL;

	dev->blk = NULL;

	dev->reg_cmd = 0;
	dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
	dev->reg_features = 0;
	dev->reg_error = 0;
	dev->reg_cyl_lo = 0;
	dev->reg_cyl_hi = 0;
	dev->reg_head = 0;
	dev->reg_sec = 0;
	dev->reg_sec_cnt = 0;
	dev->reg_dev_ctl = 0;

	dev->c = 0;
	dev->h = 0;
	dev->s = 0;

	dev->default_c = 0;
	dev->default_h = 0;
	dev->default_s = 0;

	dev->multi_block_max = 0;
	dev->multi_block_size = 0;

	dev->buf_i = 0;
	dev->buf_n = 0;
	dev->buf_m = 0;
	dev->buf_mult_i = 0;
	dev->buf_mult_n = 0;
	dev->buf_mode = ATA_BUF_MODE_NONE;
	dev->callback = NULL;

	strcpy (dev->model, "PCEDISK");
	strcpy (dev->firmware, "0");
	sprintf (dev->serial, "PD%u", serial);
	serial += 1;
}

void ata_dev_free (ata_dev_t *dev)
{
}


void ata_init (ata_chn_t *ata, unsigned long addr1, unsigned long addr2)
{
	mem_blk_init (&ata->reg_cmd, addr1, 8, 0);
	ata->reg_cmd.ext = ata;
	ata->reg_cmd.get_uint8 = (mem_get_uint8_f) ata_cmd_get_uint8;
	ata->reg_cmd.get_uint16 = (mem_get_uint16_f) ata_cmd_get_uint16;
	ata->reg_cmd.get_uint32 = (mem_get_uint32_f) ata_cmd_get_uint32;
	ata->reg_cmd.set_uint8 = (mem_set_uint8_f) ata_cmd_set_uint8;
	ata->reg_cmd.set_uint16 = (mem_set_uint16_f) ata_cmd_set_uint16;
	ata->reg_cmd.set_uint32 = (mem_set_uint32_f) ata_cmd_set_uint32;

	mem_blk_init (&ata->reg_ctl, addr2, 4, 0);
	ata->reg_ctl.ext = ata;
	ata->reg_ctl.get_uint8 = (mem_get_uint8_f) ata_ctl_get_uint8;
	ata->reg_ctl.get_uint16 = (mem_get_uint16_f) ata_ctl_get_uint16;
	ata->reg_ctl.get_uint32 = (mem_get_uint32_f) ata_ctl_get_uint32;
	ata->reg_ctl.set_uint8 = (mem_set_uint8_f) ata_ctl_set_uint8;
	ata->reg_ctl.set_uint16 = (mem_set_uint16_f) ata_ctl_set_uint16;
	ata->reg_ctl.set_uint32 = (mem_set_uint32_f) ata_ctl_set_uint32;

	ata->irq_val = 0;
	ata->irq_ext = 0;
	ata->irq = NULL;

	ata_dev_init (&ata->dev[0]);
	ata_dev_init (&ata->dev[1]);

	ata->dev[0].chn = ata;
	ata->dev[1].chn = ata;

	ata->sel = &ata->dev[0];
}

void ata_free (ata_chn_t *ata)
{
	ata_dev_free (&ata->dev[1]);
	ata_dev_free (&ata->dev[0]);

	mem_blk_free (&ata->reg_ctl);
	mem_blk_free (&ata->reg_cmd);
}

ata_chn_t *ata_new (unsigned long addr1, unsigned long addr2)
{
	ata_chn_t *ata;

	ata = malloc (sizeof (ata_chn_t));
	if (ata == NULL) {
		return (NULL);
	}

	ata_init (ata, addr1, addr2);

	return (ata);
}

void ata_del (ata_chn_t *ata)
{
	if (ata != NULL) {
		ata_free (ata);
		free (ata);
	}
}

void ata_set_irq_f (ata_chn_t *ata, void *irq, void *ext)
{
	ata->irq = irq;
	ata->irq_ext = ext;
}

void ata_set_block (ata_chn_t *ata, disk_t *blk, unsigned devi)
{
	ata_dev_t *dev;

	if (devi >= 2) {
		return;
	}

	dev = &ata->dev[devi];

	dev->blk = blk;

	dev->default_c = blk->c;
	dev->default_h = blk->h;
	dev->default_s = blk->s;

	if (dev->default_h > 16) {
		dev->default_s = 63;
		dev->default_h = 16;
		dev->default_c = dsk_get_block_cnt (blk) / (16UL * 63UL);
	}

	dev->c = dev->default_c;
	dev->h = dev->default_h;
	dev->s = dev->default_s;
}

void ata_set_model (ata_chn_t *ata, unsigned devi, const char *name)
{
	char *dst;

	if (devi < 2) {
		dst = ata->dev[devi].model;
		strncpy (dst, name, 64);
		dst[63] = 0;
	}
}

void ata_set_multi_mode (ata_chn_t *ata, unsigned devi, unsigned max)
{
	if (max > 255) {
		max = 255;
	}

	if (devi < 2) {
		ata->dev[devi].multi_block_max = max;
	}
}

static
void ata_set_irq (ata_chn_t *ata, unsigned char val)
{
	if (ata->sel->reg_dev_ctl & 0x02) {
		return;
	}

	val = (val != 0);

	if (val != ata->irq_val) {
#if DEBUG_ATA >= 3
		fprintf (stderr, "ata: irq = %d\n", val);
#endif

		ata->irq_val = val;

		if (ata->irq != NULL) {
			ata->irq (ata->irq_ext, val);
		}
	}
}

static inline
unsigned short ata_get_uint16_le (void *buf, unsigned i)
{
	unsigned short val;
	unsigned char  *tmp = (unsigned char *) buf + i;

	val = tmp[1];
	val = (val << 8) | tmp[0];

	return (val);
}

static inline
void ata_set_uint16_le (void *buf, unsigned i, unsigned short val)
{
	unsigned char *tmp = (unsigned char *) buf + i;

	tmp[0] = val & 0xff;
	tmp[1] = (val >> 8) & 0xff;
}

static
void ata_set_string (void *buf, unsigned i, const char *str, unsigned cnt)
{
	unsigned char *tmp = (unsigned char *) buf + i;
	unsigned char c1, c2;

	while (cnt >= 2) {
		c1 = (*str == 0) ? ' ' : *(str++);
		c2 = (*str == 0) ? ' ' : *(str++);

		tmp[0] = c2;
		tmp[1] = c1;

		tmp += 2;
		cnt -= 2;
	}

	if (cnt > 0) {
		c1 = (*str == 0) ? ' ' : *(str++);
		tmp[1] = c1;
	}
}


/* Get the current sector address as LBA */
static
int ata_get_lba (ata_dev_t *dev, uint32_t *lba)
{
	uint32_t c, h, s;

	if (dev->reg_head & 0x40) {
		*lba = dev->reg_head & 0x0f;
		*lba = (*lba << 8) | dev->reg_cyl_hi;
		*lba = (*lba << 8) | dev->reg_cyl_lo;
		*lba = (*lba << 8) | dev->reg_sec;
	}
	else {
		c = (dev->reg_cyl_hi << 8) | dev->reg_cyl_lo;
		h = dev->reg_head & 0x0f;
		s = dev->reg_sec;

		if ((c >= dev->c) || (h >= dev->h) || (s == 0) || (s > dev->s)) {
			return (1);
		}

		*lba = (c * dev->h + h) * dev->s + s - 1;
	}

	return (0);
}


static
void ata_cmd_ok (ata_dev_t *dev)
{
	dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
	ata_set_irq (dev->chn, 1);
}

static
void ata_cmd_abort (ata_dev_t *dev)
{
	dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_ERR;
	dev->reg_error = ATA_ERROR_ABRT;
	ata_set_irq (dev->chn, 1);
}

static
void ata_cmd_recalibrate (ata_dev_t *dev)
{
#if DEBUG_ATA >= 1
		fprintf (stderr, "ata: RECALIBRATE\n");
#endif

	dev->reg_cyl_lo = 0;
	dev->reg_cyl_hi = 0;
	dev->reg_head &= 0xf0;
	dev->reg_sec = (dev->reg_head & 0x40) ? 0 : 1;
	ata_cmd_ok (dev);
}

static
void ata_buf_reset (ata_dev_t *dev)
{
	dev->buf_i = 0;
	dev->buf_n = 0;
	dev->buf_m = 0;

	dev->buf_blk_i = 0;
	dev->buf_blk_n = 0;

	dev->buf_mult_i = 0;
	dev->buf_mult_n = 0;

	dev->buf_mode = ATA_BUF_MODE_NONE;
}

static
void ata_cmd_read_cb (ata_dev_t *dev)
{
	unsigned cnt;

	/* move to next sector in buffer */
	dev->buf_i = dev->buf_n;
	dev->buf_n += 512;

	if (dev->buf_n > dev->buf_m) {
		if (dev->buf_blk_n == 0) {
			/* all done */
			ata_buf_reset (dev);

			dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
			dev->callback = NULL;
			return;
		}

		cnt = ATA_BUF_MAX / 512;
		if (cnt > dev->buf_blk_n) {
			cnt = dev->buf_blk_n;
		}

		while (dsk_read_lba (dev->blk, dev->buf, dev->buf_blk_i, cnt)) {
			if (cnt == 1) {
				ata_cmd_abort (dev);
				return;
			}
			cnt = 1;
		}

		dev->buf_m = 512 * cnt;

		dev->buf_blk_i += cnt;
		dev->buf_blk_n -= cnt;

		dev->buf_i = 0;
		dev->buf_n = 512;
	}

	dev->buf_mode = ATA_BUF_MODE_READ;
	dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DRQ | ATA_STATUS_DSC;

	dev->buf_mult_i += 1;

	if (dev->buf_mult_i >= dev->buf_mult_n) {
		dev->buf_mult_i = 0;
		ata_set_irq (dev->chn, 1);
	}
}

static
void ata_cmd_read (ata_dev_t *dev)
{
	uint32_t idx;

	if (ata_get_lba (dev, &idx)) {
		ata_cmd_abort (dev);
		return;
	}

#if DEBUG_ATA >= 2
		fprintf (stderr, "ata: READ (%lu, %u)\n",
			(unsigned long) idx, dev->reg_sec_cnt
		);
#endif

	ata_buf_reset (dev);

	dev->buf_blk_i = idx;
	dev->buf_blk_n = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;
	dev->callback = ata_cmd_read_cb;

	ata_cmd_read_cb (dev);
}

static
void ata_cmd_read_multiple (ata_dev_t *dev)
{
	uint32_t idx;

	if (dev->multi_block_size == 0) {
		ata_cmd_abort (dev);
		return;
	}

	if (ata_get_lba (dev, &idx)) {
		ata_cmd_abort (dev);
		return;
	}

#if DEBUG_ATA >= 2
		fprintf (stderr, "ata: READ MULTIPLE (%lu, %u)\n",
			(unsigned long) idx, dev->reg_sec_cnt
		);
#endif

	ata_buf_reset (dev);

	dev->buf_blk_i = idx;
	dev->buf_blk_n = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;
	dev->buf_mult_n = dev->multi_block_size;
	dev->buf_mult_i = dev->buf_mult_n;
	dev->callback = ata_cmd_read_cb;

	ata_cmd_read_cb (dev);
}

static
void ata_cmd_write_cb (ata_dev_t *dev)
{
	unsigned cnt;

	dev->buf_i = dev->buf_n;
	dev->buf_n += 512;

	if (dev->buf_n > dev->buf_m) {
		cnt = dev->buf_i / 512;

		if (dsk_write_lba (dev->blk, dev->buf, dev->buf_blk_i, cnt)) {
			ata_cmd_abort (dev);
			return;
		}

		dev->buf_blk_i += cnt;
		dev->buf_blk_n -= cnt;

		if (dev->buf_blk_n == 0) {
			ata_buf_reset (dev);
			dev->callback = NULL;
			dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
			ata_set_irq (dev->chn, 1);
			return;
		}
		else {
			cnt = ATA_BUF_MAX / 512;
			if (cnt > dev->buf_blk_n) {
				cnt = dev->buf_blk_n;
			}
			dev->buf_i = 0;
			dev->buf_n = 512;
			dev->buf_m = 512 * cnt;
			dev->buf_mode = ATA_BUF_MODE_WRITE;
			dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DRQ | ATA_STATUS_DSC;
		}
	}

	dev->buf_mult_i += 1;

	if (dev->buf_mult_i >= dev->buf_mult_n) {
		dev->buf_mult_i = 0;
		ata_set_irq (dev->chn, 1);
	}
}

static
void ata_cmd_write (ata_dev_t *dev)
{
	unsigned cnt;
	uint32_t idx;

	if (ata_get_lba (dev, &idx)) {
		ata_cmd_abort (dev);
		return;
	}

#if DEBUG_ATA >= 2
		fprintf (stderr, "ata: WRITE (%lu, %u)\n",
			(unsigned long) idx, dev->reg_sec_cnt
		);
#endif

	ata_buf_reset (dev);

	dev->buf_i = 0;
	dev->buf_n = 512;

	dev->buf_mode = ATA_BUF_MODE_WRITE;

	dev->buf_blk_i = idx;
	dev->buf_blk_n = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;

	dev->callback = ata_cmd_write_cb;

	cnt = ATA_BUF_MAX / 512;
	if (cnt > dev->buf_blk_n) {
		cnt = dev->buf_blk_n;
	}

	dev->buf_m = 512 * cnt;

	dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DRQ | ATA_STATUS_DSC;
}

static
void ata_cmd_write_multiple (ata_dev_t *dev)
{
	unsigned cnt;
	uint32_t idx;

	if (ata_get_lba (dev, &idx)) {
		ata_cmd_abort (dev);
		return;
	}

#if DEBUG_ATA >= 2
		fprintf (stderr, "ata: WRITE MULTIPLE (%lu, %u)\n",
			(unsigned long) idx, dev->reg_sec_cnt
		);
#endif

	ata_buf_reset (dev);

	dev->buf_i = 0;
	dev->buf_n = 512;

	dev->buf_mode = ATA_BUF_MODE_WRITE;

	dev->buf_blk_i = idx;
	dev->buf_blk_n = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;
	dev->buf_mult_i = 0;
	dev->buf_mult_n = dev->multi_block_size;

	dev->callback = ata_cmd_write_cb;

	cnt = ATA_BUF_MAX / 512;
	if (cnt > dev->buf_blk_n) {
		cnt = dev->buf_blk_n;
	}

	dev->buf_m = 512 * cnt;

	dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DRQ | ATA_STATUS_DSC;
}

static
void ata_cmd_set_multiple_mode (ata_dev_t *dev)
{
	unsigned cnt;

#if DEBUG_ATA >= 1
		fprintf (stderr, "ata: SET MULTIPLE MODE (%u)\n",
			dev->reg_sec_cnt
		);
#endif

	cnt = dev->reg_sec_cnt;

	if (cnt > dev->multi_block_max) {
		ata_cmd_abort (dev);
		return;
	}

	dev->multi_block_size = cnt;

	ata_cmd_ok (dev);
}

static
void ata_cmd_device_diagnostic (ata_chn_t *ata)
{
	unsigned i;

#if DEBUG_ATA >= 1
		fprintf (stderr, "ata: DEVICE DIAGNOSTIC\n");
#endif

	for (i = 0; i < 2; i++) {
		ata->dev[i].reg_cyl_lo = 0;
		ata->dev[i].reg_cyl_hi = 0;
		ata->dev[i].reg_sec = 1;
		ata->dev[i].reg_sec_cnt = 1;
		ata->dev[i].reg_head = 0;
		ata->dev[i].reg_error = 0x01;
		ata->dev[i].reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
	}

	ata->sel = &ata->dev[0];
}

static
void ata_cmd_set_geometry (ata_dev_t *dev)
{
	dev->s = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;
	dev->h = (dev->reg_head & 0x0f) + 1;
	dev->c = dsk_get_block_cnt (dev->blk) / ((uint32_t) dev->s * dev->h);

#if DEBUG_ATA >= 1
		fprintf (stderr, "ata: SET GEOMETRY (%u / %u / %u)\n",
			dev->c, dev->h, dev->s
		);
#endif

	ata_cmd_ok (dev);
}

static
void ata_cmd_standby_immediate (ata_dev_t *dev)
{
#if DEBUG_ATA >= 1
		fprintf (stderr, "ata: STANDBY IMMEDIATE\n");
#endif

	ata_cmd_ok (dev);
}

static
void ata_cmd_flush_cache (ata_dev_t *dev)
{
#if DEBUG_ATA >= 1
		fprintf (stderr, "ata: FLUSH CACHE\n");
#endif

	ata_cmd_ok (dev);
}

static
void ata_cmd_identify (ata_dev_t *dev)
{
	uint32_t cnt1, cnt2;

#if DEBUG_ATA >= 1
		fprintf (stderr, "ata: IDENTIFY\n");
#endif

	memset (dev->buf, 0, 512);

	cnt1 = dev->c * dev->h * dev->s;
	cnt2 = dsk_get_block_cnt (dev->blk);

	ata_set_uint16_le (dev->buf, 2 * 0, 0x0040);
	ata_set_uint16_le (dev->buf, 2 * 1, dev->default_c);
	ata_set_uint16_le (dev->buf, 2 * 3, dev->default_h);
	ata_set_uint16_le (dev->buf, 2 * 6, dev->default_s);
	ata_set_string (dev->buf, 2 * 10, dev->serial, 20);
	ata_set_uint16_le (dev->buf, 2 * 22, 0x0000); /* vendor specific bytes */
	ata_set_string (dev->buf, 2 * 23, dev->firmware, 8);
	ata_set_string (dev->buf, 2 * 27, dev->model, 40);
	ata_set_uint16_le (dev->buf, 2 * 47, dev->multi_block_max & 0xff);
	ata_set_uint16_le (dev->buf, 2 * 49, 0x0200); /* lba */
	ata_set_uint16_le (dev->buf, 2 * 53, 0x0001);
	ata_set_uint16_le (dev->buf, 2 * 54, dev->c);
	ata_set_uint16_le (dev->buf, 2 * 55, dev->h);
	ata_set_uint16_le (dev->buf, 2 * 56, dev->s);
	ata_set_uint16_le (dev->buf, 2 * 57, cnt1 & 0xffff);
	ata_set_uint16_le (dev->buf, 2 * 58, (cnt1 >> 16) & 0xffff);
	ata_set_uint16_le (dev->buf, 2 * 60, cnt2 & 0xffff);
	ata_set_uint16_le (dev->buf, 2 * 61, (cnt2 >> 16) & 0xffff);

	dev->buf_i = 0;
	dev->buf_n = 512;
	dev->buf_mode = ATA_BUF_MODE_READ;
	dev->callback = NULL;

	dev->reg_status = ATA_STATUS_DRQ | 0x50;

	ata_set_irq (dev->chn, 1);
}

static
void ata_command (ata_chn_t *ata, unsigned cmd)
{
	if (ata->sel->blk == NULL) {
		ata_cmd_abort (ata->sel);
		return;
	}

	switch (cmd) {
	case ATA_CMD_RECALIBRATE:
		ata_cmd_recalibrate (ata->sel);
		break;

	case ATA_CMD_READ:
	case ATA_CMD_READ_RETRY:
		ata_cmd_read (ata->sel);
		break;

	case ATA_CMD_READ_MULTIPLE:
		ata_cmd_read_multiple (ata->sel);
		break;

	case ATA_CMD_WRITE:
	case ATA_CMD_WRITE_RETRY:
		ata_cmd_write (ata->sel);
		break;

	case ATA_CMD_WRITE_MULTIPLE:
		ata_cmd_write_multiple (ata->sel);
		break;

	case ATA_CMD_SET_MULTIPLE_MODE:
		ata_cmd_set_multiple_mode (ata->sel);
		break;

	case ATA_CMD_DIAGNOSTIC:
		ata_cmd_device_diagnostic (ata);
		break;

	case ATA_CMD_SET_GEOMETRY:
		ata_cmd_set_geometry (ata->sel);
		break;

	case ATA_CMD_STANDBY_IMMEDIATE1:
	case ATA_CMD_STANDBY_IMMEDIATE2:
		ata_cmd_standby_immediate (ata->sel);
		break;

	case ATA_CMD_FLUSH_CACHE:
		ata_cmd_flush_cache (ata->sel);
		break;

	case ATA_CMD_IDENTIFY:
		ata_cmd_identify (ata->sel);
		break;

	default:
		fprintf (stderr, "ata: unknown command (%02X)\n", cmd);
		fflush (stderr);
		ata_cmd_abort (ata->sel);
		break;
	}
}

static
unsigned short ata_get_data16 (ata_chn_t *ata)
{
	unsigned short val;
	ata_dev_t      *sel;

	sel = ata->sel;

	if (sel->buf_mode != ATA_BUF_MODE_READ) {
		return (0);
	}

	val = ata_get_uint16_le (sel->buf, sel->buf_i);

	sel->buf_i += 2;
	if (sel->buf_i >= sel->buf_n) {
		if (sel->callback != NULL) {
			sel->callback (sel);
		}
		else {
			sel->buf_i = 0;
			sel->buf_n = 0;
			sel->buf_m = 0;
			sel->buf_mode = ATA_BUF_MODE_NONE;
			sel->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
		}
	}

	return (val);
}

unsigned char ata_cmd_get_uint8 (ata_chn_t *ata, unsigned long addr)
{
	ata_dev_t     *sel;
	unsigned char val;

	sel = ata->sel;

	if (sel->blk == NULL) {
		return (0x00);
	}

	switch (addr) {
	case 0x00: /* data */
		val = 0;
		break;

	case 0x01: /* error */
		val = sel->reg_error;
		break;

	case 0x02: /* sector count */
		val = sel->reg_sec_cnt;
		break;

	case 0x03: /* sector number */
		val = sel->reg_sec;
		break;

	case 0x04: /* cylinder low */
		val = sel->reg_cyl_lo;
		break;

	case 0x05: /* cylinder high */
		val = sel->reg_cyl_hi;
		break;

	case 0x06: /* device / head */
		val = sel->reg_head;
		break;

	case 0x07: /* status */
		ata_set_irq (ata, 0);
		val = sel->reg_status;
		break;

	default:
		val = 0xff;
		break;
	}

#if DEBUG_ATA >= 2
	fprintf (stderr, "ata: get8 %08lX -> %02X\n", addr, val);
	fflush (stderr);
#endif

	return (val);
}

unsigned short ata_cmd_get_uint16 (ata_chn_t *ata, unsigned long addr)
{
	if (addr == 0) {
		return (ata_get_data16 (ata));
	}

	return (ata_cmd_get_uint8 (ata, addr));
}

unsigned long ata_cmd_get_uint32 (ata_chn_t *ata, unsigned long addr)
{
	return (ata_cmd_get_uint8 (ata, addr));
}

static
void ata_set_data16 (ata_chn_t *ata, unsigned short val)
{
	ata_dev_t *sel;

	sel = ata->sel;

	if (sel->buf_mode != ATA_BUF_MODE_WRITE) {
		return;
	}

	ata_set_uint16_le (sel->buf, sel->buf_i, val);

	sel->buf_i += 2;
	if (sel->buf_i >= sel->buf_n) {
		if (sel->callback != NULL) {
			sel->callback (sel);
		}
		else {
			ata_buf_reset (sel);
			sel->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
		}
	}
}

void ata_cmd_set_uint8 (ata_chn_t *ata, unsigned long addr, unsigned char val)
{
#if DEBUG_ATA >= 2
	fprintf (stderr, "ata: set8 %08lX <- %02X\n", addr, val);
	fflush (stderr);
#endif

	switch (addr) {
	case 0x00: /* data */
		break;

	case 0x01: /* features */
		ata->dev[0].reg_features = val;
		ata->dev[1].reg_features = val;
		break;

	case 0x02: /* sector count */
		ata->dev[0].reg_sec_cnt = val;
		ata->dev[1].reg_sec_cnt = val;
		break;

	case 0x03: /* sector number */
		ata->dev[0].reg_sec = val;
		ata->dev[1].reg_sec = val;
		break;

	case 0x04: /* cylinder low */
		ata->dev[0].reg_cyl_lo = val;
		ata->dev[1].reg_cyl_lo = val;
		break;

	case 0x05: /* cylinder high */
		ata->dev[0].reg_cyl_hi = val;
		ata->dev[1].reg_cyl_hi = val;
		break;

	case 0x06: /* device / head */
		ata->dev[0].reg_head = val;
		ata->dev[1].reg_head = val;
		ata->sel = &ata->dev[(val & 0x10) ? 1 : 0];
		break;

	case 0x07: /* command */
		ata_command (ata, val);
		break;
	}
}

void ata_cmd_set_uint16 (ata_chn_t *ata, unsigned long addr, unsigned short val)
{
	if (addr == 0) {
		ata_set_data16 (ata, val);
	}
}

void ata_cmd_set_uint32 (ata_chn_t *ata, unsigned long addr, unsigned long val)
{
}


unsigned char ata_ctl_get_uint8 (ata_chn_t *ata, unsigned long addr)
{
	unsigned char val;

	switch (addr) {
	case 0x02: /* alternate status */
		val = ata->sel->reg_status;
		break;

	default:
		val = 0;
		break;
	}

#if DEBUG_ATA >= 2
	fprintf (stderr, "ata: get ctl8 %08lX -> %02X\n", addr, val);
	fflush (stderr);
#endif

	return (val);
}

unsigned short ata_ctl_get_uint16 (ata_chn_t *ata, unsigned long addr)
{
	return (ata_ctl_get_uint8 (ata, addr));
}

unsigned long ata_ctl_get_uint32 (ata_chn_t *ata, unsigned long addr)
{
	return (ata_ctl_get_uint8 (ata, addr));
}

void ata_ctl_set_uint8 (ata_chn_t *ata, unsigned long addr, unsigned char val)
{
#if DEBUG_ATA >= 2
	fprintf (stderr, "ata: set ctl8 %08lX <- %02X\n", addr, val);
	fflush (stderr);
#endif

	switch (addr) {
	case 0x02:
		if ((ata->sel->reg_dev_ctl ^ val) & ~val & 0x04) {
			/* reset 1 -> 0 */
			unsigned i;

			for (i = 0; i < 2; i++) {
				ata->dev[i].reg_cyl_lo = 0;
				ata->dev[i].reg_cyl_hi = 0;
				ata->dev[i].reg_sec = 1;
				ata->dev[i].reg_sec_cnt = 1;
				ata->dev[i].reg_head = 0;
				ata->dev[i].reg_error = 0x01;
				ata->dev[i].reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
			}

			ata->sel = &ata->dev[0];
		}

		if (val & 0x02) {
			ata_set_irq (ata, 0);
		}

		ata->dev[0].reg_dev_ctl = val;
		ata->dev[1].reg_dev_ctl = val;
		break;
	}
}

void ata_ctl_set_uint16 (ata_chn_t *ata, unsigned long addr, unsigned short val)
{
}

void ata_ctl_set_uint32 (ata_chn_t *ata, unsigned long addr, unsigned long val)
{
}
