/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/macplus/iwm.c                                       *
 * Created:     2007-11-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2007-2009 Hampa Hug <hampa@hampa.ch>                     *
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
#include "iwm.h"
#include "macplus.h"


void mac_iwm_init (macplus_t *sim)
{
	sim->iwm_lines = 0;
	sim->iwm_status = 0;
	sim->iwm_mode = 0;
}

void mac_iwm_free (macplus_t *sim)
{
}

static
void mac_iwm_access_uint8 (macplus_t *sim, unsigned reg)
{
	switch (reg & 0x0f) {
	case 0x00: /* ca0 low */
		sim->iwm_lines &= ~0x01;
		break;

	case 0x01: /* ca0 high */
		sim->iwm_lines |= 0x01;
		break;

	case 0x02: /* ca1 low */
		sim->iwm_lines &= ~0x02;
		break;

	case 0x03: /* ca1 high */
		sim->iwm_lines |= 0x02;
		break;

	case 0x04: /* ca2 low */
		sim->iwm_lines &= ~0x04;
		break;

	case 0x05: /* ca2 high */
		sim->iwm_lines |= 0x04;
		break;

	case 0x06: /* lstrb low */
		sim->iwm_lines &= ~0x08;
		break;

	case 0x07: /* lstrb high */
		sim->iwm_lines |= 0x08;
		break;

	case 0x08: /* enable low */
		sim->iwm_lines &= ~0x10;
		break;

	case 0x09: /* enable high */
		sim->iwm_lines |= 0x10;
		break;

	case 0x0a: /* select low */
		sim->iwm_lines &= ~0x20;
		break;

	case 0x0b: /* select high */
		sim->iwm_lines |= 0x20;
		break;

	case 0x0c: /* q6 low */
		sim->iwm_lines &= ~0x40;
		break;

	case 0x0d: /* q6 high */
		sim->iwm_lines |= 0x40;
		break;

	case 0x0e: /* q7 low */
		sim->iwm_lines &= ~0x80;
		break;

	case 0x0f: /* q7 high */
		sim->iwm_lines |= 0x80;
		break;
	}
}

unsigned char mac_iwm_get_uint8 (macplus_t *sim, unsigned long addr)
{
	unsigned char val;

	if ((addr & 1) == 0) {
		return (0);
	}

	addr = (addr >> 9) & 0x0f;

	mac_iwm_access_uint8 (sim, addr);

	val = 0;

	switch (sim->iwm_lines & 0xc0) {
	case 0x00: /* read data */
		val = 0x80;
#ifdef DEBUG_IWM
		mac_log_deb ("iwm: read data (%02X)\n", val);
#endif
		break;

	case 0x40: /* read status */
		val = (sim->iwm_status & 0xe0) | (sim->iwm_mode & 0x1f);
		val |= 0x80;
#ifdef DEBUG_IWM
		mac_log_deb ("iwm: read status [%02X] (%02X)\n", sim->iwm_lines, val);
#endif
		break;

	case 0x80: /* read handshake */
#ifdef DEBUG_IWM
		mac_log_deb ("iwm: read handshake (%02X)\n", val);
#endif
		val = 0;
		break;

	case 0xc0: /* ? */
#ifdef DEBUG_IWM
		mac_log_deb ("iwm: get  8: %06lX -> %02X\n", addr, val);
#endif
		break;
	}

	return (val);
}

void mac_iwm_set_uint8 (macplus_t *sim, unsigned long addr, unsigned char val)
{
	if ((addr & 1) == 0) {
		return;
	}

	addr = (addr >> 9) & 0x0f;

	mac_iwm_access_uint8 (sim, addr);

	switch (sim->iwm_lines & 0xc0) {
	case 0xc0: /* mode write */
		if (sim->iwm_lines & 0x10) {
			/* write data */
#ifdef DEBUG_IWM
			mac_log_deb ("iwm: write data (%02X)\n", val);
#endif
			;
		}
		else {
			/* write mode */
			sim->iwm_mode = val;
#ifdef DEBUG_IWM
			mac_log_deb ("iwm: write mode (%02X)\n", val);
#endif
		}
		break;

	default:
#ifdef DEBUG_IWM
		mac_log_deb ("iwm: set  8: %04lX <- %02X\n", addr, val);
#endif
		break;
	}
}
