/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/pri/event.c                                        *
 * Created:     2015-02-23 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2015 Hampa Hug <hampa@hampa.ch>                          *
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


#include "main.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/pri/pri.h>


struct pri_event_s {
	unsigned long type;
	unsigned long idx1;
	unsigned long idx2;
	char          all;
};


const char *pri_event_get_name (unsigned long type)
{
	switch (type) {
	case PRI_EVENT_WEAK:
		return ("WEAK");

	case PRI_EVENT_CLOCK:
		return ("CLOCK");
	}

	return ("UNK");
}

int pri_event_get_type (const char *name, unsigned long *type)
{
	if (name == NULL) {
		return (1);
	}

	if (strcasecmp (name, "FUZZY") == 0) {
		*type = PRI_EVENT_WEAK;
	}
	else if (strcasecmp (name, "WEAK") == 0) {
		*type = PRI_EVENT_WEAK;
	}
	else if (strcasecmp (name, "CLOCK") == 0) {
		*type = PRI_EVENT_CLOCK;
	}
	else if (strcasecmp (name, "ALL") == 0) {
		*type = PRI_EVENT_ALL;
	}
	else {
		errno = 0;

		*type = strtoul (name, NULL, 0);

		if (errno) {
			return (1);
		}
	}

	return (0);
}


static
int pri_event_clear_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	pri_trk_evt_del_all (trk, PRI_EVENT_ALL);

	return (0);
}

int pri_event_clear (pri_img_t *img)
{
	return (pri_for_all_tracks (img, pri_event_clear_cb, NULL));
}


static
int pri_event_add_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	struct pri_event_s *par;

	par = opaque;

	if (pri_trk_evt_add (trk, par->type, par->idx1, par->idx2) == NULL) {
		return (1);
	}

	return (0);
}

int pri_event_add (pri_img_t *img, const char *type, const char *pos, const char *val)
{
	struct pri_event_s par;

	if (pri_event_get_type (type, &par.type)) {
		return (1);
	}

	errno = 0;

	par.idx1 = strtoul (pos, NULL, 0);
	par.idx2 = strtoul (val, NULL, 0);

	if (errno) {
		return (1);
	}

	return (pri_for_all_tracks (img, pri_event_add_cb, &par));
}


static
int pri_event_del_idx_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long      i;
	struct pri_event_s *par;
	pri_evt_t          *evt;

	par = opaque;

	if (par->all) {
		pri_trk_evt_del_all (trk, par->type);
	}
	else {
		for (i = par->idx1; i <= par->idx2; i++) {
			evt = pri_trk_evt_get_idx (trk, par->type, par->idx1);

			if (evt != NULL) {
				pri_trk_evt_del (trk, evt);
			}
		}
	}

	return (0);
}

static
int pri_event_del_pos_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	int                del;
	struct pri_event_s *par;
	pri_evt_t          *evt, *tmp, *dst;

	par = opaque;

	evt = trk->evt;
	dst = NULL;
	trk->evt = NULL;

	while (evt != NULL) {
		tmp = evt;
		evt = evt->next;
		tmp->next = NULL;

		if ((par->type != PRI_EVENT_ALL) && (tmp->type != par->type)) {
			del = 0;
		}
		else if (par->all) {
			del = 1;
		}
		else if ((tmp->pos >= par->idx1) && (tmp->pos <= par->idx2)) {
			del = 1;
		}
		else {
			del = 0;
		}

		if (del) {
			pri_evt_del (tmp);
		}
		else {
			if (dst == NULL) {
				trk->evt = tmp;
			}
			else {
				dst->next = tmp;
			}

			dst = tmp;
		}
	}

	return (0);
}

int pri_event_del (pri_img_t *img, const char *type, const char *range)
{
	int                byidx;
	struct pri_event_s par;

	if (pri_event_get_type (type, &par.type)) {
		return (1);
	}

	byidx = 0;

	if (*range == '@') {
		byidx = 1;
		range += 1;
	}

	if (pri_parse_range (range, &par.idx1, &par.idx2, &par.all)) {
		return (1);
	}

	if (byidx) {
		return (pri_for_all_tracks (img, pri_event_del_idx_cb, &par));
	}
	else {
		return (pri_for_all_tracks (img, pri_event_del_pos_cb, &par));
	}
}


static
void pri_print_event (const pri_evt_t *evt, unsigned long c, unsigned long h, unsigned long i)
{
	const char *str;

	str = pri_event_get_name (evt->type);

	printf ("%2lu/%lu %3lu: %08lX %08lX %08lX (%s %lu)\n",
		c, h, i, evt->type, evt->pos, evt->val,
		str, evt->pos
	);
}

static
int pri_event_list_idx_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long      i;
	struct pri_event_s *par;
	pri_evt_t          *evt1, *evt2;

	par = opaque;

	evt2 = trk->evt;

	i = 0;

	while (evt2 != NULL) {
		evt1 = evt2;
		evt2 = evt2->next;

		if (par->type != PRI_EVENT_ALL) {
			if (par->type != evt1->type) {
				continue;
			}
		}

		if (par->all == 0) {
			if ((i < par->idx1) || (i > par->idx2)) {
				i += 1;
				continue;
			}
		}

		pri_print_event (evt1, c, h, i);

		i += 1;
	}

	return (0);
}

static
int pri_event_list_pos_cb (pri_img_t *img, pri_trk_t *trk, unsigned long c, unsigned long h, void *opaque)
{
	unsigned long      i;
	struct pri_event_s *par;
	pri_evt_t          *evt1, *evt2;

	par = opaque;

	i = 0;

	evt2 = trk->evt;

	while (evt2 != NULL) {
		evt1 = evt2;
		evt2 = evt2->next;

		if (par->type != PRI_EVENT_ALL) {
			if (par->type != evt1->type) {
				continue;
			}
		}

		i += 1;

		if (par->all == 0) {
			if ((evt1->pos < par->idx1) || (evt1->pos > par->idx2)) {
				continue;
			}
		}

		pri_print_event (evt1, c, h, i - 1);
	}

	return (0);
}

int pri_event_list (pri_img_t *img, const char *type, const char *range)
{
	int                byidx;
	struct pri_event_s par;

	if (pri_event_get_type (type, &par.type)) {
		return (1);
	}

	byidx = 0;

	if (*range == '@') {
		byidx = 1;
		range += 1;
	}

	if (pri_parse_range (range, &par.idx1, &par.idx2, &par.all)) {
		return (1);
	}

	if (byidx) {
		return (pri_for_all_tracks (img, pri_event_list_idx_cb, &par));
	}
	else {
		return (pri_for_all_tracks (img, pri_event_list_pos_cb, &par));
	}
}
