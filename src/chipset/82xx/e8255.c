/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8255.c                                     *
 * Created:     2003-04-17 by Hampa Hug <hampa@hampa.ch>                     *
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


#include <stdlib.h>
#include <stdio.h>

#include "e8255.h"


void e8255_init (e8255_t *ppi)
{
	unsigned i;

	ppi->group_a_mode = 0;
	ppi->group_b_mode = 0;
	ppi->mode = 0x80;

	for (i = 0; i < 3; i++) {
		ppi->port[i].val_inp = 0;
		ppi->port[i].val_out = 0;
		ppi->port[i].inp = 0;
		ppi->port[i].read_ext = NULL;
		ppi->port[i].read = NULL;
		ppi->port[i].write_ext = NULL;
		ppi->port[i].write = NULL;
	}
}

e8255_t *e8255_new (void)
{
	e8255_t *ppi;

	ppi = malloc (sizeof (e8255_t));
	if (ppi == NULL) {
		return (NULL);
	}

	e8255_init (ppi);

	return (ppi);
}

void e8255_free (e8255_t *ppi)
{
}

void e8255_del (e8255_t *ppi)
{
	if (ppi != NULL) {
		e8255_free (ppi);
		free (ppi);
	}
}

void e8255_set_inp (e8255_t *ppi, unsigned p, unsigned char val)
{
	ppi->port[p].val_inp = val;
}

void e8255_set_out (e8255_t *ppi, unsigned p, unsigned char val)
{
	ppi->port[p].val_out = val;

	if (ppi->port[p].inp != 0xff) {
		val &= ~ppi->port[p].inp;
		if (ppi->port[p].write != NULL) {
			ppi->port[p].write (ppi->port[p].write_ext, val);
		}
	}
}

unsigned char e8255_get_inp (e8255_t *ppi, unsigned p)
{
	unsigned char val;

	if (ppi->port[p].inp != 0x00) {
		if (ppi->port[p].read != NULL) {
			ppi->port[p].val_inp = ppi->port[p].read (ppi->port[p].read_ext);
		}
	}

	val = ppi->port[p].val_inp & ppi->port[p].inp;
	val |= ppi->port[p].val_out & ~ppi->port[p].inp;

	return (val);
}

unsigned char e8255_get_out (e8255_t *ppi, unsigned p)
{
	return (ppi->port[p].val_out | ppi->port[p].inp);
}


void e8255_set_inp_a (e8255_t *ppi, unsigned char val)
{
	e8255_set_inp (ppi, 0, val);
}

void e8255_set_inp_b (e8255_t *ppi, unsigned char val)
{
	e8255_set_inp (ppi, 1, val);
}

void e8255_set_inp_c (e8255_t *ppi, unsigned char val)
{
	e8255_set_inp (ppi, 2, val);
}

void e8255_set_out_a (e8255_t *ppi, unsigned char val)
{
	e8255_set_out (ppi, 0, val);
}

void e8255_set_out_b (e8255_t *ppi, unsigned char val)
{
	e8255_set_out (ppi, 1, val);
}

void e8255_set_out_c (e8255_t *ppi, unsigned char val)
{
	e8255_set_out (ppi, 2, val);
}

unsigned char e8255_get_inp_a (e8255_t *ppi)
{
	return (e8255_get_inp (ppi, 0));
}

unsigned char e8255_get_inp_b (e8255_t *ppi)
{
	return (e8255_get_inp (ppi, 1));
}

unsigned char e8255_get_inp_c (e8255_t *ppi)
{
	return (e8255_get_inp (ppi, 2));
}

unsigned char e8255_get_out_a (e8255_t *ppi)
{
	return (e8255_get_out (ppi, 0));
}

unsigned char e8255_get_out_b (e8255_t *ppi)
{
	return (e8255_get_out (ppi, 1));
}

unsigned char e8255_get_out_c (e8255_t *ppi)
{
	return (e8255_get_out (ppi, 2));
}

void e8255_set_uint8 (e8255_t *ppi, unsigned long addr, unsigned char val)
{
	switch (addr) {
		case 0:
			e8255_set_out (ppi, 0, val);
			break;

		case 1:
			e8255_set_out (ppi, 1, val);
			break;

		case 2:
			e8255_set_out (ppi, 2, val);
			break;

		case 3:
			if (val & 0x80) {
				ppi->mode = val;

				ppi->group_a_mode = (val >> 5) & 0x03;
				ppi->group_b_mode = (val >> 2) & 0x01;
				ppi->port[0].inp = (val & 0x10) ? 0xff : 0x00;
				ppi->port[1].inp = (val & 0x02) ? 0xff : 0x00;
				ppi->port[2].inp = (val & 0x01) ? 0x0f : 0x00;
				ppi->port[2].inp |= (val & 0x08) ? 0xf0 : 0x00;
			}
			else {
				unsigned bit;

				bit = (val >> 1) & 0x07;

				if (val & 1) {
					val = ppi->port[2].val_out | (1 << bit);
				}
				else {
					val = ppi->port[2].val_out & ~(1 << bit);
				}

				e8255_set_out (ppi, 2, val);
			}

			break;
	}
}

void e8255_set_uint16 (e8255_t *ppi, unsigned long addr, unsigned short val)
{
	e8255_set_uint8 (ppi, addr, val & 0xff);
}

void e8255_set_uint32 (e8255_t *ppi, unsigned long addr, unsigned long val)
{
	e8255_set_uint8 (ppi, addr, val & 0xff);
}

unsigned char e8255_get_uint8 (e8255_t *ppi, unsigned long addr)
{
	switch (addr) {
		case 0:
			return (e8255_get_inp (ppi, 0));

		case 1:
			return (e8255_get_inp (ppi, 1));

		case 2:
			return (e8255_get_inp (ppi, 2));

		case 3:
			return (ppi->mode);
	}

	return (0);
}

unsigned short e8255_get_uint16 (e8255_t *ppi, unsigned long addr)
{
	return (e8255_get_uint8 (ppi, addr));
}

unsigned long e8255_get_uint32 (e8255_t *ppi, unsigned long addr)
{
	return (e8255_get_uint8 (ppi, addr));
}
