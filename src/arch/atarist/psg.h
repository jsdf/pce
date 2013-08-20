/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/atarist/psg.h                                       *
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


#ifndef PCE_ATARIST_PSG_H
#define PCE_ATARIST_PSG_H 1


typedef struct {
	unsigned char reg_sel;
	unsigned char reg[16];

	void          *seta_ext;
	void          (*seta_fct) (void *ext, unsigned char val);

	void          *setb_ext;
	void          (*setb_fct) (void *ext, unsigned char val);
} st_psg_t;


void st_psg_init (st_psg_t *psg);
void st_psg_free (st_psg_t *psg);

void st_psg_set_port_a_fct (st_psg_t *psg, void *ext, void *fct);
void st_psg_set_port_b_fct (st_psg_t *psg, void *ext, void *fct);

unsigned char st_psg_get_select (st_psg_t *psg);
void st_psg_set_select (st_psg_t *psg, unsigned char val);

unsigned char st_psg_get_data (st_psg_t *psg);
void st_psg_set_data (st_psg_t *psg, unsigned char val);


#endif
