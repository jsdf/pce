/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/drivers/pri/pri.c                                        *
 * Created:     2012-01-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2019 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pri.h"


/*****************************************************************************
 * Clear all bits in the range i1 < i <= i2 in buf
 *****************************************************************************/
static
void pri_clear_bits (unsigned char *buf, unsigned long i1, unsigned long i2)
{
	unsigned char m1, m2;

	if (i2 < i1) {
		return;
	}

	m1 = ~(0xff >> (i1 & 7));
	m2 = ~(0xff << (~i2 & 7));

	i1 >>= 3;
	i2 >>= 3;

	if (i1 == i2) {
		buf[i1] &= m1 | m2;
	}
	else {
		buf[i1++] &= m1;

		while (i1 < i2) {
			buf[i1++] = 0;
		}

		buf[i2] &= m2;
	}
}

pri_evt_t *pri_evt_new (unsigned long type, unsigned long pos, unsigned long val)
{
	pri_evt_t *evt;

	if ((evt = malloc (sizeof (pri_evt_t))) == NULL) {
		return (NULL);
	}

	evt->next = NULL;
	evt->type = type;
	evt->pos = pos;
	evt->val = val;

	return (evt);
}

void pri_evt_del (pri_evt_t *evt)
{
	free (evt);
}

/*
 * Get the next event of type <type> after this one.
 */
pri_evt_t *pri_evt_next (pri_evt_t *evt, unsigned long type)
{
	if (evt != NULL) {
		evt = evt->next;
	}

	while (evt != NULL) {
		if ((type == PRI_EVENT_ALL) || (evt->type == type)) {
			return (evt);
		}

		evt = evt->next;
	}

	return (NULL);
}

/*
 * Convert the clock rate multiplication factor in an event of
 * type PRI_EVENT_CLOCK to a real clock rate.
 */
unsigned long pri_evt_get_clock (pri_evt_t *evt, unsigned long base)
{
	unsigned long clk;

	if ((evt == NULL) || (evt->type != PRI_EVENT_CLOCK)) {
		return (base);
	}

	if (evt->val == 0) {
		clk = base;
	}
	else {
		clk = ((unsigned long long) evt->val * base + 32767) / 65536;
	}

	return (clk);
}

/*****************************************************************************
 * Create a new track
 *
 * @param  size   The track size in bits
 * @param  clock  The bit rate in Herz
 *
 * @return The new track or NULL on error
 *****************************************************************************/
pri_trk_t *pri_trk_new (unsigned long size, unsigned long clock)
{
	pri_trk_t *trk;

	trk = malloc (sizeof (pri_trk_t));

	if (trk == NULL) {
		return (NULL);
	}

	trk->clock = clock;
	trk->size = size;
	trk->data = NULL;

	if (size > 0) {
		trk->data = malloc ((size + 7) / 8);

		if (trk->data == NULL) {
			free (trk);
			return (NULL);
		}
	}

	trk->evt = NULL;

	trk->idx = 0;
	trk->cur_evt = NULL;
	trk->wrap = 0;

	return (trk);
}

/*****************************************************************************
 * Delete a track
 *****************************************************************************/
void pri_trk_del (pri_trk_t *trk)
{
	pri_evt_t *tmp;

	if (trk != NULL) {
		while (trk->evt != NULL) {
			tmp = trk->evt;
			trk->evt = tmp->next;
			free (tmp);
		}

		free (trk->data);
		free (trk);
	}
}

/*****************************************************************************
 * Create an exact copy of a track
 *****************************************************************************/
pri_trk_t *pri_trk_clone (const pri_trk_t *trk)
{
	pri_trk_t *ret;
	pri_evt_t *evt;

	ret = pri_trk_new (trk->size, trk->clock);

	if (ret == NULL) {
		return (NULL);
	}

	if (trk->size > 0) {
		memcpy (ret->data, trk->data, (trk->size + 7) / 8);
	}

	ret->idx = trk->idx;
	ret->cur_evt = NULL;
	ret->wrap = trk->wrap;

	evt = trk->evt;

	while (evt != NULL) {
		if (pri_trk_evt_add (ret, evt->type, evt->pos, evt->val) == NULL) {
			pri_trk_del (ret);
			return (NULL);
		}

		evt = evt->next;
	}

	return (ret);
}

