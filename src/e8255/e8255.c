/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     e8255.c                                                    *
 * Created:       2003-04-17 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-17 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: e8255.c,v 1.1 2003/04/17 11:47:27 hampa Exp $ */


#include <stdlib.h>
#include <stdio.h>

#include "e8255.h"


e8255_t *e8255_new (void)
{
  unsigned i;
  e8255_t  *ret;

  ret = (e8255_t *) malloc (sizeof (e8255_t));
  if (ret == NULL) {
    return (NULL);
  }

  ret->group_a_mode = 0;
  ret->group_b_mode = 0;
  ret->mode = 0x80;

  for (i = 0; i < 3; i++) {
    ret->port[i].val_inp = 0;
    ret->port[i].val_out = 0;
    ret->port[i].inp = 0;
    ret->port[i].read_ext = NULL;
    ret->port[i].read = NULL;
    ret->port[i].write_ext = NULL;
    ret->port[i].write = NULL;
  }

  return (ret);
}

void e8255_del (e8255_t *ppi)
{
  free (ppi);
}

void e8255_prt_state (e8255_t *ppi, FILE *fp)
{
  fprintf (fp,
    "8255: MOD=%02X  MODA=%u  MODB=%u",
    ppi->mode, ppi->group_a_mode, ppi->group_b_mode
  );

  if (ppi->port[0].inp != 0) {
    fprintf (fp, "  A=I[%02X]", e8255_get_inp (ppi, 0));
  }
  else {
    fprintf (fp, "  A=O[%02X]", e8255_get_out (ppi, 0));
  }

  if (ppi->port[1].inp != 0) {
    fprintf (fp, "  B=I[%02X]", e8255_get_inp (ppi, 1));
  }
  else {
    fprintf (fp, "  B=O[%02X]", e8255_get_out (ppi, 1));
  }

  switch (ppi->port[2].inp) {
    case 0xff:
      fprintf (fp, "  C=I[%02X]", e8255_get_inp (ppi, 2));
      break;

    case 0x00:
      fprintf (fp, "  C=O[%02X]", e8255_get_out (ppi, 2));
      break;

    case 0x0f:
      fprintf (fp, "  CH=O[%X]  CL=I[%X]",
        (e8255_get_out (ppi, 2) >> 4) & 0x0f, e8255_get_inp (ppi, 2) & 0x0f
      );
      break;

    case 0xf0:
      fprintf (fp, "  CH=I[%X]  CL=O[%X]",
        (e8255_get_inp (ppi, 2) >> 4) & 0x0f, e8255_get_out (ppi, 2) & 0x0f
      );
      break;
  }

  fputs ("\n", fp);
  fflush (fp);
}

void e8255_set_inp (e8255_t *ppi, unsigned p, unsigned char val)
{
  ppi->port[p].val_inp = val;
}

void e8255_set_out (e8255_t *ppi, unsigned p, unsigned char val)
{
  ppi->port[p].val_out = val;

  if (ppi->port[p].inp != 0xff) {
    val &= ~ppi->port[p].inp;
    if (ppi->port[p].write != NULL) {
      ppi->port[p].write (ppi->port[p].write_ext, val);
    }
  }
}

unsigned char e8255_get_inp (e8255_t *ppi, unsigned p)
{
  unsigned char val;

  if (ppi->port[p].inp != 0x00) {
    if (ppi->port[p].read != NULL) {
      ppi->port[p].val_inp = ppi->port[p].read (ppi->port[p].read_ext);
    }
  }

  val = ppi->port[p].val_inp & ppi->port[p].inp;
  val |= ppi->port[p].val_out & ~ppi->port[p].inp;

  return (val);
}

unsigned char e8255_get_out (e8255_t *ppi, unsigned p)
{
  return (ppi->port[p].val_out | ppi->port[p].inp);
}


void e8255_set_inp_a (e8255_t *ppi, unsigned char val)
{
  e8255_set_inp (ppi, 0, val);
}

void e8255_set_inp_b (e8255_t *ppi, unsigned char val)
{
  e8255_set_inp (ppi, 1, val);
}

void e8255_set_inp_c (e8255_t *ppi, unsigned char val)
{
  e8255_set_inp (ppi, 2, val);
}

void e8255_set_out_a (e8255_t *ppi, unsigned char val)
{
  e8255_set_out (ppi, 0, val);
}

void e8255_set_out_b (e8255_t *ppi, unsigned char val)
{
  e8255_set_out (ppi, 1, val);
}

void e8255_set_out_c (e8255_t *ppi, unsigned char val)
{
  e8255_set_out (ppi, 2, val);
}

unsigned char e8255_get_inp_a (e8255_t *ppi)
{
  return (e8255_get_inp (ppi, 0));
}

unsigned char e8255_get_inp_b (e8255_t *ppi)
{
  return (e8255_get_inp (ppi, 1));
}

unsigned char e8255_get_inp_c (e8255_t *ppi)
{
  return (e8255_get_inp (ppi, 2));
}

unsigned char e8255_get_out_a (e8255_t *ppi)
{
  return (e8255_get_out (ppi, 0));
}

unsigned char e8255_get_out_b (e8255_t *ppi)
{
  return (e8255_get_out (ppi, 1));
}

unsigned char e8255_get_out_c (e8255_t *ppi)
{
  return (e8255_get_out (ppi, 2));
}

void e8255_set_port8 (e8255_t *ppi, unsigned long addr, unsigned char val)
{
  switch (addr) {
    case 0:
      e8255_set_out (ppi, 0, val);
      break;

    case 1:
      e8255_set_out (ppi, 1, val);
      break;

    case 2:
      e8255_set_out (ppi, 2, val);
      break;

    case 3:
      if (val & 0x80) {
        ppi->mode = val;

        ppi->group_a_mode = (val >> 5) & 0x03;
        ppi->group_b_mode = (val >> 2) & 0x01;
        ppi->port[0].inp = (val & 0x10) ? 0xff : 0x00;
        ppi->port[1].inp = (val & 0x02) ? 0xff : 0x00;
        ppi->port[2].inp = (val & 0x01) ? 0x0f : 0x00;
        ppi->port[2].inp |= (val & 0x08) ? 0xf0 : 0x00;
      }
      else {
        unsigned bit;

        bit = (val >> 1) & 0x07;

        if (val & 1) {
          val = ppi->port[2].val_out | (1 << bit);
        }
        else {
          val = ppi->port[2].val_out & ~(1 << bit);
        }

        e8255_set_out (ppi, 2, val);
      }

      break;
  }
}

unsigned char e8255_get_port8 (e8255_t *ppi, unsigned long addr)
{
  switch (addr) {
    case 0:
      return (e8255_get_inp (ppi, 0));

    case 1:
      return (e8255_get_inp (ppi, 1));

    case 2:
      return (e8255_get_inp (ppi, 2));

    case 3:
      return (ppi->mode);
  }

  return (0);
}
