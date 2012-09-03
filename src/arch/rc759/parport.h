/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/parport.h                                     *
 * Created:     2012-07-13 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_RC759_PARPORT_H
#define PCE_RC759_PARPORT_H 1


#include <drivers/char/char.h>


typedef struct {
	unsigned char data;
	unsigned char control;
	unsigned char status;

	unsigned char reserve;

	char_drv_t    *cdrv;

	unsigned char irq_val;
	void          *irq_ext;
	void          (*irq) (void *ext, unsigned char val);
} rc759_parport_t;


void rc759_par_init (rc759_parport_t *par);
void rc759_par_free (rc759_parport_t *par);

void rc759_par_reset (rc759_parport_t *par);

void rc759_par_set_irq_fct (rc759_parport_t *par, void *ext, void *fct);

int rc759_par_set_driver (rc759_parport_t *par, const char *name);

unsigned char rc759_par_get_data (rc759_parport_t *par);
void rc759_par_set_data (rc759_parport_t *par, unsigned char val);

unsigned char rc759_par_get_control (rc759_parport_t *par);
void rc759_par_set_control (rc759_parport_t *par, unsigned char val);

unsigned char rc759_par_get_status (rc759_parport_t *par);
void rc759_par_set_status (rc759_parport_t *par, unsigned char val);

unsigned char rc759_par_get_reserve (rc759_parport_t *par);
void rc759_par_set_reserve (rc759_parport_t *par, unsigned char val);


#endif