void pri_trk_evt_ins (pri_trk_t *trk, pri_evt_t *evt)
{
	pri_evt_t *tmp;

	evt->next = NULL;

	if (trk->evt == NULL) {
		trk->evt = evt;
	}
	else if (evt->pos < trk->evt->pos) {
		evt->next = trk->evt;
		trk->evt = evt;
	}
	else {
		tmp = trk->evt;

		while ((tmp->next != NULL) && (evt->pos >= tmp->next->pos)) {
			tmp = tmp->next;
		}

		evt->next = tmp->next;
		tmp->next = evt;
	}
}

int pri_trk_evt_rmv (pri_trk_t *trk, const pri_evt_t *evt)
{
	pri_evt_t *tmp;

	if (trk->evt == NULL) {
		return (1);
	}

	if (trk->evt == evt) {
		trk->evt = trk->evt->next;
		return (0);
	}

	tmp = trk->evt;

	while ((tmp->next != NULL) && (tmp->next != evt)) {
		tmp = tmp->next;
	}

	if (tmp->next == evt) {
		tmp->next = tmp->next->next;
		return (0);
	}

	return (1);
}

pri_evt_t *pri_trk_evt_add (pri_trk_t *trk, unsigned long type, unsigned long pos, unsigned long val)
{
	pri_evt_t *evt;

	if ((evt = pri_evt_new (type, pos, val)) == NULL) {
		return (NULL);
	}

	pri_trk_evt_ins (trk, evt);

	return (evt);
}

/*
 * Get the first event of type <type>, after skipping idx events of the same
 * type.
 */
pri_evt_t *pri_trk_evt_get_idx (pri_trk_t *trk, unsigned long type, unsigned long idx)
{
	pri_evt_t *evt;

	evt = trk->evt;

	while (evt != NULL) {
		if ((type == PRI_EVENT_ALL) || (evt->type == type)) {
			if (idx == 0) {
				return (evt);
			}

			idx -= 1;
		}

		evt = evt->next;
	}

	return (NULL);
}

/*
 * Get the first event of type <type> at <pos>.
 */
pri_evt_t *pri_trk_evt_get_pos (pri_trk_t *trk, unsigned long type, unsigned long pos)
{
	pri_evt_t *evt;

	evt = trk->evt;

	while (evt != NULL) {
		if ((type == PRI_EVENT_ALL) || (evt->type == type)) {
			if (evt->pos == pos) {
				return (evt);
			}

			if (evt->pos > pos) {
				return (NULL);
			}
		}

		evt = evt->next;
	}

	return (NULL);
}

/*
 * Get the first event of type <type> at or after <pos>.
 */
pri_evt_t *pri_trk_evt_get_after (pri_trk_t *trk, unsigned long type, unsigned long pos)
{
	pri_evt_t *evt;

	evt = trk->evt;

	while (evt != NULL) {
		if ((type == PRI_EVENT_ALL) || (evt->type == type)) {
			if (evt->pos >= pos) {
				return (evt);
			}
		}

		evt = evt->next;
	}

	return (NULL);
}

/*
 * Get the last event of type <type> at or before <pos>.
 */
pri_evt_t *pri_trk_evt_get_before (pri_trk_t *trk, unsigned long type, unsigned long pos)
{
	pri_evt_t *evt, *ret;

	evt = trk->evt;
	ret = NULL;

	while (evt != NULL) {
		if ((type == PRI_EVENT_ALL) || (evt->type == type)) {
			if (evt->pos > pos) {
				return (ret);
			}

			ret = evt;
		}

		evt = evt->next;
	}

	return (NULL);
}

int pri_trk_evt_del (pri_trk_t *trk, pri_evt_t *evt)
{
	pri_evt_t *tmp, *tmp2;

	if (trk->evt == NULL) {
		return (1);
	}

	if (trk->evt == evt) {
		tmp = trk->evt;
		trk->evt = trk->evt->next;
		pri_evt_del (tmp);
		return (0);
	}

	tmp = trk->evt;

	while ((tmp->next != NULL) && (tmp->next != evt)) {
		tmp = tmp->next;
	}

	if (tmp->next == evt) {
		tmp2 = tmp->next;
		tmp->next = tmp2->next;
		pri_evt_del (tmp2);
		return (0);
	}

	return (1);
}

