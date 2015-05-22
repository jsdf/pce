/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/dos/dosmem.c                                        *
 * Created:     2012-12-31 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2012-2015 Hampa Hug <hampa@hampa.ch>                     *
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
#include "dos.h"
#include "dosmem.h"


unsigned short sim_mem_alloc (dos_t *sim, unsigned para_min, unsigned para_max)
{
	unsigned short seg, seg2, para;
	unsigned char  sig;
	unsigned short own, cnt;

	seg = sim->mem_start;

	while (1) {
		sig = sim_get_uint8 (sim, seg, 0);
		own = sim_get_uint16 (sim, seg, 1);
		cnt = sim_get_uint16 (sim, seg, 3);

		if ((own == 0) && (cnt >= para_min)) {
			para = para_max;

			if (para > cnt) {
				para = cnt;
			}

			if ((cnt - para) > 1) {
				sim_set_uint8 (sim, seg, 0, 0x4d);
				sim_set_uint16 (sim, seg, 3, para);

				seg2 = seg + para + 1;

				sim_set_uint8 (sim, seg2, 0, sig);
				sim_set_uint16 (sim, seg2, 1, 0);
				sim_set_uint16 (sim, seg2, 3, cnt - para - 1);
			}

			sim_set_uint16 (sim, seg, 1, seg + 1);

			return (seg + 1);
		}

		if (sig != 0x4d) {
			return (0);
		}

		seg += cnt + 1;
	}
}

unsigned short sim_mem_resize (dos_t *sim, unsigned short blk, unsigned short para)
{
	unsigned short max;
	unsigned short seg1, seg2;
	unsigned char  sig1, sig2;
	unsigned short own1, own2;
	unsigned short cnt1, cnt2;

	seg1 = blk - 1;
	sig1 = sim_get_uint8 (sim, seg1, 0);
	own1 = sim_get_uint16 (sim, seg1, 1);
	cnt1 = sim_get_uint16 (sim, seg1, 3);

	if ((sig1 != 0x4d) && (sig1 != 0x5a)) {
		return (0);
	}

	if (sig1 != 0x4d) {
		seg2 = 0;
		sig2 = 0;
		own2 = 0;
		cnt2 = 0;
	}
	else {
		seg2 = seg1 + cnt1 + 1;
		sig2 = sim_get_uint8 (sim, seg2, 0);
		own2 = sim_get_uint16 (sim, seg2, 1);
		cnt2 = sim_get_uint16 (sim, seg2, 3);

		if ((sig2 != 0x4d) && (sig2 != 0x5a)) {
			return (0);
		}
	}

	if ((sig2 != 0) && (own2 == 0)) {
		max = cnt1 + cnt2 + 1;
	}
	else {
		max = cnt1;
	}

	if (para > max) {
		return (0);
	}

	if ((sig2 != 0) && (own2 == 0)) {
		sig1 = sig2;
		cnt1 += cnt2 + 1;
	}

	if ((cnt1 - para) > 1) {
		seg2 = seg1 + para + 1;

		sim_set_uint8 (sim, seg2, 0, sig1);
		sim_set_uint16 (sim, seg2, 1, 0);
		sim_set_uint16 (sim, seg2, 3, cnt1 - para - 1);

		sig1 = 0x4d;
	}

	sim_set_uint8 (sim, seg1, 0, sig1);
	sim_set_uint16 (sim, seg1, 1, own1);
	sim_set_uint16 (sim, seg1, 3, para);

	return (seg1 + 1);
}

int sim_mem_free (dos_t *sim, unsigned short blk)
{
	unsigned short seg1, seg2;
	unsigned char  sig1, sig2;
	unsigned short own2;
	unsigned short cnt1, cnt2;

	seg1 = blk - 1;
	sig1 = sim_get_uint8 (sim, seg1, 0);

	if ((sig1 != 0x4d) && (sig1 != 0x5a)) {
		return (1);
	}

	sim_set_uint16 (sim, seg1, 1, 0);

	if (sig1 != 0x4d) {
		return (0);
	}

	cnt1 = sim_get_uint16 (sim, seg1, 3);

	seg2 = seg1 + cnt1 + 1;
	sig2 = sim_get_uint8 (sim, seg2, 0);
	own2 = sim_get_uint16 (sim, seg2, 1);
	cnt2 = sim_get_uint16 (sim, seg2, 3);

	if ((sig2 != 0x4d) && (sig2 != 0x5a)) {
		return (1);
	}

	if ((sig2 != 0) && (own2 == 0)) {
		sim_set_uint8 (sim, seg1, 0, sig2);
		sim_set_uint16 (sim, seg1, 3, cnt1 + cnt2 + 1);
	}

	return (0);
}

unsigned short sim_mem_get_max (dos_t *sim)
{
	unsigned short seg, own, cnt, max;
	unsigned char  sig;

	seg = sim->mem_start;
	max = 0;

	while (1) {
		sig = sim_get_uint8 (sim, seg, 0);
		own = sim_get_uint16 (sim, seg, 1);
		cnt = sim_get_uint16 (sim, seg, 3);

		if ((own == 0) && (cnt > max)) {
			max = cnt;
		}

		if (sig != 0x4d) {
			return (max);
		}

		seg += cnt + 1;
	}
}

unsigned short sim_mem_get_size (dos_t *sim, unsigned short blk)
{
	return (sim_get_uint16 (sim, blk - 1, 3));
}
