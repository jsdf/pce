/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/vt100.c                                       *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-25 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: vt100.c,v 1.1 2003/04/24 23:18:17 hampa Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/poll.h>

#include <terminal/terminal.h>
#include <terminal/vt100.h>


static
unsigned char colmap[8] = {
  0, 4, 2, 6, 1, 5, 3, 7
};

static
unsigned char chrmap[256] = {
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xb7,
  0xb0, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
   '>',  '<', 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
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
  0x2e, 0xfc, 0x2e, 0xe4, 0x2e, 0x2e, 0x2e, 0x2e, // 80
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0xf6, 0x2e, 0x2e, 0x2e, // 90
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, // A0
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xab, 0xbb,
   '#',  '#',  '#', 0x7c,  '+',  '+',  '+',  '+', // B0
   '+',  '+',  '|',  '+',  '+',  '+',  '+',  '+',
   '+',  '+',  '+',  '+',  '-',  '+',  '+',  '+', // C0
   '+',  '+',  '+',  '+',  '+',  '-',  '+',  '+',
   '+',  '+',  '+',  '+',  '+',  '+',  '+',  '+', // D0
   '+',  '+',  '+', 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0xdf, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, // E0
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xf7, 0x2e, // F0
  0xb0, 0xb7, 0xb7, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e
};


vt100_keymap_t *vt100_get_key (vt100_t *vt, unsigned char *key, unsigned cnt)
{
  unsigned i;

  if (cnt == 1) {
    return (vt->keymap + (key[0] & 0xff));
  }

  for (i = 256; i < (vt->key_cnt - 256); i++) {
    if ((vt->keymap[i].key_cnt == cnt) && (memcmp (key, vt->keymap[i].key, cnt) == 0)) {
      return (&vt->keymap[i]);
    }
  }

  return (NULL);
}

void vt100_set_key (vt100_t *vt,
  unsigned char *key, unsigned key_cnt,
  unsigned char *seq, unsigned seq_cnt)
{
  vt100_keymap_t *map;

  if ((key_cnt > 8) || (seq_cnt > 8)) {
    return;
  }

  map = vt100_get_key (vt, key, key_cnt);

  if (map == NULL) {
    vt->keymap = (vt100_keymap_t *) realloc (vt->keymap,
      (vt->key_cnt + 1) * sizeof (vt100_keymap_t)
    );
    map = vt->keymap + vt->key_cnt;
    vt->key_cnt += 1;
  }

  memcpy (map->key, key, key_cnt);
  memcpy (map->seq, seq, seq_cnt);
  map->key_cnt = key_cnt;
  map->seq_cnt = seq_cnt;
}

void vt100_init (vt100_t *vt, int inp, int out)
{
  unsigned i;

  trm_init (&vt->trm);

  vt->trm.ext = vt;

  vt->trm.del = (trm_del_f) &vt100_del;
  vt->trm.set_size = (trm_set_size_f) &vt100_set_size;
  vt->trm.set_col = (trm_set_col_f) &vt100_set_col;
  vt->trm.set_crs = (trm_set_crs_f) &vt100_set_crs;
  vt->trm.set_pos = (trm_set_pos_f) &vt100_set_pos;
  vt->trm.set_chr = (trm_set_chr_f) &vt100_set_chr;
  vt->trm.check = (trm_check_f) &vt100_check;

  vt->scn_x = 0;
  vt->scn_y = 0;

  vt->crs_x = 0;
  vt->crs_y = 0;

  vt->crs_y1 = 1;
  vt->crs_y2 = 0;

  vt->fd_inp = inp;
  vt->fd_out = out;

  vt->key_cnt = 256;
  vt->keymap = (vt100_keymap_t *) malloc (256 * sizeof (vt100_keymap_t));

  for (i = 0; i < 256; i++) {
    vt->keymap[i].key[0] = i;
    vt->keymap[i].key_cnt = 1;
    vt->keymap[i].seq_cnt = 0;
  }

  vt100_set_key (vt, "\x20", 1, "\x39\xb9", 2);
  vt100_set_key (vt, "\x21", 1, "\x2a\x02\x82\xaa", 4);
  vt100_set_key (vt, "\x22", 1, "\x2a\x28\xa8\xaa", 4);
  vt100_set_key (vt, "\x23", 1, "\x2a\x04\x84\xaa", 4);
  vt100_set_key (vt, "\x24", 1, "\x2a\x05\x85\xaa", 4);
  vt100_set_key (vt, "\x25", 1, "\x2a\x06\x86\xaa", 4);
  vt100_set_key (vt, "\x26", 1, "\x2a\x08\x88\xaa", 4);
  vt100_set_key (vt, "\x27", 1, "\x2a\x28\xa8\xaa", 4);
  vt100_set_key (vt, "\x28", 1, "\x2a\x0a\x8a\xaa", 4);
  vt100_set_key (vt, "\x29", 1, "\x2a\x0b\x8b\xaa", 4);
  vt100_set_key (vt, "\x2a", 1, "\x2a\x09\x89\xaa", 4);
  vt100_set_key (vt, "\x2b", 1, "\x2a\x0d\x8d\xaa", 4);
  vt100_set_key (vt, "\x2c", 1, "\x33\xb3", 2);
  vt100_set_key (vt, "\x2d", 1, "\x0c\x8c", 2);

#if 0
  0x34, 0x35,
  0x0b, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0a, 0x27, 0x27, 0x33, 0x0d, 0x34, 0x35,
  0x03,
  0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22,
  0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
  0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11,
  0x2d, 0x15, 0x2c, 0x1a,
  0x2b,
  0x1b, 0x07, 0x0c,
  0x29, 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22,
  0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
  0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11,
  0x2d, 0x15, 0x2c, 0x1a,
  0x2b,

  0x1b,

  0x29,
  0
#endif
  vt100_set_key (vt, "a", 1, "\x1e\x9e", 2);
  vt100_set_key (vt, "b", 1, "\x30\xb0", 2);

  vt100_set_key (vt, "A", 1, "\x2a\x1e\x9e\xaa", 4);
  vt100_set_key (vt, "B", 1, "\x2a\x30\xb0\xaa", 4);

  vt100_set_key (vt, "\x08", 1, "\x0e\x8e", 2);
  vt100_set_key (vt, "\x0a", 1, "\x1c\x9c", 2);
  vt100_set_key (vt, "\x1b", 1, "\x01\x81", 2);

  vt100_set_key (vt, "\x1b\x4f\x50", 3, "\x3b\xbb", 2);
}