void pri_trk_evt_del_all (pri_trk_t *trk, unsigned long type)
{
	pri_evt_t *evt, *dst1, *dst2, *tmp;

	evt = trk->evt;

	dst1 = NULL;
	dst2 = NULL;

	while (evt != NULL) {
		if ((type == PRI_EVENT_ALL) || (evt->type == type)) {
			tmp = evt;
			evt = evt->next;
			free (tmp);

		}
		else {
			if (dst1 == NULL) {
				dst1 = evt;
			}
			else {
				dst2->next = evt;
			}

			dst2 = evt;

			evt = evt->next;
		}
	}

	if (dst2 != NULL) {
		dst2->next = NULL;
	}

	trk->evt = dst1;
	trk->cur_evt = NULL;
}

unsigned pri_trk_evt_count (const pri_trk_t *trk, unsigned long type)
{
	unsigned cnt;

	pri_evt_t *evt;

	cnt = 0;
	evt = trk->evt;

	while (evt != NULL) {
		if ((type == PRI_EVENT_ALL) || (evt->type == type)) {
			cnt += 1;
		}

		evt = evt->next;
	}

	return (cnt);
}

/*
 * Add ofs to all event positions, wrapping around if necessary
 */
static
void pri_trk_evt_shift (pri_trk_t *trk, unsigned long ofs)
{
	pri_evt_t *evt, *tmp;

	evt = trk->evt;
	trk->evt = NULL;

	while (evt != NULL) {
		if (evt->pos < trk->size) {
			evt->pos += ofs;

			while (evt->pos >= trk->size) {
				evt->pos -= trk->size;
			}
		}

		tmp = evt;
		evt = evt->next;

		pri_trk_evt_ins (trk, tmp);
	}
}

/*****************************************************************************
 * Clear a track
 *
 * @param val  An 8 bit value that is used to initialize the track
 *****************************************************************************/
void pri_trk_clear (pri_trk_t *trk, unsigned val)
{
	if (trk->size > 0) {
		memset (trk->data, val, (trk->size + 7) / 8);
		pri_clear_bits (trk->data, trk->size, (trk->size - 1) | 7);
	}
}

/*****************************************************************************
 * Clear a track
 *
 * @param val  A 16 bit value that is used to initialize the track
 *****************************************************************************/
void pri_trk_clear_16 (pri_trk_t *trk, unsigned val)
{
	unsigned long i, n;
	unsigned char buf[2];

	if (trk->size == 0) {
		return;
	}

	buf[0] = (val >> 8) & 0xff;
	buf[1] = val & 0xff;

	n = (trk->size + 7) / 8;

	for (i = 0; i < n; i++) {
		trk->data[i] = buf[i & 1];
	}

	pri_clear_bits (trk->data, trk->size, (trk->size - 1) | 7);
}

/*****************************************************************************
 * Clear the remaining bits of a track
 *****************************************************************************/
void pri_trk_clear_slack (pri_trk_t *trk)
{
	if (trk->size & 7) {
		trk->data[trk->size / 8] &= 0xff << (8 - (trk->size & 7));
	}
}

/*****************************************************************************
 * Set the bit rate
 *
 * @param clock  The bit rate in Herz
 *****************************************************************************/
void pri_trk_set_clock (pri_trk_t *trk, unsigned long clock)
{
	trk->clock = clock;
}

/*****************************************************************************
 * Get the bit rate
 *
 * @return The bit rate in Herz
 *****************************************************************************/
unsigned long pri_trk_get_clock (const pri_trk_t *trk)
{
	return (trk->clock);
}

/*****************************************************************************
 * Get the track size
 *
 * @return The track size in bits
 *****************************************************************************/
unsigned long pri_trk_get_size (const pri_trk_t *trk)
{
	return (trk->size);
}

/*****************************************************************************
 * Set the track size
 *
 * @param  size  The new track size in bits
 *
 * If the new size is greater than the old size, the new bits are
 * initialized to 0.
 *****************************************************************************/
