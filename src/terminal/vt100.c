/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/vt100.c                                       *
 * Created:       2003-04-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2006-07-24 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2006 Hampa Hug <hampa@hampa.ch>                   *
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


#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

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
	0x2e, 0xfc, 0x2e, 0xe4, 0x2e, 0x2e, 0x2e, 0x2e, /* 80 */
	0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
	0x2e, 0x2e, 0x2e, 0x2e, 0xf6, 0x2e, 0x2e, 0x2e, /* 90 */
	0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
	0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, /* A0 */
	0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xab, 0xbb,
	 '#',  '#',  '#', 0x7c,  '+',  '+',  '+',  '+', /* B0 */
	 '+',  '+',  '|',  '+',  '+',  '+',  '+',  '+',
	 '+',  '+',  '+',  '+',  '-',  '+',  '+',  '+', /* C0 */
	 '+',  '+',  '+',  '+',  '+',  '-',  '+',  '+',
	 '+',  '+',  '+',  '+',  '+',  '+',  '+',  '+', /* D0 */
	 '+',  '+',  '+', 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
	0x2e, 0xdf, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, /* E0 */
	0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
	0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0xf7, 0x2e, /* F0 */
	0xb0, 0xb7, 0xb7, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e
};


static
vt100_keymap_t *vt100_get_key (vt100_t *vt, const void *key, unsigned cnt)
{
	unsigned            i;
	const unsigned char *tkey;
	vt100_keymap_t      *map, *ret;

	tkey = key;

	if (cnt == 1) {
		return (vt->keymap + (tkey[0] & 0xff));
	}

	map = vt->keymap;
	ret = NULL;

	for (i = 0; i < vt->key_cnt; i++) {
		if (map->key_cnt <= cnt) {
			if (memcmp (tkey, map->key, map->key_cnt) == 0) {
				if ((ret == NULL) || (ret->key_cnt < map->key_cnt)) {
					ret = map;
				}
			}
		}

		map += 1;
	}

	return (ret);
}

