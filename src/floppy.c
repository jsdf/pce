/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     floppy.c                                                   *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 1996-2003 by Hampa Hug <hampa@hampa.ch>                *
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

/* $Id: floppy.c,v 1.1 2003/04/15 04:03:56 hampa Exp $ */


#include <stdio.h>

#include <pce.h>

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/poll.h>


floppy_t *flp_new (unsigned c, unsigned h, unsigned s)
{
  floppy_t *flp;

  flp = (floppy_t *) malloc (sizeof (floppy_t));
  if (flp == NULL) {
    return (NULL);
  }

  flp->c = c;
  flp->h = h;
  flp->s = s;
  flp->size = c * h * s * 512UL;

  flp->data = (unsigned char *) malloc (flp->size);
  if (flp->data == NULL) {
    free (flp);
    return (NULL);
  }

  return (flp);
}

void flp_del (floppy_t *flp)
{
  if (flp != NULL) {
    free (flp->data);
    free (flp);
  }
}

int flp_load_img (floppy_t *flp, const char *fname)
{
  FILE *fp;

  fp = fopen (fname, "rb");
  if (fp == NULL) {
    return (1);
  }

  if (fread (flp->data, 1, flp->size, fp) != flp->size) {
    return (1);
  }

  return (0);
}

void flp_int13_set_status (floppy_t *flp, e8086_t *cpu, unsigned val)
{
  cpu->dreg[E86_REG_AX] &= 0x00ff;
  cpu->dreg[E86_REG_AX] |= (val & 0xff) << 8;

  cpu->mem_set_uint8 (cpu->mem, 0x441, val);

  if (val == 0) {
    cpu->flg &= ~E86_FLG_C;
  }
  else {
    cpu->flg |= E86_FLG_C;
  }
}

void flp_int13_read (floppy_t *flp, e8086_t *cpu)
{
  unsigned cnt;
  unsigned drive;
  unsigned c, h, s;
  unsigned seg, ofs;
  unsigned long i, n;

  drive = cpu->dreg[E86_REG_DX] & 0xff;

  cnt = cpu->dreg[E86_REG_AX] & 0xff;
  c = ((cpu->dreg[E86_REG_CX] >> 8) & 0xff);
  c |= ((cpu->dreg[E86_REG_CX] >> 6) & 0x03) << 8;
  h = (cpu->dreg[E86_REG_DX] >> 8) & 0xff;
  s = cpu->dreg[E86_REG_CX] & 0x3f;

  if ((s < 1) || (s > flp->s) || (h >= flp->h) || (c >= flp->c)) {
    flp_int13_set_status (flp, cpu, 0x04);
    return;
  }

  seg = cpu->sreg[E86_REG_ES];
  ofs = cpu->dreg[E86_REG_BX];

  i = 512UL * ((c * flp->h + h) * flp->s + s - 1);
  n = 512UL * cnt;

  fprintf (stderr, "int13: read %u at %u/%u/%u (%lu)\n", cnt, c, h, s, i);

  while (n > 0) {
    cpu->mem_set_uint8 (cpu->mem, (seg << 4) + ofs, flp->data[i]);
    i += 1;
    n -= 1;
    ofs = (ofs + 1) & 0xffff;
  }

  flp_int13_set_status (flp, cpu, 0);
}

void flp_int13_write (floppy_t *flp, e8086_t *cpu)
{
  unsigned cnt;
  unsigned drive;
  unsigned c, h, s;
  unsigned seg, ofs;
  unsigned long i, n;

  drive = cpu->dreg[E86_REG_DX] & 0xff;

  cnt = cpu->dreg[E86_REG_AX] & 0xff;
  c = ((cpu->dreg[E86_REG_CX] >> 8) & 0xff);
  c |= ((cpu->dreg[E86_REG_CX] >> 6) & 0x03) << 8;
  h = (cpu->dreg[E86_REG_DX] >> 8) & 0xff;
  s = cpu->dreg[E86_REG_CX] & 0x3f;

  if ((s < 1) || (s > flp->s) || (h >= flp->h) || (c >= flp->c)) {
    flp_int13_set_status (flp, cpu, 0x04);
    return;
  }

  seg = cpu->sreg[E86_REG_ES];
  ofs = cpu->dreg[E86_REG_BX];

  i = 512UL * ((c * flp->h + h) * flp->s + s - 1);
  n = 512UL * cnt;

  fprintf (stderr, "int13: write %u at %u/%u/%u (%lu)\n", cnt, c, h, s, i);

  while (n > 0) {
    flp->data[i] = cpu->mem_get_uint8 (cpu->mem, (seg << 4) + ofs);
    i += 1;
    n -= 1;
    ofs = (ofs + 1) & 0xffff;
  }

  flp_int13_set_status (flp, cpu, 0);
}

void flp_int_13 (floppy_t *flp, e8086_t *cpu)
{
  unsigned func;

  func = (cpu->dreg[E86_REG_AX] >> 8) & 0xff;

  switch (func) {
    case 0x00:
      flp_int13_set_status (flp, cpu, 0);
      break;

    case 0x01:
      flp_int13_set_status (flp, cpu, cpu->mem_get_uint8 (cpu->mem, 0x441));
      break;

    case 0x02:
      flp_int13_read (flp, cpu);
      break;

    case 0x03:
      flp_int13_write (flp, cpu);
      break;

    case 0x04:
      flp_int13_set_status (flp, cpu, 0);
      break;

    case 0x08:
      if ((cpu->dreg[E86_REG_DX] & 0xff) > 0) {
        flp_int13_set_status (flp, cpu, 1);
        return;
      }
      cpu->dreg[E86_REG_CX] = (flp->c << 8) | flp->s;
      cpu->dreg[E86_REG_DX] = (flp->h << 8);
      flp_int13_set_status (flp, cpu, 0);
      break;

    case 0x15:
      flp_int13_set_status (flp, cpu, 1);
      break;

    default:
      fprintf (stderr, "int 13 func %02x\n", func);
      flp_int13_set_status (flp, cpu, 1);
      break;
  }
}