int pri_trk_set_size (pri_trk_t *trk, unsigned long size)
{
	unsigned long i1;
	unsigned char *tmp;

	if (trk->size == size) {
		return (0);
	}

	trk->idx = 0;
	trk->cur_evt = trk->evt;
	trk->wrap = 0;

	if (size == 0) {
		free (trk->data);
		trk->size = 0;
		trk->data = NULL;
		return (0);
	}

	if ((tmp = realloc (trk->data, (size + 7) / 8)) == NULL) {
		return (1);
	}

	i1 = (size < trk->size) ? size : trk->size;
	pri_clear_bits (tmp, i1, (size - 1) | 7);

	trk->size = size;
	trk->data = tmp;

	return (0);
}

/*****************************************************************************
 * Get the track position
 *
 * @return  The current track position
 *****************************************************************************/
unsigned long pri_trk_get_pos (const pri_trk_t *trk)
{
	return (trk->idx);
}

/*****************************************************************************
 * Set the track position
 *
 * @param  pos  The new track position in bits
 *****************************************************************************/
void pri_trk_set_pos (pri_trk_t *trk, unsigned long pos)
{
	if (trk->size == 0) {
		return;
	}

	trk->idx = pos % trk->size;
	trk->wrap = 0;

	trk->cur_evt = trk->evt;

	while ((trk->cur_evt != NULL) && (trk->cur_evt->pos < trk->idx)) {
		trk->cur_evt = trk->cur_evt->next;
	}
}

/*****************************************************************************
 * Get bits from the current position
 *
 * @param  val  The bits, in the low order cnt bits
 * @param  cnt  The number of bits (cnt <= 32)
 *
 * @return Non-zero if the current position wrapped around
 *****************************************************************************/
int pri_trk_get_bits (pri_trk_t *trk, unsigned long *val, unsigned cnt)
{
	unsigned long       v;
	unsigned char       m;
	const unsigned char *p;

	if (trk->size == 0) {
		*val = 0;
		return (1);
	}

	p = trk->data + (trk->idx / 8);
	m = 0x80 >> (trk->idx & 7);
	v = 0;

	while (cnt > 0) {
		v = (v << 1) | ((*p & m) != 0);

		m >>= 1;
		trk->idx += 1;

		if (trk->idx >= trk->size) {
			p = trk->data;
			m = 0x80;
			trk->idx = 0;
			trk->wrap = 1;
		}
		else if (m == 0) {
			p += 1;
			m = 0x80;
		}

		cnt -= 1;
	}

	*val = v;

	return (trk->wrap);
}

/*****************************************************************************
 * Set bits at the current position
 *
 * @param  val  The bits, in the low order cnt bits
 * @param  cnt  The number of bits (cnt <= 32)
 *
 * @return Non-zero if the current position wrapped around
 *****************************************************************************/
int pri_trk_set_bits (pri_trk_t *trk, unsigned long val, unsigned cnt)
{
	unsigned char m;
	unsigned char *p;

	if (trk->size == 0) {
		return (1);
	}

	p = trk->data + (trk->idx / 8);
	m = 0x80 >> (trk->idx & 7);

	while (cnt > 0) {
		cnt -= 1;

		if ((val >> cnt) & 1) {
			*p |= m;
		}
		else {
			*p &= ~m;
		}

		m >>= 1;
		trk->idx += 1;

		if (trk->idx >= trk->size) {
			p = trk->data;
			m = 0x80;
			trk->idx = 0;
			trk->wrap = 1;
		}
		else if (m == 0) {
			p += 1;
			m = 0x80;
		}
	}

	return (trk->wrap);
}

/*****************************************************************************
 * Get an event from the current position
 *
 * @retval  type  The event type
 * @retval  val   The event value
 *
 * @return Non-zero if there are no more events at the current position
 *****************************************************************************/
int pri_trk_get_event (pri_trk_t *trk, unsigned long *type, unsigned long *val)
{
	pri_evt_t *evt;

	evt = trk->cur_evt;

	while ((evt != NULL) && (evt->pos < trk->idx)) {
		evt = evt->next;
	}

	if ((evt != NULL) && (evt->pos == trk->idx)) {
		*type = evt->type;
		*val = evt->val;

		trk->cur_evt = evt->next;

		return (0);
	}

	trk->cur_evt = evt;

	return (1);
}

