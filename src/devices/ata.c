/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/devices/ata.c                                          *
 * Created:       2004-12-03 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-12-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2004 Hampa Hug <hampa@hampa.ch>                        *
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

/* $Id$ */


#include <stdio.h>

#include "ata.h"


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

#define ATA_CMD_RECALIBRATE  0x10
#define ATA_CMD_READ         0x20
#define ATA_CMD_READ_RETRY   0x21
#define ATA_CMD_WRITE        0x30
#define ATA_CMD_WRITE_RETRY  0x31
#define ATA_CMD_DIAGNOSTIC   0x90
#define ATA_CMD_SET_GEOMETRY 0x91
#define ATA_CMD_IDENTIFY     0xEC


unsigned char ata_ctl_get_uint8 (ata_chn_t *ata, unsigned long addr);
unsigned short ata_ctl_get_uint16 (ata_chn_t *ata, unsigned long addr);
unsigned long ata_ctl_get_uint32 (ata_chn_t *ata, unsigned long addr);

void ata_ctl_set_uint8 (ata_chn_t *ata, unsigned long addr, unsigned char val);
void ata_ctl_set_uint16 (ata_chn_t *ata, unsigned long addr, unsigned short val);
void ata_ctl_set_uint32 (ata_chn_t *ata, unsigned long addr, unsigned long val);


void ata_dev_init (ata_dev_t *dev)
{
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

  dev->buf_i = 0;
  dev->buf_n = 0;
  dev->buf_mode = ATA_BUF_MODE_NONE;
  dev->callback = NULL;
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

  dev->c = blk->c;
  dev->h = blk->h;
  dev->s = blk->s;

  dev->default_c = blk->c;
  dev->default_h = blk->h;
  dev->default_s = blk->s;
}

static
void ata_set_irq (ata_chn_t *ata, unsigned char val)
{
  val = (val != 0);

  if (val != ata->irq_val) {
    ata->irq_val = val;

    if (ata->irq != NULL) {
      ata->irq (ata->irq_ext, val);
    }
  }
}

static
unsigned short ata_get_uint16_le (void *buf, unsigned i)
{
  unsigned short val;
  unsigned char  *tmp = (unsigned char *) buf + i;

  val = tmp[1];
  val = (val << 8) | tmp[0];

  return (val);
}

static
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
  dev->reg_cyl_lo = 0;
  dev->reg_cyl_hi = 0;
  dev->reg_head &= 0xf0;
  dev->reg_sec = (dev->reg_head & 0x40) ? 0 : 1;
  ata_cmd_ok (dev);
}

static
void ata_cmd_read_cb (ata_dev_t *dev)
{
  if (dev->buf_blk_n == 0) {
    dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
    dev->buf_i = 0;
    dev->buf_n = 0;
    dev->buf_mode = ATA_BUF_MODE_NONE;
    dev->callback = NULL;
    return;
  }

  if (dsk_read_lba (dev->blk, dev->buf, dev->buf_blk_i, 1)) {
    ata_cmd_abort (dev);
    return;
  }

  dev->buf_blk_i += 1;
  dev->buf_blk_n -= 1;
  dev->buf_i = 0;
  dev->buf_n = 512;
  dev->buf_mode = ATA_BUF_MODE_READ;
  dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DRQ | ATA_STATUS_DSC;

  ata_set_irq (dev->chn, 1);
}

static
void ata_cmd_read (ata_dev_t *dev)
{
  uint32_t c, h, s;
  uint32_t idx;

  c = (dev->reg_cyl_hi << 8) | dev->reg_cyl_lo;
  h = dev->reg_head & 0x0f;
  s = dev->reg_sec;

  if (dsk_get_lba (dev->blk, c, h, s, &idx)) {
    ata_cmd_abort (dev);
    return;
  }

  dev->buf_blk_i = idx;
  dev->buf_blk_n = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;
  dev->callback = ata_cmd_read_cb;

  ata_cmd_read_cb (dev);
}

static
void ata_cmd_write_cb (ata_dev_t *dev)
{
  if (dsk_write_lba (dev->blk, dev->buf, dev->buf_blk_i, 1)) {
    ata_cmd_abort (dev);
    return;
  }

  dev->buf_blk_i += 1;
  dev->buf_blk_n -= 1;

  if (dev->buf_blk_n == 0) {
    dev->buf_i = 0;
    dev->buf_n = 0;
    dev->buf_mode = ATA_BUF_MODE_NONE;
    dev->callback = NULL;
    dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
  }
  else {
    dev->buf_i = 0;
    dev->buf_n = 512;
    dev->buf_mode = ATA_BUF_MODE_WRITE;
    dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DRQ | ATA_STATUS_DSC;
  }

  ata_set_irq (dev->chn, 1);
}

