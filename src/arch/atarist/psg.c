/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/psg.c                                       *
 * Created:     2013-06-06 by Hampa Hug <hampa@hampa.ch>                     *
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
#include "psg.h"

#include <stdlib.h>


void st_psg_init (st_psg_t *psg)
{
	unsigned i;

	for (i = 0; i < 16; i++) {
		psg->reg[i] = 0;
	}

	psg->seta_ext = NULL;
	psg->seta_fct = NULL;

	psg->setb_ext = NULL;
	psg->setb_fct = NULL;
}

void st_psg_free (st_psg_t *psg)
{
}

void st_psg_set_port_a_fct (st_psg_t *psg, void *ext, void *fct)
{
	psg->seta_ext = ext;
	psg->seta_fct = fct;
}

void st_psg_set_port_b_fct (st_psg_t *psg, void *ext, void *fct)
{
	psg->setb_ext = ext;
	psg->setb_fct = fct;
}

static
void st_psg_set_port_a (st_psg_t *psg, unsigned char val)
{
	if (psg->reg[14] != val) {
		psg->reg[14] = val;

		if (psg->seta_fct != NULL) {
			psg->seta_fct (psg->seta_ext, val);
		}
	}
}

static
void st_psg_set_port_b (st_psg_t *psg, unsigned char val)
{
	if (psg->reg[15] != val) {
		psg->reg[15] = val;

		if (psg->setb_fct != NULL) {
			psg->setb_fct (psg->setb_ext, val);
		}
	}
}

unsigned char st_psg_get_select (st_psg_t *psg)
{
	return (psg->reg_sel);
}

void st_psg_set_select (st_psg_t *psg, unsigned char val)
{
	psg->reg_sel = val;
}

unsigned char st_psg_get_data (st_psg_t *psg)
{
	unsigned char val;

	switch (psg->reg_sel) {
	case 14:
		val = psg->reg[14];
		break;

	case 15:
		val = psg->reg[15];
		break;

	default:
		val = 0;
		break;
	}

	return (val);
}

void st_psg_set_data (st_psg_t *psg, unsigned char val)
{
	switch (psg->reg_sel) {
	case 14:
		st_psg_set_port_a (psg, val);
		break;

	case 15:
		st_psg_set_port_b (psg, val);
		break;
	}
}