int pri_trk_rotate (pri_trk_t *trk, unsigned long idx)
{
	unsigned long i;
	unsigned char dm, sm;
	unsigned char *sp, *dp, *tmp;

	if (idx >= trk->size) {
		return (1);
	}

	if (idx == 0) {
		return (0);
	}

	tmp = malloc ((trk->size + 7) / 8);

	if (tmp == NULL) {
		return (1);
	}

	memset (tmp, 0, (trk->size + 7) / 8);

	sp = trk->data + (idx >> 3);
	sm = 0x80 >> (idx & 7);

	dp = tmp;
	dm = 0x80;

	for (i = 0; i < trk->size; i++) {
		if (*sp & sm) {
			*dp |= dm;
		}

		dm >>= 1;

		if (dm == 0) {
			dp += 1;
			dm = 0x80;
		}

		idx += 1;

		if (idx >= trk->size) {
			idx = 0;
			sp = trk->data;
			sm = 0x80;
		}
		else {
			sm >>= 1;

			if (sm == 0) {
				sp += 1;
				sm = 0x80;
			}
		}

	}

	free (trk->data);
	trk->data = tmp;

	pri_trk_evt_shift (trk, trk->size - idx);

	return (0);
}

int pri_trk_get_weak_mask (pri_trk_t *trk, unsigned char **buf, unsigned long *cnt)
{
	unsigned long pos, val;
	unsigned char *ptr;
	pri_evt_t     *evt;

	*cnt = (trk->size + 7) / 8;

	if ((*buf = malloc (*cnt)) == NULL) {
		return (1);
	}

	memset (*buf, 0, *cnt);

	ptr = *buf;

	evt = pri_trk_evt_get_idx (trk, PRI_EVENT_WEAK, 0);

	while (evt != NULL) {
		pos = evt->pos;
		val = evt->val;

		while ((val != 0) && (pos < trk->size)) {
			if (val & 0x80000000) {
				ptr[pos >> 3] |= 0x80 >> (pos & 7);
			}

			pos += 1;
			val = (val << 1) & 0xffffffff;
		}

		evt = pri_evt_next (evt, PRI_EVENT_WEAK);
	}

	return (0);
}

int pri_trk_set_weak_mask (pri_trk_t *trk, const void *buf, unsigned long cnt)
{
	unsigned long       i, n;
	unsigned long       val;
	const unsigned char *ptr;

	n = pri_trk_get_size (trk);

	if ((8 * cnt) < n) {
		n = 8 * cnt;
	}

	pri_trk_evt_del_all (trk, PRI_EVENT_WEAK);

	ptr = buf;
	val = 0;

	for (i = 0; i < n; i++) {
		val = (val << 1) | ((ptr[i >> 3] >> (~i & 7)) & 1);

		if (val & 0x80000000) {
			pri_trk_evt_add (trk, PRI_EVENT_WEAK, i - 31, val);
			val = 0;
		}
	}

	while (val != 0) {
		val = val << 1;

		if (val & 0x80000000) {
			pri_trk_evt_add (trk, PRI_EVENT_WEAK, i - 31, val);
			val = 0;
		}

		i += 1;
	}

	return (0);
}


pri_cyl_t *pri_cyl_new (void)
{
	pri_cyl_t *cyl;

	cyl = malloc (sizeof (pri_cyl_t));

	if (cyl == NULL) {
		return (NULL);
	}

	cyl->trk_cnt = 0;
	cyl->trk = NULL;

	return (cyl);
}

void pri_cyl_del (pri_cyl_t *cyl)
{
	unsigned long i;

	if (cyl != NULL) {
		for (i = 0; i < cyl->trk_cnt; i++) {
			pri_trk_del (cyl->trk[i]);
		}

		free (cyl->trk);
		free (cyl);
	}
}

