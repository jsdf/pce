/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     term.c                                                     *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: term.c,v 1.1 2003/04/18 20:05:50 hampa Exp $ */


#include <stdio.h>

#include <pce.h>


static
unsigned char colmap[8] = {
  0, 4, 2, 6, 1, 5, 3, 7
};

static
unsigned char chrmap[256] = {
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xb7,
  0xb0, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, // 80
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e,  'o', 0x2e, 0x2e, 0x2e, // 90
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, // A0
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
   '#',  '#',  '#', 0x7c,  '+',  '+',  '+',  '+', // B0
   '+',  '+',  '|',  '+',  '+',  '+',  '+',  '+',
   '+',  '+',  '+',  '+',  '-',  '+',  '+',  '+', // C0
   '+',  '+',  '+',  '+',  '+',  '-',  '+',  '+',
   '+',  '+',  '+',  '+',  '+',  '+',  '+',  '+', // D0
   '+',  '+',  '+', 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, // E0
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, // F0
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e
};

void trm_init (term_t *trm, FILE *fp)
{
  trm->x = ~0;
  trm->y = ~0;
  trm->fg = ~0;
  trm->bg = ~0;

  trm->col_chg = 1;

  trm->fp = fp;
}

void trm_free (term_t *trm)
{
}

static
unsigned trm_str_int (unsigned char *buf, unsigned n)
{
  unsigned      ret;
  unsigned      i, j;
  unsigned char tmp;

  i = 0;
  do {
    buf[i++] = '0' + n % 10;
    n = n / 10;
  } while (n != 0);

  ret = i;

  j = 0;
  while (j < i) {
    i -= 1;
    tmp = buf[j];
    buf[j] = buf[i];
    buf[i] = tmp;
    j += 1;
  }

  return (ret);
}

static
unsigned trm_str_pos (unsigned char *buf, unsigned x, unsigned y)
{
  unsigned i;

  buf[0] = 0x1b;
  buf[1] = 0x5b;

  i = 2;

  i += trm_str_int (buf + i, y + 1);

  buf[i++] = ';';

  i += trm_str_int (buf + i, x + 1);

  buf[i++] = 'H';

  return (i);
}

static
unsigned trm_str_col (unsigned char *buf, unsigned fg, unsigned bg)
{
  unsigned i;

  buf[0] = 0x1b;
  buf[1] = 0x5b;
  buf[2] = (fg & 0x08) ? '1' : '0';
  buf[3] = ';';

  i = 4;

  i += trm_str_int (buf + i, 30 + (fg & 7));

  buf[i++] = ';';

  i += trm_str_int (buf + i, 40 + (bg & 7));

  buf[i++] = 'm';

  return (i);
}

void trm_clr_scn (term_t *trm)
{
  fputs ("\x1b[2J" "\x1b[H", trm->fp);
  fflush (trm->fp);

  trm->x = 0;
  trm->y = 0;
}

void trm_set_pos (term_t *trm, unsigned x, unsigned y)
{
  unsigned      n;
  unsigned char buf[256];

  if ((x != trm->x) || (y != trm->y)) {
    n = trm_str_pos (buf, x, y);
    fwrite (buf, 1, n, trm->fp);
    fflush (trm->fp);

    trm->x = x;
    trm->y = y;
  }
}

void trm_set_col (term_t *trm, unsigned fg, unsigned bg)
{
  if ((trm->fg != fg) || (trm->bg != bg)) {
    trm->fg = fg;
    trm->bg = bg;
    trm->col_chg = 1;
  }
}

void trm_set_attr_mono (term_t *trm, unsigned char a)
{
  unsigned it, fg, bg;

  it = (a & 0x08) ? 8 : 0;
  fg = (a & 0x07) ? 7 : 0;
  bg = (a & 0x70) ? 7 : 0;

  trm_set_col (trm, fg + it, bg);
}

void trm_set_attr_col (term_t *trm, unsigned char a)
{
  unsigned it, fg, bg;

  it = (a & 0x08) ? 8 : 0;
  fg = a & 0x07;
  bg = (a >> 4) & 0x07;

  fg = colmap[fg];
  bg = colmap[bg];

  trm_set_col (trm, fg + it, bg);
}

void trm_set_chr (term_t *trm, unsigned char chr)
{
  unsigned      n;
  unsigned char buf[256];

  n = 0;

  if (trm->col_chg) {
    n += trm_str_col (buf + n, trm->fg, trm->bg);

    trm->col_chg = 0;
  }

  buf[n++] = chrmap[chr & 0xff];

  trm->x += 1;

  fwrite (buf, 1, n, trm->fp);
  fflush (trm->fp);
}

void trm_set_chr_xy (term_t *trm, unsigned x, unsigned y, unsigned char c)
{
  unsigned      n;
  unsigned char buf[256];

  n = 0;

  if ((x != trm->x) || (y != trm->y)) {
    n += trm_str_pos (buf + n, x, y);

    trm->x = x;
    trm->y = y;
  }

  if (trm->col_chg) {
    n += trm_str_col (buf + n, trm->fg, trm->bg);

    trm->col_chg = 0;
  }

  buf[n++] = chrmap[c & 0xff];

  trm->x += 1;

  fwrite (buf, 1, n, trm->fp);
  fflush (trm->fp);
}
