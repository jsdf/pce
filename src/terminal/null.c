/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/terminal/null.c                                        *
 * Created:       2003-10-18 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-08-01 by Hampa Hug <hampa@hampa.ch>                   *
 * Copyright:     (C) 2003-2004 Hampa Hug <hampa@hampa.ch>                   *
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


#include <stdlib.h>

#include <terminal/terminal.h>
#include <terminal/null.h>


void null_init (null_t *vt, ini_sct_t *ini)
{
	trm_init (&vt->trm);

	vt->trm.del = (trm_del_f) &null_del;
	vt->trm.set_mode = (trm_set_mode_f) &null_set_mode;
	vt->trm.set_size = (trm_set_size_f) &null_set_size;
	vt->trm.set_col = (trm_set_col_f) &null_set_col;
	vt->trm.set_crs = (trm_set_crs_f) &null_set_crs;
	vt->trm.set_pos = (trm_set_pos_f) &null_set_pos;
	vt->trm.set_chr = (trm_set_chr_f) &null_set_chr;
	vt->trm.set_pxl = (trm_set_pxl_f) &null_set_pxl;
	vt->trm.set_rct = (trm_set_rct_f) &null_set_rct;
	vt->trm.check = (trm_check_f) &null_check;
}

terminal_t *null_new (ini_sct_t *ini)
{
	null_t *vt;

	vt = (null_t *) malloc (sizeof (null_t));
	if (vt == NULL) {
		return (NULL);
	}

	null_init (vt, ini);

	return (&vt->trm);
}

void null_free (null_t *vt)
{
}

void null_del (null_t *vt)
{
	if (vt != NULL) {
		null_free (vt);
		free (vt);
	}
}

void null_set_mode (null_t *vt, unsigned m, unsigned w, unsigned h)
{
}

void null_set_size (null_t *vt, unsigned w, unsigned h)
{
}

void null_set_col (null_t *vt, unsigned fg, unsigned bg)
{
}

void null_set_crs (null_t *vt, unsigned y1, unsigned y2, int show)
{
}

void null_set_pos (null_t *vt, unsigned x, unsigned y)
{
}

void null_set_chr (null_t *vt, unsigned x, unsigned y, unsigned char c)
{
}

void null_set_pxl (null_t *vt, unsigned x, unsigned y)
{
}

void null_set_rct (null_t *vt, unsigned x, unsigned y, unsigned w, unsigned h)
{
}

void null_check (null_t *vt)
{
}