pri_cyl_t *pri_cyl_clone (const pri_cyl_t *cyl)
{
	unsigned long i;
	pri_trk_t     *trk;
	pri_cyl_t     *ret;

	if ((ret = pri_cyl_new()) == NULL) {
		return (NULL);
	}

	for (i = 0; i < cyl->trk_cnt; i++) {
		if (cyl->trk[i] == NULL) {
			continue;
		}

		if ((trk = pri_trk_clone (cyl->trk[i])) == NULL) {
			pri_cyl_del (ret);
			return (NULL);
		}

		if (pri_cyl_set_track (ret, trk, i)) {
			pri_cyl_del (ret);
			return (NULL);
		}
	}

	return (ret);
}

unsigned long pri_cyl_get_trk_cnt (const pri_cyl_t *cyl)
{
	return (cyl->trk_cnt);
}

pri_trk_t *pri_cyl_get_track (pri_cyl_t *cyl, unsigned long h, int alloc)
{
	pri_trk_t *trk;

	if ((h < cyl->trk_cnt) && (cyl->trk[h] != NULL)) {
		return (cyl->trk[h]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	trk = pri_trk_new (0, 0);

	if (trk == NULL) {
		return (NULL);
	}

	if (pri_cyl_set_track (cyl, trk, h)) {
		pri_trk_del (trk);
		return (NULL);
	}

	return (trk);
}

int pri_cyl_set_track (pri_cyl_t *cyl, pri_trk_t *trk, unsigned long h)
{
	unsigned long i;
	pri_trk_t    **tmp;

	if (h < cyl->trk_cnt) {
		pri_trk_del (cyl->trk[h]);

		cyl->trk[h] = trk;

		return (0);
	}

	tmp = realloc (cyl->trk, (h + 1) * sizeof (pri_trk_t *));

	if (tmp == NULL) {
		return (1);
	}

	for (i = cyl->trk_cnt; i < h; i++) {
		tmp[i] = NULL;
	}

	tmp[h] = trk;

	cyl->trk = tmp;
	cyl->trk_cnt = h + 1;

	return (0);
}

int pri_cyl_add_track (pri_cyl_t *cyl, pri_trk_t *trk)
{
	return (pri_cyl_set_track (cyl, trk, cyl->trk_cnt));
}

int pri_cyl_del_track (pri_cyl_t *cyl, unsigned long h)
{
	if ((h >= cyl->trk_cnt) || (cyl->trk[h] == NULL)) {
		return (1);
	}

	pri_trk_del (cyl->trk[h]);

	cyl->trk[h] = NULL;

	while ((cyl->trk_cnt > 0) && (cyl->trk[cyl->trk_cnt - 1] == NULL)) {
		cyl->trk_cnt -= 1;
	}

	return (0);
}


pri_img_t *pri_img_new (void)
{
	pri_img_t *img;

	img = malloc (sizeof (pri_img_t));

	if (img == NULL) {
		return (NULL);
	}

	img->cyl_cnt = 0;
	img->cyl = NULL;

	img->comment_size = 0;
	img->comment = NULL;

	img->readonly = 0;

	img->woz_track_sync = 0;
	img->woz_cleaned = 0;

	return (img);
}

void pri_img_del (pri_img_t *img)
{
	unsigned long i;

	if (img != NULL) {
		for (i = 0; i < img->cyl_cnt; i++) {
			pri_cyl_del (img->cyl[i]);
		}

		free (img->comment);
		free (img->cyl);
		free (img);
	}
}

unsigned long pri_img_get_cyl_cnt (const pri_img_t *img)
{
	return (img->cyl_cnt);
}

unsigned long pri_img_get_trk_cnt (const pri_img_t *img, unsigned long c)
{
	if ((c >= img->cyl_cnt) || (img->cyl[c] == NULL)) {
		return (0);
	}

	return (img->cyl[c]->trk_cnt);
}

static
void pri_img_fix_cyl (pri_img_t *img)
{
	while ((img->cyl_cnt > 0) && (img->cyl[img->cyl_cnt - 1] == NULL)) {
		img->cyl_cnt -= 1;
	}
}

pri_cyl_t *pri_img_get_cylinder (pri_img_t *img, unsigned long c, int alloc)
{
	pri_cyl_t *cyl;

	if ((c < img->cyl_cnt) && (img->cyl[c] != NULL)) {
		return (img->cyl[c]);
	}

	if (alloc == 0) {
		return (NULL);
	}

	cyl = pri_cyl_new();

	if (cyl == NULL) {
		return (NULL);
	}

	if (pri_img_set_cylinder (img, cyl, c)) {
		pri_cyl_del (cyl);
		return (NULL);
	}

	return (cyl);
}

pri_cyl_t *pri_img_rmv_cylinder (pri_img_t *img, unsigned long c)
{
	pri_cyl_t *cyl;

	if ((c >= img->cyl_cnt) || (img->cyl[c] == NULL)) {
		return (NULL);
	}

	cyl = img->cyl[c];

	img->cyl[c] = NULL;

	pri_img_fix_cyl (img);

	return (cyl);
}

int pri_img_set_cylinder (pri_img_t *img, pri_cyl_t *cyl, unsigned long c)
{
	unsigned long i;
	pri_cyl_t    **tmp;

	if (c < img->cyl_cnt) {
		pri_cyl_del (img->cyl[c]);
		img->cyl[c] = cyl;
		pri_img_fix_cyl (img);

		return (0);
	}

	tmp = realloc (img->cyl, (c + 1) * sizeof (pri_cyl_t *));

	if (tmp == NULL) {
		return (1);
	}

	for (i = img->cyl_cnt; i < c; i++) {
		tmp[i] = NULL;
	}

	tmp[c] = cyl;

	img->cyl = tmp;
	img->cyl_cnt = c + 1;

	pri_img_fix_cyl (img);

	return (0);
}

int pri_img_add_cylinder (pri_img_t *img, pri_cyl_t *cyl)
{
	return (pri_img_set_cylinder (img, cyl, img->cyl_cnt));
}

int pri_img_del_cylinder (pri_img_t *img, unsigned long c)
{
	if ((c >= img->cyl_cnt) || (img->cyl[c] == NULL)) {
		return (1);
	}

	pri_cyl_del (img->cyl[c]);

	img->cyl[c] = NULL;

	pri_img_fix_cyl (img);

	return (0);
}

pri_trk_t *pri_img_get_track_const (const pri_img_t *img, unsigned long c, unsigned long h)
{
	pri_cyl_t *cyl;

	if ((c >= img->cyl_cnt) || (img->cyl[c] == NULL)) {
		return (NULL);
	}

	cyl = img->cyl[c];

	if (h >= cyl->trk_cnt) {
		return (NULL);
	}

	return (cyl->trk[h]);
}

pri_trk_t *pri_img_get_track (pri_img_t *img, unsigned long c, unsigned long h, int alloc)
{
	pri_cyl_t *cyl;
	pri_trk_t *trk;

	cyl = pri_img_get_cylinder (img, c, alloc);

	if (cyl == NULL) {
		return (NULL);
	}

	trk = pri_cyl_get_track (cyl, h, alloc);

	if (trk == NULL) {
		return (NULL);
	}

	return (trk);
}

int pri_img_set_track (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h)
{
	pri_cyl_t *cyl;

	cyl = pri_img_get_cylinder (img, c, 1);

	if (cyl == NULL) {
		return (1);
	}

	if (pri_cyl_set_track (cyl, trk, h)) {
		return (1);
	}

	return (0);
}

int pri_img_del_track (pri_img_t *img, unsigned long c, unsigned long h)
{
	if ((c >= img->cyl_cnt) || (img->cyl[c] == NULL)) {
		return (1);
	}

	if (pri_cyl_del_track (img->cyl[c], h)) {
		return (1);
	}

	return (0);
}

int pri_img_add_comment (pri_img_t *img, const unsigned char *buf, unsigned cnt)
{
	unsigned char *tmp;

	tmp = realloc (img->comment, img->comment_size + cnt);

	if (tmp == NULL) {
		return (1);
	}

	memcpy (tmp + img->comment_size, buf, cnt);

	img->comment_size += cnt;
	img->comment = tmp;

	return (0);
}

int pri_img_set_comment (pri_img_t *img, const unsigned char *buf, unsigned cnt)
{
	free (img->comment);

	img->comment_size = 0;
	img->comment = NULL;

	if ((buf == NULL) || (cnt == 0)) {
		return (0);
	}

	if (pri_img_add_comment (img, buf, cnt)) {
		return (1);
	}

	return (0);
}