static
void vt100_set_key (vt100_t *vt,
	const void *key, unsigned key_cnt,
	const void *seq, unsigned seq_cnt)
{
	vt100_keymap_t *map;

	if ((key_cnt > VT100_KEY_MAX) || (seq_cnt > VT100_KEY_MAX)) {
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

static
int get_key_byte_hex (unsigned char c, unsigned char *ret)
{
	if ((c >= '0') && (c <= '9')) {
		*ret = c - '0';
	}
	else if ((c >= 'a') && (c <= 'f')) {
		*ret = c - 'a' + 10;
	}
	else if ((c >= 'A') && (c <= 'F')) {
		*ret = c - 'A' + 10;
	}
	else {
		return (1);
	}

	return (0);
}

static
int get_key_byte (const char **str, unsigned char *ret)
{
	const char *tmp;

	tmp = *str;

	if (*tmp == 0) {
		return (1);
	}

	if (*tmp == '\\') {
		if (tmp[1] == 0) {
			return (1);
		}
		else if ((tmp[1] == 'x') || (tmp[1] == 'X')) {
			unsigned char d1, d2;

			if (get_key_byte_hex (tmp[2], &d1)) {
				return (1);
			}

			if (get_key_byte_hex (tmp[3], &d2)) {
				return (1);
			}

			*ret = (d1 << 4) | d2;
			*str += 4;
		}
		else {
			*ret = tmp[1];
			*str += 2;
		}

		return (0);
	}

	*ret = *tmp;
	*str += 1;

	return (0);
}

int vt100_set_key_str (vt100_t *vt, const char *str)
{
	const char    *tmp;
	unsigned      key_cnt, seq_cnt;
	unsigned char key[256], seq[256];

	key_cnt = 0;
	seq_cnt = 0;

	tmp = str;

	while ((*tmp != 0) && (*tmp != '=')) {
		if (get_key_byte (&tmp, &key[key_cnt])) {
			return (1);
		}

		key_cnt += 1;
	}

	if (*tmp != '=') {
		return (1);
	}

	tmp += 1;

	while (*tmp != 0) {
		if (get_key_byte (&tmp, &seq[seq_cnt])) {
			return (1);
		}

		seq_cnt += 1;
	}

#if 0
	{
		unsigned i;

		fputs ("vt100: keymap:", stderr);
		for (i = 0; i < key_cnt; i++) {
			fprintf (stderr, " %02x", key[i]);
		}
		fputs (" ->", stderr);
		for (i = 0; i < seq_cnt; i++) {
			fprintf (stderr, " %02x", seq[i]);
		}
		fputs ("\n", stderr);
	}
#endif

	vt100_set_key (vt, key, key_cnt, seq, seq_cnt);

	return (0);
}

void vt100_init (vt100_t *vt, ini_sct_t *ini, int inp, int out)
{
	unsigned i;

	trm_init (&vt->trm);

	vt->trm.ext = vt;

	vt->trm.del = (void *) vt100_del;
	vt->trm.set_mode = (trm_set_mode_f) &vt100_set_mode;
	vt->trm.set_col = (trm_set_col_f) &vt100_set_col;
	vt->trm.set_crs = (trm_set_crs_f) &vt100_set_crs;
	vt->trm.set_pos = (trm_set_pos_f) &vt100_set_pos;
	vt->trm.set_chr = (trm_set_chr_f) &vt100_set_chr;
	vt->trm.check = (trm_check_f) &vt100_check;

	vt->scn_x = ~0;
	vt->scn_y = ~0;

	vt->crs_x = 0;
	vt->crs_y = 0;

	vt->crs_on = 0;
	vt->crs_y1 = 0;
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

	vt100_set_key (vt, "\x01", 1, "\x1d\x1e\x9e\x9d", 4);
	vt100_set_key (vt, "\x02", 1, "\x1d\x30\xb0\x9d", 4);
	vt100_set_key (vt, "\x03", 1, "\x1d\x2e\xae\x9d", 4);
	vt100_set_key (vt, "\x04", 1, "\x1d\x20\xa0\x9d", 4);
	vt100_set_key (vt, "\x05", 1, "\x1d\x12\x92\x9d", 4);
	vt100_set_key (vt, "\x06", 1, "\x1d\x21\xa1\x9d", 4);
	vt100_set_key (vt, "\x07", 1, "\x1d\x22\xa2\x9d", 4);
	vt100_set_key (vt, "\x08", 1, "\x1d\x23\xa3\x9d", 4);
	vt100_set_key (vt, "\x09", 1, "\x1d\x17\x97\x9d", 4);
	vt100_set_key (vt, "\x0a", 1, "\x1d\x24\xa4\x9d", 4);
	vt100_set_key (vt, "\x0b", 1, "\x1d\x25\xa5\x9d", 4);
	vt100_set_key (vt, "\x0c", 1, "\x1d\x26\xa6\x9d", 4);
	vt100_set_key (vt, "\x0d", 1, "\x1d\x32\xb2\x9d", 4);
	vt100_set_key (vt, "\x0e", 1, "\x1d\x31\xb1\x9d", 4);
	vt100_set_key (vt, "\x0f", 1, "\x1d\x18\x98\x9d", 4);
	vt100_set_key (vt, "\x10", 1, "\x1d\x19\x99\x9d", 4);
	vt100_set_key (vt, "\x11", 1, "\x1d\x10\x90\x9d", 4);
	vt100_set_key (vt, "\x12", 1, "\x1d\x13\x93\x9d", 4);
	vt100_set_key (vt, "\x13", 1, "\x1d\x1f\x9f\x9d", 4);
	vt100_set_key (vt, "\x14", 1, "\x1d\x14\x94\x9d", 4);
	vt100_set_key (vt, "\x15", 1, "\x1d\x16\x96\x9d", 4);
	vt100_set_key (vt, "\x16", 1, "\x1d\x2f\xaf\x9d", 4);
	vt100_set_key (vt, "\x17", 1, "\x1d\x11\x91\x9d", 4);
	vt100_set_key (vt, "\x18", 1, "\x1d\x2d\xad\x9d", 4);
	vt100_set_key (vt, "\x19", 1, "\x1d\x15\x95\x9d", 4);
	vt100_set_key (vt, "\x1a", 1, "\x1d\x2c\xac\x9d", 4);

	vt100_set_key (vt, "\x08", 1, "\x0e\x8e", 2);
	vt100_set_key (vt, "\x0a", 1, "\x1c\x9c", 2);
	vt100_set_key (vt, "\x1b\x1b", 2, "\x01\x81", 2);

	vt100_set_key (vt, "\x20", 1, "\x39\xb9", 2);
	vt100_set_key (vt, "\x21", 1, "\x2a\x02\x82\xaa", 4);
	vt100_set_key (vt, "\x22", 1, "\x2a\x28\xa8\xaa", 4);
	vt100_set_key (vt, "\x23", 1, "\x2a\x04\x84\xaa", 4);
	vt100_set_key (vt, "\x24", 1, "\x2a\x05\x85\xaa", 4);
	vt100_set_key (vt, "\x25", 1, "\x2a\x06\x86\xaa", 4);
	vt100_set_key (vt, "\x26", 1, "\x2a\x08\x88\xaa", 4);
	vt100_set_key (vt, "\x27", 1, "\x28\xa8", 2);
	vt100_set_key (vt, "\x28", 1, "\x2a\x0a\x8a\xaa", 4);
	vt100_set_key (vt, "\x29", 1, "\x2a\x0b\x8b\xaa", 4);
	vt100_set_key (vt, "\x2a", 1, "\x2a\x09\x89\xaa", 4);
	vt100_set_key (vt, "\x2b", 1, "\x2a\x0d\x8d\xaa", 4);
	vt100_set_key (vt, "\x2c", 1, "\x33\xb3", 2);
	vt100_set_key (vt, "\x2d", 1, "\x0c\x8c", 2);
	vt100_set_key (vt, "\x2e", 1, "\x34\xb4", 2);
	vt100_set_key (vt, "\x2f", 1, "\x35\xb5", 2);

	vt100_set_key (vt, "0", 1, "\x0b\x8b", 2);
	vt100_set_key (vt, "1", 1, "\x02\x82", 2);
	vt100_set_key (vt, "2", 1, "\x03\x83", 2);
	vt100_set_key (vt, "3", 1, "\x04\x84", 2);
	vt100_set_key (vt, "4", 1, "\x05\x85", 2);
	vt100_set_key (vt, "5", 1, "\x06\x86", 2);
	vt100_set_key (vt, "6", 1, "\x07\x87", 2);
	vt100_set_key (vt, "7", 1, "\x08\x88", 2);
	vt100_set_key (vt, "8", 1, "\x09\x89", 2);
	vt100_set_key (vt, "9", 1, "\x0a\x8a", 2);

	vt100_set_key (vt, "\x3a", 1, "\x2a\x27\xa7\xaa", 4);
	vt100_set_key (vt, "\x3b", 1, "\x27\xa7", 2);
	vt100_set_key (vt, "\x3c", 1, "\x2a\x33\xb3\xaa", 4);
	vt100_set_key (vt, "\x3d", 1, "\x0d\x8d", 2);
	vt100_set_key (vt, "\x3e", 1, "\x2a\x34\xb4\xaa", 4);
	vt100_set_key (vt, "\x3f", 1, "\x2a\x35\xb5\xaa", 4);
	vt100_set_key (vt, "\x40", 1, "\x2a\x03\x83\xaa", 4);

	vt100_set_key (vt, "A", 1, "\x2a\x1e\x9e\xaa", 4);
	vt100_set_key (vt, "B", 1, "\x2a\x30\xb0\xaa", 4);
	vt100_set_key (vt, "C", 1, "\x2a\x2e\xae\xaa", 4);
	vt100_set_key (vt, "D", 1, "\x2a\x20\xa0\xaa", 4);
	vt100_set_key (vt, "E", 1, "\x2a\x12\x92\xaa", 4);
	vt100_set_key (vt, "F", 1, "\x2a\x21\xa1\xaa", 4);
	vt100_set_key (vt, "G", 1, "\x2a\x22\xa2\xaa", 4);
	vt100_set_key (vt, "H", 1, "\x2a\x23\xa3\xaa", 4);
	vt100_set_key (vt, "I", 1, "\x2a\x17\x97\xaa", 4);
	vt100_set_key (vt, "J", 1, "\x2a\x24\xa4\xaa", 4);
	vt100_set_key (vt, "K", 1, "\x2a\x25\xa5\xaa", 4);
	vt100_set_key (vt, "L", 1, "\x2a\x26\xa6\xaa", 4);
	vt100_set_key (vt, "M", 1, "\x2a\x32\xb2\xaa", 4);
	vt100_set_key (vt, "N", 1, "\x2a\x31\xb1\xaa", 4);
	vt100_set_key (vt, "O", 1, "\x2a\x18\x98\xaa", 4);
	vt100_set_key (vt, "P", 1, "\x2a\x19\x99\xaa", 4);
	vt100_set_key (vt, "Q", 1, "\x2a\x10\x90\xaa", 4);
	vt100_set_key (vt, "R", 1, "\x2a\x13\x93\xaa", 4);
	vt100_set_key (vt, "S", 1, "\x2a\x1f\x9f\xaa", 4);
	vt100_set_key (vt, "T", 1, "\x2a\x14\x94\xaa", 4);
	vt100_set_key (vt, "U", 1, "\x2a\x16\x96\xaa", 4);
	vt100_set_key (vt, "V", 1, "\x2a\x2f\xaf\xaa", 4);
	vt100_set_key (vt, "W", 1, "\x2a\x11\x91\xaa", 4);
	vt100_set_key (vt, "X", 1, "\x2a\x2d\xad\xaa", 4);
	vt100_set_key (vt, "Y", 1, "\x2a\x15\x95\xaa", 4);
	vt100_set_key (vt, "Z", 1, "\x2a\x2c\xac\xaa", 4);

	vt100_set_key (vt, "\x5b", 1, "\x1a\x9a", 2);
	vt100_set_key (vt, "\x5c", 1, "\x2b\xab", 2);
	vt100_set_key (vt, "\x5d", 1, "\x1b\x9b", 2);
	vt100_set_key (vt, "\x5e", 1, "\x2a\x07\x87\xaa", 4);
	vt100_set_key (vt, "\x5f", 1, "\x2a\x0c\x8c\xaa", 4);
	vt100_set_key (vt, "\x60", 1, "\x29\xa9", 2);

	vt100_set_key (vt, "a", 1, "\x1e\x9e", 2);
	vt100_set_key (vt, "b", 1, "\x30\xb0", 2);
	vt100_set_key (vt, "c", 1, "\x2e\xae", 2);
	vt100_set_key (vt, "d", 1, "\x20\xa0", 2);
	vt100_set_key (vt, "e", 1, "\x12\x92", 2);
	vt100_set_key (vt, "f", 1, "\x21\xa1", 2);
	vt100_set_key (vt, "g", 1, "\x22\xa2", 2);
	vt100_set_key (vt, "h", 1, "\x23\xa3", 2);
	vt100_set_key (vt, "i", 1, "\x17\x97", 2);
	vt100_set_key (vt, "j", 1, "\x24\xa4", 2);
	vt100_set_key (vt, "k", 1, "\x25\xa5", 2);
	vt100_set_key (vt, "l", 1, "\x26\xa6", 2);
	vt100_set_key (vt, "m", 1, "\x32\xb2", 2);
	vt100_set_key (vt, "n", 1, "\x31\xb1", 2);
	vt100_set_key (vt, "o", 1, "\x18\x98", 2);
	vt100_set_key (vt, "p", 1, "\x19\x99", 2);
	vt100_set_key (vt, "q", 1, "\x10\x90", 2);
	vt100_set_key (vt, "r", 1, "\x13\x93", 2);
	vt100_set_key (vt, "s", 1, "\x1f\x9f", 2);
	vt100_set_key (vt, "t", 1, "\x14\x94", 2);
	vt100_set_key (vt, "u", 1, "\x16\x96", 2);
	vt100_set_key (vt, "v", 1, "\x2f\xaf", 2);
	vt100_set_key (vt, "w", 1, "\x11\x91", 2);
	vt100_set_key (vt, "x", 1, "\x2d\xad", 2);
	vt100_set_key (vt, "y", 1, "\x15\x95", 2);
	vt100_set_key (vt, "z", 1, "\x2c\xac", 2);

	vt100_set_key (vt, "\x7b", 1, "\x2a\x1a\x9a\xaa", 4);
	vt100_set_key (vt, "\x7c", 1, "\x2a\x2b\xab\xaa", 4);
	vt100_set_key (vt, "\x7d", 1, "\x2a\x1b\x9b\xaa", 4);
	vt100_set_key (vt, "\x7e", 1, "\x2a\x29\xa9\xaa", 4);
	vt100_set_key (vt, "\x7f", 1, "\x1d\x23\xa3\x9d", 4);

	vt100_set_key (vt, "\xe1", 1, "\x38\x1e\x9e\xb8", 4);
	vt100_set_key (vt, "\xe2", 1, "\x38\x30\xb0\xb8", 4);
	vt100_set_key (vt, "\xe3", 1, "\x38\x2e\xae\xb8", 4);
	vt100_set_key (vt, "\xe4", 1, "\x38\x20\xa0\xb8", 4);
	vt100_set_key (vt, "\xe5", 1, "\x38\x12\x92\xb8", 4);
	vt100_set_key (vt, "\xe6", 1, "\x38\x21\xa1\xb8", 4);
	vt100_set_key (vt, "\xe7", 1, "\x38\x22\xa2\xb8", 4);
	vt100_set_key (vt, "\xe8", 1, "\x38\x23\xa3\xb8", 4);
	vt100_set_key (vt, "\xe9", 1, "\x38\x17\x97\xb8", 4);
	vt100_set_key (vt, "\xea", 1, "\x38\x24\xa4\xb8", 4);
	vt100_set_key (vt, "\xeb", 1, "\x38\x25\xa5\xb8", 4);
	vt100_set_key (vt, "\xec", 1, "\x38\x26\xa6\xb8", 4);
	vt100_set_key (vt, "\xed", 1, "\x38\x32\xb2\xb8", 4);
	vt100_set_key (vt, "\xee", 1, "\x38\x31\xb1\xb8", 4);
	vt100_set_key (vt, "\xef", 1, "\x38\x18\x98\xb8", 4);
	vt100_set_key (vt, "\xf0", 1, "\x38\x19\x99\xb8", 4);
	vt100_set_key (vt, "\xf1", 1, "\x38\x10\x90\xb8", 4);
	vt100_set_key (vt, "\xf2", 1, "\x38\x13\x93\xb8", 4);
	vt100_set_key (vt, "\xf3", 1, "\x38\x1f\x9f\xb8", 4);
	vt100_set_key (vt, "\xf4", 1, "\x38\x14\x94\xb8", 4);
	vt100_set_key (vt, "\xf5", 1, "\x38\x16\x96\xb8", 4);
	vt100_set_key (vt, "\xf6", 1, "\x38\x2f\xaf\xb8", 4);
	vt100_set_key (vt, "\xf7", 1, "\x38\x11\x91\xb8", 4);
	vt100_set_key (vt, "\xf8", 1, "\x38\x2d\xad\xb8", 4);
	vt100_set_key (vt, "\xf9", 1, "\x38\x15\x95\xb8", 4);
	vt100_set_key (vt, "\xfa", 1, "\x38\x2c\xac\xb8", 4);

	vt100_set_key (vt, "\x1b\x4f\x50", 3, "\x3b\xbb", 2); /* F1 */
	vt100_set_key (vt, "\x1b\x4f\x51", 3, "\x3c\xbc", 2); /* F2 */
	vt100_set_key (vt, "\x1b\x4f\x52", 3, "\x3d\xbd", 2); /* F3 */
	vt100_set_key (vt, "\x1b\x4f\x53", 3, "\x3e\xbe", 2); /* F4 */
	vt100_set_key (vt, "\x1b\x5b\x31\x35\x7e", 5, "\x3f\xbf", 2); /* F5 */
	vt100_set_key (vt, "\x1b\x5b\x31\x37\x7e", 5, "\x40\xc0", 2); /* F6 */
	vt100_set_key (vt, "\x1b\x5b\x31\x38\x7e", 5, "\x41\xc1", 2); /* F7 */
	vt100_set_key (vt, "\x1b\x5b\x32\x30\x7e", 5, "\x43\xc3", 2); /* F9 */
	vt100_set_key (vt, "\x1b\x5b\x32\x31\x7e", 5, "\x44\xc4", 2); /* F10 */

	vt100_set_key (vt, "\x1b\x5b\x48", 3, "\x47\xc7", 2); /* Home */
	vt100_set_key (vt, "\x1b\x5b\x41", 3, "\x48\xc8", 2); /* Up */
	vt100_set_key (vt, "\x1b\x5b\x35\x7e", 4, "\x49\xc9", 2); /* PgUp */
	vt100_set_key (vt, "\x1b\x5b\x44", 3, "\x4b\xcb", 2); /* Left */
	vt100_set_key (vt, "\x1b\x5b\x45", 3, "\x4c\xcc", 2); /* KP-5 */
	vt100_set_key (vt, "\x1b\x5b\x43", 3, "\x4d\xcd", 2); /* Right */
	vt100_set_key (vt, "\x1b\x5b\x46", 3, "\x4f\xcf", 2); /* End */
	vt100_set_key (vt, "\x1b\x5b\x42", 3, "\x50\xd0", 2); /* Down */
	vt100_set_key (vt, "\x1b\x5b\x36\x7e", 4, "\x51\xd1", 2); /* PgDn */
	vt100_set_key (vt, "\x1b\x5b\x32\x7e", 4, "\x52\xd2", 2); /* Ins */
	vt100_set_key (vt, "\x1b\x5b\x33\x7e", 4, "\x53\xd3", 2); /* Del */

	if (ini != NULL) {
		const char *str;
		ini_val_t  *val;

		val = ini_sct_find_val (ini, "keymap");

		while (val != NULL) {
			str = ini_val_get_str (val);
			if (str != NULL) {
				if (vt100_set_key_str (vt, str)) {
					fprintf (stderr, "vt100: bad keymap (%s)\n", str);
				}
			}

			val = ini_val_find_next (val, "keymap");
		}
	}
}

terminal_t *vt100_new (ini_sct_t *ini, int inp, int out)
{
	vt100_t *vt;

	vt = (vt100_t *) malloc (sizeof (vt100_t));
	if (vt == NULL) {
		return (NULL);
	}

	vt100_init (vt, ini, inp, out);

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

#ifdef HAVE_SYS_POLL_H

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

#else

static
int vt100_readable (vt100_t *vt, int t)
{
	return (0);
}

#endif

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

void vt100_set_mode (vt100_t *vt, unsigned m, unsigned w, unsigned h)
{
}

static
void vt100_set_pos_scn (vt100_t *vt, unsigned x, unsigned y)
{
	unsigned      i;
	unsigned char buf[256];

	if ((x == vt->scn_x) && (y == vt->scn_y)) {
		return;
	}

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

void vt100_set_crs (vt100_t *vt, unsigned y1, unsigned y2, int show)
{
	vt->crs_on = (show != 0);
	vt->crs_y1 = y1;
	vt->crs_y2 = y2;
}

void vt100_set_pos (vt100_t *vt, unsigned x, unsigned y)
{
	vt->crs_x = x;
	vt->crs_y = y;
}

void vt100_set_chr (vt100_t *vt, unsigned x, unsigned y, unsigned char c)
{
	vt100_set_pos_scn (vt, x, y);

	c = chrmap[c & 0xff];

	vt100_write (vt, &c, 1);

	vt->scn_x += 1;
}

void vt100_check (vt100_t *vt)
{
	unsigned      i, j, n;
	unsigned char buf[8];
	ssize_t       r;
	vt100_keymap_t *key;

	if (vt->crs_on) {
		vt100_set_pos_scn (vt, vt->crs_x, vt->crs_y);
	}
	else {
		vt100_set_pos_scn (vt, 0, 0);
	}

	if (!vt100_readable (vt, 0)) {
		return;
	}

	r = read (vt->fd_inp, buf, 8);
	if (r <= 0) {
		return;
	}

	n = (unsigned) r;

	if ((n == 1) && (buf[0] == 0)) {
		trm_set_msg (&vt->trm, "emu.stop", "1");
		return;
	}
	else if ((n == 1) && (buf[0] == 0x80)) {
		trm_set_msg (&vt->trm, "video.screenshot", "");
		return;
	}
	else if ((n == 1) && (buf[0] == 0xe0)) {
		trm_set_msg (&vt->trm, "emu.exit", "1");
		return;
	}

	i = 0;
	while (i < n) {
		key = vt100_get_key (vt, buf + i, n - i);

		if (key == NULL) {
			fprintf (stderr, "vt100: unknown key %u %02X %02X %02X\n", n, buf[0], buf[1], buf[2]);
			fprintf (stderr, "vt100: %u keys\n", vt->key_cnt);
			return;
		}

		if (vt->trm.set_key != NULL) {
			for (j = 0; j < key->seq_cnt; j++) {
				vt->trm.set_key (vt->trm.key_ext, key->seq[j]);
			}
		}

		i += key->key_cnt;
	}
}