static
void ata_cmd_write (ata_dev_t *dev)
{
  uint32_t c, h, s;
  uint32_t idx;

  c = (dev->reg_cyl_hi << 8) | dev->reg_cyl_lo;
  h = dev->reg_head & 0x0f;
  s = dev->reg_sec;

  if (dsk_get_lba (dev->blk, c, h, s, &idx)) {
    ata_cmd_abort (dev);
    return;
  }

  dev->buf_i = 0;
  dev->buf_n = 512;
  dev->buf_mode = ATA_BUF_MODE_WRITE;
  dev->buf_blk_i = idx;
  dev->buf_blk_n = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;
  dev->callback = ata_cmd_write_cb;

  dev->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DRQ | ATA_STATUS_DSC;
}

static
void ata_cmd_device_diagnostic (ata_chn_t *ata)
{
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

static
void ata_cmd_set_geometry (ata_dev_t *dev)
{
  dev->s = (dev->reg_sec_cnt == 0) ? 256 : dev->reg_sec_cnt;
  dev->h = (dev->reg_head & 0x0f) + 1;
  dev->c = dsk_get_block_cnt (dev->blk) / ((uint32_t) dev->s * dev->h);

  ata_cmd_ok (dev);
}

static
void ata_cmd_identify (ata_dev_t *dev)
{
  memset (dev->buf, 0, 512);

  ata_set_uint16_le (dev->buf, 2 * 0, 0x0040);
  ata_set_uint16_le (dev->buf, 2 * 1, dev->default_c);
  ata_set_uint16_le (dev->buf, 2 * 3, dev->default_h);
  ata_set_uint16_le (dev->buf, 2 * 6, dev->default_s);
  ata_set_uint16_le (dev->buf, 2 * 10, 0);
  ata_set_string (dev->buf, 2 * 27, "PCEDISK", 40);
  ata_set_uint16_le (dev->buf, 2 * 47, 0x0000); /* multiple */
  ata_set_uint16_le (dev->buf, 2 * 49, 0x0200); /* lba */
  ata_set_uint16_le (dev->buf, 2 * 54, dev->c);
  ata_set_uint16_le (dev->buf, 2 * 55, dev->h);
  ata_set_uint16_le (dev->buf, 2 * 56, dev->s);

  dev->buf_i = 0;
  dev->buf_n = 512;
  dev->buf_mode = ATA_BUF_MODE_READ;
  dev->callback = NULL;

  dev->reg_status = ATA_STATUS_DRQ;

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

  case ATA_CMD_WRITE:
  case ATA_CMD_WRITE_RETRY:
    ata_cmd_write (ata->sel);
    break;

  case ATA_CMD_DIAGNOSTIC:
    ata_cmd_device_diagnostic (ata);
    break;

  case ATA_CMD_SET_GEOMETRY:
    ata_cmd_set_geometry (ata->sel);
    break;

  case ATA_CMD_IDENTIFY:
    ata_cmd_identify (ata->sel);
    break;

  default:
    fprintf (stderr, "ata: cmd=%02X\n", cmd);
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

//  fprintf (stderr, "ata: get8 %08lX -> %02X\n", addr, val);
//  fflush (stderr);

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
      sel->buf_i = 0;
      sel->buf_n = 0;
      sel->buf_mode = ATA_BUF_MODE_NONE;
      sel->reg_status = ATA_STATUS_DRDY | ATA_STATUS_DSC;
    }
  }
}

void ata_cmd_set_uint8 (ata_chn_t *ata, unsigned long addr, unsigned char val)
{
//  fprintf (stderr, "ata: set8 %08lX <- %02X\n", addr, val);
//  fflush (stderr);

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
//  fprintf (stderr, "ata: get ctl8 %08lX\n", addr);
//  fflush (stderr);

  switch (addr) {
  case 0x02: /* alternate status */
    return (ata->sel->reg_status);
  }

  return (0);
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
    ata->sel->reg_dev_ctl = val;
    break;
  }
}

void ata_ctl_set_uint16 (ata_chn_t *ata, unsigned long addr, unsigned short val)
{
}

void ata_ctl_set_uint32 (ata_chn_t *ata, unsigned long addr, unsigned long val)
{
}
