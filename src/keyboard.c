/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     keyboard.c                                                 *
 * Created:       2003-04-14 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2003-04-19 by Hampa Hug <hampa@hampa.ch>                   *
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

/* $Id: keyboard.c,v 1.5 2003/04/19 02:41:22 hampa Exp $ */


#include <stdio.h>

#include <pce.h>

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/poll.h>


keymap_t *keymap_new (void)
{
  unsigned i;
  keymap_t *map;

  map = (keymap_t *) malloc (sizeof (keymap_t));
  if (map == NULL) {
    return (NULL);
  }

  for (i = 0; i < 256; i++) {
    map->key[i] = 0;
    map->map[i] = NULL;
  }

  return (map);
}

void keymap_del (keymap_t *map)
{
  unsigned i;

  if (map != NULL) {
    for (i = 0; i < 256; i++) {
      keymap_del (map->map[i]);
    }

    free (map);
  }
}

void keymap_set_key (keymap_t *map, unsigned char *seq, unsigned cnt, unsigned long key)
{
  unsigned i;
  unsigned c;

  if (cnt == 0) {
    return;
  }

  cnt -= 1;

  for (i = 0; i < cnt; i++) {
    c = seq[i] & 0xff;
    if (map->map[c] == NULL) {
      map->map[c] = keymap_new();
    }

    map = map->map[c];
  }

  map->key[seq[cnt]] = key;
}

unsigned long keymap_get_key (keymap_t *map, unsigned char **seq, unsigned *cnt)
{
  unsigned c;
  unsigned i;

  i = 0;

  while (i < *cnt) {
    c = *(*seq + i) & 0xff;

    i += 1;

    if (map->map[c] == NULL) {
      if (map->key[c] != 0) {
        *seq += i;
        *cnt -= i;
      }

      return (map->key[c]);
    }

    map = map->map[c];
  }

  return (0);
}

keyboard_t *key_new (void)
{
  unsigned      i;
  unsigned char c;
  keyboard_t    *key;

  key = (keyboard_t *) malloc (sizeof (keyboard_t));
  if (key == NULL) {
    return (NULL);
  }

  key->brk_ext = NULL;
  key->brk = NULL;

  key->map = keymap_new();

  for (i = 32; i < 128; i++) {
    c = i;
    keymap_set_key (key->map, &c, 1, i);
  }

  keymap_set_key (key->map, "\x00", 1, 0x10000); // brk

  keymap_set_key (key->map, "\x1b", 1, 0x011b);

  keymap_set_key (key->map, "\x0a", 1, 0x1c0d);
  keymap_set_key (key->map, "\x08", 1, 0x0e08);
  keymap_set_key (key->map, "\x09", 1, 0x0f09);

  keymap_set_key (key->map, "z", 1, 0x2c7a);

  keymap_set_key (key->map, "\x1b\x5b\x32\x7e", 4, 0x5200); // ins
  keymap_set_key (key->map, "\x1b\x5b\x33\x7e", 4, 0x5300); // del

  keymap_set_key (key->map, "\x1b\x5b\x41", 3, 0x4800); // up
  keymap_set_key (key->map, "\x1b\x5b\x42", 3, 0x5000); // down
  keymap_set_key (key->map, "\x1b\x5b\x43", 3, 0x4d00); // right
  keymap_set_key (key->map, "\x1b\x5b\x44", 3, 0x4b00); // left
  keymap_set_key (key->map, "\x1b\x5b\x48", 3, 0x4700); // home
  keymap_set_key (key->map, "\x1b\x4f\x48", 3, 0x4700); // home
  keymap_set_key (key->map, "\x1b\x5b\x46", 3, 0x4f00); // end
  keymap_set_key (key->map, "\x1b\x4f\x46", 3, 0x4f00); // end
  keymap_set_key (key->map, "\x1b\x5b\x35\x7e", 4, 0x4900); // pgup
  keymap_set_key (key->map, "\x1b\x5b\x36\x7e", 4, 0x5100); // pgdn

  keymap_set_key (key->map, "\x1b\x4f\x50", 3, 0x3b00); // F1
  keymap_set_key (key->map, "\x1b\x4f\x51", 3, 0x3c00); // F2
  keymap_set_key (key->map, "\x1b\x4f\x52", 3, 0x3d00); // F3
  keymap_set_key (key->map, "\x1b\x4f\x53", 3, 0x3e00); // F4
  keymap_set_key (key->map, "\x1b\x5b\x31\x35\x7e", 5, 0x3f00); // F5
  keymap_set_key (key->map, "\x1b\x5b\x31\x37\x7e", 5, 0x4000); // F6
  keymap_set_key (key->map, "\x1b\x5b\x31\x38\x7e", 5, 0x4100); // F7
  keymap_set_key (key->map, "\x1b\x5b\x31\x39\x7e", 5, 0x4200); // F8
  keymap_set_key (key->map, "\x1b\x5b\x32\x30\x7e", 5, 0x4300); // F9
  keymap_set_key (key->map, "\x1b\x5b\x32\x31\x7e", 5, 0x4400); // F10

  keymap_set_key (key->map, "\xe3", 1, 0x2e00); // Alt-C
  keymap_set_key (key->map, "\xe6", 1, 0x2100); // Alt-F
  keymap_set_key (key->map, "\xf1", 1, 0x1000); // Alt-Q
  keymap_set_key (key->map, "\xf2", 1, 0x1300); // Alt-R
  keymap_set_key (key->map, "\xf8", 1, 0x2d00); // Alt-X

  key->mem = NULL;

  key->fd = -1;

  return (key);
}

