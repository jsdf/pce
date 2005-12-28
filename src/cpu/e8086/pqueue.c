/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:     src/cpu/e8086/pqueue.c                                     *
 * Created:       2003-09-20 by Hampa Hug <hampa@hampa.ch>                   *
 * Last modified: 2004-11-27 by Hampa Hug <hampa@hampa.ch>                   *
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


#include "e8086.h"
#include "internal.h"


void e86_pq_init (e8086_t *c)
{
	c->pq_cnt = 0;
}

void e86_pq_fill (e8086_t *c)
{
	unsigned       i;
	unsigned short val;
	unsigned short seg, ofs;

	seg = e86_get_cs (c);
	ofs = e86_get_ip (c);

	if (ofs <= (0xffff - E86_PQ_FILL)) {
		unsigned long addr;

		addr = e86_get_linear (seg, ofs);

		if (addr <= (c->ram_cnt - E86_PQ_FILL)) {
			for (i = c->pq_cnt; i < E86_PQ_FILL; i++) {
				c->pq[i] = c->ram[addr + i];
			}
		}
		else {
			i = c->pq_cnt;
			while (i < E86_PQ_FILL) {
				val = c->mem_get_uint16 (c->mem, addr + i);
				c->pq[i] = val & 0xff;
				c->pq[i + 1] = (val >> 8) & 0xff;
				i += 2;
			}
		}
	}
	else {
		i = c->pq_cnt;
		while (i < E86_PQ_FILL) {
			val = e86_get_mem16 (c, seg, ofs + i);
			c->pq[i] = val & 0xff;
			c->pq[i + 1] = (val >> 8) & 0xff;

			i += 2;
		}
	}

	c->pq_cnt = E86_PQ_SIZE;
}

void e86_pq_adjust (e8086_t *c, unsigned cnt)
{
	unsigned      n;
	unsigned char *d, *s;

	if (cnt >= c->pq_cnt) {
		c->pq_cnt = 0;
		return;
	}

	n = c->pq_cnt - cnt;
	s = c->pq + cnt;
	d = c->pq;

	while (n > 0) {
		*(d++) = *(s++);
		n -= 1;
	}

	c->pq_cnt -= cnt;
}