terminal_t *vt100_new (int inp, int out)
{
  vt100_t *vt;

  vt = (vt100_t *) malloc (sizeof (vt100_t));
  if (vt == NULL) {
    return (NULL);
  }

  vt100_init (vt, inp, out);

  return (&vt->trm);
}

void vt100_free (vt100_t *vt)
{
}

void vt100_del (vt100_t *vt)
{
  if (vt != NULL) {
    vt100_free (vt);
    free (vt);
  }
}

static
int vt100_readable (vt100_t *vt, int t)
{
  int           r;
  struct pollfd pfd[1];

  if (vt->fd_inp < 0) {
    return (0);
  }

  pfd[0].fd = vt->fd_inp;
  pfd[0].events = POLLIN;

  r = poll (pfd, 1, t);
  if (r < 0) {
    return (0);
  }

  if ((pfd[0].revents & POLLIN) == 0) {
    return (0);
  }

  return (1);
}

#if 0
static
int vt100_writeable (vt100_t *vt, int t)
{
  int           r;
  struct pollfd pfd[1];

  if (vt->fd_out < 0) {
    return (0);
  }

  pfd[0].fd = vt->fd_out;
  pfd[0].events = POLLOUT;

  r = poll (pfd, 1, t);
  if (r < 0) {
    return (0);
  }

  if ((pfd[0].revents & POLLOUT) == 0) {
    return (0);
  }

  return (1);
}
#endif

static
void vt100_write (vt100_t *vt, const void *buf, unsigned cnt)
{
  write (vt->fd_out, buf, cnt);
}

static
unsigned vt100_str_int (unsigned char *buf, unsigned n)
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

void vt100_set_size (vt100_t *vt, unsigned w, unsigned h)
{
}

static
void vt100_set_pos_scn (vt100_t *vt, unsigned x, unsigned y)
{
  unsigned      i;
  unsigned char buf[256];

  vt->scn_x = x;
  vt->scn_y = y;

  buf[0] = 0x1b;
  buf[1] = 0x5b;
  i = 2;
  i += vt100_str_int (buf + i, y + 1);
  buf[i++] = ';';
  i += vt100_str_int (buf + i, x + 1);
  buf[i++] = 'H';

  vt100_write (vt, buf, i);
}

void vt100_set_col (vt100_t *vt, unsigned fg, unsigned bg)
{
  unsigned      i;
  unsigned      it;
  unsigned char buf[256];

  fg &= 15;
  bg &= 15;

  if ((vt->fg == fg) && (vt->bg == bg)) {
    return;
  }

  vt->fg = fg;
  vt->bg = bg;

  it = (fg > 7) ? 1 : 0;
  fg = colmap[fg & 7];
  bg = colmap[bg & 7];

  buf[0] = 0x1b;
  buf[1] = 0x5b;
  buf[2] = '0' + it;
  buf[3] = ';';

  i = 4;
  i += vt100_str_int (buf + i, 30 + fg);
  buf[i++] = ';';
  i += vt100_str_int (buf + i, 40 + bg);
  buf[i++] = 'm';

  vt100_write (vt, buf, i);
}

void vt100_set_crs (vt100_t *vt, unsigned y1, unsigned y2)
{
  vt->crs_y1 = y1;
  vt->crs_y2 = y2;
}

void vt100_set_pos (vt100_t *vt, unsigned x, unsigned y)
{
  vt->crs_x = x;
  vt->crs_y = y;

  if (vt->crs_y1 <= vt->crs_y2) {
    vt100_set_pos_scn (vt, x, y);
  }
}

void vt100_set_chr (vt100_t *vt, unsigned x, unsigned y, unsigned char c)
{
  if ((vt->scn_x != x) || (vt->scn_y != y)) {
    vt100_set_pos_scn (vt, x, y);
  }

  c = chrmap[c & 0xff];

  vt100_write (vt, &c, 1);

  vt->scn_x += 1;
}

void vt100_check (vt100_t *vt)
{
  unsigned      i, n;
  unsigned char buf[8];
  ssize_t       r;
  vt100_keymap_t *key;

//  fprintf (stderr, "vt100: check\n");

  if (!vt100_readable (vt, vt->fd_inp)) {
    return;
  }

  r = read (vt->fd_inp, buf, 8);
  if (r <= 0) {
    return;
  }

  n = (unsigned) r;

  if ((n == 1) && (buf[0] == 0)) {
    if (vt->trm.set_brk != NULL) {
      vt->trm.set_brk (vt->trm.key_ext, 0);
    }
    return;
  }

  key = vt100_get_key (vt, buf, n);

  if (key == NULL) {
    fprintf (stderr, "vt100: unknown key %u %02X %02X %02X\n", n, buf[0], buf[1], buf[2]);
    return;
  }

  if (vt->trm.set_key != NULL) {
    for (i = 0; i < key->seq_cnt; i++) {
      vt->trm.set_key (vt->trm.key_ext, key->seq[i]);
    }
  }
}
