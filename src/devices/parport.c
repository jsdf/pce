/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/devices/parport.c                                        *
 * Created:     2003-04-29 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2009 Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "parport.h"


void parport_init (parport_t *par, unsigned long base)
{
	par->io = base;

	mem_blk_init (&par->port, base, 4, 0);
	par->port.ext = par;
	par->port.get_uint8 = (mem_get_uint8_f) &parport_get_uint8;
	par->port.set_uint8 = (mem_set_uint8_f) &parport_set_uint8;
	par->port.get_uint16 = (mem_get_uint16_f) &parport_get_uint16;
	par->port.set_uint16 = (mem_set_uint16_f) &parport_set_uint16;

	par->status = 0;
	par->control = 0;
	par->data = 0;

	par->cdrv = NULL;
}

parport_t *parport_new (unsigned long base)
{
	parport_t *par;

	par = malloc (sizeof (parport_t));
	if (par == NULL) {
		return (NULL);
	}

	parport_init (par, base);

	return (par);
}

void parport_free (parport_t *par)
{
	chr_close (par->cdrv);

	mem_blk_free (&par->port);
}

void parport_del (parport_t *par)
{
	if (par != NULL) {
		parport_free (par);
		free (par);
	}
}

mem_blk_t *parport_get_reg (parport_t *par)
{
	return (&par->port);
}

int parport_set_driver (parport_t *par, const char *name)
{
	if (par->cdrv != NULL) {
		chr_close (par->cdrv);
	}

	par->cdrv = chr_open (name);

	if (par->cdrv == NULL) {
		par->status &= ~(PARPORT_BSY | PARPORT_OFF | PARPORT_ERR);
	}
	else {
		par->status |= (PARPORT_BSY | PARPORT_OFF | PARPORT_ERR);
	}

	if (par->cdrv == NULL) {
		return (1);
	}

	return (0);
}

void parport_set_control (parport_t *par, unsigned char val)
{
	par->control = val;

	if (val & PARPORT_STR) {
		chr_write (par->cdrv, &par->data, 1);

		par->status |= PARPORT_BSY;
		par->status &= ~PARPORT_ACK;
	}
}

void parport_set_uint8 (parport_t *par, unsigned long addr, unsigned char val)
{
	switch (addr) {
	case 0x00:
		par->data = val;
		break;

	case 0x01:
		break;

	case 0x02:
		parport_set_control (par, val);
		break;
	}
}

void parport_set_uint16 (parport_t *par, unsigned long addr, unsigned short val)
{
	parport_set_uint8 (par, addr, val);
}

unsigned char parport_get_uint8 (parport_t *par, unsigned long addr)
{
	switch (addr) {
	case 0x00:
		return (par->data);

	case 0x01:
		return (par->status);

	case 0x02:
		return (par->control);

	default:
		return (0xff);
	}
}

unsigned short parport_get_uint16 (parport_t *par, unsigned long addr)
{
	return (parport_get_uint8 (par, addr));
}
