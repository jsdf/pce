/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/mouse.c                                          *
 * Created:       2003-08-25 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-09-13 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003 by Hampa Hug <hampa@hampa.ch>                     *
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

/* $Id: mouse.c,v 1.2 2003/09/13 18:11:42 hampa Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "pce.h"


#define MSE_IAR_SINP 0x08
#define MSE_IAR_ERBK 0x04
#define MSE_IAR_TBE  0x02
#define MSE_IAR_RRD  0x01

#define MSE_IID_SINP (0x00 << 1)
#define MSE_IID_TBE  (0x01 << 1)
#define MSE_IID_RRD  (0x02 << 1)
#define MSE_IID_ERBK (0x03 << 1)
#define MSE_IID_PND  0x01

#define MSE_DFR_DLAB 0x80
#define MSE_DFR_BRK  0x40

#define MSE_MCR_RTS 0x02
#define MSE_MCR_DTR 0x01

#define MSE_SSR_TXE 0x40
#define MSE_SSR_TBE 0x20
#define MSE_SSR_RRD 0x01


mouse_t *mse_new (unsigned short base)
{
  mouse_t *mse;

  mse = (mouse_t *) malloc (sizeof (mouse_t));
  if (mse == NULL) {
    return (NULL);
  }

  mse->reg = mem_blk_new (base, 8, 1);
  mem_blk_set_ext (mse->reg, mse);
  mse->reg->set_uint8 = (seta_uint8_f) &mse_reg_set_uint8;
  mse->reg->set_uint16 = (seta_uint16_f) &mse_reg_set_uint16;
  mse->reg->get_uint8 = (geta_uint8_f) &mse_reg_get_uint8;
  mse->reg->get_uint16 = (geta_uint16_f) &mse_reg_get_uint16;
  mem_blk_init (mse->reg, 0x00);

  mse->rcnt = 0;

  mse->accu_ok = 0;

  mse->reg->data[1] = 0x00;
  mse->reg->data[2] = MSE_IID_PND;
  mse->reg->data[3] = 0x03;
  mse->reg->data[5] = (MSE_SSR_TXE | MSE_SSR_TBE);
  mse->reg->data[6] = 0x00;

  mse->divisor = 1;

  return (mse);
}

void mse_del (mouse_t *mse)
{
  if (mse != NULL) {
    mem_blk_del (mse->reg);
    free (mse);
  }
}

mem_blk_t *mse_get_reg (mouse_t *mse)
{
  return (mse->reg);
}

void mse_send_byte (mouse_t *mse, unsigned char val)
{
  mse->reg->data[2] = MSE_IID_PND;

  if (mse->reg->data[1] & MSE_IAR_TBE) {
    mse->reg->data[2] = MSE_IID_TBE;
    if (mse->intr != NULL) {
      mse->intr (mse->intr_ext, 1);
    }
  }

  mse->reg->data[5] |= MSE_SSR_TXE | MSE_SSR_TBE;
}

void mse_recv_intr (mouse_t *mse)
{
  if (mse->reg->data[1] & MSE_IAR_RRD) {
    mse->reg->data[2] = MSE_IID_RRD;
    if (mse->intr != NULL) {
      mse->intr (mse->intr_ext, 1);
    }
  }
}

unsigned char mse_recv_get_byte (mouse_t *mse)
{
  unsigned      i;
  unsigned char ret;

  if (mse->rcnt == 0) {
    return (mse->rbuf[0]);
  }

  ret = mse->rbuf[0];

  for (i = 1; i < mse->rcnt; i++) {
    mse->rbuf[i - 1] = mse->rbuf[i];
  }

  mse->rcnt -= 1;

  if (mse->rcnt == 0) {
    mse_accu_check (mse);
  }

  if (mse->rcnt == 0) {
    mse->reg->data[5] &= ~MSE_SSR_RRD;
    if (mse->reg->data[2] == MSE_IID_RRD) {
      mse->reg->data[2] = MSE_IID_PND;
    }
  }
  else {
    mse->reg->data[5] |= MSE_SSR_RRD;
    mse_recv_intr (mse);
  }

  return (ret);
}

void mse_recv_set_byte (mouse_t *mse, unsigned char val)
{
  if (mse->rcnt >= MSE_BUF) {
    pce_log (MSG_INF, "mouse: RxRD buffer full\n");
    return;
  }

  mse->rbuf[mse->rcnt] = val;
  mse->rcnt += 1;

  mse->reg->data[5] |= MSE_SSR_RRD;

  if (mse->rcnt > 0) {
    mse_recv_intr (mse);
  }
}

void mse_accu_check (mouse_t *mse)
{
  unsigned char val;
  int           dx, dy;
  unsigned char x, y;

  if (mse->accu_ok == 0) {
    return;
  }

  if (mse->accu_dx < -127) {
    dx = -127;
  }
  else if (mse->accu_dx > 127) {
    dx = 127;
  }
  else {
    dx = mse->accu_dx;
  }

  if (mse->accu_dy < -127) {
    dy = -127;
  }
  else if (mse->accu_dy > 127) {
    dy = 127;
  }
  else {
    dy = mse->accu_dy;
  }

  if (dx < 0) {
    x = -dx;
    x = (~x + 1) & 0xff;
  }
  else {
    x = dx;
  }

  if (dy < 0) {
    y = -dy;
    y = (~y + 1) & 0xff;
  }
  else {
    y = dy;
  }

  val = 0x40;
  val |= (mse->accu_b & 0x01) ? 0x20 : 0x00;
  val |= (mse->accu_b & 0x02) ? 0x10 : 0x00;
  val |= (y >> 4) & 0x0c;
  val |= (x >> 6) & 0x03;

  mse_recv_set_byte (mse, val);
  mse_recv_set_byte (mse, x & 0x3f);
  mse_recv_set_byte (mse, y & 0x3f);

  mse->accu_dx -= dx;
  mse->accu_dy -= dy;

  mse->accu_ok = ((mse->accu_dx != 0) || (mse->accu_dy != 0));
}

void mse_set (mouse_t *mse, int dx, int dy, unsigned but)
{
  if (mse->accu_ok) {
    if (mse->accu_b != but) {
      mse_accu_check (mse);
    }

    mse->accu_ok = 1;
    mse->accu_dx += dx;
    mse->accu_dy += dy;
    mse->accu_b = but;
  }
  else {
    mse->accu_ok = 1;
    mse->accu_dx = dx;
    mse->accu_dy = dy;
    mse->accu_b = but;
  }

  if (mse->rcnt < 3) {
    mse_accu_check (mse);
  }
}

void mse_reg_set_uint8 (mouse_t *mse, unsigned long addr, unsigned char val)
{
  switch (addr) {
    case 0x00:
      if (mse->reg->data[3] & MSE_DFR_DLAB) {
        mse->divisor &= 0xff00;
        mse->divisor |= val & 0xff;
      }
      else {
        mse_send_byte (mse, val);
      }
      break;

    case 0x01:
      if (mse->reg->data[3] & MSE_DFR_DLAB) {
        mse->divisor &= 0x00ff;
        mse->divisor |= (val & 0xff) << 8;
      }
      else {
        mse->reg->data[1] = val & 0x0f;
      }
      break;

    case 0x02:
      break;

    case 0x03:
      break;

    case 0x04:
      if (val & MSE_MCR_DTR) {
        pce_log (MSG_DEB, "mouse: reset\n");
        mse->accu_ok = 0;
        mse->rcnt = 0;
        mse->reg->data[5] &= ~MSE_SSR_RRD;
        mse_recv_set_byte (mse, 'M');
      }
      mse->reg->data[4] = val & 0x1f;
      break;

    case 0x05:
      break;

    case 0x06:
      break;

    case 0x07:
      mse->reg->data[7] = val;
      break;
  }
}

void mse_reg_set_uint16 (mouse_t *mse, unsigned long addr, unsigned short val)
{
}

unsigned char mse_reg_get_uint8 (mouse_t *mse, unsigned long addr)
{
  switch (addr) {
    case 0x00:
      if (mse->reg->data[3] & MSE_DFR_DLAB) {
        return (mse->divisor & 0x00ff);
      }
      else {
        return (mse_recv_get_byte (mse));
      }
      break;

    case 0x01:
      if (mse->reg->data[3] & MSE_DFR_DLAB) {
        return ((mse->divisor & 0xff00) >> 8);
      }
      else {
        return (0xff);
      }
      break;

    case 0x02:
      return (mse->reg->data[2]);

    case 0x03:
      return (0xff);

    case 0x04:
      return (0xff);

    case 0x05:
      return (mse->reg->data[5]);

    case 0x06:
      return (mse->reg->data[6]);

    case 0x07:
      return (mse->reg->data[7]);
  }

  return (mse->reg->data[addr]);
  return (0xff);
}

unsigned short mse_reg_get_uint16 (mouse_t *mse, unsigned long addr)
{
  return (0xffff);
}