void key_del (keyboard_t *key)
{
  if (key != NULL) {
    keymap_del (key->map);
    free (key);
  }
}

void key_set_fd (keyboard_t *key, int fd)
{
  static struct termios sios;
  struct termios        tios;

  if (fd < 0) {
    tcsetattr (key->fd, TCSANOW, &sios);
    key->fd = -1;
    return;
  }

  key->fd = fd;

  tcgetattr (fd, &sios);

  tios = sios;

  tios.c_lflag &= ~(ICANON | ECHO);
  tios.c_cc[VMIN] = 1;
  tios.c_cc[VTIME] = 0;

  if (tcsetattr (fd, TCSANOW, &tios)) {
    return;
  }
}

int fd_readable (int fd, int t)
{
  int           r;
  struct pollfd pfd[1];

  if (fd < 0) {
    return (0);
  }

  pfd[0].fd = fd;
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

void key_add_key (keyboard_t *key, unsigned short val)
{
  unsigned short buf1, buf2;

  buf1 = mem_get_uint16_le (key->mem, 0x041a);
  buf2 = mem_get_uint16_le (key->mem, 0x041c);

  mem_set_uint16_le (key->mem, 0x400 + buf2, val);

  buf2 += 2;
  if (buf2 >= 0x3e) {
    buf2 = 0x1e;
  }

  mem_set_uint16_le (key->mem, 0x41c, buf2);
}

void key_clock (keyboard_t *key)
{
  unsigned char buf[256];
  unsigned char *tmp;
  unsigned long val;
  ssize_t       r;
  unsigned      n;

  static unsigned long cnt = 0;

  cnt += 1;

  if (cnt < 32) {
    return;
  }

  cnt = 0;

  if (!fd_readable (key->fd, 0)) {
    return;
  }

  r = read (0, buf, 256);
  if (r <= 0) {
    return;
  }

  n = (unsigned) r;

  while ((n < 256) && fd_readable (key->fd, 10)) {
    r = read (0, buf + n, 256 - n);
    if (r <= 0) {
      return;
    }

    n += (unsigned) r;
  }

  tmp = buf;

  while (n > 0) {
    val = keymap_get_key (key->map, &tmp, &n);

    if (val == 0) {
      unsigned i;

      fprintf (stderr, "unknown key:"); fflush (stderr);

      for (i = 0; i < n; i++) {
        fprintf (stderr, " %02X", (unsigned) tmp[i]);
      }

      fputs ("\n", stderr);

      return;
    }


    if (val == 0x10000) {
      if (key->brk != NULL) {
        key->brk (key->brk_ext, 0);
        return;
      }
    }

    if (val != 0) {
      key_add_key (key, val & 0xffff);
    }
  }
}
