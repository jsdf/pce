/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/rc759/parport.c                                     *
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


#include "main.h"
#include "parport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/char/char.h>

#include <lib/log.h>
#include <lib/string.h>


#define PAR_CTL_STROBE       0x01
#define PAR_CTL_AUTOLF       0x02
#define PAR_CTL_INIT         0x04
#define PAR_CTL_SELECT       0x08
#define PAR_CTL_DISABLE_DATA 0x10
#define PAR_CTL_DISABLE_IRQ  0x80

#define PAR_ST_BUSY     0x01
#define PAR_ST_ACK      0x02
#define PAR_ST_FAULT    0x04
#define PAR_ST_PAPER    0x08
#define PAR_ST_SELECTED 0x10
#define PAR_ST_STROBE   0x20
#define PAR_ST_INIT     0x40
#define PAR_ST_SELECT   0x80


#ifndef DEBUG_PAR
#define DEBUG_PAR 0
#endif


void rc759_par_init (rc759_parport_t *par)
{
	par->data = 0;
	par->control = 0;
	par->status = 0;

	par->reserve = 0;

	par->cdrv = NULL;

	par->irq_val = 0;
	par->irq_ext = NULL;
	par->irq = NULL;
}

void rc759_par_free (rc759_parport_t *par)
{
	chr_close (par->cdrv);
}

static
void rc759_par_set_irq (rc759_parport_t *par, int val)
{
	val = (val != 0);

	if (par->irq_val == val) {
		return;
	}

#if DEBUG_PAR >= 1
	sim_log_deb ("par: irq=%d\n", val);
#endif

	par->irq_val = val;

	if (par->irq != NULL) {
		par->irq (par->irq_ext, val);
	}
}

void rc759_par_reset (rc759_parport_t *par)
{
	par->data = 0;
	par->control = 0;
	par->status &= 0x1f;
}

void rc759_par_set_irq_fct (rc759_parport_t *par, void *ext, void *fct)
{
	par->irq_ext = ext;
	par->irq = fct;
}

int rc759_par_set_driver (rc759_parport_t *par, const char *name)
{
	if (par->cdrv != NULL) {
		chr_close (par->cdrv);
	}

	par->cdrv = chr_open (name);

	if (par->cdrv == NULL) {
		par->status = PAR_ST_ACK | PAR_ST_FAULT | PAR_ST_SELECTED;
	}
	else {
		par->status = PAR_ST_BUSY | PAR_ST_PAPER;
	}

	if (par->cdrv == NULL) {
		return (1);
	}

	return (0);
}

unsigned char rc759_par_get_data (rc759_parport_t *par)
{
	return (par->data);
}

void rc759_par_set_data (rc759_parport_t *par, unsigned char val)
{
	par->data = val;
}

unsigned char rc759_par_get_control (rc759_parport_t *par)
{
	return (par->control);
}

void rc759_par_set_control (rc759_parport_t *par, unsigned char val)
{
	unsigned char tmp;

#if DEBUG_PAR >= 1
	sim_log_deb ("par: set ctl: %02X\n", val);
#endif

	if (val & PAR_CTL_STROBE) {
		par->status |= PAR_ST_BUSY | PAR_ST_ACK;
	}
	else {
		if (par->control & PAR_CTL_STROBE) {
			chr_write (par->cdrv, &par->data, 1);
		}

		par->status &= ~(PAR_ST_BUSY | PAR_ST_ACK);
	}

	par->control = val;

	tmp = ((val & 0x01) << 1) | (val & 0x0c);

	if (par->cdrv != NULL) {
		tmp |= (val >> 4) & 0x01;
	}

	par->status = (par->status & 0x0f) | (tmp << 4);

	rc759_par_set_irq (par, (~par->control & 0x80) && (~par->status & PAR_ST_BUSY));
}

unsigned char rc759_par_get_status (rc759_parport_t *par)
{
#if DEBUG_PAR >= 1
	sim_log_deb ("par: get status: %02X\n", par->status);
#endif

	return (par->status);
}

void rc759_par_set_status (rc759_parport_t *par, unsigned char val)
{
	par->status = val;
}

unsigned char rc759_par_get_reserve (rc759_parport_t *par)
{
	return (par->reserve);
}

void rc759_par_set_reserve (rc759_parport_t *par, unsigned char val)
{
	par->reserve = val ? 0 : 0x80;
}
