/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/utils/psi/delete.c                                       *
 * Created:     2013-06-09 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2013 Hampa Hug <hampa@hampa.ch>                          *
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
#include "delete.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <drivers/psi/psi.h>


static
int psi_delete_sectors_cb (psi_img_t *img, psi_trk_t *trk,
	unsigned c, unsigned h, void *opaque)
{
	unsigned  i, j, a;
	psi_sct_t *sct, *new, *tmp;

	j = 0;

	for (i = 0; i < trk->sct_cnt; i++) {
		a = 0;

		sct = trk->sct[i];
		new = NULL;

		trk->sct[j] = NULL;

		while (sct != NULL) {
			tmp = sct;
			sct = tmp->next;
			tmp->next = NULL;

			if (psi_sel_match (c, h, tmp->s, i, a)) {
				psi_sct_del (tmp);
				par_cnt += 1;
			}
			else {
				if (new == NULL) {
					trk->sct[j] = tmp;
				}
				else {
					new->next = tmp;
				}

				new = tmp;
			}

			a += 1;
		}

		if (trk->sct[j] != NULL) {
			j += 1;
		}
	}

	trk->sct_cnt = j;

	return (0);
}

int psi_delete_sectors (psi_img_t *img)
{
	int r;

	par_cnt = 0;

	r = psi_for_all_tracks (img, psi_delete_sectors_cb, NULL);

	if (par_verbose) {
		fprintf (stderr, "%s: delete %lu sectors\n", arg0, par_cnt);
	}

	if (r) {
		fprintf (stderr, "%s: deleting failed\n", arg0);
	}

	return (r);
}
