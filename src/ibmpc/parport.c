/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/ibmpc/parport.c                                        *
 * Created:       2003-04-29 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-29 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: parport.c,v 1.1 2003/04/29 00:51:54 hampa Exp $ */


#include <stdio.h>

#include "pce.h"


#define PARPORT_STR 0x01

#define PARPORT_BSY 0x80
#define PARPORT_ACK 0x40
#define PARPORT_OFF 0x10
#define PARPORT_ERR 0x08


parport_t *parport_new (unsigned base)
{
  parport_t *par;

  par = (parport_t *) malloc (sizeof (parport_t));
  if (par == NULL) {
    return (NULL);
  }

  par->prt = mem_blk_new (base, 4, 0);
  par->prt->ext = par;
  par->prt->get_uint8 = (geta_uint8_f) &parport_get_uint8;
  par->prt->set_uint8 = (seta_uint8_f) &parport_set_uint8;
  par->prt->get_uint16 = (geta_uint16_f) &parport_get_uint16;
  par->prt->set_uint16 = (seta_uint16_f) &parport_set_uint16;

  par->status = 0;
  par->control = 0;
  par->data = 0;

  par->fp = NULL;
  par->close = 0;

  return (par);
}

void parport_del (parport_t *par)
{
  if (par != NULL) {
    mem_blk_del (par->prt);

    if (par->close && (par->fp != NULL)) {
      fclose (par->fp);
    }

    free (par);
  }
}

void parport_set_fp (parport_t *par, FILE *fp, int close)
{
  if (par->close) {
    fclose (par->fp);
  }

  par->fp = fp;

  if (fp == NULL) {
    par->close = 0;
    par->status &= ~(PARPORT_BSY | PARPORT_OFF | PARPORT_ERR);
  }
  else {
    par->close = close;
    par->status |= (PARPORT_BSY | PARPORT_OFF | PARPORT_ERR);
  }
}

int parport_set_fname (parport_t *par, const char *fname)
{
  FILE *fp;

  fp = fopen (fname, "wb");
  if (fp == NULL) {
    return (1);
  }

  parport_set_fp (par, fp, 1);

  return (0);
}

void parport_set_control (parport_t *par, unsigned char val)
{
  par->control = val;

  if (val & PARPORT_STR) {
    if (par->fp != NULL) {
      fputc (par->data, par->fp);
      par->status |= PARPORT_BSY;
      par->status &= ~(PARPORT_ACK);
      fflush (par->fp);
    }
  }
}

void parport_set_uint8 (parport_t *par, unsigned long addr, unsigned char val)
{
  switch (addr) {
    case 0x00:
      par->data = val;
      break;

    case 0x01:
      break;

    case 0x02:
      parport_set_control (par, val);
      break;
  }
}

void parport_set_uint16 (parport_t *par, unsigned long addr, unsigned short val)
{
  parport_set_uint8 (par, addr, val);
}

unsigned char parport_get_uint8 (parport_t *par, unsigned long addr)
{
  switch (addr) {
    case 0x00:
      return (par->data);

    case 0x01:
      return (par->status);

    case 0x02:
      return (par->control);

    default:
      return (0xff);
  }
}

unsigned short parport_get_uint16 (parport_t *par, unsigned long addr)
{
  return (parport_get_uint8 (par, addr));
}
